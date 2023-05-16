@echo off

set outdir=C:\hd-tools

if not exist %outdir% md %outdir%
call build.bat release

move calex.exe %outdir%\calex.exe