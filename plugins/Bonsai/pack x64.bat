for /F "tokens=4-8* delims=. " %%i in (docs\Bonsai_readme.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
d:\usr\PowerArchiver\pacl\pacomp.exe -a -c2 "Bonsai %1.%2.%3.%4 x64.zip" @files_releasex64.txt
d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -c2 "Bonsai %1.%2.%3.%4 x64.zip" docs\*.* *.caca
call "pack symbols.bat" Bonsai FixOptions %1.%2.%3.%4
exit

error:
echo "Error packing Bonsai"
