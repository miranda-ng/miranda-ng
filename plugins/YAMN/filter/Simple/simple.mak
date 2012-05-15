# Microsoft Developer Studio Generated NMAKE File, Based on simple.dsp
!IF "$(CFG)" == ""
CFG=simple - Win32 Release
!MESSAGE No configuration specified. Defaulting to simple - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "simple - Win32 Release" && "$(CFG)" != "simple - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "simple.mak" CFG="simple - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "simple - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "simple - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "simple - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\..\bin\release\plugins\YAMN-filter\simple.dll"


CLEAN :
	-@erase "$(INTDIR)\maindll.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\simple.exp"
	-@erase "..\..\..\..\bin\release\plugins\YAMN-filter\simple.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\simple.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simple.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\simple.pdb" /machine:I386 /out:"../../../../bin/release/plugins/YAMN-filter/simple.dll" /implib:"$(OUTDIR)\simple.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\maindll.obj"

"..\..\..\..\bin\release\plugins\YAMN-filter\simple.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "simple - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\..\..\bin\Debug\plugins\YAMN-filter\simple.dll" "$(OUTDIR)\simple.bsc"


CLEAN :
	-@erase "$(INTDIR)\maindll.obj"
	-@erase "$(INTDIR)\maindll.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\simple.bsc"
	-@erase "$(OUTDIR)\simple.exp"
	-@erase "$(OUTDIR)\simple.pdb"
	-@erase "..\..\..\..\bin\Debug\plugins\YAMN-filter\simple.dll"
	-@erase "..\..\..\..\bin\Debug\plugins\YAMN-filter\simple.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\simple.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\simple.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\maindll.sbr"

"$(OUTDIR)\simple.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\simple.pdb" /debug /machine:I386 /out:"../../../../bin/Debug/plugins/YAMN-filter/simple.dll" /implib:"$(OUTDIR)\simple.lib" 
LINK32_OBJS= \
	"$(INTDIR)\maindll.obj"

"..\..\..\..\bin\Debug\plugins\YAMN-filter\simple.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("simple.dep")
!INCLUDE "simple.dep"
!ELSE 
!MESSAGE Warning: cannot find "simple.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "simple - Win32 Release" || "$(CFG)" == "simple - Win32 Debug"
SOURCE=.\maindll.cpp

!IF  "$(CFG)" == "simple - Win32 Release"


"$(INTDIR)\maindll.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "simple - Win32 Debug"


"$(INTDIR)\maindll.obj"	"$(INTDIR)\maindll.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 


!ENDIF 

