set comp=bin15

call git_update.bat master
set GIT_STATUS=%ERRORLEVEL%
if %GIT_STATUS%==0 echo Git update success
if not %GIT_STATUS%==0 goto :Error

xcopy /S /Y tools\build_scripts\*.* . >nul

call z1_ReBuild_Full.bat 32 %comp%

call z1_ReBuild_Full.bat 64 %comp%

call z2_PackPluginUpdater.bat 32 DEV_N_STABLE %comp%

call z2_PackPluginUpdater.bat 64 DEV_N_STABLE %comp%

pushd %comp%
if exist z_Errors*.txt echo There were errors! && pause
popd

call z3_PackArchives.bat 32 %comp%

call z3_PackArchives.bat 64 %comp%

call z4_UploadPluginUpdater.bat 32

call z4_UploadPluginUpdater.bat 64

call z5_UploadArchives.bat PASS

call z6_CompileInstallers.bat

call z7_UploadInstallers.bat

goto :eof

:Error
echo GIT FAIL! > git_error.txt
echo ============================= GIT FAIL! =============================
pause
exit