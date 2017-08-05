rem del /F /Q version.txt

svn cleanup

svn update > update.log

cd build

for /F %%g in ('svnversion build.no') do set Revision=%%g

for /F "tokens=1,2,3 delims= " %%i in (build.no) do set MirVer=%%i.%%j.%%k

cd ..

rem echo v%MirVer%_%Revision% > version.txt

rem svn log -l 30 >> version.txt

echo %date% %time% > timestamp.chk