/* vim:set et sts=4: */
/* ibus - The Input Bus
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
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <ibus.h>
#include "ibusimcontext.h"

/* IBusIMContextPriv */
struct _IBusIMContextPrivate {
    GtkIMContext *slave;
    GdkWindow *client_window;

    /* enabled */
    gboolean        enable;
    IBusInputContext *ibus_context;

    /* preedit status */
    gchar           *preedit_string;
    PangoAttrList   *preedit_attrs;
    gint             preedit_cursor_pos;
    gboolean         preedit_visible;

    GdkRectangle     cursor_area;
    gboolean         has_focus;

    gint             caps;
};

static guint    _signal_commit_id = 0;
static guint    _signal_preedit_changed_id = 0;
static guint    _signal_preedit_start_id = 0;
static guint    _signal_preedit_end_id = 0;
static guint    _signal_delete_surrounding_id = 0;
static guint    _signal_retrieve_surrounding_id = 0;

/* functions prototype */
static void     ibus_im_context_class_init   (IBusIMContextClass    *klass);
static void     ibus_im_context_init         (GObject               *obj);
static void     ibus_im_context_finalize     (GObject               *obj);
static void     ibus_im_context_reset        (GtkIMContext          *context);
static gboolean ibus_im_context_filter_keypress
                                            (GtkIMContext           *context,
                                             GdkEventKey            *key);
static void     ibus_im_context_focus_in     (GtkIMContext          *context);
static void     ibus_im_context_focus_out    (GtkIMContext          *context);
static void     ibus_im_context_get_preedit_string
                                            (GtkIMContext           *context,
                                             gchar                  **str,
                                             PangoAttrList          **attrs,
                                             gint                   *cursor_pos);
static void     ibus_im_context_set_client_window
                                            (GtkIMContext           *context,
                                             GdkWindow              *client);
static void     ibus_im_context_set_cursor_location
                                            (GtkIMContext           *context,
                                             GdkRectangle           *area);
static void     ibus_im_context_set_use_preedit
                                            (GtkIMContext           *context,
                                             gboolean               use_preedit);

/* static methods*/
static void     _create_input_context       (IBusIMContext      *context);
static void     _set_cursor_location_internal
                                            (GtkIMContext       *context);

static void     _bus_connected_cb           (IBusBus            *bus,
                                             IBusIMContext      *context);
/* callback functions for slave context */
static void     _slave_commit_cb            (GtkIMContext       *slave,
                                             gchar              *string,
                                             IBusIMContext       *context);
static void     _slave_preedit_changed_cb   (GtkIMContext       *slave,
                                             IBusIMContext       *context);
static void     _slave_preedit_start_cb     (GtkIMContext       *slave,
                                             IBusIMContext       *context);
static void     _slave_preedit_end_cb       (GtkIMContext       *slave,
                                             IBusIMContext       *context);
static void     _slave_retrieve_surrounding_cb
                                            (GtkIMContext       *slave,
                                             IBusIMContext       *context);
static void     _slave_delete_surrounding_cb
                                            (GtkIMContext       *slave,
                                             gint               arg1,
                                             gint               arg2,
                                             IBusIMContext       *context);



static GType                _ibus_type_im_context = 0;
static GtkIMContextClass    *parent_class = NULL;

static IBusBus              *_bus = NULL;

void
ibus_im_context_register_type (GTypeModule *type_module)
{
    static const GTypeInfo ibus_im_context_info = {
        sizeof (IBusIMContextClass),
        (GBaseInitFunc)        NULL,
        (GBaseFinalizeFunc)     NULL,
        (GClassInitFunc)     ibus_im_context_class_init,
        NULL,            /* class finialize */
        NULL,            /* class data */
        sizeof (IBusIMContext),
        0,
        (GInstanceInitFunc)    ibus_im_context_init,
    };

    if (! _ibus_type_im_context ) {
        if (type_module) {
            _ibus_type_im_context =
                g_type_module_register_type (type_module,
                    GTK_TYPE_IM_CONTEXT,
                    "IBusIMContext",
                    &ibus_im_context_info,
                    (GTypeFlags)0);
        }
        else {
            _ibus_type_im_context =
                g_type_register_static (GTK_TYPE_IM_CONTEXT,
                    "IBusIMContext",
                    &ibus_im_context_info,
                    (GTypeFlags)0);
        }
    }
}

