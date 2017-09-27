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

call a_Sleep.bat