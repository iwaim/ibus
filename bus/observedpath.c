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
#include "observedpath.h"

enum {
    LAST_SIGNAL,
};


/* BusObservedPathPriv */
struct _BusObservedPathPrivate {
    gpointer pad;
};
typedef struct _BusObservedPathPrivate BusObservedPathPrivate;

#define BUS_OBSERVED_PATH_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_OBSERVED_PATH, BusObservedPathPrivate))

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void              bus_observed_path_class_init        (BusObservedPathClass  *klass);
static void              bus_observed_path_init              (BusObservedPath       *path);
static void              bus_observed_path_destroy           (BusObservedPath       *path);
static gboolean          bus_observed_path_parse_xml_node    (BusObservedPath       *path,
                                                              XMLNode               *node);

static IBusObjectClass  *parent_class = NULL;

GType
bus_observed_path_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusObservedPathClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_observed_path_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusObservedPath),
        0,
        (GInstanceInitFunc) bus_observed_path_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "BusObservedPath",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}


static void
bus_observed_path_class_init (BusObservedPathClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    // g_type_class_add_private (klass, sizeof (BusObservedPathPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_observed_path_destroy;

}


static void
bus_observed_path_init (BusObservedPath *path)
{
    path->path = NULL;
}

static void
bus_observed_path_destroy (BusObservedPath *path)
{
    g_free (path->path);
    IBUS_OBJECT_CLASS (parent_class)->destroy (IBUS_OBJECT (path));
}


#define g_string_append_indent(string, indent)  \
    {                                           \
        gint i;                                 \
        for (i = 0; i < (indent); i++) {        \
            g_string_append (string, "    ");   \
        }                                       \
    }

void
bus_observed_path_output (BusObservedPath *path,
                          GString         *output,
                          gint             indent)
{
    g_assert (BUS_IS_OBSERVED_PATH (path));
    g_assert (output);
    
    g_string_append_indent (output, indent);
    g_string_append_printf (output, "<path mtime=\"%ld\" >%s</path>\n",
                                    path->mtime,
                                    path->path);
}

gboolean
bus_observed_path_check_modification (BusObservedPath *path)
{
    g_assert (BUS_IS_OBSERVED_PATH (path));
    struct stat buf;

    if (g_stat (path->path, &buf) != 0) {
        if (path->mtime != 0l)
            return FALSE;
        return TRUE;
    }

    if (path->mtime != buf.st_mtime)
        return FALSE;
    return TRUE;
}

static void
bus_observed_path_fill_stat (BusObservedPath *path)
{
    g_assert (BUS_IS_OBSERVED_PATH (path));
    
    struct stat buf;
    
    if (g_stat (path->path, &buf) == 0) {
        path->is_exist = 1;
        if (S_ISDIR (buf.st_mode)) {
            path->is_dir = 1;
        }
        path->mtime = buf.st_mtime;
    }
    else {
        path->is_dir = 0;
        path->is_exist = 0;
        path->mtime = 0;
    }
}

GSList *
bus_observed_path_traverse (BusObservedPath *path)
{
    g_assert (BUS_IS_OBSERVED_PATH (path));

    GDir *dir;
    const gchar *name;
    GSList *paths = NULL;

    dir = g_dir_open (path->path, 0, NULL);

    if (dir == NULL)
        return NULL;

    while ((name = g_dir_read_name (dir)) != NULL) {
        BusObservedPath *sub;
        
        sub = g_object_new (BUS_TYPE_OBSERVED_PATH, NULL);
        sub->path = g_build_filename (path->path, name, NULL);
        
        bus_observed_path_fill_stat (sub);
        paths = g_slist_append (paths, sub);
        
        if (sub->is_exist && sub->is_dir)
            paths = g_slist_concat (paths, bus_observed_path_traverse (sub));
    }
    g_dir_close (dir);

    return paths;
}

static gboolean
bus_observed_path_parse_xml_node (BusObservedPath *path,
                                  XMLNode         *node)
{
    g_assert (BUS_IS_OBSERVED_PATH (path));
    g_assert (node);

    if (G_UNLIKELY (g_strcmp0 (node->name, "path") != 0)) {
        return FALSE;
    }
    
    if (node->text[0] == '~' && node->text[1] != G_DIR_SEPARATOR) {
        g_warning ("invalide path \"%s\"", node->text);
        return FALSE;
    }

    if (node->text[0] == '~') {
        const gchar *homedir = g_getenv ("HOME");
        if (homedir == NULL)
            homedir = g_get_home_dir ();
        path->path = g_build_filename (homedir, node->text + 2, NULL);
    }
    else {
        path->path = g_strdup (node->text);
    }
    
    gchar **attr;
    for (attr = node->attributes; attr[0]; attr += 2) {
        if (g_strcmp0 (*attr, "mtime") == 0) {
            path->mtime = atol (attr[1]);
            continue;
        }
        g_warning ("Unkonwn attribute %s", attr[0]);
    }
    
    return TRUE;
}

BusObservedPath *
bus_observed_path_new_from_xml_node (XMLNode *node,
                                     gboolean fill_stat)
{
    g_assert (node);
    
    BusObservedPath *path;

    path = (BusObservedPath *) g_object_new (BUS_TYPE_OBSERVED_PATH, NULL);

    if (!bus_observed_path_parse_xml_node (path, node)) {
        g_object_unref (path);
        path = NULL;
    }
    else if (fill_stat) {
        bus_observed_path_fill_stat (path);
    }

    return path;
}

BusObservedPath *
bus_observed_path_new_from_path (const gchar *path)
{
    g_assert (path);
    
    BusObservedPath *op;
    
    op = (BusObservedPath *) g_object_new (BUS_TYPE_OBSERVED_PATH, NULL);
    op->path = g_strdup (path);
    bus_observed_path_fill_stat (op);

    return op;
}

