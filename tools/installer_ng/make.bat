@echo off
rem Get version
if not exist tmp mkdir tmp
wget -O tmp\build.no http://svn.miranda-ng.org/main/trunk/build/build.no
pushd tmp
for /F "tokens=1,2,3 delims= " %%i in (build.no) do set MirVer=%%i.%%j.%%k
popd
rem end

rem Get archives if needed
echo ------------------------------------------------------------------------------------
echo ------------------------------------------------------------------------------------
:again
set /p answer=Do you want to create folder structure and download new components? (Y/N):
if /i "%answer:~,1%" EQU "Y" goto download
if /i "%answer:~,1%" EQU "N" goto chk32
echo Please type Y for Yes or N for No
goto again
:chk32
if not exist InnoNG_32 (goto error) else (goto chk64)
:chk64
if not exist InnoNG_64 (goto error) else (goto continue)
:error
echo Some of the components are missing, please run script again and agree to create folder structure and download new components!
pause
goto end
:download
echo Creating folders and downloading components!
call createstructure.bat
:continue
rem end

rem Make
if defined ProgramFiles(x86) (
	"%ProgramFiles(x86)%\Inno Setup 5\ISCC.exe" /Dptx86 /DAppVer=%MirVer% /O"Output" "InnoNG_32\MirandaNG.iss"
	"%ProgramFiles(x86)%\Inno Setup 5\ISCC.exe" /DAppVer=%MirVer% /O"Output" "InnoNG_64\MirandaNG.iss"
) else (
	"%ProgramFiles%\Inno Setup 5\ISCC.exe" /Dptx86 /DAppVer=%MirVer% /O"Output" "InnoNG_32\MirandaNG.iss"
	"%ProgramFiles%\Inno Setup 5\ISCC.exe" /DAppVer=%MirVer% /O"Output" "InnoNG_64\MirandaNG.iss"
)
rem end

rem Cleanup
echo ------------------------------------------------------------------------------------
echo ------------------------------------------------------------------------------------
:again1
set /p ans=Do you wish to delete temp files and build folders? (Y/N):
if /i "%ans:~,1%" EQU "Y" goto cleanup
if /i "%ans:~,1%" EQU "N" goto end
echo Please type Y for Yes or N for No
goto again1
:cleanup
echo Running cleanup!
call cleanup.bat
:end