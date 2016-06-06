#pragma once

// Windows Version
#define _WIN32_WINNT 0x0501     // _WIN32_WINNT_WINXP
#include <SDKDDKVer.h>

// System Includes
#define WIN32_LEAN_AND_MEAN
#include <windows.h>



#include <cstdio>
#define _ENABLE_REPORT 1
#define PUBLIC_API
#include "windows.h"
class Time {};

#define INIT_PRIORITY_URGENT