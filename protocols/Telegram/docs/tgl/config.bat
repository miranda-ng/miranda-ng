@echo off
setlocal enabledelayedexpansion
set PACKAGE_NAME="telegram-cli"
set PACKAGE_TARNAME="telegram-cli"
set PACKAGE_VERSION="1.0"
set PACKAGE_STRING="!PACKAGE_TARNAME:~1,-1! %PACKAGE_VERSION:~1%"
set PACKAGE_BUGREPORT=""
set PACKAGE_URL=""

set SRC_FOLDER=..\..
set SOURCE_LIST=()
set AUTO_FOLDER=%SRC_FOLDER%\auto
set BIN_FOLDER=bin
set LIB_FOLDER=lib
set DEFAULT_OUT_FOLDER=build
set OUT_PATH=

set GEN_TOOL=generate.exe
set PARSER_TOOL=%SRC_FOLDER%\tl-parser\contrib\vs\build\Release\tl-parser.exe

set CONFIG_HEADER_IN=%SRC_FOLDER%\config.h.in
set CONFIG_HEADER=%SRC_FOLDER%\config.h

set /p SCHEME_TL=<%SRC_FOLDER%\scheme.tl
set SCHEME_TL=%SRC_FOLDER%\%SCHEME_TL%

set /p ENCRYPTED_SCHEME_TL=<%SRC_FOLDER%\encrypted_scheme.tl
set ENCRYPTED_SCHEME_TL=%SRC_FOLDER%\%ENCRYPTED_SCHEME_TL%

set BINLOG_TL=%SRC_FOLDER%\binlog.tl
set MTPROTO_TL=%SRC_FOLDER%\mtproto.tl
set APPEND_TL=%SRC_FOLDER%\append.tl
set CONSTANTS_HEADER=constants.h


::#################################################################################################################################################
:parse_args
if not "%1"=="" (
    if "%1"=="-o" (
        set OUT_PATH=%2
    )
    if "%1"=="--gen-config" (
        set /A GEN_CONFIG=1
    )
    if "%1"=="--gen-scheme" (
        set /A GEN_SCHEME=1
    )
    if "%1"=="-debug" (
        set /A DEBUG=1
        set PARSER_TOOL=%PARSER_TOOL:Release=Debug%
    )
    if "%1"=="--parser-path" (
        set PARSER_TOOL=%2
    )
    if "%1"=="--help" (
       echo TGL Config
       echo Usage:
       echo    --gen-config		generate config.h
       echo    --gen-scheme     	generate Telegram API scheme headers and source files
       echo    --parser-path	path to tl-parser.exe
       echo    -debug		debuge mode
       exit /b 0
    )
  
    shift
    goto :parse_args
)

if {%OUT_PATH%}=={} (
   set OUT_PATH=%DEFAULT_OUT_FOLDER%
)
::Make output folder if not exists
if not exist %OUT_PATH% mkdir %OUT_PATH%


::Copy source files to the output folder
for %%i in %SOURCE_LIST% do (
   if not exist %OUT_PATH%\%%i (
      echo |set /p=Copying file: %%i	
      copy %%i %OUT_PATH%\%%i 1>NUL  && (
         echo Done
      ) || (
         echo Failed
      )
   )
)

::Generate config.h
if defined GEN_CONFIG (
  if not exist %CONFIG_HEADER%	  call :gen_config %CONFIG_HEADER%
)


