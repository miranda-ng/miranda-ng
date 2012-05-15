@echo off

:if %1 == "" goto USAGE
:if %2 == "" goto USAGE
:if %3 NEQ "" goto USAGE

SET OUT_BASE=publish

: Prepare Release for publication
SET BUILT=%1
SET VER=%2

SET IN_ICON=%BUILT%\Icons
SET IN_PLUG=%BUILT%\Plugins
SET OUT_PATH=%OUT_BASE%\%VER%\%BUILT%
SET OUT_DOCS=%OUT_PATH%\Docs
SET OUT_DOCS_UINFOEX=%OUT_DOCS%\uinfoex
SET OUT_ICON=%OUT_PATH%\Icons
SET OUT_PLUG=%OUT_PATH%\Plugins
SET OUT_SOUND=%OUT_PATH%\Sounds

: Create folder structure
if not exist %OUT_BASE% mkdir %OUT_BASE%
if not exist %OUT_BASE%\%VER% mkdir %OUT_BASE%\%VER%
if not exist %OUT_PATH% mkdir %OUT_PATH%
if not exist %OUT_DOCS% mkdir %OUT_DOCS%
if not exist %OUT_DOCS_UINFOEX% mkdir %OUT_DOCS_UINFOEX%
if not exist %OUT_ICON% mkdir %OUT_ICON%
if not exist %OUT_PLUG% mkdir %OUT_PLUG%
if not exist %OUT_SOUND% mkdir %OUT_SOUND%

: Copy binary files
copy /y %IN_ICON%\uinfoex_icons.dll %OUT_ICON%  > nul
if not errorlevel 0 goto MISSING
copy /y %IN_PLUG%\uinfoex?.dll %OUT_PLUG%  > nul
if not errorlevel 0 goto MISSING
copy /y changelog.txt %OUT_DOCS_UINFOEX%  > nul
if not errorlevel 0 goto MISSING
copy /y docs\uinfoex_translate.txt %OUT_DOCS_UINFOEX%  > nul
if not errorlevel 0 goto MISSING
copy /y sdk\m_userinfoex.h %OUT_DOCS_UINFOEX%  > nul
if not errorlevel 0 goto MISSING
copy /y sounds\*.wav %OUT_SOUND%  > nul
if not errorlevel 0 goto MISSING

echo Fertig!
goto END

:MISSING
echo Missing required file!
goto END

:USAGE
echo.
echo ===============================================================================
echo UserInfoEx Postbuilt Script for automatic preparation of published binaries.
echo.
echo USAGE: _publishbin [BUILT] [VERSION]
echo.
echo Example: _publishbin "Release Unicode" "0.8.3.2"
echo.
echo ===============================================================================
echo.

:END