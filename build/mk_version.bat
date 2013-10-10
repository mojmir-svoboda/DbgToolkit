@echo off

set VER_SRC="..\version.cpp"
IF EXIST %VER_SRC% (
	echo "removing old file"
	del %VER_SRC%
) ELSE (
	REM Do another thing
)

echo "creating version file"

echo #include "version.h" > %VER_SRC%
>> %VER_SRC% echo char const g_Version[] = "
>> %VER_SRC% git describe
>> %VER_SRC% echo ";
