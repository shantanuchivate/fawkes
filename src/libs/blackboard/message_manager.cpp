
/***************************************************************************
 *  message_manager.cpp - BlackBoard message manager
 *
 *  Created: Fri Oct 06 11:36:24 2006
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

#include <blackboard/message_manager.h>
#include <blackboard/interface_manager.h>
#include <blackboard/notifier.h>
#include <blackboard/exceptions.h>

#include <interface/message.h>
#include <interface/interface.h>

#include <core/exceptions/software.h>
#include <utils/logging/liblogger.h>


/** @class BlackBoardMessageManager <blackboard/message_manager.h>
 * BlackBoard message manager.
 * Transmits messages from reading interface instances to the writer instance
 * if the interface, if there is any.
 * @author Tim Niemueller
 */

/** Constructor.
 * @param notifier BlackBoard notifier to all for events
 */
BlackBoardMessageManager::BlackBoardMessageManager(BlackBoardNotifier *notifier)
{
  __im = NULL;
  __notifier = notifier;
}


/** Destructor */
BlackBoardMessageManager::~BlackBoardMessageManager()
{
}


unsigned int
BlackBoardMessageManager::transmit(Message *message)
{
  if ( __im == NULL ) {
    throw NullPointerException("InterfaceManager has not been set for MessageManager");
  }
  try {
    Interface *writer = __im->writer_for_mem_serial(message->recipient());
    if ( __notifier->notify_of_message_received(writer, message) ) {
      return writer->msgq_append(message);
    } else {
      return 0;
    }
  } catch (BlackBoardNoWritingInstanceException &e) {
    Interface *iface = message->interface();
    LibLogger::log_warn("BlackBoardMessageManager", "Cannot transmit message from sender %s "
			                            "via interface %s (type %s), no writing "
			                            "instance exists!",
			message->sender_thread_name(),
			(iface != NULL) ? iface->id() : "Unknown",
			(iface != NULL) ? iface->type() : "unknown");
    throw;
  }
}


/** Set interface manager.
 * @param im interface manager
 */
void
BlackBoardMessageManager::set_interface_manager(BlackBoardInterfaceManager *im)
{
  __im = im;
}
