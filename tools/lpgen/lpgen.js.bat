rem echo off
cscript /nologo lpgen.js
copy ..\..\langpacks\english\"=HEAD=.txt"+..\..\langpacks\english\"=CORE=.txt" +..\..\langpacks\english\Plugins\*.txt ..\..\langpacks\english\Langpack_english.txt