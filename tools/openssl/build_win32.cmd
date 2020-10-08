@echo off

rd /s /q out32dll
rd /s /q tmp32dll

set LDFLAGS=/nologo /debug /SUBSYSTEM:CONSOLE",5.01"
perl Configure shared VC-WIN32 /D\"_USING_V110_SDK71_\" /D\"_WIN32_WINNT=0x0501\"

call %VS141COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars32.bat 
nmake clean
nmake 
