echo off

mkdir _project
cd _project

"c:\Program Files (x86)\CMake 2.8\bin\cmake.exe" -G "Visual Studio 10 Win64" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX="W:/code/externals_code/flogging/" ../cmake_dll


echo *************************
echo * output is in directory: _project/
echo *************************
pause
