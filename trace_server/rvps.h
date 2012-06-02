#pragma once
#include "rendezvous.h"
#include "profilerblockinfo.h"
//#include "profilerwindow.h"
//#include "profilerconnection.h"

namespace profiler
{
	class Connection;
	class ProfilerWindow;
	typedef RendezVousPoint<threadinfos_t, Connection, ProfilerWindow> profiler_rvp_t;
	RendezVouses<profiler_rvp_t, Connection, ProfilerWindow> & getRendezVouses ();
}