GType
ibus_im_context_get_type (void)
{
    if (_ibus_type_im_context == 0) {
        ibus_im_context_register_type (NULL);
    }

    g_assert (_ibus_type_im_context != 0);
    return _ibus_type_im_context;
}

IBusIMContext *
ibus_im_context_new (void)
{
    IBusIMContext *obj;
    obj = IBUS_IM_CONTEXT(g_object_new (IBUS_TYPE_IM_CONTEXT, NULL));

    return obj;
}

static void
ibus_im_context_class_init     (IBusIMContextClass *klass)
{
    GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (klass);
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

    parent_class = (GtkIMContextClass *) g_type_class_peek_parent (klass);

    g_type_class_add_private (klass, sizeof (IBusIMContextPrivate));

    im_context_class->reset = ibus_im_context_reset;
    im_context_class->focus_in = ibus_im_context_focus_in;
    im_context_class->focus_out = ibus_im_context_focus_out;
    im_context_class->filter_keypress = ibus_im_context_filter_keypress;
    im_context_class->get_preedit_string = ibus_im_context_get_preedit_string;
    im_context_class->set_client_window = ibus_im_context_set_client_window;
    im_context_class->set_cursor_location = ibus_im_context_set_cursor_location;
    im_context_class->set_use_preedit = ibus_im_context_set_use_preedit;
    gobject_class->finalize = ibus_im_context_finalize;

    _signal_commit_id =
        g_signal_lookup ("commit", G_TYPE_FROM_CLASS (klass));
    g_assert (_signal_commit_id != 0);

    _signal_preedit_changed_id =
        g_signal_lookup ("preedit-changed", G_TYPE_FROM_CLASS (klass));
    g_assert (_signal_preedit_changed_id != 0);

    _signal_preedit_start_id =
        g_signal_lookup ("preedit-start", G_TYPE_FROM_CLASS (klass));
    g_assert (_signal_preedit_start_id != 0);

    _signal_preedit_end_id =
        g_signal_lookup ("preedit-end", G_TYPE_FROM_CLASS (klass));
    g_assert (_signal_preedit_end_id != 0);

    _signal_delete_surrounding_id =
        g_signal_lookup ("delete-surrounding", G_TYPE_FROM_CLASS (klass));
    g_assert (_signal_delete_surrounding_id != 0);

    _signal_retrieve_surrounding_id =
        g_signal_lookup ("retrieve-surrounding", G_TYPE_FROM_CLASS (klass));
    g_assert (_signal_retrieve_surrounding_id != 0);

}

static void
ibus_im_context_init (GObject *obj)
{

    IBusIMContext *ibuscontext = IBUS_IM_CONTEXT (obj);
    IBusIMContextPrivate *priv = ibuscontext->priv =
        G_TYPE_INSTANCE_GET_PRIVATE (ibuscontext, IBUS_TYPE_IM_CONTEXT, IBusIMContextPrivate);

    priv->client_window = NULL;

    // Init ibus status
    priv->enable = FALSE;

    // Init preedit status
    priv->preedit_string = NULL;
    priv->preedit_attrs = NULL;
    priv->preedit_cursor_pos = 0;
    priv->preedit_visible = FALSE;

    // Init cursor area
    priv->cursor_area.x = -1;
    priv->cursor_area.y = -1;
    priv->cursor_area.width = 0;
    priv->cursor_area.height = 0;

    priv->ibus_context = NULL;
    priv->has_focus = FALSE;
    priv->caps = IBUS_CAP_PREEDIT_TEXT | IBUS_CAP_FOCUS;


    // Create slave im context
    priv->slave = gtk_im_context_simple_new ();
    g_signal_connect (priv->slave,
                      "commit",
                      G_CALLBACK (_slave_commit_cb),
                      ibuscontext);
    g_signal_connect (priv->slave,
                      "preedit-start",
                      G_CALLBACK (_slave_preedit_start_cb),
                      ibuscontext);
    g_signal_connect (priv->slave,
                      "preedit-end",
                      G_CALLBACK (_slave_preedit_end_cb),
                      ibuscontext);
    g_signal_connect (priv->slave,
                      "preedit-changed",
                      G_CALLBACK (_slave_preedit_changed_cb),
                      ibuscontext);
    g_signal_connect (priv->slave,
                      "retrieve-surrounding",
                      G_CALLBACK (_slave_retrieve_surrounding_cb),
                      ibuscontext);
    g_signal_connect (priv->slave,
                      "delete-surrounding",
                      G_CALLBACK (_slave_delete_surrounding_cb),
                      ibuscontext);

    /* init bus object */
    if (_bus == NULL)
        _bus = ibus_bus_new();

    if (ibus_bus_is_connected (_bus)) {
        _create_input_context (ibuscontext);
    }

    g_signal_connect (_bus, "connected", G_CALLBACK (_bus_connected_cb), obj);
}

