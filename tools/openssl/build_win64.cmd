@echo off

rd /s /q out32dll
rd /s /q tmp32dll

perl Configure shared VC-WIN64A

call %VS141COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars64.bat 
nmake clean
nmake 
