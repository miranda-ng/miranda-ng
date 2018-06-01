@echo off

cl.exe /MD /O2 /Os /EHsc /I..\..\..\include SendLog.cpp SLImp.cpp /link ws2_32.lib user32.lib
cl.exe /MD /O2 /Os /EHsc  /I..\..\..\include SendLogWin.cpp SLImp.cpp /link ws2_32.lib user32.lib

mkdir ..\..\..\%1\Release 2>nul
copy /Y *.exe ..\..\..\%1\Release >nul

del *.obj;*.exe >nul
goto :eof