static void
ibus_im_context_finalize (GObject *obj)
{
    g_return_if_fail (obj != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (obj));

    IBusIMContext *ibus = IBUS_IM_CONTEXT (obj);
    IBusIMContextPrivate *priv = ibus->priv;

    g_signal_handlers_disconnect_by_func (_bus, G_CALLBACK (_bus_connected_cb), obj);

    if (priv->ibus_context) {
        ibus_object_destroy ((IBusObject *)priv->ibus_context);
    }

    g_object_unref (priv->slave);

    if (priv->client_window) {
        g_object_unref (priv->client_window);
    }

    // release preedit
    if (priv->preedit_string) {
        g_free (priv->preedit_string);
    }
    if (priv->preedit_attrs) {
        pango_attr_list_unref (priv->preedit_attrs);
    }

    G_OBJECT_CLASS(parent_class)->finalize (obj);
}

static gboolean
ibus_im_context_filter_keypress (GtkIMContext *context,
                GdkEventKey  *event)
{
    g_return_val_if_fail (context != NULL, FALSE);
    g_return_val_if_fail (IBUS_IS_IM_CONTEXT (context), FALSE);

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    if (priv->ibus_context && priv->has_focus) {
        /* If context does not have focus, ibus will process key event in sync mode.
         * It is a workaround for increase search in treeview.
         */
        gboolean retval;
        switch (event->type) {
        case GDK_KEY_RELEASE:
            retval = ibus_input_context_process_key_event (priv->ibus_context,
                                                           event->keyval,
                                                           event->state | IBUS_RELEASE_MASK);
            break;
        case GDK_KEY_PRESS:
            retval = ibus_input_context_process_key_event (priv->ibus_context,
                                                           event->keyval,
                                                           event->state);
            break;
        default:
            retval = FALSE;
        }

        if (retval) {
            return TRUE;
        }
        return gtk_im_context_filter_keypress (priv->slave, event);
    }
    else {
        return gtk_im_context_filter_keypress (priv->slave, event);
    }
}

static void
ibus_im_context_focus_in (GtkIMContext *context)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));

    IBusIMContext *ibuscontext;
    IBusIMContextPrivate *priv;

    ibuscontext = IBUS_IM_CONTEXT (context);
    priv = ibuscontext->priv;

    priv->has_focus = TRUE;
    if (priv->ibus_context) {
        ibus_input_context_focus_in (priv->ibus_context);
    }

    gtk_im_context_focus_in (priv->slave);

    _set_cursor_location_internal (context);
}

static void
ibus_im_context_focus_out (GtkIMContext *context)
{

    g_assert (IBUS_IS_IM_CONTEXT (context));

    IBusIMContext *ibuscontext;
    IBusIMContextPrivate *priv;

    ibuscontext = IBUS_IM_CONTEXT (context);
    priv = ibuscontext->priv;

    priv->has_focus = FALSE;
    if (priv->ibus_context) {
        ibus_input_context_focus_out (priv->ibus_context);
    }
    gtk_im_context_focus_out (priv->slave);
}

static void
ibus_im_context_reset (GtkIMContext *context)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));

    IBusIMContext *ibuscontext;
    IBusIMContextPrivate *priv;

    ibuscontext = IBUS_IM_CONTEXT (context);
    priv = ibuscontext->priv;

    if (priv->ibus_context) {
        ibus_input_context_reset (priv->ibus_context);
    }
    gtk_im_context_reset (priv->slave);
}


