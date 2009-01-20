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

#include "ibusimpl.h"
#include "dbusimpl.h"
#include "server.h"
#include "connection.h"
#include "registry.h"
#include "factoryproxy.h"
#include "panelproxy.h"
#include "inputcontext.h"


enum {
    LAST_SIGNAL,
};

enum {
    PROP_0,
};

// static guint            _signals[LAST_SIGNAL] = { 0 };

/* functions prototype */
static void     bus_ibus_impl_class_init        (BusIBusImplClass   *klass);
static void     bus_ibus_impl_init              (BusIBusImpl        *ibus);
static void     bus_ibus_impl_destroy           (BusIBusImpl        *ibus);
static gboolean bus_ibus_impl_ibus_message      (BusIBusImpl        *ibus,
                                                 BusConnection      *connection,
                                                 IBusMessage        *message);

static void     bus_ibus_impl_set_default_engine(BusIBusImpl        *ibus,
                                                 IBusEngineDesc     *engine);
static void     bus_ibus_impl_add_factory       (BusIBusImpl        *ibus,
                                                 BusFactoryProxy    *factory); 
#if 0
static void     _factory_destroy_cb             (BusFactoryProxy      *factory,
                                                 BusIBusImpl          *ibus);
#endif

static IBusServiceClass  *parent_class = NULL;

GType
bus_ibus_impl_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusIBusImplClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_ibus_impl_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusIBusImpl),
        0,
        (GInstanceInitFunc) bus_ibus_impl_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_SERVICE,
                    "BusIBusImpl",
                    &type_info,
                    (GTypeFlags) 0);
    }
    return type;
}

BusIBusImpl *
bus_ibus_impl_get_default (void)
{
    static BusIBusImpl *ibus = NULL;

    if (ibus == NULL) {
        ibus = (BusIBusImpl *) g_object_new (BUS_TYPE_IBUS_IMPL,
                                             "path", IBUS_PATH_IBUS,
                                             NULL);
        bus_dbus_impl_register_object (BUS_DEFAULT_DBUS,
                                       (IBusService *)ibus);
    }
    return ibus;
}

static void
bus_ibus_impl_class_init (BusIBusImplClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusServiceClass *) g_type_class_peek_parent (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_ibus_impl_destroy;

    IBUS_SERVICE_CLASS (klass)->ibus_message = (ServiceIBusMessageFunc) bus_ibus_impl_ibus_message;

}

static void
_panel_destroy_cb (BusPanelProxy *panel,
                   BusIBusImpl   *ibus)
{
    g_assert (BUS_IS_PANEL_PROXY (panel));
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    g_return_if_fail (ibus->panel == panel);

    ibus->panel = NULL;
    g_object_unref (panel);
}

static void
bus_ibus_impl_set_trigger (BusIBusImpl *ibus,
                           GValue      *value)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    GValueArray *array;
    gint i;

    ibus_hotkey_profile_remove_hotkey_by_event (ibus->hotkey_profile,
                                                g_quark_from_static_string ("trigger"));

    if (value == NULL) {
        ibus_hotkey_profile_add_hotkey (ibus->hotkey_profile,
                                        IBUS_space,
                                        IBUS_CONTROL_MASK,
                                        g_quark_from_static_string ("trigger"));
        return;
    }

    g_return_if_fail (G_VALUE_TYPE (value) == G_TYPE_VALUE_ARRAY);
    array = g_value_get_boxed (value);

    for (i = 0; i < array->n_values; i++) {
       GValue *str;

       str = g_value_array_get_nth (array, i);
       g_return_if_fail (G_VALUE_TYPE (str) == G_TYPE_STRING);

       ibus_hotkey_profile_add_hotkey_from_string (ibus->hotkey_profile,
                                                   g_value_get_string (str),
                                                   g_quark_from_static_string ("trigger"));
   }
}

static void
bus_ibus_impl_reload_config (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    GValue value = { 0 };

    if (ibus->config == NULL) {
        bus_ibus_impl_set_trigger (ibus, NULL);
        return;
    }

    if (ibus_config_get_value (ibus->config, "general/hotkey", "trigger", &value)) {
        bus_ibus_impl_set_trigger (ibus, &value);
        g_value_unset (&value);
    }
    else {
        bus_ibus_impl_set_trigger (ibus, NULL);
    }

}

