cmake_policy(SET CMP0048 NEW)
cmake_minimum_required (VERSION 3.2)
project (simple_client VERSION 1.1.0)

find_package (Threads)

message ("Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")

add_definitions ("-DTRACE_CONFIG_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/sample_config.h\"")
add_definitions ("-DTRACE_LEVELS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/default_levels.h\"")
add_definitions ("-DTRACE_CONTEXTS_INCLUDE=\"${CMAKE_CURRENT_SOURCE_DIR}/default_contexts.h\"")
add_definitions ("-DTRACE_ENABLED")
add_definitions ("-DTRACE_STATIC")
include_directories(${CMAKE_BINARY_DIR}/trace_proto)

add_definitions ("-DASSUMESTDTYPES") # asn1: use types from cstdint
add_definitions ("-DUSE_CXX_ALLOCATOR") # asn1: use custom c++ allocator for encoder / decoder

add_definitions ("-D_CRT_SECURE_NO_WARNINGS") # msvc shut up pls

if (BUILD_ASN1_FILES)
  include_directories(${CMAKE_BINARY_DIR}/trace_proto)
  include_directories(${trace_server_SOURCE_DIR})
else (BUILD_ASN1_FILES)
  include_directories(${CMAKE_SOURCE_DIR}/trace_proto/generated)
  include_directories(${trace_server_SOURCE_DIR})
endif (BUILD_ASN1_FILES)

add_library (trace_client_lib STATIC
	"${CMAKE_SOURCE_DIR}/trace_client/trace.cpp"
	"${CMAKE_SOURCE_DIR}/trace_client/encoder.cpp"
	"${CMAKE_SOURCE_DIR}/trace_client/decoder.cpp"
	"${CMAKE_SOURCE_DIR}/trace_client/socket_win.cpp"
)

add_executable (simple_client main.cpp)

if (MINGW)
	target_link_libraries (simple_client trace_client "c:/devel/QtSDK/mingw/lib/libws2_32.a")
endif()
target_link_libraries (simple_client trace_client_lib trace_proto trace_version ${CMAKE_THREAD_LIBS_INIT}) 
