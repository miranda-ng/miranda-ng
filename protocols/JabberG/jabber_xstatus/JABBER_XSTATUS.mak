# Microsoft Developer Studio Generated NMAKE File, Based on JABBER_XSTATUS.dsp
!IF "$(CFG)" == ""
CFG=JABBER_XSTATUS - Win32 Release
!MESSAGE No configuration specified. Defaulting to JABBER_XSTATUS - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "JABBER_XSTATUS - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "JABBER_XSTATUS.mak" CFG="JABBER_XSTATUS - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "JABBER_XSTATUS - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\bin\release\ICONS\xstatus_jabber.dll"


CLEAN :
	-@erase "$(INTDIR)\JABBER_XSTATUS.res"
	-@erase "$(OUTDIR)\xstatus_jabber.exp"
	-@erase "..\..\..\bin\release\ICONS\xstatus_jabber.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\JABBER_XSTATUS.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /dll /pdb:none /machine:I386 /nodefaultlib /out:"..\..\..\bin\release\ICONS\xstatus_jabber.dll" /implib:"$(OUTDIR)\xstatus_jabber.lib" /noentry /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\JABBER_XSTATUS.res"

"..\..\..\bin\release\ICONS\xstatus_jabber.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /ML /Za /W3 /GX- /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "JABBER_XSTATUS_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\JABBER_XSTATUS.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /c 

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

MTL_PROJ=/nologo /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\JABBER_XSTATUS.res" 

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("JABBER_XSTATUS.dep")
!INCLUDE "JABBER_XSTATUS.dep"
!ELSE 
!MESSAGE Warning: cannot find "JABBER_XSTATUS.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "JABBER_XSTATUS - Win32 Release"
SOURCE=.\JABBER_XSTATUS.rc

"$(INTDIR)\JABBER_XSTATUS.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

