@echo off

set tp=%1
if "%tp%"=="" (echo "please specify target platform 32 or 64!" && pause && goto :EOF)

rc version.rc

MsBuild.exe historypp.dproj /t:Rebuild /p:Configuration=Release;Platform=Win%tp% /fileLogger /fileLoggerParameters:LogFile=Logs\icons%tp%.log;errorsonly;warningsonly;summary
