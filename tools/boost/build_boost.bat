@echo off
set p1=%1
set p2=%2
if  "%p1%" == "" (echo "please specify target platform by adding '86' or '64' parameter to command line!'"&&pause&&goto :EOF)
if  "%p2%" == "" (echo "please specify target msvc compiler by adding '14.1' for MSVC2017 to command line!'"&&pause&&goto :EOF)

call bootstrap.bat

if "%p2%" == "14.1" (
   set VCRoot=%VS141COMNTOOLS%
) else (
   set VCRoot=%VS142COMNTOOLS%
)

if /i '%1' == '86' (
	call "%VCRoot%\..\..\VC\Auxiliary\Build\vcvars32.bat"
	b2 --toolset=msvc-%2 runtime-link=shared threading=multi link=static architecture=x86 address-model=32 cxxflags=/Zc:threadSafeInit-
) else if /i '%1' == '64' (
	call "%VCRoot%\..\..\VC\Auxiliary\Build\vcvars64.bat"
	b2 --toolset=msvc-%2 runtime-link=shared threading=multi link=static architecture=x86 address-model=64 cxxflags=/Zc:threadSafeInit- --stagedir=stage64
)