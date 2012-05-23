@echo off
set PluginName=tabSRMM
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

7z.exe a -tzip -r -mx=9 "%temp%\%PluginName%.zip" ../../bin/Release/%PluginName%*.dll
del ../../../bin/Release/Plugins/%PluginName%.dll >nul

7z.exe a -tzip -r -mx=9 "%temp%\%PluginName%W.zip" "../../bin/Release Unicode/%PluginName%*.dll"
del "../../../bin/Release Unicode/Plugins/%PluginName%.dll" >nul

7z.exe a -tzip -r0 -mx=9 "%temp%\%PluginName%Src.zip" -xr!.svn -x!*.cmd
goto :eof
