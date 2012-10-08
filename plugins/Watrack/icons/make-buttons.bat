:@echo off
if /i '%1' == '' (set asm=tasm) else set asm=%1
if /i '%2' == '' (set iconpack=true+256-solid) else set iconpack=%2
call make.bat buttons %iconpack% %asm%