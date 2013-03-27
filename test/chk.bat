set tp=%1
if  "%tp%"=="" (echo "please specify target platform 32 or 64!"&&pause&&goto :EOF)

set hashes=%cd%\hashes.txt
set checksum=%cd%\..\checksum.exe
%checksum% Miranda%tp%.exe > %hashes%
%checksum% Zlib.dll >> %hashes%

rem get checksum  for folder(first param) and ext (second param)
call :checksum Plugins dll
call :checksum Icons dll

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