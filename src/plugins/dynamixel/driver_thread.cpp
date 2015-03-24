
/***************************************************************************
 *  driver_thread.cpp - Robotis dynamixel servo driver thread
 *
 *  Created: Mon Mar 23 20:37:32 2015 (based on pantilt plugin)
 *  Copyright  2006-2015  Tim Niemueller [www.niemueller.de]
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "driver_thread.h"
#include "servo_chain.h"

#include <utils/math/angle.h>
#include <core/threading/mutex_locker.h>
#include <core/threading/read_write_lock.h>
#include <core/threading/scoped_rwlock.h>
#include <core/threading/wait_condition.h>
#include <interfaces/DynamixelServoInterface.h>
#include <interfaces/LedInterface.h>
#include <interfaces/JointInterface.h>
#include <utils/misc/string_split.h>

#include <cstdarg>
#include <cmath>
#include <unistd.h>
#include <algorithm>
#include <cstring>

using namespace fawkes;

/** @class DynamixelDriverThread "driver_thread.h"
 * Driver thread for Robotis dynamixel servos.
 * @author Tim Niemueller
 */

/** Constructor.
 * @param cfg_prefix configuration prefix specific for the servo chain
 * @param cfg_name name of the servo configuration
 */
DynamixelDriverThread::DynamixelDriverThread(std::string &cfg_name,
					     std::string &cfg_prefix)
  : Thread("DynamixelDriverThread", Thread::OPMODE_WAITFORWAKEUP),
    BlackBoardInterfaceListener("DynamixelDriverThread(%s)", cfg_name.c_str())
{
  set_name("DynamixelDriverThread(%s)", cfg_name.c_str());

  cfg_prefix_ = cfg_prefix;
  cfg_name_   = cfg_name;
}


