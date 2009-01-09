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
#include "xmlparser.h"
#include "registry.h"

enum {
    LAST_SIGNAL,
};


/* BusRegistryPriv */
struct _BusRegistryPrivate {
    IBusComponent **components;
};
typedef struct _BusRegistryPrivate BusRegistryPrivate;

#define BUS_REGISTRY_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_REGISTRY, BusRegistryPrivate))

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void              bus_registry_class_init        (BusRegistryClass       *klass);
static void              bus_registry_init              (BusRegistry            *registry);
static void              bus_registry_destroy           (BusRegistry            *registry);
static void              bus_registry_load              (BusRegistry            *registry);
static void              bus_registry_load_in_dir       (GArray                 *components,
                                                         const gchar            *dirname);
static void              bus_registry_load_component
                                                        (GArray                 *components,
                                                         const gchar            *filename);
static IBusComponent    *bus_registry_parse_component   (XMLNode                *node);
static void              bus_registry_parse_engines     (XMLNode                *node,
                                                         GArray                 *engines);
static IBusEngineInfo   *bus_registry_parse_engine      (XMLNode                *node);
static void              bus_registry_parse_observed_paths
                                                        (XMLNode                *node,
                                                         GArray                 *paths);
static void              bus_registry_parse_path        (XMLNode                *node,
                                                         GArray                 *paths);
static void              bus_registry_traverse_dir      (const gchar            *dirname,
                                                         GArray                 *paths);
static IBusObservedPath *bus_registry_get_path_info     (const gchar            *dirname);
static gboolean          bus_registry_save_cache        (BusRegistry            *registry);
static gboolean          bus_registry_load_cache        (BusRegistry            *registry);

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
    GArray *components;

    components = g_array_new (TRUE, TRUE, sizeof (IBusComponent *));

    dirname = g_build_filename (PKGDATADIR, "component", NULL);
    bus_registry_load_in_dir (components, dirname);
    g_free (dirname);

    homedir = (gchar *) g_getenv ("HOME");
    if (!homedir)
        homedir = (gchar *) g_get_home_dir ();
    dirname = g_build_filename (homedir, ".ibus", "component", NULL);
    bus_registry_load_in_dir (components, dirname);
    g_free (dirname);

    priv->components = (IBusComponent **) g_array_free (components, FALSE);

    bus_registry_save_cache (registry);
}


static gboolean
bus_registry_load_cache (BusRegistry *registry)
{
    return TRUE;
}

#define g_string_append_indent(string, indent)  \
    {                                           \
        gint i;                                 \
        for (i = 0; i < (indent); i++) {        \
            g_string_append (string, "    ");   \
        }                                       \
    }

static void
bus_registry_save_engine (IBusEngineInfo   *engine,
                          GString          *output,
                          gint              indent_level)
{
    g_string_append_indent (output, indent_level);
    g_string_append (output, "<engine>\n");
#define OUTPUT_ENTRY(field, element)                                            \
    g_string_append_indent (output, indent_level + 1);                          \
    g_string_append_printf (output, "<"element">%s</"element">\n", engine->field);
#define OUTPUT_ENTRY_1(name) OUTPUT_ENTRY(name, #name)
    OUTPUT_ENTRY_1(name);
    OUTPUT_ENTRY_1(longname);
    OUTPUT_ENTRY_1(description);
    OUTPUT_ENTRY_1(language);
    OUTPUT_ENTRY_1(license);
    OUTPUT_ENTRY_1(author);
    OUTPUT_ENTRY_1(icon);
    OUTPUT_ENTRY_1(layout);
#undef OUTPUT_ENTRY
#undef OUTPUT_ENTRY_1
    g_string_append_indent (output, indent_level);
    g_string_append (output, "</engine>\n");
}

