#pragma once

#if defined WH_TARGET_WINDOWS
#	define USE_CRY_ENGINE	1
#	include <FrameWork/include/Threading/platforms/threadpool_cryengine.h>
#else
#	if defined WIN32 || defined WIN64
#		include "platforms/atomic_win.h"
#	elif defined _XBOX
#		include "platforms/atomic_x360.h"
#	else
#		include "atomic_gcc.h"
#	endif
#endif


