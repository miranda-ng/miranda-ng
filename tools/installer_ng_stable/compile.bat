@echo off
rem Get version
if not exist tmp mkdir tmp
Tools\wget.exe -O tmp\build.no http://svn.miranda-ng.org/main/trunk/build/build.no
pushd tmp
for /F "tokens=1,2 delims= " %%i in (build.no) do set ver1=%%i.%%j
for /F "tokens=3 delims= " %%k in (build.no) do (set /a "ver2=%%k-1")
set Mirver=%ver1%.%ver2%
popd
rem end

rem Download and extract compiler
Tools\wget.exe -O tmp\InnoSetup5.7z http://miranda-ng.org/distr/installer/InnoSetup5.7z
..\7-zip\7z.exe x tmp\InnoSetup5.7z -y -oTools
rem end

rem Set compiler variables
set Compile32=Tools\InnoSetup5\ISCC.exe /Dptx86 /DAppVer=%MirVer% "InnoNG_32\MirandaNG.iss"
set Compile64=Tools\InnoSetup5\ISCC.exe /DAppVer=%MirVer% "InnoNG_64\MirandaNG.iss"
rem end

rem Get archives if needed
if not exist InnoNG_32 call createstructure.bat
if not exist InnoNG_64 call createstructure.bat
rem end

rem Make
%Compile32%
%Compile64%
rem end