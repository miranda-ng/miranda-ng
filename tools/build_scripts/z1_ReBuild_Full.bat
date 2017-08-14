set tp=%1
if  "%tp%"=="" (echo "please specify target platform 32 or 64!"&&pause&&goto :EOF)
if /i '%tp%' == '64' set bt=64
if /i '%tp%' == '32' (
  set ptr=Win32
) else if /i '%tp%' == '64' (
  set ptr=X64
)

call a_SetVar%tp%.bat
call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat"
if exist git_error.txt del /f /q git_error.txt
REM call git_update.bat

pushd bin10
if not exist log mkdir log
REM if /i '%tp%' == '32' (
REM if exist "Release" rd /Q /S "Release" >nul
REM )
REM if exist "Release%tp%" rd /Q /S "Release%tp%" >nul
REM if exist "Symbols%tp%" rd /Q /S "Symbols%tp%" >nul

if exist "..\include\m_version.h" del /F /Q "..\include\m_version.h"
pushd ..\build
call make_ver.bat
popd

MsBuild.exe "full.sln" /m /t:Rebuild /p:Configuration=Release;Platform="%ptr%" /fileLogger /fileLoggerParameters:LogFile=Logs\full%tp%.log;errorsonly;warningsonly;summary
call pascal%tp%.bat
pushd ..\plugins\NotifyAnything\SendLog
call compile%tp%.bat
popd

if /i '%tp%' == '32' (
  %GetIt% -P Release/Libs -r -np -nd -nH -R html -e robots=off https://www.miranda-ng.org/distr/build/x86/
) else if /i '%tp%' == '64' (
  %GetIt% -P Release64/Libs -r -np -nd -nH -R html -e robots=off https://www.miranda-ng.org/distr/build/x64/
)

if exist Release ren Release Release32
md Symbols%tp%
rd /S /Q "Release%tp%\Obj" >nul
xcopy /S /V /Y "Release%tp%\*.pdb" "Symbols%tp%\"
del /F /S /Q "Release%tp%\*.pdb" >> nul

md "Release%tp%\Languages"

pushd "..\langpacks\tool"
LangpackSuite.exe \q \prussian \n..\..\bin10\Release%tp%\Languages\langpack_russian.txt
LangpackSuite.exe \q \pukrainian \n..\..\bin10\Release%tp%\Languages\langpack_ukrainian.txt
popd

pushd "..\tools\lpgen"
cscript /nologo translate.js /sourcelang:"belarusian" /release:"..\..\bin10\Release%tp%\Languages\langpack_belarusian.txt"
cscript /nologo translate.js /sourcelang:"bulgarian" /release:"..\..\bin10\Release%tp%\langpack_bulgarian_nodupes.txt"
copy /B "..\..\bin10\Release%tp%\langpack_bulgarian_nodupes.txt"+"..\..\langpacks\bulgarian\=DUPES=.txt" ..\..\bin10\Release%tp%\Languages\langpack_bulgarian.txt
REM cscript /nologo translate.js /sourcelang:"chinese" /release:"..\..\bin10\Release%tp%\Languages\langpack_chinese.txt"
cscript /nologo translate.js /sourcelang:"czech" /release:"..\..\bin10\Release%tp%\Languages\langpack_czech.txt"
REM cscript /nologo translate.js /sourcelang:"dutch" /release:"..\..\bin10\Release%tp%\Languages\langpack_dutch.txt"
cscript /nologo translate.js /sourcelang:"english_gb" /release:"..\..\bin10\Release%tp%\Languages\langpack_english_gb.txt"
REM cscript /nologo translate.js /sourcelang:"estonian" /release:"..\..\bin10\Release%tp%\Languages\langpack_estonian.txt"
cscript /nologo translate.js /sourcelang:"french" /release:"..\..\bin10\Release%tp%\Languages\langpack_french.txt"
cscript /nologo translate.js /sourcelang:"german" /release:"..\..\bin10\Release%tp%\Languages\langpack_german.txt"
REM cscript /nologo translate.js /sourcelang:"hebrew" /release:"..\..\bin10\Release%tp%\Languages\langpack_hebrew.txt"
REM cscript /nologo translate.js /sourcelang:"hungarian" /release:"..\..\bin10\Release%tp%\Languages\langpack_hungarian.txt"
REM cscript /nologo translate.js /sourcelang:"italian" /release:"..\..\bin10\Release%tp%\Languages\langpack_italian.txt"
REM cscript /nologo translate.js /sourcelang:"japanese" /release:"..\..\bin10\Release%tp%\Languages\langpack_japanese.txt"
REM cscript /nologo translate.js /sourcelang:"korean" /release:"..\..\bin10\Release%tp%\Languages\langpack_korean.txt"
REM cscript /nologo translate.js /sourcelang:"norwegian" /release:"..\..\bin10\Release%tp%\Languages\langpack_norwegian.txt"
cscript /nologo translate.js /sourcelang:"polish" /release:"..\..\bin10\Release%tp%\Languages\langpack_polish.txt"
REM cscript /nologo translate.js /sourcelang:"portuguese_br" /release:"..\..\bin10\Release%tp%\Languages\langpack_portuguese_br.txt"
REM cscript /nologo translate.js /sourcelang:"slovak" /release:"..\..\bin10\Release%tp%\Languages\langpack_slovak.txt"
cscript /nologo translate.js /sourcelang:"serbian" /release:"..\..\bin10\Release%tp%\Languages\langpack_serbian.txt"
cscript /nologo translate.js /sourcelang:"spanish" /release:"..\..\bin10\Release%tp%\Languages\langpack_spanish.txt"
REM cscript /nologo translate.js /sourcelang:"turkish" /release:"..\..\bin10\Release%tp%\Languages\langpack_turkish.txt"
del /f /q ..\..\bin10\Release%tp%\*_nodupes.txt
popd