static void
bus_registry_save_component (IBusComponent *comp,
                             GString       *output,
                             gint           indent_level)
{
    g_string_append_indent (output, indent_level);
    g_string_append (output, "<component>\n");

#define OUTPUT_ENTRY(field, element)                                            \
    g_string_append_indent (output, indent_level + 1);                          \
    g_string_append_printf (output, "<"element">%s</"element">\n", comp->field);
#define OUTPUT_ENTRY_1(name) OUTPUT_ENTRY(name, #name)
    OUTPUT_ENTRY_1 (name);
    OUTPUT_ENTRY_1 (description);
    OUTPUT_ENTRY_1 (filename);
    g_string_append_indent (output, indent_level + 1);
    g_string_append_printf (output, "<mtime>%ld</mtime>\n", comp->mtime);
    OUTPUT_ENTRY_1 (exec);
    OUTPUT_ENTRY_1 (version);
    OUTPUT_ENTRY_1 (author);
    OUTPUT_ENTRY_1 (license);
    OUTPUT_ENTRY_1 (homepage);
    OUTPUT_ENTRY_1 (textdomain);
    OUTPUT_ENTRY (service_name, "service-name");
#undef OUTPUT_ENTRY
#undef OUTPUT_ENTRY_1

    if (comp->observed_paths && comp->observed_paths[0]) {
        gint i;

        g_string_append_indent (output, indent_level + 1);
        g_string_append (output, "<observed-paths>\n");

        for (i = 0; comp->observed_paths[i]; i++) {
            g_string_append_indent (output, indent_level + 2);
            g_string_append_printf (output, "<path mtime=%ld>%s</path>\n",
                                    comp->observed_paths[i]->mtime,
                                    comp->observed_paths[i]->path);
        }

        g_string_append_indent (output, indent_level + 1);
        g_string_append (output, "</observed-paths>\n");
    }

    if (comp->engines && comp->engines[0]) {
        gint i;

        g_string_append_indent (output, indent_level + 1);
        g_string_append (output, "<engines>\n");

        for (i = 0; comp->engines[i]; i++) {
            bus_registry_save_engine (comp->engines[i], output, indent_level + 2);
        }

        g_string_append_indent (output, indent_level + 1);
        g_string_append (output, "</engines>\n");
    }


    g_string_append_indent (output, indent_level);
    g_string_append (output, "</component>\n");
}

static gboolean
bus_registry_save_cache (BusRegistry *registry)
{
    gchar *cachedir;
    gchar *filename;
    GString *output;
    FILE *pf;

    BusRegistryPrivate *priv = BUS_REGISTRY_GET_PRIVATE (registry);

    cachedir = g_build_filename (g_get_user_cache_dir(), "ibus", NULL);
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
    g_string_append (output, "<ibus>\n");
    gint i;
    for (i = 0; priv->components && priv->components[i] != NULL; i++) {
        bus_registry_save_component (priv->components[i], output, 1);
    }
    g_string_append (output, "</ibus>");

    g_debug ("%s", output->str);
    fwrite (output->str, output->len, 1, pf);
    fclose (pf);

    return TRUE;
}

static void
bus_registry_load_in_dir (GArray      *components,
                          const gchar *dirname)
{
    g_assert (dirname != NULL);

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
        glong size = g_utf8_strlen (filename, -1);
        if (g_strcmp0 (MAX (filename, filename + size -4), ".xml" ) != 0)
            continue;
        gchar *path = g_build_filename (dirname, filename, NULL);
        bus_registry_load_component (components, path);
        g_free (path);
    }

    g_dir_close (dir);
}



