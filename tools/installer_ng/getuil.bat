@echo off
rem Download tools needed for compiling installer
mkdir util
Tools\wget.exe -P util http://www.jrsoftware.org/download.php/ispack-unicode.exe
rem end