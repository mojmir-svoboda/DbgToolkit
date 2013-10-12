@echo off

set VER_SRC="%2version.cpp"
IF EXIST %VER_SRC% (
	echo "removing old file"
	del %VER_SRC%
) ELSE (
	REM Do another thing
)

call %1 describe > ver.tmp
set /p myvar= < ver.tmp
del ver.tmp
echo "creating version file"

echo #include "version.h" > %VER_SRC%
set /p foo="char const g_Version[] = "<nul >> %VER_SRC%
set /p foo=""%myvar%""<nul >>%VER_SRC%
echo ; >>%VER_SRC%

