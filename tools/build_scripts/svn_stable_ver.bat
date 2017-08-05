del /F /Q version.txt

cd build

for /F "tokens=1,2,3 delims= " %%i in (build.no) do set MirVer=%%i.%%j.%%k

cd ..

echo v%MirVer% > version.txt

svn log -l 30 >> version.txt