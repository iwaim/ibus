#include <stdlib.h>
#include <locale.h>
#include "server.h"

static gboolean daemonize = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry entries[] = 
{
    { "daemonize", 'd', 0, G_OPTION_ARG_NONE, &daemonize, "run ibus as background process", NULL },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
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

	g_type_init ();
	
	server = bus_server_get_default ();
	bus_server_listen (server);
	bus_server_run (server);

	return 0;
}
