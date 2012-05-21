for /F "tokens=3-7* delims=. " %%i in (readme_versioninfo.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -c2 "VersionInfo src %1.%2.%3.%4.zip" @"pack files\files_source.txt" -x*.zip -x*.ncb -x*.user
exit

error:
echo "Error packing Versioninfo"
