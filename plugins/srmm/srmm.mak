# Microsoft Developer Studio Generated NMAKE File, Based on srmm.dsp
!IF "$(CFG)" == ""
CFG=srmm - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to srmm - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "srmm - Win32 Release" && "$(CFG)" != "srmm - Win32 Debug" && "$(CFG)" != "srmm - Win32 Release Unicode" && "$(CFG)" != "srmm - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "srmm.mak" CFG="srmm - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "srmm - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "srmm - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "srmm - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "srmm - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "srmm - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\release\plugins\srmm.dll"


CLEAN :
	-@erase "$(INTDIR)\cmdlist.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\msgtimedout.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\richutil.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\srmm.pch"
	-@erase "$(INTDIR)\statusicon.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\srmm.exp"
	-@erase "$(OUTDIR)\srmm.map"
	-@erase "$(OUTDIR)\srmm.pdb"
	-@erase "..\..\bin\release\plugins\srmm.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\srmm.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\srmm.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\srmm.pdb" /map:"$(INTDIR)\srmm.map" /debug /machine:I386 /out:"../../bin/release/plugins/srmm.dll" /implib:"$(OUTDIR)\srmm.lib" 
LINK32_OBJS= \
	"$(INTDIR)\cmdlist.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\msgtimedout.obj" \
	"$(INTDIR)\richutil.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\statusicon.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\plugins\srmm.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "srmm - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\bin\debug\Plugins\srmm.dll"


CLEAN :
	-@erase "$(INTDIR)\cmdlist.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\msgtimedout.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\richutil.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\srmm.pch"
	-@erase "$(INTDIR)\statusicon.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\srmm.exp"
	-@erase "$(OUTDIR)\srmm.pdb"
	-@erase "..\..\bin\debug\Plugins\srmm.dll"
	-@erase "..\..\bin\debug\Plugins\srmm.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\srmm.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\srmm.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\srmm.pdb" /debug /machine:I386 /out:"../../bin/debug/Plugins/srmm.dll" /implib:"$(OUTDIR)\srmm.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\cmdlist.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\msgtimedout.obj" \
	"$(INTDIR)\richutil.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\statusicon.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug\Plugins\srmm.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "srmm - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\Release Unicode\plugins\srmm.dll"


CLEAN :
	-@erase "$(INTDIR)\cmdlist.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\msgtimedout.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\richutil.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\srmm.pch"
	-@erase "$(INTDIR)\statusicon.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\srmm.exp"
	-@erase "$(OUTDIR)\srmm.map"
	-@erase "$(OUTDIR)\srmm.pdb"
	-@erase "..\..\bin\Release Unicode\plugins\srmm.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "UNICODE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\srmm.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "NDEBUG" /d "UNICODE" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\srmm.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\srmm.pdb" /map:"$(INTDIR)\srmm.map" /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/srmm.dll" /implib:"$(OUTDIR)\srmm.lib" 
LINK32_OBJS= \
	"$(INTDIR)\cmdlist.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\msgtimedout.obj" \
	"$(INTDIR)\richutil.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\statusicon.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Release Unicode\plugins\srmm.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "srmm - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode

ALL : "..\..\bin\Debug Unicode\Plugins\srmm.dll"


CLEAN :
	-@erase "$(INTDIR)\cmdlist.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\msgtimedout.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\richutil.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\srmm.pch"
	-@erase "$(INTDIR)\statusicon.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\srmm.exp"
	-@erase "$(OUTDIR)\srmm.pdb"
	-@erase "..\..\bin\Debug Unicode\Plugins\srmm.dll"
	-@erase "..\..\bin\Debug Unicode\Plugins\srmm.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "UNICODE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\srmm.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "_DEBUG" /d "UNICODE" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\srmm.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\srmm.pdb" /debug /machine:I386 /out:"../../bin/Debug Unicode/Plugins/srmm.dll" /implib:"$(OUTDIR)\srmm.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\cmdlist.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\msgtimedout.obj" \
	"$(INTDIR)\richutil.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\statusicon.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Debug Unicode\Plugins\srmm.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("srmm.dep")
!INCLUDE "srmm.dep"
!ELSE 
!MESSAGE Warning: cannot find "srmm.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "srmm - Win32 Release" || "$(CFG)" == "srmm - Win32 Debug" || "$(CFG)" == "srmm - Win32 Release Unicode" || "$(CFG)" == "srmm - Win32 Debug Unicode"
SOURCE=.\cmdlist.c

"$(INTDIR)\cmdlist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"


SOURCE=.\globals.c

"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"


SOURCE=.\msgdialog.c

"$(INTDIR)\msgdialog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"


SOURCE=.\msglog.c

"$(INTDIR)\msglog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"


SOURCE=.\msgoptions.c

"$(INTDIR)\msgoptions.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"


SOURCE=.\msgs.c

"$(INTDIR)\msgs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"


SOURCE=.\msgtimedout.c

"$(INTDIR)\msgtimedout.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"


SOURCE=.\richutil.c

"$(INTDIR)\richutil.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"


SOURCE=.\srmm.c

!IF  "$(CFG)" == "srmm - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\srmm.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\srmm.obj"	"$(INTDIR)\srmm.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "srmm - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\srmm.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\srmm.obj"	"$(INTDIR)\srmm.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "srmm - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "UNICODE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\srmm.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\srmm.obj"	"$(INTDIR)\srmm.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "srmm - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "UNICODE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\srmm.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\srmm.obj"	"$(INTDIR)\srmm.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\statusicon.c

"$(INTDIR)\statusicon.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

