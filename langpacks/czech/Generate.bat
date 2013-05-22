rem echo off
cd "..\..\tools\lpgen\"

mkdir "Plugins"
mkdir "Untranslated"

cscript /nologo translate.js /log:"yes" /out:"Plugins" /untranslated:"Untranslated" /outfile:"langpack_czech.txt" /path:"..\..\langpacks\czech\Plugins" /core:"..\..\langpacks\czech\=CORE=.txt" /langpack:"..\..\langpacks\czech\Deprecated\old langpack\langpack_czech2.txt"

rm -r "..\..\langpacks\czech\Plugins"
rm -r "..\..\langpacks\czech\Untranslated"
rm -r "..\..\langpacks\czech\=CORE=.txt"
rm -r "..\..\langpacks\czech\Langpack_czech.txt"

mkdir "..\..\langpacks\czech\Plugins"
mkdir "..\..\langpacks\czech\Untranslated"
copy "Plugins\" "..\..\langpacks\czech\Plugins\"
copy "Untranslated\" "..\..\langpacks\czech\Untranslated\"
copy "=CORE=.txt" "..\..\langpacks\czech\=CORE=.txt"

rm -r "Plugins"
rm -r "Untranslated"
rm "=CORE=.txt"

cd "..\..\langpacks\tool\"
LangpackSuite.exe