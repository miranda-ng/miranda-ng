set tp=%1
if  "%tp%"=="" (echo "please specify target platform 32 or 64!"&&pause&&goto :EOF)
for /F "tokens=1,2,3 delims= " %%i in (build\build.no) do set MirVer=%%i.%%j.%%k
if /i '%tp%' == '64' set bit=_x64
if /i '%tp%' == '32' set CompileString=..\Tools\InnoSetup5\ISCC.exe /Dptx86 /DAppVer=%MirVer% "MirandaNG.iss" 
if /i '%tp%' == '64' set CompileString=..\Tools\InnoSetup5\ISCC.exe /DAppVer=%MirVer% "MirandaNG.iss"

set comp=%2
if "%comp%"=="" (echo "please specify target compiler folder!" && pause && goto :EOF)

call a_SetVar%tp%.bat
if not exist %ArchDistr% mkdir %ArchDistr%

pushd "%comp%\Symbols%tp%"
%CompressIt% a -mx=9 "miranda-ng-debug-symbols_pdb%bit%.7z" *.pdb .\Plugins\*.pdb .\Core\*.pdb .\Libs\*.pdb
move /Y miranda-ng-debug*.7z %ArchDistr%
popd

pushd "%comp%\Release%tp%"

if exist %ArchDistr%\miranda-ng-v0.9*%bit%.7z del /F /Q %ArchDistr%\miranda-ng-v0.9*%bit%.7z
if exist %ArchDistr%\miranda-ng-v0.9*%bit%.exe del /F /Q %ArchDistr%\miranda-ng-v0.9*%bit%.exe
%CompressIt% a -r -mx=9 "miranda-ng-v%MirVer%%bit%.7z" Miranda%tp%.exe -i@..\..\%comp%\z3_PackArchives.txt
move /Y miranda-ng*.7z %ArchDistr%

del /f /q miranda%tp%.exe
del /f /q *.dll
del /f /q /s *.iobj
del /f /q /s *.ipdb
del /f /q hashes.txt
for /f %%a in (..\..\z3_PackArchives.txt) do del /s /q  %%a
rd /s /q Core
rd /s /q Libs

rem Create directories and copy script and download needed files
pushd %StableInstDir%
mkdir tmp
mkdir InnoNG_%tp%
mkdir InnoNG_%tp%\Files\Languages
copy /V /Y MirandaNG.iss InnoNG_%tp%
xcopy Common\* InnoNG_%tp% /I /S /V /Y
Tools\wget.exe -O tmp\InnoSetup5.7z https://miranda-ng.org/distr/installer/InnoSetup5.7z
Tools\wget.exe -O tmp\MNG_Sounds.7z https://miranda-ng.org/distr/addons/Sounds/MNG_Sounds.7z
%CompressIt% x tmp\InnoSetup5.7z -y -oTools
%CompressIt% x %ArchDistr%\miranda-ng-v%MirVer%%bit%.7z -y -oInnoNG_%tp%\Files
%CompressIt% x tmp\MNG_Sounds.7z -y -oInnoNG_%tp%\Files
popd
rem end

copy /V /Y Languages\langpack_czech.txt %StableInstDir%\InnoNG_%tp%\Files\Languages
copy /V /Y Languages\langpack_german.txt %StableInstDir%\InnoNG_%tp%\Files\Languages
copy /V /Y Languages\langpack_polish.txt %StableInstDir%\InnoNG_%tp%\Files\Languages
copy /V /Y Languages\langpack_russian.txt %StableInstDir%\InnoNG_%tp%\Files\Languages
copy /V /Y Plugins\Clist_blind.dll %StableInstDir%\InnoNG_%tp%\Files\Plugins
copy /V /Y Plugins\Clist_nicer.dll %StableInstDir%\InnoNG_%tp%\Files\Plugins
copy /V /Y Plugins\Cln_skinedit.dll %StableInstDir%\InnoNG_%tp%\Files\Plugins
copy /V /Y Plugins\Scriver.dll %StableInstDir%\InnoNG_%tp%\Files\Plugins
rd /s /q Languages
pushd %StableInstDir%\InnoNG_%tp%
%CompileString%
cd ..
call cleanup.bat
move /Y miranda-ng-v0.9*%bit%.exe "%ArchDistr%\"
popd

REM %CompressIt% a -r -mx=9 "miranda-ng-plugins-stable%bit%.7z"
REM move /Y miranda-ng-plugins*.7z %ArchDistr%

cd ..

rem if exist "Release%tp%" rd /Q /S "Release%tp%" >nul
rem if exist "Symbols%tp%" rd /Q /S "Symbols%tp%" >nul

popd