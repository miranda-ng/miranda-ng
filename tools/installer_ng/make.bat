rem Download
mkdir tmp
wget -O tmp\miranda-ng-alpha-latest.7z http://miranda-ng.org/distr/miranda-ng-alpha-latest.7z
wget -O tmp\miranda-ng-alpha-latest_x64.7z http://miranda-ng.org/distr/miranda-ng-alpha-latest_x64.7z
wget -O tmp\clist_blind_x32.zip http://miranda-ng.org/x32/Plugins/clist_blind.zip
wget -O tmp\clist_blind_x64.zip http://miranda-ng.org/x64/Plugins/clist_blind.zip
wget -O tmp\scriver_x32.zip http://miranda-ng.org/x32/Plugins/scriver.zip
wget -O tmp\scriver_x64.zip http://miranda-ng.org/x64/Plugins/scriver.zip
wget -O tmp\langpack_czech.zip http://miranda-ng.org/x32/langpack_czech.zip
wget -O tmp\langpack_german.zip http://miranda-ng.org/x32/langpack_german.zip
wget -O tmp\langpack_polish.zip http://miranda-ng.org/x32/langpack_polish.zip
wget -O tmp\langpack_russian.zip http://miranda-ng.org/x32/langpack_russian.zip
rem end

rem Extract
"%PROGRAMFILES%\7-zip\7z.exe" x tmp\miranda-ng-alpha-latest.7z -y -oInnoNG_32\Files
"%PROGRAMFILES%\7-zip\7z.exe" x tmp\clist_blind_x32.zip -y -oInnoNG_32\Files
"%PROGRAMFILES%\7-zip\7z.exe" x tmp\scriver_x32.zip -y -oInnoNG_32\Files
"%PROGRAMFILES%\7-zip\7z.exe" x tmp\miranda-ng-alpha-latest_x64.7z -y -oInnoNG_64\Files
"%PROGRAMFILES%\7-zip\7z.exe" x tmp\clist_blind_x64.zip -y -oInnoNG_64\Files
"%PROGRAMFILES%\7-zip\7z.exe" x tmp\scriver_x64.zip -y -oInnoNG_64\Files
"%PROGRAMFILES%\7-zip\7z.exe" x tmp\lang*.zip -y -oInnoNG_32\Files
"%PROGRAMFILES%\7-zip\7z.exe" x tmp\lang*.zip -y -oInnoNG_64\Files
rem end

rem Make
"%PROGRAMFILES%\Inno Setup 5\Compil32.exe" /cc "InnoNG_32\MirandaNG.iss"
"%PROGRAMFILES%\Inno Setup 5\Compil32.exe" /cc "InnoNG_64\MirandaNG.iss"
rem end

rem Cleanup
rd /S /Q tmp

pushd InnoNG_32\Files
rd /S /Q Core
rd /S /Q Icons
rd /S /Q Plugins
del /F /Q *.ini
del /F /Q *.txt
del /F /Q *.dll
del /F /Q *.exe
popd

pushd InnoNG_64\Files
rd /S /Q Core
rd /S /Q Icons
rd /S /Q Plugins
del /F /Q *.ini
del /F /Q *.txt
del /F /Q *.dll
del /F /Q *.exe
popd
rem end