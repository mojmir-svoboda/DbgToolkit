#pragma once
#include <boost/shared_ptr.hpp>
#include "rendezvous.h"
#include "profilerblockinfo.h"
//#include "profilerwindow.h"
//#include "profilerconnection.h"

namespace profiler
{
	struct Connection;
	class ProfilerWindow;
	typedef boost::shared_ptr<Connection> connection_ptr_t;
	typedef RendezVousPoint<threadinfos_t, connection_ptr_t, ProfilerWindow * > profiler_rvp_t;
	RendezVouses<profiler_rvp_t, Connection, ProfilerWindow> & getRendezVouses ();
}


