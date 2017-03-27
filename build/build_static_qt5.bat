rem QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
rem QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
rem

set PATH=c:/Program Files (x86)/Git/bin;%PATH%
set PATH=C:/strawberry/perl/bin;%PATH%
set PATH=c:\devel\qt5\qtbase\gnuwin32\bin;%PATH%
set PATH=C:\Python34;%PATH%
git clone git://code.qt.io/qt/qt5.git qt5
cd qt5
perl init-repository --module-subset=default

rem developpers:
rem perl init-repository --no-webkit --codereview-user mojmir.svoboda
rem update repo:
rem
rem git pull
rem git submodule sync
rem git submodule update --recursive

rem clean:
rem git submodule foreach --recursive "git clean -dfx"

configure.bat -debug-and-release -opensource -static -nomake examples -nomake tests

rem profile
rem configure -force-debug-info -debug-and-release -opensource -platform win32-msvc2012 -static -no-ltcg -no-accessibility -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -nomake demos -nomake examples -nomake tests -no-nis -angle -no-directwrite -no-qml-debug -no-icu

rem nmake
