QT          += widgets designer

QTDIR_build {
# This is only for the Qt build. Do not use externally. We mean it.
PLUGIN_TYPE = designer
PLUGIN_CLASS_NAME = FindBoxPlugin
load(qt_plugin)
} else {
# Public example:

CONFIG      += plugin
TEMPLATE    = lib

TARGET = $$qtLibraryTarget($$TARGET)

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target

}

HEADERS     = findbox.h \
              findboxplugin.h
SOURCES     = findbox.cpp \
              findboxplugin.cpp
OTHER_FILES += findbox.json
