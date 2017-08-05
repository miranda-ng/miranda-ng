rem @echo off
if not "%VS100COMNTOOLS%" == "" (
   call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat"
   "%VS100COMNTOOLS%\..\..\vc\bin\cl.exe" /MD /O2 /Os /EHsc /I..\..\..\include SendLog.cpp SLImp.cpp /link ws2_32.lib user32.lib
   "%VS100COMNTOOLS%\..\..\vc\bin\cl.exe" /MD /O2 /Os /EHsc  /I..\..\..\include SendLogWin.cpp SLImp.cpp /link ws2_32.lib user32.lib
   call :ProcessFiles 10
)

goto :eof

:ProcessFiles
mkdir ..\..\..\bin%1\Release64 2>nul
copy /Y *.exe ..\..\..\bin%1\Release64 >nul

del *.obj;*.exe >nul
goto :eof
