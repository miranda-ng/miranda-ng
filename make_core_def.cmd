@echo off
echo.
.\tools\makedef\makedef.exe /IMAGE:bin12\debug\libs\mir_core.dll /DEF:src\mir_core\src\mir_core.def
echo.
.\tools\makedef\makedef64.exe /IMAGE:bin12\debug64\libs\mir_core.dll /DEF:src\mir_core\src\mir_core64.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin12\debug\libs\mir_app.dll /DEF:src\mir_app\src\mir_app.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin12\debug64\libs\mir_app.dll /DEF:src\mir_app\src\mir_app64.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin12\debug\libs\libjson.dll /DEF:libs\libjson\src\libjson.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin12\debug64\libs\libjson.dll /DEF:libs\libjson\src\libjson64.def
