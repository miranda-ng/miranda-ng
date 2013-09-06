@echo off
if not exist MirandaNG.iss goto end
set GetVer=for /F "tokens=1,2,3 delims= " %%i in (build.no) do set MirVer=%%i.%%j.%%k
set BomRem=http://miranda-ng.org/distr/installer/bom_remove.exe
set BomAdd=http://miranda-ng.org/distr/installer/bom_add.exe
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
  ..\Tools\wget.exe -O utf-8_32.vbs http://miranda-ng.org/distr/installer/utf-8_32.vbs
  ..\Tools\wget.exe -O bom_remove.exe %BomRem%
  ..\Tools\wget.exe -O bom_add.exe %BomAdd%
  bom_remove.exe MirandaNG.iss
  cscript utf-8_32.vbs
  echo #define AppVer "%MirVer%" >> MirandaNG32.iss
  echo.>> MirandaNG32.iss
  type MirandaNG.iss >>  MirandaNG32.iss
  bom_add.exe MirandaNG.iss
  del /f /q utf-8_32.vbs
  del /f /q bom_remove.exe
  del /f /q bom_add.exe
) else if %InnoX% == InnoNG_64 (
  ..\Tools\wget.exe -O utf-8_64.vbs http://miranda-ng.org/distr/installer/utf-8_64.vbs
  ..\Tools\wget.exe -O bom_remove.exe %BomRem%
  ..\Tools\wget.exe -O bom_add.exe %BomAdd%
  bom_remove.exe MirandaNG.iss
  cscript utf-8_64.vbs
  echo #define AppVer "%MirVer%" >> MirandaNG64.iss
  echo.>> MirandaNG64.iss
  type MirandaNG.iss >> MirandaNG64.iss
  bom_add.exe MirandaNG.iss
  del /f /q utf-8_64.vbs
  del /f /q bom_remove.exe
  del /f /q bom_add.exe
)
rem end

rem Delete bat file
del "%~f0
rem end
:end