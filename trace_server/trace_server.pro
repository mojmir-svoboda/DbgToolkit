#-------------------------------------------------
#
# Project created by QtCreator 2011-12-16T12:05:25
#
#-------------------------------------------------

QT       += core gui network
TARGET = trace_server
TEMPLATE = app
DEFINES += TRACE_ENABLED
INCLUDEPATH += ..
INCLUDEPATH += ../../boost_1_47_0

win32-msvc* {
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


SOURCES += main.cpp\
        mainwindow.cpp \
    modelview.cpp \
    server.cpp \
    connection.cpp \
    sessionstate.cpp \
    settings.cpp

HEADERS  += mainwindow.h \
    modelview.h \
    server.h \
    connection.h \
    ../tlv_parser/tlv_parser.h \
    ../tlv_parser/tlv_encoder.h \
    ../tlv_parser/tlv_decoder.h \
    ../filters/nnode.hpp \
    ../filters/file_filter.hpp \
    ../trace_client/trace.h \
    sessionstate.h \
    settings.h

FORMS    += mainwindow.ui \
    settings.ui

RESOURCES += \
    resources.qrc
