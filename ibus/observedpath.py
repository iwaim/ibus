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
        "ObservedPath",
    )

import dbus
from exception import IBusException
from serializable import *

class ObservedPath(Serializable):
    __NAME__ = "IBusObservedPath"
    def __init__ (self, path="", mtime=0):
        super(ObservedPath, self).__init__()
        self.__path = path
        self.__mtime = mtime

    def get_path(self):
        return self.__path

    def get_mtime(self):
        return self.__mtime

    path        = property(get_path)
    mtime       = property(get_mtime)

    def serialize(self, struct):
        super(ObservedPath, self).serialize(struct)
        struct.append (dbus.String(self.__path))
        struct.append (dbus.Int64(self.__mtime))

    def deserialize(self, struct):
        super(ObservedPath, self).deserialize(struct)
        self.__path = struct.pop(0)
        self.__mtime = struct.pop(0)

serializable_register(ObservedPath)

def test():
    op = ObservedPath("/tmp", 111)
    value = serialize_object(op)
    op=  deserialize_object(value)

if __name__ == "__main__":
    test()
