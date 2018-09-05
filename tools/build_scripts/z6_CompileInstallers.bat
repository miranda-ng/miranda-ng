call a_SetVar32.bat
if not exist %ArchDistr% mkdir %ArchDistr%

pushd tools\installer_ng
call createstructure.bat
mkdir tmp
copy /V /Y ..\..\build\build.no tmp\build.no
pushd tmp
for /F "tokens=1,2,3 delims= " %%i in (build.no) do set MirVer=%%i.%%j.%%k
popd
%ISCC% /Dptx86 /DAppVer=%MirVer% /O"Output" "InnoNG_32\MirandaNG.iss"
%ISCC% /DAppVer=%MirVer% /O"Output" "InnoNG_64\MirandaNG.iss"
call cleanup.bat
copy /V /Y Output\miranda-ng-alpha-latest*.exe "%ArchDistr%\"
rd /S /Q Output
popd
