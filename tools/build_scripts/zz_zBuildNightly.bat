set comp=bin10

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

call a_Sleep.bat