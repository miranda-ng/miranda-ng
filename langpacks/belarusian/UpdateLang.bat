for %%* in (.) do set CurrDirName=%%~n*
set langdir=%CD%
set lang=%CurrDirName%

pushd ..\..\tools\lpgen
cscript /nologo translate.js /sourcelang:"%lang%" /popuntranslated:"yes" /out:"%langdir%"
popd