::Generate Telegram API scheme source files
if defined GEN_SCHEME (
  set /a ERR_ENC=0
  
  ::Check if tl-parser.exe exists
  if not exist %PARSER_TOOL% (
    echo Error: Cannot find %PARSER_TOOL%
    exit /b 1
  )

  ::Check if generate.exe exists
  set "GEN_TOOL=%OUT_PATH%\%BIN_FOLDER%\%GEN_TOOL%"
  if not exist !GEN_TOOL! (
    echo Error: Cannot find !GEN_TOOL!
    exit /b 1
  )

  ::Make auto folder
  if not exist %AUTO_FOLDER%   mkdir %AUTO_FOLDER%
  
  ::Generate scheme.tl
  echo |set /p=Generating scheme.tl			
  type %SCHEME_TL% %ENCRYPTED_SCHEME_TL% %ENCRYPTED_SCHEME% %BINLOG_TL% %MTPROTO_TL% %APPEND_TL% 2>NUL > %AUTO_FOLDER%\scheme.tl
  echo Done

  ::Generate scheme2.tl
  echo |set /p=Generating scheme2.tl			
  %PARSER_TOOL% -E %AUTO_FOLDER%\scheme.tl 2> %AUTO_FOLDER%\scheme2.tl
  if not !ERRORLEVEL!==0 (
    echo Failed
    exit /b 1
  ) else (
    echo Done
  )
  
  ::Generate scheme.tlo
  echo |set /p=Generating scheme.tlo			
  "%PARSER_TOOL%" -e "%AUTO_FOLDER%\scheme.tlo" %AUTO_FOLDER%\scheme.tl
  if not !ERRORLEVEL!==0 (
    echo Failed
    exit /b 1
  ) else (
    echo Done
  )

  ::Generate constants.h
  echo |set /p=Generating constants.h			
  call :gen_constants %AUTO_FOLDER%\scheme2.tl > %AUTO_FOLDER%\constants.h
  if not !ERRORLEVEL!==0 (
    echo Failed
    exit /b 1
  ) else (
    echo Done
  )


  ::Generate auto-*.h
  echo |set /p=Generating auto-skip.h			
  !GEN_TOOL! -g skip-header %AUTO_FOLDER%\scheme.tlo > %AUTO_FOLDER%\auto-skip.h
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-skip.h del /q /f %AUTO_FOLDER%\auto-skip.h
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-fetch.h			
  !GEN_TOOL! -g fetch-header %AUTO_FOLDER%\scheme.tlo > %AUTO_FOLDER%\auto-fetch.h
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-fetch.h del /q /f %AUTO_FOLDER%\auto-fetch.h
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-store.h			
  !GEN_TOOL! -g store-header %AUTO_FOLDER%\scheme.tlo > %AUTO_FOLDER%\auto-store.h
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-store.h del /q /f %AUTO_FOLDER%\auto-store.h
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-autocomplete.h	
  !GEN_TOOL! -g autocomplete-header %AUTO_FOLDER%\scheme.tlo > %AUTO_FOLDER%\auto-autocomplete.h
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-autocomplete.h del /q /f %AUTO_FOLDER%\auto-autocomplete.h
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-types.h			
  !GEN_TOOL! -g types-header %AUTO_FOLDER%\scheme.tlo > %AUTO_FOLDER%\auto-types.h
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-types.h del /q /f %AUTO_FOLDER%\auto-types.h
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-fetch-ds.h		
  !GEN_TOOL! -g fetch-ds-header %AUTO_FOLDER%\scheme.tlo > %AUTO_FOLDER%\auto-fetch-ds.h
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-fetch-ds.h del /q /f %AUTO_FOLDER%\auto-fetch-ds.h
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-free-ds.h		
  !GEN_TOOL! -g free-ds-header %AUTO_FOLDER%\scheme.tlo > %AUTO_FOLDER%\auto-free-ds.h
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-free-ds.h del /q /f %AUTO_FOLDER%\auto-free-ds.h
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-store-ds.h		
  !GEN_TOOL! -g store-ds-header %AUTO_FOLDER%\scheme.tlo > %AUTO_FOLDER%\auto-store-ds.h
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-store-ds.h del /q /f %AUTO_FOLDER%\auto-store-ds.h
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-print-ds.h		
  !GEN_TOOL! -g print-ds-header %AUTO_FOLDER%\scheme.tlo > %AUTO_FOLDER%\auto-print-ds.h
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-print-ds.h del /q /f %AUTO_FOLDER%\auto-print-ds.h
    echo Failed
  ) else (
    echo Done
  )

  
  ::Generate auto-*.c
  echo |set /p=Generating auto-skip.c			
  !GEN_TOOL! -g skip %AUTO_FOLDER%\scheme.tlo >  %AUTO_FOLDER%\auto-skip.c
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-skip.c del /q /f %AUTO_FOLDER%\auto-skip.c
    echo Failed
  ) else (
    echo Done
  )
  
  echo |set /p=Generating auto-fetch.c			
  !GEN_TOOL! -g fetch %AUTO_FOLDER%\scheme.tlo >  %AUTO_FOLDER%\auto-fetch.c
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-fetch.c del /q /f %AUTO_FOLDER%\auto-fetch.c
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-store.c 		
  !GEN_TOOL! -g store %AUTO_FOLDER%\scheme.tlo >  %AUTO_FOLDER%\auto-store.c
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-store.c del /q /f %AUTO_FOLDER%\auto-store.c
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-autocomplete.c	
  !GEN_TOOL! -g autocomplete %AUTO_FOLDER%\scheme.tlo >  %AUTO_FOLDER%\auto-autocomplete.c
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-autocomplete.c del /q /f %AUTO_FOLDER%\auto-autocomplete.c
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-types.c			
  !GEN_TOOL! -g types %AUTO_FOLDER%\scheme.tlo >  %AUTO_FOLDER%\auto-types.c
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-types.c del /q /f %AUTO_FOLDER%\auto-types.c
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-fetch-ds.c		
  !GEN_TOOL! -g fetch-ds %AUTO_FOLDER%\scheme.tlo >  %AUTO_FOLDER%\auto-fetch-ds.c
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-fetch-ds.c del /q /f %AUTO_FOLDER%\auto-fetch-ds.c
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-free-ds.c		
  !GEN_TOOL! -g free-ds %AUTO_FOLDER%\scheme.tlo >  %AUTO_FOLDER%\auto-free-ds.c
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-free-ds.c del /q /f %AUTO_FOLDER%\auto-free-ds.c
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-store-ds.c		
  !GEN_TOOL! -g store-ds %AUTO_FOLDER%\scheme.tlo >  %AUTO_FOLDER%\auto-store-ds.c
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-store-ds.c del /q /f %AUTO_FOLDER%\auto-store-ds.c
    echo Failed
  ) else (
    echo Done
  )

  echo |set /p=Generating auto-print-ds.c		
  !GEN_TOOL! -g print-ds %AUTO_FOLDER%\scheme.tlo >  %AUTO_FOLDER%\auto-print-ds.c
  if not !ERRORLEVEL!==0 (
    set /a ERR_ENC=!ERRORLEVEL!
    if exist %AUTO_FOLDER%\auto-print-ds.c del /q /f %AUTO_FOLDER%\auto-print-ds.c
    echo Failed
  ) else (
    echo Done
  )
  set /a ERRORLEVEL=!ERR_ENC!
)

