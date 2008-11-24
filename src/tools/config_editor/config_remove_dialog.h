
/***************************************************************************
 *  config_remove_dialog.h - Remove config entries
 *
 *  Created: Thu Sep 25 18:49:58 2008
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

#ifndef __TOOLS_CONFIG_EDITOR_CONFIG_REMOVE_DIALOG_H_
#define __TOOLS_CONFIG_EDITOR_CONFIG_REMOVE_DIALOG_H_

#include <gtkmm.h>
#include <libglademm/xml.h>

class ConfigRemoveDialog : public Gtk::Dialog
{
 public:
  ConfigRemoveDialog( BaseObjectType* cobject,
		      const Glib::RefPtr<Gnome::Glade::Xml>& ref_xml );
  virtual ~ConfigRemoveDialog();

  void init(const Glib::ustring& path, bool is_default);
  bool get_remove_default() const;

 protected:
  Gtk::Label* m_lbl_path;
  Gtk::CheckButton* m_chb_is_default;
};

#endif /* __TOOLS_CONFIG_EDITOR_CONFIG_REMOVE_DIALOG_H_ */