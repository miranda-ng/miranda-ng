rem @echo off

rem Batch file to build and upload files
rem 
rem TODO: Integration with FL

set name=spellchecker
set version=0.2.6.0

rem To upload, this var must be set here or in other batch
rem set ftp=ftp://<user>:<password>@<ftp>/<path>

echo Building %name% ...

rem msdev ..\%name%.dsp /MAKE "%name% - Win32 Release" /REBUILD
rem msdev ..\%name%.dsp /MAKE "%name% - Win32 Unicode Release" /REBUILD

echo Generating files for %name% ...

del *.zip
del *.dll
del *.pdb
rd /S /Q Plugins
rd /S /Q Docs
rd /S /Q src

copy "..\bin\Win32\Release\%name%.pdb"
copy "..\bin\Win32\Unicode Release\%name%W.pdb"
copy "..\bin\x64\Unicode Release\%name%64.pdb"
copy ..\Docs\%name%_changelog.txt
copy ..\Docs\%name%_version.txt
copy ..\Docs\%name%_readme.txt
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\Docs\%name%_readme.txt
copy ..\..\Docs\langpack_%name%.txt
rem copy ..\..\Docs\helppack_%name%.txt
copy ..\..\m_%name%.h
cd ..
mkdir src
cd src
mkdir %name%
cd %name%
del /Q *.*
copy ..\..\..\*.h
copy ..\..\..\*.cpp
copy ..\..\..\*.rc
copy ..\..\..\*.dsp
copy ..\..\..\*.dsw
mkdir res
cd res
del /Q *.*
copy ..\..\..\..\res\*.*
cd ..
mkdir sdk
cd sdk
del /Q *.*
copy ..\..\..\..\sdk\*.*
cd ..
mkdir hunspell
cd hunspell
del /Q *.*
copy ..\..\..\..\hunspell\*.*
cd ..
mkdir Docs
cd Docs
del /Q *.*
copy ..\..\..\..\Docs\*.*
cd ..
cd ..
mkdir utils
cd utils
del /Q *.*
copy ..\..\..\..\utils\*.*
cd ..
cd ..

mkdir Plugins
cd Plugins
del /Q *.dll
copy "..\..\bin\Win32\Release\%name%.dll"
cd ..

zip -r -q %name%.%version%.zip Plugins Docs  
copy %name%.%version%.zip %name%.zip
zip -r -q %name%.FL.zip Plugins Docs Dictionaries Icons

cd Plugins
del /Q *.dll
copy "..\..\bin\Win32\Unicode Release\%name%W.dll"
cd ..

zip -r -q %name%W.%version%.zip Plugins Docs
copy %name%W.%version%.zip %name%W.zip
zip -r -q %name%W.FL.zip Plugins Docs Dictionaries Icons


cd Plugins
del /Q *.dll
copy "..\..\bin\x64\Unicode Release\%name%64.dll"
cd ..

zip -r -q %name%64.%version%.zip Plugins Docs
copy %name%64.%version%.zip %name%64.zip
zip -r -q %name%64.FL.zip Plugins Docs Dictionaries Icons


zip -r -q %name%.pdb.%version%.zip %name%.pdb
copy %name%.pdb.%version%.zip %name%.pdb.zip
zip -r -q %name%W.pdb.%version%.zip %name%W.pdb
copy %name%W.pdb.%version%.zip %name%W.pdb.zip
zip -r -q %name%64.pdb.%version%.zip %name%64.pdb
copy %name%64.pdb.%version%.zip %name%64.pdb.zip

zip -r -q %name%_src.zip src\*.*

del *.dll
del *.PDB

rd /S /Q Plugins
rd /S /Q Docs
rd /S /Q src

if "%ftp%"=="" GOTO END

echo Going to upload files...
pause

"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%W.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%.pdb.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%W.pdb.zip %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_changelog.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_version.txt %ftp% -overwrite -close 
"C:\Program Files\FileZilla\FileZilla.exe" -u .\%name%_readme.txt %ftp% -overwrite -close 
rem "C:\Program Files\FileZilla\FileZilla.exe" -u .\srmm.spellchecker.patch %ftp% -overwrite -close 

:END

echo Done.
