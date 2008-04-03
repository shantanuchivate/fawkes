
/***************************************************************************
 *  png.h - PNG Reader
 *
 *  Created: Thu Apr 03 12:55:36 2008
 *  Copyright  2005-2008  Tim Niemueller [www.niemueller.de]
 *
 *  $Id$
 *
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __FIREVISION_UTILS_READERS_PNG_H_
#define __FIREVISION_UTILS_READERS_PNG_H_

#include <fvutils/readers/reader.h>

class PNGReaderData;

class PNGReader : public Reader {

 public:
  PNGReader(const char *filename);
  virtual ~PNGReader();

  virtual void             set_buffer(unsigned char *yuv422planar_buffer);
  virtual colorspace_t     colorspace();
  virtual unsigned int     pixel_width();
  virtual unsigned int     pixel_height();
  virtual void             read();

 private:
  PNGReaderData *  setup_read(const char *filename);

  PNGReaderData *__d;
  bool opened;
  unsigned char *buffer;
};


#endif