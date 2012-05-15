@echo off

rem ---------------------------------------------------------------------------
rem Zip it
rem ---------------------------------------------------------------------------

pushd "..\bin9\Release Unicode64"

dir /B /S *.dll | ..\..\bin\rebaser

for /F "tokens=1,2,3 delims= " %%i in (..\..\bin\build.no) do call :Pack %%i %%j %%k

popd
goto :eof

:Nmake
echo.
echo ===========================================================================
echo Building %1
echo ===========================================================================
nmake /NOLOGO /f %1 CFG=%2
goto :eof

:SetBuildDate
set dd=%1
set mm=%2
set yy=%3
goto :eof

:SetBuildTime
set hh=%1
set mn=%2
goto :eof

:Pack
if %2 == 00 (
   set FileVer=v0%1a%3w64.7z
) else (
   set FileVer=v0%1%2a%3w64.7z
)

del /Q /F "%Temp%\miranda-%FileVer%"
"%PROGRAMFILES%\7-zip\7z.exe" a -r -mx=9 "%Temp%\miranda-%FileVer%" ./*.exe ./*.dll ..\..\bin\ChangeLog.txt

del /Q /F "%Temp%\miranda-pdb-%FileVer%"
"%PROGRAMFILES%\7-zip\7z.exe" a -r -mx=9 "%Temp%\miranda-pdb-%FileVer%" ./*.pdb -x!Obj/*.pdb
goto :eof

:Error
echo Make failed
goto :eof
