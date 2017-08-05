set p1=%1
if  "%p1%" == "" (echo "please spesify 'PASS' parameter to make this batch file work."&&pause&&goto :EOF)

call a_SetVar32.bat

:upload
%WinSCP% /script=z5_UploadArchives.txt

if errorlevel 1 goto error

echo Upload Archives Success! >> zzReport.txt
goto end

:error
echo Upload Archives Error! >> zzReport.txt

:end