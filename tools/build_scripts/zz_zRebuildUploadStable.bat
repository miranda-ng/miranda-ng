@echo off
echo WARNING!!! You are about to compile and upload STABLE versions of Miranda NG! Are you absolutely sure?!
pause

set comp=bin15

for /F "tokens=1,2,3,4 delims= " %%i in (build/build.no.stable) do (
  if "%%l" == "" (
     set Branch=%%i_%%j_%%k
     set MirVer=%%i.%%j.%%k
  ) else (
     set Branch=%%i_%%j_%%k_%%l
     set MirVer=%%i.%%j.%%k.%%l
  )
)

call git_update.bat %Branch%
set GIT_STATUS=%ERRORLEVEL%
if %GIT_STATUS%==0 echo Git update success
if not %GIT_STATUS%==0 goto :Error

xcopy /S /Y tools\build_scripts\*.* . >nul

del /F /Q version.txt

pushd build
copy build.no.stable build.no
copy make_ver_stable.bat make_ver.bat
popd

echo Building v%MirVer% > version.txt

call z1_ReBuild_Full.bat 32 %comp%

call z1_ReBuild_Full.bat 64 %comp%

call z2_PackPluginUpdater.bat 32 DEV_STABLE %comp%

call z2_PackPluginUpdater.bat 64 DEV_STABLE %comp%

pushd %comp%
if exist z_Errors*.txt echo There were errors! && pause
popd

call z3_PackArchivesStable.bat 32 %comp%

call z3_PackArchivesStable.bat 64 %comp%

call z4_UploadPluginUpdaterStable.bat 32

call z4_UploadPluginUpdaterStable.bat 64

call z5_UploadArchivesStable.bat PASS

goto :eof

:Error
echo GIT FAIL! > git_error.txt
echo ============================= GIT FAIL! =============================
pause
exit