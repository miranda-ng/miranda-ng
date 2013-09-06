@echo off
rem Get archives if needed
cls
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
cls
:again1
set /p answ=Do you want to compile the installers now? (Y/N):
if /i "%answ:~,1%" EQU "Y" goto compile
if /i "%answ:~,1%" EQU "N" goto moveahead
echo Please type Y for Yes or N for No
goto again1
:compile
echo Compiling!
call compile.bat
:moveahead
rem end

rem Cleanup
cls
:again2
set /p ans=Do you want to delete temp files and build folders? (Y/N):
if /i "%ans:~,1%" EQU "Y" goto cleanup
if /i "%ans:~,1%" EQU "N" goto end
echo Please type Y for Yes or N for No
goto again2
:cleanup
echo Running cleanup!
call cleanup.bat
:end
rem end