static void
ibus_im_context_get_preedit_string (GtkIMContext   *context,
                                    gchar         **str,
                                    PangoAttrList **attrs,
                                    gint           *cursor_pos)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));

    IBusIMContext *ibuscontext;
    IBusIMContextPrivate *priv;

    ibuscontext = IBUS_IM_CONTEXT (context);
    priv = ibuscontext->priv;

    if (priv->enable) {
        if (priv->preedit_visible) {
            if (str) {
                *str = g_strdup (priv->preedit_string ? priv->preedit_string: "");
            }

            if (attrs) {
                *attrs = priv->preedit_attrs ?
                            pango_attr_list_ref (priv->preedit_attrs):
                            pango_attr_list_new ();
            }

            if (cursor_pos) {
                *cursor_pos = priv->preedit_cursor_pos;
            }
        }
        else {
            if (str) {
                *str = g_strdup ("");
            }
            if (attrs) {
                *attrs = pango_attr_list_new ();
            }
            if (cursor_pos) {
                *cursor_pos = 0;
            }
        }
    }
    else {
        gtk_im_context_get_preedit_string (priv->slave, str, attrs, cursor_pos);
    }
}


static void
ibus_im_context_set_client_window  (GtkIMContext *context, GdkWindow *client)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    if (priv->client_window) {
        g_object_unref (priv->client_window);
    }

    if (client) {
        g_object_ref (client);
    }

    priv->client_window = client;
    gtk_im_context_set_client_window (priv->slave, client);
}

static void
_set_cursor_location_internal (GtkIMContext *context)
{
    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;
    GdkRectangle area;
    gint x, y;

    if(priv->client_window == NULL || priv->ibus_context == NULL) {
        return;
    }

    area = priv->cursor_area;
    if (area.x == -1 && area.y == -1 && area.width == 0 && area.height == 0) {
        gint w, h;
        gdk_drawable_get_size (priv->client_window, &w, &h);
        area.y += h;
        area.x = 0;
    }

    gdk_window_get_origin (priv->client_window, &x, &y);
    area.x += x;
    area.y += y;
    ibus_input_context_set_cursor_location (priv->ibus_context,
                                            area.x,
                                            area.y,
                                            area.width,
                                            area.height);
}

static void
ibus_im_context_set_cursor_location (GtkIMContext *context, GdkRectangle *area)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    priv->cursor_area = *area;
    _set_cursor_location_internal (context);
    gtk_im_context_set_cursor_location (priv->slave, area);
}

static void
ibus_im_context_set_use_preedit (GtkIMContext *context, gboolean use_preedit)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    IBusIMContext *ibus = IBUS_IM_CONTEXT (context);
    IBusIMContextPrivate *priv = ibus->priv;

    if(priv->ibus_context) {
        if (use_preedit) {
            priv->caps |= IBUS_CAP_PREEDIT_TEXT;
        }
        else {
            priv->caps &= ~IBUS_CAP_PREEDIT_TEXT;
        }
        ibus_input_context_set_capabilities (priv->ibus_context, priv->caps);
    }
    gtk_im_context_set_use_preedit (priv->slave, use_preedit);
}

static void
_bus_connected_cb (IBusBus          *bus,
                   IBusIMContext    *context)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));
    g_assert (context->priv->ibus_context == NULL);

    _create_input_context (context);
}

static void
_ibus_context_commit_text_cb (IBusInputContext *ibus_context,
                              IBusText         *text,
                              IBusIMContext    *context)
{
    g_assert (IBUS_IS_INPUT_CONTEXT (ibus_context));
    g_assert (IBUS_IS_TEXT (text));
    g_assert (IBUS_IS_IM_CONTEXT (context));

    g_signal_emit (context, _signal_commit_id, 0, text->text);
}

static void
_ibus_context_forward_key_event_cb (IBusInputContext  *ibus_context,
                                    guint              keyval,
                                    gboolean           is_press,
                                    guint              state,
                                    IBusIMContext     *context)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));

    GdkEventKey *event;
    IBusIMContextPrivate *priv;

    priv = context->priv;
    event = (GdkEventKey *)gdk_event_new (is_press ? GDK_KEY_PRESS : GDK_KEY_RELEASE);

    event->time = GDK_CURRENT_TIME;
    event->window = g_object_ref (priv->client_window);
    event->send_event = FALSE;
    event->state = state;
    event->keyval = keyval;
    event->string = gdk_keyval_name (keyval);
    event->length = strlen (event->string);
    event->hardware_keycode = 0;
    event->group = 0;
    event->is_modifier = 0;

    gdk_event_put ((GdkEvent *)event);
    gdk_event_free ((GdkEvent *)event);
}

