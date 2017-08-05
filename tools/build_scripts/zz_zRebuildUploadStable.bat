@echo off
echo WARNING!!! You are about to compile and upload STABLE versions of Miranda NG! Are you absolutely sure?!
pause

git_update.bat
set GIT_STATUS=%ERRORLEVEL%
if %GIT_STATUS%==0 echo Git update success
if not %GIT_STATUS%==0 goto :Error

pushd bin10
start /wait z1_ReBuild_w810.bat 32

start /wait z1_ReBuild_w810.bat 64
popd

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