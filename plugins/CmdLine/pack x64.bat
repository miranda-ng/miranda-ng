if NOT EXIST "plugins" (
	mkdir "plugins" 
)

xcopy "CmdLine\x64\Release\*.dll" "plugins\*"

for /F "tokens=4-8* delims=. " %%i in (docs\CmdLine_readme.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
d:\usr\PowerArchiver\pacl\pacomp.exe -a -c2 "CmdLine %1.%2.%3.%4 x64.zip" @files_releasex64.txt
d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -c2 "CmdLine %1.%2.%3.%4 x64.zip" docs\*.* *.caca
d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -c2 "CmdLine %1.%2.%3.%4 x64.zip" plugins\*.* *.caca
rmdir "plugins\" /Q /S
call "pack symbols.bat" CmdLine executable\MimCmd %1.%2.%3.%4
exit

error:
echo "Error packing CmdLine"
