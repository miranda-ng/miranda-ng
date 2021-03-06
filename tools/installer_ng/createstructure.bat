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
curl.exe -o tmp\InnoSetup5.7z https://miranda-ng.org/distr/installer/InnoSetup5.7z
curl.exe -o tmp\MNG_Sounds.7z https://miranda-ng.org/distr/addons/Sounds/MNG_Sounds.7z
curl.exe -o tmp\miranda-ng-alpha-latest.7z https://miranda-ng.org/distr/miranda-ng-alpha-latest.7z
curl.exe -o tmp\miranda-ng-alpha-latest_x64.7z https://miranda-ng.org/distr/miranda-ng-alpha-latest_x64.7z
curl.exe -o tmp\clist_blind_x32.zip https://miranda-ng.org/x32/Plugins/clist_blind.zip
curl.exe -o tmp\clist_blind_x64.zip https://miranda-ng.org/x64/Plugins/clist_blind.zip
curl.exe -o tmp\clist_nicer_x32.zip https://miranda-ng.org/x32/Plugins/clist_nicer.zip
curl.exe -o tmp\clist_nicer_x64.zip https://miranda-ng.org/x64/Plugins/clist_nicer.zip
curl.exe -o tmp\cln_skinedit_x32.zip https://miranda-ng.org/x32/Plugins/cln_skinedit.zip
curl.exe -o tmp\cln_skinedit_x64.zip https://miranda-ng.org/x64/Plugins/cln_skinedit.zip
curl.exe -o tmp\scriver_x32.zip https://miranda-ng.org/x32/Plugins/scriver.zip
curl.exe -o tmp\scriver_x64.zip https://miranda-ng.org/x64/Plugins/scriver.zip
curl.exe -o tmp\langpack_czech.zip https://miranda-ng.org/x32/Languages/langpack_czech.zip
curl.exe -o tmp\langpack_german.zip https://miranda-ng.org/x32/Languages/langpack_german.zip
curl.exe -o tmp\langpack_polish.zip https://miranda-ng.org/x32/Languages/langpack_polish.zip
curl.exe -o tmp\langpack_russian.zip https://miranda-ng.org/x32/Languages/langpack_russian.zip
rem end

rem Extract
%CompressIt% x tmp\InnoSetup5.7z -y -oTools
%CompressIt% x tmp\miranda-ng-alpha-latest.7z -y -oInnoNG_32\Files
%CompressIt% x tmp\clist_blind_x32.zip -y -oInnoNG_32\Files
%CompressIt% x tmp\clist_nicer_x32.zip -y -oInnoNG_32\Files
%CompressIt% x tmp\cln_skinedit_x32.zip -y -oInnoNG_32\Files
%CompressIt% x tmp\scriver_x32.zip -y -oInnoNG_32\Files
%CompressIt% x tmp\miranda-ng-alpha-latest_x64.7z -y -oInnoNG_64\Files
%CompressIt% x tmp\clist_blind_x64.zip -y -oInnoNG_64\Files
%CompressIt% x tmp\clist_nicer_x64.zip -y -oInnoNG_64\Files
%CompressIt% x tmp\cln_skinedit_x64.zip -y -oInnoNG_64\Files
%CompressIt% x tmp\scriver_x64.zip -y -oInnoNG_64\Files
%CompressIt% x tmp\MNG_Sounds.7z -y -oInnoNG_32\Files
%CompressIt% x tmp\MNG_Sounds.7z -y -oInnoNG_64\Files
%CompressIt% x tmp\lang*.zip -y -oInnoNG_32\Files
%CompressIt% x tmp\lang*.zip -y -oInnoNG_64\Files
rem end