static void
_config_value_changed_cb (IBusConfig  *config,
                          gchar       *section,
                          gchar       *name,
                          GValue      *value,
                          BusIBusImpl *ibus)
{
    g_assert (IBUS_IS_CONFIG (config));
    g_assert (section);
    g_assert (name);
    g_assert (value);
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    if (g_strcmp0 (section, "general/hotkey") != 0) {
        return;
    }

    if (g_strcmp0 (name, "trigger") == 0) {
        bus_ibus_impl_set_trigger (ibus, value);
    }
}

static void
_config_destroy_cb (IBusConfig  *config,
                    BusIBusImpl *ibus)
{
    g_assert (IBUS_IS_CONFIG (config));
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    g_assert (ibus->config == config);

    ibus->config = NULL;
    g_object_unref (config);
}

static void
_dbus_name_owner_changed_cb (BusDBusImpl *dbus,
                             const gchar *name,
                             const gchar *old_name,
                             const gchar *new_name,
                             BusIBusImpl *ibus)
{
    g_assert (BUS_IS_DBUS_IMPL (dbus));
    g_assert (name != NULL);
    g_assert (old_name != NULL);
    g_assert (new_name != NULL);
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    BusFactoryProxy *factory;

    if (g_strcmp0 (name, IBUS_SERVICE_PANEL) == 0) {
        if (g_strcmp0 (new_name, "") != 0) {
            BusConnection *connection;

            if (ibus->panel != NULL) {
                ibus_object_destroy (IBUS_OBJECT (ibus->panel));
                ibus->panel = NULL;
            }

            connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, new_name);
            g_return_if_fail (connection != NULL);

            ibus->panel = bus_panel_proxy_new (connection);

            g_signal_connect (ibus->panel,
                              "destroy",
                              G_CALLBACK (_panel_destroy_cb),
                              ibus);

            if (ibus->focused_context != NULL) {
                bus_panel_proxy_focus_in (ibus->panel, ibus->focused_context);
            }
        }
    }
    else if (g_strcmp0 (name, IBUS_SERVICE_CONFIG) == 0) {
        if (g_strcmp0 (new_name, "") != 0) {
            BusConnection *connection;

            if (ibus->config != NULL) {
                ibus_object_destroy (IBUS_OBJECT (ibus->config));
                ibus->config = NULL;
            }

            connection = bus_dbus_impl_get_connection_by_name (BUS_DEFAULT_DBUS, new_name);
            g_return_if_fail (connection != NULL);

            ibus->config = g_object_new (IBUS_TYPE_CONFIG,
                                         "name", NULL,
                                         "path", IBUS_PATH_CONFIG,
                                         "connection", connection,
                                         NULL);

            g_signal_connect (ibus->config,
                              "value-changed",
                              G_CALLBACK (_config_value_changed_cb),
                              ibus);

            g_signal_connect (ibus->config,
                              "destroy",
                              G_CALLBACK (_config_destroy_cb),
                              ibus);

            bus_ibus_impl_reload_config (ibus);
        }
    }
    
    factory = bus_registry_name_owner_changed (ibus->registry, name, old_name, new_name);

    if (factory) {
        bus_ibus_impl_add_factory (ibus, factory);   
    }
}

static void
bus_ibus_impl_init (BusIBusImpl *ibus)
{
    ibus->factory_dict = g_hash_table_new_full (g_str_hash,
                                                g_str_equal,
                                                NULL,
                                                (GDestroyNotify) g_object_unref);

    ibus->registry = bus_registry_new ();
    ibus->engine_list = NULL;
    ibus->default_engine = NULL;
    ibus->contexts = NULL;
    ibus->focused_context = NULL;
    ibus->panel = NULL;
    ibus->config = NULL;

    ibus->hotkey_profile = ibus_hotkey_profile_new ();

    bus_ibus_impl_reload_config (ibus);

    g_signal_connect (BUS_DEFAULT_DBUS,
                      "name-owner-changed",
                      G_CALLBACK (_dbus_name_owner_changed_cb),
                      ibus);
}

