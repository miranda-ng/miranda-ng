for /F "tokens=5-9* delims=. " %%i in (docs\folders_readme.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
d:\usr\7z\7za.exe a -i@files_source.txt -xr!*.zip -xr!*.ncb -xr!*.user -xr!*.sdf "Folders src %1.%2.%3.%4.zip" @files_source.txt 
exit

error:
echo "Error packing Folders"
