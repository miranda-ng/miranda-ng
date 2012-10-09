rem @echo off
REM -Fi/u are for include/unit dirs
REM -Mdelphi is delphi mode
REM -WG - graphical app
REM -v0 turn off warnings
REM -O2 -Os // optimise
REM -Rintel (intel style asm)
REM -WB (relocatable) -WR (relocate)
set OUTDIR="..\..\bin10\Release\Plugins" 
if not exist %OUTDIR% mkdir %OUTDIR%
md out
fpc shlext.dpr -FEout -Fi..\..\include;..\..\include\delphi -Fi..\ExternalAPI\delphi -Fu..\..\include;..\..\include\delphi -Mdelphi -WG -O2 -Os -Rintel -WR -WB49ac0000 -v0

move out\shlext.dll .
del /Q out\*
rd /Q out

move /y shlext.dll ..\..\bin10\Release\Plugins\ShlExt.dll