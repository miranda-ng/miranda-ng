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
Tools\wget.exe --no-check-certificate -O tmp\InnoSetup5.7z https://miranda-ng.org/distr/installer/InnoSetup5.7z
Tools\wget.exe --no-check-certificate -O tmp\MNG_Sounds.7z https://miranda-ng.org/distr/addons/Sounds/MNG_Sounds.7z
Tools\wget.exe --no-check-certificate -O tmp\miranda-ng-alpha-latest.7z https://miranda-ng.org/distr/miranda-ng-alpha-latest.7z
Tools\wget.exe --no-check-certificate -O tmp\miranda-ng-alpha-latest_x64.7z https://miranda-ng.org/distr/miranda-ng-alpha-latest_x64.7z
Tools\wget.exe --no-check-certificate -O tmp\clist_blind_x32.zip https://miranda-ng.org/x32/Plugins/clist_blind.zip
Tools\wget.exe --no-check-certificate -O tmp\clist_blind_x64.zip https://miranda-ng.org/x64/Plugins/clist_blind.zip
Tools\wget.exe --no-check-certificate -O tmp\clist_nicer_x32.zip https://miranda-ng.org/x32/Plugins/clist_nicer.zip
Tools\wget.exe --no-check-certificate -O tmp\clist_nicer_x64.zip https://miranda-ng.org/x64/Plugins/clist_nicer.zip
Tools\wget.exe --no-check-certificate -O tmp\cln_skinedit_x32.zip https://miranda-ng.org/x32/Plugins/cln_skinedit.zip
Tools\wget.exe --no-check-certificate -O tmp\cln_skinedit_x64.zip https://miranda-ng.org/x64/Plugins/cln_skinedit.zip
Tools\wget.exe --no-check-certificate -O tmp\scriver_x32.zip https://miranda-ng.org/x32/Plugins/scriver.zip
Tools\wget.exe --no-check-certificate -O tmp\scriver_x64.zip https://miranda-ng.org/x64/Plugins/scriver.zip
Tools\wget.exe --no-check-certificate -O tmp\langpack_czech.zip https://miranda-ng.org/x32/Languages/langpack_czech.zip
Tools\wget.exe --no-check-certificate -O tmp\langpack_german.zip https://miranda-ng.org/x32/Languages/langpack_german.zip
Tools\wget.exe --no-check-certificate -O tmp\langpack_polish.zip https://miranda-ng.org/x32/Languages/langpack_polish.zip
Tools\wget.exe --no-check-certificate -O tmp\langpack_russian.zip https://miranda-ng.org/x32/Languages/langpack_russian.zip
Tools\wget.exe --no-check-certificate -O InnoNG_32\Installer\vcredist_x86.exe https://download.visualstudio.microsoft.com/download/pr/11100229/78c1e864d806e36f6035d80a0e80399e/VC_redist.x86.exe
Tools\wget.exe --no-check-certificate -O InnoNG_64\Installer\vcredist_x64.exe https://download.visualstudio.microsoft.com/download/pr/11100230/15ccb3f02745c7b206ad10373cbca89b/VC_redist.x64.exe
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