cmake_policy(SET CMP0048 NEW)
cmake_minimum_required (VERSION 3.2)
project (asio_file_client VERSION 1.1.0)

find_package (Threads)

option(BUILD_SAMPLES_WITH_ASIO "Uses asio library for networking." ON)

add_definitions ("-DTRACE_CONFIG_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/config.h\"")
add_definitions ("-DTRACE_LEVELS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/levels.h\"")
add_definitions ("-DTRACE_CONTEXTS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/contexts.h\"")
add_definitions ("-DTRACE_ENABLED")
add_definitions ("-DTRACE_STATIC")

include_directories(${CMAKE_BINARY_DIR}/trace_proto)
add_definitions ("-DASSUMESTDTYPES") # asn1: use types from cstdint
add_definitions ("-DUSE_CXX_ALLOCATOR") # asn1: use custom c++ allocator for encoder / decoder
if (BUILD_ASN1_FILES)
	include_directories(${CMAKE_BINARY_DIR}/trace_proto)
	include_directories(${trace_server_SOURCE_DIR})
else (BUILD_ASN1_FILES)
	include_directories(${CMAKE_SOURCE_DIR}/trace_proto/generated)
	include_directories(${trace_server_SOURCE_DIR})
endif (BUILD_ASN1_FILES)

if(BUILD_ASN1_DEBUG)
	set(ASN1_DEBUG_SOURCES asn1_debug.cpp)
endif(BUILD_ASN1_DEBUG)
if(BUILD_SAMPLES_WITH_ASIO)
	# asio
	add_definitions("-DASIO_STANDALONE")
	include_directories (${CMAKE_SOURCE_DIR}/3rd/asio/include)

	set(NETWORKING_SOURCES "${CMAKE_SOURCE_DIR}/trace_client/AsioSocketClient.cpp")
else()
	set(NETWORKING_SOURCES "${CMAKE_SOURCE_DIR}/trace_client/socket_win.cpp")
endif()

add_executable (asio_file_client
	main.cpp
	${ASN1_DEBUG_SOURCES}
	${NETWORKING_SOURCES}
	"${CMAKE_CURRENT_SOURCE_DIR}/config.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/levels.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/contexts.h"
	"${CMAKE_SOURCE_DIR}/trace_client/trace.cpp"
	"${CMAKE_SOURCE_DIR}/trace_client/trace_client.cpp"
	"${CMAKE_SOURCE_DIR}/trace_client/Client.cpp"
	"${CMAKE_SOURCE_DIR}/trace_client/decoder.cpp"
)
target_link_libraries (asio_file_client trace_proto trace_version ${CMAKE_THREAD_LIBS_INIT})

add_definitions ("-D_CRT_SECURE_NO_WARNINGS") # msvc shut up pls