void
DynamixelDriverThread::init()
{
  cfg_device_           = config->get_string((cfg_prefix_ + "device").c_str());
  cfg_read_timeout_ms_  = config->get_uint((cfg_prefix_ + "read_timeout_ms").c_str());
  cfg_disc_timeout_ms_  = config->get_uint((cfg_prefix_ + "discover_timeout_ms").c_str());
  cfg_goto_zero_start_  = config->get_bool((cfg_prefix_ + "goto_zero_start").c_str());
  cfg_turn_off_         = config->get_bool((cfg_prefix_ + "turn_off").c_str());
  cfg_cw_compl_margin_  = config->get_uint((cfg_prefix_ + "cw_compl_margin").c_str());
  cfg_ccw_compl_margin_ = config->get_uint((cfg_prefix_ + "ccw_compl_margin").c_str());
  cfg_cw_compl_slope_   = config->get_uint((cfg_prefix_ + "cw_compl_slope").c_str());
  cfg_ccw_compl_slope_  = config->get_uint((cfg_prefix_ + "ccw_compl_slope").c_str());
  cfg_def_angle_margin_ = config->get_float((cfg_prefix_ + "angle_margin").c_str());
  cfg_enable_echo_fix_  = config->get_bool((cfg_prefix_ + "enable_echo_fix").c_str());

  chain_ = new DynamixelChain(cfg_device_.c_str(), cfg_read_timeout_ms_);
  DynamixelChain::DeviceList devl = chain_->discover();
  std::list<std::string> found_servos;
  for (DynamixelChain::DeviceList::iterator i = devl.begin(); i != devl.end(); ++i) {
    found_servos.push_back(std::to_string(*i));
    Servo s;
    s.servo_if = NULL;
    s.led_if = NULL;
    s.joint_if = NULL;

    try {
      s.servo_if =
	blackboard->open_for_writing_f<DynamixelServoInterface>("/dynamixel/%s/%u",
								cfg_name_.c_str(), *i);
      s.led_if =
	blackboard->open_for_writing_f<LedInterface>("/dynamixel/%s/%u",
						     cfg_name_.c_str(), *i);
      s.joint_if =
	blackboard->open_for_writing_f<JointInterface>("/dynamixel/%s/%u",
						       cfg_name_.c_str(), *i);

      bbil_add_message_interface(s.servo_if);

    } catch (Exception &e) {
      blackboard->close(s.servo_if);
      blackboard->close(s.led_if);
      blackboard->close(s.joint_if);
      throw;
    }

    s.move_pending     = false;
    s.target_angle     = 0;
    s.velo_pending     = false;
    s.vel              = 0.;
    s.enable           = false;
    s.disable          = false;
    s.led_enable       = false;
    s.led_disable      = false;
    s.last_angle       = 0.f;
    s.value_rwlock     = new ReadWriteLock();
    s.angle_margin     = cfg_def_angle_margin_;

    servos_[*i] = s;
  }

  logger->log_info(name(), "Found servos [%s]", str_join(found_servos, ",").c_str());

  chain_rwlock_      = new ReadWriteLock();
  fresh_data_mutex_ = new Mutex();
  update_waitcond_  = new WaitCondition();

  if (servos_.empty()) {
    throw Exception("No servos found in chain %s", cfg_name_.c_str());
  }

  // We only want responses to be sent on explicit READ to speed up communication
  chain_->set_status_return_level(DynamixelChain::BROADCAST_ID, DynamixelChain::SRL_RESPOND_READ);
  // set compliance values
  chain_->set_compliance_values(DynamixelChain::BROADCAST_ID,
				cfg_cw_compl_margin_, cfg_cw_compl_slope_,
				cfg_ccw_compl_margin_, cfg_ccw_compl_slope_);

  for (auto &sp : servos_) {
    unsigned int servo_id = sp.first;
    Servo &s = sp.second;



    chain_->set_led_enabled(servo_id, false);
    chain_->set_torque_enabled(servo_id, true);

    chain_->read_table_values(servo_id);

    s.max_speed = chain_->get_max_supported_speed(servo_id);
    
    unsigned int cw_limit, ccw_limit;
    chain_->get_angle_limits(servo_id, cw_limit, ccw_limit);

    unsigned char cw_margin, cw_slope, ccw_margin, ccw_slope;
    chain_->get_compliance_values(servo_id, cw_margin, cw_slope, ccw_margin, ccw_slope);

    s.servo_if->set_model(chain_->get_model(servo_id));
    s.servo_if->set_model_number(chain_->get_model_number(servo_id));
    s.servo_if->set_cw_angle_limit(cw_limit);
    s.servo_if->set_ccw_angle_limit(ccw_limit);
    s.servo_if->set_temperature_limit(chain_->get_temperature_limit(servo_id));
    s.servo_if->set_max_torque(chain_->get_max_torque(servo_id));
    s.servo_if->set_cw_slope(cw_slope);
    s.servo_if->set_ccw_slope(ccw_slope);
    s.servo_if->set_cw_margin(cw_margin);
    s.servo_if->set_ccw_margin(ccw_margin);
    s.servo_if->set_torque_limit(chain_->get_torque_limit(servo_id));
    s.servo_if->set_max_velocity(s.max_speed);
    s.servo_if->write();

    s.servo_if->set_auto_timestamping(false);
  }

  if ( cfg_goto_zero_start_ ) {
    for (auto &s : servos_) {
      goto_angle_timed(s.first, 0., 3.0);
    }
  }

  blackboard->register_listener(this);
}

void
DynamixelDriverThread::finalize()
{
  blackboard->unregister_listener(this);

  for (auto &s : servos_) {
    blackboard->close(s.second.servo_if);
    blackboard->close(s.second.led_if);
    blackboard->close(s.second.joint_if);
  }

  delete chain_rwlock_;
  delete fresh_data_mutex_;
  delete update_waitcond_;

  if (cfg_turn_off_) {
    for (auto &s : servos_) {
      try {
	logger->log_debug(name(), "Turning off servo %s:%u", cfg_name_.c_str(), s.first);
	chain_->set_led_enabled(s.first,  false);
	chain_->set_torque_enabled(s.first, false);
      } catch (Exception &e) {
	logger->log_warn(name(), "Failed to turn of servo %s:%u: %s",
			 cfg_name_.c_str(), s.first, e.what());
      }
    }
    // Give some time for shutdown comands to get through
    usleep(10000);
  }

  // Setting to NULL deletes instance (RefPtr)
  chain_ = NULL;
}


/** Update sensor values as necessary.
 * To be called only from PanTiltSensorThread. Writes the current pan/tilt
 * data into the interface.
 */
