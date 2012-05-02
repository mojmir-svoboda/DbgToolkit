#!/bin/bash

. ~/.vs10
cd _proj
#devenv.com parallel_for.sln /project pfor.vcxproj /build Debug 2>&1 | errsvs.awk
devenv.com win_timer.sln /project pfor.vcxproj /build Release 2>&1 | errsvs.awk
cd -
