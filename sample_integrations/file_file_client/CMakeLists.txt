cmake_policy(SET CMP0048 NEW)
cmake_minimum_required (VERSION 3.2)
project (file_file_client VERSION 1.1.0)

find_package (Threads)

add_definitions ("-DTRACE_CONFIG_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/config.h\"")
add_definitions ("-DTRACE_LEVELS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/levels.h\"")
add_definitions ("-DTRACE_CONTEXTS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/contexts.h\"")
add_definitions ("-DTRACE_ENABLED")
add_definitions ("-DTRACE_STATIC")
# disable networking for file only client
add_definitions ("-DTRACE_CLIENT_DISABLE_NETWORKING")

add_executable (file_file_client
	main.cpp
	"${CMAKE_SOURCE_DIR}/trace_client/trace.cpp"
	"${CMAKE_SOURCE_DIR}/trace_client/trace_client.cpp"
	"${CMAKE_SOURCE_DIR}/trace_client/Client.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/config.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/levels.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/contexts.h"
)
target_link_libraries (file_file_client trace_version ${CMAKE_THREAD_LIBS_INIT}) 

add_definitions ("-D_CRT_SECURE_NO_WARNINGS") # msvc shut up pls