static void
bus_ibus_impl_destroy (BusIBusImpl *ibus)
{
#if 0
    for (p = ibus->factory_list; p != NULL; p = p->next) {
        g_signal_handlers_disconnect_by_func (p->data,
                                              G_CALLBACK (_factory_destroy_cb),
                                              ibus);
    }
    g_list_free (ibus->factory_list);
    ibus->factory_list = NULL;
#endif

    g_list_foreach (ibus->engine_list, (GFunc) g_object_unref, NULL);
    g_list_free (ibus->engine_list);
    ibus->engine_list = NULL;

    if (ibus->factory_dict != NULL) {
        g_hash_table_destroy (ibus->factory_dict);
        ibus->factory_dict = NULL;
    }

    if (ibus->hotkey_profile != NULL) {
        g_object_unref (ibus->hotkey_profile);
        ibus->hotkey_profile = NULL;
    }

    bus_server_quit (BUS_DEFAULT_SERVER);

    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (ibus));
}

/* introspectable interface */
static IBusMessage *
_ibus_introspect (BusIBusImpl     *ibus,
                  IBusMessage     *message,
                  BusConnection   *connection)
{
    static const gchar *introspect =
        "<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
        "\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
        "<node>\n"
        "  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
        "    <method name=\"Introspect\">\n"
        "      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
        "    </method>\n"
        "  </interface>\n"
        "  <interface name=\"org.freedesktop.DBus\">\n"
        "    <method name=\"RequestName\">\n"
        "      <arg direction=\"in\" type=\"s\"/>\n"
        "      <arg direction=\"in\" type=\"u\"/>\n"
        "      <arg direction=\"out\" type=\"u\"/>\n"
        "    </method>\n"
        "    <signal name=\"NameOwnerChanged\">\n"
        "      <arg type=\"s\"/>\n"
        "      <arg type=\"s\"/>\n"
        "      <arg type=\"s\"/>\n"
        "    </signal>\n"
        "  </interface>\n"
        "</node>\n";

    IBusMessage *reply_message;
    reply_message = ibus_message_new_method_return (message);
    ibus_message_append_args (reply_message,
                              G_TYPE_STRING, &introspect,
                              G_TYPE_INVALID);

    return reply_message;
}



static IBusMessage *
_ibus_get_address (BusIBusImpl     *ibus,
                   IBusMessage     *message,
                   BusConnection   *connection)
{
    const gchar *address;
    IBusMessage *reply;

    address = ibus_server_get_address (IBUS_SERVER (BUS_DEFAULT_SERVER));

    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (message,
                              G_TYPE_STRING, &address,
                              G_TYPE_INVALID);

    return reply;
}

static void
_engine_destroy_cb (IBusEngineDesc *engine,
                    BusIBusImpl    *ibus)
{
    if (engine == ibus->default_engine) {
       bus_ibus_impl_set_default_engine (ibus, NULL); 
    }
}


static void
bus_ibus_impl_set_default_engine (BusIBusImpl    *ibus,
                                  IBusEngineDesc *engine)
{
    if (ibus->default_engine != NULL) {
        g_signal_handlers_disconnect_by_func (ibus->default_engine, _engine_destroy_cb, ibus);
        g_object_unref (ibus->default_engine);
        ibus->default_engine = NULL;
    }

    if (engine != NULL) {
        ibus->default_engine = engine;
        g_object_ref (ibus->default_engine);
        g_signal_connect (ibus->default_engine, "destroy", G_CALLBACK (_engine_destroy_cb), ibus);
    }
}

