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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <unistd.h>
#include <stdlib.h>
#include <locale.h>
#include "server.h"

static gboolean daemonize = FALSE;
static gchar *panel = "default";
static gchar *config = "default";
static gchar *desktop = "gnome";
static gboolean verbose = FALSE;

static const GOptionEntry entries[] = 
{
    { "daemonize", 'd', 0, G_OPTION_ARG_NONE, &daemonize, "run ibus as background process", NULL },
    { "desktop",   'n', 0, G_OPTION_ARG_STRING, &desktop, "specify the name of desktop session. [default=gnome]", "name" },
    { "panel",     'p', 0, G_OPTION_ARG_STRING, &panel, "specify the path of panel program", NULL },
    { "config",    'c', 0, G_OPTION_ARG_STRING, &config, "specify the path of config program", NULL },
    { "verbose",   'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
    { NULL },
};


gint
main (gint argc, gchar **argv)
{
    GOptionContext *context;
    BusServer *server;
    GError *error = NULL;
    
    setlocale (LC_ALL, "");
    
    context = g_option_context_new ("- ibus daemon");
    
    g_option_context_add_main_entries (context, entries, "ibus-daemon");

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("Option parsing failed: %s\n", error->message);
        exit (-1);
    }

    if (daemonize) {
        if (daemon (1, 0) != 0) {
            g_print ("Can not daemonize ibus.");
            exit (-1);
        }
    }

    g_type_init ();
    
    server = bus_server_get_default ();
    bus_server_listen (server);
    bus_server_run (server);

    return 0;
}
