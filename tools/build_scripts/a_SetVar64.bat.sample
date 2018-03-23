@echo off
set Drive=%~d0
set SourceDir=%~dp0
for %%a in (%SourceDir:~0,-1%) do set "SourceDir=%%a"
set DropDir=%Drive%\Dropbox
set StableInstDir=%SourceDir%\tools\installer_ng_stable
set GetIt=%StableInstDir%\Tools\wget.exe
set Arch=%Drive%\Arch64
set ArchPdb=%Drive%\ArchPdb64
set ArchDistr=%Drive%\ArchDistr
set Errors=..\z_Errors64.txt
set NoErrors=..\z_No_Errors64.txt
set CompressIt=%SourceDir%\tools\7-zip\7z.exe
set ZipIt=%SourceDir%\tools\7-zip\7z.exe a -r -tzip
set WinSCP=%SourceDir%\tools\WinSCP\WinSCP.com
