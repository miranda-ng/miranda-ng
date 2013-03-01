:first  parameter - 'icons' or 'buttons' - type of iconpack ()
:second parameter - iconpack name (for buttons mainly)
:third  parameter - assembler? (tasm)
:@echo off
if /i '%1' == '' (set pack=buttons) else set pack=%1
if '%pack%' == 'icons' goto players
if /i '%2' == '' (set iconpack=true+256-solid) else set iconpack=%2
goto next
:players
if /i '%2' == '' (set iconpack=players) else set iconpack=%2
:next
if /i '%3' == '' (set asm=tasm) else set asm=%3
:@echo off
cd %asm%
call icons.bat %pack% ..\iconsets\%iconpack% %4 %5 %6 %7 %8 %9
cd ..\
exit /b 0
