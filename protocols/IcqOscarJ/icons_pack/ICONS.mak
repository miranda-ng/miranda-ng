# Microsoft Developer Studio Generated NMAKE File, Based on ICONS.dsp
!IF "$(CFG)" == ""
CFG=ICONS - Win32 Release
!MESSAGE No configuration specified. Defaulting to ICONS - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "ICONS - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ICONS.mak" CFG="ICONS - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ICONS - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\bin\release\Icons\xstatus_ICQ.dll"


CLEAN :
	-@erase "$(INTDIR)\ICONS.res"
	-@erase "$(OUTDIR)\xstatus_ICQ.exp"
	-@erase "$(OUTDIR)\xstatus_ICQ.lib"
	-@erase "$(OUTDIR)\xstatus_ICQ.pdb"
	-@erase "..\..\..\bin\release\Icons\xstatus_ICQ.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /Zi /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ICONS_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\ICONS.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\ICONS.res" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ICONS.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /dll /incremental:no /pdb:"$(OUTDIR)\xstatus_ICQ.pdb" /debug /machine:I386 /out:"..\..\..\bin\release\Icons\xstatus_ICQ.dll" /implib:"$(OUTDIR)\xstatus_ICQ.lib" /noentry /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\ICONS.res"

"..\..\..\bin\release\Icons\xstatus_ICQ.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("ICONS.dep")
!INCLUDE "ICONS.dep"
!ELSE 
!MESSAGE Warning: cannot find "ICONS.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ICONS - Win32 Release"
SOURCE=.\ICONS.rc

"$(INTDIR)\ICONS.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

