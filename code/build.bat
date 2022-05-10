@echo off

REM DEBUG BUILD
set flags=-Od -MTd -Z7 

REM RELEASE BUILD
REM set flags=-Ot -O2 -EHa- -GR- -MT -Oi

IF NOT EXIST ..\bin mkdir ..\bin
pushd ..\bin
cl -WX -W4 -wd4189 -wd4100 -wd4505 -wd4201 -nologo -FC %flags% ..\code\win32_test.cpp /link -opt:ref -incremental:no
popd