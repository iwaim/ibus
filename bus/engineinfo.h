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
#ifndef __ENGINE_INFO_H_
#define __ENGINE_INFO_H_

#include <ibus.h>

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_ENGINE_INFO             \
    (bus_engine_info_get_type ())
#define BUS_ENGINE_INFO(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_ENGINE_INFO, BusEngineInfo))
#define BUS_ENGINE_INFO_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_ENGINE_INFO, BusEngineInfoClass))
#define BUS_IS_ENGINE_INFO(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_ENGINE_INFO))
#define BUS_IS_ENGINE_INFO_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_ENGINE_INFO))
#define BUS_ENGINE_INFO_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_ENGINE_INFO, BusEngineInfoClass))

G_BEGIN_DECLS

typedef struct _BusEngineInfo BusEngineInfo;
typedef struct _BusEngineInfoClass BusEngineInfoClass;
typedef struct _BusComponent BusComponent;

struct _BusEngineInfo {
    IBusObject parent;
    /* instance members */
    
    gchar *name;
    gchar *longname;
    gchar *description;
    gchar *language;
    gchar *license;
    gchar *author;
    gchar *icon;
    gchar *layout;
    
    BusComponent *component;
};

struct _BusEngineInfoClass {
  IBusObjectClass parent;

  /* class members */
};

GType            bus_engine_info_get_type       (void);
BusEngineInfo   *bus_engine_info_from_xml_node  (BusComponent   *component,
                                                 XMLNode        *node);
void             bus_engine_info_output         (BusEngineInfo  *info,
                                                 GString        *output,
                                                 gint            indent);

G_END_DECLS
#endif

