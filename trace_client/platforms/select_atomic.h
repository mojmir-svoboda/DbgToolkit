#pragma once

#if defined USE_CRY_ENGINE
#	include <FrameWork/include/Threading/platforms/threadpool_cryengine.h>
#else
#	if defined WIN32 || defined WIN64
#		include "platforms/atomic_win.h"
#	elif defined _XBOX
#		include "platforms/atomic_x360.h"
#	else
#		include "platforms/atomic_gcc.h"
#	endif
#endif


