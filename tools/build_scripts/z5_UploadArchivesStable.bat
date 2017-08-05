set p1=%1
if  "%p1%" == "" (echo "please spesify 'PASS' parameter to make this batch file work."&&pause&&goto :EOF)

call a_SetVar32.bat

:upload
%WinSCP% /script=z5_UploadArchivesStable.txt

if errorlevel 1 goto error

echo Upload Archives and Installers Success! >> zzReport.txt
if exist %ArchDistr% rd /Q /S "%ArchDistr%%" >nul
goto end

:error
echo Upload Archives and Installers Error! >> zzReport.txt

:end