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
#include "component.h"

enum {
    LAST_SIGNAL,
};


/* BusComponentPriv */
struct _BusComponentPrivate {
    gpointer pad;
};
typedef struct _BusComponentPrivate BusComponentPrivate;

#define BUS_COMPONENT_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_COMPONENT, BusComponentPrivate))

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void              bus_component_class_init        (BusComponentClass       *klass);
static void              bus_component_init              (BusComponent            *component);
static void              bus_component_destroy           (BusComponent            *component);
static gboolean          bus_component_parse_xml_node   (BusComponent           *component,
                                                         XMLNode                *node,
                                                         gboolean                access_fs);

static void              bus_component_parse_engines    (BusComponent           *component,
                                                         XMLNode                *node);
static void              bus_component_parse_observed_paths
                                                        (BusComponent           *component,
                                                         XMLNode                *node,
                                                         gboolean                access_fs);

static IBusObjectClass  *parent_class = NULL;

GType
bus_component_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusComponentClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_component_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusComponent),
        0,
        (GInstanceInitFunc) bus_component_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "BusComponent",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}


static void
bus_component_class_init (BusComponentClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (BusComponentPrivate));

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_component_destroy;

}



static void
bus_component_init (BusComponent *component)
{
    component->name = NULL;
    component->description = NULL;
    component->exec = NULL;
    component->version = NULL;
    component->author = NULL;
    component->license = NULL;
    component->homepage = NULL;
    component->service_name = NULL;
    component->filename = NULL;
    component->textdomain = NULL;
    component->engines = NULL;
    component->observed_paths = NULL;
}

static void
bus_component_destroy (BusComponent *component)
{
    g_free (component->name);
    g_free (component->description);
    g_free (component->exec);
    g_free (component->version);
    g_free (component->author);
    g_free (component->license);
    g_free (component->homepage);
    g_free (component->service_name);
    g_free (component->filename);
    g_free (component->textdomain);

    GList *p;
    for (p = component->observed_paths; p != NULL; p = p->next) {
        g_object_unref (p->data);
    }
    g_list_free (component->observed_paths);

    for (p = component->engines; p != NULL; p = p->next) {
        g_object_unref (p->data);
    }
    g_list_free (component->engines);

    IBUS_OBJECT_CLASS (parent_class)->destroy (IBUS_OBJECT (component));
}

#define g_string_append_indent(string, indent)  \
    {                                           \
        gint i;                                 \
        for (i = 0; i < (indent); i++) {        \
            g_string_append (string, "    ");   \
        }                                       \
    }

void
bus_component_output (BusComponent *component,
                      GString      *output,
                      gint          indent)
{
    g_assert (BUS_IS_COMPONENT (component));
    GList *p;

    g_string_append_indent (output, indent);
    g_string_append (output, "<component>\n");

#define OUTPUT_ENTRY(field, element)                                        \
    {                                                                       \
        gchar *escape_text = g_markup_escape_text (component->field, -1);   \
        g_string_append_indent (output, indent + 1);                        \
        g_string_append_printf (output, "<"element">%s</"element">\n",      \
                                escape_text);                               \
        g_free (escape_text);                                               \
    }
#define OUTPUT_ENTRY_1(name) OUTPUT_ENTRY(name, #name)
    OUTPUT_ENTRY_1 (name);
    OUTPUT_ENTRY_1 (description);
    OUTPUT_ENTRY_1 (filename);
    g_string_append_indent (output, indent + 1);
    g_string_append_printf (output, "<mtime>%ld</mtime>\n", component->mtime);
    OUTPUT_ENTRY_1 (exec);
    OUTPUT_ENTRY_1 (version);
    OUTPUT_ENTRY_1 (author);
    OUTPUT_ENTRY_1 (license);
    OUTPUT_ENTRY_1 (homepage);
    OUTPUT_ENTRY_1 (textdomain);
    OUTPUT_ENTRY (service_name, "service-name");
#undef OUTPUT_ENTRY
#undef OUTPUT_ENTRY_1

    if (component->observed_paths) {
        g_string_append_indent (output, indent + 1);
        g_string_append (output, "<observed-paths>\n");
        
        for (p = component->observed_paths; p != NULL; p = p->next ) {
            BusObservedPath *path = (BusObservedPath *) p->data;

            g_string_append_indent (output, indent + 2);
            g_string_append_printf (output, "<path mtime=\"%ld\" >%s</path>\n",
                                    path->mtime,
                                    path->path);
        }

        g_string_append_indent (output, indent + 1);
        g_string_append (output, "</observed-paths>\n");
    }

    if (component->engines) {
        g_string_append_indent (output, indent + 1);
        g_string_append (output, "<engines>\n");

        for (p = component->engines; p != NULL; p = p->next) {
            bus_engine_info_output ((BusEngineInfo *)p->data, output, indent + 2);
        }

        g_string_append_indent (output, indent + 1);
        g_string_append (output, "</engines>\n");
    }


    g_string_append_indent (output, indent);
    g_string_append (output, "</component>\n");
}

