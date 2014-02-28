@echo off
set p1=%1
set p2=%2
if  "%p1%" == "" (echo "please specify target platform by adding '86' or '64' parameter to command line!'"&&pause&&goto :EOF)
if  "%p2%" == "" (echo "please specify target msvc compiler by adding '10' for MSVC2010 or '12' for MSVC2013 to command line!'"&&pause&&goto :EOF)

call bootstrap.bat

if /i '%1' == '86' (
	b2 --toolset=msvc-%2.0 runtime-link=shared threading=multi link=static architecture=x86 address-model=32
) else if /i '%1' == '64' (
	b2 --toolset=msvc-%2.0 runtime-link=shared threading=multi link=static architecture=x86 address-model=64 --stagedir=stage64
)