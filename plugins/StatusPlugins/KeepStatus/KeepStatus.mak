# Microsoft Developer Studio Generated NMAKE File, Based on KeepStatus.dsp
!IF "$(CFG)" == ""
CFG=KeepStatus - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to KeepStatus - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "KeepStatus - Win32 Release" && "$(CFG)" != "KeepStatus - Win32 Debug" && "$(CFG)" != "KeepStatus - Win32 Debug Unicode" && "$(CFG)" != "KeepStatus - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "KeepStatus.mak" CFG="KeepStatus - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "KeepStatus - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "KeepStatus - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "KeepStatus - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "KeepStatus - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "KeepStatus - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\bin\release\plugins\KeepStatus.dll"


CLEAN :
	-@erase "$(INTDIR)\action_keepstatus.obj"
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\keepstatus.obj"
	-@erase "$(INTDIR)\KeepStatus.pch"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\trigger_keepstatus.obj"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\KeepStatus.exp"
	-@erase "$(OUTDIR)\KeepStatus.pdb"
	-@erase "..\..\..\bin\release\plugins\KeepStatus.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\KeepStatus.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msvcrt.lib wininet.lib Delayimp.lib ws2_32.lib /nologo /base:"0x11cd0000" /dll /incremental:no /pdb:"$(OUTDIR)\KeepStatus.pdb" /debug /machine:I386 /nodefaultlib /out:"../../../bin/release/plugins/KeepStatus.dll" /implib:"$(OUTDIR)\KeepStatus.lib" /delayload:wininet.dll 
LINK32_OBJS= \
	"$(INTDIR)\action_keepstatus.obj" \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\keepstatus.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\trigger_keepstatus.obj" \
	"$(INTDIR)\vc6.res"

"..\..\..\bin\release\plugins\KeepStatus.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\..\bin\debug\plugins\KeepStatus.dll" "$(OUTDIR)\KeepStatus.bsc"


CLEAN :
	-@erase "$(INTDIR)\action_keepstatus.obj"
	-@erase "$(INTDIR)\action_keepstatus.sbr"
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\commonstatus.sbr"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\confirmdialog.sbr"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\gen_helpers.sbr"
	-@erase "$(INTDIR)\keepstatus.obj"
	-@erase "$(INTDIR)\KeepStatus.pch"
	-@erase "$(INTDIR)\keepstatus.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\trigger_keepstatus.obj"
	-@erase "$(INTDIR)\trigger_keepstatus.sbr"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\KeepStatus.bsc"
	-@erase "$(OUTDIR)\KeepStatus.exp"
	-@erase "$(OUTDIR)\KeepStatus.pdb"
	-@erase "..\..\..\bin\debug\plugins\KeepStatus.dll"
	-@erase "..\..\..\bin\debug\plugins\KeepStatus.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\KeepStatus.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\action_keepstatus.sbr" \
	"$(INTDIR)\commonstatus.sbr" \
	"$(INTDIR)\confirmdialog.sbr" \
	"$(INTDIR)\gen_helpers.sbr" \
	"$(INTDIR)\keepstatus.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\trigger_keepstatus.sbr"

"$(OUTDIR)\KeepStatus.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=comctl32.lib wininet.lib Delayimp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\KeepStatus.pdb" /debug /machine:I386 /out:"../../../bin/debug/plugins/KeepStatus.dll" /implib:"$(OUTDIR)\KeepStatus.lib" /pdbtype:sept /DelayLoad:wininet.dll 
LINK32_OBJS= \
	"$(INTDIR)\action_keepstatus.obj" \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\keepstatus.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\trigger_keepstatus.obj" \
	"$(INTDIR)\vc6.res"

"..\..\..\bin\debug\plugins\KeepStatus.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\..\bin\Debug Unicode\plugins\KeepStatus.dll" "$(OUTDIR)\KeepStatus.bsc"


CLEAN :
	-@erase "$(INTDIR)\action_keepstatus.obj"
	-@erase "$(INTDIR)\action_keepstatus.sbr"
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\commonstatus.sbr"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\confirmdialog.sbr"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\gen_helpers.sbr"
	-@erase "$(INTDIR)\keepstatus.obj"
	-@erase "$(INTDIR)\KeepStatus.pch"
	-@erase "$(INTDIR)\keepstatus.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\trigger_keepstatus.obj"
	-@erase "$(INTDIR)\trigger_keepstatus.sbr"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\KeepStatus.bsc"
	-@erase "$(OUTDIR)\KeepStatus.exp"
	-@erase "$(OUTDIR)\KeepStatus.pdb"
	-@erase "..\..\..\bin\Debug Unicode\plugins\KeepStatus.dll"
	-@erase "..\..\..\bin\Debug Unicode\plugins\KeepStatus.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\KeepStatus.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\action_keepstatus.sbr" \
	"$(INTDIR)\commonstatus.sbr" \
	"$(INTDIR)\confirmdialog.sbr" \
	"$(INTDIR)\gen_helpers.sbr" \
	"$(INTDIR)\keepstatus.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\trigger_keepstatus.sbr"

