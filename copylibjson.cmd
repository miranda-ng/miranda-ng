@echo off
copy /Y .\Bin12\Debug\Obj\libjson\libjson.lib .\Bin10\lib\libjson.lib
copy /Y .\Bin12\Debug64\Obj\libjson\libjson.lib .\Bin10\lib\libjson64.lib

copy /Y .\Bin12\Debug\Obj\libjson\libjson.lib .\Bin12\lib\libjson.lib
copy /Y .\Bin12\Debug64\Obj\libjson\libjson.lib .\Bin12\lib\libjson64.lib

copy /Y .\Bin12\Debug\Obj\libjson\libjson.lib .\Bin14\lib\libjson.lib
copy /Y .\Bin12\Debug64\Obj\libjson\libjson.lib .\Bin14\lib\libjson64.lib
