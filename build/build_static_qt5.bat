rem QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
rem QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
rem

set PATH=c:/Program Files (x86)/Git/bin;%PATH%
set PATH=C:/strawberry/perl/bin;%PATH%
set PATH=e:\qt5.1\qtbase\gnuwin32\bin;%PATH%
git clone git://gitorious.org/qt/qt5.git qt5
rem cd qt5
rem perl init-repository --no-webkit --codereview-user mojmir.svoboda
rem rem configure.bat -debug-and-release -opensource -platform win32-msvc2012 -static -ltcg -no-accessibility -no-openvg -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -no-qml-debug -nomake demos -nomake examples -nomake tests -no-icu

rem profile
rem configure -force-debug-info -debug-and-release -opensource -platform win32-msvc2012 -static -no-ltcg -no-accessibility -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -nomake demos -nomake examples -nomake tests -no-nis -angle -no-directwrite -no-qml-debug -no-icu

rem nmake
