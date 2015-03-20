for %%* in (.) do set CurrDirName=%%~n*
set langdir=%CD%
set lang=%CurrDirName%

del /f /q %langdir%\Untranslated\*.txt

pushd ..\..\tools\lpgen
call lpgen.js.bat
cscript /nologo translate.js /sourcelang:"%lang%" /noref:"yes" /popuntranslated:"yes" /out:"%langdir%" /untranslated:"%langdir%\Untranslated"
popd