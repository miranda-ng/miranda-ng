@echo off

echo *                                              * 
echo ------------------------------------------------
echo build tabSRMM with vcbuild.exe from Windows SDK
echo requires a Windows PSDK (Version 7 or later)
echo must be run from the SDK cmd shell with properly 
echo configured environment.
echo ------------------------------------------------
echo *                                              *

vcbuild.exe tabsrmm_9.vcproj "Release Unicode" /platform:Win32 /r
vcbuild.exe tabsrmm_9.vcproj "Release Unicode" /platform:x64 /r
