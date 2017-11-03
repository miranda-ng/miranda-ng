@echo off

rd /s /q out32dll
rd /s /q tmp32dll

perl Configure VC-WIN64A
call ms\do_win64a

call %VS141COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars64.bat 
nmake -f ms\ntdll.mak clean
nmake -f ms\ntdll.mak

move out32dll\libeay32.dll out32dll\libeay32.mir
move out32dll\ssleay32.dll out32dll\ssleay32.mir