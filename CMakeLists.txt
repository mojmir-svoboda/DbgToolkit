cmake_policy(SET CMP0048 NEW)
project (DbgToolkit VERSION 1.1.0)

set(CMAKE_INSTALL_PREFIX "c:/opt/")
option(BUILD_SERVER "Builds server (requires Boost and Qt5)." OFF)
option(BUILD_SAMPLES "Builds samples." OFF)
option(BUILD_QT_TESTS "Builds qt tests (requires Boost and Qt5)." OFF)

cmake_minimum_required (VERSION 3.1)
set (CMAKE_VERBOSE_MAKEFILE on)
set (DbgToolkit_VERSION 1.1.0)

# set up include-directories
include_directories (
  "${PROJECT_SOURCE_DIR}"   # to find trace_client/trace.h
  "${PROJECT_BINARY_DIR}")  # to find trace_client/config.h

include_directories(include)
add_subdirectory ("trace_version")
add_subdirectory ("trace_proto")

if(BUILD_SAMPLES)
	add_subdirectory ("sample_integrations")
endif(BUILD_SAMPLES)

if(BUILD_SERVER)
	add_subdirectory ("trace_server")
endif(BUILD_SERVER)

if(BUILD_QT_TESTS)
	add_subdirectory ("tests/qt_server")
#	add_subdirectory ("tests/qt_only")
endif(BUILD_QT_TESTS)

