for /F "tokens=6-10* delims=. " %%i in (docs\WhenWasIt_readme.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
d:\usr\PowerArchiver\pacl\pacomp.exe -a -c2 "WhenWasIt %1.%2.%3.%4 x32.zip" @files_release.txt
d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -c2 "WhenWasIt %1.%2.%3.%4 x32.zip" docs\*.* *.caca
call "pack symbols.bat" WhenWasIt Release %1.%2.%3.%4
exit

error:
echo "Error packing WhenWasIt"
