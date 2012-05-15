"%ProgramFiles%\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.exe" /REBUILD "Release" menuex.sln
"%ProgramFiles%\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv.exe" /REBUILD "Release Unicode" menuex.sln
"%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe" /REBUILD "Release Unicode|x64" menuex_9.sln
if not exist plugins md plugins
copy /Y Release\plugins\MenuEx.dll plugins\MenuEx.dll
if exist menuitemex.zip del menuitemex.zip
"%ProgramFiles%\7-Zip\7z.exe" a -mx9 menuitemex.zip plugins\MenuEx.dll docs\menuex_changelog.txt
copy /Y "Release Unicode\plugins\MenuEx.dll" plugins\MenuEx.dll
if exist menuitemexw.zip del menuitemexw.zip
"%ProgramFiles%\7-Zip\7z.exe" a -mx9 menuitemexw.zip plugins\MenuEx.dll docs\menuex_changelog.txt
copy /Y "Release Unicode x64\plugins\MenuEx.dll" plugins\MenuEx.dll
if exist menuitemexxw.zip del menuitemexxw.zip
"%ProgramFiles%\7-Zip\7z.exe" a -mx9 menuitemexxw.zip plugins\MenuEx.dll docs\menuex_changelog.txt
del plugins\MenuEx.dll
if exist menuitemex_src.zip del menuitemex_src.zip
"%ProgramFiles%\7-Zip\7z.exe" a -mx9 menuitemex_src.zip *.c *.h *.rc *.vcproj *.vcxproj *.vcxproj.filters sdk\*.h icons\*.ico docs\*.txt
