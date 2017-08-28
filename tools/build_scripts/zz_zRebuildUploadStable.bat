@echo off
echo WARNING!!! You are about to compile and upload STABLE versions of Miranda NG! Are you absolutely sure?!
pause

call git_update.bat
set GIT_STATUS=%ERRORLEVEL%
if %GIT_STATUS%==0 echo Git update success
if not %GIT_STATUS%==0 goto :Error

call z1_ReBuild_Full_Stable.bat 32

call z1_ReBuild_Full_Stable.bat 64

call z2_PackPluginUpater.bat 32

call z2_PackPluginUpater.bat 64

pushd bin10
if exist z_Errors*.txt echo There were errors! && pause
popd

call z3_PackArchivesStable.bat 32

call z3_PackArchivesStable.bat 64

call z4_UploadPluginUpdaterStable.bat 32

call z4_UploadPluginUpdaterStable.bat 64

call z5_UploadArchivesStable.bat PASS

goto :eof

:Error
echo GIT FAIL! > git_error.txt
echo ============================= GIT FAIL! =============================
pause
exit