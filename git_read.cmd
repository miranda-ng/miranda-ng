@echo off
git pull --rebase=true --progress -v 2>&1 > "%temp%\report.txt"
call .\build\make_ver.bat
start "" "%temp%\report.txt"
goto :eof
