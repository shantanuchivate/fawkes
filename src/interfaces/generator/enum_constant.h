 
/***************************************************************************
 *  enum_constant.h - Interface generator enum constant representation
 *
 *  Generated: Wed Oct 11 19:40:37 2006
 *  Copyright  2006  Tim Niemueller [www.niemueller.de]
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

#ifndef __INTERFACES_GENERATOR_ENUM_CONSTANT_H_
#define __INTERFACES_GENERATOR_ENUM_CONSTANT_H_

#include <string>
#include <vector>
#include <utility>

class InterfaceEnumConstant
{
 public:
  InterfaceEnumConstant(const std::string &name, const std::string &comment);

  std::string getName();
  std::string getComment();
  std::vector< std::pair< std::string, std::string > > getItems();
  void addItem(std::string name, std::string comment);

 private:
  std::string name;
  std::string comment;
  std::vector< std::pair<std::string, std::string> > items;
};

#endif
