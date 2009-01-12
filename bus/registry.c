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
#include <gio/gio.h>
#include <stdlib.h>
#include "xmlparser.h"
#include "registry.h"
#include "component.h"
#include "observedpath.h"

enum {
    LAST_SIGNAL,
};


/* BusRegistryPriv */
struct _BusRegistryPrivate {
    gpointer pad;
};
typedef struct _BusRegistryPrivate BusRegistryPrivate;

#define BUS_REGISTRY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_REGISTRY, BusRegistryPrivate))

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void              bus_registry_class_init        (BusRegistryClass   *klass);
static void              bus_registry_init              (BusRegistry        *registry);
static void              bus_registry_destroy           (BusRegistry        *registry);
static void              bus_registry_load              (BusRegistry        *registry);
static void              bus_registry_load_in_dir       (BusRegistry        *registry,
                                                         const gchar        *dirname);
static gboolean          bus_registry_save_cache        (BusRegistry        *registry);
static gboolean          bus_registry_load_cache        (BusRegistry        *registry);
static gboolean          bus_registry_check_modification(BusRegistry        *registry);
static void              bus_registry_remove_all        (BusRegistry        *registry);

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

    // g_type_class_add_private (klass, sizeof (BusRegistryPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_registry_destroy;

}

static void
bus_registry_init (BusRegistry *registry)
{
    registry->observed_paths = NULL;
    registry->components = NULL;

    if (bus_registry_load_cache (registry) == FALSE || bus_registry_check_modification (registry)) {
        bus_registry_remove_all (registry);
        bus_registry_load (registry);
        bus_registry_save_cache (registry);
    }
}

static void
bus_registry_remove_all (BusRegistry *registry)
{
    GSList *p;

    for (p = registry->observed_paths; p != NULL; p = p->next) {
        g_object_unref (p->data);
    }
    g_slist_free (registry->observed_paths);
    registry->observed_paths = NULL;

    for (p = registry->components; p != NULL; p = p ->next) {
        g_object_unref (p->data);
    }
    g_slist_free (registry->components);
    registry->components = NULL;
   
}

static void
bus_registry_destroy (BusRegistry *registry)
{
    bus_registry_remove_all (registry);
    IBUS_OBJECT_CLASS (parent_class)->destroy (IBUS_OBJECT (registry));
}


static void
bus_registry_load (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    gchar *dirname;
    gchar *homedir;
    BusObservedPath *path;

    dirname = g_build_filename (PKGDATADIR, "component", NULL);
    
    path = bus_observed_path_new_from_path (dirname);
    registry->observed_paths = g_slist_append (registry->observed_paths, path);
    
    bus_registry_load_in_dir (registry, dirname);
    
    g_free (dirname);

    homedir = (gchar *) g_getenv ("HOME");
    if (!homedir)
        homedir = (gchar *) g_get_home_dir ();
    dirname = g_build_filename (homedir, ".ibus", "component", NULL);
    
    path = bus_observed_path_new_from_path (dirname);
    registry->observed_paths = g_slist_append (registry->observed_paths, path);
    
    bus_registry_load_in_dir (registry, dirname);
    
    g_free (dirname);
}


#define g_string_append_indent(string, indent)  \
    {                                           \
        gint i;                                 \
        for (i = 0; i < (indent); i++) {        \
            g_string_append (string, "    ");   \
        }                                       \
    }

static gboolean
bus_registry_load_cache (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    gchar *filename;
    XMLNode *node;
    GSList *p;

    filename = g_build_filename (g_get_user_cache_dir (), "ibus", "registry.xml", NULL);
    node = xml_parse_file (filename);
    g_free (filename);

    if (node == NULL) {
        return FALSE;
    }

    if (g_strcmp0 (node->name, "ibus-registry") != 0) {
        xml_free_node (node);
        return FALSE;
    }

    for (p = node->sub_nodes; p != NULL; p = p->next) {
        XMLNode *sub_node = (XMLNode *) p->data;

        if (g_strcmp0 (sub_node->name, "observed-paths") == 0) {
            GSList *pp;
            for (pp = sub_node->sub_nodes; pp != NULL; pp = pp->next) {
                BusObservedPath *path;
                path = bus_observed_path_new_from_xml_node (pp->data, FALSE);
                if (path) {
                    registry->observed_paths = g_slist_append (registry->observed_paths, path);
                }
            }
            continue;
        }
        if (g_strcmp0 (sub_node->name, "components") == 0) {
            GSList *pp;
            for (pp = sub_node->sub_nodes; pp != NULL; pp = pp->next) {
                BusComponent *component;
                component = bus_component_new_from_xml_node (pp->data);
                if (component) {
                    registry->components = g_slist_append (registry->components, component);
                }
            }

            continue;
        }
        g_warning ("Unknown element <%s>", sub_node->name);
    }

    xml_free_node (node);
    return TRUE;
}

