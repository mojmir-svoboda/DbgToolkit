#include "rvps.h"
#include "rendezvous.h"
#include "profilerwindow.h"
#include "profilerconnection.h"

namespace profiler
{
	RendezVouses<profiler_rvp_t, Connection, ProfilerWindow> g_RVPs;

	RendezVouses<profiler_rvp_t, Connection, ProfilerWindow> & getRendezVouses () { return g_RVPs; }
}
