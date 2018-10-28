@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
cd %~dp0

IF NOT EXIST build mkdir build
pushd build

set Flags=-MT -nologo -Gm- -GR- -EHsc- -FC -O2 -Oi

cl %Flags% ..\img2braille.c
del img2braille.obj

popd