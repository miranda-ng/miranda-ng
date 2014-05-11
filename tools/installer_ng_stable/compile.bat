@echo off
rem Get version
if not exist tmp mkdir tmp
if exist tmp\build.no goto mkver
Tools\wget.exe -O tmp\build.no http://svn.miranda-ng.org/main/trunk/build/build.no
:mkver
pushd tmp
for /F "tokens=1,2 delims= " %%i in (build.no) do set ver1=%%i.%%j
for /F "tokens=3 delims= " %%k in (build.no) do (set /a "ver2=%%k-1")
set Mirver=%ver1%.%ver2%
popd
rem end

rem Set compiler variables
set Compile32=Tools\InnoSetup5\ISCC.exe /Dptx86 /DAppVer=%MirVer% /O"Output" "InnoNG_32\MirandaNG.iss"
set Compile64=Tools\InnoSetup5\ISCC.exe /DAppVer=%MirVer% /O"Output" "InnoNG_64\MirandaNG.iss"
rem end

:check32
if not exist InnoNG_32 (goto compileerror) else (goto check64)
:check64
if not exist InnoNG_64 (goto compileerror) else (goto compile)

rem Make
:compile
%Compile32%
%Compile64%
rem end

rem Error handling
if errorlevel 1 goto :compileerror
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
goto check32
pause
:end
rem end