REM md "Release%tp%\Dictionaries"
md "Release%tp%\Docs"
md "Release%tp%\Gadgets"
md "Release%tp%\Sounds"
md "Release%tp%\Scripts"
md "Release%tp%\Plugins\Listeningto"
md "Release%tp%\Plugins\BASS"
md "Release%tp%\Plugins\Quotes"
md "Release%tp%\Plugins\Weather"

copy /V /Y ..\plugins\FavContacts\gadget\favcontacts.gadget "Release%tp%\Gadgets\"
REM copy /V /Y ..\plugins\SpellChecker\Docs\Dictionaries\*.* "Release%tp%\Dictionaries\"
copy /V /Y ..\plugins\Nudge\docs\Sounds\*.wav "Release%tp%\Sounds\"
REM copy /V /Y ..\plugins\MirLua\docs\examples\*.lua "Release%tp%\Scripts\"
copy /V /Y ..\plugins\ListeningTo\listeningto\*.dll "Release%tp%\Plugins\Listeningto\"
copy /V /Y ..\plugins\Quotes\docs\Utility\*.xml "Release%tp%\Plugins\Quotes\"
copy /V /Y ..\plugins\Weather\docs\weather\*.ini "Release%tp%\Plugins\Weather\"
copy /V /Y ..\plugins\Actman\services.ini "Release%tp%\Plugins\"
copy /V /Y ..\plugins\Boltun\src\Engine\boltun.mindw "Release%tp%\Plugins\"
copy /V /Y ..\plugins\Watrack\player.ini "Release%tp%\Plugins\"
copy /V /Y ..\plugins\RemovePersonalSettings\docs\RemovePersonalSettings.ini "Release%tp%\"
rem copy /V /Y ..\protocols\FacebookRM\docs\*.* "Release%tp%\Docs\"
rem copy /V /Y ..\protocols\Omegle\docs\*.* "Release%tp%\Docs\"

if /i '%tp%' == '32' (
  md "Release32\Plugins\Importtxt"
  copy /V /Y ..\plugins\ImportTXT\importtxt\*.ini "Release32\Plugins\Importtxt\"
  copy /V /Y ..\redist\pcre3.dll "Release32\Libs"
)

rem xcopy /S /V /Y "..\delphi\%tp%" "Release%tp%"
xcopy /S /V /Y "..\pre-symbols\%tp%" "Symbols%tp%"
xcopy /S /V /Y "..\pre-build\%tp%" "Release%tp%"
copy /v /y ..\tools\checksum\checksum.exe

cd "Release%tp%"
dir /B /S *.dll | ..\rebaser /BASE:13000000
dir /B /S *.mir | ..\rebaser /BASE:12000000
copy /V /Y ..\..\redist\x%tp%\msvc*.dll

set hashes=%cd%\hashes.txt
set checksum=%cd%\..\checksum.exe
set md5=%cd%\..\md5.exe
%checksum% Miranda%tp%.exe > %hashes%
%checksum% msvcp100.dll >> %hashes%
%checksum% msvcr100.dll >> %hashes%
for /f "tokens=1,2 delims= " %%i in ('%md5% Languages\lang*.txt') do echo %%j %%i >> %hashes%

rem get checksum  for folder(first param) and ext (second param)
call :checksum Plugins dll
call :checksum Icons dll

copy /V /Y ..\..\docs\mirandaboot.ini
copy /V /Y ..\..\redist\x%tp%\DbChecker.bat
copy /V /Y ..\..\redist\x%tp%\bass\*.dll "Plugins\BASS"

popd

rem put me at the end of script
rem getting checksum for dlls
rem "goto :eof" here to avoid run this subroutine at the end of script, so script will run till this point, and subroutine will called only where we need it
goto :eof
rem the subroutine itself starts here
:checksum
rem change the dir to first parameter, and remember it
pushd %1
rem now we are here, let's get the hashes for dll
for /F "tokens=1,2 delims= " %%a in ('%checksum% *.%2') do echo %1\%%a %%b >> %hashes%
rem return to the dir, where subroutine was called
popd
rem this is just for the case if another sobroutine will be after this one :)
goto :eof