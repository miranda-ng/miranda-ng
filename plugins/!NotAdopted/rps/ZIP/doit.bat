@echo off

rem Batch file to build and upload files
rem 
rem TODO: Integration with FL

set name=rps

rem To upload, this var must be set here or in other batch
rem set ftp=ftp://<user>:<password>@<ftp>/<path>

echo Building %name% ...

msdev ..\%name%.dsp /MAKE "%name% - Win32 Release" /REBUILD

echo Generating files for %name% ...

del *.zip
del *.dll
copy ..\Docs\%name%_changelog.txt
copy ..\Docs\%name%_version.txt
copy ..\Docs\%name%_readme.txt
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\Docs\%name%_readme.txt
cd ..
mkdir Plugins
cd Plugins
del /Q *.*
copy ..\..\..\..\bin\release\Plugins\RemovePersonalSettings.dll
copy ..\..\RemovePersonalSettings.ini
cd ..

"C:\Program Files\Filzip\Filzip.exe" -a -rp %name%.zip Docs Plugins

cd Docs
del /Q *.*
cd ..
rmdir Docs
cd Plugins
del /Q *.*
cd ..
rmdir Plugins

if "%ftp%"=="" GOTO END

echo Going to upload files...
pause

"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_changelog.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_version.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_readme.txt %ftp% -overwrite -close 

:END

echo Done.
