# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330,
# Boston, MA  02111-1307  USA

import locale
import gettext
import os
import sys
import time
import gtk
import gobject
import pango
import ibus
import keyboardshortcut
from os import path
from xdg import BaseDirectory
from gtk import gdk
from gtk import glade
from enginecombobox import EngineComboBox
from enginetreeview import EngineTreeView
from icon import load_icon

_  = lambda a : gettext.dgettext("ibus", a)
N_ = lambda a : a

(
    COLUMN_NAME,
    COLUMN_ENABLE,
    COLUMN_PRELOAD,
    COLUMN_VISIBLE,
    COLUMN_ICON,
    COLUMN_DATA,
) = range(6)

(
    DATA_NAME,
    DATA_LOCAL_NAME,
    DATA_LANG,
    DATA_ICON,
    DATA_AUTHOR,
    DATA_CREDITS,
    DATA_EXEC,
    DATA_STARTED,
    DATA_PRELOAD
) = range(9)

class Setup(object):
    def __flush_gtk_events(self):
        while gtk.events_pending():
            gtk.main_iteration()

    def __init__(self):
        super(Setup, self).__init__()
        locale.bind_textdomain_codeset("ibus", "UTF-8")
        glade.textdomain("ibus")
        glade_file = path.join(path.dirname(__file__), "./setup.glade")
        self.__xml = glade.XML(glade_file)
        self.__bus = None
        self.__init_bus()
        self.__init_ui()

    def __init_ui(self):
        # add icon search path
        self.__dialog = self.__xml.get_widget("dialog_setup")

        # auto start ibus
        self.__checkbutton_auto_start = self.__xml.get_widget("checkbutton_auto_start")
        self.__checkbutton_auto_start.set_active(self.__is_auto_start())
        self.__checkbutton_auto_start.connect("toggled", self.__checkbutton_auto_start_toggled_cb)

        # keyboard shortcut
        # trigger
        self.__config = self.__bus.get_config()
        shortcuts = self.__config.get_value(
                        "general/hotkey", "trigger",
                        ibus.CONFIG_GENERAL_SHORTCUT_TRIGGER_DEFAULT)

        button = self.__xml.get_widget("button_trigger")
        entry = self.__xml.get_widget("entry_trigger")
        entry.set_text("; ".join(shortcuts))
        button.connect("clicked", self.__shortcut_button_clicked_cb,
                    N_("trigger"), "general/hotkey", "trigger", entry)

        # next engine
        shortcuts = self.__config.get_value(
                        "general/hotkey", "next_engine",
                        ibus.CONFIG_GENERAL_SHORTCUT_NEXT_ENGINE_DEFAULT)
        button = self.__xml.get_widget("button_next_engine")
        entry = self.__xml.get_widget("entry_next_engine")
        entry.set_text("; ".join(shortcuts))
        button.connect("clicked", self.__shortcut_button_clicked_cb,
                    N_("next engine"), "general/hotkey", "next_engine", entry)

        # prev engine
        shortcuts = self.__config.get_value(
                        "general/hotkey", "prev_engine",
                        ibus.CONFIG_GENERAL_SHORTCUT_PREV_ENGINE_DEFAULT)
        button = self.__xml.get_widget("button_prev_engine")
        entry = self.__xml.get_widget("entry_prev_engine")
        entry.set_text("; ".join(shortcuts))
        button.connect("clicked", self.__shortcut_button_clicked_cb,
                    N_("prev engine"), "general/hotkey", "prev_engine", entry)

        # lookup table orientation
        self.__combobox_lookup_table_orientation = self.__xml.get_widget("combobox_lookup_table_orientation")
        self.__combobox_lookup_table_orientation.set_active(
            self.__config.get_value("panel", "lookup_table_orientation", 0))
        self.__combobox_lookup_table_orientation.connect("changed",
            self.__combobox_lookup_table_orientation_changed_cb)

        # auto hide
        self.__combobox_panel_show = self.__xml.get_widget("combobox_panel_show")
        self.__combobox_panel_show.set_active(
            self.__config.get_value("panel", "show", 1))
        self.__combobox_panel_show.connect("changed", self.__combobox_panel_show_changed_cb)

        # custom font
        self.__checkbutton_custom_font = self.__xml.get_widget("checkbutton_custom_font")
        self.__checkbutton_custom_font.set_active(
            self.__config.get_value("panel", "use_custom_font", False))
        self.__checkbutton_custom_font.connect("toggled", self.__checkbutton_custom_font_toggled_cb)

        self.__fontbutton_custom_font = self.__xml.get_widget("fontbutton_custom_font")
        if self.__config.get_value("panel", "use_custom_font", False):
            self.__fontbutton_custom_font.set_sensitive(True)
        else:
            self.__fontbutton_custom_font.set_sensitive(False)
        font_name = gtk.settings_get_default().get_property("gtk-font-name")
        font_name = unicode(font_name, "utf-8")
        font_name = self.__config.get_value("panel", "custom_font", font_name)
        self.__fontbutton_custom_font.connect("notify::font-name", self.__fontbutton_custom_font_notify_cb)
        self.__fontbutton_custom_font.set_font_name(font_name)

        # init engine page
        self.__engines = self.__bus.list_engines()
        self.__combobox = EngineComboBox(self.__engines)
        self.__combobox.show()
        self.__xml.get_widget("alignment_engine_combobox").add(self.__combobox)

        tmp_dict = {}
        for e in self.__engines:
            tmp_dict[e.name] = e
        engine_names = self.__config.get_value("general", "preload_engines", [])
        engines = []
        for n in engine_names:
            if n in tmp_dict:
                engines.append(tmp_dict[n])
        self.__treeview = EngineTreeView(engines)
        self.__treeview.show()
        self.__xml.get_widget("scrolledwindow_engine_treeview").add(self.__treeview)

        self.__treeview.connect("changed", self.__treeview_changed_cb)

        button = self.__xml.get_widget("button_engine_add")
        button.connect("clicked",
                       lambda *args:self.__treeview.prepend_engine(self.__combobox.get_active_engine()))
        button = self.__xml.get_widget("button_engine_remove")
        button.connect("clicked", lambda *args:self.__treeview.remove_engine())
        button = self.__xml.get_widget("button_engine_up")
        button.connect("clicked", lambda *args:self.__treeview.move_up_engine())

        button = self.__xml.get_widget("button_engine_down")
        button.connect("clicked", lambda *args:self.__treeview.move_down_engine())

    def __treeview_changed_cb(self, treeview):
        engines = self.__treeview.get_engines()
        engine_names = map(lambda e: e.name, engines)
        self.__config.set_list("general", "preload_engines", engine_names, "s")

    def __init_bus(self):
        try:
            self.__bus = ibus.Bus()
            # self.__bus.connect("config-value-changed", self.__config_value_changed_cb)
            # self.__bus.connect("config-reloaded", self.__config_reloaded_cb)
            # self.__bus.config_add_watch("/general")
            # self.__bus.config_add_watch("/general/hotkey")
            # self.__bus.config_add_watch("/panel")
        except:
            while self.__bus == None:
                message = _("IBus daemon is not started. Do you want to start it now?")
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_QUESTION,
                        buttons = gtk.BUTTONS_YES_NO,
                        message_format = message)
                id = dlg.run()
                dlg.destroy()
                self.__flush_gtk_events()
                if id != gtk.RESPONSE_YES:
                    sys.exit(0)
                pid = os.spawnlp(os.P_NOWAIT, "ibus-daemon", "ibus-daemon")
                time.sleep(1)
                try:
                    self.__bus = ibus.Bus()
                except:
                    continue
                message = _("IBus has been started! "
                    "If you can not use IBus, please add below lines in $HOME/.bashrc, and relogin your desktop.\n"
                    "  export GTK_IM_MODULE=ibus\n"
                    "  export XMODIFIERS=@im=ibus\n"
                    "  export QT_IM_MODULE=ibus"
                    )
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_INFO,
                                        buttons = gtk.BUTTONS_OK,
                                        message_format = message)
                id = dlg.run()
                dlg.destroy()
                self.__flush_gtk_events()

    def __shortcut_button_clicked_cb(self, button, name, section, _name, entry):
        buttons = (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OK, gtk.RESPONSE_OK)
        title = _("Select keyboard shortcut for %s") %  _(name)
        dialog = keyboardshortcut.KeyboardShortcutSelectionDialog(buttons = buttons, title = title)
        text = entry.get_text()
        if text:
            shortcuts = text.split("; ")
        else:
            shortcuts = None
        dialog.set_shortcuts(shortcuts)
        id = dialog.run()
        shortcuts = list(set(dialog.get_shortcuts()))
        dialog.destroy()
        if id != gtk.RESPONSE_OK:
            return
        self.__config.set_value(section, _name, shortcuts)
        entry.set_text("; ".join(shortcuts))


    def __item_started_column_toggled_cb(self, cell, path_str, model):

        # get toggled iter
        iter = model.get_iter_from_string(path_str)
        data = model.get_value(iter, COLUMN_DATA)

        # do something with the value
        if data[DATA_STARTED] == False:
            try:
                self.__bus.register_start_engine(data[DATA_LANG], data[DATA_NAME])
            except Exception, e:
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_ERROR,
                        buttons = gtk.BUTTONS_CLOSE,
                        message_format = str(e))
                dlg.run()
                dlg.destroy()
                self.__flush_gtk_events()
                return
        else:
            try:
                self.__bus.register_stop_engine(data[DATA_LANG], data[DATA_NAME])
            except Exception, e:
                dlg = gtk.MessageDialog(type = gtk.MESSAGE_ERROR,
                        buttons = gtk.BUTTONS_CLOSE,
                        message_format = str(e))
                dlg.run()
                dlg.destroy()
                self.__flush_gtk_events()
                return
        data[DATA_STARTED] = not data[DATA_STARTED]

        # set new value
        model.set(iter, COLUMN_ENABLE, data[DATA_STARTED])

    def __item_preload_column_toggled_cb(self, cell, path_str, model):

        # get toggled iter
        iter = model.get_iter_from_string(path_str)
        data = model.get_value(iter, COLUMN_DATA)

        data[DATA_PRELOAD] = not data[DATA_PRELOAD]
        engine = "%s:%s" % (data[DATA_LANG], data[DATA_NAME])

        if data[DATA_PRELOAD]:
            if engine not in self.__preload_engines:
                self.__preload_engines.add(engine)
                self.__config.set_list("general", "preload_engines", list(self.__preload_engines), "s")
        else:
            if engine in self.__preload_engines:
                self.__preload_engines.remove(engine)
                self.__config.set_list("general", "preload_engines", list(self.__preload_engines), "s")

        # set new value
        model.set(iter, COLUMN_PRELOAD, data[DATA_PRELOAD])

    def __is_auto_start(self):
        link_file = path.join(BaseDirectory.xdg_config_home, "autostart/ibus.desktop")
        ibus_desktop = path.join(os.getenv("IBUS_PREFIX"), "share/applications/ibus.desktop")

        if not path.exists(link_file):
            return False
        if not path.islink(link_file):
            return False
        if path.realpath(link_file) != ibus_desktop:
            return False
        return True

    def __checkbutton_auto_start_toggled_cb(self, button):
        auto_start_dir = path.join(BaseDirectory.xdg_config_home, "autostart")
        if not path.isdir(auto_start_dir):
            os.makedirs(auto_start_dir)

        link_file = path.join(BaseDirectory.xdg_config_home, "autostart/ibus.desktop")
        ibus_desktop = path.join(os.getenv("IBUS_PREFIX"), "share/applications/ibus.desktop")
        # unlink file
        try:
            os.unlink(link_file)
        except:
            pass
        if self.__checkbutton_auto_start.get_active():
            os.symlink(ibus_desktop, link_file)

    def __combobox_lookup_table_orientation_changed_cb(self, combobox):
        self.__config.set_value(
            "panel", "lookup_table_orientation",
            self.__combobox_lookup_table_orientation.get_active())

    def __combobox_panel_show_changed_cb(self, combobox):
        self.__config.set_value(
            "panel", "show",
            self.__combobox_panel_show.get_active())

    def __checkbutton_custom_font_toggled_cb(self, button):
        if self.__checkbutton_custom_font.get_active():
            self.__fontbutton_custom_font.set_sensitive(True)
            self.__config.set_value("panel", "use_custom_font", True)
        else:
            self.__fontbutton_custom_font.set_sensitive(False)
            self.__config.set_value("panel", "use_custom_font", False)

    def __fontbutton_custom_font_notify_cb(self, button, arg):
        font_name = self.__fontbutton_custom_font.get_font_name()
        font_name = unicode(font_name, "utf-8")
        self.__config.set_value("panel", "custom_font", font_name)

    def __config_value_changed_cb(self, bus, section, name, value):
        pass

    def __config_reloaded_cb(self, bus):
        pass

    def run(self):
        return self.__dialog.run()

if __name__ == "__main__":
    Setup().run()
