rem @echo off

cl.exe /MT /O2 /Os /EHsc /I..\..\..\include SendLog.cpp SLImp.cpp /link ws2_32.lib user32.lib
cl.exe /MT /O2 /Os /EHsc  /I..\..\..\include SendLogWin.cpp SLImp.cpp /link ws2_32.lib user32.lib

mkdir ..\..\..\%1\Release64 2>nul
copy /Y *.exe ..\..\..\%1\Release64 >nul

del *.obj;*.exe >nul
goto :eof
