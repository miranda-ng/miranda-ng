for /F "tokens=6-10* delims=. " %%i in (docs\WhenWasIt_readme.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -c2 "WhenWasIt src %1.%2.%3.%4.zip" @files_source.txt -x*.zip -x*.ncb -x*.user -x*.sdf
exit

error:
echo "Error packing WhenWasIt"
