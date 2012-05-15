# Microsoft Developer Studio Generated NMAKE File, Based on proto_YAMN.dsp
!IF "$(CFG)" == ""
CFG=proto_YAMN - Win32 Release
!MESSAGE No configuration specified. Defaulting to proto_YAMN - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "proto_YAMN - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "proto_YAMN.mak" CFG="proto_YAMN - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "proto_YAMN - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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

ALL : "..\..\..\bin\release\icons\proto_YAMN.dll"


CLEAN :
	-@erase "$(INTDIR)\proto_YAMN.res"
	-@erase "$(OUTDIR)\proto_YAMN.exp"
	-@erase "$(OUTDIR)\proto_YAMN.lib"
	-@erase "..\..\..\bin\release\icons\proto_YAMN.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /Fo"$(INTDIR)\\" 

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
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\proto_YAMN.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\proto_YAMN.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=/nologo /subsystem:windows /dll /pdb:none /machine:I386 /nodefaultlib /out:"../../../bin/release/icons/proto_YAMN.dll" /implib:"$(OUTDIR)\proto_YAMN.lib" /filealign:512 /noentry 
LINK32_OBJS= \
	"$(INTDIR)\proto_YAMN.res"

"..\..\..\bin\release\icons\proto_YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("proto_YAMN.dep")
!INCLUDE "proto_YAMN.dep"
!ELSE 
!MESSAGE Warning: cannot find "proto_YAMN.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "proto_YAMN - Win32 Release"
SOURCE=proto_YAMN.rc

"$(INTDIR)\proto_YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

