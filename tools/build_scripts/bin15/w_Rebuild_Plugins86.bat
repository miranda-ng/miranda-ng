call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"

call git_update.bat

if exist "Release" rd /Q /S "Release" >nul

if exist "..\include\m_version.h" del /F /Q "..\include\m_version.h"
echo on
pushd ..\build
call make_ver.bat
popd

MsBuild.exe "plugs_vad.sln" /m /t:Rebuild /p:Configuration=Release /p:Platform="Win32" /fileLogger /fileLoggerParameters:LogFile=Logs\plugs_vad.log;errorsonly;warningsonly
set BUILD_STATUS=%ERRORLEVEL%
if %BUILD_STATUS%==0 echo Build success
if not %BUILD_STATUS%==0 goto :Error

call vad.bat

rd /S /Q "Release\Obj"

..\tools\installer_ng_stable\Tools\wget.exe -P Release/Libs -r -np -nd -nH -R html -e robots=off http://www.miranda-ng.org/distr/build/x86/

rem del /F /S /Q "Release\*.pdb"

rem popd

goto :eof

:Error
echo ============================= FAIL! =============================
pause
exit
