
/***************************************************************************
 *  segment.h - Header of segmentation filter
 *
 *  Created: Mon Jun 27 11:36:25 2005
 *  Copyright  2005-2007  Tim Niemueller [www.niemueller.de]
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

#ifndef __FIREVISION_FILTER_SEGMENT_H_
#define __FIREVISION_FILTER_SEGMENT_H_

#include <filters/filter.h>
#include <fvutils/base/types.h>

class ColorModel;

class FilterSegment : public Filter
{
 public:
  FilterSegment(ColorModel *cm, color_t what);

  virtual void apply();

 private:
  ColorModel    *cm;
  color_t        what;
};

#endif