static gboolean
bus_component_parse_xml_node (BusComponent   *component,
                              XMLNode        *node,
                              gboolean        access_fs)
{
    g_assert (component);
    g_assert (node);

    if (G_UNLIKELY (g_strcmp0 (node->name, "component") != 0)) {
        return FALSE;
    }
    
    GList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        XMLNode *sub_node = (XMLNode *)p->data;

#define PARSE_ENTRY(field_name, element_name)                   \
        if (g_strcmp0 (sub_node->name, element_name) == 0) {    \
            if (component->field_name != NULL) {                \
                g_free (component->field_name);                 \
            }                                                   \
            component->field_name = g_strdup (sub_node->text);  \
            continue;                                           \
        }
#define PARSE_ENTRY_1(name) PARSE_ENTRY (name, #name)
        PARSE_ENTRY_1 (name);
        PARSE_ENTRY_1 (description);
        PARSE_ENTRY_1 (filename);
        PARSE_ENTRY_1 (exec);
        PARSE_ENTRY_1 (version);
        PARSE_ENTRY_1 (author);
        PARSE_ENTRY_1 (license);
        PARSE_ENTRY_1 (homepage);
        PARSE_ENTRY_1 (textdomain);
        PARSE_ENTRY (service_name, "service-name");
#undef PARSE_ENTRY
#undef PARSE_ENTRY_1

        if (g_strcmp0 (sub_node->name, "mtime") == 0) {
            component->mtime = atol (sub_node->text);
            continue;
        }

        if (g_strcmp0 (sub_node->name, "engines") == 0) {
            bus_component_parse_engines (component, sub_node);
            continue;
        }

        if (g_strcmp0 (sub_node->name, "observed-paths") == 0) {
            bus_component_parse_observed_paths (component, sub_node, access_fs);
            continue;
        }

        g_warning ("<component> element contains invalidate element <%s>", sub_node->name);
    }

    return TRUE;
}


static void
bus_component_parse_engines (BusComponent *component,
                             XMLNode      *node)
{
    g_assert (BUS_IS_COMPONENT (component));
    g_assert (node);

    if (g_strcmp0 (node->name, "engines") != 0) {
        return;
    }

    GList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        BusEngineInfo *engine;
        engine = bus_engine_info_new_from_xml_node (component, (XMLNode *)p->data);

        if (G_UNLIKELY (engine == NULL))
            continue;
        component->engines = g_list_append(component->engines, engine);
    }
}

static void
bus_component_parse_observed_paths (BusComponent    *component,
                                    XMLNode         *node,
                                    gboolean         access_fs)
{
    g_assert (BUS_IS_COMPONENT (component));
    g_assert (node);

    if (g_strcmp0 (node->name, "observed-paths") != 0) {
        return;
    }

    GList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        BusObservedPath *path;
        
        path = bus_observed_path_new_from_xml_node ((XMLNode *)p->data, access_fs);
        component->observed_paths = g_list_append (component->observed_paths, path);

        if (access_fs && path->is_dir && path->is_exist) {
            component->observed_paths = g_list_concat (component->observed_paths,
                                            bus_observed_path_traverse (path));
        }
    }
}

BusComponent *
bus_component_new_from_xml_node (XMLNode  *node)
{
    g_assert (node);

    BusComponent *component;

    component = (BusComponent *)g_object_new (BUS_TYPE_COMPONENT, NULL);
    if (!bus_component_parse_xml_node (component, node, FALSE)) {
        g_object_unref (component);
        component = NULL;
    }

    return component;
}

BusComponent *
bus_component_new_from_file (const gchar *filename)
{
    g_assert (filename);
    
    XMLNode *node;
    struct stat buf;
    BusComponent *component;
    gboolean retval;
    
    if (g_stat (filename, &buf) != 0) {
        g_warning ("Can not get stat of file %s", filename);
        return NULL;
    }

    node = xml_parse_file (filename);

    if (!node) {
        return NULL;
    }

    component = (BusComponent *)g_object_new (BUS_TYPE_COMPONENT, NULL);
    retval = bus_component_parse_xml_node (component, node, TRUE);
    xml_free_node (node);

    if (!retval) {
        g_object_unref (component);
        component = NULL;
    }

    component->filename = g_strdup (filename);
    component->mtime = buf.st_mtime;

    return component;
}

static void
bus_component_child_cb (GPid          pid,
           gint          status,
           BusComponent  *component)
{
    g_assert (BUS_IS_COMPONENT (component));
    g_assert (component->pid == pid);
    
    g_spawn_close_pid (pid);
    component->pid = 0;
}

gboolean
bus_component_start (BusComponent *component)
{
    g_assert (BUS_IS_COMPONENT (component));

    if (component->pid != 0)
        return TRUE;
    
    gint argc;
    gchar **argv;
    gboolean retval;
    GError *error;

    if (!g_shell_parse_argv (component->exec, &argc, &argv, &error)) {
        g_warning ("Can not parse component %s exec: %s", component->name, error->message);
        g_error_free (error);
        return FALSE;
    }

    retval = g_spawn_async (NULL, argv, NULL,
                            G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL,
                            &(component->pid), &error);
    g_strfreev (argv)
    ;
    if (!retval) {
        g_warning ("Can not execute component %s: %s", component->name, error->message);
        g_error_free (error);
        return FALSE;
    }

    g_child_watch_add (component->pid, (GChildWatchFunc) bus_component_child_cb, component); 

    return TRUE;
}

gboolean
bus_component_stop (BusComponent *component)
{
    g_assert (BUS_IS_COMPONENT (component));
    
    if (component->pid == 0)
        return TRUE;

    kill (component->pid, SIGTERM);
    return TRUE;
}

gboolean
bus_component_is_running (BusComponent *component)
{
    g_assert (BUS_IS_COMPONENT (component));

    return (component->pid != 0);
}


gboolean
bus_component_check_modification (BusComponent *component)
{
    g_assert (BUS_IS_COMPONENT (component));

    GList *p;

    for (p = component->observed_paths; p != NULL; p = p->next) {
        if (bus_observed_path_check_modification ((BusObservedPath *)p->data))
            return TRUE;
    }
    return FALSE;
}