static void
_context_request_engine_cb (BusInputContext *context,
                            gchar           *engine_name,
                            BusIBusImpl     *ibus)
{
    IBusEngineDesc *engine_desc;
    IBusComponent *comp;
    BusFactoryProxy *factory;
    BusEngineProxy *engine;
    
    if (engine_name == NULL) {
        if (ibus->default_engine == NULL) {
            if (ibus->engine_list) {
                bus_ibus_impl_set_default_engine (ibus, (IBusEngineDesc *)ibus->engine_list->data);
            }
        }
        engine_desc = ibus->default_engine;
    }
    else {
        GList *p;
        for (p = ibus->engine_list; p != NULL; p = p->next) {
            engine_desc = (IBusEngineDesc *)p->data;
            if (g_strcmp0 (engine_desc->name, engine_name) == 0)
                break;
        }

        if (p == NULL) {
            engine_desc = bus_registry_find_engine_by_name (ibus->registry, engine_name);
        }
        else {
            engine_desc = NULL;
        }
    }

    if (engine_desc == NULL)
        return;

    factory = bus_factory_proxy_get_from_engine (engine_desc);
    comp = ibus_component_get_from_engine (engine_desc);

    if (comp == NULL)
        return;

    if (factory == NULL) {
        if (!ibus_component_is_running (comp)) {
            ibus_component_start (comp);

            if (g_main_context_pending (NULL)) {
                g_main_context_iteration (NULL, FALSE);
                g_usleep (50);
            }
        }
        factory = bus_factory_proxy_get_from_engine (engine_desc);
    }

    if (factory == NULL)
        return;
    
    engine = bus_factory_proxy_create_engine (factory, engine_desc);

    if (engine == NULL)
        return;

    bus_input_context_set_engine (context, engine);
}

static void
_context_request_next_engine_cb (BusInputContext *context,
                                 BusIBusImpl     *ibus)
{
    
}

static void
_context_request_prev_engine_cb (BusInputContext *context,
                                 BusIBusImpl     *ibus)
{
    
}

static void
_context_focus_in_cb (BusInputContext *context,
                      BusIBusImpl     *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (ibus->focused_context) {
        bus_input_context_focus_out (ibus->focused_context);
        g_object_unref (ibus->focused_context);
    }

    g_object_ref (context);
    ibus->focused_context = context;

    if (ibus->panel != NULL) {
        bus_panel_proxy_focus_in (ibus->panel, ibus->focused_context);
    }

}

static void
_context_focus_out_cb (BusInputContext    *context,
                       BusIBusImpl        *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (ibus->focused_context != context)
        return;

    if (ibus->panel != NULL) {
        bus_panel_proxy_focus_out (ibus->panel, ibus->focused_context);
    }

    if (ibus->focused_context) {
        g_object_unref (ibus->focused_context);
        ibus->focused_context = NULL;
    }
}

static void
_context_destroy_cb (BusInputContext    *context,
                     BusIBusImpl        *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_INPUT_CONTEXT (context));

    if (context == ibus->focused_context) {

    }

    ibus->contexts = g_list_remove (ibus->contexts, context);
    g_object_unref (context);
}

static IBusMessage *
_ibus_create_input_context (BusIBusImpl     *ibus,
                            IBusMessage     *message,
                            BusConnection   *connection)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (message != NULL);
    g_assert (BUS_IS_CONNECTION (connection));

    gint i;
    gchar *client;
    IBusError *error;
    IBusMessage *reply;
    BusInputContext *context;
    const gchar *path;

    if (!ibus_message_get_args (message,
                                &error,
                                G_TYPE_STRING, &client,
                                G_TYPE_INVALID)) {
        reply = ibus_message_new_error (message,
                                        DBUS_ERROR_INVALID_ARGS,
                                        "Argument 1 of CreateInputContext should be an string");
        ibus_error_free (error);
        return reply;
    }

    context = bus_input_context_new (connection, client);
    ibus->contexts = g_list_append (ibus->contexts, context);

    static const struct {
        gchar *name;
        GCallback callback;
    } signals [] = {
        { "request-engine",      G_CALLBACK (_context_request_engine_cb) },
        { "request-next-engine", G_CALLBACK (_context_request_next_engine_cb) },
        { "request-prev-engine", G_CALLBACK (_context_request_prev_engine_cb) },
        { "focus-in",       G_CALLBACK (_context_focus_in_cb) },
        { "focus-out",      G_CALLBACK (_context_focus_out_cb) },
        { "destroy",        G_CALLBACK (_context_destroy_cb) },
        { NULL, NULL }
    };

    for (i = 0; signals[i].name != NULL; i++) {
        g_signal_connect (context,
                          signals[i].name,
                          signals[i].callback,
                          ibus);
    }

    path = ibus_service_get_path (IBUS_SERVICE (context));
    reply = ibus_message_new_method_return (message);
    ibus_message_append_args (reply,
                              IBUS_TYPE_OBJECT_PATH, &path,
                              G_TYPE_INVALID);

    bus_dbus_impl_register_object (BUS_DEFAULT_DBUS,
                                   (IBusService *)context);
    return reply;
}

