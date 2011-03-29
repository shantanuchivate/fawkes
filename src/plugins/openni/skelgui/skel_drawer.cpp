
/***************************************************************************
 *  skel_drawer.cpp - Skeleton Visualization GUI: skeleton drawer
 *
 *  Created: Wed Mar 02 11:36:43 2011
 *  Copyright  2006-2011  Tim Niemueller [www.niemueller.de]
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

#include "skel_drawer.h"
#include "colors.h"

#include <cstring>
#include <cstdio>
#include <GL/glut.h>

using namespace fawkes;

bool g_print_id = true;
bool g_print_state = true;

void
glPrintString(void *font, char *str)
{
  int i,l = strlen(str);
  for(i=0; i<l; i++) {
    glutBitmapCharacter(font,*str++);
  }
}

void
draw_limb(float *proj1, float conf1, float *proj2, float conf2)
{
  if (conf1 < 0.5 || conf2 < 0.5)  return;

  //GLint x1 = proj1[0], y1 = proj1[1], x2 = proj2[0], y2 = proj2[1];
  //printf("Drawing from (%i,%i) to (%i,%i)\n", x1, y1, x2, y2);

  glVertex3i(proj1[0], proj1[1], 0);
  glVertex3i(proj2[0], proj2[1], 0);
}

#define DRAW_LIMB(user, joint1, joint2)					\
  draw_limb(user.proj_if->proj_##joint1(),				\
            user.skel_if->pos_##joint1##_confidence(),			\
	    user.proj_if->proj_##joint2(),				\
	    user.skel_if->pos_##joint2##_confidence());

void
draw_user(UserInfo &user)
{
  if (user.skel_if->state() != HumanSkeletonInterface::STATE_TRACKING)  return;

  DRAW_LIMB(user, head, neck);
  
  DRAW_LIMB(user, neck, left_shoulder);
  DRAW_LIMB(user, left_shoulder, left_elbow);
  DRAW_LIMB(user, left_elbow, left_hand);
  
  DRAW_LIMB(user, neck, right_shoulder);
  DRAW_LIMB(user, right_shoulder, right_elbow);
  DRAW_LIMB(user, right_elbow, right_hand);

  DRAW_LIMB(user, left_shoulder, torso);
  DRAW_LIMB(user, right_shoulder, torso);

  DRAW_LIMB(user, torso, left_hip);
  DRAW_LIMB(user, left_hip, left_knee);
  DRAW_LIMB(user, left_knee, left_foot);

  DRAW_LIMB(user, torso, right_hip);
  DRAW_LIMB(user, right_hip, right_knee);
  DRAW_LIMB(user, right_knee, right_foot);

  DRAW_LIMB(user, left_hip, right_hip);

}

void
draw_skeletons(UserMap &users, unsigned int x_res, unsigned int y_res)
{
  if (users.empty()) return;

  char label[50] = "";
  for (UserMap::iterator i = users.begin(); i != users.end(); ++i) {
    if (g_print_id) {
      memset(label, 0, sizeof(label));
      if (!g_print_state) {
	sprintf(label, "%s", i->first.c_str());
      }
      else if (i->second.skel_if->state() == HumanSkeletonInterface::STATE_TRACKING)
      {
	sprintf(label, "%s - Tracking", i->first.c_str());
      } else if (i->second.skel_if->state() == HumanSkeletonInterface::STATE_CALIBRATING)
      {
	sprintf(label, "%s - Calibrating...", i->first.c_str());
      } else {
	sprintf(label, "%s - Looking for pose", i->first.c_str());
      }
      
      glColor4f(1 - USER_COLORS[i->second.skel_if->user_id() % NUM_USER_COLORS][0],
		1 - USER_COLORS[i->second.skel_if->user_id() % NUM_USER_COLORS][1],
		1 - USER_COLORS[i->second.skel_if->user_id() % NUM_USER_COLORS][2],
		1);
      
      glRasterPos2i(i->second.proj_if->proj_com(0), i->second.proj_if->proj_com(1));
      glPrintString(GLUT_BITMAP_HELVETICA_18, label);
    }

    glBegin(GL_LINES);
    glColor4f(1 - USER_COLORS[i->second.skel_if->user_id() % NUM_USER_COLORS][0],
	      1 - USER_COLORS[i->second.skel_if->user_id() % NUM_USER_COLORS][1],
	      1 - USER_COLORS[i->second.skel_if->user_id() % NUM_USER_COLORS][2],
	      1);

    draw_user(i->second);
    glEnd();
  }
}
