 
/***************************************************************************
 *  interface_listener.cpp - BlackBoard event listener
 *
 *  Created: Wed Nov 08 10:00:34 2007
 *  Copyright  2007  Tim Niemueller [www.niemueller.de]
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
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307, USA.
 */

#include <blackboard/interface_listener.h>
#include <interface/interface.h>

/** @class BlackBoardInterfaceListener <blackboard/interface_listener.h>
 * BlackBoard interface listener.
 * Derive this class if you want to be notified of specific BlackBoard 
 * events regarding instances of interfaces.
 *
 * The bb_interface_* methods are called during the appropriate operation. The
 * operation that you carry out in this event handler really has to damn fast, or
 * the performance of the whole system will suffer severely. For this reason use
 * this notification facility only rarely and only register for the appropriate
 * events.
 *
 * This class provides the basic infrastructure that can be used to build
 * your own event handler. During the life time of your event handler your
 * first add all the interfaces to the appropriate structures that you want
 * to listen for and add the interface types where you want to be notified
 * of creation events.
 *
 * The reader/writer added/removed and data changed notifications act upon a
 * specific interface. Any modification done with any instance of the interface
 * is reported to you. The interface creation notification deals only
 * with types of interfaces. There is no interface deletion notification because
 * the general idea is that you opened the interface by yourself for reading and
 * thus the deletion will not happen before you close the interface.
 *
 * Here is a simple life cycle of a BlackBoard interface listener:
 * First you create your interface that you want to listen for.
 * The protected methods bbil_add_data_interface(), bbil_add_reader_interface(),
 * bbil_add_writer_interface() and bbil_add_interface_create_type() have to
 * be called with the appropriate interfaces <i>before</i> the event handler is
 * actually registered with the interface manager! From
 * now on will be called for the all registered events.
 * In the end you unregister the event listener and <i>then</i> close any
 * interface that you had registered before.
 *
 * It is important that you first unregister as an event handler before closing
 * the interface. Otherwise it could happen that you close the interface and
 * the instance is deleted and afterwards an event for that very interface
 * happens. A warning is reported via the LibLogger whenever you forget this.
 *
 * @author Tim Niemueller
 * @see BlackBoardInterfaceManager::register_listener()
 * @see BlackBoardInterfaceManager::unregister_listener()
 */

/** Empty constructor. */
BlackBoardInterfaceListener::BlackBoardInterfaceListener()
{
}

/** Destructor. */
BlackBoardInterfaceListener::~BlackBoardInterfaceListener()
{
  char *tmp;

  while ( ! __bbil_data_interfaces.empty() ) {
    __bbil_ii = __bbil_data_interfaces.begin();
    tmp = (*__bbil_ii).first;
    __bbil_data_interfaces.erase(__bbil_ii);
    free(tmp);
  }

  while ( ! __bbil_reader_interfaces.empty() ) {
    __bbil_ii = __bbil_reader_interfaces.begin();
    tmp = (*__bbil_ii).first;
    __bbil_reader_interfaces.erase(__bbil_ii);
    free(tmp);
  }

  while ( ! __bbil_writer_interfaces.empty() ) {
    __bbil_ii = __bbil_writer_interfaces.begin();
    tmp = (*__bbil_ii).first;
    __bbil_writer_interfaces.erase(__bbil_ii);
    free(tmp);
  }
}


/** BlackBoard data changed notification.
 * This is called whenever the data in an interface that you registered for is
 * modified. This happens if a writer calls the Interface::write() method.
 * @param interface interface instance that you supplied to bbil_add_data_interface()
 */
void
BlackBoardInterfaceListener::bb_interface_data_changed(Interface *interface) throw()
{
}


/** A reading instance has been opened for a watched interface.
 * This is called whenever a reading instance of the interface you are watching
 * is opened.
 * @param interface interface instance that you supplied to bbil_add_reader_interface()
 */
void
BlackBoardInterfaceListener::bb_interface_reader_added(Interface *interface) throw()
{
}


/** A reading instance has been closed for a watched interface.
 * This is called whenever a reading instance of an interface you are watching
 * is closed.
 * @param interface interface instance that you supplied to bbil_add_reader_interface()
 */
void
BlackBoardInterfaceListener::bb_interface_reader_removed(Interface *interface) throw()
{
}


/** A writing instance has been opened for a watched interface.
 * This is called whenever a writing instance of the interface you are watching
 * is opened.
 * @param interface interface instance that you supplied to bbil_add_writer_interface()
 */
void
BlackBoardInterfaceListener::bb_interface_writer_added(Interface *interface) throw()
{
}


/** A writing instance has been closed for a watched interface.
 * This is called whenever a writing instance of an interface you are watching
 * is closed.
 * @param interface interface instance that you supplied to bbil_add_writer_interface()
 */
