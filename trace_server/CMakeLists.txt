cmake_minimum_required(VERSION 2.8)
PROJECT(trace_server)
FIND_PACKAGE(Qt4 REQUIRED)
SET (LIBRARY_OUTPUT_PATH
   ${PROJECT_BINARY_DIR}/Bin
   CACHE PATH
   "Single Directory for all Libraries"
   )

SET (EXECUTABLE_OUTPUT_PATH
   ${PROJECT_BINARY_DIR}/Bin
   CACHE PATH
   "Single Directory for all Executables."
   )

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "c:/devel/boost_1_47_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "c:/devel/boost_1_47_0")

add_definitions("-DTRACE_ENABLED")
add_definitions("-DTRACE_STATIC")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..)

FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

SET(trace_server_SOURCES
	main.cpp
	mainwindow.cpp
    modelview.cpp
    server.cpp
    connection.cpp
    sessionstate.cpp
    settings.cpp )

SET(trace_server_HEADERS
	mainwindow.h 
    modelview.h
    server.h
    connection.h
    ../tlv_parser/tlv_parser.h
    ../tlv_parser/tlv_encoder.h
    ../tlv_parser/tlv_decoder.h
    ../filters/nnode.hpp
    ../filters/file_filter.hpp
    sessionstate.h
    settings.h)

SET(trace_server_UI mainwindow.ui settings.ui)
SET(trace_server_RESOURCES resources.qrc)
SET(QT_USE_QTNETWORK true)

QT4_WRAP_UI(trace_server_UI_HDRS ${trace_server_UI})
QT4_WRAP_CPP(trace_server_MOC ${trace_server_HEADERS})
QT4_ADD_RESOURCES(trace_server_RESOURCES_RCC ${trace_server_RESOURCES})
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_STATIC)
#ADD_LIBRARY(qico STATIC)


message("Generated headers: ${trace_server_UI_HDRS}")
message("Generated files ${CMAKE_BINARY_DIR}")
message("Generated files trace_server_MOC ${trace_server_MOC}")
message("Generated files trace_server_UI_HDRS ${trace_server_UI_HDRS}")
message("Generated files trace_server_UI_SRCS ${trace_server_UI_SRCS}")
message("Generated files trace_server_RESOURCES_RCC ${trace_server_RESOURCES_RCC}")
message("Generated files trace_server_SOURCES ${trace_server_SOURCES}")
ADD_EXECUTABLE(trace_server
		${trace_server_MOC}
		${trace_server_UI_HDRS}
		${trace_server_UI_SRCS}
		${trace_server_RESOURCES_RCC}
		${trace_server_SOURCES})
TARGET_LINK_LIBRARIES(trace_server ${QT_LIBRARIES})

