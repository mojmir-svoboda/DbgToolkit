#!/bin/bash

. ~/.vs10
cd _proj
devenv.com profile_server.sln /project profile_server /build Release
cd -
