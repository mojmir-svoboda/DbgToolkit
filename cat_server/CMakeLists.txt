cmake_minimum_required (VERSION 2.8)
project (cat_server)
set (CMAKE_MODULE_PATH "~/devel/flogging;${CMAKE_MODULE_PATH}")

find_package (Flogging REQUIRED)
find_package (Threads)

set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS 1)
FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )

    set(BOOST_LIBS thread system date_time regex chrono)
    find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)
ENDIF()

message ("Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message ("Flog includes: ${FLOGGING_INCLUDE_DIRS}")
message ("Flog srcs: ${FLOGGING_SOURCE_DIRS}")

add_definitions ("-DUSE_STD_STRING")
add_definitions ("-DTRACE_ENABLED")
add_definitions ("-DTRACE_STATIC")
add_definitions ("-DPROFILE_ENABLED")
add_definitions ("-DPROFILE_STATIC")
include_directories (${FLOGGING_INCLUDE_DIRS})
add_library (fprof_client_lib STATIC "${FLOGGING_SOURCE_DIRS}/trace_client/profile.cpp")
add_library (flog_client_lib STATIC "${FLOGGING_SOURCE_DIRS}/trace_client/trace.cpp")

add_executable (cat_server connection.cpp main.cpp)

if (MINGW)
	target_link_libraries (cat_server fprof_client_lib "c:/devel/QtSDK/mingw/lib/libws2_32.a")
endif()
target_link_libraries (cat_server ${Boost_LIBRARIES} fprof_client_lib flog_client_lib ${CMAKE_THREAD_LIBS_INIT}) 
