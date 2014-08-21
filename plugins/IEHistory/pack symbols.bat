@echo off
if NOT EXIST "symbols\%1 - %3" ( 
	mkdir "symbols\%1 - %3"
)
xcopy "%2\Release\*.pdb" "symbols\%1 - %3\*" /EXCLUDE:symbols_exclude.txt /Y

d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -r -c2 "symbols - %1.zip" "symbols\*.*"
rmdir "symbols\" /Q /S