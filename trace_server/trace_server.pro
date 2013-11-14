#-------------------------------------------------
#
# Project created by QtCreator 2011-12-16T12:05:25
#
#-------------------------------------------------

######################################################
######################################################
# better to use CMake. i use these only for ui stuff #
######################################################
######################################################

QT       += core gui network
TARGET = trace_server
TEMPLATE = app
DEFINES += TRACE_ENABLED
INCLUDEPATH += ..
INCLUDEPATH += $(BOOST_ROOT)/

win32-msvc* {
	DEFINES += QT_NO_OPENGL
	LIBS += $(BOOST_ROOT)/stage/lib/libboost_system-vc100-mt-1_49.lib
	LIBS += $(BOOST_ROOT)/stage/lib/libboost_date_time-vc100-mt-1_49.lib
	LIBS += $(BOOST_ROOT)/stage/lib/libboost_thread-vc100-mt-1_49.lib
	LIBS += $(BOOST_ROOT)/stage/lib/libboost_regex-vc100-mt-1_49.lib
	LIBS += $(BOOST_ROOT)/stage/lib/libboost_serialization-vc100-mt-1_49.lib
	#LIBS += c:/devel/boost_1_47_0/stage/lib/libboost_system-vc100-mt-1_47.lib
	#LIBS += c:/devel/boost_1_47_0/stage/lib/libboost_date_time-vc100-mt-1_47.lib
	#LIBS += c:/devel/boost_1_47_0/stage/lib/libboost_thread-vc100-mt-1_47.lib
	#LIBS += c:/devel/boost_1_47_0/stage/lib/libboost_regex-vc100-mt-1_47.lib
}
win32-g++ {
    QMAKE_CXXFLAGS += -Wno-deprecated
    LIBS += c:/devel/QtSDK/mingw/lib/libws2_32.a

    #QMAKE_STRIP = echo
    #QMAKE_LFLAGS_RELEASE =
    #QMAKE_CXXFLAGS += -pg
    #QMAKE_LFLAGS += -pg
}

static { # everything below takes effect with CONFIG += static
    CONFIG += static
    CONFIG += staticlib # this is needed if you create a static library, not a static executable
    DEFINES += STATIC
    QTPLUGIN += qico qsvg
    message("~~~ static build ~~~") # this is for information, that the static build is done
    mac: TARGET = $$join(TARGET,,,_static) #this adds an _static in the end, so you can seperate static build from non static build
    win32: TARGET = $$join(TARGET,,,_static) #this adds an s in the end, so you can seperate static build from non static build
}


SOURCES += ../version.cpp \
    main.cpp \
    mainwindow.cpp \
    server.cpp \
    connection.cpp \
    filterwidget.cpp \
    findwidget.cpp \
    combolist.cpp

HEADERS  += ../version.h \
    mainwindow.h \
    modelview.h \
    server.h \
    connection.h \
    filterwidget.h \
    findwidget.h \
    combolist.h

FORMS    += help.ui \
    mainwindow.ui \
    settings.ui \
    settingslog.ui \
    settingsplot.ui \
    settingstable.ui \
    settingsgantt.ui \
    settingsframeview.ui \
    settingsfilters.ui \
    filterwidget.ui \
    findwidget.ui \
    filter_fileline.ui \
    filter_tid.ui \
    filter_string.ui \
    filter_script.ui \
    filter_regex.ui \
    filter_lvl.ui \
    filter_ctx.ui \
    combolist.ui

RESOURCES += \
    resources.qrc
