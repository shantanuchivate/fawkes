
/***************************************************************************
 *  vision_master.h - Vision Master aspect for Fawkes
 *
 *  Created: Tue May 29 14:45:54 2007
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

#include <aspect/vision_master.h>

/** @class VisionMasterAspect aspect/vision_master.h
 * Vision Master Aspect.
 *
 * This aspect provides access to the vision muster. Your thread having
 * this aspect has to call the proper constructor that sets the vision
 * master.
 *
 * @ingroup Aspects
 * @author Tim Niemueller
 */


/** Constructor.
 * @param master vision master
 */
VisionMasterAspect::VisionMasterAspect(VisionMaster *master)
{
  this->master = master;
}


/** Virtual empty Destructor. */
VisionMasterAspect::~VisionMasterAspect()
{
}


/** Get vision master.
 * @return vision master
 */
VisionMaster *
VisionMasterAspect::vision_master()
{
  return master;
}
