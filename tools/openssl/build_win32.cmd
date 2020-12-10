@echo off

cd /d "%TEMP%"
rd /s /q openssl
mkdir openssl
cd /d openssl

set LDFLAGS=/nologo /debug /SUBSYSTEM:CONSOLE",5.01"
perl "%OPENSSL_ROOT_DIR%\Configure" shared VC-WIN32 /D\"_USING_V110_SDK71_\" /D\"_WIN32_WINNT=0x0501\"

call %VS141COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars32.bat 
nmake 
