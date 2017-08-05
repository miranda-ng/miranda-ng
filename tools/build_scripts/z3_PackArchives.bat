set tp=%1
if  "%tp%"=="" (echo "please specify target platform 32 or 64!"&&pause&&goto :EOF)
if /i '%tp%' == '64' set bit=_x64

call a_SetVar%tp%.bat

if not exist %ArchDistr% mkdir %ArchDistr%

cd bin10
rem for /F "tokens=2" %%x in (..\build\build.no) do set ver2=%%x
rem for /F "tokens=3" %%y in (..\build\build.no) do set ver3=%%y

cd Symbols%tp%
%CompressIt% a -mx=9 "miranda-ng-debug-symbols_pdb%bit%.7z" *.pdb .\Plugins\*.pdb .\Core\*.pdb .\Libs\*.pdb
move /Y miranda-ng-debug*.7z %ArchDistr%
cd ../..

pushd "bin10\Release%tp%"

rem if exist %AutoCompile%\miranda-ng-v0.9*-alpha-latest%bit%.7z del /F /Q %AutoCompile%\miranda-ng-v0.9*-alpha-latest%bit%.7z
%CompressIt% a -r -mx=9 "miranda-ng-alpha-latest%bit%.7z" Miranda%tp%.exe -i@..\..\z3_PackArchives.txt
move /Y miranda-ng*.7z %ArchDistr%

del /f /q miranda%tp%.exe
del /f /q hashes.txt
for /f %%a in (..\..\z3_PackArchives.txt) do del /s /q  %%a
rd /s /q Core
REM rd /s /q Libs
rd /s /q Languages

%CompressIt% a -r -mx=9 "miranda-ng-plugins-latest%bit%.7z"
move /Y miranda-ng-plugins*.7z %ArchDistr%

cd ..

if /i '%tp%' == '32' (
if exist "Release" rd /Q /S "Release" >nul
)
if exist "Release%tp%" rd /Q /S "Release%tp%" >nul
if exist "Symbols%tp%" rd /Q /S "Symbols%tp%" >nul

popd