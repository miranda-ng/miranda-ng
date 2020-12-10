@echo off

cd /d "%TEMP%"
rd /s /q openssl
mkdir openssl
cd /d openssl

perl "%OPENSSL_ROOT_DIR%\Configure" shared VC-WIN64A

call %VS141COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars64.bat 
nmake 
