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

set FullProfileName=%ProfileDir%\%ProfileName%\%ProfileName%.dat

del "%FullProfileName%.bak" > nul
copy "%FullProfileName%" "%FullProfileName%.bak"

mdbx_chk.exe -i -w -v -t -1 "%FullProfileName%"

echo Operation succeeded
goto :eof
