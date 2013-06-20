@echo off
SET /P USER=Skype username:
IF "%USER%"=="" (
  echo Username is mandatory, aborting.
  GOTO fini
)
SET /P PASS=Skype password:
IF "%PASS%"=="" (
  echo Password is mandatory, aborting.
  GOTO fini
)
SET /P IP=Bind to IP [0.0.0.0]:
SET /P PORT=Bind to Port [1401]:
SET /P DAEM=Do you want to daemonize [N]:
IF /I "%DAEM%" NEQ "Y" SET /P VERB=Do you want verbose command output [N]:
IF /I "%DAEM%"=="Y" SET CMDLINE=%CMDLINE% -d
IF /I "%VERB%"=="Y" SET CMDLINE=%CMDLINE% -v
IF NOT "%IP%"=="" SET CMDLINE=%CMDLINE% -h %IP%
IF NOT "%PORT%"=="" SET CMDLINE=%CMDLINE% -p %PORT%
SET CMDLINE=%CMDLINE% %USER% %PASS%
echo imo2sproxy %CMDLINE% >startimo.bat
echo startimo.bat created.
:fini
pause
