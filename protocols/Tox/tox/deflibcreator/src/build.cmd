@echo off
rc /d "_UNICODE" /d "UNICODE" /fo".\release\resource.res" resource.rc
cl /EHsc /DUNICODE /D_UNICODE /Ox /Oi /Ot /DWIN32 /DNDEBUG /nologo /GL /TP /GS /Gz /MD deflibcreator.cpp /link /LTCG /release /manifest:no /Machine:x86 .\release\resource.res /out:.\release\deflibcreator.exe
del deflibcreator.obj
del .\release\resource.res