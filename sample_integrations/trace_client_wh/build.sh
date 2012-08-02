#!/bin/bash

. ~/.vs10
cd _project
devenv.com wh_client.sln /project trace_client_wh.vcxproj /build Debug
cd -
