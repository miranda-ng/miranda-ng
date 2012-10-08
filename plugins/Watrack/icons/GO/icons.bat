:@echo off
GoRC /r /d incpath="%2" icons.rc 
:GoRC /r icons.rc 
GoAsm watrack_buttons.asm
GoLink /dll watrack_buttons.obj icons.res
del *.obj
del *.res
move watrack_buttons.dll ..\..\..\bin