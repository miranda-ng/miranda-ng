for %%* in (.) do set CurrDirName=%%~n*
set langdir=%CD%
set lang=%CurrDirName%

pushd ..\..\tools\lpgen
cscript /nologo translate.js /sourcelang:"%lang%" /noref:"yes" /popuntranslated:"yes" /out:"%langdir%" /untranslated:"%langdir%\Untranslated"
popd