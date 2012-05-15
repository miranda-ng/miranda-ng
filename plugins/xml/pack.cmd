rem @echo off
nmake -f xml.mak CFG="xml - Win32 Release"
if errorlevel 1 (
   echo "Make failed"
   goto :eof )
nmake -f xml.mak CFG="xml - Win32 Release Unicode"
if errorlevel 1 (
   echo "Make failed"
   goto :eof )

del "%temp%\xml.zip" > nul
del "%temp%\xmlW.zip" > nul
del "%temp%\xmlSrc.zip" > nul
del *.user > nul

7z.exe a -tzip -r- -mx=9 "%temp%\xml.zip" ./Release/xml.dll
7z.exe a -tzip -r- -mx=9 "%temp%\xmlW.zip" ./Release_Unicode/xml.dll

rd /S /Q Release
rd /S /Q Release_Unicode

cd ..
7z.exe a -tzip -r0 -mx=9 "%temp%\xmlSrc.zip" -ir!xml/*.* -xr!.svn -x!*.cmd
goto :eof