project(kde)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
set(CMAKE_AUTOMOC ON)

set(kde_SOURCES
	kselectionproxymodel.cpp
	kmodelindexproxymapper.cpp
	klinkitemselectionmodel.cpp
	)

set(kde_HEADERS
	kbihash_p.h
	kmodelindexproxymapper.h
	kselectionproxymodel.h
	kvoidpointerfactory_p.h
	klinkitemselectionmodel.h
	)

#qt5_wrap_cpp(kde_MOC ${kde_HEADERS})
include_directories(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_STATIC)
add_definitions(-DQT_NO_PRINTER)
add_definitions(-DSTATIC)

add_library (kde STATIC ${kde_SOURCES})
set_target_properties(kde PROPERTIES AUTOMOC TRUE)
set_property(TARGET kde PROPERTY FOLDER "3rd")
qt5_use_modules(kde Widgets Gui Core)