"$(OUTDIR)\KeepStatus.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=comctl32.lib wininet.lib Delayimp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\KeepStatus.pdb" /debug /machine:I386 /out:"../../../bin/Debug Unicode/plugins/KeepStatus.dll" /implib:"$(OUTDIR)\KeepStatus.lib" /pdbtype:sept /DelayLoad:wininet.dll 
LINK32_OBJS= \
	"$(INTDIR)\action_keepstatus.obj" \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\keepstatus.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\trigger_keepstatus.obj" \
	"$(INTDIR)\vc6.res"

"..\..\..\bin\Debug Unicode\plugins\KeepStatus.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\..\bin\Release Unicode\plugins\KeepStatus.dll"


CLEAN :
	-@erase "$(INTDIR)\action_keepstatus.obj"
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\keepstatus.obj"
	-@erase "$(INTDIR)\KeepStatus.pch"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\trigger_keepstatus.obj"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\KeepStatus.exp"
	-@erase "$(OUTDIR)\KeepStatus.pdb"
	-@erase "..\..\..\bin\Release Unicode\plugins\KeepStatus.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\KeepStatus.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msvcrt.lib wininet.lib Delayimp.lib ws2_32.lib /nologo /base:"0x11cd0000" /dll /incremental:no /pdb:"$(OUTDIR)\KeepStatus.pdb" /debug /machine:I386 /nodefaultlib /out:"../../../bin/Release Unicode/plugins/KeepStatus.dll" /implib:"$(OUTDIR)\KeepStatus.lib" /delayload:wininet.dll 
LINK32_OBJS= \
	"$(INTDIR)\action_keepstatus.obj" \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\keepstatus.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\trigger_keepstatus.obj" \
	"$(INTDIR)\vc6.res"

"..\..\..\bin\Release Unicode\plugins\KeepStatus.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("KeepStatus.dep")
!INCLUDE "KeepStatus.dep"
!ELSE 
!MESSAGE Warning: cannot find "KeepStatus.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "KeepStatus - Win32 Release" || "$(CFG)" == "KeepStatus - Win32 Debug" || "$(CFG)" == "KeepStatus - Win32 Debug Unicode" || "$(CFG)" == "KeepStatus - Win32 Release Unicode"
SOURCE=.\action_keepstatus.cpp

!IF  "$(CFG)" == "KeepStatus - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\action_keepstatus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\action_keepstatus.obj"	"$(INTDIR)\action_keepstatus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\action_keepstatus.obj"	"$(INTDIR)\action_keepstatus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\action_keepstatus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\commonstatus.cpp

!IF  "$(CFG)" == "KeepStatus - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonstatus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonstatus.obj"	"$(INTDIR)\commonstatus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonstatus.obj"	"$(INTDIR)\commonstatus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonstatus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\confirmdialog.cpp

!IF  "$(CFG)" == "KeepStatus - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\confirmdialog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\confirmdialog.obj"	"$(INTDIR)\confirmdialog.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\confirmdialog.obj"	"$(INTDIR)\confirmdialog.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /Fp"$(INTDIR)\KeepStatus.pch" /Yu"commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\confirmdialog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=..\..\helpers\gen_helpers.cpp

!IF  "$(CFG)" == "KeepStatus - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\gen_helpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\gen_helpers.obj"	"$(INTDIR)\gen_helpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\gen_helpers.obj"	"$(INTDIR)\gen_helpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\gen_helpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\keepstatus.cpp

!IF  "$(CFG)" == "KeepStatus - Win32 Release"


"$(INTDIR)\keepstatus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"


"$(INTDIR)\keepstatus.obj"	"$(INTDIR)\keepstatus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"


"$(INTDIR)\keepstatus.obj"	"$(INTDIR)\keepstatus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"


"$(INTDIR)\keepstatus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ENDIF 

SOURCE=.\main.cpp

!IF  "$(CFG)" == "KeepStatus - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /Fp"$(INTDIR)\KeepStatus.pch" /Yc"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\KeepStatus.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\KeepStatus.pch" /Yc"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr"	"$(INTDIR)\KeepStatus.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\KeepStatus.pch" /Yc"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr"	"$(INTDIR)\KeepStatus.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /Fp"$(INTDIR)\KeepStatus.pch" /Yc"../commonstatus.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\KeepStatus.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\options.cpp

!IF  "$(CFG)" == "KeepStatus - Win32 Release"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ENDIF 

SOURCE=.\trigger_keepstatus.cpp

!IF  "$(CFG)" == "KeepStatus - Win32 Release"


"$(INTDIR)\trigger_keepstatus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"


"$(INTDIR)\trigger_keepstatus.obj"	"$(INTDIR)\trigger_keepstatus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"


"$(INTDIR)\trigger_keepstatus.obj"	"$(INTDIR)\trigger_keepstatus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"


"$(INTDIR)\trigger_keepstatus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\KeepStatus.pch"


!ENDIF 

SOURCE=.\vc6.rc

"$(INTDIR)\vc6.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

