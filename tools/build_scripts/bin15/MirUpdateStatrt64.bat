Set Release=Release64
Set MirDir=E:\MirandaNG\

for %%i IN (1) do call taskkill /T /IM miranda64.exe
if %errorlevel% ==0 (goto :EXIT) else (goto :ACT)
:EXIT
timeout 1
for %%i IN (1) do call taskkill /T /IM miranda64.exe
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

REM rd /S /Q "Release"

REM pushd %MirDir%
REM call rebaser.cmd
REM start /b miranda64.exe
REM cd Languages\Lang
REM svn update
REM popd

pushd "..\langpacks\tool"
LangpackSuite.exe \q \prussian \o"=MINE=.txt" \n%MirDir%\Languages\Langpack_russian.txt
popd

pushd %MirDir%
call rebaser.cmd
start /b miranda64.exe
popd