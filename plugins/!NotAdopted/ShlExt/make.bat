@echo off
REM -Fi/u are for include/unit dirs
REM -Mdelphi is delphi mode
REM -WG - graphical app
REM -v0 turn off warnings
REM -O2 -Os // optimise
REM -Rintel (intel style asm)
REM -WB (relocatable) -WR (relocate)
md out
fpc shlext.dpr -FEout -Fi..\..\include;..\..\include\delphi -Fu..\..\include;..\..\include\delphi -Mdelphi -WG -O2 -Os -Rintel -WR -WB49ac0000 -v0
del /Q out\*
rd /Q out