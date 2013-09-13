#include "findboxplugin.h"
#include "findbox.h"
#include <QtPlugin>

FindBoxPlugin::FindBoxPlugin (QObject * parent)
    : QObject(parent)
{
    initialized = false;
}

void FindBoxPlugin::initialize (QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool FindBoxPlugin::isInitialized () const
{
    return initialized;
}

QWidget * FindBoxPlugin::createWidget (QWidget * parent)
{
    return new FindBox(parent);
}

QString FindBoxPlugin::name () const
{
    return "FindBox";
}

QString FindBoxPlugin::group () const
{
    return "Display Widgets [Examples]";
}

QIcon FindBoxPlugin::icon () const
{
    return QIcon();
}

QString FindBoxPlugin::toolTip () const
{
    return "";
}

QString FindBoxPlugin::whatsThis () const
{
    return "";
}

bool FindBoxPlugin::isContainer () const
{
    return false;
}

QString FindBoxPlugin::domXml () const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"FindBox\" name=\"findBox\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>100</width>\n"
           "    <height>40</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"toolTip\" >\n"
           "   <string>slightly complicated search combobox</string>\n"
           "  </property>\n"
           "  <property name=\"whatsThis\" >\n"
           "   <string>...</string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString FindBoxPlugin::includeFile () const
{
    return "analogclock.h";
}
