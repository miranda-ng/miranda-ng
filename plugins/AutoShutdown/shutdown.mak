# Microsoft Developer Studio Generated NMAKE File, Based on shutdown.dsp
!IF "$(CFG)" == ""
CFG=shutdown - Win32 Release
!MESSAGE Keine Konfiguration angegeben. shutdown - Win32 Release wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "shutdown - Win32 Release" && "$(CFG)" != "shutdown - Win32 Debug" && "$(CFG)" != "shutdown - Win32 Release Unicode" && "$(CFG)" != "shutdown - Win32 Debug Unicode"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "shutdown.mak" CFG="shutdown - Win32 Release"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "shutdown - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "shutdown - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "shutdown - Win32 Release Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "shutdown - Win32 Debug Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "shutdown - Win32 Release"

OUTDIR=.\temp/Release/ANSI
INTDIR=.\temp/Release/ANSI
# Begin Custom Macros
OutDir=.\temp/Release/ANSI
# End Custom Macros

ALL : ".\release\ANSI\shutdown.dll" "$(OUTDIR)\shutdown.bsc"


CLEAN :
	-@erase "$(INTDIR)\cpuusage.obj"
	-@erase "$(INTDIR)\cpuusage.sbr"
	-@erase "$(INTDIR)\frame.obj"
	-@erase "$(INTDIR)\frame.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\settingsdlg.obj"
	-@erase "$(INTDIR)\settingsdlg.sbr"
	-@erase "$(INTDIR)\shutdownsvc.obj"
	-@erase "$(INTDIR)\shutdownsvc.sbr"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\utils.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\watcher.obj"
	-@erase "$(INTDIR)\watcher.sbr"
	-@erase "$(OUTDIR)\shutdown.bsc"
	-@erase "$(OUTDIR)\shutdown.exp"
	-@erase ".\release\ANSI\shutdown.dll"
	-@erase ".\release\ANSI\shutdown.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STRICT" /D "SHUTDOWN_EXPORTS" /U "NO_STRICT" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /opt:nowin98 /c 
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /i ".\include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\shutdown.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cpuusage.sbr" \
	"$(INTDIR)\frame.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\settingsdlg.sbr" \
	"$(INTDIR)\shutdownsvc.sbr" \
	"$(INTDIR)\utils.sbr" \
	"$(INTDIR)\watcher.sbr"

"$(OUTDIR)\shutdown.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib comctl32.lib gdi32.lib user32.lib shell32.lib advapi32.lib /nologo /base:"0x11070000" /dll /incremental:no /pdb:"./release/ANSI/shutdown.pdb" /debug /machine:I386 /nodefaultlib:"uuid.lib" /nodefaultlib:"OLDNAMES" /def:"shutdown.def" /out:"./release/ANSI/shutdown.dll" /implib:"$(OUTDIR)\shutdown.lib" /opt:nowin98 /ignore:4078 /RELEASE 
LINK32_OBJS= \
	"$(INTDIR)\cpuusage.obj" \
	"$(INTDIR)\frame.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\settingsdlg.obj" \
	"$(INTDIR)\shutdownsvc.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\watcher.obj" \
	"$(INTDIR)\resource.res"

".\release\ANSI\shutdown.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "shutdown - Win32 Debug"

OUTDIR=.\temp/Debug/ANSI
INTDIR=.\temp/Debug/ANSI
# Begin Custom Macros
OutDir=.\temp/Debug/ANSI
# End Custom Macros

ALL : "..\Miranda IM\ANSI\Plugins\shutdown.dll" "$(OUTDIR)\shutdown.bsc"


