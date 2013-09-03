@echo off
rem Create directories and copy script
mkdir tmp
mkdir InnoNG_32
mkdir InnoNG_64
mkdir InnoNG_32\Installer
mkdir InnoNG_64\Installer
copy /V /Y MirandaNG.iss InnoNG_32\
copy /V /Y MirandaNG.iss InnoNG_64\
rem end

rem Download
wget -O tmp\common.7z http://miranda-ng.org/distr/installer/common.7z
wget -O tmp\MNG_Sounds.7z http://miranda-ng.org/distr/addons/Sounds/MNG_Sounds.7z
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
wget -O InnoNG_32\Installer\vcredist_x86.exe http://download.microsoft.com/download/C/6/D/C6D0FD4E-9E53-4897-9B91-836EBA2AACD3/vcredist_x86.exe
wget -O InnoNG_64\Installer\vcredist_x64.exe http://download.microsoft.com/download/A/8/0/A80747C3-41BD-45DF-B505-E9710D2744E0/vcredist_x64.exe
rem end

rem Extract
if defined ProgramFiles(x86) (
	"%ProgramW6432%\7-zip\7z.exe" x tmp\common.7z -y -oInnoNG_32
	"%ProgramW6432%\7-zip\7z.exe" x tmp\common.7z -y -oInnoNG_64
	"%ProgramW6432%\7-zip\7z.exe" x tmp\miranda-ng-alpha-latest.7z -y -oInnoNG_32\Files
	"%ProgramW6432%\7-zip\7z.exe" x tmp\clist_blind_x32.zip -y -oInnoNG_32\Files
	"%ProgramW6432%\7-zip\7z.exe" x tmp\scriver_x32.zip -y -oInnoNG_32\Files
	"%ProgramW6432%\7-zip\7z.exe" x tmp\miranda-ng-alpha-latest_x64.7z -y -oInnoNG_64\Files
	"%ProgramW6432%\7-zip\7z.exe" x tmp\clist_blind_x64.zip -y -oInnoNG_64\Files
	"%ProgramW6432%\7-zip\7z.exe" x tmp\scriver_x64.zip -y -oInnoNG_64\Files
	"%ProgramW6432%\7-zip\7z.exe" x tmp\MNG_Sounds.7z -y -oInnoNG_32\Files
	"%ProgramW6432%\7-zip\7z.exe" x tmp\MNG_Sounds.7z -y -oInnoNG_64\Files
	"%ProgramW6432%\7-zip\7z.exe" x tmp\lang*.zip -y -oInnoNG_32\Files
	"%ProgramW6432%\7-zip\7z.exe" x tmp\lang*.zip -y -oInnoNG_64\Files
) else (
	"%ProgramFiles%\7-zip\7z.exe" x tmp\common.7z -y -oInnoNG_32
	"%ProgramFiles%\7-zip\7z.exe" x tmp\common.7z -y -oInnoNG_64
	"%ProgramFiles%\7-zip\7z.exe" x tmp\miranda-ng-alpha-latest.7z -y -oInnoNG_32\Files
	"%ProgramFiles%\7-zip\7z.exe" x tmp\clist_blind_x32.zip -y -oInnoNG_32\Files
	"%ProgramFiles%\7-zip\7z.exe" x tmp\scriver_x32.zip -y -oInnoNG_32\Files
	"%ProgramFiles%\7-zip\7z.exe" x tmp\miranda-ng-alpha-latest_x64.7z -y -oInnoNG_64\Files
	"%ProgramFiles%\7-zip\7z.exe" x tmp\clist_blind_x64.zip -y -oInnoNG_64\Files
	"%ProgramFiles%\7-zip\7z.exe" x tmp\scriver_x64.zip -y -oInnoNG_64\Files
	"%ProgramFiles%\7-zip\7z.exe" x tmp\MNG_Sounds.7z -y -oInnoNG_32\Files
	"%ProgramFiles%\7-zip\7z.exe" x tmp\MNG_Sounds.7z -y -oInnoNG_64\Files
	"%ProgramFiles%\7-zip\7z.exe" x tmp\lang*.zip -y -oInnoNG_32\Files
	"%ProgramFiles%\7-zip\7z.exe" x tmp\lang*.zip -y -oInnoNG_64\Files
)
rem end

rem Make
if defined ProgramFiles(x86) (
	"%ProgramFiles(x86)%\Inno Setup 5\ISCC.exe" /Dptx86 /O"Output" "InnoNG_32\MirandaNG.iss"
	"%ProgramFiles(x86)%\Inno Setup 5\ISCC.exe" /O"Output" "InnoNG_64\MirandaNG.iss"
) else (
	"%ProgramFiles%\Inno Setup 5\ISCC.exe" /Dptx86 /O"Output" "InnoNG_32\MirandaNG.iss"
	"%ProgramFiles%\Inno Setup 5\ISCC.exe" /O"Output" "InnoNG_64\MirandaNG.iss"
)
rem end