#pragma once

#   if defined (__GNUC__) && defined(__unix__)
#       define MY_API __attribute__ ((__visibility__("default")))
#   elif defined (WIN32)
#       if defined BUILD_STATIC
#           define MY_API 
#       elif defined BUILD_DLL
#           define MY_API __declspec(dllexport)
#       else
#           define MY_API __declspec(dllimport)
#       endif
#   endif
