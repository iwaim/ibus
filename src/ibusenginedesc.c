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
#include "ibusenginedesc.h"
#include "ibusxml.h"

enum {
    LAST_SIGNAL,
};


/* IBusEngineDescPriv */
struct _IBusEngineDescPrivate {
    gpointer pad;
};
typedef struct _IBusEngineDescPrivate IBusEngineDescPrivate;

#define IBUS_ENGINE_DESC_GET_PRIVATE(o)  \
   (G_TYPE_INSTANCE_GET_PRIVATE ((o), BUS_TYPE_ENGINE_INFO, IBusEngineDescPrivate))

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void         ibus_engine_desc_class_init      (IBusEngineDescClass     *klass);
static void         ibus_engine_desc_init            (IBusEngineDesc          *desc);
static void         ibus_engine_desc_destroy         (IBusEngineDesc          *desc);
static gboolean     ibus_engine_desc_parse_xml_node  (IBusEngineDesc          *desc,
                                                     XMLNode                *node);

static IBusObjectClass  *parent_class = NULL;

GType
ibus_engine_desc_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusEngineDescClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_engine_desc_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (IBusEngineDesc),
        0,
        (GInstanceInitFunc) ibus_engine_desc_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_OBJECT,
                    "IBusEngineDesc",
                    &type_info,
                    (GTypeFlags)0);
    }

    return type;
}


static void
ibus_engine_desc_class_init (IBusEngineDescClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusObjectClass *) g_type_class_peek_parent (klass);

    // g_type_class_add_private (klass, sizeof (IBusEngineDescPrivate));
    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_engine_desc_destroy;

}

static void
ibus_engine_desc_init (IBusEngineDesc *desc)
{

    desc->name = NULL;
    desc->longname = NULL;
    desc->description = NULL;
    desc->language = NULL;
    desc->license = NULL;
    desc->author = NULL;
    desc->icon = NULL;
    desc->layout = NULL;
}

static void
ibus_engine_desc_destroy (IBusEngineDesc *desc)
{
    g_free (desc->name);
    g_free (desc->longname);
    g_free (desc->description);
    g_free (desc->language);
    g_free (desc->license);
    g_free (desc->author);
    g_free (desc->icon);
    g_free (desc->layout);

    g_object_unref (desc->component);

    IBUS_OBJECT_CLASS (parent_class)->destroy (IBUS_OBJECT (desc));
}

#define g_string_append_indent(string, indent)  \
    {                                           \
        gint i;                                 \
        for (i = 0; i < (indent); i++) {        \
            g_string_append (string, "    ");   \
        }                                       \
    }

void
ibus_engine_desc_output (IBusEngineDesc *desc,
                        GString       *output,
                        gint           indent)
{
    g_string_append_indent (output, indent);
    g_string_append (output, "<engine>\n");
#define OUTPUT_ENTRY(field, element)                                    \
    {                                                                   \
        gchar *escape_text = g_markup_escape_text (desc->field, -1);    \
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
ibus_engine_desc_parse_xml_node (IBusEngineDesc *desc,
                                XMLNode       *node)
{
    GList *p;
    for (p = node->sub_nodes; p != NULL; p = p->next) {
        XMLNode *sub_node = (XMLNode *) p->data;

#define PARSE_ENTRY(field_name, element_name)                   \
        if (g_strcmp0 (sub_node->name, element_name) == 0) {    \
            if (desc->field_name != NULL) {                     \
                g_free (desc->field_name);                      \
            }                                                   \
            desc->field_name = g_strdup (sub_node->text);       \
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

IBusEngineDesc *
ibus_engine_desc_new_from_xml_node (BusComponent *component,
                                   XMLNode      *node)
{
    g_assert (component);
    g_assert (node);

    IBusEngineDesc *desc;

    if (G_UNLIKELY (g_strcmp0 (node->name, "engine") != 0)) {
        return NULL;
    }

    desc = (IBusEngineDesc *)g_object_new (IBUS_TYPE_ENGINE_DESC, NULL);
    
    if (!ibus_engine_desc_parse_xml_node (desc, node)) {
        g_object_unref (desc);
        desc = NULL;
    }
    else {
        desc->component = component;
        g_object_ref (component);
    }
    
    return desc;
}

