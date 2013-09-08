@echo off
if not exist MirandaNG.iss goto end
set GetVer=for /F "tokens=1,2,3 delims= " %%i in (build.no) do set MirVer=%%i.%%j.%%k
for %%* in (.) do set InnoX=%%~n*
rem Get version
if exist ..\..\..\build\build.no goto localgetver
..\Tools\wget.exe -O build.no http://svn.miranda-ng.org/main/trunk/build/build.no
%GetVer%
del /F /Q build.no
goto esclocal
:localgetver
pushd ..\..\..\build
%GetVer%
popd
:esclocal
rem end

rem Create Inno setup compatible script that compiles without passing parameters to compiler
if %InnoX% == InnoNG_32 (
  if exist MirandaNG32.iss del /F /Q MirandaNG32.iss
  ..\Tools\bom_remove.exe MirandaNG.iss
  echo #define ptx86 > MirandaNG32.iss
  echo #define AppVer "%MirVer%" >> MirandaNG32.iss
  echo.>> MirandaNG32.iss
  type MirandaNG.iss >>  MirandaNG32.iss
  ..\Tools\bom_add.exe MirandaNG32.iss
  ..\Tools\bom_add.exe MirandaNG.iss
) else if %InnoX% == InnoNG_64 (
  if exist MirandaNG64.iss del /F /Q MirandaNG64.iss
  ..\Tools\bom_remove.exe MirandaNG.iss
  echo #define AppVer "%MirVer%" > MirandaNG64.iss
  echo.>> MirandaNG64.iss
  type MirandaNG.iss >> MirandaNG64.iss
  ..\Tools\bom_add.exe MirandaNG64.iss
  ..\Tools\bom_add.exe MirandaNG.iss
)
rem end

rem Delete bat file
del "%~f0
rem end
:end