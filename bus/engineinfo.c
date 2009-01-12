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
#include "engineinfo.h"

enum {
    LAST_SIGNAL,
};


/* BusEngineInfoPriv */
struct _BusEngineInfoPrivate {
    gpointer pad;
};
typedef struct _BusEngineInfoPrivate BusEngineInfoPrivate;

#define BUS_ENGINE_INFO_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_ENGINE_INFO, BusEngineInfoPrivate))

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void         bus_engine_info_class_init      (BusEngineInfoClass     *klass);
static void         bus_engine_info_init            (BusEngineInfo          *info);
static void         bus_engine_info_destroy         (BusEngineInfo          *info);
static gboolean     bus_engine_info_parse_xml_node  (BusEngineInfo          *info,
                                                     XMLNode                *node);

static IBusObjectClass  *parent_class = NULL;

GType
bus_engine_info_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusEngineInfoClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_engine_info_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusEngineInfo),
        0,
        (GInstanceInitFunc) bus_engine_info_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "BusEngineInfo",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}


static void
bus_engine_info_class_init (BusEngineInfoClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    // g_type_class_add_private (klass, sizeof (BusEngineInfoPrivate));
    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_engine_info_destroy;

}

static void
bus_engine_info_init (BusEngineInfo *info)
{

    info->name = NULL;
    info->longname = NULL;
    info->description = NULL;
    info->language = NULL;
    info->license = NULL;
    info->author = NULL;
    info->icon = NULL;
    info->layout = NULL;
}

static void
bus_engine_info_destroy (BusEngineInfo *info)
{
    g_free (info->name);
    g_free (info->longname);
    g_free (info->description);
    g_free (info->language);
    g_free (info->license);
    g_free (info->author);
    g_free (info->icon);
    g_free (info->layout);

    g_object_unref (info->component);

    IBUS_OBJECT_CLASS (parent_class)->destroy (IBUS_OBJECT (info));
}

#define g_string_append_indent(string, indent)  \
    {                                           \
        gint i;                                 \
        for (i = 0; i < (indent); i++) {        \
            g_string_append (string, "    ");   \
        }                                       \
    }

void
bus_engine_info_output (BusEngineInfo *info,
                        GString       *output,
                        gint           indent)
{
    g_string_append_indent (output, indent);
    g_string_append (output, "<engine>\n");
#define OUTPUT_ENTRY(field, element)                                    \
    {                                                                   \
        gchar *escape_text = g_markup_escape_text (info->field, -1);    \
        g_string_append_indent (output, indent + 1);                    \
        g_string_append_printf (output, "<"element">%s</"element">\n",  \
                                escape_text);                           \
        g_free (escape_text);                                           \
    }
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
    g_string_append_indent (output, indent);
    g_string_append (output, "</engine>\n");
}

static gboolean
bus_engine_info_parse_xml_node (BusEngineInfo *info,
                                XMLNode       *node)
{
    GSList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        XMLNode *sub_node = (XMLNode *) p->data;

#define PARSE_ENTRY(field_name, element_name)                   \
        if (g_strcmp0 (sub_node->name, element_name) == 0) {    \
            if (info->field_name != NULL) {                     \
                g_free (info->field_name);                      \
            }                                                   \
            info->field_name = g_strdup (sub_node->text);       \
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
    return TRUE;
}

BusEngineInfo *
bus_engine_info_new_from_xml_node (BusComponent *component,
                                   XMLNode      *node)
{
    g_assert (component);
    g_assert (node);

    BusEngineInfo *info;

    if (G_UNLIKELY (g_strcmp0 (node->name, "engine") != 0)) {
        return NULL;
    }

    info = (BusEngineInfo *)g_object_new (BUS_TYPE_ENGINE_INFO, NULL);
    
    if (!bus_engine_info_parse_xml_node (info, node)) {
        g_object_unref (info);
        info = NULL;
    }
    else {
        info->component = component;
        g_object_ref (component);
    }
    
    return info;
}

