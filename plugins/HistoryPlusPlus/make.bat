@echo off

set tp=%1
if "%tp%"=="" (echo "please specify target platform 32 or 64!" && pause && goto :EOF)

rc version.rc

for /F "tokens=1,2,3" %%i in ('reg.exe query "HKLM\SOFTWARE\Microsoft\MSBuild\ToolsVersions\4.0" /v MSBuildToolsPath') do call :SetPath %%i %%j %%k
echo Using .NET Framework Directory '%FrameworkDir%'
set PATH=%FrameworkDir%;%PATH%

MsBuild.exe historypp.dproj /t:Rebuild /p:Configuration=Release;Platform=Win%tp% /fileLogger /fileLoggerParameters:LogFile=Logs\icons%tp%.log;errorsonly;warningsonly;summary
goto :eof

:SetPath
if "%2" == "REG_SZ" set FrameworkDir="%3"
goto :eof
