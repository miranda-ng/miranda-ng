@echo off
copy /Y .\Bin14\Debug\Obj\mir_core\mir_core.lib .\Bin14\lib
copy /Y .\Bin14\Debug64\Obj\mir_core\mir_core.lib .\Bin14\lib\mir_core64.lib

copy /Y .\Bin14\Debug\Obj\mir_app\mir_app.lib .\Bin14\lib\mir_app.lib
copy /Y .\Bin14\Debug64\Obj\mir_app\mir_app.lib .\Bin14\lib\mir_app64.lib

copy /Y .\Bin14\Debug\Obj\libjson\libjson.lib .\Bin14\lib\libjson.lib
copy /Y .\Bin14\Debug64\Obj\libjson\libjson.lib .\Bin14\lib\libjson64.lib
