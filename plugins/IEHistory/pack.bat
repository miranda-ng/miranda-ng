for /F "tokens=5-9* delims=. " %%i in (docs\IEHistory_readme.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
D:\usr\PowerArchiver\pacl\pacomp.exe -a -c2 "IEHistory %1.%2.%3.%4.zip" @files_release.txt
D:\usr\PowerArchiver\pacl\pacomp.exe -p -a -c2 "IEHistory %1.%2.%3.%4.zip" docs\*.* *.caca
call "pack symbols.bat" IEHistory IEHistory %1.%2.%3.%4
exit

error:
echo "Error packing IEHistory"