CLEAN :
	-@erase "$(INTDIR)\cpuusage.obj"
	-@erase "$(INTDIR)\cpuusage.sbr"
	-@erase "$(INTDIR)\frame.obj"
	-@erase "$(INTDIR)\frame.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\settingsdlg.obj"
	-@erase "$(INTDIR)\settingsdlg.sbr"
	-@erase "$(INTDIR)\shutdownsvc.obj"
	-@erase "$(INTDIR)\shutdownsvc.sbr"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\utils.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\watcher.obj"
	-@erase "$(INTDIR)\watcher.sbr"
	-@erase "$(OUTDIR)\shutdown.bsc"
	-@erase "$(OUTDIR)\shutdown.exp"
	-@erase "$(OUTDIR)\shutdown.map"
	-@erase "..\Miranda IM\ANSI\Plugins\shutdown.dll"
	-@erase "..\Miranda IM\ANSI\Plugins\shutdown.ilk"
	-@erase "..\Miranda IM\ANSI\Plugins\shutdown.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W4 /Gm /Gi /GR /GX /ZI /Od /I ".\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STRICT" /D "SHUTDOWN_EXPORTS" /U "NO_STRICT" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /i ".\include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\shutdown.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cpuusage.sbr" \
	"$(INTDIR)\frame.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\settingsdlg.sbr" \
	"$(INTDIR)\shutdownsvc.sbr" \
	"$(INTDIR)\utils.sbr" \
	"$(INTDIR)\watcher.sbr"

"$(OUTDIR)\shutdown.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib comctl32.lib gdi32.lib user32.lib shell32.lib advapi32.lib /nologo /base:"0x11070000" /dll /incremental:yes /pdb:"C:/Dokumente und Einstellungen/bib-nutzer/Desktop/Miranda IM/ANSI/Plugins/shutdown.pdb" /map:"$(INTDIR)\shutdown.map" /debug /machine:I386 /nodefaultlib:"uuid.lib" /nodefaultlib:"OLDNAMES" /def:"shutdown.def" /out:"C:/Dokumente und Einstellungen/bib-nutzer/Desktop/Miranda IM/ANSI/Plugins/shutdown.dll" /implib:"$(OUTDIR)\shutdown.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\cpuusage.obj" \
	"$(INTDIR)\frame.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\settingsdlg.obj" \
	"$(INTDIR)\shutdownsvc.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\watcher.obj" \
	"$(INTDIR)\resource.res"

"..\Miranda IM\ANSI\Plugins\shutdown.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "shutdown - Win32 Release Unicode"

OUTDIR=.\temp/Release/Unicode
INTDIR=.\temp/Release/Unicode
# Begin Custom Macros
OutDir=.\temp/Release/Unicode
# End Custom Macros

ALL : ".\release\Unicode\shutdown.dll" "$(OUTDIR)\shutdown.bsc"


CLEAN :
	-@erase "$(INTDIR)\cpuusage.obj"
	-@erase "$(INTDIR)\cpuusage.sbr"
	-@erase "$(INTDIR)\frame.obj"
	-@erase "$(INTDIR)\frame.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\settingsdlg.obj"
	-@erase "$(INTDIR)\settingsdlg.sbr"
	-@erase "$(INTDIR)\shutdownsvc.obj"
	-@erase "$(INTDIR)\shutdownsvc.sbr"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\utils.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\watcher.obj"
	-@erase "$(INTDIR)\watcher.sbr"
	-@erase "$(OUTDIR)\shutdown.bsc"
	-@erase "$(OUTDIR)\shutdown.exp"
	-@erase ".\release\Unicode\shutdown.dll"
	-@erase ".\release\Unicode\shutdown.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W4 /GX /O2 /I ".\include" /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "STRICT" /D "SHUTDOWN_EXPORTS" /U "_MBCS" /U "NO_STRICT" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\shutdown.pch" /YX"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /opt:nowin98 /c 
MTL_PROJ=/nologo /D "NDEBUG" /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /i ".\include" /d "NDEBUG" /d "_UNICODE" /d "UNICODE" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\shutdown.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cpuusage.sbr" \
	"$(INTDIR)\frame.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\settingsdlg.sbr" \
	"$(INTDIR)\shutdownsvc.sbr" \
	"$(INTDIR)\utils.sbr" \
	"$(INTDIR)\watcher.sbr"

"$(OUTDIR)\shutdown.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib comctl32.lib gdi32.lib user32.lib shell32.lib advapi32.lib /nologo /base:"0x11070000" /dll /incremental:no /pdb:"./release/Unicode/shutdown.pdb" /debug /machine:I386 /nodefaultlib:"uuid.lib" /nodefaultlib:"OLDNAMES" /def:"shutdown.def" /out:"./release/Unicode/shutdown.dll" /implib:"$(OUTDIR)\shutdown.lib" /opt:nowin98 /ignore:4078 /RELEASE 
LINK32_OBJS= \
	"$(INTDIR)\cpuusage.obj" \
	"$(INTDIR)\frame.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\settingsdlg.obj" \
	"$(INTDIR)\shutdownsvc.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\watcher.obj" \
	"$(INTDIR)\resource.res"