void
DynamixelDriverThread::exec_sensor()
{
  if (has_fresh_data()) {
    for (auto &sp : servos_) {
      unsigned int servo_id = sp.first;
      Servo &s = sp.second;

      fawkes::Time time;
      float angle = get_angle(servo_id, time);
      float vel   = get_velocity(servo_id);
    
      // poor man's filter: only update if we get a change of least half a degree
      if (fabs(s.last_angle - angle) >= deg2rad(0.5)) {
	s.last_angle = angle;
      } else {
	angle = s.last_angle;
      }

      ScopedRWLock lock(chain_rwlock_, ScopedRWLock::LOCK_READ);

      s.servo_if->set_timestamp(&s.time);
      s.servo_if->set_position(chain_->get_position(servo_id));
      s.servo_if->set_speed(chain_->get_speed(servo_id));
      s.servo_if->set_goal_position(chain_->get_goal_position(servo_id));
      s.servo_if->set_goal_speed(chain_->get_goal_speed(servo_id));
      s.servo_if->set_load(chain_->get_load(servo_id));
      s.servo_if->set_voltage(chain_->get_voltage(servo_id));
      s.servo_if->set_temperature(chain_->get_temperature(servo_id));
      s.servo_if->set_punch(chain_->get_punch(servo_id));
      s.servo_if->set_angle(angle);
      s.servo_if->set_velocity(vel);
      s.servo_if->set_enabled(chain_->is_torque_enabled(servo_id));
      s.servo_if->set_final(is_final(servo_id));
      s.servo_if->set_velocity(get_velocity(servo_id));
      s.servo_if->write();

      s.joint_if->set_position(angle);
      s.joint_if->set_velocity(vel);
      s.joint_if->write();
    }
  }
}


/** Process commands. */
void
DynamixelDriverThread::exec_act()
{
  for (auto &sp : servos_) {
    unsigned int servo_id = sp.first;
    Servo &s = sp.second;

    s.servo_if->set_final(is_final(servo_id));

    while (! s.servo_if->msgq_empty() ) {
      if (s.servo_if->msgq_first_is<DynamixelServoInterface::GotoMessage>()) {
      DynamixelServoInterface::GotoMessage *msg = s.servo_if->msgq_first(msg);

      goto_angle(servo_id, msg->angle());
      s.servo_if->set_msgid(msg->id());
      s.servo_if->set_final(false);

      } else if (s.servo_if->msgq_first_is<DynamixelServoInterface::TimedGotoMessage>()) {
	DynamixelServoInterface::TimedGotoMessage *msg = s.servo_if->msgq_first(msg);

	goto_angle_timed(servo_id, msg->angle(), msg->time_sec());
	s.servo_if->set_msgid(msg->id());
	s.servo_if->set_final(false);

      } else if (s.servo_if->msgq_first_is<DynamixelServoInterface::SetEnabledMessage>()) {
	DynamixelServoInterface::SetEnabledMessage *msg = s.servo_if->msgq_first(msg);

	set_enabled(servo_id, msg->is_enabled());
	
      } else if (s.servo_if->msgq_first_is<DynamixelServoInterface::SetVelocityMessage>()) {
	DynamixelServoInterface::SetVelocityMessage *msg = s.servo_if->msgq_first(msg);

	if (msg->velocity() > s.servo_if->max_velocity()) {
	  logger->log_warn(name(), "Desired velocity %f too high, max is %f",
			   msg->velocity(), s.servo_if->max_velocity());
	} else {
	  set_velocity(servo_id, msg->velocity());
	}

      } else if (s.servo_if->msgq_first_is<DynamixelServoInterface::SetMarginMessage>()) {
	DynamixelServoInterface::SetMarginMessage *msg = s.servo_if->msgq_first(msg);

	set_margin(servo_id, msg->angle_margin());
	s.servo_if->set_angle_margin(msg->angle_margin());
	
      } else {
	logger->log_warn(name(), "Unknown message received");
      }

      s.servo_if->msgq_pop();
    }

    s.servo_if->write();

    bool write_led_if = false;
    while (! s.led_if->msgq_empty() ) {
      write_led_if = true;
      if (s.led_if->msgq_first_is<LedInterface::SetIntensityMessage>()) {
	LedInterface::SetIntensityMessage *msg = s.led_if->msgq_first(msg);
	set_led_enabled(servo_id, (msg->intensity() >= 0.5));
	s.led_if->set_intensity((msg->intensity() >= 0.5) ? LedInterface::ON : LedInterface::OFF);
      } else if (s.led_if->msgq_first_is<LedInterface::TurnOnMessage>()) {
	set_led_enabled(servo_id, true);
	s.led_if->set_intensity(LedInterface::ON);
      } else if (s.led_if->msgq_first_is<LedInterface::TurnOffMessage>()) {
	set_led_enabled(servo_id, false);
	s.led_if->set_intensity(LedInterface::OFF);
      }
      
      s.led_if->msgq_pop();
    }
    if (write_led_if) s.led_if->write();
  }
}


