@echo off
if /i '%1' == 'buttons' (set iconres=icons) else set iconres=iconspl
porc /i%2 %iconres%.rc /Foicons.res
poasm watrack.asm
polink /DLL /RELEASE /NODEFAULTLIB /NOENTRY /NOLOGO /OUT:watrack_%1.dll watrack.obj icons.res
del *.obj
del *.res
move watrack_%1.dll ..\..\..\bin