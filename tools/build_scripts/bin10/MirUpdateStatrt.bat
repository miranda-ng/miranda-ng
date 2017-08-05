for %%i IN (1) do call taskkill /T /IM miranda32.exe
if %errorlevel% ==0 (goto :EXIT) else (goto :ACT)
:EXIT
timeout 1
for %%i IN (1) do call taskkill /T /IM miranda32.exe
if %errorlevel% ==0 goto :EXIT

:ACT
pushd Release
rd /s /q OBJ >nul
popd
xcopy /S /Y "Release\*.dll" "E:\MirandaNG\"
xcopy /S /Y "Release\*.mir" "E:\MirandaNG\"
copy /V /Y "Release\Plugins\tox.ini" "E:\MirandaNG\Plugins\"
copy /V /Y "Release\miranda32.exe" "E:\MirandaNG\"
del /F /S /Q "Release\*.dll" >> nul
del /F /S /Q "Release\*.mir" >> nul
del /F /Q "Release\miranda32.exe"

rd /S /Q "Release" >nul

pushd E:\MirandaNG
call rebaser.cmd
start /b miranda32.exe
cd Languages\Lang
svn update
popd