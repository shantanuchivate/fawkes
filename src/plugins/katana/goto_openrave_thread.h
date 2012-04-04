
/***************************************************************************
 *  goto_openrave_thread.h - Katana goto one-time thread using openrave lib
 *
 *  Created: Wed Jun 10 11:44:24 2009
 *  Copyright  2006-2009  Tim Niemueller [www.niemueller.de]
 *                  2010  Bahram Maleki-Fard
 *
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

#ifndef __PLUGINS_KATANA_GOTO_OPENRAVE_THREAD_H_
#define __PLUGINS_KATANA_GOTO_OPENRAVE_THREAD_H_

#include "motion_thread.h"

#include <vector>
#include <string>

#include <plugins/openrave/aspect/openrave.h>

#ifdef HAVE_OPENRAVE
namespace fawkes {
  class OpenRaveRobot;
  class OpenRaveManipulatorKatana6M180;
}
#endif

/** class KatanaGotoOpenRaveThread */
class KatanaGotoOpenRaveThread : public KatanaMotionThread
{
#ifdef HAVE_OPENRAVE

 public:
  KatanaGotoOpenRaveThread(fawkes::RefPtr<CLMBase> katana, fawkes::Logger *logger, fawkes::OpenRaveConnector* openrave,
		   unsigned int poll_interval_ms,
                   std::string robot_file,
                   bool autoload_IK,
                   bool use_viewer);

  virtual void once();
  virtual void init();
  virtual void finalize();

  void set_target(float x, float y, float z, float phi, float theta, float psi);
  void set_target(const std::string& object_name, float rot_x);

  virtual void update_openrave_data();
  virtual bool update_motor_data();
  virtual bool move_katana();

 private:
  fawkes::OpenRaveRobot*                        __OR_robot;
  fawkes::OpenRaveManipulatorKatana6M180*       __OR_manip;

  std::string                                   __target_object;
  std::vector< std::vector<float> >*            __target_traj;
  std::vector< std::vector<float> >::iterator   __it;

  std::vector< int >    __motor_encoders;
  std::vector< float >  __motor_angles;

  const std::string     __cfg_robot_file;
  bool                  __cfg_autoload_IK;
  bool                  __cfg_use_viewer;

  bool                  __is_target_object;

  fawkes::OpenRaveConnector*    _openrave;

  float __x, __y, __z;
  float __phi, __theta, __psi;
  unsigned int __poll_interval_usec;

#endif //HAVE_OPENRAVE
};

#endif