::Script End
exit /b !ERRORLEVEL!

::#################################################################################################################################################
:: Script Functions
::#################################################################################################################################################

:gen_config <output folder>
  :rem Copy config.h.in to the output folder
  echo |set /p=Copying file: %CONFIG_HEADER_in%		
  copy %CONFIG_HEADER_IN% %1 1>NUL && (
    echo Done	
  ) || (
    echo Failed
    exit /B %ERRORLEVEL%
  )

  echo |set /p=Configurating file: config.h		
  call :find_and_replace %1 "#undef PACKAGE_BUGREPORT"  "#define PACKAGE_BUGREPORT %PACKAGE_BUGREPORT%"
  call :find_and_replace %1 "#undef PACKAGE_NAME"       "#define PACKAGE_NAME %PACKAGE_NAME%"
  call :find_and_replace %1 "#undef PACKAGE_STRING"     "#define PACKAGE_STRING %PACKAGE_STRING%"
  call :find_and_replace %1 "#undef PACKAGE_TARNAME"    "#define PACKAGE_TARNAME %PACKAGE_TARNAME%"
  call :find_and_replace %1 "#undef PACKAGE_URL"        "#define PACKAGE_URL %PACKAGE_URL%"
  call :find_and_replace %1 "#undef PACKAGE_VERSION"    "#define PACKAGE_VERSION %PACKAGE_VERSION%"

  call :find_and_replace %1 "#undef EVENT_V2"           "#define EVENT_V2 1"
  call :find_and_replace %1 "#undef HAVE_FCNTL_H"       "#define HAVE_FCNTL_H 1"
  call :find_and_replace %1 "#undef HAVE_INTTYPES_H"    "#define HAVE_INTTYPES_H 1"
  call :find_and_replace %1 "#undef HAVE_LIBM"          "#define HAVE_LIBM 1"
  call :find_and_replace %1 "#undef HAVE_LIBZ"          "#define HAVE_LIBZ 1"
  call :find_and_replace %1 "#undef HAVE_MALLOC_H"      "#define HAVE_MALLOC_H 1"
  call :find_and_replace %1 "#undef HAVE_MALLOC"        "#define HAVE_MALLOC 1"
  call :find_and_replace %1 "#undef HAVE_MEMMOVE"       "#define HAVE_MEMMOVE 1"
  call :find_and_replace %1 "#undef HAVE_MEMORY_H"      "#define HAVE_MEMORY_H 1"
  call :find_and_replace %1 "#undef HAVE_MEMSET"        "#define HAVE_MEMSET 1"
  call :find_and_replace %1 "#undef HAVE_MKDIR"         "#define HAVE_MKDIR 1"
  call :find_and_replace %1 "#undef HAVE_REALLOC"       "#define HAVE_REALLOC 1"
  call :find_and_replace %1 "#undef HAVE_SELECT"        "#define HAVE_SELECT 1"
  call :find_and_replace %1 "#undef HAVE_SOCKET"        "#define HAVE_SOCKET 1"
  call :find_and_replace %1 "#undef HAVE_STDINT_H"      "#define HAVE_STDINT_H 1"
  call :find_and_replace %1 "#undef HAVE_STDLIB_H"      "#define HAVE_STDLIB_H 1"
  call :find_and_replace %1 "#undef HAVE_STRDUP"        "#define HAVE_STRDUP 1"
  call :find_and_replace %1 "#undef HAVE_STRING_H"      "#define HAVE_STRING_H 1"
  call :find_and_replace %1 "#undef HAVE_STRNDUP"       "#define HAVE_STRNDUP 1"
  call :find_and_replace %1 "#undef HAVE_SYS_STAT_H"    "#define HAVE_SYS_STAT_H 1"
  call :find_and_replace %1 "#undef HAVE_SYS_TYPES_H"   "#define HAVE_SYS_TYPES_H 1"
  call :find_and_replace %1 "#undef STDC_HEADERS"       "#define STDC_HEADERS 1"
  call :find_and_replace %1 "#undef gid_t"              "#define gid_t int"
  call :find_and_replace %1 "#undef uid_t"              "#define uid_t int"
  call :find_and_replace %1 "#undef inline"             "#define inline __inline"
  call :find_and_replace %1 "#undef"                    "// #undef"
  if %ERRORLEVEL% == 0 echo Done
  if not %ERRORLEVEL% == 0 (
    echo Failed
    if exist %1\%CONFIG_HEADER% del /q /f %1\%CONFIG_HEADER%
    exit /b 1
  )
  exit /B 0

