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

:chk32
if not exist InnoNG_32 (goto compileerror) else (goto chk64)
:chk64
if not exist InnoNG_64 (goto compileerror) else (goto compile)

rem Make
:compile
if defined ProgramFiles(x86) (
	"%ProgramFiles(x86)%\%Compile32%
	"%ProgramFiles(x86)%\%Compile64%
) else (
	"%ProgramFiles%\%Compile32%
	"%ProgramFiles%\%Compile64%
)
rem end

rem Error handling
goto end
:compileerror
rem Get archives if needed
cls
:again3
set /p ans1=Something went wrong... Do you want to re-create folder structure and re-download components? (Y/N):
if /i "%ans1:~,1%" EQU "Y" goto download
if /i "%ans1:~,1%" EQU "N" goto end
echo Please type Y for Yes or N for No
goto again3
:download
echo Creating folders and downloading components!
call createstructure.bat
goto chk32
pause
:end
rem end