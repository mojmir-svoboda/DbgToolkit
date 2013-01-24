cmake_minimum_required(VERSION 2.8)
PROJECT(trace_server)
set (CMAKE_VERBOSE_MAKEFILE on)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5OpenGL REQUIRED)
set(CMAKE_AUTOMOC ON)

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

#SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "c:/devel/boost_1_47_0")
#SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "c:/devel/boost_1_47_0")

add_subdirectory("qwt")
add_subdirectory("qtsln")

add_definitions("-DTRACE_ENABLED")
add_definitions("-DTRACE_STATIC")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..)

set(Boost_dir "c:/devel/boost/boost_1_52_0")
set(Boost_DEBUG 1)
set(Boost_USE_STATIC_LIBS 1)
FIND_PACKAGE(Boost)
IF (Boost_FOUND)
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
	ADD_DEFINITIONS( "-DHAS_BOOST" )

	set(BOOST_LIBS thread date_time system regex serialization)
	find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)
ENDIF()


SET(trace_server_SOURCES
	main.cpp
	../version.cpp
	mainwindow.cpp
	mainwindow_presets.cpp
	treeview.cpp
	treemodel.cpp
	modelview.cpp
	tableview.cpp
	delegates.cpp
	server.cpp
	server_filtering.cpp
	filterproxy.cpp
	connection.cpp
	connection_find.cpp
	connection_stream.cpp
	connection_plot.cpp
	connection_table.cpp
	connection_setup.cpp
	connection_filtering.cpp
	sessionstate.cpp
	settings.cpp
	rvps.cpp
	statswindow.cpp
	profilerconnection.cpp
	profilerbar.cpp
	profilergraphicsview.cpp
	profilerview.cpp
	profilerwindow.cpp
	profilermainwindow.cpp
	baseplot.cpp
	simpleplot.cpp
	pickableplot.cpp
	serialization.cpp
	constants.cpp
	config.cpp
	plotconfig.cpp
	plottypes.cpp
	basetable.cpp
	tableconfig.cpp
	tablemodelview.cpp
	dock.cpp
	editableheaderview.cpp
	sparseproxy.cpp
	)

SET(trace_server_HEADERS
	mainwindow.h 
	treeview.h
	treemodel.h
	modelview.h
	tableview.h
	server.h
	connection.h
	filterproxy.h
	../tlv_parser/tlv_parser.h
	../tlv_parser/tlv_encoder.h
	../tlv_parser/tlv_decoder.h
	../filters/nnode.hpp
	../filters/file_filter.hpp
	sessionstate.h
	settings.h
	statswindow.h
	delegates.h
	profilerconnection.h
	profilerbar.h
	profilerconnection.h
	profilergraphicsview.h
	profilerview.h
	profilerwindow.h
	profilermainwindow.h
	curves.h
	baseplot.h
	simpleplot.h
	pickableplot.h
	rvps.h
	rendezvous.h
	serialization.h
	utils.h
	dock.h
	config.h
	plotconfig.h
	constants.h
	plotctxmenu.h
	basetable.h
	tableconfig.h
	tablectxmenu.h
	tablemodelview.h
	editableheaderview.h
	sparseproxy.h
	)

SET(trace_server_UI 
	mainwindow.ui
	settings.ui
	settingsplot.ui
	settingstable.ui
	help.ui
	profilermainwindow.ui
	)

SET(trace_server_RESOURCES resources.qrc)
SET(QT_USE_QTNETWORK true)
SET(QT_USE_QTOPENGL true)
IF (WIN32)
	SET (QT_USE_QTMAIN TRUE)
	ADD_DEFINITIONS(-DNOMINMAX)
	ADD_DEFINITIONS(-DWIN_LEAN_AND_MEAN)
ENDIF (WIN32) 

QT5_WRAP_UI(trace_server_UI_HDRS ${trace_server_UI})
QT5_WRAP_CPP(trace_server_MOC ${trace_server_HEADERS})
QT5_ADD_RESOURCES(trace_server_RESOURCES_RCC ${trace_server_RESOURCES})
INCLUDE_DIRECTORIES(${QT_USE_FILE})
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
	#FIND_RELEASE_AND_DEBUG(QJPEG qjpeg qjpegd "${QT_PLUGINS_DIR}/imageformats")
	#FIND_RELEASE_AND_DEBUG(QGIF qgif qgifd "${QT_PLUGINS_DIR}/imageformats")
	#FIND_RELEASE_AND_DEBUG(QMNG qmng qmngd "${QT_PLUGINS_DIR}/imageformats")
	FIND_RELEASE_AND_DEBUG(QICO qico qicod "${QT_PLUGINS_DIR}/imageformats")
	#FIND_RELEASE_AND_DEBUG(QSVG qsvg qsvgd "${QT_PLUGINS_DIR}/imageformats")
	#FIND_RELEASE_AND_DEBUG(QTIFF qtiff qtiffd "${QT_PLUGINS_DIR}/imageformats")
	#FIND_RELEASE_AND_DEBUG(QSVGR QtSvg QtSvgd "C:/devel/QtSDK/QtSources/4.7.4/lib")		# @TODO: fixed path

	message("ico: ${QICO_LIBRARY}")
	#message("svg: ${QSVG_LIBRARY}")
	#message("svgr: ${QSVGR_LIBRARY}")
	message("boost: ${Boost_LIBRARIES}")
  
	target_link_libraries(${target} ${QJPEG_LIBRARY} ${QGIF_LIBRARY}
	  ${QMNG_LIBRARY} ${QICO_LIBRARY} ${QTIFF_LIBRARY})
	#	 ${QSVG_LIBRARY} ${QSVGR_LIBRARY})
  
  endif(QT_CONFIG MATCHES "static")

endmacro(QT_ADD_IMG_PLUGINS)

message("Generated headers: ${trace_server_UI_HDRS}")
message("Generated files ${CMAKE_BINARY_DIR}")
message("Generated files trace_server_MOC ${trace_server_MOC}")
message("Generated files trace_server_UI_HDRS ${trace_server_UI_HDRS}")
message("Generated files trace_server_UI_SRCS ${trace_server_UI_SRCS}")
message("Generated files trace_server_RESOURCES_RCC ${trace_server_RESOURCES_RCC}")
message("Generated files trace_server_SOURCES ${trace_server_SOURCES}")


ADD_EXECUTABLE(trace_server WIN32
		${trace_server_MOC}
		${trace_server_UI_HDRS}
		${trace_server_UI_SRCS}
		${trace_server_RESOURCES_RCC}
		${trace_server_SOURCES})

QT_ADD_IMG_PLUGINS(trace_server)
#TARGET_LINK_LIBRARIES(trace_server ${QT_LIBRARIES} ${QICO} ${QSVG} ${Boost_LIBRARIES} qwt)
TARGET_LINK_LIBRARIES(trace_server ${QT_LIBRARIES} ${QICO} ${Boost_LIBRARIES} qwt qtsln)

qt5_use_modules(trace_server Widgets Core Gui Network OpenGL)
