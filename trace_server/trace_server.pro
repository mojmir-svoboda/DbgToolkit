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
INCUDEPATH += c:/devel/boost_1_47_0

win32-msvc* {
	DEFINES += QT_NO_OPENGL
	#LIBS += c:/devel/boost_1_49_0/stage/lib/libboost_system-vc100-mt-1_49.lib
	#LIBS += c:/devel/boost_1_49_0/stage/lib/libboost_date_time-vc100-mt-1_49.lib
	#LIBS += c:/devel/boost_1_49_0/stage/lib/libboost_thread-vc100-mt-1_49.lib
	#LIBS += c:/devel/boost_1_49_0/stage/lib/libboost_regex-vc100-mt-1_49.lib
	LIBS += c:/devel/boost_1_47_0/stage/lib/libboost_system-vc100-mt-1_47.lib
	LIBS += c:/devel/boost_1_47_0/stage/lib/libboost_date_time-vc100-mt-1_47.lib
	LIBS += c:/devel/boost_1_47_0/stage/lib/libboost_thread-vc100-mt-1_47.lib
	LIBS += c:/devel/boost_1_47_0/stage/lib/libboost_regex-vc100-mt-1_47.lib

	#LIBS += c:/devel/boost_1_48_0/stage/lib/libboost_system-vc100-mt-1_48.lib
	#LIBS += c:/devel/boost_1_48_0/stage/lib/libboost_date_time-vc100-mt-1_48.lib
	#LIBS += c:/devel/boost_1_48_0/stage/lib/libboost_thread-vc100-mt-1_48.lib
	#LIBS += c:/devel/boost_1_48_0/stage/lib/libboost_regex-vc100-mt-1_48.lib

	#LIBS += c:/devel/boost_1_48_0/stage/lib/libboost_system-vc100-mt-gd-1_48.lib
	#LIBS += c:/devel/boost_1_48_0/stage/lib/libboost_date_time-vc100-mt-gd-1_48.lib
	#LIBS += c:/devel/boost_1_48_0/stage/lib/libboost_thread-vc100-mt-gd-1_48.lib
	#LIBS += c:/devel/boost_1_48_0/stage/lib/libboost_regex-vc100-mt-gd-1_48.lib

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
	main.cpp\
	mainwindow.cpp \
    modelview.cpp \
    server.cpp \
    filterproxy.cpp \
    connection.cpp \
    connection_find.cpp \
    connection_stream.cpp \
    connection_setup.cpp \
    connection_filtering.cpp \
    connection.cpp \
    sessionstate.cpp \
    settings.cpp	\
	rvps.cpp	\
	profilerconnection.cpp \
	profilerbar.cpp	\
	profilerconnection.cpp	\
	profilergraphicsview.cpp	\
	profilerview.cpp	\
	profilerwindow.cpp

HEADERS  += ../version.h	\
	mainwindow.h \
    modelview.h \
    server.h \
    connection.h \
    filterproxy.h \
    ../tlv_parser/tlv_parser.h \
    ../tlv_parser/tlv_encoder.h \
    ../tlv_parser/tlv_decoder.h \
    ../filters/nnode.hpp \
    ../filters/file_filter.hpp \
    ../trace_client/trace.h \
    sessionstate.h \
    settings.h	\
	rvps.h	\
	rendezvous.h	\
	profilerbar.h	\
	profilerblockinfo.h	\
	profilerconnection.h	\
	profilergraphicsview.h	\
	profilerserver.h	\
	profilerview.h	\
	profilerwindow.h

FORMS    += mainwindow.ui \
    settings.ui \
    help.ui

RESOURCES += \
    resources.qrc