#if 0


#endif

#if 0
static int
_factory_cmp (BusFactoryProxy   *a,
              BusFactoryProxy   *b)
{
    g_assert (BUS_IS_FACTORY_PROXY (a));
    g_assert (BUS_IS_FACTORY_PROXY (b));

    gint retval;

    retval = g_strcmp0 (bus_factory_proxy_get_lang (a), bus_factory_proxy_get_lang (b));
    if (retval != 0)
        return retval;
    retval = g_strcmp0 (bus_factory_proxy_get_name (a), bus_factory_proxy_get_name (b));
    return retval;
}
#endif

static IBusMessage *
_ibus_register_factories (BusIBusImpl     *ibus,
                          IBusMessage     *message,
                          BusConnection   *connection)
{
#if 0
    IBusMessageIter iter, sub_iter;
    IBusMessage *reply;
    gboolean retval;
    BusFactoryProxy *factory;
    GArray *info_array;
    IBusFactoryInfo *info;
    gint i;

    retval = ibus_message_iter_init (message, &iter);
    g_assert (retval);

    retval = ibus_message_iter_recurse (&iter, IBUS_TYPE_ARRAY, &sub_iter);
    g_assert (retval);


    info_array = g_array_new (TRUE, TRUE, sizeof (IBusFactoryInfo *));

    for (;;) {

        if (ibus_message_iter_get_arg_type (&sub_iter) == G_TYPE_INVALID)
            break;

        retval = ibus_message_iter_get (&sub_iter,
                                        IBUS_TYPE_FACTORY_INFO,
                                        &info);
        g_assert (retval);

        g_array_append_val (info_array, info);


        if (g_hash_table_lookup (ibus->factory_dict, info->path) != NULL) {
            reply = ibus_message_new_error_printf (message,
                                                   DBUS_ERROR_FAILED,
                                                   "Factory %s has been registered!",
                                                   info->path);
            goto _out;
        }

    }

    for (i = 0; i < info_array->len; i++) {
        info = g_array_index (info_array, IBusFactoryInfo *, i);

        factory = bus_factory_proxy_new (info, connection);

        g_hash_table_insert (ibus->factory_dict,
                             (gpointer) ibus_proxy_get_path (IBUS_PROXY (factory)),
                             factory);

        g_object_ref (factory);
        ibus->factory_list = g_list_append (ibus->factory_list, factory);

        g_signal_connect (factory,
                          "destroy",
                          (GCallback) _factory_destroy_cb,
                          ibus);
    }

    reply = ibus_message_new_method_return (message);

    ibus->factory_list = g_list_sort (ibus->factory_list, (GCompareFunc) _factory_cmp);

_out:
    for (i = 0; i < info_array->len; i++) {
        info = g_array_index (info_array, IBusFactoryInfo *, i);
        g_object_unref (info);
    }

    g_array_free (info_array, TRUE);

    return reply;
#endif
    return NULL;
}

static void
_factory_destroy_cb (BusFactoryProxy    *factory,
                     BusIBusImpl        *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_FACTORY_PROXY (factory));

    ibus->factory_list = g_list_remove (ibus->factory_list, factory);
    g_object_unref (factory);

}

static void
bus_ibus_impl_add_factory (BusIBusImpl     *ibus,
                           BusFactoryProxy *factory)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_FACTORY_PROXY (factory));

    ibus->factory_list = g_list_append (ibus->factory_list, factory);
}


static IBusMessage *
_ibus_register_component (BusIBusImpl     *ibus,
                          IBusMessage     *message,
                          BusConnection   *connection)
{
    IBusMessage *reply;
    IBusError *error;
    gboolean retval;
    GList *engines;
    IBusComponent *component;
    BusFactoryProxy *factory;

    retval = ibus_message_get_args (message, &error,
                                    IBUS_TYPE_COMPONENT, &component,
                                    G_TYPE_INVALID);

    if (!retval) {
        reply = ibus_message_new_error_printf (message,
                                               DBUS_ERROR_INVALID_ARGS,
                                               "1st Argument must be IBusComponent: %s",
                                               error->message);
        ibus_error_free (error);
        return reply;
    }

    factory = bus_factory_proxy_new (component, connection);

    if (factory == NULL) {
        reply = ibus_message_new_error (message,
                                        DBUS_ERROR_FAILED,
                                        "Can not create factory");
        g_object_unref (component);
        return reply;
    }

    bus_ibus_impl_add_factory (ibus, factory);

    engines = ibus_component_get_engines (component);
    g_list_foreach (engines, (GFunc) g_object_ref, NULL);
    ibus->engine_list = g_list_concat (ibus->engine_list, engines);

    reply = ibus_message_new_method_return (message);
    return reply;
}

