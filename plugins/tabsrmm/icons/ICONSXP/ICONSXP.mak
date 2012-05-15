# Microsoft Developer Studio Generated NMAKE File, Based on ICONSXP.dsp
!IF "$(CFG)" == ""
CFG=ICONSXP - Win32 Debug
!MESSAGE No configuration specified. Defaulting to ICONSXP - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ICONSXP - Win32 Debug" && "$(CFG)" != "ICONSXP - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ICONSXP.mak" CFG="ICONSXP - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ICONSXP - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ICONSXP - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "ICONSXP - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\ICONSXP.dll"


CLEAN :
	-@erase "$(INTDIR)\ICONSXP.res"
	-@erase "$(OUTDIR)\ICONSXP.dll"
	-@erase "$(OUTDIR)\ICONSXP.ilk"
	-@erase "$(OUTDIR)\ICONSXP.pdb"
	-@erase ".\$(OutDir)\ICONSXP.exp"
	-@erase ".\$(OutDir)\ICONSXP.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ICONSXP_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\ICONSXP.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\ICONSXP.res" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ICONSXP.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\ICONSXP.pdb" /debug /machine:IX86 /out:"$(OUTDIR)\ICONSXP.dll" /implib:"$(OutDir)/ICONSXP.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\ICONSXP.res"

"$(OUTDIR)\ICONSXP.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ICONSXP - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\tabsrmm_icons.dll"


CLEAN :
	-@erase "$(INTDIR)\ICONSXP.res"
	-@erase "$(OUTDIR)\tabsrmm_icons.dll"
	-@erase ".\$(OutDir)\ICONSXP.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /Zi /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ICONSXP_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\ICONSXP.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\ICONSXP.res" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\ICONSXP.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\tabsrmm_icons.pdb" /machine:IX86 /nodefaultlib /out:"$(OUTDIR)\tabsrmm_icons.dll" /implib:"$(OutDir)/ICONSXP.lib" /pdbtype:sept /opt:ref /opt:icf /noentry 
LINK32_OBJS= \
	"$(INTDIR)\ICONSXP.res"

"$(OUTDIR)\tabsrmm_icons.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("ICONSXP.dep")
!INCLUDE "ICONSXP.dep"
!ELSE 
!MESSAGE Warning: cannot find "ICONSXP.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "ICONSXP - Win32 Debug" || "$(CFG)" == "ICONSXP - Win32 Release"
SOURCE=.\ICONSXP.rc

"$(INTDIR)\ICONSXP.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

