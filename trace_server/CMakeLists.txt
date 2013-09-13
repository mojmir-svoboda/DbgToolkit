cmake_minimum_required(VERSION 2.8.9)
PROJECT(trace_server)
set (CMAKE_VERBOSE_MAKEFILE on)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5OpenGL REQUIRED)
set(CMAKE_AUTOMOC ON)
set (CMAKE_PREFIX_PATH "${QTDIR}; ${WINSDK_LIB}")

include("${CMAKE_CURRENT_SOURCE_DIR}/FindPlugins.cmake")

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

set(Boost_DEBUG 0)
set(Boost_USE_STATIC_LIBS 1)
FIND_PACKAGE(Boost)
IF (Boost_FOUND)
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
	ADD_DEFINITIONS( "-DHAS_BOOST" )

	set(BOOST_LIBS thread date_time system regex serialization)
	find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)
ENDIF()
message("boost: ${Boost_LIBRARIES}")


SET(trace_server_SOURCES
	main.cpp
	qt_plugins.cpp
	../version.cpp
	mainwindow.cpp
	mainwindow_presets.cpp
	mainwindow_obsolette.cpp
	mainwindow_connection.cpp
	mainwindow_docking.cpp
	tableview.cpp
	treeview.cpp
	treeproxy.cpp
	delegates.cpp
	server.cpp
	server_filtering.cpp
	filterwidget.cpp
	connection.cpp
	connection_stream.cpp
	connection_setup.cpp
	filterstate.cpp
	filterstate_serialize.cpp
	constants.cpp
	config.cpp
	dock.cpp
	dockconfig.cpp
	syncwidgets.cpp
	findwidget.cpp

	kde/kselectionproxymodel.cpp
	kde/kmodelindexproxymapper.cpp

	connection_log.cpp
	logs/logtablemodel.cpp
	logs/filterproxymodel.cpp
	logs/logwidget.cpp
	logs/logwidget_find.cpp
	logs/logwidget_filtering.cpp
	logs/logconfig.cpp
	logs/logdelegate.cpp

	connection_plot.cpp
	plot/plotwidget.cpp
	plot/plotconfig.cpp
	plot/plottypes.cpp

	connection_table.cpp
	table/editableheaderview.cpp
	table/baseproxymodel.cpp
	table/sparseproxymodel.cpp
	table/tableconfig.cpp
	table/tablemodelview.cpp
	table/tablewidget.cpp

	connection_gantt.cpp
	connection_frame.cpp
	gantt/arrow.cpp
	gantt/scrollzoomer.cpp
	gantt/scrollbar.cpp
	gantt/frameview.cpp
	gantt/frameviewconfig.cpp
	gantt/ganttconfig.cpp
	gantt/ganttitem.cpp
	gantt/ganttview.cpp
	gantt/ganttwidget.cpp
	gantt/histogram.cpp
	)

SET(trace_server_HEADERS
	mainwindow.h 
	tableview.h
	treeview.h
	treemodel.h
	treeproxy.h
	server.h
	connection.h
	../tlv_parser/tlv_parser.h
	../tlv_parser/tlv_encoder.h
	../tlv_parser/tlv_decoder.h
	../filters/nnode.hpp
	../filters/file_filter.hpp
	filterwidget.h
	filterstate.h
	delegates.h
	rendezvous.h
	utils.h
	dock.h
	dockconfig.h
	constants.h
	syncwidgets.h
	config.h
	findwidget.h

	logs/logtablemodel.h
	logs/logwidget.h
	logs/logconfig.h
	logs/logctxmenu.h
	logs/filterproxymodel.h
	logs/logdelegate.h

	kde/kbihash_p.h
	kde/kmodelindexproxymapper.h
	kde/kselectionproxymodel.h
	kde/kvoidpointerfactory_p.h

	plot/curves.h
	plot/plotwidget.h
	plot/plotconfig.h
	plot/plotctxmenu.h

	table/editableheaderview.h
	table/baseproxymodel.h
	table/sparseproxymodel.h
	table/tableconfig.h
	table/tablectxmenu.h
	table/tablemodelview.h
	table/tablewidget.h

	gantt/arrow.h
	gantt/frameview.h
	gantt/frameviewconfig.h
	gantt/frameviewctxmenu.h
	gantt/scrollzoomer.h
	gantt/scrollbar.h
	gantt/ganttconfig.h
	gantt/ganttctxmenu.h
	gantt/ganttdata.h
	gantt/ganttitem.h
	gantt/ganttview.h
	gantt/ganttwidget.h
	gantt/histogram.h
	gantt/scalewidget.h
	)

SET(trace_server_UI 
	mainwindow.ui
	filterwidget.ui
	findwidget.ui
	settings.ui
	settingslog.ui
	settingsplot.ui
	settingstable.ui
	settingsgantt.ui
	settingsframeview.ui
	help.ui
	)

SET(trace_server_RESOURCES resources.qrc)
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
INCLUDE_DIRECTORIES(${trace_server_SOURCE_DIR})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_STATIC)
ADD_DEFINITIONS(-DSTATIC)

set(Qt_DEBUG 1)


message("Qt config: ${QT_CONFIG}")
message("Plugin libs: ${QT_PLUGINS_DIR}")
macro(QT_ADD_IMG_PLUGINS target)
	add_definitions(-DQT_STATICPLUGIN)
   
	# Links against the static plugins
	#FIND_PLUGINS(QICO qico qicod "${QT_PLUGINS_DIR}/imageformats")
	FIND_PLUGINS(QICO NAMES qico qicod DEFAULT_SUFFIXES PATHS "e:/qt5/qtbase/plugins/imageformats")
	#FIND_RELEASE_AND_DEBUG(QWINDOWS qwindows qwindowsd "${QT_PLUGINS_DIR}/platforms")

	message("ico: ${QICO_LIBRARY}")
  
	target_link_libraries(${target} ${QICO_LIBRARY} ${QWINDOWS_LIBRARY})
endmacro(QT_ADD_IMG_PLUGINS)

message("Generated headers: ${trace_server_UI_HDRS}")
message("Generated files ${CMAKE_BINARY_DIR}")
message("Generated files trace_server_MOC ${trace_server_MOC}")
message("Generated files trace_server_UI_HDRS ${trace_server_UI_HDRS}")
message("Generated files trace_server_UI_SRCS ${trace_server_UI_SRCS}")
message("Generated files trace_server_RESOURCES_RCC ${trace_server_RESOURCES_RCC}")
message("Generated files trace_server_SOURCES ${trace_server_SOURCES}")


ADD_EXECUTABLE(trace_server WIN32
		${trace_server_UI_HDRS}
		${trace_server_UI_SRCS}
		${trace_server_RESOURCES_RCC}
		${trace_server_SOURCES})

qt5_use_modules(trace_server Widgets Core Gui Network OpenGL)
QT_ADD_IMG_PLUGINS(trace_server)
#TARGET_LINK_LIBRARIES(trace_server ${QT_LIBRARIES} ${QICO} ${QSVG} ${Boost_LIBRARIES} qwt)
TARGET_LINK_LIBRARIES(trace_server ${Boost_LIBRARIES}
	qwt
	qtsln
	wsock32 ws2_32
	${Qt5Gui_EGL_LIBRARIES}
	${Qt5Gui_OPENGL_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
	imm32
	winmm
	d3d9
	dxguid
	d3dcompiler
	Qt5::WinMain)


#message("Debug variable dump:")
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()
