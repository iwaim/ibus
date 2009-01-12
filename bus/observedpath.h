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
#ifndef __OBSERVED_PATH_H_
#define __OBSERVED_PATH_H_

#include <ibus.h>

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define BUS_TYPE_OBSERVED_PATH             \
    (bus_observed_path_get_type ())
#define BUS_OBSERVED_PATH(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), BUS_TYPE_OBSERVED_PATH, BusObservedPath))
#define BUS_OBSERVED_PATH_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), BUS_TYPE_OBSERVED_PATH, BusObservedPathClass))
#define BUS_IS_OBSERVED_PATH(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BUS_TYPE_OBSERVED_PATH))
#define BUS_IS_OBSERVED_PATH_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), BUS_TYPE_OBSERVED_PATH))
#define BUS_OBSERVED_PATH_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), BUS_TYPE_OBSERVED_PATH, BusObservedPathClass))

G_BEGIN_DECLS

typedef struct _BusObservedPath BusObservedPath;
typedef struct _BusObservedPathClass BusObservedPathClass;

struct _BusObservedPath {
    IBusObject parent;
    /* instance members */

    gchar *path;
    glong mtime;
    gboolean is_dir;
    gboolean is_exist;

};

struct _BusObservedPathClass {
  IBusObjectClass parent;

  /* class members */
};

GType            bus_observed_path_get_type             (void);
BusObservedPath *bus_observed_path_new_from_xml_node    (XMLNode            *node,
                                                         gboolean            fill_stat);
BusObservedPath *bus_observed_path_new_from_path        (const gchar        *path);
GList           *bus_observed_path_traverse             (BusObservedPath    *path);
gboolean         bus_observed_path_check_modification   (BusObservedPath    *path);
void             bus_observed_path_output               (BusObservedPath    *path,
                                                         GString            *output,
                                                         gint                indent);

G_END_DECLS
#endif

