# vim:set et ts=4:
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

# if not 1, append datestamp to the version number.
m4_define([ibus_released], [0])
m4_define([ibus_major_version], [0])
m4_define([ibus_minor_version], [1])
m4_define([ibus_micro_version], [1])
m4_define(ibus_maybe_datestamp,
    m4_esyscmd([if test x]ibus_released[ != x1; then date +.%Y%m%d | tr -d '\n\r'; fi]))

m4_define([ibus_version],
    ibus_major_version.ibus_minor_version.ibus_micro_version[]ibus_maybe_datestamp)

AC_INIT([ibus], [ibus_version], [http://code.google.com/p/ibus/issues/entry],[ibus])
AM_INIT_AUTOMAKE([1.10])
AC_GNU_SOURCE

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# define PACKAGE_VERSION_* variables
AS_VERSION
AS_NANO
AM_MAINTAINER_MODE
AM_DISABLE_STATIC
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_ISC_POSIX
AC_HEADER_STDC
AM_PROG_LIBTOOL

# check inotify
AC_CHECK_HEADERS([sys/inotify.h])

# check glib2
AM_PATH_GLIB_2_0
PKG_CHECK_MODULES(GLIB2, [
    glib-2.0
])
PKG_CHECK_MODULES(GOBJECT2, [
    gobject-2.0
])


# check gtk, gdk & pygtk
PKG_CHECK_MODULES(GTK2, [
    gtk+-2.0
])

GTK_BINARY_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
GTK_IM_MODULEDIR="$libdir"/gtk-2.0/immodules

PKG_CHECK_MODULES(GDK2, [
    gdk-2.0
])

# check dbus-glib
PKG_CHECK_MODULES(DBUS, [
    dbus-glib-1
])


# Check for Qt4
PKG_CHECK_MODULES(QT4,
    [QtCore >= 4.4.0 QtDBus >= 4.4.0],
    [IBUS_HAS_QT4=yes],
    [IBUS_HAS_QT4=no]
)

if test x"$IBUS_HAS_QT4" == x"yes"; then
    QMAKE4=`$PKG_CONFIG --variable=prefix Qt`/bin/qmake
    AC_SUBST(QMAKE4)
fi

# Check for Qt3
PKG_CHECK_MODULES(QT3,
    [qt-mt >= 3.3],
    [IBUS_HAS_QT3=yes],
    [IBUS_HAS_QT3=no]
)

if test x"$IBUS_HAS_QT3" == x"yes"; then
    QMAKE3=`$PKG_CONFIG --variable=prefix qt-mt`/bin/qmake
    AC_SUBST(QMAKE3)
fi

# Check dbus-qt3
PKG_CHECK_MODULES(DBUS_QT3,
    [dbus-1-qt3 >= 0.9.0],
    [IBUS_HAS_DBUS_QT3=yes],
    [IBUS_HAS_DBUS_QT3=no]
)

# Check for x11
PKG_CHECK_MODULES(X11, [
    x11
])

AC_CHECK_HEADERS([X11/XKBlib.h])

# check env
AC_PATH_PROG(ENV, env)
AC_SUBST(ENV)


# check python
AM_PATH_PYTHON([2.5])
AC_PATH_PROG(PYTHON_CONFIG, python$PYTHON_VERSION-config)
if test x"$PYTHON_CONFIG" == x""; then
    AC_PATH_PROG(PYTHON_CONFIG, python-config)
fi
if test x"$PYTHON_CONFIG" != x""; then
    PYTHON_CFLAGS=`$PYTHON_CONFIG --includes`
    PYTHON_LIBS=`$PYTHON_CONFIG --libs`
else
    PYTHON_CFLAGS=`$PYTHON $srcdir/python-config.py --includes`
    PYTHON_LIBS=`$PYTHON $srcdir/python-config.py --libs`
fi
PYTHON_INCLUDES="$PYTHON_CFLAGS"
AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_INCLUDES)
AC_SUBST(PYTHON_LIBS)

# check pygtk-2.0
PKG_CHECK_MODULES(PYGTK,
    pygtk-2.0 >= 2.10.3,
    [IBUS_HAS_PYGTK=yes],
    [IBUS_HAS_PYGTK=no]
)
AC_SUBST(PYGTK_CFLAGS)
AC_SUBST(PYGTK_LIBS)

# check pygtk-code-gen-2.0
AC_PATH_PROG(PYGTK_CODEGEN, pygtk-codegen-2.0, no)

# check gconf-2.0
PKG_CHECK_MODULES(GCONF,
    [gconf-2.0 >= 2.11.1],
    [IBUS_HAS_GCONF=yes],
    [IBUS_HAS_GCONF=no]
)
AC_SUBST(GCONF_CFLAGS)
AC_SUBST(GCONF_LIBS)

# check iso-codes
PKG_CHECK_MODULES(ISOCODES,
    iso-codes,
    [IBUS_HAS_ISOCODES=yes],
    [IBUS_HAS_ISOCODES=no],
)

# define GETTEXT_* variables
GETTEXT_PACKAGE=ibus
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Define to the read-only architecture-independent data directory.])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.16.1)


