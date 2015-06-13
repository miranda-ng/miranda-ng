@echo off
copy /Y .\Bin12\Debug\Obj\mir_core\mir_core.lib .\Bin10\lib
copy /Y .\Bin12\Debug64\Obj\mir_core\mir_core64.lib .\Bin10\lib

copy /Y .\Bin12\Debug\Obj\mir_core\mir_core.lib .\Bin12\lib
copy /Y .\Bin12\Debug64\Obj\mir_core\mir_core64.lib .\Bin12\lib

copy /Y .\Bin12\Debug\Obj\mir_app\mir_app.lib .\Bin10\lib\mir_app.lib
copy /Y .\Bin12\Debug64\Obj\mir_app\mir_app.lib .\Bin10\lib\mir_app64.lib

copy /Y .\Bin12\Debug\Obj\mir_app\mir_app.lib .\Bin12\lib\mir_app.lib
copy /Y .\Bin12\Debug64\Obj\mir_app\mir_app.lib .\Bin12\lib\mir_app64.lib
