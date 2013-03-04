rem @echo off
if not "%VS100COMNTOOLS%" == "" (
   call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat"
   "%VS100COMNTOOLS%\..\..\vc\bin\cl.exe" /MD /O2 /Os /EHsc SendLog.cpp SLImp.cpp /link ws2_32.lib user32.lib
   "%VS100COMNTOOLS%\..\..\vc\bin\cl.exe" /MD /O2 /Os /EHsc SendLogWin.cpp SLImp.cpp /link ws2_32.lib user32.lib
   call :ProcessFiles 10
)

if not "%VS110COMNTOOLS%" == "" (
   call "%VS110COMNTOOLS%\..\..\vc\vcvarsall.bat"
   "%VS110COMNTOOLS%\..\..\vc\bin\cl.exe" /MD /O2 /Os /EHsc SendLog.cpp SLImp.cpp /link ws2_32.lib user32.lib
   "%VS110COMNTOOLS%\..\..\vc\bin\cl.exe" /MD /O2 /Os /EHsc SendLogWin.cpp SLImp.cpp /link ws2_32.lib user32.lib
   call :ProcessFiles 11
)
goto :eof

:ProcessFiles
mkdir ..\..\..\bin%1\Debug 2>nul
copy /Y *.exe ..\..\..\bin%1\Debug >nul

mkdir ..\..\..\bin%1\Debug64 2>nul
copy /Y *.exe ..\..\..\bin%1\Debug64 >nul

mkdir ..\..\..\bin%1\Release 2>nul
copy /Y *.exe ..\..\..\bin%1\Release >nul

mkdir ..\..\..\bin%1\Release64 2>nul
copy /Y *.exe ..\..\..\bin%1\Release64 >nul

del *.obj;*.exe >nul
goto :eof
