# Microsoft Developer Studio Generated NMAKE File, Based on ICONS_NOVA.dsp
!IF "$(CFG)" == ""
CFG=ICONS_NOVA - Win32 Release
!MESSAGE No configuration specified. Defaulting to ICONS_NOVA - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "ICONS_NOVA - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ICONS_NOVA.mak" CFG="ICONS_NOVA - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ICONS_NOVA - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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

ALL : "..\..\..\..\bin\Release Unicode\Icons\tabsrmm_icons.dll"


CLEAN :
	-@erase "$(INTDIR)\ICONS_NOVA.res"
	-@erase "$(OUTDIR)\tabsrmm_icons.exp"
	-@erase "..\..\..\..\bin\Release Unicode\Icons\tabsrmm_icons.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ICONS_NOVA.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /dll /pdb:none /machine:I386 /nodefaultlib /out:"..\..\..\..\bin\Release Unicode\Icons\tabsrmm_icons.dll" /implib:"$(OUTDIR)\tabsrmm_icons.lib" /noentry 
LINK32_OBJS= \
	"$(INTDIR)\ICONS_NOVA.res"

"..\..\..\..\bin\Release Unicode\Icons\tabsrmm_icons.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /ML /Za /W3 /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ICONS_NOVA_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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

MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc07 /fo"$(INTDIR)\ICONS_NOVA.res" /d "NDEBUG" 

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("ICONS_NOVA.dep")
!INCLUDE "ICONS_NOVA.dep"
!ELSE 
!MESSAGE Warning: cannot find "ICONS_NOVA.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ICONS_NOVA - Win32 Release"
SOURCE=.\ICONS_NOVA.rc

"$(INTDIR)\ICONS_NOVA.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

