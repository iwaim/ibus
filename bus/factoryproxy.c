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

#include <ibusinternal.h>
#include <ibusmarshalers.h>
#include "factoryproxy.h"

/* functions prototype */
static void      bus_factory_proxy_class_init   (BusFactoryProxyClass   *klass);
static void      bus_factory_proxy_init         (BusFactoryProxy        *factory);
static void      bus_factory_proxy_destroy     (BusFactoryProxy        *factory);


static IBusProxyClass  *parent_class = NULL;

GType
bus_factory_proxy_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (BusFactoryProxyClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    bus_factory_proxy_class_init,
        NULL,               /* class finalize */
        NULL,               /* class data */
        sizeof (BusFactoryProxy),
        0,
        (GInstanceInitFunc) bus_factory_proxy_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_PROXY,
                    "BusFactoryProxy",
                    &type_info,
                    (GTypeFlags)0);
    }
    return type;
}

BusFactoryProxy *
bus_factory_proxy_new (IBusFactoryInfo *info,
                       BusConnection   *connection)
{
    g_assert (IBUS_IS_FACTORY_INFO (info));
    g_assert (BUS_IS_CONNECTION (connection));
    BusFactoryProxy *factory;

    factory = g_object_new (BUS_TYPE_FACTORY_PROXY,
                            "name", NULL,
                            "path", info->path,
                            "connection", connection,
                            NULL);
    g_object_ref (info);
    factory->info = info;

    return factory;
}

static void
bus_factory_proxy_class_init (BusFactoryProxyClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);

    parent_class = (IBusProxyClass *) g_type_class_peek_parent (klass);

    ibus_object_class->destroy = (IBusObjectDestroyFunc) bus_factory_proxy_destroy;

}

static void
bus_factory_proxy_init (BusFactoryProxy *factory)
{
    factory->info = NULL;
}

static void
bus_factory_proxy_destroy (BusFactoryProxy *factory)
{
    if (factory->info) {
        g_object_unref (factory->info);
        factory->info = NULL;
    }

    IBUS_OBJECT_CLASS(parent_class)->destroy (IBUS_OBJECT (factory));
}

IBusFactoryInfo *
bus_factory_proxy_get_info (BusFactoryProxy *factory)
{
    return factory->info;
}

const gchar *
bus_factory_proxy_get_name (BusFactoryProxy *factory)
{
    return factory->info->name;
}

const gchar *
bus_factory_proxy_get_lang (BusFactoryProxy *factory)
{
    return factory->info->lang;
}

const gchar *
bus_factory_proxy_get_icon (BusFactoryProxy *factory)
{
    return factory->info->icon;
}

const gchar *
bus_factory_proxy_get_authors (BusFactoryProxy *factory)
{
    return factory->info->authors;
}

const gchar *
bus_factory_proxy_get_credits (BusFactoryProxy *factory)
{
    return factory->info->credits;
}

BusEngineProxy *
bus_factory_create_engine (BusFactoryProxy  *factory)
{
    g_assert (BUS_IS_FACTORY_PROXY (factory));

    IBusMessage *reply_message;
    IBusError *error;
    BusEngineProxy *engine;
    gchar *object_path;

    reply_message = ibus_proxy_call_with_reply_and_block (IBUS_PROXY (factory),
                                                  "CreateEngine",
                                                  -1,
                                                  &error,
                                                  DBUS_TYPE_INVALID);
    if (reply_message == NULL) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        return NULL;
    }

    if ((error = ibus_error_from_message (reply_message)) != NULL) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        ibus_message_unref (reply_message);
        return NULL;
    }

    if (!ibus_message_get_args (reply_message,
                                &error,
                                IBUS_TYPE_OBJECT_PATH, &object_path,
                                G_TYPE_INVALID)) {
        g_warning ("%s: %s", error->name, error->message);
        ibus_error_free (error);
        ibus_message_unref (reply_message);

        return NULL;
    }

    IBusConnection *connection = ibus_proxy_get_connection (IBUS_PROXY (factory));
    engine = bus_engine_proxy_new (object_path, BUS_CONNECTION (connection));
    ibus_message_unref (reply_message);

    return engine;
}

