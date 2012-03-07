RESOURCES += images.qrc

HEADERS += mainwindow.h view.h bar.h server.h connection.h
SOURCES += main.cpp
SOURCES += mainwindow.cpp view.cpp bar.cpp connection.cpp

INCLUDEPATH += ../..
INCLUDEPATH += ../../boost_1_48_0

contains(QT_CONFIG, opengl):QT += opengl

build_all:!build_pass {
    CONFIG -= build_all
    CONFIG += release
}

# install
target.path = $$[QT_INSTALL_DEMOS]/bar
sources.files = $$SOURCES $$HEADERS $$RESOURCES *.png *.pro *.html *.doc images
sources.path = $$[QT_INSTALL_DEMOS]/bar
INSTALLS += target sources

symbian: include($$QT_SOURCE_TREE/demos/symbianpkgrules.pri)
