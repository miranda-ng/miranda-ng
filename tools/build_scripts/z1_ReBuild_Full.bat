set tp=%1
if "%tp%"=="" (echo "please specify target platform 32 or 64!" && pause && goto :EOF)
if /i '%tp%' == '64' set bt=64
if /i '%tp%' == '32' (
  set ptr=Win32
) else if /i '%tp%' == '64' (
  set ptr=X64
)

set comp=%2
if "%comp%"=="" (echo "please specify target compiler folder!" && pause && goto :EOF)

call a_SetVar%tp%.bat

if exist git_error.txt del /f /q git_error.txt

pushd %comp%
if not exist Logs mkdir Logs

if /i '%tp%' == '32' (
  if exist "Release" rd /Q /S "Release" >nul
)
if exist "Release%tp%" rd /Q /S "Release%tp%" >nul
if exist "Symbols%tp%" rd /Q /S "Symbols%tp%" >nul

if exist "..\include\m_version.h" del /F /Q "..\include\m_version.h"
pushd ..\build
call make_ver.bat
popd

if "%comp%"=="bin15" (
   call "%VS141COMNTOOLS%\..\..\VC\Auxiliary\Build\vcvars%tp%.bat"
)

MsBuild.exe "mir_full.sln" /m /t:Rebuild /p:Configuration=Release;Platform="%ptr%" /fileLogger /fileLoggerParameters:LogFile=Logs\full%tp%.log;errorsonly;warningsonly;summary
if errorlevel 1 exit

MsBuild.exe "mir_icons.sln" /m /t:Rebuild /p:Configuration=Release;Platform="%ptr%" /fileLogger /fileLoggerParameters:LogFile=Logs\icons%tp%.log;errorsonly;warningsonly;summary
if errorlevel 1 exit

call pascal.bat %tp%

pushd ..\plugins\NotifyAnything\SendLog 
call compile%tp%.bat "%comp%"
popd

if exist Release ren Release Release32
md Symbols%tp%
rd /S /Q "Release%tp%\Obj" >nul
xcopy /S /V /Y "Release%tp%\*.pdb" "Symbols%tp%\"
del /F /S /Q "Release%tp%\*.pdb" >> nul

md "Release%tp%\Languages"

pushd "..\tools\lpgen"
cscript /nologo translate.js /sourcelang:"belarusian" /release:"..\..\%comp%\Release%tp%\Languages\langpack_belarusian.txt"
cscript /nologo translate.js /sourcelang:"bulgarian" /release:"..\..\%comp%\Release%tp%\langpack_bulgarian.txt"
cscript /nologo translate.js /sourcelang:"czech" /release:"..\..\%comp%\Release%tp%\Languages\langpack_czech.txt"
cscript /nologo translate.js /sourcelang:"english_gb" /release:"..\..\%comp%\Release%tp%\Languages\langpack_english_gb.txt"
cscript /nologo translate.js /sourcelang:"french" /release:"..\..\%comp%\Release%tp%\Languages\langpack_french.txt"
cscript /nologo translate.js /sourcelang:"german" /release:"..\..\%comp%\Release%tp%\Languages\langpack_german.txt"
cscript /nologo translate.js /sourcelang:"polish" /release:"..\..\%comp%\Release%tp%\Languages\langpack_polish.txt"
cscript /nologo translate.js /sourcelang:"russian" /release:"..\..\%comp%\Release%tp%\Languages\langpack_russian.txt"
cscript /nologo translate.js /sourcelang:"spanish" /release:"..\..\%comp%\Release%tp%\Languages\langpack_spanish.txt"
cscript /nologo translate.js /sourcelang:"ukrainian" /release:"..\..\%comp%\Release%tp%\Languages\langpack_ukrainian.txt"
popd

md "Release%tp%\Docs"
md "Release%tp%\Gadgets"
md "Release%tp%\Sounds"
md "Release%tp%\Scripts"
md "Release%tp%\Plugins\Listeningto"
md "Release%tp%\Plugins\BASS"
md "Release%tp%\Plugins\CurrencyRates"
md "Release%tp%\Plugins\Import"
md "Release%tp%\Plugins\Weather"

copy /V /Y ..\tools\build_scripts\fixme.cmd "Release%tp%\"
copy /V /Y ..\plugins\Actman\services.ini "Release%tp%\Plugins\"
copy /V /Y ..\plugins\Boltun\src\Engine\boltun.mindw "Release%tp%\Plugins\"
copy /V /Y ..\plugins\FavContacts\gadget\favcontacts.gadget "Release%tp%\Gadgets\"
copy /V /Y ..\plugins\Import\ini\*.ini "Release%tp%\Plugins\Import\"
copy /V /Y ..\plugins\ListeningTo\listeningto\*.dll "Release%tp%\Plugins\Listeningto\"
copy /V /Y ..\plugins\Nudge\docs\Sounds\*.wav "Release%tp%\Sounds\"
copy /V /Y ..\plugins\RemovePersonalSettings\docs\RemovePersonalSettings.ini "Release%tp%\"
copy /V /Y ..\plugins\Watrack\player.ini "Release%tp%\Plugins\"

copy /V /Y ..\protocols\CurrencyRates\docs\Utility\*.xml "Release%tp%\Plugins\CurrencyRates\"
copy /V /Y ..\protocols\CurrencyRates\CurrencyRatesChart\bin\x%tp%\CurrencyRatesChart.exe "Release%tp%\Plugins\CurrencyRates\"
copy /V /Y ..\protocols\Weather\docs\weather\*.ini "Release%tp%\Plugins\Weather\"

rem xcopy /S /V /Y "..\delphi\%tp%" "Release%tp%"
xcopy /S /V /Y "..\pre-symbols\%comp%\%tp%" "Symbols%tp%"
xcopy /S /V /Y "..\pre-build\%comp%\%tp%" "Release%tp%"

cd "Release%tp%"
dir /B /S *.dll | %SourceDir%\tools\rebaser\rebaser.exe /BASE:13000000
dir /B /S *.mir | %SourceDir%\tools\rebaser\rebaser.exe /BASE:12000000

set hashes=%cd%\hashes.txt
%SourceDir%\tools\checksum\checksum.exe Miranda%tp%.exe > %hashes%
for /f "tokens=1,2 delims= " %%i in ('%SourceDir%\tools\md5.exe Languages\lang*.txt') do echo %%j %%i >> %hashes%

rem get checksum  for folder(first param) and ext (second param)
call :checksum Plugins dll
call :checksum Icons dll

copy /V /Y ..\..\docs\mirandaboot.ini
copy /V /Y %SourceDir%\tools\build_scripts\redist\x%tp%\bass\*.dll "Plugins\BASS"

popd
goto :eof

rem the subroutine itself starts here
:checksum
rem change the dir to first parameter, and remember it
pushd %1
rem now we are here, let's get the hashes for dll
for /F "tokens=1,2 delims= " %%a in ('%SourceDir%\tools\checksum\checksum.exe *.%2') do echo %1\%%a %%b >> %hashes%
rem return to the dir, where subroutine was called
popd
rem this is just for the case if another sobroutine will be after this one :)
goto :eof
