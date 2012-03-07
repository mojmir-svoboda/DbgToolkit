cmake_minimum_required(VERSION 2.8)
PROJECT(profile_server)
FIND_PACKAGE(Qt4 REQUIRED)

include("${CMAKE_CURRENT_SOURCE_DIR}/FindReleaseAndDebug.cmake")

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

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "~/devel/boost_1_48_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "~/devel/boost_1_48_0")

add_definitions("-DPROFILE_ENABLED")
add_definitions("-DPROFILE_STATIC")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package (Threads)

FIND_PACKAGE(Boost)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

set(BOOST_LIBS thread date_time system)
find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)

find_package (Flogging REQUIRED)
message ("Current dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message ("Flog includes: ${FLOGGING_INCLUDE_DIRS}")
message ("Flog srcs: ${FLOGGING_SOURCE_DIRS}")

include_directories (${FLOGGING_INCLUDE_DIRS})

SET(profile_server_SOURCES
	main.cpp
	mainwindow.cpp
    connection.cpp
	view.cpp
	bar.cpp)

SET(profile_server_HEADERS
	mainwindow.h 
    server.h
    connection.h
	view.h
	bar.h)

#SET(profile_server_UI mainwindow.ui settings.ui)
SET(profile_server_RESOURCES resources.qrc)
SET(QT_USE_QTNETWORK true)
SET(QT_USE_QTOPENGL true)
IF (WIN32)
	SET (QT_USE_QTMAIN TRUE)
ENDIF (WIN32) 

#QT4_WRAP_UI(profile_server_UI_HDRS ${profile_server_UI})
QT4_WRAP_CPP(profile_server_MOC ${profile_server_HEADERS})
QT4_ADD_RESOURCES(profile_server_RESOURCES_RCC ${profile_server_RESOURCES})
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_STATIC)
ADD_DEFINITIONS(-DSTATIC)
message("Plugin libs ${QT_PLUGINS_DIR}")


macro(QT_ADD_IMG_PLUGINS target)

  IF(QT_CONFIG MATCHES "static")
    add_definitions(-DQT_STATICPLUGIN)
   
    # Links against the static plugins
    FIND_RELEASE_AND_DEBUG(QJPEG qjpeg qjpegd "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QGIF qgif qgifd "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QMNG qmng qmngd "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QICO qico qicod "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QSVG qsvg qsvgd "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QTIFF qtiff qtiffd "${QT_PLUGINS_DIR}/imageformats")
    FIND_RELEASE_AND_DEBUG(QSVGR QtSvg QtSvgd "C:/devel/QtSDK/QtSources/4.7.4/lib")		# @TODO: fixed path

	message("ico: ${QICO_LIBRARY}")
	message("svg: ${QSVG_LIBRARY}")
	message("svgr: ${QSVGR_LIBRARY}")
  
    target_link_libraries(${target} ${QJPEG_LIBRARY} ${QGIF_LIBRARY}
      ${QMNG_LIBRARY} ${QICO_LIBRARY} ${QTIFF_LIBRARY} ${QSVG_LIBRARY} ${QSVGR_LIBRARY})
  
  endif(QT_CONFIG MATCHES "static")

endmacro(QT_ADD_IMG_PLUGINS)

message("Generated headers: ${profile_server_UI_HDRS}")
message("Generated files ${CMAKE_BINARY_DIR}")
message("Generated files profile_server_MOC ${profile_server_MOC}")
message("Generated files profile_server_UI_HDRS ${profile_server_UI_HDRS}")
message("Generated files profile_server_UI_SRCS ${profile_server_UI_SRCS}")
message("Generated files profile_server_RESOURCES_RCC ${profile_server_RESOURCES_RCC}")
message("Generated files profile_server_SOURCES ${profile_server_SOURCES}")

#add_library (fprof_client_lib STATIC "${FLOGGING_SOURCE_DIRS}/trace_client/profile.cpp")

ADD_EXECUTABLE(profile_server WIN32
		${profile_server_MOC}
		#		${profile_server_UI_HDRS}
		#${profile_server_UI_SRCS}
		${profile_server_RESOURCES_RCC}
		${profile_server_SOURCES})

QT_ADD_IMG_PLUGINS(profile_server)
TARGET_LINK_LIBRARIES(profile_server ${QT_LIBRARIES} ${QICO} ${QSVG} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})



