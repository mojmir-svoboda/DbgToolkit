#!/bin/bash

VERSION=`git describe`

VER_SRC=../version.cpp
if [[ -e "$VER_SRC" ]]; then
	rm -v $VER_SRC;
fi

echo "#include \"version.h\"" > $VER_SRC
echo "char const g_Version[] = \"${VERSION}\";" >> $VER_SRC
