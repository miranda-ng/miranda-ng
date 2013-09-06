@echo off
set GetVer=for /F "tokens=1,2,3 delims= " %%i in (build.no) do set MirVer=%%i.%%j.%%k
rem Get version
if exist ..\..\build\build.no goto localgetver
if not exist tmp mkdir tmp
Tools\wget.exe -O tmp\build.no http://svn.miranda-ng.org/main/trunk/build/build.no
pushd tmp
%GetVer%
popd
goto esclocal
:localgetver
pushd ..\..\build
%GetVer%
popd
:esclocal
rem end

rem Set compiler variables
set Compile32=Inno Setup 5\ISCC.exe" /Dptx86 /DAppVer=%MirVer% /O"Output" "InnoNG_32\MirandaNG.iss"
set Compile64=Inno Setup 5\ISCC.exe" /DAppVer=%MirVer% /O"Output" "InnoNG_64\MirandaNG.iss"
rem end

rem Make
if defined ProgramFiles(x86) (
	"%ProgramFiles(x86)%\%Compile32%
	"%ProgramFiles(x86)%\%Compile64%
) else (
	"%ProgramFiles%\%Compile32%
	"%ProgramFiles%\%Compile64%
)
rem end