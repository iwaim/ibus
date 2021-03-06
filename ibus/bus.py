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

__all__ = (
        "Bus",
    )

import dbus
import dbus.lowlevel
import dbus.connection
import dbus.mainloop.glib
import gobject
import common
import object
import serializable
import config

dbus.mainloop.glib.DBusGMainLoop(set_as_default = True)

class Bus(object.Object):
    __gsignals__ = {
        "disconnected" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
        "config-reloaded" : (
            gobject.SIGNAL_RUN_LAST,
            gobject.TYPE_NONE,
            ()
        ),
    }

    def __init__(self):
        super(Bus, self).__init__()
        self.__dbusconn = dbus.connection.Connection(common.IBUS_ADDR)
        self.__dbus = self.__dbusconn.get_object(dbus.BUS_DAEMON_NAME,
                                                 dbus.BUS_DAEMON_PATH)
        self.__unique_name = self.hello()
        self.__ibus = self.__dbusconn.get_object(common.IBUS_SERVICE_IBUS,
                                                 common.IBUS_PATH_IBUS)

        self.__dbusconn.call_on_disconnection(self.__dbusconn_disconnected_cb)
        # self.__dbusconn.add_message_filter(self.__filter_cb)

    def __filter_cb(self, conn, message):
        if message.get_type() == 4:
            print "Signal %s" % message.get_member()
            print " sender = %s" % message.get_sender()
            print " path = %s" % message.get_path()
        return dbus.lowlevel.HANDLER_RESULT_NOT_YET_HANDLED

    def __dbusconn_disconnected_cb(self, dbusconn):
        assert self.__dbusconn == dbusconn
        self.__dbusconn = None
        self.emit("disconnected")

    def get_name(self):
        return self.__unique_name

    def get_is_connected(self):
        if self.__dbusconn == None:
            return False
        return self.__dbusconn.get_is_connected()

    # define dbus methods
    def get_dbus(self):
        return self.__dbus

    def hello(self):
        return self.__dbus.Hello()

    def request_name(self, name, flags):
        return self.__dbus.RequestName(name, dbus.UInt32 (flags))

    def release_name(self, name):
        return self.__dbus.ReleaseName(name)

    def get_name_owner(self, name):
        return self.__dbus.GetNameOwner(name)

    def add_match(self, rule):
        return self.__dbus.AddMatch(rule)

    def remove_match(self, rule):
        return self.__dbus.RemoveMatch(rule)

    def get_dbusconn(self):
        return self.__dbusconn

    def get_address(self):
        return common.IBUS_ADDR

    # define ibus methods
    def register_component(self, component):
        component = serializable.serialize_object(component)
        return self.__ibus.RegisterComponent(component)

    def list_engines(self):
        engines = self.__ibus.ListEngines()
        return map(serializable.deserialize_object, engines)

    def list_active_engines(self):
        engines = self.__ibus.ListActiveEngines()
        return map(serializable.deserialize_object, engines)

    def create_input_context(self, client_name):
        return self.__ibus.CreateInputContext(client_name)

    def exit(self, restart):
        return self.__ibus.Exit(restart)

    def get_config(self):
        try:
            return self.__config
        except:
            self.__config = config.Config(self)
            return self.__config

def test():
    import glib
    import factory
    import text

    mainloop = glib.MainLoop()

    def __disconnected_cb(*args):
        print "Disconnected", args
        mainloop.quit()

    b = Bus()
    b.connect("disconnected", __disconnected_cb)

    print "unique_name =", b.get_name()

    for i in b.list_factories():
        print i.name

    mainloop.run()
    print "Exit"


if __name__ == "__main__":
    test()
