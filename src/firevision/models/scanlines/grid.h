
/***************************************************************************
 *  grid.h - Scanline model implementation: grid
 *
 *  Generated: Sun May 08 21:54:49 2005
 *  Copyright  2005  Tim Niemueller [www.niemueller.de]
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

#ifndef __FIREVISION_SCANLINE_GRID_H_
#define __FIREVISION_SCANLINE_GRID_H_

#include "models/scanlines/scanlinemodel.h"
#include <fvutils/base/types.h>

class ScanlineGrid : public ScanlineModel
{

 public:

  ScanlineGrid(unsigned int width, unsigned int height,
	       unsigned int offset_x, unsigned int offset_y);

  point_t  operator*();
  point_t* operator->();
  point_t* operator++();
  point_t* operator++(int); 

  bool          finished();
  void          reset();
  const char *  getName();
  unsigned int  getMargin();

  virtual void  setRobotPose(float x, float y, float ori);
  virtual void  setPanTilt(float pan, float tilt);

  void setDimensions(unsigned int width, unsigned int height);
  void setOffset(unsigned int offset_x, unsigned int offset_y);
  void setGridParams(unsigned int width, unsigned int height,
		     unsigned int offset_x, unsigned int offset_y);

 private:
  unsigned int width;
  unsigned int height;
  unsigned int offset_x;
  unsigned int offset_y;

  point_t coord;
  point_t tmp_coord;

};

#endif
