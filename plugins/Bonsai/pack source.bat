for /F "tokens=4-8* delims=. " %%i in (docs\Bonsai_readme.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -c2 "Bonsai src %1.%2.%3.%4.zip" @files_source.txt -x*.zip -x*.ncb -x*.user
exit

error:
echo "Error packing Bonsai"
