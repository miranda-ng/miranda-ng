@echo off
if /i '%1' == 'buttons' (set iconres=icons) else set iconres=iconspl
brcc32 %iconres%.rc -i%2 -foicons.res
tasm32 watrack.asm
tlink32 -Tpd watrack.obj,Watrack_%1.dll,,,,icons.res
del *.map
del *.obj
del *.res