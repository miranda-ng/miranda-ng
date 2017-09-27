call git_update.bat master
set GIT_STATUS=%ERRORLEVEL%
if %GIT_STATUS%==0 echo Git update success
if not %GIT_STATUS%==0 goto :Error

call z1_ReBuild_Full.bat 32

call z1_ReBuild_Full.bat 64

call z2_PackPluginUpdater.bat 32 DEV_N_STABLE

call z2_PackPluginUpdater.bat 64 DEV_N_STABLE

pushd bin10
if exist z_Errors*.txt echo There were errors! && pause
popd

call z3_PackArchives.bat 32

call z3_PackArchives.bat 64

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