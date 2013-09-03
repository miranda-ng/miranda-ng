@echo off
rem Cleanup
rd /S /Q tmp
rd /S /Q util
for /D %%p in ("InnoNG_32\*.*") do rd "%%p" /s /q
for /D %%p in ("InnoNG_64\*.*") do rd "%%p" /s /q
rem end