bool
DynamixelDriverThread::bb_interface_message_received(Interface *interface,
							Message *message) throw()
{
  std::map<unsigned int, Servo>::iterator si =
    std::find_if(servos_.begin(), servos_.end(),
		 [interface](const std::pair<unsigned int, Servo> &sp){
		   return (strcmp(sp.second.servo_if->uid(), interface->uid()) == 0);
		 });
  if (si != servos_.end()) {
    if (message->is_of_type<DynamixelServoInterface::StopMessage>()) {
      stop_motion(si->first);
      return false; // do not enqueue StopMessage
    } else if (message->is_of_type<DynamixelServoInterface::FlushMessage>()) {
      stop_motion(si->first);
      logger->log_info(name(), "Flushing message queue");
      si->second.servo_if->msgq_flush();
      return false;
    } else {
      logger->log_info(name(), "Received message of type %s, enqueueing", message->type());
      return true;
    }
  }
  return true;
}


/** Enable or disable servo.
 * @param enabled true to enable servos, false to turn them off
 */
void
DynamixelDriverThread::set_enabled(unsigned int servo_id, bool enabled)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set LED",
		     servo_id, cfg_name_.c_str());
    return;
  }

  Servo &s = servos_[servo_id];

  logger->log_debug(name(), "Lock %zu", __LINE__);
  ScopedRWLock lock(s.value_rwlock);
  if (enabled) {
    s.enable  = true;
    s.disable = false;
    } else {
    s.enable  = false;
    s.disable = true;
  }
  wakeup();
  logger->log_debug(name(), "UNLock %zu", __LINE__);
}


/** Enable or disable LED.
 * @param enabled true to enable LED, false to turn it off
 */
void
DynamixelDriverThread::set_led_enabled(unsigned int servo_id, bool enabled)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set LED",
		     servo_id, cfg_name_.c_str());
    return;
  }

  Servo &s = servos_[servo_id];
  logger->log_debug(name(), "Lock %zu", __LINE__);
  ScopedRWLock lock(s.value_rwlock);
  if (enabled) {
    s.led_enable  = true;
    s.led_disable = false;
  } else {
    s.led_enable  = false;
    s.led_disable = true;
  }
  wakeup();
  logger->log_debug(name(), "UNLock %zu", __LINE__);
}


/** Stop currently running motion. */
void
DynamixelDriverThread::stop_motion(unsigned int servo_id)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set LED",
		     servo_id, cfg_name_.c_str());
    return;
  }

  float angle = get_angle(servo_id);
  goto_angle(servo_id, angle);
}


/** Goto desired pan/tilt values.
 * @param pan pan in radians
 * @param tilt tilt in radians
 */
void
DynamixelDriverThread::goto_angle(unsigned int servo_id, float angle)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set LED",
		     servo_id, cfg_name_.c_str());
    return;
  }

  Servo &s = servos_[servo_id];

  logger->log_debug(name(), "Lock %zu", __LINE__);
  ScopedRWLock lock(s.value_rwlock);
  s.target_angle = angle;
  s.move_pending = true;
  wakeup();
  logger->log_debug(name(), "UNLock %zu", __LINE__);
}


/** Goto desired pan/tilt values in a specified time.
 * @param pan pan in radians
 * @param tilt tilt in radians
 * @param time_sec time when to reach the desired pan/tilt values
 */
void
DynamixelDriverThread::goto_angle_timed(unsigned int servo_id, float angle, float time_sec)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set LED",
		     servo_id, cfg_name_.c_str());
    return;
  }
  Servo &s = servos_[servo_id];

  s.target_angle = angle;
  s.move_pending = true;

  float cangle = get_angle(servo_id);
  float angle_diff  = fabs(angle - cangle);
  float req_angle_vel  = angle_diff  / time_sec;

  if (req_angle_vel > s.max_speed) {
    logger->log_warn(name(), "Requested move to %f in %f sec requires a "
		     "angle speed of %f rad/s, which is greater than the maximum "
		     "of %f rad/s, reducing to max", angle, time_sec, req_angle_vel);
    req_angle_vel = s.max_speed;
  }
  set_velocity(servo_id, req_angle_vel);

  wakeup();
}


