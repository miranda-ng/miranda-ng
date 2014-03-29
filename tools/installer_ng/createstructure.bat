@echo off
rem Create directories and copy script
mkdir tmp
mkdir InnoNG_32
mkdir InnoNG_64
copy /V /Y MirandaNG.iss InnoNG_32\
copy /V /Y MirandaNG.iss InnoNG_64\
xcopy Common\* InnoNG_32 /I /S /V /Y
xcopy Common\* InnoNG_64 /I /S /V /Y
rem end

rem Download
Tools\wget.exe -O tmp\InnoSetup5.7z http://miranda-ng.org/distr/installer/InnoSetup5.7z
Tools\wget.exe -O tmp\MNG_Sounds.7z http://miranda-ng.org/distr/addons/Sounds/MNG_Sounds.7z
Tools\wget.exe -O tmp\miranda-ng-alpha-latest.7z http://miranda-ng.org/distr/miranda-ng-alpha-latest.7z
Tools\wget.exe -O tmp\miranda-ng-alpha-latest_x64.7z http://miranda-ng.org/distr/miranda-ng-alpha-latest_x64.7z
Tools\wget.exe -O tmp\clist_blind_x32.zip http://miranda-ng.org/x32/Plugins/clist_blind.zip
Tools\wget.exe -O tmp\clist_blind_x64.zip http://miranda-ng.org/x64/Plugins/clist_blind.zip
Tools\wget.exe -O tmp\clist_nicer_x32.zip http://miranda-ng.org/x32/Plugins/clist_nicer.zip
Tools\wget.exe -O tmp\clist_nicer_x64.zip http://miranda-ng.org/x64/Plugins/clist_nicer.zip
Tools\wget.exe -O tmp\cln_skinedit_x32.zip http://miranda-ng.org/x32/Plugins/cln_skinedit.zip
Tools\wget.exe -O tmp\cln_skinedit_x64.zip http://miranda-ng.org/x64/Plugins/cln_skinedit.zip
Tools\wget.exe -O tmp\scriver_x32.zip http://miranda-ng.org/x32/Plugins/scriver.zip
Tools\wget.exe -O tmp\scriver_x64.zip http://miranda-ng.org/x64/Plugins/scriver.zip
Tools\wget.exe -O tmp\langpack_czech.zip http://miranda-ng.org/x32/Languages/langpack_czech.zip
Tools\wget.exe -O tmp\langpack_german.zip http://miranda-ng.org/x32/Languages/langpack_german.zip
Tools\wget.exe -O tmp\langpack_polish.zip http://miranda-ng.org/x32/Languages/langpack_polish.zip
Tools\wget.exe -O tmp\langpack_russian.zip http://miranda-ng.org/x32/Languages/langpack_russian.zip
Tools\wget.exe -O InnoNG_32\Installer\vcredist_x86.exe http://download.microsoft.com/download/C/6/D/C6D0FD4E-9E53-4897-9B91-836EBA2AACD3/vcredist_x86.exe
Tools\wget.exe -O InnoNG_64\Installer\vcredist_x64.exe http://download.microsoft.com/download/A/8/0/A80747C3-41BD-45DF-B505-E9710D2744E0/vcredist_x64.exe
rem end

rem Extract
..\7-zip\7z.exe x tmp\InnoSetup5.7z -y -oTools
..\7-zip\7z.exe x tmp\miranda-ng-alpha-latest.7z -y -oInnoNG_32\Files
..\7-zip\7z.exe x tmp\clist_blind_x32.zip -y -oInnoNG_32\Files
..\7-zip\7z.exe x tmp\clist_nicer_x32.zip -y -oInnoNG_32\Files
..\7-zip\7z.exe x tmp\cln_skinedit_x32.zip -y -oInnoNG_32\Files
..\7-zip\7z.exe x tmp\scriver_x32.zip -y -oInnoNG_32\Files
..\7-zip\7z.exe x tmp\miranda-ng-alpha-latest_x64.7z -y -oInnoNG_64\Files
..\7-zip\7z.exe x tmp\clist_blind_x64.zip -y -oInnoNG_64\Files
..\7-zip\7z.exe x tmp\clist_nicer_x64.zip -y -oInnoNG_64\Files
..\7-zip\7z.exe x tmp\cln_skinedit_x64.zip -y -oInnoNG_64\Files
..\7-zip\7z.exe x tmp\scriver_x64.zip -y -oInnoNG_64\Files
..\7-zip\7z.exe x tmp\MNG_Sounds.7z -y -oInnoNG_32\Files
..\7-zip\7z.exe x tmp\MNG_Sounds.7z -y -oInnoNG_64\Files
..\7-zip\7z.exe x tmp\lang*.zip -y -oInnoNG_32\Files
..\7-zip\7z.exe x tmp\lang*.zip -y -oInnoNG_64\Files
rem end