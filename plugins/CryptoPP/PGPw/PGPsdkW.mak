# Microsoft Developer Studio Generated NMAKE File, Based on PGPsdkW.dsp
!IF "$(CFG)" == ""
CFG=PGPsdkW - Win32 Release6
!MESSAGE No configuration specified. Defaulting to PGPsdkW - Win32 Release6.
!ENDIF 

!IF "$(CFG)" != "PGPsdkW - Win32 Release6" && "$(CFG)" != "PGPsdkW - Win32 Release8"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPsdkW.mak" CFG="PGPsdkW - Win32 Release6"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPsdkW - Win32 Release6" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPsdkW - Win32 Release8" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "PGPsdkW - Win32 Release6"

OUTDIR=.\Release6
INTDIR=.\Release6
# Begin Custom Macros
OutDir=.\Release6
# End Custom Macros

ALL : "$(OUTDIR)\PGPsdkW6.dll"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\PGPsdkW.pch"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\PGPsdkW6.dll"
	-@erase "$(OUTDIR)\PGPsdkW6.exp"
	-@erase "$(OUTDIR)\PGPsdkW6.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Gi /GX /O1 /I "sdk6/include" /D PGP_WIN32=0x658 /Fp"$(INTDIR)\PGPsdkW.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PGPsdkW.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=delayimp.lib pgp_sdk.lib pgpsdknl.lib pgpsdkui.lib kernel32.lib user32.lib shell32.lib gdi32.lib msimg32.lib comdlg32.lib msvcrt.lib /nologo /base:"0x46600000" /dll /incremental:no /pdb:"$(OUTDIR)\PGPsdkW6.pdb" /map:"$(INTDIR)\PGPsdkW6.map" /machine:I386 /out:"$(OUTDIR)\PGPsdkW6.dll" /implib:"$(OUTDIR)\PGPsdkW6.lib" /libpath:"sdk6/lib" /filealign:0x200 /delayload:pgp_sdk.dll /delayload:pgpsdkui.dll 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\main.obj"

"$(OUTDIR)\PGPsdkW6.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PGPsdkW - Win32 Release8"

OUTDIR=.\Release8
INTDIR=.\Release8
# Begin Custom Macros
OutDir=.\Release8
# End Custom Macros

ALL : "$(OUTDIR)\PGPsdkW8.dll"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\PGPsdkW.pch"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\PGPsdkW8.dll"
	-@erase "$(OUTDIR)\PGPsdkW8.exp"
	-@erase "$(OUTDIR)\PGPsdkW8.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Gi /GX /O1 /I "sdk8/include" /D PGP_WIN32=0x800 /Fp"$(INTDIR)\PGPsdkW.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PGPsdkW.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=delayimp.lib pgpsdk.lib pgpsdknl.lib pgpsdkui.lib kernel32.lib user32.lib shell32.lib gdi32.lib msimg32.lib comdlg32.lib msvcrt.lib /nologo /base:"0x46800000" /dll /incremental:no /pdb:"$(OUTDIR)\PGPsdkW8.pdb" /map:"$(INTDIR)\PGPsdkW8.map" /machine:I386 /out:"$(OUTDIR)\PGPsdkW8.dll" /implib:"$(OUTDIR)\PGPsdkW8.lib" /libpath:"sdk8/lib" /filealign:0x200 /delayload:pgpsdk.dll /delayload:pgpsdkui.dll 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\main.obj"

"$(OUTDIR)\PGPsdkW8.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("PGPsdkW.dep")
!INCLUDE "PGPsdkW.dep"
!ELSE 
!MESSAGE Warning: cannot find "PGPsdkW.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "PGPsdkW - Win32 Release6" || "$(CFG)" == "PGPsdkW - Win32 Release8"
SOURCE=.\commonheaders.cpp

!IF  "$(CFG)" == "PGPsdkW - Win32 Release6"

CPP_SWITCHES=/nologo /MD /W3 /Gi /GX /O1 /I "sdk6/include" /D PGP_WIN32=0x658 /Fp"$(INTDIR)\PGPsdkW.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\PGPsdkW.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "PGPsdkW - Win32 Release8"

CPP_SWITCHES=/nologo /MD /W3 /Gi /GX /O1 /I "sdk8/include" /D PGP_WIN32=0x800 /Fp"$(INTDIR)\PGPsdkW.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\PGPsdkW.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\PGPsdkW.pch"



!ENDIF 

