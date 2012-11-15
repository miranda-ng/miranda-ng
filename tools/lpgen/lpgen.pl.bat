call lpgen.pl

for /F %%i in ('dir /B /S ..\..\plugins\*-translation.txt') do copy /Y %%i ..\..\i18n

for /F %%i in ('dir /B /S ..\..\protocols\*-translation.txt') do copy /Y %%i ..\..\i18n

for /F %%i in ('dir /B /S ..\..\src\*-translation.txt') do copy /Y %%i ..\..\i18n

for /F %%i in ('dir /B /S ..\..\tools\*-translation.txt') do copy /Y %%i ..\..\i18n

pushd ..\..\i18n

if exist langpack_english.txt del /Q langpack_english.txt


for %%j in (*-translation.txt) do type "%%j" >> langpack_english.txt

del /Q *-translation.txt

popd