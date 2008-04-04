
/***************************************************************************
 *  dilation.h - header for morphological dilation filter
 *
 *  Created: Thu May 25 15:29:05 2006
 *  Copyright  2005-2006  Tim Niemueller [www.niemueller.de]
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

#ifndef __FIREVISION_FILTER_MORPHOLOGY_DILATION_H_
#define __FIREVISION_FILTER_MORPHOLOGY_DILATION_H_

#include <filters/morphology/morphologicalfilter.h>

class FilterDilation : public MorphologicalFilter
{
 public:
  FilterDilation();
  FilterDilation(unsigned char *se, unsigned int se_width, unsigned int se_height,
		 unsigned int se_anchor_x, unsigned int se_anchor_y);

  virtual void apply();

};

#endif