/** Set desired velocities.
 * @param pan_vel pan velocity
 * @param tilt_vel tilt velocity
 */
void
DynamixelDriverThread::set_velocity(unsigned int servo_id, float vel)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set velocity",
		     servo_id, cfg_name_.c_str());
    return;
  }
  Servo &s = servos_[servo_id];

  ScopedRWLock lock(s.value_rwlock);
  float velo_tmp  = roundf((vel  / s.max_speed)  * DynamixelChain::MAX_SPEED);

  if ((velo_tmp >= 0) && (velo_tmp <= DynamixelChain::MAX_SPEED)) {
    s.vel = (unsigned int)velo_tmp;
    s.velo_pending = true;
  } else {
    logger->log_warn(name(), "Calculated velocity value out of bounds, "
		      "min: 0  max: %u  des: %u",
		      DynamixelChain::MAX_SPEED, (unsigned int)velo_tmp);
  }
}


/** Get current velocities.
 * @param pan_vel upon return contains current pan velocity
 * @param tilt_vel upon return contains current tilt velocity
 */
float
DynamixelDriverThread::get_velocity(unsigned int servo_id)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set velocity",
		     servo_id, cfg_name_.c_str());
    return 0.;
  }
  Servo &s = servos_[servo_id];

  unsigned int velticks  = chain_->get_speed(servo_id);

  return
    (((float)velticks  / (float)DynamixelChain::MAX_SPEED) * s.max_speed);
}


/** Set desired velocities.
 * @param pan_margin pan margin
 * @param tilt_margin tilt margin
 */
void
DynamixelDriverThread::set_margin(unsigned int servo_id, float angle_margin)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set velocity",
		     servo_id, cfg_name_.c_str());
    return;
  }
  Servo &s = servos_[servo_id];
  if (angle_margin  > 0.0) s.angle_margin  = angle_margin;
}


/** Get pan/tilt value.
 * @param pan upon return contains the current pan value
 * @param tilt upon return contains the current tilt value
 */
float
DynamixelDriverThread::get_angle(unsigned int servo_id)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set velocity",
		     servo_id, cfg_name_.c_str());
    return 0.;
  }

  ScopedRWLock lock(chain_rwlock_, ScopedRWLock::LOCK_READ);

  int ticks  = ((int)chain_->get_position(servo_id)  - (int)DynamixelChain::CENTER_POSITION);

  return ticks * DynamixelChain::RAD_PER_POS_TICK;
}


/** Get pan/tilt value with time.
 * @param pan upon return contains the current pan value
 * @param tilt upon return contains the current tilt value
 * @param time upon return contains the time the pan and tilt values were read
 */
float
DynamixelDriverThread::get_angle(unsigned int servo_id,
				    fawkes::Time &time)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set velocity",
		     servo_id, cfg_name_.c_str());
    return 0.;
  }
  Servo &s = servos_[servo_id];

  time = s.time;
  return get_angle(servo_id);
}


/** Check if motion is final.
 * @return true if motion is final, false otherwise
 */
bool
DynamixelDriverThread::is_final(unsigned int servo_id)
{
  if (servos_.find(servo_id) == servos_.end()) {
    logger->log_warn(name(), "No servo with ID %u in chain %s, cannot set velocity",
		     servo_id, cfg_name_.c_str());
    return 0.;
  }
  Servo &s = servos_[servo_id];

  float angle = get_angle(servo_id);

  ScopedRWLock lock(chain_rwlock_, ScopedRWLock::LOCK_READ);

  return  ( (fabs(angle  - s.target_angle)  <= s.angle_margin) ||
	    (! chain_->is_moving(servo_id)));
}


/** Check if servo is enabled.
 * @return true if torque is enabled for both servos, false otherwise
 */
bool
DynamixelDriverThread::is_enabled(unsigned int servo_id)
{
  return chain_->is_torque_enabled(servo_id);
}


/** Check is fresh sensor data is available.
 * Note that this method will return true at once per sensor update cycle.
 * @return true if fresh data is available, false otherwise
 */
