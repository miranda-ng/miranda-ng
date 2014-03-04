@echo off

call "C:\Program Files\Microsoft Visual Studio .NET 2003\Common7\Tools\vsvars32.bat"

:BuildAnsi
echo.
echo Preparing ANSI build...
echo.
if not exist ansi mkdir ansi
if not exist ansi\docs mkdir ansi\docs
del /q ansi\*.*
del /q ansi\docs\*.*
devenv /rebuild "Release" ..\historystats.sln
copy ..\release\historystats.dll ansi\
copy ..\docs\historystats_changelog.txt ansi\docs\
copy ..\docs\historystats_readme.txt ansi\docs\
copy ..\langpack\historystats_langpack_template.txt ansi\docs\
copy ..\include\m_historystats.h ansi\docs\

:BuildUnicode
echo.
echo Prepartin Unicode build...
echo.
if not exist unicode mkdir unicode
if not exist unicode\docs mkdir unicode\docs
del /q unicode\*.*
del /q unicode\docs\*.*
devenv /rebuild "Unicode Release" ..\historystats.sln
copy ..\releaseu\historystats.dll unicode\
copy ..\docs\historystats_changelog.txt unicode\docs\
copy ..\docs\historystats_readme.txt unicode\docs\
copy ..\langpack\historystats_langpack_template.txt unicode\docs\
copy ..\include\m_historystats.h unicode\docs\

echo.
echo Done.
echo.
pause > nul