static IBusMessage *
_ibus_list_factories (BusIBusImpl     *ibus,
                      IBusMessage     *message,
                      BusConnection   *connection)
{
#if 0
    IBusMessage *reply;
    IBusMessageIter iter, sub_iter;
    GList *p;

    reply = ibus_message_new_method_return (message);

    ibus_message_iter_init_append (reply, &iter);
    ibus_message_iter_open_container (&iter, IBUS_TYPE_ARRAY, "v", &sub_iter);

    for (p = ibus->factory_list; p != NULL; p = p->next) {
        BusFactoryProxy *factory;
        IBusFactoryInfo *info;

        factory = BUS_FACTORY_PROXY (p->data);
        info = bus_factory_proxy_get_info (factory);
        ibus_message_iter_append (&sub_iter, IBUS_TYPE_FACTORY_INFO, &info);
    }
    ibus_message_iter_close_container (&iter, &sub_iter);
    return reply;
#endif
    return NULL;
}

static IBusMessage *
_ibus_set_factory (BusIBusImpl      *ibus,
                   IBusMessage      *message,
                   BusConnection    *connection)
{
    return NULL;
}

static IBusMessage *
_ibus_list_engines (BusIBusImpl   *ibus,
                    IBusMessage   *message,
                    BusConnection *connection)
{
    IBusMessage *reply;
    IBusMessageIter iter, sub_iter;
    GList *engines, *p;

    reply = ibus_message_new_method_return (message);

    ibus_message_iter_init_append (reply, &iter);
    ibus_message_iter_open_container (&iter, IBUS_TYPE_ARRAY, "v", &sub_iter);

    engines = bus_registry_get_engines (ibus->registry);
    for (p = engines; p != NULL; p = p->next) {
        ibus_message_iter_append (&sub_iter, IBUS_TYPE_ENGINE_DESC, &(p->data));
    }
    g_list_free (engines);
    ibus_message_iter_close_container (&iter, &sub_iter);

    return reply;
}

static IBusMessage *
_ibus_list_active_engines (BusIBusImpl   *ibus,
                           IBusMessage   *message,
                           BusConnection *connection)
{
    IBusMessage *reply;
    IBusMessageIter iter, sub_iter;
    GList *p;

    reply = ibus_message_new_method_return (message);

    ibus_message_iter_init_append (reply, &iter);
    ibus_message_iter_open_container (&iter, IBUS_TYPE_ARRAY, "v", &sub_iter);

    for (p = ibus->engine_list; p != NULL; p = p->next) {
        ibus_message_iter_append (&sub_iter, IBUS_TYPE_ENGINE_DESC, &(p->data));
    }
    ibus_message_iter_close_container (&iter, &sub_iter);

    return reply;
}

static IBusMessage *
_ibus_kill (BusIBusImpl     *ibus,
            IBusMessage     *message,
            BusConnection   *connection)
{
    IBusMessage *reply;

    reply = ibus_message_new_method_return (message);
    ibus_connection_send (IBUS_CONNECTION (connection), reply);
    ibus_connection_flush (IBUS_CONNECTION (connection));
    ibus_message_unref (reply);

    ibus_object_destroy (IBUS_OBJECT (ibus));
    return NULL;
}

