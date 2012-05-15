copy folders\m_folders.h docs /y
for /F "tokens=6-10* delims=. " %%i in (docs\WhenWasIt_readme.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
d:\usr\PowerArchiver\pacl\pacomp.exe -a -c2 "WhenWasIt %1.%2.%3.%4 x64 unicode.zip" @files_releasewx64.txt
d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -c2 "WhenWasIt %1.%2.%3.%4 x64 unicode.zip" docs\*.* *.caca
call "pack symbols.bat" WhenWasIt "Release Unicode" %1.%2.%3.%4 unicode
exit

error:
echo "Error packing WhenWasIt"
