
/***************************************************************************
 *  messages.h - World Info Transceiver Messages
 *
 *  Created: Wed May 02 10:25:56 2007
 *  Copyright  2006-2007  Tim Niemueller [www.niemueller.de]
 *
 *  $Id$
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version. A runtime exception applies to
 *  this software (see LICENSE file mentioned below for details).
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You can read the full text in the LICENSE file in the doc directory. 
 */

#ifndef __NETCOMM_WORLDINFO_MESSAGES_H_
#define __NETCOMM_WORLDINFO_MESSAGES_H_

#include <netcomm/worldinfo/defs.h>
#include <stdint.h>

/** Robot pose message type ID */
#define WORLDINFO_MSGTYPE_POSE           1
/** Robot velocity message type ID */
#define WORLDINFO_MSGTYPE_VELO           2
/** Relative ball position message type ID */
#define WORLDINFO_MSGTYPE_RELBALL        3
/** Relative ball velocity message type ID */
#define WORLDINFO_MSGTYPE_RELBALLVELO    4
/** Opponent pose message type ID */
#define WORLDINFO_MSGTYPE_OPP_POSE       5
/** Fat message containing all the information */
#define WORLDINFO_MSGTYPE_FAT_WORLDINFO  6


/** Per-message header.
 * In the sent buffer each message is preceeded by a per-message header which
 * contains the message type ID and the message length.
 */
typedef struct {
  uint16_t type;	/**< message type in network byte order */
  uint16_t size;	/**< message size in network byte order */
} worldinfo_message_header_t;


/** World info header.
 * Every message conglomerate (each packet) is prefixed by this general
 * header. It is used to determine if decryption has been successful, if protocol
 * versions are compatible and if the received sequence number is valid to prevent
 * replay attacks or problems with packets received out-of-order.
 */
typedef struct {
  uint16_t beef;	/**< has to contain 0xBEEF in network byte order */
  uint8_t  version;	/**< version of the following content */
  uint8_t  reserved;	/**< reserved for future use */
  uint32_t seq;		/**< sequence number in network byte order */
} worldinfo_header_t;


/** Robot pose message.
 * This message ought to be send by a robot to distribute its belief about its
 * position. This is NOT meant to be send with information about positions of
 * any other object or robot. There is a strong binding between the sender and
 * the object which this data describes.
 */
typedef struct {
  float x;	/**< X coordinate */
  float y;	/**< Y coordinate */
  float theta;	/**< orientation */
  float covariance[WORLDINFO_COVARIANCE_SIZE_3X3];	/**< position covariance matrix */
} worldinfo_pose_message_t;


/** Robot velocity message.
 * This message ought to be send by a robot to distribute its belief about its
 * velocity. This is NOT meant to be send with information about velocities of
 * any other object or robot. There is a strong binding between the sender and
 * the object which this data describes.
 */
typedef struct {
  float vel_x;	/**< Velocity in X direction */
  float vel_y;	/**< Velocity in Y direction */
  float vel_theta;	/**< Rotational velocity */
  float covariance[WORLDINFO_COVARIANCE_SIZE_3X3];	/**< velocity covariance matrix */
} worldinfo_velocity_message_t;


/** Relative ball position message.
 * This message describes a robots belief about the position of a ball relative
 * to itself.
 * This is NOT meant to be send with information about any other object but the ball.
 * There is a strong binding between the sender and the object which this data
 * describes which means that with this message a robot may not distribute another
 * robots belief of a ball position.
 */
typedef struct {
  float dist;	/**< distance to the robot */
  float pitch;	/**< pitch to the ball, this is the angle between the robots center position
		 * on the ground plane and the ball */
  float yaw;	/**< yaw to the ball, this is the angle between the robots forward direction
		 * and the ball on the ground plane */
  float covariance[WORLDINFO_COVARIANCE_SIZE_3X3];	/**< ball covariance matrix */
} worldinfo_relballpos_message_t;


/** Relative ball velocity message.
 * This message describes a robots belief about the velocity of a ball relative
 * to itself.
 * This is NOT meant to be send with information about any other object but the ball.
 * There is a strong binding between the sender and the object which this data
 * describes which means that with this message a robot may not distribute another
 * robots belief of a ball position.
 */
typedef struct {
  float vel_x;	/**< relative velocity of the ball in x direction */
  float vel_y;	/**< relative velocity of the ball in y direction */
  float vel_z;	/**< relative velocity of the ball in z direction */
  float covariance[WORLDINFO_COVARIANCE_SIZE_3X3];	/**< ball velocity covariance matrix */
} worldinfo_relballvelo_message_t;


/** Opponent message.
 * This message should be sent for every opponent that a robot detects. The position
 * is given in robot-relative polar coordinates on the ground plane.
 * This is NOT meant to be send with information about any other object but an opponent.
 * There is a strong binding between the sender and the object which this data
 * describes which means that with this message a robot may not distribute another
 * robots belief of an opponent position.
 */
typedef struct {
  float dist;	/**< distance to the opponent. */
  float angle;	/**< angle to the opponent */
  float covariance[WORLDINFO_COVARIANCE_SIZE_2X2];	/**< opponent position covariance matrix */
} worldinfo_opppose_message_t;


/** Fat worldinfo message.
 * Legacy adapter message to communicate with TU Graz team.
 */
typedef struct {
  uint32_t                        valid_pose          :  1;	/**< 1 if pose is valid, 0 otherwise*/
  uint32_t                        valid_velo          :  1;	/**< 1 if velo is valid, 0 otherwise*/
  uint32_t                        valid_relball_pos   :  1;	/**< 1 if relball_pos is valid, 0 otherwise*/
  uint32_t                        valid_relball_velo  :  1;	/**< 1 if relball_velo is valid, 0 otherwise*/
  uint32_t                        num_opponents       :  8;	/**< number of opponents with valid data in opponents */
  uint32_t                        reserved            : 20;	/**< reserved for future use */
  worldinfo_pose_message_t        pose;				/**< sending robot's pose */
  worldinfo_velocity_message_t    velo;				/**< sending robot's velocity */
  worldinfo_relballpos_message_t  relball_pos;			/**< ball position relative to sending robot */
  worldinfo_relballvelo_message_t relball_velo;			/**< ball velocity relative to sending robot */
  worldinfo_opppose_message_t     opponents[WORLDINFO_FATMSG_NUMOPPS];	/**< best seen opponents */
} worldinfo_fat_message_t;

#endif
