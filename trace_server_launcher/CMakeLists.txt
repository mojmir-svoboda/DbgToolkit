cmake_minimum_required (VERSION 2.8)
project (trace_server_launcher)
set (CMAKE_MODULE_PATH "~/devel/flogging;${CMAKE_MODULE_PATH}")

find_package (Flogging REQUIRED)
find_package (Threads)

message ("Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message ("Flog includes: ${FLOGGING_INCLUDE_DIRS}")
message ("Flog srcs: ${FLOGGING_SOURCE_DIRS}")

add_definitions ("-DTRACE_ENABLED")
add_definitions ("-DTRACE_STATIC")
include_directories (${FLOGGING_INCLUDE_DIRS})
add_library (flog_client_lib STATIC "${FLOGGING_SOURCE_DIRS}/trace_client/trace.cpp")

add_executable (trace_server_launcher launcher.cpp main.cpp)

target_link_libraries (trace_server_launcher flog_client_lib ${CMAKE_THREAD_LIBS_INIT}) 