static void
bus_registry_load_component (GArray             *components,
                             const gchar        *filename)
{
    IBusComponent *comp;
    struct stat buf;
    XMLNode *node;

    g_assert (filename != NULL);

    if (g_stat (filename, &buf) != 0) {
        g_warning ("Can not get stat of file %s", filename);
        return;
    }

    node = xml_parse_file (filename);

#if 0
    GString *string = g_string_new ("");

    xml_output (node, string);

    g_debug ("%s", string->str);
    g_string_free (string, TRUE);
#endif

    if (node == NULL) {
        g_warning ("parse %s failed", filename);
        return;
    }

    comp = bus_registry_parse_component (node);
    if (comp != NULL) {
        comp->filename = g_strdup (filename);
        comp->mtime = buf.st_mtime;
        g_array_append_val (components, comp);
    }

    xml_free_node (node);
}

static IBusComponent *
bus_registry_parse_component (XMLNode *node)
{
    g_assert (node != NULL);

    IBusComponent *comp;

    if (G_UNLIKELY (g_strcmp0 (node->name, "component") != 0)) {
        g_warning ("The root element must be <component>, but it is <%s>", node->name);
        return NULL;
    }

    comp = g_slice_new0 (IBusComponent);
    GArray *engines = g_array_new (TRUE, TRUE, sizeof (IBusEngineInfo *));
    GArray *paths = g_array_new (TRUE, TRUE, sizeof (IBusObservedPath *));
    GSList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        XMLNode *sub_node = (XMLNode *)p->data;

#define PARSE_ENTRY(field_name, element_name)                   \
        if (g_strcmp0 (sub_node->name, element_name) == 0) {    \
            if (comp->field_name != NULL) {                     \
                g_free (comp->field_name);                      \
            }                                                   \
            comp->field_name = g_strdup (sub_node->text);       \
            continue;                                           \
        }
#define PARSE_ENTRY_1(name) PARSE_ENTRY (name, #name)
        PARSE_ENTRY_1 (name);
        PARSE_ENTRY_1 (description);
        PARSE_ENTRY_1 (exec);
        PARSE_ENTRY_1 (version);
        PARSE_ENTRY_1 (author);
        PARSE_ENTRY_1 (license);
        PARSE_ENTRY_1 (homepage);
        PARSE_ENTRY_1 (textdomain);
        PARSE_ENTRY (service_name, "service-name");
#undef PARSE_ENTRY
#undef PARSE_ENTRY_1

        if (g_strcmp0 (sub_node->name, "engines") == 0) {
            bus_registry_parse_engines (sub_node, engines);
            continue;
        }

        if (g_strcmp0 (sub_node->name, "observed-paths") == 0) {
            bus_registry_parse_observed_paths (sub_node, paths);
            continue;
        }

        g_warning ("<component> element contains invalidate element <%s>", sub_node->name);
    }

    comp->engines = (IBusEngineInfo **)g_array_free (engines, FALSE);
    comp->observed_paths = (IBusObservedPath **)g_array_free (paths, FALSE);

    return comp;
}


static void
bus_registry_parse_engines (XMLNode *node,
                            GArray  *engines)
{
    g_assert (node != NULL);
    g_assert (engines != NULL);

    if (g_strcmp0 (node->name, "engines") != 0) {
        return;
    }

    GSList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        IBusEngineInfo *engine;
        engine = bus_registry_parse_engine ((XMLNode *)p->data);

        if (G_UNLIKELY (engine == NULL))
            continue;
        g_array_append_val (engines, engine);
    }
}

static IBusEngineInfo *
bus_registry_parse_engine (XMLNode *node)
{
    IBusEngineInfo *engine;

    g_assert (node != NULL);

    if (G_UNLIKELY (g_strcmp0 (node->name, "engine") != 0)) {
        g_warning ("engines element contains invalidate sub element <%s> ", node->name);
        return NULL;
    }

    engine = g_slice_new0 (IBusEngineInfo);

    GSList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        XMLNode *sub_node = (XMLNode *) p->data;

#define PARSE_ENTRY(field_name, element_name)                   \
        if (g_strcmp0 (sub_node->name, element_name) == 0) {    \
            if (engine->field_name != NULL) {                   \
                g_free (engine->field_name);                    \
            }                                                   \
            engine->field_name = g_strdup (sub_node->text);     \
            continue;                                           \
        }
#define PARSE_ENTRY_1(name) PARSE_ENTRY(name, #name)
        PARSE_ENTRY_1(name);
        PARSE_ENTRY_1(longname);
        PARSE_ENTRY_1(description);
        PARSE_ENTRY_1(language);
        PARSE_ENTRY_1(license);
        PARSE_ENTRY_1(author);
        PARSE_ENTRY_1(icon);
        PARSE_ENTRY_1(layout);
#undef PARSE_ENTRY
#undef PARSE_ENTRY1
        g_warning ("<engines> element contains invalidate element <%s>", sub_node->name);
    }
    return engine;
}

