@echo off

for /F "tokens=1,2 delims==" %%a in ('findstr "ProfileDir=" mirandaboot.ini') do (
  call set ProfileDir=%%b
)

if "%ProfileDir%"=="" set ProfileDir=%~dp0Profiles

echo Using profile directory %ProfileDir%

if "%1" == "" (
  pushd "%ProfileDir%"

  for /D %%i in (*) do (
    if exist "%%i\%%i.dat" set ProfileName=%%i
  )

  popd

  if "!ProfileName!" == "" (
     echo No default profile found, exiting
     goto :eof
  )

) else (
  if not exist "%ProfileDir%\%1\%1.dat" (
    echo Wrong profile name specified: %1
    goto :eof
  )

  set ProfileName=%1
)

echo Backing up %ProfileName%...

set FullProfileName=%ProfileDir%\%ProfileName%\%ProfileName%.dat
set TmpFileName=%TEMP%\%ProfileName%.tmp

del "%TmpFileName%" > nul

mdbx_dump.exe -n -a -f "%TmpFileName%" "%FullProfileName%"
if not exist "%TmpFileName%" (
  echo Backup failed, exiting
  goto :eof
)

mdbx_load.exe -n -a -f "%TmpFileName%" "%FullProfileName%.tmp"
if errorlevel 1 (
  echo Restore failed, exiting
  goto :eof
)

del "%FullProfileName%.bak" > nul
move "%FullProfileName%" "%FullProfileName%.bak"
move "%FullProfileName%.tmp" "%FullProfileName%"

echo Operation succeeded
goto :eof
