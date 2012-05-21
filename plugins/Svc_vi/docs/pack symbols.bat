@echo off
if NOT EXIST "symbols\%1 - %3" ( 
	mkdir "symbols\%1 - %3"
)
xcopy "..\%2\win32\Release\*.pdb" "symbols\%1 - %3\win32\*" /EXCLUDE:symbols_exclude.txt /Y
xcopy "..\%2\x64\Release\*.pdb" "symbols\%1 - %3\x64\*" /EXCLUDE:symbols_exclude.txt /Y

d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -r -c2 "symbols - %1.zip" "symbols\*.*"
rmdir "symbols\" /Q /S