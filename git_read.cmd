@echo off
git pull --rebase=true --progress -v 2>&1 > "%temp%\report.txt"
git submodule update --recursive --rebase
call .\build\make_ver.bat
start "" "%temp%\report.txt"
goto :eof
