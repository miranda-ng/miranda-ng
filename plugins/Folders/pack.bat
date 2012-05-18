copy folders\m_folders.h docs /y
for /F "tokens=5-9* delims=. " %%i in (docs\folders_readme.txt) do (call :Pack %%i %%j %%k %%l; exit)

:Pack
xcopy folders\win32\release\folders.dll . /y
xcopy folders\m_folders.h docs\m_folders.h /y
d:\usr\7z\7za.exe a -i!docs\*.* -i!Folders.dll "Folders %1.%2.%3.%4 x32.zip"
del folders.dll
call "pack symbols.bat" Folders Folders %1.%2.%3.%4
exit

error:
echo "Error packing Folders"