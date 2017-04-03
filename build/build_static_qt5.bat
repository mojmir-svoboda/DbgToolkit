rem QMAKE_CXXFLAGS_RELEASE = $$QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO
rem QMAKE_LFLAGS_RELEASE = $$QMAKE_LFLAGS_RELEASE_WITH_DEBUGINFO
rem

set PATH=c:/Program Files (x86)/Git/bin;%PATH%
set PATH=C:/strawberry/perl/bin;%PATH%
rem set PATH=k:\qt5\qtbase\bin;%PATH%
rem set PATH=c:\devel\qt5\qtbase\gnuwin32\bin;%PATH%
set PATH=C:\Python34;%PATH%
git clone git://code.qt.io/qt/qt5.git qt5
cd qt5
perl init-repository --module-subset=default

rem developpers:
rem perl init-repository --no-webkit --codereview-user mojmir.svoboda

rem update repo:
rem git pull
rem perl init-repository -f --module-subset=default

rem clean:
rem git submodule foreach --recursive "git clean -dfx" && git clean -dfx

rem qt 5.3.x
rem configure.bat -debug-and-release -opensource -platform win32-msvc2012 -static -ltcg -no-accessibility -no-openvg -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -no-qml-debug -nomake examples -nomake tests -no-icu
rem qt 5.8.x
configure.bat -debug-and-release -developer-build -opensource -static -nomake examples -nomake tests
rem configure.bat -debug-and-release -opensource -platform win32-msvc2017 -static -ltcg -no-sql-sqlite -no-qml-debug -nomake examples -nomake tests -no-icu
rem configure.bat -developer-build  -debug-and-release -opensource -platform win32-msvc2017 -static -nomake examples -nomake tests
rem configure.bat -confirm-license -developer-build -opensource -nomake examples -nomake tests -qt-harfbuzz
rem configure.bat -confirm-license -opensource -nomake examples -nomake tests -qt-harfbuzz  -static -mp -debug-and-release

rem profile
rem configure -force-debug-info -debug-and-release -opensource -platform win32-msvc2012 -static -no-ltcg -no-accessibility -no-gif -no-dbus -no-audio-backend -no-sql-sqlite -nomake demos -nomake examples -nomake tests -no-nis -angle -no-directwrite -no-qml-debug -no-icu

rem nmake