static void
_ibus_context_update_preedit_text_cb (IBusInputContext  *ibus_context,
                                      IBusText          *text,
                                      gint               cursor_pos,
                                      gboolean           visible,
                                      IBusIMContext     *context)
{
    g_assert (IBUS_IS_INPUT_CONTEXT (ibus_context));
    g_assert (IBUS_IS_TEXT (text));
    g_assert (IBUS_IS_IM_CONTEXT (context));

    IBusIMContextPrivate *priv;
    priv = context->priv;
    const gchar *str;

    if (priv->preedit_string) {
        g_free (priv->preedit_string);
    }
    if (priv->preedit_attrs) {
        pango_attr_list_unref (priv->preedit_attrs);
        priv->preedit_attrs = NULL;
    }

    str = text->text;
    priv->preedit_string = g_strdup (str);
    if (text->attrs) {
        guint i;
        priv->preedit_attrs = pango_attr_list_new ();
        for (i = 0; ; i++) {
            IBusAttribute *attr = ibus_attr_list_get (text->attrs, i);
            if (attr == NULL) {
                break;
            }

            PangoAttribute *pango_attr;
            switch (attr->type) {
            case IBUS_ATTR_TYPE_UNDERLINE:
                pango_attr = pango_attr_underline_new (attr->value);
                break;
            case IBUS_ATTR_TYPE_FOREGROUND:
                pango_attr = pango_attr_foreground_new (
                                        ((attr->value & 0xff0000) >> 8) | 0xff,
                                        ((attr->value & 0x00ff00)) | 0xff,
                                        ((attr->value & 0x0000ff) << 8) | 0xff);
                break;
            case IBUS_ATTR_TYPE_BACKGROUND:
                pango_attr = pango_attr_background_new (
                                        ((attr->value & 0xff0000) >> 8) | 0xff,
                                        ((attr->value & 0x00ff00)) | 0xff,
                                        ((attr->value & 0x0000ff) << 8) | 0xff);
                break;
            default:
                continue;
            }
            pango_attr->start_index = g_utf8_offset_to_pointer (str, attr->start_index) - str;
            pango_attr->end_index = g_utf8_offset_to_pointer (str, attr->end_index) - str;
            pango_attr_list_insert (priv->preedit_attrs, pango_attr);
        }
    }
    priv->preedit_cursor_pos = cursor_pos;
    priv->preedit_visible = visible;
    g_signal_emit (context, _signal_preedit_changed_id, 0);
}

static void
_ibus_context_show_preedit_text_cb (IBusInputContext   *ibus_context,
                                    IBusIMContext      *context)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));
    IBusIMContextPrivate *priv = context->priv;

    if (priv->preedit_visible == FALSE) {
        priv->preedit_visible = TRUE;
        g_signal_emit (context, _signal_preedit_changed_id, 0);
    }
}

static void
_ibus_context_hide_preedit_text_cb (IBusInputContext   *ibus_context,
                                    IBusIMContext      *context)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));
    IBusIMContextPrivate *priv = context->priv;

    if (priv->preedit_visible == TRUE) {
        priv->preedit_visible = FALSE;
        g_signal_emit (context, _signal_preedit_changed_id, 0);
    }
}

static void
_ibus_context_enabled_cb (IBusInputContext    *ibus_context,
                IBusIMContext       *context)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));
    IBusIMContextPrivate *priv = context->priv;

    priv->enable = TRUE;
    g_signal_emit (context, _signal_preedit_changed_id, 0);
}

static void
_ibus_context_disabled_cb (IBusInputContext   *ibus_context,
                 IBusIMContext      *context)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));
    IBusIMContextPrivate *priv = context->priv;

    priv->enable = FALSE;
    
    /* clear preedit */
    priv->preedit_visible = FALSE;
    priv->preedit_cursor_pos = 0;
    g_free (priv->preedit_string);
    priv->preedit_string = NULL;
    
    g_signal_emit (context, _signal_preedit_changed_id, 0);
}

static void
_ibus_context_destroy_cb (IBusInputContext    *ibus_context,
                IBusIMContext       *context)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));

    IBusIMContextPrivate *priv = context->priv;

    g_assert (priv->ibus_context == ibus_context);

    g_object_unref (priv->ibus_context);
    priv->ibus_context = NULL;
    priv->enable = FALSE;
}

