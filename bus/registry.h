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
#ifndef __REGISTRY_H_
#define __REGISTRY_H_

#include <ibus.h>

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_REGISTRY             \
    (bus_registry_get_type ())
#define BUS_REGISTRY(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_REGISTRY, BusRegistry))
#define BUS_REGISTRY_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_REGISTRY, BusRegistryClass))
#define BUS_IS_REGISTRY(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_REGISTRY))
#define BUS_IS_REGISTRY_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_REGISTRY))
#define BUS_REGISTRY_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_REGISTRY, BusRegistryClass))
#define BUS_DEFAULT_REGISTRY          \
    (bus_registry_get_default ())

G_BEGIN_DECLS

typedef struct _BusRegistry BusRegistry;
typedef struct _BusRegistryClass BusRegistryClass;

struct _BusRegistry {
  IBusObject parent;
  /* instance members */
};

struct _BusRegistryClass {
  IBusObjectClass parent;

  /* class members */
};

GType            bus_registry_get_type        (void);
BusRegistry     *bus_registry_get_default     (void);

G_END_DECLS
#endif

