#include "registry.h"

int main()
{
	g_type_init ();
	BusRegistry *registry = bus_registry_get_default ();
	g_object_unref (registry);
	return 0;
}
