
set PATH=c:\Program Files (x86)\Git\bin;%PATH%
set PATH=C:\strawberry\perl\bin;%PATH%
set PATH=c:\qt5\qtbase\gnuwin32\bin;%PATH%
set PATH=C:\Python34;C:\Python33;%PATH%

cd c:\devel

git clone git://code.qt.io/qt/qt5.git

cd qt5

perl init-repository @-skip qtwebkit@

set QTDIR=c:\devel\qt5
set PATH=%QTDIR%\bin;%PATH%
set QMAKESPEC=win32-msvc2015

pause

call configure.bat -debug-and-release -opensource -confirm-license -c++11 -platform win32-msvc2015 -static -ltcg -qt-pcre -audio-backend -no-icu -no-accessibility -no-openvg -no-dbus -no-qml-debug -no-sql-mysql -no-sql-psql -no-sql-sqlite -no-gif -qt-libpng -qt-libjpeg -no-openssl -no-compile-examples -no-openvg -nomake examples -nomake tests -skip qtwebkit -skip qtwebsockets -skip qtwebkit-examples -skip qtwebchannel  -skip qtwebengine -skip qtwayland -skip qtsvg -skip qtsensors -skip qtcanvas3d -skip qtconnectivity -skip declarative -skip quick1