bool
DynamixelDriverThread::has_fresh_data()
{
  MutexLocker lock(fresh_data_mutex_);

  bool rv = fresh_data_;
  fresh_data_ = false;
  return rv;
}


void
DynamixelDriverThread::loop()
{
  for (auto &sp : servos_) {
    unsigned int servo_id = sp.first;
    Servo &s = sp.second;
    if (s.enable) {
      s.value_rwlock->lock_for_write();
      s.enable  = false;
      s.value_rwlock->unlock();
      ScopedRWLock lock(chain_rwlock_);
      chain_->set_led_enabled(servo_id, true);
      chain_->set_torque_enabled(servo_id, true);
      if (s.led_enable || s.led_disable || s.velo_pending || s.move_pending) usleep(3000);
    } else if (s.disable) {
      s.value_rwlock->lock_for_write();
      s.disable = false;
      s.value_rwlock->unlock();
      ScopedRWLock lock(chain_rwlock_);
      chain_->set_torque_enabled(servo_id, false);
      if (s.led_enable || s.led_disable || s.velo_pending || s.move_pending) usleep(3000);
    }

    if (s.led_enable) {
      s.value_rwlock->lock_for_write();
      s.led_enable = false;
      s.value_rwlock->unlock();    
      ScopedRWLock lock(chain_rwlock_);
      chain_->set_led_enabled(servo_id, true);
      if (s.velo_pending || s.move_pending) usleep(3000);
    } else if (s.led_disable) {
      s.value_rwlock->lock_for_write();
      s.led_disable = false;
      s.value_rwlock->unlock();    
      ScopedRWLock lock(chain_rwlock_);
      chain_->set_led_enabled(servo_id, false);    
      if (s.velo_pending || s.move_pending) usleep(3000);
    }

    if (s.velo_pending) {
      s.value_rwlock->lock_for_write();
      s.velo_pending = false;
      unsigned int vel  = s.vel;
      s.value_rwlock->unlock();
      ScopedRWLock lock(chain_rwlock_);
      chain_->set_goal_speed(servo_id, vel);
      if (s.move_pending) usleep(3000);
    }

    if (s.move_pending) {
      s.value_rwlock->lock_for_write();
      s.move_pending    = false;
      float target_angle  = s.target_angle;
      s.value_rwlock->unlock();
      exec_goto_angle(servo_id, target_angle);
    }

    try {
      ScopedRWLock lock(chain_rwlock_, ScopedRWLock::LOCK_READ);
      chain_->read_table_values(servo_id);

      MutexLocker lock_fresh_data(fresh_data_mutex_);
      fresh_data_ = true;
      s.time.stamp();
    } catch (Exception &e) {
      // usually just a timeout, too noisy
      //logger_->log_warn(name(), "Error while reading table values from servos, exception follows");
      //logger_->log_warn(name(), e);
    }
  }

  update_waitcond_->wake_all();

  // Wakeup ourselves for faster updates
  wakeup();
}


/** Execute pan/tilt motion.
 * @param angle_rad angle in rad to move to
 */
void
DynamixelDriverThread::exec_goto_angle(unsigned int servo_id, float angle_rad)
{
  /*
  if ( (pan_rad < pan_min_) || (pan_rad > pan_max_) ) {
    logger_->log_warn(name(), "Pan value out of bounds, min: %f  max: %f  des: %f",
		       pan_min_, pan_max_, pan_rad);
    return;
  }
  */

  unsigned int pos_min = 0, pos_max = 0;
  chain_->get_angle_limits(servo_id, pos_min, pos_max);

  int pos  = (int)roundf(DynamixelChain::POS_TICKS_PER_RAD * angle_rad)
    + DynamixelChain::CENTER_POSITION;

  if ( (pos < 0) || ((unsigned int)pos < pos_min) || ((unsigned int)pos > pos_max) ) {
    logger->log_warn(name(), "Position out of bounds, min: %u  max: %u  des: %i",
		     pos_min, pos_max, pos);
    return;
  }

  ScopedRWLock lock(chain_rwlock_);
  chain_->goto_position(servo_id, pos);
}


/** Wait for fresh data to be received.
 * Blocks the calling thread.
 */
void
DynamixelDriverThread::wait_for_fresh_data()
{
  update_waitcond_->wait();
}