".\release\Unicode\shutdown.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "shutdown - Win32 Debug Unicode"

OUTDIR=.\temp/Debug/Unicode
INTDIR=.\temp/Debug/Unicode
# Begin Custom Macros
OutDir=.\temp/Debug/Unicode
# End Custom Macros

ALL : "..\Miranda IM\Unicode\Plugins\shutdown.dll" "$(OUTDIR)\shutdown.bsc"


CLEAN :
	-@erase "$(INTDIR)\cpuusage.obj"
	-@erase "$(INTDIR)\cpuusage.sbr"
	-@erase "$(INTDIR)\frame.obj"
	-@erase "$(INTDIR)\frame.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\settingsdlg.obj"
	-@erase "$(INTDIR)\settingsdlg.sbr"
	-@erase "$(INTDIR)\shutdownsvc.obj"
	-@erase "$(INTDIR)\shutdownsvc.sbr"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\utils.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\watcher.obj"
	-@erase "$(INTDIR)\watcher.sbr"
	-@erase "$(OUTDIR)\shutdown.bsc"
	-@erase "$(OUTDIR)\shutdown.exp"
	-@erase "$(OUTDIR)\shutdown.map"
	-@erase "..\Miranda IM\Unicode\Plugins\shutdown.dll"
	-@erase "..\Miranda IM\Unicode\Plugins\shutdown.ilk"
	-@erase "..\Miranda IM\Unicode\Plugins\shutdown.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W4 /Gm /Gi /GR /GX /ZI /Od /I ".\include" /D "_DEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "STRICT" /D "SHUTDOWN_EXPORTS" /U "_MBCS" /U "NO_STRICT" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /i ".\include" /d "_DEBUG" /d "_UNICODE" /d "UNICODE" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\shutdown.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\cpuusage.sbr" \
	"$(INTDIR)\frame.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\settingsdlg.sbr" \
	"$(INTDIR)\shutdownsvc.sbr" \
	"$(INTDIR)\utils.sbr" \
	"$(INTDIR)\watcher.sbr"

"$(OUTDIR)\shutdown.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib comctl32.lib gdi32.lib user32.lib shell32.lib advapi32.lib /nologo /base:"0x11070000" /dll /incremental:yes /pdb:"C:/Dokumente und Einstellungen/bib-nutzer/Desktop/Miranda IM/Unicode/Plugins/shutdown.pdb" /map:"$(INTDIR)\shutdown.map" /debug /machine:I386 /nodefaultlib:"uuid.lib" /nodefaultlib:"OLDNAMES" /def:"shutdown.def" /out:"C:/Dokumente und Einstellungen/bib-nutzer/Desktop/Miranda IM/Unicode/Plugins/shutdown.dll" /implib:"$(OUTDIR)\shutdown.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\cpuusage.obj" \
	"$(INTDIR)\frame.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\settingsdlg.obj" \
	"$(INTDIR)\shutdownsvc.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\watcher.obj" \
	"$(INTDIR)\resource.res"

"..\Miranda IM\Unicode\Plugins\shutdown.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("shutdown.dep")
!INCLUDE "shutdown.dep"
!ELSE 
!MESSAGE Warning: cannot find "shutdown.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "shutdown - Win32 Release" || "$(CFG)" == "shutdown - Win32 Debug" || "$(CFG)" == "shutdown - Win32 Release Unicode" || "$(CFG)" == "shutdown - Win32 Debug Unicode"
SOURCE=.\cpuusage.c

"$(INTDIR)\cpuusage.obj"	"$(INTDIR)\cpuusage.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\frame.c

"$(INTDIR)\frame.obj"	"$(INTDIR)\frame.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\main.c

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\options.c

"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\settingsdlg.c

"$(INTDIR)\settingsdlg.obj"	"$(INTDIR)\settingsdlg.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\shutdownsvc.c

"$(INTDIR)\shutdownsvc.obj"	"$(INTDIR)\shutdownsvc.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\utils.c

"$(INTDIR)\utils.obj"	"$(INTDIR)\utils.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\watcher.c

"$(INTDIR)\watcher.obj"	"$(INTDIR)\watcher.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

