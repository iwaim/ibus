# vim:set et sts=4 sw=4:
#
# ibus - The Input Bus
#
# Copyright (c) 2007-2009 Huang Peng <shawn.p.huang@gmail.com>
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

__all__ = (
    "KeyboardShortcutSelection",
    "KeyboardShortcutSelectionDialog",
);

import gobject
import gtk
from gtk import gdk

from gettext import dgettext
_  = lambda a : dgettext("ibus", a)
N_ = lambda a : a

class KeyboardShortcutSelection(gtk.VBox):
    def __init__(self, shortcuts = None):
        super(KeyboardShortcutSelection, self).__init__()
        self.__init_ui()
        self.set_shortcuts(shortcuts)

    def __init_ui(self):
        # label = gtk.Label(_("Keyboard shortcuts:"))
        # label.set_justify(gtk.JUSTIFY_LEFT)
        # label.set_alignment(0.0, 0.5)
        # self.pack_start(label, False, True, 4)

        # shortcuts view
        viewport =  gtk.Viewport()
        viewport.set_shadow_type(gtk.SHADOW_IN)
        self.__shortcut_view = gtk.TreeView(gtk.ListStore(gobject.TYPE_STRING))
        self.__shortcut_view.set_size_request(-1, 100)
        renderer = gtk.CellRendererText()
        column = gtk.TreeViewColumn(_("Keyboard shortcuts"), renderer, text = 0)
        self.__shortcut_view.append_column(column)
        self.__shortcut_view.connect("cursor-changed", self.__shortcut_view_cursor_changed_cb)
        viewport.add(self.__shortcut_view)
        self.pack_start(viewport, True, True, 4)

        # key code
        hbox = gtk.HBox()
        label = gtk.Label(_("Key code:"))
        label.set_justify(gtk.JUSTIFY_LEFT)
        label.set_alignment(0.0, 0.5)
        hbox.pack_start(label, False, True, 4)

        self.__keycode_entry = gtk.Entry()
        self.__keycode_entry.connect("notify::text", self.__keycode_entry_notify_cb)
        hbox.pack_start(self.__keycode_entry, True, True, 4)
        self.__keycode_button = gtk.Button("...")
        self.__keycode_button.connect("clicked", self.__keycode_button_clicked_cb)
        hbox.pack_start(self.__keycode_button, False, True, 4)
        self.pack_start(hbox, False, True, 4)

        # modifiers
        hbox = gtk.HBox()
        label = gtk.Label(_("Modifiers:"))
        label.set_justify(gtk.JUSTIFY_LEFT)
        label.set_alignment(0.0, 0.5)
        hbox.pack_start(label, False, True, 4)

        table = gtk.Table(4, 2)
        self.__modifier_buttons = []
        self.__modifier_buttons.append(("Control",     gtk.CheckButton("_Control"),       gdk.CONTROL_MASK))
        self.__modifier_buttons.append(("Alt",      gtk.CheckButton("A_lt"),        gdk.MOD1_MASK))
        self.__modifier_buttons.append(("Shift",    gtk.CheckButton("_Shift"),      gdk.SHIFT_MASK))
        self.__modifier_buttons.append(("Meta",     gtk.CheckButton("_Meta"),       gdk.META_MASK))
        self.__modifier_buttons.append(("Super",    gtk.CheckButton("S_uper"),      gdk.SUPER_MASK))
        self.__modifier_buttons.append(("Hyper",    gtk.CheckButton("_Hyper"),      gdk.HYPER_MASK))
        self.__modifier_buttons.append(("Capslock", gtk.CheckButton("Capsloc_k"),   gdk.LOCK_MASK))
        self.__modifier_buttons.append(("Release",  gtk.CheckButton("_Release"),    gdk.RELEASE_MASK))
        for name, button, mask in self.__modifier_buttons:
            button.connect("toggled", self.__modifier_button_toggled_cb, name)

        table.attach(self.__modifier_buttons[0][1], 0, 1, 0, 1)
        table.attach(self.__modifier_buttons[1][1], 1, 2, 0, 1)
        table.attach(self.__modifier_buttons[2][1], 2, 3, 0, 1)
        table.attach(self.__modifier_buttons[3][1], 3, 4, 0, 1)
        table.attach(self.__modifier_buttons[4][1], 0, 1, 1, 2)
        table.attach(self.__modifier_buttons[5][1], 1, 2, 1, 2)
        table.attach(self.__modifier_buttons[6][1], 2, 3, 1, 2)
        table.attach(self.__modifier_buttons[7][1], 3, 4, 1, 2)
        hbox.pack_start(table, True, True, 4)
        self.pack_start(hbox, False, True, 4)

        # buttons
        hbox = gtk.HBox()
        # add button
        self.__add_button = gtk.Button(stock = gtk.STOCK_ADD)
        self.__add_button.connect("clicked", self.__add_button_clicked_cb)
        hbox.pack_start(self.__add_button)
        # apply button
        self.__apply_button = gtk.Button(stock = gtk.STOCK_APPLY)
        self.__apply_button.connect("clicked", self.__apply_button_clicked_cb)
        hbox.pack_start(self.__apply_button)
        # delete button
        self.__delete_button = gtk.Button(stock = gtk.STOCK_DELETE)
        self.__delete_button.connect("clicked", self.__delete_button_clicked_cb)
        hbox.pack_start(self.__delete_button)
        self.pack_start(hbox, False, True, 4)

    def set_shortcuts(self, shortcuts = None):
        if shortcuts == None:
            shortcuts = []
        model = self.__shortcut_view.get_model()
        model.clear()
        for shortcut in shortcuts:
            model.insert(0, (shortcut,))
        # self.__shortcut_view.set_model(model)

    def get_shortcuts(self):
        model = self.__shortcut_view.get_model()
        return map(lambda i:i[0] , model)

    def add_shortcut(self, shortcut):
        model = self.__shortcut_view.get_model()
        model.insert(-1, (shortcut,))

    def __get_shortcut_from_buttons(self):
        modifiers = []
        keycode = self.__keycode_entry.get_text()
        if gdk.keyval_from_name(keycode) == 0:
            return None

        for name, button, mask in self.__modifier_buttons:
            if button.get_active():
                modifiers.append(name)
        if keycode.startswith("_"):
            keycode = keycode[1:]
        keys = modifiers + [keycode]
        shortcut = "+".join(keys)
        return shortcut

    def __set_shortcut_to_buttons(self, shortcut):
        keys = shortcut.split("+")
        mods = keys[:-1]
        for name, button, mask in self.__modifier_buttons:
            if name in mods:
                button.set_active(True)
            else:
                button.set_active(False)
        self.__keycode_entry.set_text(keys[-1])

    def __get_selected_shortcut(self):
        model = self.__shortcut_view.get_model()
        path, column = self.__shortcut_view.get_cursor()
        if path == None:
            return None
        return model[path[0]][0]

    def __set_selected_shortcut(self, shortcut):
        model = self.__shortcut_view.get_model()
        path, column = self.__shortcut_view.get_cursor()
        model[path[0]][0] = shortcut

    def __del_selected_shortcut(self):
        model = self.__shortcut_view.get_model()
        path, column = self.__shortcut_view.get_cursor()
        del model[path[0]]

    def __shortcut_view_cursor_changed_cb(self, treeview):
        shortcut = self.__get_selected_shortcut()
        self.__set_shortcut_to_buttons(shortcut)
        if shortcut != None:
            self.__delete_button.set_sensitive(True)
        else:
            self.__delete_button.set_sensitive(False)

    def __modifier_button_toggled_cb(self, button, name):
        shortcut = self.__get_shortcut_from_buttons()
        selected_shortcut = self.__get_selected_shortcut()
        self.__add_button.set_sensitive(shortcut != None)
        can_apply = shortcut != selected_shortcut and shortcut != None and selected_shortcut != None
        self.__apply_button.set_sensitive(can_apply)

    def __keycode_entry_notify_cb(self, entry, arg):
        shortcut = self.__get_shortcut_from_buttons()
        selected_shortcut = self.__get_selected_shortcut()
        self.__add_button.set_sensitive(shortcut != None)
        can_apply = shortcut != selected_shortcut and shortcut != None and selected_shortcut != None
        self.__apply_button.set_sensitive(can_apply)

    def __keycode_button_clicked_cb(self, button):
        out = []
        dlg = gtk.MessageDialog(parent = self.get_toplevel(), buttons = gtk.BUTTONS_CLOSE)
        message = _("Please press a key (or a key combination).\nThe dialog will be closed when the key is released.")
        dlg.set_markup(message)
        dlg.set_title(_("Please press a key (or a key combination)"))

        def __key_release_event(d, k, out):
            out.append(k.copy())
            d.response(gtk.RESPONSE_OK)

        dlg.connect("key-release-event", __key_release_event, out)
        id = dlg.run()
        dlg.destroy()
        if id != gtk.RESPONSE_OK or not out:
            return
        keyevent = out[0]
        for name, button, mask in self.__modifier_buttons:
            if keyevent.state & mask:
                button.set_active(True)
            else:
                button.set_active(False)
        self.__keycode_entry.set_text(gdk.keyval_name(keyevent.keyval))

    def __add_button_clicked_cb(self, button):
        shortcut = self.__get_shortcut_from_buttons()
        self.add_shortcut(shortcut)

    def __apply_button_clicked_cb(self, button):
        shortcut = self.__get_shortcut_from_buttons()
        self.__set_selected_shortcut(shortcut)

    def __delete_button_clicked_cb(self, button):
        self.__del_selected_shortcut()
        self.__delete_button.set_sensitive(False)
        self.__apply_button.set_sensitive(False)

class KeyboardShortcutSelectionDialog(gtk.Dialog):
    def __init__(self, title = None, parent = None, flags = 0, buttons = None):
        super(KeyboardShortcutSelectionDialog, self).__init__(title, parent, flags, buttons)
        self.__selection_view = KeyboardShortcutSelection()
        self.vbox.pack_start(self.__selection_view)
        self.vbox.show_all()

    def set_shortcuts(self, shotrcuts = None):
        self.__selection_view.set_shortcuts(shotrcuts)

    def add_shortcut(self, shotrcut):
        self.__selection_view.add_shortcut(shotrcut)

    def get_shortcuts(self):
        return self.__selection_view.get_shortcuts()



if __name__ == "__main__":
    dlg = KeyboardShortcutSelectionDialog(
        title = "Select test",
        buttons = (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_OK, gtk.RESPONSE_OK))
    dlg.add_shortcut("Control+Shift+space")
    dlg.set_shortcuts(None)
    print dlg.run()
    print dlg.get_shortcuts()