void
BlackBoardInterfaceListener::bb_interface_writer_removed(Interface *interface) throw()
{
}


/** Add an interface to the data modification watch list.
 * @param interface interface to watch for data modifications.
 */
void
BlackBoardInterfaceListener::bbil_add_data_interface(Interface *interface)
{
  if ( __bbil_data_interfaces.find((char *)interface->uid()) != __bbil_data_interfaces.end() ) {
    throw Exception("Interface %s already registered (data)", interface->uid());
  }
  __bbil_data_interfaces[strdup(interface->uid())] = interface;
}

/** Add an interface to the reader addition/removal watch list.
 * This method does not mean that you add interfaces that you opened for reading
 * but that you add an interface that you want to be informed for when reader
 * addition/removal happens.
 * @param interface interface to watch for addition/removal of readers
 */
void
BlackBoardInterfaceListener::bbil_add_reader_interface(Interface *interface)
{
  if ( __bbil_reader_interfaces.find((char *)interface->uid()) != __bbil_reader_interfaces.end() ) {
    throw Exception("Interface %s already registered (reader)", interface->uid());
  }
  __bbil_reader_interfaces[strdup(interface->uid())] = interface;
}


/** Add an interface to the writer addition/removal watch list.
 * This method does not mean that you add interfaces that you opened for writing
 * but that you add an interface that you want to be informed for when writer
 * addition/removal happens.
 * @param interface interface to watch for addition/removal of writers
 */
void
BlackBoardInterfaceListener::bbil_add_writer_interface(Interface *interface)
{
  if ( __bbil_writer_interfaces.find((char *)interface->uid()) != __bbil_writer_interfaces.end() ) {
    throw Exception("Interface %s already registered (writer)", interface->uid());
  }
  __bbil_writer_interfaces[strdup(interface->uid())] = interface;
}


/** Get data modification watch list.
 * @return data modification watch list
 */
BlackBoardInterfaceListener::InterfaceLockHashMap *
BlackBoardInterfaceListener::bbil_data_interfaces() throw()
{
  return &__bbil_data_interfaces;
}

/** Get reader watch list.
 * @return reader watch list
 */
BlackBoardInterfaceListener::InterfaceLockHashMap *
BlackBoardInterfaceListener::bbil_reader_interfaces() throw()
{
  return &__bbil_reader_interfaces;
}

/** Get writer watch list.
 * @return writer watch list
 */
BlackBoardInterfaceListener::InterfaceLockHashMap *
BlackBoardInterfaceListener::bbil_writer_interfaces() throw()
{
  return &__bbil_writer_interfaces;
}


/** Get interface instance for given UID.
 * A data modification notification is about to be triggered. For this the
 * interface instance that has been added to the event listener is determined.
 * @param iuid interface unique ID
 * @return interface instance, NULL if not in list (non-fatal error)
 */
Interface *
BlackBoardInterfaceListener::bbil_data_interface(const char *iuid) throw()
{
  __bbil_data_interfaces.lock();
  bool found = ((__bbil_ii = __bbil_data_interfaces.find((char *)iuid)) != __bbil_data_interfaces.end());
  __bbil_data_interfaces.unlock();
  if ( found ) {
    return (*__bbil_ii).second;
  } else {
    return NULL;
  }
}


/** Get interface instance for given UID.
 * A reader notification is about to be triggered. For this the
 * interface instance that has been added to the event listener is determined.
 * @param iuid interface unique ID
 * @return interface instance, NULL if not in list (non-fatal error)
 */
Interface *
BlackBoardInterfaceListener::bbil_reader_interface(const char *iuid) throw()
{
  __bbil_reader_interfaces.lock();
  bool found = ((__bbil_ii = __bbil_reader_interfaces.find((char *)iuid)) != __bbil_reader_interfaces.end());
  __bbil_reader_interfaces.unlock();
  if ( found ) {
    return (*__bbil_ii).second;
  } else {
    return NULL;
  }
}


/** Get interface instance for given UID.
 * A writer notification is about to be triggered. For this the
 * interface instance that has been added to the event listener is determined.
 * @param iuid interface unique ID
 * @return interface instance, NULL if not in list (non-fatal error)
 */
Interface *
BlackBoardInterfaceListener::bbil_writer_interface(const char *iuid) throw()
{
  __bbil_writer_interfaces.lock();
  bool found = ((__bbil_ii = __bbil_writer_interfaces.find((char *)iuid)) != __bbil_writer_interfaces.end());
  __bbil_writer_interfaces.unlock();
  if ( found ) {
    return (*__bbil_ii).second;
  } else {
    return NULL;
  }
}