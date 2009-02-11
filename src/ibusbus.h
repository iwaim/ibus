/* vim:set et sts=4: */
/* ibus - The Input Bus
 * Copyright (C) 2008-2009 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/**
 * SECTION: ibusbus
 * @short_description: iBus-daemon communicating functions.
 * 
 * iBus-bus handles the communication with iBus-daemon, including 
 * component registration, name request/release,
 * and connection status checking.
 *
 * It also provides some function to invoke functions in iBus-daemon.
 */
#ifndef __IBUS_BUS_H_
#define __IBUS_BUS_H_

#include <dbus/dbus.h>
#include "ibusinputcontext.h"
#include "ibusconfig.h"
#include "ibuscomponent.h"


/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_BUS             \
    (ibus_bus_get_type ())
#define IBUS_BUS(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_BUS, IBusBus))
#define IBUS_BUS_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_BUS, IBusBusClass))
#define IBUS_IS_BUS(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_BUS))
#define IBUS_IS_BUS_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_BUS))
#define IBUS_BUS_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_BUS, IBusBusClass))

G_BEGIN_DECLS

/**
 * IBusBus:
 *
 * Struct for containing iBus bus (daemon communication) status.
 */
typedef struct _IBusBus IBusBus;
typedef struct _IBusBusClass IBusBusClass;

struct _IBusBus {
  IBusObject parent;
  /* instance members */
};

struct _IBusBusClass {
  IBusObjectClass parent;
  /* class members */
};

GType        ibus_bus_get_type          (void);

/**
 * ibus_bus_new:
 * @returns: A newly allocated IBusBus instance.
 *
 * New an IBusBus instance.
 */
IBusBus     *ibus_bus_new               (void);

/**
 * ibus_bus_is_connected:
 * @bus: the IBusBus instance to be processed.
 * @returns: TRUE if @bus is connected, FALSE otherwise.
 *
 * Return TRUE if @bus is connected to iBus daemon.
 */
gboolean     ibus_bus_is_connected      (IBusBus        *bus);


/**
 * ibus_bus_get_connection:
 * @bus: the IBusBus instance to be processed.
 * @returns: TRUE if @bus is connected, FALSE otherwise.
 *
 * Return IBusConnection of an IBusIBus instance.
 */
IBusConnection
            *ibus_bus_get_connection    (IBusBus        *bus);

/* declare dbus methods */
const gchar *ibus_bus_hello             (IBusBus        *bus);

/**
 * ibus_bus_request_name:
 * @bus: the IBusBus instance to be processed.
 * @name: Name to be requested.
 * @returns: 0 if failed; positive number otherwise.
 * 
 * Request a name from iBus daemon.
 */
guint        ibus_bus_request_name      (IBusBus        *bus,
                                         const gchar    *name,
                                         guint           flags);
/**
 * ibus_bus_release_name:
 * @bus: the IBusBus instance to be processed.
 * @name: Name to be released.
 * @returns: 0 if failed; positive number otherwise.
 * 
 * Release a name to iBus daemon.
 */
guint        ibus_bus_release_name      (IBusBus        *bus,
                                         const gchar    *name);
gboolean     ibus_bus_name_has_owner    (IBusBus        *bus,
                                         const gchar    *name);
GList       *ibus_bus_list_names        (IBusBus        *bus);
void         ibus_bus_add_match         (IBusBus        *bus,
                                         const gchar    *rule);
void         ibus_bus_remove_match      (IBusBus        *bus,
                                         const gchar    *rule);
const gchar *ibus_bus_get_name_owner    (IBusBus        *bus,
                                         const gchar    *name);
/* declare ibus methods */
gboolean     ibus_bus_kill              (IBusBus        *bus);
IBusInputContext
            *ibus_bus_create_input_context
                                        (IBusBus        *bus,
                                         const gchar    *client_name);
gboolean     ibus_bus_register_component(IBusBus        *bus,
                                         IBusComponent  *component);
GList       *ibus_bus_list_engines      (IBusBus        *bus);
GList       *ibus_bus_list_active_engines
                                        (IBusBus        *bus);

G_END_DECLS
#endif

