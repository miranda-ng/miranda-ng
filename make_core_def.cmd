@echo off
echo.
.\tools\makedef\makedef.exe /IMAGE:bin18\debug\libs\mir_core.mir /DEF:src\mir_core\src\mir_core.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin18\debug64\libs\mir_core.mir /DEF:src\mir_core\src\mir_core64.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin18\debug\libs\mir_app.mir /DEF:src\mir_app\src\mir_app.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin18\debug64\libs\mir_app.mir /DEF:src\mir_app\src\mir_app64.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin18\debug\libs\libjson.mir /DEF:libs\libjson\src\libjson.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin18\debug64\libs\libjson.mir /DEF:libs\libjson\src\libjson64.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin18\debug\libs\tdapi.mir /DEF:protocols\Telegram\tdlib\src\tdapi.def
echo.
.\tools\makedef\makedef.exe /IMAGE:bin18\debug64\libs\tdapi.mir /DEF:protocols\Telegram\tdlib\src\tdapi64.def
