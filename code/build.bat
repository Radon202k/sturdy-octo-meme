@echo off
set flags=-WX -W4 -wd4189 -wd4100 -wd4505 -wd4201 -nologo -FC -MTd -Z7
IF NOT EXIST ..\bin mkdir ..\bin
pushd ..\bin
cl %flags% ..\code\win32_test.cpp
popd