for /F "tokens=3-7* delims=. " %%i in (readme_versioninfo.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
d:\usr\PowerArchiver\pacl\pacomp.exe -a -c2 "VersionInfo %1.%2.%3.%4 x64.zip" @"Pack files\files_releasex64.txt"
d:\usr\PowerArchiver\pacl\pacomp.exe -p -a -c2 "VersionInfo %1.%2.%3.%4 x64.zip" ..\docs\*.txt ..\*.caca -xsymbols_exclude.txt
call "pack symbols.bat" VersionInfo "" %1.%2.%3.%4
exit

error:
echo "Error packing Versioninfo"
