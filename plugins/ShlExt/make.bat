@echo off
REM -Fi/u are for include/unit dirs 
REM -Mdelphi is delphi mode
REM -WG - graphical app
REM -v0 turn off warnings
REM -O2 -Os // optimise
REM -Rintel (intel style asm)
REM -WB (relocatable) -WR (relocate)
fpc shlext.dpr -Fiinc -Fuinc -Mdelphi -WG -O2 -Os -Rintel -WR -WB49ac0000 -v0