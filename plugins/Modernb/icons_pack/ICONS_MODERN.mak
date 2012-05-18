# Microsoft Developer Studio Generated NMAKE File, Based on ICONS_MODERN.dsp
!IF "$(CFG)" == ""
CFG=ICONS_MODERN - Win32 Release
!MESSAGE No configuration specified. Defaulting to ICONS_MODERN - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "ICONS_MODERN - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ICONS_MODERN.mak" CFG="ICONS_MODERN - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ICONS_MODERN - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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

ALL : "..\..\..\bin\Release\Icons\toolbar_icons.dll"


CLEAN :
	-@erase "$(INTDIR)\ICONS.res"
	-@erase "$(OUTDIR)\toolbar_icons.exp"
	-@erase "..\..\..\bin\Release\Icons\toolbar_icons.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /Za /W3 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ICONS_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\ICONS_MODERN.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ICONS_MODERN.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:none /machine:IX86 /nodefaultlib /out:"..\..\..\bin\Release\Icons\toolbar_icons.dll" /implib:"$(OUTDIR)\toolbar_icons.lib" /noentry /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\ICONS.res"

"..\..\..\bin\Release\Icons\toolbar_icons.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("ICONS_MODERN.dep")
!INCLUDE "ICONS_MODERN.dep"
!ELSE 
!MESSAGE Warning: cannot find "ICONS_MODERN.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ICONS_MODERN - Win32 Release"
SOURCE=ICONS.rc

"$(INTDIR)\ICONS.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

