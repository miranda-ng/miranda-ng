@echo off

if "%1" == "" (
  set /A ProfileCount=0

  pushd %~dp0Profiles

  for /D %%i in (*) do (
    if exist "%~dp0Profiles\%%i\%%i.dat" (
      set ProfileName=%%i
      set /A ProfileCount=%ProfileCount%+1
    )
  )

  popd

) else (
  if not exist "%~dp0Profiles\%1\%1.dat" (
    echo Wrong profile name specified: %1
    goto :eof
  )

  set ProfileName=%1
)

echo Backing up %ProfileName%...

set FullProfileName=.\Profiles\%ProfileName%\%ProfileName%.dat
set TmpFileName=%TEMP%\%ProfileName%.tmp

del "%TmpFileName%" > nul

mdbx_dump.exe -n -a -f "%TmpFileName%" "%FullProfileName%"
if not exist "%TmpFileName%" (
  echo Backup failed, exiting
  goto :eof
)

mdbx_load.exe -n -f "%TmpFileName%" "%FullProfileName%.tmp"
if errorlevel 1 (
  echo Restore failed, exiting
  goto :eof
)

del "%FullProfileName%.bak" > nul
move "%FullProfileName%" "%FullProfileName%.bak"
move "%FullProfileName%.tmp" "%FullProfileName%"

echo Operation succeeded
goto :eof
