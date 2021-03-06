# vim:set noet ts=4:
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

libibus = $(top_builddir)/src/libibus.la

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/src \
	$(NULL)

AM_CFLAGS = \
	@GLIB2_CFLAGS@ \
	@DBUS_CFLAGS@ \
	-DG_LOG_DOMAIN=\"IBUS\" \
	-DPKGDATADIR=\"$(pkgdatadir)\" \
	-DLIBEXECDIR=\"$(libexecdir)\" \
	$(INCLUDES) \
	$(NULL)
AM_LDFLAGS = \
	@GLIB2_LIBS@ \
	@DBUS_LIBS@ \
	$(libibus) \
	$(NULL)

TESTS = \
	test-matchrule \
	$(NULL)
xdgautostart_DATA = \
	ibus.desktop \
	$(NULL)
xdgautostartdir = $(sysconfdir)/xdg/autostart

desktopdir = $(datadir)/applications
desktop_in_files = ibus.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

noinst_PROGRAMS = $(TESTS)
bin_PROGRAMS = ibus-daemon
ibus_daemon_DEPENDENCIES = \
	$(libibus) \
	$(NULL)
ibus_daemon_SOURCES = \
	main.c \
	dbusimpl.c \
	dbusimpl.h \
	ibusimpl.c \
	ibusimpl.h \
	inputcontext.c \
	inputcontext.h \
	engineproxy.c \
	engineproxy.h \
	panelproxy.c \
	panelproxy.h \
	factoryproxy.c \
	factoryproxy.h \
	server.c \
	server.h \
	connection.c \
	connection.h \
	matchrule.c \
	matchrule.h \
	registry.c \
	registry.h \
	$(NULL)
ibus_daemon_CFLAGS = \
	$(AM_CFLAGS) \
	$(NULL)
ibus_daemon_LDADD = \
	$(AM_LDFLAGS) \
	$(NULL)

test_registry_SOURCES = \
	registry.c \
	registry.h \
	factoryproxy.c \
	factoryproxy.h \
	test-registry.c \
	$(NULL)

test_matchrule_SOURCES = \
	connection.c \
	matchrule.c \
	test-matchrule.c \
	$(NULL)

EXTRA_DIST = \
	$(desktop_in_files) \
	$(NULL)

DISTCLEANFILES = \
	$(desktop_DATA) \
	$(NULL)

$(libibus):
	$(MAKE) -C $(top_builddir)/src

test: ibus-daemon
	$(builddir)/ibus-daemon
