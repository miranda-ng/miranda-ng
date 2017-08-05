call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat"

call git_update.bat

if exist "Release" rd /Q /S "Release" >nul

if exist "..\include\m_version.h" del /F /Q "..\include\m_version.h"
echo on
pushd ..\build
call make_ver.bat
popd

MsBuild.exe "plugs_vad.sln" /m /t:Rebuild /p:Configuration=Release /p:Platform="Win32" /fileLogger /fileLoggerParameters:LogFile=Logs\plugs_vad.log;errorsonly;warningsonly

call vad.bat

rd /S /Q "Release\Obj"

del /F /S /Q "Release\*.pdb"

popd