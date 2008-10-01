
/***************************************************************************
 *  service_model.h - Manages list of discovered services of given type
 *
 *  Created: Mon Sep 29 16:26:04 2008
 *  Copyright  2008  Daniel Beck
 *
 *  $Id$
 *
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#ifndef __LIBS_GUI_UTILS_SERVICE_MODEL_H_
#define __LIBS_GUI_UTILS_SERVICE_MODEL_H_

#include <netcomm/service_discovery/browse_handler.h>
#include <core/utils/lock_queue.h>
#include <gtkmm.h>

namespace fawkes {
class AvahiThread;

class ServiceView
: public Gtk::TreeModel,
  public fawkes::ServiceBrowseHandler
{
 public:
  ServiceView(const char* service = "_fawkes._tcp");
  ServiceView(fawkes::AvahiThread* avahi_thread);
  virtual ~ServiceView();
  
 protected:
  
  class ServiceRecord : public Gtk::TreeModelColumnRecord
    {
    public:
      ServiceRecord()
	{
	  add(name);
	  add(type);
	  add(domain);
	  add(hostname);
	  add(port);
	}
      
      Gtk::TreeModelColumn<Glib::ustring> name;      /**< The name of the service */
      Gtk::TreeModelColumn<Glib::ustring> type;      /**< The type of the service */
      Gtk::TreeModelColumn<Glib::ustring> domain;    /**< The domain of the service */
      Gtk::TreeModelColumn<Glib::ustring> hostname;  /**< The name of the host the service is running on */
      Gtk::TreeModelColumn<unsigned short> port;     /**< The port the service is running on */
    };

  virtual void initialize() =0;
  
  // service browser handler
  void all_for_now();
  void cache_exhausted();
  void browse_failed( const char* name,
		      const char* type,
		      const char* domain );
  void service_added( const char* name,
		      const char* type,
		      const char* domain,
		      const char* host_name,
		      const struct sockaddr* addr,
		      const socklen_t addr_size,
		      uint16_t port,
		      std::list<std::string>& txt,
		      int flags );
  void service_removed( const char* name,
			const char* type,
			const char* domain );
  
  Glib::RefPtr<Gtk::ListStore> m_service_list;
  ServiceRecord m_service_record;
  
  fawkes::AvahiThread* m_avahi;
  
  Glib::Dispatcher m_signal_service_added;
  Glib::Dispatcher m_signal_service_removed;
  
  virtual void on_service_added();
  virtual void on_service_removed();

  struct ServiceAddedRecord
  {
    std::string name;      /**< the name of the new service */
    std::string type;      /**< the type of the new service */
    std::string domain;    /**< the domain of the new service */
    std::string hostname;  /**< the hostname of the new service */
    unsigned short port;   /**< the port the new serice is running on */
  };

  struct ServiceRemovedRecord
  {
    std::string name;    /**< the name of the service */
    std::string type;    /**< the type of the service */
    std::string domain;  /**< the domain of the service */
  };
      
  fawkes::LockQueue<ServiceAddedRecord> m_added_services;
  fawkes::LockQueue<ServiceRemovedRecord> m_removed_services;
  
 private:
  bool m_own_avahi_thread;
};
 
}

#endif /* __LIBS_GUI_UTILS_HOST_MODEL_H_ */