# define --with-gtk-im-module-dir
AC_ARG_WITH(gtk-im-module-dir,
    AS_HELP_STRING([--with-gtk-im-module-dir[=DIR]],
        [Select gtk immodule dir]),
    GTK_IM_MODULEDIR=$with_gtk_im_module_dir
)
AC_SUBST(GTK_IM_MODULEDIR)

#define --disable-qt4-immodule
AC_ARG_ENABLE(qt4-immodule,
    AS_HELP_STRING([--disable-qt4-immodule],
        [do not build qt4 immodule]),
    [enable_qt4=$enableval],
    [enable_qt4=yes],
)
if test x"$IBUS_HAS_QT4" != x"yes"; then
    AC_MSG_WARN([can not find qt4 for building qt4 im module.])
    enable_qt4=no
fi
AM_CONDITIONAL(IBUS_BUILD_QT4, [test x"$enable_qt4" = x"yes" ])

#define --disable-qt3-immodule
AC_ARG_ENABLE(qt3-immodule,
    AS_HELP_STRING([--disable-qt3-immodule],
        [do not build qt3 immodule]),
    [enable_qt3=$enableval],
    [enable_qt3=yes],
)
if test x"$IBUS_HAS_QT3" != x"yes"; then
    AC_MSG_WARN([can not find qt3 for building qt3 im module.])
    enable_qt3=no
fi
if test x"$IBUS_HAS_DBUS_QT3" != x"yes"; then
    AC_MSG_WARN([can not find dbus-qt3 for building qt3 im module.])
    enable_qt3=no
fi

AM_CONDITIONAL(IBUS_BUILD_QT3, [test x"$enable_qt3" = x"yes" ])

# define --enable-pygconf
AC_ARG_ENABLE(pygconf,
    AS_HELP_STRING([--enable-pygconf],
        [Build python gconf binding, with it ibus will not depend on gnome-python]),
    [enable_pygconf=$enableval],
    [enable_pygconf=no],
)
if test x"$IBUS_HAS_PYGTK" != x"yes" -a x"$enable_pygconf" = x"yes"; then
  AC_MSG_ERROR(can not build python gconf binding without pygtk-2.0)
fi
if test x"$PYGTK_CODEGEN" = x"no" -a x"$enable_pygconf" = x"yes"; then
  AC_MSG_ERROR(can not build python gconf binding without pygtk-codegen-2.0)
fi
if test x"$IBUS_HAS_GCONF" != x"yes" -a x"$enable_pygconf" = x"yes"; then
  AC_MSG_ERROR(can not build python gconf binding without gconf-2.0)
fi
AM_CONDITIONAL(IBUS_BUILD_PYGCONF, [test x"$enable_pygconf" = x"yes" ])

# define --disable-iso-codes-check
AC_ARG_ENABLE(iso-codes-check,
    AS_HELP_STRING([--disable-iso-codes-check],
        [Do not check iso-codes]),
    [enable_iso_codes_check=$enableval],
    [enable_iso_codes_check=yes],
)
if test x"$IBUS_HAS_ISOCODES" != x"yes"; then
    if test x"$enable_iso_codes_check" != x"no"; then
        AC_MSG_ERROR([can not find iso-codes. Please install iso-codes or configure with --disable-iso-codes-check])
    else
        AC_MSG_WARN([can not find iso-codes. It is required.])
    fi
fi

# OUTPUT files
AC_CONFIG_FILES([ po/Makefile.in
Makefile
ibus.spec
ibus/config.py
ibus/Makefile
ibus/interface/Makefile
daemon/Makefile
daemon/ibus-daemon
launcher/ibus
launcher/ibus.desktop
launcher/Makefile
panel/Makefile
panel/ibus-panel
gconf/Makefile
gconf/ibus-gconf
lib/Makefile
lib/gtk2/Makefile
client/Makefile
client/gtk2/Makefile
client/qt4/Makefile
client/qt3/Makefile
client/x11/Makefile
setup/Makefile
setup/ibus-setup
setup/ibus-setup.desktop
util/Makefile
util/IMdkit/Makefile
util/gconf/Makefile
icons/Makefile
m4/Makefile
])

AC_OUTPUT
AC_MSG_RESULT([
Build options:
  Version                  $VERSION
  Install prefix           $prefix
  Build shared libs        $enable_shared
  Build static libs        $enable_static
  Gtk immodule dir         $GTK_IM_MODULEDIR
  Enable Qt4 IM module     $enable_qt4
  Enable Qt3 IM module     $enable_qt3
  Build pygconf            $enable_pygconf
])

