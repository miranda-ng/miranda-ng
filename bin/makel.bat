@echo off

for /F "tokens=1,2,3 delims= " %%i in (build.no) do call :LPGen %%i %%j %%k
if errorlevel 1 goto :Error

for /F "tokens=1,2,3 delims= " %%i in (build.no) do call :Pack %%i %%j %%k
if errorlevel 1 goto :Error

goto :eof

:LPGen
pushd ..\..\miranda-tools\lpgen
call perl lpgen.pl version %1 %2 %3
popd
goto :eof

:Pack
if %2 == 00 (
   set FileVer=v%1a%3.zip
) else (
   set FileVer=v%1%2a%3.zip
)

del /Q /F "%Temp%\miranda-langpack-%FileVer%"
7z.exe a -tzip -r -mx=9 "%Temp%\miranda-langpack-%FileVer%" ..\i18n\langpack_english.txt

goto :eof

:Error
echo Language pack generation failed
goto :eof
