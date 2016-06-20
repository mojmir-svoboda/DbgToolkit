cmake_minimum_required(VERSION 2.8.9)
project(trace_server)
set (CMAKE_VERBOSE_MAKEFILE on)
#include("${CMAKE_CURRENT_SOURCE_DIR}/cotire.cmake")
include("${CMAKE_SOURCE_DIR}/build/msvc_pch.cmake")

# use this for develoment, if you do not pass it from command line:
message("trace_server install path=${CMAKE_INSTALL_PREFIX}")

### directX
if (WIN32)
	include("${CMAKE_CURRENT_SOURCE_DIR}/FindDirectX.cmake")
endif (WIN32)

### Qt
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Multimedia REQUIRED)
find_program(QT_QMAKE_EXECUTABLE NAMES qmake qmake5 qmake-qt5 qmake-mac PATHS ${QTROOTS})
message("qmake: ${QT_QMAKE_EXECUTABLE}")

get_filename_component(QT_QMAKE_PATH ${QT_QMAKE_EXECUTABLE} PATH)
get_filename_component(QT_PLUGINS_PATH "${QT_QMAKE_PATH}/../plugins" ABSOLUTE)
get_filename_component(QT_LIB_PATH "${QT_QMAKE_PATH}/../lib" ABSOLUTE)
message("qt lib path: ${QT_LIB_PATH}")
message("qt plugins path: ${QT_PLUGINS_PATH}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_PREFIX_PATH "${QTDIR}; ${WINSDK_LIB}")

include("${CMAKE_CURRENT_SOURCE_DIR}/FindPlugins.cmake")

set(LIBRARY_OUTPUT_PATH
   ${PROJECT_BINARY_DIR}/Bin
   CACHE PATH
   "Single Directory for all Libraries"
   )

set(EXECUTABLE_OUTPUT_PATH
   ${PROJECT_BINARY_DIR}/Bin
   CACHE PATH
   "Single Directory for all Executables."
   )

### boost
set(Boost_DEBUG 0)
set(Boost_USE_STATIC_LIBS 1)
find_package(Boost)
if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
	add_definitions( "-DHAS_BOOST" )

	set(BOOST_LIBS thread date_time system regex serialization filesystem)
	find_package(Boost COMPONENTS ${BOOST_LIBS} REQUIRED)
endif()
message("boost: ${Boost_LIBRARIES}")

# 3rd party components
add_subdirectory("3rd")

### trace server files

add_definitions("-DTRACE_ENABLED")
add_definitions("-DTRACE_STATIC")
add_definitions("-DUSE_CXX_ALLOCATOR")

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
set(trace_server_SOURCES
	main.cpp
	types.cpp
	qt_plugins.cpp
	decoder_alloc.cpp
	server/server.cpp
	mainwindow.cpp

	mainwindow_presets.cpp
	mainwindow_controlwidget.cpp
	mainwindow_connection.cpp
	mainwindow_csv.cpp
	mainwindow_docking.cpp
	mainwindow_find.cpp

	widgets/tableview.cpp
	widgets/treeview.cpp

	utils/delegates.cpp
	connection.cpp
	connectionconfig.cpp
	connection_stream.cpp
	connection_config.cpp
	connection_controlwidget.cpp
	connection_csv.cpp
	connection_sound.cpp
	constants.cpp
	config.cpp

	dock/dock.cpp
	dock/dockwidget.cpp
	dock/dockdelegates.cpp
	dock/dockmanagerconfig.cpp
	dock/dockmanager.cpp
	dock/dockmanagerview.cpp
	dock/dockmanagermodel.cpp

	models/basetablemodel.cpp
	models/baseproxymodel.cpp
	models/sparseproxymodel.cpp
	models/treeproxy.cpp
	models/filterproxymodel.cpp

	syncwidgets.cpp
	widgets/combolist.cpp
	widgets/movabletabwidget.cpp
	widgets/timecombobox.cpp
	widgets/buttoncache.cpp
	widgets/controlbar/controlbarcommon.cpp
	widgets/controlbar/controlbardockedwidgets.cpp
	widgets/controlbar/controlbarlog.cpp
	widgets/controlbar/controlwidgetdata.cpp
	widgets/setupdialogcsv.cpp

	filters/filterbase.cpp
	filters/filtermgrbase.cpp
	widgets/logs/filters/filtermgr.cpp
	widgets/logs/filters/filter_script.cpp
	widgets/logs/filters/filter_string.cpp
	#widgets/logs/filters/filter_regex.cpp
	widgets/logs/filters/filter_ctx.cpp
	widgets/logs/filters/filter_lvl.cpp
	widgets/logs/filters/filter_tid.cpp
	widgets/logs/filters/filter_fileline.cpp
	widgets/logs/filters/filter_row.cpp
	widgets/logs/filters/filter_time.cpp
	#filters/colorizers/filter_function.cpp
	#filters/colorizers/filter_dt.cpp
	filters/colorizers/colorizermgr.cpp
	#filters/colorizers/colorizer_script.cpp
	filters/colorizers/colorizer_string.cpp
	filters/colorizers/colorizer_regex.cpp
	#filters/colorizers/colorizer_ctx.cpp
	#filters/colorizers/colorizer_lvl.cpp
	#filters/colorizers/colorizer_tid.cpp
	#filters/colorizers/colorizer_fileline.cpp
	filters/colorizers/colorizer_row.cpp
	#filters/colorizers/colorizer_time.cpp
	#filters/colorizers/colorizer_function.cpp
	#filters/colorizers/colorizer_dt.cpp
	filters/colorizers/colorizer_regex.cpp
	widgets/logs/sounds/soundmgr.cpp
	widgets/logs/sounds/sound_regex.cpp

	widgets/warnimage.cpp
	widgets/mixer.cpp
	widgets/mixerbutton.cpp
	widgets/mixerbar.cpp
	widgets/mixerbarv.cpp
	widgets/findwidget.cpp
	widgets/colorizewidget.cpp
	widgets/findconfig.cpp
	widgets/colorizeconfig.cpp

	connection_log.cpp
	widgets/logs/logtablemodel.cpp
	widgets/logs/logfilterproxymodel.cpp
	widgets/logs/logtableview.cpp
	widgets/logs/findproxymodel.cpp
	widgets/logs/logselectionproxymodel.cpp
	widgets/logs/logwidget.cpp
	widgets/logs/logwidget_find.cpp
	widgets/logs/logwidget_colorize.cpp
	widgets/logs/logwidget_filtering.cpp
	widgets/logs/logwidget_legacycfg.cpp
	widgets/logs/logwidget_actions.cpp
	widgets/logs/logwidget_view.cpp
	widgets/logs/logconfig.cpp
	widgets/logs/logctxmenu.cpp
	widgets/logs/loghheaderctxmenu.cpp
	widgets/logs/logwidget_tools.cpp
	widgets/logs/logdelegate.cpp

	connection_plot.cpp
	widgets/plot/plotwidget.cpp
	widgets/plot/plotconfig.cpp
	widgets/plot/plottypes.cpp

	connection_table.cpp
	widgets/table/editableheaderview.cpp
	widgets/table/tableconfig.cpp
	widgets/table/tablemodel.cpp
	widgets/table/tablewidget.cpp
	widgets/table/tablewidget_filtering.cpp

	connection_gantt.cpp
	connection_frame.cpp
	widgets/gantt/arrow.cpp
	widgets/gantt/scrollzoomer.cpp
	widgets/gantt/scrollbar.cpp
	widgets/gantt/frameview.cpp
	widgets/gantt/frameviewconfig.cpp
	widgets/gantt/ganttconfig.cpp
	widgets/gantt/ganttitem.cpp
	widgets/gantt/ganttview.cpp
	widgets/gantt/ganttwidget.cpp
	widgets/gantt/histogram.cpp
	)

set(trace_server_HEADERS
  action.h
  config.h
  connection.h
  connectionconfig.h
  constants.h
  dock/dock.h
  dock/dockdelegates.h
  dock/dockedconfig.h
  dock/dockedinfo.h
  dock/dockedwidgets.h
  dock/dockmanager.h
  dock/dockmanagerconfig.h
  dock/dockmanagermodel.h
  dock/dockmanagerview.h
  dock/dockwidget.h
  filters/colorizers/colorizermgr.h
  filters/colorizers/colorizer_regex.h
  filters/colorizers/colorizer_row.h
  filters/colorizers/colorizer_string.h
  filters/file_filter.hpp
  filters/filterbase.h
  widgets/logs/filters/filtermgr.h
  filters/filtermgrbase.h
  filters/filtersctxmenu.h
  widgets/logs/filters/filter_ctx.h
  widgets/logs/filters/filter_fileline.h
  widgets/logs/filters/filter_lvl.h
	#widgets/logs/filters/filter_regex.h
  widgets/logs/filters/filter_row.h
  widgets/logs/filters/filter_script.h
  widgets/logs/filters/filter_string.h
  widgets/logs/filters/filter_tid.h
  widgets/logs/filters/filter_time.h
	widgets/logs/sounds/soundmgr.h
	widgets/logs/sounds/sound_regex.h
  filters/nnode.hpp
  mainwindow.h
  models/baseproxymodel.h
  models/basetablemodel.h
  models/filterproxymodel.h
  models/movablelistmodel.h
  models/sparseproxymodel.h
  models/treemodel.h
  models/treeproxy.h
  platform.h
  qt_plugins.h
  serialize/serialize.h
  serialize/ser_qbytearray.h
  serialize/ser_qcolor.h
  serialize/ser_qlist.h
  serialize/ser_qregexp.h
  serialize/ser_qstring.h
  serialize/ser_qstringlist.h
  serialize/ser_qt.h
  serialize/ser_qvector.h
  server/server.h
  syncwidgets.h
  types.h
  utils/delegates.h
  utils/find_utils_table.h
  utils/hash.h
  utils/history.h
  utils/hsv.h
  utils/set_with_blocked_signals.h
  utils/utils.h
  utils/utils_boost.h
  utils/utils_color.h
  utils/utils_file_filters.h
  utils/utils_history.h
  utils/utils_openfilelinewith.h
  utils/utils_qsettings.h
  utils/utils_qstandarditem.h
  utils/utils_std.h
  widgets/buttoncache.h
  widgets/colorizeconfig.h
  widgets/colorizewidget.h
  widgets/combolist.h
  widgets/controlbar/controlbarcommon.h
  widgets/controlbar/controlbardockedwidgets.h
  widgets/controlbar/controlbargantts.h
  widgets/controlbar/controlbarlog.h
	widgets/controlbar/controlwidgetdata.h
  widgets/controlbar/controlbarlogs.h
  widgets/controlbar/controlbarplots.h
  widgets/controlbar/controlbartables.h
  widgets/controlbar/controlbar_dockmanager.h
  widgets/findconfig.h
  widgets/findwidget.h
  widgets/gantt/arrow.h
  widgets/gantt/frameview.h
  widgets/gantt/frameviewconfig.h
  widgets/gantt/frameviewctxmenu.h
  widgets/gantt/ganttconfig.h
  widgets/gantt/ganttctxmenu.h
  widgets/gantt/ganttdata.h
  widgets/gantt/ganttitem.h
  widgets/gantt/ganttview.h
  widgets/gantt/ganttwidget.h
  widgets/gantt/histogram.h
  widgets/gantt/scalewidget.h
  widgets/gantt/scrollbar.h
  widgets/gantt/scrollzoomer.h
  widgets/help.h
  widgets/label.h
  widgets/mixer.h
  widgets/mixerbutton.h
  widgets/mixerbar.h
  widgets/mixerbarv.h
  widgets/logs/findproxymodel.h
  widgets/logs/logconfig.h
  widgets/logs/logctxmenu.h
  widgets/logs/loghheaderctxmenu.h
  widgets/logs/logdelegate.h
  widgets/logs/logfile.h
  widgets/logs/logfile_protocol.h
  widgets/logs/logfilterproxymodel.h
  widgets/logs/logselectionproxymodel.h
  widgets/logs/logtablemodel.h
  widgets/logs/logtableview.h
  widgets/logs/logwidget.h
  widgets/logs/tls.h
  widgets/movabletabwidget.h
  widgets/plot/curves.h
  widgets/plot/plotconfig.h
  widgets/plot/plotctxmenu.h
  widgets/plot/plottypes.h
  widgets/plot/plotwidget.h
  widgets/setupdialogcsv.h
  widgets/table/editableheaderview.h
  widgets/table/tableconfig.h
  widgets/table/tablectxmenu.h
  widgets/table/tablemodel.h
  widgets/table/tablewidget.h
  widgets/tableview.h
  widgets/timecombobox.h
  widgets/treeview.h
  widgets/warnimage.h
  winpch.h
	)

set(trace_server_UI
	mainwindow.ui
	widgets/controlbar/controlbarcommon.ui
	widgets/controlbar/controlbardockedwidgets.ui
	widgets/controlbar/controlbarlog.ui
	widgets/controlbar/controlwidgetdata.ui
	widgets/findwidget.ui
	widgets/mixer.ui
	widgets/mixerbar.ui
	widgets/mixerbarv.ui
	widgets/colorizewidget.ui
	widgets/combolist.ui
	widgets/timecombobox.ui
	widgets/setupdialogcsv.ui
	widgets/settings.ui
	widgets/logs/settingslog.ui
	widgets/logs/loghheaderctxmenu.ui
	widgets/plot/settingsplot.ui
	widgets/table/settingstable.ui
	widgets/gantt/settingsgantt.ui
	widgets/gantt/settingsframeview.ui
	widgets/help.ui
	widgets/logs/filters/filter_string.ui
	#widgets/logs/filters/filter_regex.ui
	widgets/logs/filters/filter_script.ui
	widgets/logs/filters/filter_tid.ui
	widgets/logs/filters/filter_lvl.ui
	widgets/logs/filters/filter_ctx.ui
	widgets/logs/filters/filter_fileline.ui
	widgets/logs/filters/filter_row.ui
	widgets/logs/filters/filter_time.ui
	filters/colorizers/colorizer_regex.ui
	filters/colorizers/colorizer_row.ui
	filters/colorizers/colorizer_string.ui
	widgets/logs/sounds/sound_regex.ui
	)

set(trace_server_RESOURCES resources/resources.qrc)

if (WIN32)
	set(trace_server_WIN32_RESOURCES resources/resources_win.rc)
	set(CMAKE_RC_COMPILER_INIT windres)
	enable_language(RC)
endif (WIN32)

qt5_wrap_ui(trace_server_UI_HDRS ${trace_server_UI})
qt5_wrap_cpp(trace_server_MOC ${trace_server_HEADERS})
qt5_add_resources(trace_server_RESOURCES_RCC ${trace_server_RESOURCES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm300 /bigobj")
ADD_MSVC_PRECOMPILED_HEADER("winpch.h" "winpch.cpp" trace_server_SOURCES)

add_executable(trace_server WIN32
		${trace_server_UI_HDRS}
		${trace_server_UI_SRCS}
		${trace_server_RESOURCES_RCC}
		${trace_server_SOURCES}
    ${trace_server_HEADERS}
		${trace_server_WIN32_RESOURCES}
)

####################### Platform specific ######################
if (WIN32)
	set (QT_USE_QTMAIN TRUE)
	add_definitions(-DNOMINMAX)
	add_definitions(-DWIN_LEAN_AND_MEAN)
	target_link_libraries(trace_server Qt5::WinMain)
endif (WIN32)
####################### Platform specific ######################

include_directories(${QT_USE_FILE} ${Qt5Gui_PRIVATE_INCLUDE_DIRS})
add_definitions(${QT_DEFINITIONS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/trace_proto)
include_directories(${trace_server_SOURCE_DIR})

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_STATIC)
add_definitions(-DQT_NO_SVGRENDERER)
add_definitions(-DSTATIC)


######################## Plugin section ########################
# Locate static plugins
add_definitions(-DQT_STATICPLUGIN)
get_filename_component(QT_IMG_PLUGINS_PATH "${QT_PLUGINS_PATH}/imageformats" ABSOLUTE)
get_filename_component(QT_PLATFORM_PLUGINS_PATH "${QT_PLUGINS_PATH}/platforms" ABSOLUTE)
get_filename_component(QT_AUDIO_PLUGINS_PATH "${QT_PLUGINS_PATH}/audio" ABSOLUTE)

FIND_PLUGINS(QICO          NAMES qico                   DEFAULT_SUFFIXES PATHS ${QT_IMG_PLUGINS_PATH})
message("plugin ico: ${QICO_LIBRARIES}")
target_link_libraries(trace_server ${QICO_LIBRARIES})

if (WIN32)
	FIND_PLUGINS(QWINDOWS      NAMES qwindows               DEFAULT_SUFFIXES PATHS ${QT_PLATFORM_PLUGINS_PATH})
  FIND_PLUGINS(QAUDIOWINDOWS NAMES qtaudio_windows        DEFAULT_SUFFIXES PATHS ${QT_AUDIO_PLUGINS_PATH})
	FIND_PLUGINS(QPLATFORM     NAMES Qt5PlatformSupport     DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
	FIND_PLUGINS(QTRANSLATOR   NAMES translator             DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
	FIND_PLUGINS(QTRANS_COMMON NAMES translator_common      DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
	FIND_PLUGINS(QTRANS_HLSL   NAMES translator_hlsl        DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
	FIND_PLUGINS(QPREPROC      NAMES preprocessor           DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
	message("QPA plugin for win: ${QWINDOWS_LIBRARIES}")

	FIND_LIBRARY(DXGUID_LIBRARY      NAMES dxguid      PATHS ${DirectX_LIBRARY_PATHS})
	FIND_LIBRARY(D3DCOMPILER_LIBRARY NAMES d3dcompiler PATHS ${DirectX_LIBRARY_PATHS})
	message("DirectX: ${DXGUID_LIBRARY} ${D3DCOMPILER_LIBRARY}")

	if(QTRANSLATOR_LIBRARY)
		set(TRANSLATOR_LIBRARIES ${TRANSLATOR_LIBRARIES} ${QTRANSLATOR_LIBRARY})
	endif()
	if(QTRANS_COMMON_LIBRARY)
		set(TRANSLATOR_LIBRARIES ${TRANSLATOR_LIBRARIES} ${QTRANS_COMMON_LIBRARY})
	endif()
	if(QTRANS_HLSL_LIBRARY)
		set(TRANSLATOR_LIBRARIES ${TRANSLATOR_LIBRARIES} ${QTRANS_HLSL_LIBRARY})
	endif()

  FIND_PLUGINS(QPCRE         NAMES qtpcre                DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
  FIND_PLUGINS(QHARFBUZZ     NAMES qtharfbuzzng          DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})
  FIND_PLUGINS(QFREETYPE     NAMES qtfreetype            DEFAULT_SUFFIXES PATHS ${QT_LIB_PATH})

	target_link_libraries(trace_server
		wsock32 ws2_32 Iphlpapi
		imm32 winmm d3d9 ${DXGUID_LIBRARY} ${D3DCOMPILER_LIBRARY} dwrite
		${QPLATFORM_LIBRARY}
		${TRANSLATOR_LIBRARIES}
		${QPREPROC_LIBRARY}
		${QWINDOWS_LIBRARY}
    ${QPCRE_LIBRARY}
    ${QHARFBUZZ_LIBRARY}
    ${QFREETYPE_LIBRARY}
    ${QAUDIOWINDOWS_LIBRARY} Strmiids
    )
elseif (UNIX)
	message("not implemented yet, but should be trivial to do so")
elseif (APPLE)
	message("not implemented yet, but should be trivial to do so")
endif()
######################## Plugin section ########################

qt5_use_modules(trace_server Widgets Core Gui Network OpenGL Multimedia)

target_link_libraries(trace_server ${Boost_LIBRARIES}
	qwt
	qtsln
	kde
	${Qt5Gui_EGL_LIBRARIES}
	${Qt5Gui_OPENGL_LIBRARIES}
	${Qt5Widgets_LIBRARIES}
	${QICO}
	Qt5::WinMain)

target_link_libraries(trace_server trace_proto trace_version)

install(TARGETS trace_server
	RUNTIME DESTINATION .
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib/static)

if(MSVC)
  add_definitions("-D_SCL_SECURE_NO_WARNINGS")
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	#install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../utils/bin/VisualStudioFileOpenTool.all.exe
	#	DESTINATION .)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/trace_server.pdb
		DESTINATION .
		CONFIGURATIONS Debug)
endif(MSVC)

#cotire(trace_server)

#if (WIN32 AND NOT MINGW)
#	get_filename_component(MK_VERSION_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../build/mk_version.bat" ABSOLUTE)
#	message("mk_version.bat: ${MK_VERSION_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/")
#	add_custom_command(TARGET trace_server PRE_BUILD
#		COMMAND cmd /c ${MK_VERSION_PATH} ${GIT_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/)
#endif (WIN32 AND NOT MINGW)



############################# debug ############################
#message("Generated headers: ${trace_server_UI_HDRS}")
#message("Generated files ${CMAKE_BINARY_DIR}")
#message("Generated files trace_server_MOC ${trace_server_MOC}")
#message("Generated files trace_server_UI_HDRS ${trace_server_UI_HDRS}")
#message("Generated files trace_server_UI_SRCS ${trace_server_UI_SRCS}")
#message("Generated files trace_server_RESOURCES_RCC ${trace_server_RESOURCES_RCC}")
#message("Generated files trace_server_SOURCES ${trace_server_SOURCES}")

#message("Debug variable dump:")
#get_cmake_property(_variableNames VARIABLES)
#foreach (_variableName ${_variableNames})
#	message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

