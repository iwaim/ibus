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
#ifndef __COMPONENT_H_
#define __COMPONENT_H_

#include <ibus.h>
#include "engineinfo.h"
#include "observedpath.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_COMPONENT             \
    (bus_component_get_type ())
#define BUS_COMPONENT(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_COMPONENT, BusComponent))
#define BUS_COMPONENT_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_COMPONENT, BusComponentClass))
#define BUS_IS_COMPONENT(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_COMPONENT))
#define BUS_IS_COMPONENT_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_COMPONENT))
#define BUS_COMPONENT_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_COMPONENT, BusComponentClass))

G_BEGIN_DECLS

// typedef struct _BusComponent BusComponent;
typedef struct _BusComponentClass BusComponentClass;

struct _BusComponent {
    IBusObject parent;
    /* instance members */
    
    gchar *name;
    gchar *description;
    gchar *exec;
    gchar *version;
    gchar *author;
    gchar *license;
    gchar *homepage;
    gchar *service_name;
    
    /* text domain for dgettext */
    gchar *textdomain;
    
    /* engines */
    GList *engines;
    
    /* observed paths */
    GList *observed_paths;
    
    GPid     pid;
};

struct _BusComponentClass {
  IBusObjectClass parent;

  /* class members */
};

GType            bus_component_get_type         (void);
BusComponent    *bus_component_new_from_xml_node(XMLNode        *node);
BusComponent    *bus_component_new_from_file    (const gchar    *filename);
void             bus_component_output           (BusComponent   *component,
                                                 GString        *output,
                                                 gint            indent);
gboolean         bus_component_check_modification
                                                (BusComponent   *component);
gboolean         bus_component_start            (BusComponent   *component);
gboolean         bus_component_stop             (BusComponent   *component);
gboolean         bus_component_is_running       (BusComponent   *component);

G_END_DECLS
#endif

