call a_SetVar32.bat

:upload
%WinSCP% /script=z7_UploadInstallers.txt

if errorlevel 1 goto error

echo Upload Installers Success! >> zzReport.txt
if exist %ArchDistr% rd /Q /S "%ArchDistr%%" >nul
goto end

:error
echo Upload Installers Error! >> zzReport.txt

:end