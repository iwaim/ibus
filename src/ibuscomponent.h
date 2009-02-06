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
#ifndef __IBUS_COMPONENT_H_
#define __IBUS_COMPONENT_H_

#include "ibusserializable.h"
#include "ibusobservedpath.h"
#include "ibusenginedesc.h"
#include "ibusxml.h"

/*
 * Type macros.
 */

/* define GOBJECT macros */
#define IBUS_TYPE_COMPONENT             \
    (ibus_component_get_type ())
#define IBUS_COMPONENT(obj)             \
    (G_TYPE_CHECK_INSTANCE_CAST ((obj), IBUS_TYPE_COMPONENT, IBusComponent))
#define IBUS_COMPONENT_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_COMPONENT, IBusComponentClass))
#define IBUS_IS_COMPONENT(obj)          \
    (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IBUS_TYPE_COMPONENT))
#define IBUS_IS_COMPONENT_CLASS(klass)  \
    (G_TYPE_CHECK_CLASS_TYPE ((klass), IBUS_TYPE_COMPONENT))
#define IBUS_COMPONENT_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_COMPONENT, IBusComponentClass))

G_BEGIN_DECLS

typedef struct _IBusComponent IBusComponent;
typedef struct _IBusComponentClass IBusComponentClass;

struct _IBusComponent {
    IBusSerializable parent;
    /* instance members */

    gchar *name;
    gchar *description;
    gchar *version;
    gchar *license;
    gchar *author;
    gchar *homepage;
    gchar *exec;

    /* text domain for dgettext */
    gchar *textdomain;

    /* engines */
    GList *engines;

    /* observed paths */
    GList *observed_paths;

    GPid     pid;
};

struct _IBusComponentClass {
  IBusSerializableClass parent;

  /* class members */
};

GType            ibus_component_get_type        (void);
IBusComponent   *ibus_component_new             (const gchar    *name,
                                                 const gchar    *descritpion,
                                                 const gchar    *version,
                                                 const gchar    *license,
                                                 const gchar    *author,
                                                 const gchar    *homepage,
                                                 const gchar    *exec,
                                                 const gchar    *textdomain);
IBusComponent   *ibus_component_new_from_xml_node
                                                (XMLNode        *node);
IBusComponent   *ibus_component_new_from_file   (const gchar    *filename);
void             ibus_component_add_observed_path
                                                (IBusComponent  *component,
                                                 const gchar    *path,
                                                 gboolean        access_fs);
void             ibus_component_add_engine      (IBusComponent  *component,
                                                 IBusEngineDesc *engine);
GList           *ibus_component_get_engines     (IBusComponent  *component);
void             ibus_component_output          (IBusComponent  *component,
                                                 GString        *output,
                                                 gint            indent);
void             ibus_component_output_engines  (IBusComponent  *component,
                                                 GString        *output,
                                                 gint            indent);
gboolean         ibus_component_check_modification
                                                (IBusComponent  *component);
gboolean         ibus_component_start           (IBusComponent  *component);
gboolean         ibus_component_stop            (IBusComponent  *component);
gboolean         ibus_component_is_running      (IBusComponent  *component);
IBusComponent   *ibus_component_get_from_engine (IBusEngineDesc *engine);

G_END_DECLS
#endif

