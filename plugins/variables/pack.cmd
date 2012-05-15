@echo off
nmake -f Variables.mak CFG="Variables - Win32 Release"
if errorlevel 1 (
   echo "Make failed"
   goto :eof )
nmake -f Variables.mak CFG="Variables - Win32 Release Unicode"
if errorlevel 1 (
   echo "Make failed"
   goto :eof )

del "%temp%\Variables.zip" > nul
del "%temp%\VariablesW.zip" > nul
del "%temp%\VariablesSrc.zip" > nul
del *.user > nul

rd /S /Q Release
rd /S /Q Release_Unicode

for /F "tokens=1-6 delims=, " %%i in (buildnumber.h) do call :Translate %%i %%j %%k %%l %%m %%n

"%PROGRAMFILES%\7-zip\7z.exe" a -tzip -r- -mx=9 "%temp%\Variables.zip" ../../bin/Release/Plugins/Variables.dll variables-translation.txt m_variables.h
del ../../bin/Release/Plugins/Variables.dll >nul

"%PROGRAMFILES%\7-zip\7z.exe" a -tzip -r- -mx=9 "%temp%\VariablesW.zip" "../../bin/Release Unicode/Plugins/Variables.dll" variables-translation.txt m_variables.h
del "../../bin/Release Unicode/Plugins/Variables.dll" >nul

cd ..
"%PROGRAMFILES%\7-zip\7z.exe" a -tzip -r0 -mx=9 "%temp%\VariablesSrc.zip" -ir!helpers/*.* -ir!Variables/*.* -ir!NewTriggerPlugin/*.* -xr!.svn -x!*.cmd
goto :eof

:Translate
if %2 == __FILEVERSION_STRING (
   perl lpgen.pl variables version %3 %4 %5 %6 )

goto :eof
