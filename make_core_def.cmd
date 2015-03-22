@echo off
echo.
.\tools\makedef\makedef.exe /IMAGE:bin12\debug\mir_core.dll /DEF:src\mir_core\src\mir_core.def
echo.
.\tools\makedef\makedef64.exe /IMAGE:bin12\debug64\mir_core.dll /DEF:src\mir_core\src\mir_core64.def