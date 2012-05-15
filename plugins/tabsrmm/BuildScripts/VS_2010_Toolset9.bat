@echo off

REM ignore this if you do not have Visual Studio 2010 installed.

echo * ------------------------------------------------- *
echo This builds x86/x64 targets using the Visual C++ 9
echo toolset. The builds will use the msvcr90 runtime, 
echo same as builds created with Visual Studio 2008 SP1.
echo
echo This must be used from a Visual Studio 2010 command
echo line prompt to be found in:
echo
echo Start->Visual Studio 2010->Visual Studio Tools
echo * ------------------------------------------------- *

cd ..
msbuild tabsrmm_10.sln /t:rebuild /p:"Configuration=Release Unicode";platform=Win32;PlatformToolset=v90
msbuild tabsrmm_10.sln /t:rebuild /p:"Configuration=Release Unicode";platform=x64;PlatformToolset=v90