static void
print_paths (GArray *paths)
{
    GString *output = g_string_new ("");

    IBusObservedPath *path;
    gint i;
    for (i = 0;; i++) {
        path = g_array_index (paths, IBusObservedPath *, i);
        if (path == NULL)
            break;
        g_string_append_printf (output, "<path mtime=%ld>%s</path>\n", path->mtime, path->path);
    }

    g_debug (output->str);
    g_string_free (output, TRUE);
}

static void
bus_registry_parse_observed_paths (XMLNode *node,
                                   GArray  *paths)
{
    g_assert (node != NULL);
    g_assert (paths != NULL);

    if (g_strcmp0 (node->name, "observed-paths") != 0) {
        return;
    }

    GSList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        bus_registry_parse_path ((XMLNode *)p->data, paths);
    }

    print_paths (paths);
}


static void
bus_registry_parse_path (XMLNode *node,
                         GArray  *paths)
{
    g_assert (node != NULL);
    g_assert (paths != NULL);

    if (g_strcmp0 (node->name, "path") != 0) {
        return;
    }

    if (node->text == NULL || g_utf8_strlen (node->text, -1) <= 0) {
        g_warning ("path must not be empty");
        return;
    }

    if (node->text[0] == '~' && node->text[1] != G_DIR_SEPARATOR) {
        g_warning ("invalide path \"%s\"", node->text);
        return;
    }

    IBusObservedPath *path;
    if (node->text[0] == '~') {
        gchar *extend_path;
        const gchar *homedir = g_getenv ("HOME");
        if (homedir == NULL)
            homedir = g_get_home_dir ();
        extend_path = g_build_filename (homedir, node->text + 2, NULL);
        path = bus_registry_get_path_info (extend_path);
        g_free (extend_path);
    }
    else {
        path = bus_registry_get_path_info (node->text);
    }

    g_array_append_val (paths, path);

    if (path->is_exist && path->is_dir) {
        bus_registry_traverse_dir (path->path, paths); 
    }
}

static IBusObservedPath *
bus_registry_get_path_info (const gchar *dirname)
{
    g_assert (dirname);

    IBusObservedPath *path = g_slice_new0 (IBusObservedPath);

    path->path = g_strdup (dirname);

    struct stat buf;
    if (g_stat (path->path, &buf) == 0) {
        path->is_exist = 1;
        if (S_ISDIR (buf.st_mode)) {
            path->is_dir = 1;
        }
        path->mtime = buf.st_mtime;
    }
    return path;
}

static void
bus_registry_traverse_dir (const gchar *dirname,
                           GArray      *paths)
{
    g_assert (dirname != NULL);
    g_assert (paths != NULL);

    GDir *dir;
    const gchar *name;

    dir = g_dir_open (dirname, 0, NULL);

    if (dir == NULL)
        return;


    while ((name = g_dir_read_name (dir)) != NULL) {
        gchar *path = g_build_filename (dirname, name, NULL);
        IBusObservedPath *p = bus_registry_get_path_info (path);
        g_array_append_val (paths, p);
        if (p->is_exist && p->is_dir)
            bus_registry_traverse_dir (path, paths);
        g_free (path);
    }

    g_dir_close (dir);
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
