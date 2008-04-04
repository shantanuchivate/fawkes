
/***************************************************************************
 *  service_browser.h - browse services
 *
 *  Created: Wed Nov 08 13:05:34 2006
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

#ifndef __NETCOMM_SERVICE_DISCOVERY_SERVICE_BROWSER_H_
#define __NETCOMM_SERVICE_DISCOVERY_SERVICE_BROWSER_H_

#include <netcomm/service_discovery/service.h>
#include <netcomm/service_discovery/browse_handler.h>

class ServiceBrowser
{
 public:
  virtual ~ServiceBrowser();

  virtual void watch_service(const char *service_type, ServiceBrowseHandler *h)   = 0;
  virtual void unwatch_service(const char *service_type, ServiceBrowseHandler *h) = 0;
};


#endif
