/* vim:set et sts=4: */
/* bus - The Input Bus
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

#include "registry.h"

enum {
    LAST_SIGNAL,
};


/* BusRegistryPriv */
struct _BusRegistryPrivate {
    gpointer pad;
};
typedef struct _BusRegistryPrivate BusRegistryPrivate;

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void      bus_registry_class_init  (BusRegistryClass     *klass);
static void      bus_registry_init        (BusRegistry          *server);
static void      bus_registry_destroy     (BusRegistry          *server);

static IBusObjectClass  *parent_class = NULL;

GType
bus_registry_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusRegistryClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_registry_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusRegistry),
        0,
        (GInstanceInitFunc) bus_registry_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "BusRegistry",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}


static void
bus_registry_class_init (BusRegistryClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusRegistryPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_registry_destroy;

}

static void
bus_registry_init (BusRegistry *server)
{
}

static void
bus_registry_destroy (BusRegistry *server)
{
    IBUS_OBJECT_CLASS (parent_class)->destroy (IBUS_OBJECT (server));
}