static void
_create_input_context (IBusIMContext *context)
{
    g_assert (IBUS_IS_IM_CONTEXT (context));
    g_assert (context->priv->ibus_context == NULL);

    IBusIMContextPrivate *priv;
    priv = context->priv;

    priv->ibus_context = ibus_bus_create_input_context (_bus, "test");

    g_signal_connect (priv->ibus_context,
                      "commit-text",
                      G_CALLBACK (_ibus_context_commit_text_cb),
                      context);
    g_signal_connect (priv->ibus_context,
                      "forward-key-event",
                      G_CALLBACK (_ibus_context_forward_key_event_cb),
                      context);
    g_signal_connect (priv->ibus_context,
                      "update-preedit-text",
                      G_CALLBACK (_ibus_context_update_preedit_text_cb),
                      context);
    g_signal_connect (priv->ibus_context,
                      "show-preedit-text",
                      G_CALLBACK (_ibus_context_show_preedit_text_cb),
                      context);
    g_signal_connect (priv->ibus_context,
                      "hide-preedit-text",
                      G_CALLBACK (_ibus_context_hide_preedit_text_cb),
                      context);
    g_signal_connect (priv->ibus_context,
                      "enabled",
                      G_CALLBACK (_ibus_context_enabled_cb),
                      context);
    g_signal_connect (priv->ibus_context,
                      "disabled",
                      G_CALLBACK (_ibus_context_disabled_cb),
                      context);
    g_signal_connect (priv->ibus_context, "destroy",
                      G_CALLBACK (_ibus_context_destroy_cb),
                      context);

    ibus_input_context_set_capabilities (priv->ibus_context, priv->caps);

    if (priv->has_focus) {
        ibus_input_context_focus_in (priv->ibus_context);
    }
}

/* Callback functions for slave context */
static void
_slave_commit_cb (GtkIMContext *slave, gchar *string, IBusIMContext *context)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    /* IBusIMContextPrivate *priv = context->priv; */
#if 0
    if ((GtkIMContext *)context == CURRENT_CONTEXT && ibus_im_client_is_enabled (_client))
        return;
#endif
    g_signal_emit (context, _signal_commit_id, 0, string);
}

static void
_slave_preedit_changed_cb (GtkIMContext *slave, IBusIMContext *context)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    IBusIMContextPrivate *priv = context->priv;

    if (priv->enable && priv->ibus_context) {
        return;
    }

    g_signal_emit (context, _signal_preedit_changed_id, 0);
}

static void
_slave_preedit_start_cb (GtkIMContext *slave, IBusIMContext *context)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    IBusIMContextPrivate *priv = context->priv;

    if (priv->enable && priv->ibus_context) {
        return;
    }
    g_signal_emit (context, _signal_preedit_start_id, 0);
}

static void
_slave_preedit_end_cb (GtkIMContext *slave, IBusIMContext *context)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    IBusIMContextPrivate *priv = context->priv;

    if (priv->enable && priv->ibus_context) {
        return;
    }
    g_signal_emit (context, _signal_preedit_end_id, 0);
}

static void
_slave_retrieve_surrounding_cb (GtkIMContext *slave, IBusIMContext *context)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    IBusIMContextPrivate *priv = context->priv;

    if (priv->enable && priv->ibus_context) {
        return;
    }
    g_signal_emit (context, _signal_retrieve_surrounding_id, 0);
}

static void
_slave_delete_surrounding_cb (GtkIMContext *slave, gint a1, gint a2, IBusIMContext *context)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    IBusIMContextPrivate *priv = context->priv;

    if (priv->enable && priv->ibus_context) {
        return;
    }
    g_signal_emit (context, _signal_delete_surrounding_id, 0, a1, a2);
}

void
ibus_im_context_show_preedit (IBusIMContext *context)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    IBusIMContextPrivate *priv = context->priv;
    if (priv->preedit_visible) {
        return;
    }

    priv->preedit_visible = TRUE;

    g_signal_emit (context, _signal_preedit_changed_id, 0);
}

void
ibus_im_context_hide_preedit (IBusIMContext *context)
{
    g_return_if_fail (context != NULL);
    g_return_if_fail (IBUS_IS_IM_CONTEXT (context));

    IBusIMContextPrivate *priv = context->priv;

    if (!priv->preedit_visible) {
        return;
    }

    priv->preedit_visible = FALSE;

    g_signal_emit (context, _signal_preedit_changed_id, 0);
}
