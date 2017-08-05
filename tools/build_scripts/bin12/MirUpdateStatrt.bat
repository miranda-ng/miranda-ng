Set Release=Release
Set MirDir=E:\MirandaNG\

for %%i IN (1) do call taskkill /T /IM miranda32.exe
if %errorlevel% ==0 (goto :EXIT) else (goto :ACT)
:EXIT
timeout 1
for %%i IN (1) do call taskkill /T /IM miranda32.exe
if %errorlevel% ==0 goto :EXIT

:ACT
pushd %Release%
rd /s /q OBJ >nul
popd
xcopy /S /Y "%Release%\*.dll" "%MirDir%"
xcopy /S /Y "%Release%\*.mir" "%MirDir%"
copy /V /Y "%Release%\Plugins\tox.ini" "%MirDir%\Plugins\"
copy /V /Y "%Release%\miranda*.exe" "%MirDir%"
del /F /S /Q "%Release%\*.dll" >> nul
del /F /S /Q "%Release%\*.mir" >> nul
del /F /Q "%Release%\miranda*.exe"
del /F /Q "%Release%\Plugins\tox.ini"

rem rd /S /Q "Release"

pushd %MirDir%
call rebaser.cmd
start /b miranda32.exe
cd Languages\Lang
svn update
popd