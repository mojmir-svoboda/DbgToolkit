#!/bin/bash
source vs10.env
mkdir _proj_server
cd _proj_server
cmake -G "Visual Studio 10" ../../trace_server
devenv.com trace_server.sln /build "RelWithDebInfo|Win32" /project trace_server
VERSION=`git describe`
"c:/Program Files/WinRAR/Rar.exe" a -m5 Release_${VERSION}.rar Bin/RelWithDebInfo/trace_server.exe Bin/RelWithDebInfo/trace_server.pdb
