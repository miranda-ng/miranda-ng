@echo off
setlocal enabledelayedexpansion

set PACKAGE_NAME="tl-parser"
set PACKAGE_TARNAME="tl-parser"
set PACKAGE_VERSION="1.0"
set PACKAGE_STRING="tl-parser %PACKAGE_VERSION:~1%"
set PACKAGE_BUGREPORT=""
set PACKAGE_URL=""

set CONFIG_HEADER_IN=..\..\config.h.in
set CONFIG_HEADER=config.h
set DEFAULT_OUT_FOLDER=build
set OUT_PATH=""
set SOURCE_LIST=()

:rem Parse Args
for %%a in (%*) do (
   if defined arg (
      if /I "-o"=="!arg!" set OUT_PATH=%%a
      set set arg=
   )
   if /I "%%a" geq "-" (
      set arg=%%a
   ) else (
      set set arg=
   )
)
if %OUT_PATH%=="" (
   set OUT_PATH=%DEFAULT_OUT_FOLDER%
)

:rem Make output folder if not exists
if not exist !OUT_PATH! mkdir !OUT_PATH!

:rem Copy source files to the output folder
for %%i in %SOURCE_LIST% do (
   if not exist %OUT_PATH%\%%i (
      echo |set /p=Copying file: %%i	
      copy %%i "%OUT_PATH%\%%i" 1>NUL && (
         echo Done
      ) || (
         echo Failed
      )
   )
)

set OUT_PATH="%OUT_PATH%\%CONFIG_HEADER%"
if exist %OUT_PATH% exit /B 0

:rem Copy config.h.in to the output folder
echo |set /p=Copying file: %CONFIG_HEADER%		
copy %CONFIG_HEADER_IN% %OUT_PATH% 1>NUL && (
  echo Done	
) || (
  echo Failed
  exit /B %ERRORLEVEL%
)

:rem Configure config.h
echo |set /p=Configurating file: config.h	
call :find_and_replace "#undef PACKAGE_BUGREPORT"  "#define PACKAGE_BUGREPORT %PACKAGE_BUGREPORT%"
call :find_and_replace "#undef PACKAGE_NAME"       "#define PACKAGE_NAME %PACKAGE_NAME%"
call :find_and_replace "#undef PACKAGE_STRING"     "#define PACKAGE_STRING %PACKAGE_STRING%"
call :find_and_replace "#undef PACKAGE_TARNAME"    "#define PACKAGE_TARNAME %PACKAGE_TARNAME%"
call :find_and_replace "#undef PACKAGE_URL"        "#define PACKAGE_URL %PACKAGE_URL%"
call :find_and_replace "#undef PACKAGE_VERSION"    "#define PACKAGE_VERSION %PACKAGE_VERSION%"

call :find_and_replace "#undef HAVE_FCNTL_H"       "#define HAVE_FCNTL_H 1"
call :find_and_replace "#undef HAVE_INTTYPES_H"    "#define HAVE_INTTYPES_H 1"
call :find_and_replace "#undef HAVE_LIBZ"          "#define HAVE_LIBZ 1"
call :find_and_replace "#undef HAVE_MALLOC"        "#define HAVE_MALLOC 1"
call :find_and_replace "#undef HAVE_MEMORY_H"      "#define HAVE_MEMORY_H 1"
call :find_and_replace "#undef HAVE_MEMSET"        "#define HAVE_MEMSET 1"
call :find_and_replace "#undef HAVE_REALLOC"       "#define HAVE_REALLOC 1"
call :find_and_replace "#undef HAVE_STDINT_H"      "#define HAVE_STDINT_H 1"
call :find_and_replace "#undef HAVE_STDLIB_H"      "#define HAVE_STDLIB_H 1"
call :find_and_replace "#undef HAVE_STRDUP"        "#define HAVE_STRDUP 1"
call :find_and_replace "#undef HAVE_STRING_H"      "#define HAVE_STRING_H 1"
call :find_and_replace "#undef HAVE_SYS_STAT_H"    "#define HAVE_SYS_STAT_H 1"
call :find_and_replace "#undef HAVE_SYS_TYPES_H"   "#define HAVE_SYS_TYPES_H 1"
call :find_and_replace "#undef STDC_HEADERS"       "#define STDC_HEADERS 1"
call :find_and_replace "#undef inline"             "#define inline __inline"
call :find_and_replace "#undef"                    "// #undef"

if %ERRORLEVEL% == 0 echo Done
if not %ERRORLEVEL% == 0 (
   echo Failed
   if exist %OUT_PATH% del /q /f %OUT_PATH%
)
exit /B %ERRORLEVEL%

:find_and_replace
set FIND_STR=%1
set "FIND_STR=!FIND_STR:~1,-1!"
set REPLACE_STR=%2 %3
set "REPLACE_STR=!REPLACE_STR:~1,-2!"

for /f "delims=" %%i in ('type "%OUT_PATH%" ^| find /n /v "" ^& break ^> "%OUT_PATH%" ') do (
  set "line=%%i"
  setlocal enableDelayedExpansion
  set "line=!line:*]%FIND_STR%=%REPLACE_STR%!"
  set "line=!line:*]=!"
  >>"%OUT_PATH%" echo(!line!
  endlocal
)
exit /B %ERRORLEVEL%
