set tp=%1
if  "%tp%"=="" (echo "please specify target platform 32 or 64!"&&pause&&goto :EOF)

call a_SetVar%tp%.bat

if exist zzReport.txt del /f /q zzReport.txt

:upload
%WinSCP% /script=z4_UploadPluginUpdater.txt

if errorlevel 1 goto error

echo Upload Plugin Updater Success! >> zzReport.txt
if exist %Arch% rd /Q /S "%Arch%" >nul
goto end

:error
echo Upload Plugin Updater x%tp% Error! >> zzReport.txt

:end

