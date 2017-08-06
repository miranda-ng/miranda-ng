call call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"

call git_update.bat

if exist "Release64" rd /Q /S "Release64" >nul

if exist "..\include\m_version.h" del /F /Q "..\include\m_version.h"
echo on
pushd ..\build
call make_ver.bat
popd

MsBuild.exe "plugs_vad.sln" /m /t:Rebuild /p:Configuration=Release /p:Platform="x64" /fileLogger /fileLoggerParameters:LogFile=Logs\plugs_vad.log;errorsonly;warningsonly
set BUILD_STATUS=%ERRORLEVEL%
if %BUILD_STATUS%==0 echo Build success
if not %BUILD_STATUS%==0 goto :Error

call vad64.bat

..\tools\installer_ng_stable\Tools\wget.exe -P Release64/Libs -r -np -nd -nH -R html -e robots=off https://www.miranda-ng.org/distr/build/x64/

rd /S /Q "Release64\Obj"

rem del /F /S /Q "Release64\*.pdb"

rem popd

goto :eof

:Error
echo ============================= FAIL! =============================
pause
exit
