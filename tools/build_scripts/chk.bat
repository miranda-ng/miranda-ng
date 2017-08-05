rem set tp=%1
rem if  "%tp%"=="" (echo "please specify target platform 32 or 64!"&&pause&&goto :EOF)

copy ..\crc32.exe /v /y %cd%

set hashes=%cd%\hash.txt
set checksum=%cd%\..\checksum.exe
rem %checksum% Miranda%tp%.exe > %hashes%
rem %checksum% Zlib.dll >> %hashes%

rem get checksum  for folder(first param) and ext (second param)
call :checksum Plugins dll
call :checksum Icons dll
for /f "tokens=1,2 delims= " %%i in (%hashes%) do call :WriteLine %%i %%j
:WriteLine
for /F "tokens=1,2 delims=:" %%i in ('crc32.exe %~n1.zip') do echo %1 %2 %%j > %hashes%
del /f /q crc32.exe

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