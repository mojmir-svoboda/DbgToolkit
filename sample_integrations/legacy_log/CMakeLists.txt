cmake_policy(SET CMP0048 NEW)
cmake_minimum_required (VERSION 3.2)
project (legacy_log VERSION 1.1.0)

#add_definitions ("-DTRACE_ENABLED")
#add_definitions ("-DTRACE_STATIC")

add_executable(legacy_log
	main.cpp
	appFrame.cpp
	appFrameExt.cpp
	utils_debuglog.cpp
)