:gen_constants <input file>
  echo /*
  echo    This file is part of telegram-client.
  echo.
  echo    Telegram-client is free software: you can redistribute it and/or modify
  echo    it under the terms of the GNU General Public License as published by
  echo    the Free Software Foundation, either version 2 of the License, or
  echo    (at your option) any later version.
  echo.
  echo    Telegram-client is distributed in the hope that it will be useful,
  echo    but WITHOUT ANY WARRANTY; without even the implied warranty of
  echo    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  echo    GNU General Public License for more details.
  echo.
  echo    You should have received a copy of the GNU General Public License
  echo    along with this telegram-client.  If not, see ^<http://www.gnu.org/licenses/^>.
  echo.
  echo    Copyright Vitaly Valtman 2013
  echo */
  echo #ifndef CONSTANTS_H
  echo #define CONSTANTS_H

  setlocal EnableDelayedExpansion
  for /F "tokens=1" %%A in (%1) do (
    call :split %%A  # a  
    if %ERRORLEVEL% == 0 (

      call :sub a[0]
      if not "!h[%a[1]%]!"=="" (
        echo ERROR: Duplicate magic %a[1]% for define %a[0]% and !h[%a[1]%]! 1>&2
        exit /b 1
      )

      set h[!a[1]!]=!a[0]!
      echo #define CODE_!a[0]! 0x!a[1]! 
    )
  )  
  endlocal
  echo #endif
  exit /b0


