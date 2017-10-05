set tp=%1
if "%tp%"=="" (echo "please specify target platform 32 or 64!" && pause && goto :EOF)
if /i '%tp%' == '64' set bit=_x64

set comp=%2
if "%comp%"=="" (echo "please specify target compiler folder!" && pause && goto :EOF)

call a_SetVar%tp%.bat

if not exist %ArchDistr% mkdir %ArchDistr%

cd %comp%
rem for /F "tokens=2" %%x in (..\build\build.no) do set ver2=%%x
rem for /F "tokens=3" %%y in (..\build\build.no) do set ver3=%%y

cd Symbols%tp%
%CompressIt% a -mx=9 "miranda-ng-debug-symbols_pdb%bit%.7z" *.pdb .\Plugins\*.pdb .\Core\*.pdb .\Libs\*.pdb
move /Y miranda-ng-debug*.7z %ArchDistr%
cd ../..

pushd "%comp%\Release%tp%"

%CompressIt% a -r -mx=9 "miranda-ng-alpha-latest%bit%.7z" Miranda%tp%.exe -i@..\..\%comp%\z3_PackArchives.txt
move /Y miranda-ng*.7z %ArchDistr%

del /f /q miranda%tp%.exe
del /f /q hashes.txt
del /f /q *.dll
del /f /q /s *.iobj
del /f /q /s *.ipdb
for /f %%a in (..\..\z3_PackArchives.txt) do del /s /q  %%a
rd /s /q Core
REM rd /s /q Libs
rd /s /q Languages

%CompressIt% a -r -mx=9 "miranda-ng-plugins-latest%bit%.7z"
move /Y miranda-ng-plugins*.7z %ArchDistr%

cd ..

rem if /i '%tp%' == '32' (
rem if exist "Release" rd /Q /S "Release" >nul
rem )
rem if exist "Release%tp%" rd /Q /S "Release%tp%" >nul
rem if exist "Symbols%tp%" rd /Q /S "Symbols%tp%" >nul

popd