static gboolean
bus_registry_check_modification (BusRegistry *registry)
{
    GSList *p;

    for (p = registry->observed_paths; p != NULL; p = p->next) {
        if (bus_observed_path_check_modification ((BusObservedPath *)p->data))
            return TRUE;
    }

    for (p = registry->components; p != NULL; p = p->next) {
        if (bus_component_check_modification ((BusComponent *)p->data))
            return TRUE;
    }

    return FALSE;
}

static gboolean
bus_registry_save_cache (BusRegistry *registry)
{
    g_assert (BUS_IS_REGISTRY (registry));

    gchar *cachedir;
    gchar *filename;
    GString *output;
    GSList *p;
    FILE *pf;

    cachedir = g_build_filename (g_get_user_cache_dir (), "ibus", NULL);
    filename = g_build_filename (cachedir, "registry.xml", NULL);
    g_mkdir_with_parents (cachedir, 0775);
    pf = g_fopen (filename, "w");
    g_free (filename);
    g_free (cachedir);

    if (pf == NULL) {
        g_warning ("create registry.xml failed");
        return FALSE;
    }

    output = g_string_new ("");
    g_string_append (output, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
    g_string_append (output, "<!-- \n"
                             "    This file was generated by ibus-daemon. Please don't modify it.\n"
                             "    -->\n");
    g_string_append (output, "<ibus-registry>\n");

    if (registry->observed_paths) {
        g_string_append_indent (output, 1);
        g_string_append (output, "<observed-paths>\n");
        for (p = registry->observed_paths; p != NULL; p = p->next) {
            bus_observed_path_output ((BusObservedPath *)p->data,
                                      output, 2);
        }
        g_string_append_indent (output, 1);
        g_string_append (output, "</observed-paths>\n");
    }
    
    if (registry->components) {
        g_string_append_indent (output, 1);
        g_string_append (output, "<components>\n");
        for (p = registry->components; p != NULL; p = p->next) {
            bus_component_output ((BusComponent *)p->data,
                                      output, 2);
        }
        g_string_append_indent (output, 1);
        g_string_append (output, "</components>\n");
    }

    g_string_append (output, "</ibus-registry>\n");
    fwrite (output->str, output->len, 1, pf);
    g_string_free (output, TRUE);
    fclose (pf);
    return TRUE;
}

static void
bus_registry_load_in_dir (BusRegistry *registry,
                          const gchar *dirname)
{
    g_assert (BUS_IS_REGISTRY (registry));
    g_assert (dirname);

    GError *error = NULL;
    GDir *dir;
    const gchar *filename;

    dir = g_dir_open (dirname, 0, &error);

    if (dir == NULL) {
        g_warning ("Unable open directory %s : %s", dirname, error->message);
        g_error_free (error);
        return;
    }

    while ((filename = g_dir_read_name (dir)) != NULL) {
        glong size;
        gchar *path;
        BusComponent *component;

        size = g_utf8_strlen (filename, -1);
        if (g_strcmp0 (MAX (filename, filename + size -4), ".xml" ) != 0)
            continue;

        path = g_build_filename (dirname, filename, NULL);
        component = bus_component_new_from_file (path);
        registry->components = g_slist_append (registry->components, component);
        
        g_free (path);
    }

    g_dir_close (dir);
}


BusRegistry *
bus_registry_new (void)
{
    BusRegistry *registry;
    registry = (BusRegistry *)g_object_new (BUS_TYPE_REGISTRY, NULL);
    return registry;
}

static BusComponent *
bus_registry_lookup_component_by_name (BusRegistry *registry,
                                       const gchar *name)
{
    return NULL;
}

static BusComponent *
bus_registry_lookup_component_by_pid (BusRegistry *registry,
                                      GPid         pid)
{
    return NULL;
}
