
/***************************************************************************
 *  message_queue.cpp - Fawkes network message queue
 *
 *  Created: Mon Nov 20 18:21:39 2006
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

#include <netcomm/fawkes/message_queue.h>

/** @class FawkesNetworkMessageQueue netcomm/fawkes/message_queue.h
 * A LockQueue of FawkesNetworkMessage to hold messages in inbound and
 * outbound queues.
 *
 * @ingroup NetComm
 * @author Tim Niemueller
 */

/** Constructor. */
FawkesNetworkMessageQueue::FawkesNetworkMessageQueue()
{
}


/** Destructor. */
FawkesNetworkMessageQueue::~FawkesNetworkMessageQueue()
{
}
