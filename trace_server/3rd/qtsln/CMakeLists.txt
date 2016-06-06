project(qtsln)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
set(CMAKE_AUTOMOC ON)

set(qtsln_SOURCES
	qtcolorpicker/qtcolorpicker.cpp
	flowlayout.cpp
	)

set(qtsln_HEADERS
	qtcolorpicker/qtcolorpicker.h
	flowlayout.h
	)

#qt5_wrap_cpp(qtsln_MOC ${qtsln_HEADERS})
#qt4_add_resources(qtsln_RESOURCES_RCC ${qtsln_RESOURCES})
include_directories(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
include_directories(${CMAKE_BINARY_DIR})

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_STATIC)
add_definitions(-DQT_NO_PRINTER)
add_definitions(-DSTATIC)

add_library (qtsln STATIC ${qtsln_SOURCES})
set_target_properties(qtsln PROPERTIES AUTOMOC TRUE)
set_property(TARGET qtsln PROPERTY FOLDER "3rd")
qt5_use_modules(qtsln Widgets Gui Core)
