rem QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
rem QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
rem

set PATH=c:/Program Files (x86)/Git/bin;%PATH%
set PATH=C:/strawberry/perl/bin;%PATH%
set PATH=C:\devel\QtSDK\src\qt5\gnuwin32\bin;%PATH%
git clone git://gitorious.org/qt/qt5.git qt5
rem cd qt5
rem perl init-repository --no-webkit
rem configure.bat -debug-and-release -opensource -platform win32-msvc2012 -static -ltcg -no-accessibility -no-openvg -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -no-qml-debug -nomake demos -nomake examples -nomake tests

rem profile
rem configure -force-debug-info -debug-and-release -opensource -platform win32-msvc2012 -static -no-ltcg -no-accessibility -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -nomake demos -nomake examples -nomake tests -no-nis -angle -directwrite -no-qml-debug 

rem release
rem configure -force-debug-info -debug-and-release -opensource -platform win32-msvc2012 -static -ltcg -no-accessibility -no-openvg -no-libtiff -no-libmng -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -no-phonon -no-multimedia -no-webkit -nomake demos -nomake examples -nomake tests
rem
rem 


rem *  -no-directwrite .... Do not build support for DirectWrite font rendering.
rem    -directwrite ....... Build support for DirectWrite font rendering 
rem                         (experimental, requires DirectWrite availability on 
rem                         target systems, e.g. Windows Vista with Platform 
rem                         Update, Windows 7, etc.)

rem nmake
