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
        "Text",
    )

import dbus
from exception import IBusException
from serializable import *

class Component(Serializable):
    __NAME__ = "IBusComponent"
    def __init__ (self, name, description, version, license, author, homepage, _exec, textdomain):
        super(Text, self).__init__()
        self.__name = name
        self.__descritpion = descritpion
        self.__version = version
        self.__license = license
        self.__author = author
        self.__homepage = homepage
        self.__exec = _exec
        self.__textdomain = textdomain

    def get_name(self):
        return self.__name

    def get_descritpion(self):
        return self.__descritpion

    def get_version(self):
        return self.__version

    def get_license(self):
        return self.__license

    def get_author(self):
        return self.__author
        
    def get_homepage(self):
        return self.__homepage

    def get_textdomain(self):
        return self.__textdomain

    text        = property(get_text)
    attributes  = property(get_attributes)

    def serialize(self, struct):
        super(Text, self).serialize(struct)
        struct.append (dbus.String(self.__text))
        if self.__attrs == None:
            self.__attrs = AttrList()
        struct.append (serialize_object(self.__attrs))

    def deserialize(self, struct):
        super(Text, self).deserialize(struct)

        self.__text = struct.pop(0)
        self.__attrs = deserialize_object(struct.pop(0))

serializable_register(Text)

def test():
    text = Text("Hello")
    value = serialize_object(text)
    text = deserialize_object(value)

if __name__ == "__main__":
    test()