:find_and_replace <file> <findString> <replaceString>
  set file_path=%1
  set find_str=%2
  set "find_str=!find_str:~1,-1!"
  set replace_str=%3 %4
  set "replace_str=!replace_str:~1,-2!"

  for /f "delims=" %%i in ('type "%file_path%" ^| find /n /v "" ^& break ^> "%file_path%" ') do (
    set "line=%%i"
    setlocal enableDelayedExpansion
    set "line=!line:*]%find_str%=%replace_str%!"
    set "line=!line:*]=!"
    >>%file_path% echo(!line!
    endlocal
  )
  exit /B %ERRORLEVEL%

:split <string> <delim> <resultVar> 
  for /F "tokens=1,2 delims=%2" %%a in ("%1") do (
     if "%%a"=="" exit /b 1
     set %3[0]=%%a
     if "%%b"=="" exit /b 1
     set %3[1]=%%b
  )
  exit /b0


:sub <string>
  set /A len= 0
  call :strlen %%1  len 
  (
    setlocal EnableDelayedExpansion
    set "substr="
    for /l %%a IN (0,1,!len!) DO (
      set Char=!%1:~%%a,1!
      if "!Char!" EQU "A" SET Char=_a
      if "!Char!" EQU "B" SET Char=_b
      if "!Char!" EQU "C" SET Char=_c
      if "!Char!" EQU "D" SET Char=_d
      if "!Char!" EQU "E" SET Char=_e
      if "!Char!" EQU "F" SET Char=_f
      if "!Char!" EQU "G" SET Char=_g
      if "!Char!" EQU "H" SET Char=_h
      if "!Char!" EQU "I" SET Char=_i
      if "!Char!" EQU "J" SET Char=_j
      if "!Char!" EQU "K" SET Char=_k
      if "!Char!" EQU "L" SET Char=_l
      if "!Char!" EQU "M" SET Char=_m
      if "!Char!" EQU "N" SET Char=_n
      if "!Char!" EQU "O" SET Char=_o
      if "!Char!" EQU "P" SET Char=_p
      if "!Char!" EQU "Q" SET Char=_q
      if "!Char!" EQU "R" SET Char=_r
      if "!Char!" EQU "S" SET Char=_s
      if "!Char!" EQU "T" SET Char=_t
      if "!Char!" EQU "U" SET Char=_u
      if "!Char!" EQU "V" SET Char=_v
      if "!Char!" EQU "W" SET Char=_w
      if "!Char!" EQU "X" SET Char=_x
      if "!Char!" EQU "Y" SET Char=_y
      if "!Char!" EQU "Z" SET Char=_z
      if "!Char!" EQU "." SET Char=_

      set substr=!substr!!Char!
    )
   )
  (
    endlocal
    set "%~1=%substr%"
    exit /b 
  )

:strlen <stringVar> <resultVar> 
  ( 
    setlocal EnableDelayedExpansion
    set "s=!%~1!#"
    set "len=0"
    for %%P in (4096 2048 1024 512 256 128 64 32 16 8 4 2 1) do (
      if "!s:~%%P,1!" NEQ "" ( 
        set /a "len+=%%P"
        set "s=!s:~%%P!"
      )
    )
  )
  ( 
    endlocal
    set "%~2=%len%"
    exit /b
  )
