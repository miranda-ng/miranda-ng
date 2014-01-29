RD /S /Q  backup\
RD /S /Q  lib\
del langpackmgr.res
strip langpackmgr.exe
upx -9 --lzma langpackmgr.exe
