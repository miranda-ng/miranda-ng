@echo off
rem Cleanup
if exist tmp rd /S /Q tmp
if exist InnoNG_32 rd /S /Q InnoNG_32
if exist InnoNG_64 rd /S /Q InnoNG_64
if exist Tools\ispack*.exe del /F /Q Tools\ispack*.exe
rem end