@echo off
set PluginName=AdvancedAutoAway
nmake -f %PluginName%.mak CFG="%PluginName% - Win32 Release"
nmake -f %PluginName%.mak CFG="%PluginName% - Win32 Release Unicode"
if errorlevel 1 (
   echo "Make failed"
   goto :eof )

del "%temp%\%PluginName%.zip" > nul
del "%temp%\%PluginName%W.zip" > nul
del "%temp%\%PluginName%Src.zip" > nul
del *.user > nul

rd /S /Q Release
rd /S /Q Release_Unicode

for /F "tokens=1-6 delims=, " %%i in (buildnumber.h) do call :Translate %%i %%j %%k %%l %%m %%n

"%PROGRAMFILES%\7-zip\7z.exe" a -tzip -r- -mx=9 "%temp%\%PluginName%.zip" ../../../bin/Release/Plugins/%PluginName%.dll ../statusplugins-translation.txt ../m_statusplugins.h
del ../../../bin/Release/Plugins/%PluginName%.dll >nul

"%PROGRAMFILES%\7-zip\7z.exe" a -tzip -r- -mx=9 "%temp%\%PluginName%W.zip" "../../../bin/Release Unicode/Plugins/%PluginName%.dll" ../statusplugins-translation.txt ../m_statusplugins.h
del "../../../bin/Release Unicode/Plugins/%PluginName%.dll" >nul

if exist "%PROGRAMFILES%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" (
   "%PROGRAMFILES%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" AdvancedAutoAway_9.vcproj /Build "Release Unicode|x64"
   copy "Release Unicode64\Plugins\%PluginName%.dll" .
   "%PROGRAMFILES%\7-zip\7z.exe" a -tzip -r0 -mx=9 "%temp%\%PluginName%64.zip" %PluginName%.dll ../statusplugins-translation.txt ../m_statusplugins.h
   del %PluginName%.dll
   rd /S /Q "Release Unicode64" )

cd ../..
"%PROGRAMFILES%\7-zip\7z.exe" a -tzip -r0 -mx=9 "%temp%\%PluginName%Src.zip" -ir!helpers/*.* -ir!StatusPlugins/*.* -ir!NewTriggerPlugin/*.* -xr!StatusPlugins/KeepStatus/* -xr!StatusPlugins/StartupStatus/* -xr!.svn -x!*.cmd
goto :eof

:Translate
if %2 == __FILEVERSION_STRING (
   pushd ..
   perl lpgen.pl status version %3 %4 %5 %6
   popd)

goto :eof
