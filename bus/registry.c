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
#include <glib/gstdio.h>
#include "xmlparser.h"
#include "registry.h"

enum {
    LAST_SIGNAL,
};


/* BusRegistryPriv */
struct _BusRegistryPrivate {
    GSList *components;
};
typedef struct _BusRegistryPrivate BusRegistryPrivate;

#define BUS_REGISTRY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_REGISTRY, BusRegistryPrivate))

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void      bus_registry_class_init  (BusRegistryClass     *klass);
static void      bus_registry_init        (BusRegistry          *registry);
static void      bus_registry_destroy     (BusRegistry          *registry);
static void      bus_registry_load        (BusRegistry          *registry);
static void      bus_registry_load_in_dir (BusRegistryPrivate   *priv,
                                           const gchar          *dirname);
static void      bus_registry_load_component
                                          (BusRegistryPrivate   *priv,
                                           const gchar          *filename);

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
bus_registry_init (BusRegistry *registry)
{
    BusRegistryPrivate *priv = BUS_REGISTRY_GET_PRIVATE (registry);

    priv->components = NULL;

    bus_registry_load (registry);
}

static void
bus_registry_destroy (BusRegistry *registry)
{
    IBUS_OBJECT_CLASS (parent_class)->destroy (IBUS_OBJECT (registry));
}


static void
bus_registry_load (BusRegistry *registry)
{
    BusRegistryPrivate *priv = BUS_REGISTRY_GET_PRIVATE (registry);

    g_assert (priv->components == NULL);

    gchar *dirname;
    gchar *homedir;

    dirname = g_build_filename (PKGDATADIR, "component", NULL);
    bus_registry_load_in_dir (priv, dirname);
    g_free (dirname);

    homedir = (gchar *) g_getenv ("HOME");
    if (!homedir)
        homedir = (gchar *) g_get_home_dir ();
    dirname = g_build_filename (homedir, ".ibus", "component", NULL);
    bus_registry_load_in_dir (priv, dirname);
    g_free (dirname);
}


static void
bus_registry_load_in_dir (BusRegistryPrivate   *priv,
                          const gchar          *dirname)
{
    g_assert (priv != NULL);
    g_assert (dirname != NULL);

    GError *error = NULL;
    GDir *dir;
    const gchar *path;
    
    dir = g_dir_open (dirname, 0, &error);

    if (dir == NULL) {
        g_warning ("Unable open directory %s : %s", dirname, error->message);
        g_error_free (error);
        return;
    }
    
    while ((path = g_dir_read_name (dir)) != NULL) {
        glong size = g_utf8_strlen (path, -1);
        if (g_strcmp0 (MAX (path, path + size -4), ".xml" ) != 0)
            continue;
        path = g_build_filename (dirname, path, NULL);
        bus_registry_load_component (priv, path);
        g_free (path);
    }

    g_dir_close (dir);
}


static void
bus_registry_load_component (BusRegistryPrivate *priv,
                             const gchar        *filename)
{
    IBusComponent *comp;
    struct stat buf;
    XMLNode *node;

    g_assert (priv != NULL);
    g_assert (filename != NULL);

    if (g_stat (filename, &buf) != 0) {
        g_warning ("Can not get stat of file %s", filename);
        return;
    }

    comp = g_slice_new0 (IBusComponent);

    comp->filename = g_strdup (filename);
    comp->mtime = buf.st_mtime;

    node = xml_parse_file (filename);

    if (node == NULL) {
        g_warning ("parse %s failed", filename);
        return;
    }

    GString *string = g_string_new ("");

    xml_output (node, string);

    g_debug ("%s", string->str);

    xml_free_node (node);
}


BusRegistry *
bus_registry_get_default (void)
{
    static BusRegistry *registry = NULL;

    if (registry == NULL) {
        registry = g_object_new (BUS_TYPE_REGISTRY, NULL);
    }

    return registry;
}