static gboolean
bus_ibus_impl_ibus_message (BusIBusImpl     *ibus,
                            BusConnection   *connection,
                            IBusMessage     *message)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_CONNECTION (connection));
    g_assert (message != NULL);

    gint i;
    IBusMessage *reply_message = NULL;

    static const struct {
        const gchar *interface;
        const gchar *name;
        IBusMessage *(* handler) (BusIBusImpl *, IBusMessage *, BusConnection *);
    } handlers[] =  {
        /* Introspectable interface */
        { DBUS_INTERFACE_INTROSPECTABLE,
                               "Introspect", _ibus_introspect },
        /* IBus interface */
        { IBUS_INTERFACE_IBUS, "GetAddress",            _ibus_get_address },
        { IBUS_INTERFACE_IBUS, "CreateInputContext",    _ibus_create_input_context },
        { IBUS_INTERFACE_IBUS, "RegisterFactories",     _ibus_register_factories },
        { IBUS_INTERFACE_IBUS, "RegisterComponent",     _ibus_register_component },
        { IBUS_INTERFACE_IBUS, "ListFactories",         _ibus_list_factories },
        { IBUS_INTERFACE_IBUS, "SetFactory",            _ibus_set_factory },
        { IBUS_INTERFACE_IBUS, "ListEngines",           _ibus_list_engines },
        { IBUS_INTERFACE_IBUS, "ListActiveEngines",     _ibus_list_active_engines },
        { IBUS_INTERFACE_IBUS, "Kill",                  _ibus_kill },
        { NULL, NULL, NULL }
    };

    ibus_message_set_sender (message, bus_connection_get_unique_name (connection));
    ibus_message_set_destination (message, DBUS_SERVICE_DBUS);

    for (i = 0; handlers[i].interface != NULL; i++) {
        if (ibus_message_is_method_call (message,
                                         handlers[i].interface,
                                         handlers[i].name)) {

            reply_message = handlers[i].handler (ibus, message, connection);
            if (reply_message) {

                ibus_message_set_sender (reply_message, DBUS_SERVICE_DBUS);
                ibus_message_set_destination (reply_message, bus_connection_get_unique_name (connection));
                ibus_message_set_no_reply (reply_message, TRUE);

                ibus_connection_send (IBUS_CONNECTION (connection), reply_message);
                ibus_message_unref (reply_message);
            }

            g_signal_stop_emission_by_name (ibus, "ibus-message");
            return TRUE;
        }
    }

    return parent_class->ibus_message ((IBusService *) ibus,
                                       (IBusConnection *) connection,
                                       message);
}

#if 0
BusFactoryProxy *
bus_ibus_impl_get_default_factory (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    if (ibus->default_factory == NULL && ibus->factory_list != NULL) {
        ibus->default_factory = BUS_FACTORY_PROXY (ibus->factory_list->data);
        g_object_ref (ibus->default_factory);
    }

    if (ibus->default_factory == NULL) {
        /* TODO */
    }

    return ibus->default_factory;
}

BusFactoryProxy *
bus_ibus_impl_get_next_factory (BusIBusImpl     *ibus,
                                BusFactoryProxy *factory)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_FACTORY_PROXY (factory) || factory == NULL);

    GList *link;

    if (factory == NULL) {
        return bus_ibus_impl_get_default_factory (ibus);
    }

    link = g_list_find (ibus->factory_list, factory);

    g_assert (link != NULL);

    link = link->next;

    if (link != NULL) {
        link = ibus->factory_list;
    }

    return BUS_FACTORY_PROXY (link->data);
}

BusFactoryProxy *
bus_ibus_impl_get_previous_factory (BusIBusImpl     *ibus,
                                    BusFactoryProxy *factory)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));
    g_assert (BUS_IS_FACTORY_PROXY (factory) || factory == NULL);

    GList *link;

    if (factory == NULL) {
        return bus_ibus_impl_get_default_factory (ibus);
    }

    link = g_list_find (ibus->factory_list, factory);

    g_assert (link != NULL);

    link = link->prev;

    if (link != NULL) {
        link = g_list_last (ibus->factory_list);
    }

    return BUS_FACTORY_PROXY (link->data);
}
#endif

BusFactoryProxy *
bus_ibus_impl_lookup_factory (BusIBusImpl *ibus,
                              const gchar *path)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    BusFactoryProxy *factory;

    factory = (BusFactoryProxy *) g_hash_table_lookup (ibus->factory_dict, path);

    return factory;
}

IBusHotkeyProfile *
bus_ibus_impl_get_hotkey_profile (BusIBusImpl *ibus)
{
    g_assert (BUS_IS_IBUS_IMPL (ibus));

    return ibus->hotkey_profile;
}
