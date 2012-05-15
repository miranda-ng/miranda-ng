# Microsoft Developer Studio Generated NMAKE File, Based on AdvancedAutoAway.dsp
!IF "$(CFG)" == ""
CFG=AdvancedAutoAway - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to AdvancedAutoAway - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "AdvancedAutoAway - Win32 Release" && "$(CFG)" != "AdvancedAutoAway - Win32 Debug" && "$(CFG)" != "AdvancedAutoAway - Win32 Debug Unicode" && "$(CFG)" != "AdvancedAutoAway - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AdvancedAutoAway.mak" CFG="AdvancedAutoAway - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AdvancedAutoAway - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "AdvancedAutoAway - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "AdvancedAutoAway - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "AdvancedAutoAway - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "AdvancedAutoAway - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\bin\release\plugins\AdvancedAutoAway.dll"


CLEAN :
	-@erase "$(INTDIR)\advancedautoaway.obj"
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\trigger_aaa.obj"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\AdvancedAutoAway.exp"
	-@erase "$(OUTDIR)\AdvancedAutoAway.lib"
	-@erase "$(OUTDIR)\AdvancedAutoAway.pdb"
	-@erase "..\..\..\bin\release\plugins\AdvancedAutoAway.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ADVANCEDAUTOAWAY_EXPORTS" /Fp"$(INTDIR)\AdvancedAutoAway.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AdvancedAutoAway.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msvcrt.lib /nologo /base:"0x11bd0000" /dll /incremental:no /pdb:"$(OUTDIR)\AdvancedAutoAway.pdb" /debug /machine:I386 /nodefaultlib /out:"../../../bin/release/plugins/AdvancedAutoAway.dll" /implib:"$(OUTDIR)\AdvancedAutoAway.lib" 
LINK32_OBJS= \
	"$(INTDIR)\advancedautoaway.obj" \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\trigger_aaa.obj" \
	"$(INTDIR)\vc6.res"

"..\..\..\bin\release\plugins\AdvancedAutoAway.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\..\bin\debug\plugins\AdvancedAutoAway.dll" "$(OUTDIR)\AdvancedAutoAway.bsc"


CLEAN :
	-@erase "$(INTDIR)\advancedautoaway.obj"
	-@erase "$(INTDIR)\advancedautoaway.sbr"
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\commonstatus.sbr"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\confirmdialog.sbr"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\gen_helpers.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgoptions.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\trigger_aaa.obj"
	-@erase "$(INTDIR)\trigger_aaa.sbr"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\AdvancedAutoAway.bsc"
	-@erase "$(OUTDIR)\AdvancedAutoAway.exp"
	-@erase "$(OUTDIR)\AdvancedAutoAway.lib"
	-@erase "$(OUTDIR)\AdvancedAutoAway.pdb"
	-@erase "..\..\..\bin\debug\plugins\AdvancedAutoAway.dll"
	-@erase "..\..\..\bin\debug\plugins\AdvancedAutoAway.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ADVANCEDAUTOAWAY_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\AdvancedAutoAway.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AdvancedAutoAway.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\advancedautoaway.sbr" \
	"$(INTDIR)\commonstatus.sbr" \
	"$(INTDIR)\confirmdialog.sbr" \
	"$(INTDIR)\gen_helpers.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\msgoptions.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\trigger_aaa.sbr"

"$(OUTDIR)\AdvancedAutoAway.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\AdvancedAutoAway.pdb" /debug /machine:I386 /out:"../../../bin/debug/plugins/AdvancedAutoAway.dll" /implib:"$(OUTDIR)\AdvancedAutoAway.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\advancedautoaway.obj" \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\trigger_aaa.obj" \
	"$(INTDIR)\vc6.res"

"..\..\..\bin\debug\plugins\AdvancedAutoAway.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\..\bin\Debug Unicode\plugins\AdvancedAutoAway.dll" "$(OUTDIR)\AdvancedAutoAway.bsc"


CLEAN :
	-@erase "$(INTDIR)\advancedautoaway.obj"
	-@erase "$(INTDIR)\advancedautoaway.sbr"
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\commonstatus.sbr"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\confirmdialog.sbr"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\gen_helpers.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgoptions.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\trigger_aaa.obj"
	-@erase "$(INTDIR)\trigger_aaa.sbr"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\AdvancedAutoAway.bsc"
	-@erase "$(OUTDIR)\AdvancedAutoAway.exp"
	-@erase "$(OUTDIR)\AdvancedAutoAway.lib"
	-@erase "$(OUTDIR)\AdvancedAutoAway.pdb"
	-@erase "..\..\..\bin\Debug Unicode\plugins\AdvancedAutoAway.dll"
	-@erase "..\..\..\bin\Debug Unicode\plugins\AdvancedAutoAway.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\AdvancedAutoAway.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AdvancedAutoAway.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\advancedautoaway.sbr" \
	"$(INTDIR)\commonstatus.sbr" \
	"$(INTDIR)\confirmdialog.sbr" \
	"$(INTDIR)\gen_helpers.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\msgoptions.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\trigger_aaa.sbr"

"$(OUTDIR)\AdvancedAutoAway.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\AdvancedAutoAway.pdb" /debug /machine:I386 /out:"../../../bin/Debug Unicode/plugins/AdvancedAutoAway.dll" /implib:"$(OUTDIR)\AdvancedAutoAway.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\advancedautoaway.obj" \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\trigger_aaa.obj" \
	"$(INTDIR)\vc6.res"

"..\..\..\bin\Debug Unicode\plugins\AdvancedAutoAway.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\..\bin\Release Unicode\plugins\AdvancedAutoAway.dll"


CLEAN :
	-@erase "$(INTDIR)\advancedautoaway.obj"
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\trigger_aaa.obj"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\AdvancedAutoAway.exp"
	-@erase "$(OUTDIR)\AdvancedAutoAway.lib"
	-@erase "$(OUTDIR)\AdvancedAutoAway.pdb"
	-@erase "..\..\..\bin\Release Unicode\plugins\AdvancedAutoAway.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /Fp"$(INTDIR)\AdvancedAutoAway.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AdvancedAutoAway.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msvcrt.lib /nologo /base:"0x11bd0000" /dll /incremental:no /pdb:"$(OUTDIR)\AdvancedAutoAway.pdb" /debug /machine:I386 /nodefaultlib /out:"../../../bin/Release Unicode/plugins/AdvancedAutoAway.dll" /implib:"$(OUTDIR)\AdvancedAutoAway.lib" 
LINK32_OBJS= \
	"$(INTDIR)\advancedautoaway.obj" \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\trigger_aaa.obj" \
	"$(INTDIR)\vc6.res"

"..\..\..\bin\Release Unicode\plugins\AdvancedAutoAway.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("AdvancedAutoAway.dep")
!INCLUDE "AdvancedAutoAway.dep"
!ELSE 
!MESSAGE Warning: cannot find "AdvancedAutoAway.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "AdvancedAutoAway - Win32 Release" || "$(CFG)" == "AdvancedAutoAway - Win32 Debug" || "$(CFG)" == "AdvancedAutoAway - Win32 Debug Unicode" || "$(CFG)" == "AdvancedAutoAway - Win32 Release Unicode"
SOURCE=.\advancedautoaway.cpp

!IF  "$(CFG)" == "AdvancedAutoAway - Win32 Release"


"$(INTDIR)\advancedautoaway.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug"


"$(INTDIR)\advancedautoaway.obj"	"$(INTDIR)\advancedautoaway.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug Unicode"


"$(INTDIR)\advancedautoaway.obj"	"$(INTDIR)\advancedautoaway.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Release Unicode"


"$(INTDIR)\advancedautoaway.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=..\commonstatus.cpp

!IF  "$(CFG)" == "AdvancedAutoAway - Win32 Release"


"$(INTDIR)\commonstatus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug"


"$(INTDIR)\commonstatus.obj"	"$(INTDIR)\commonstatus.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug Unicode"


"$(INTDIR)\commonstatus.obj"	"$(INTDIR)\commonstatus.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Release Unicode"


"$(INTDIR)\commonstatus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\confirmdialog.cpp

!IF  "$(CFG)" == "AdvancedAutoAway - Win32 Release"


"$(INTDIR)\confirmdialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug"


"$(INTDIR)\confirmdialog.obj"	"$(INTDIR)\confirmdialog.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug Unicode"


"$(INTDIR)\confirmdialog.obj"	"$(INTDIR)\confirmdialog.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Release Unicode"


"$(INTDIR)\confirmdialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=..\..\helpers\gen_helpers.cpp

!IF  "$(CFG)" == "AdvancedAutoAway - Win32 Release"


"$(INTDIR)\gen_helpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug"


"$(INTDIR)\gen_helpers.obj"	"$(INTDIR)\gen_helpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug Unicode"


"$(INTDIR)\gen_helpers.obj"	"$(INTDIR)\gen_helpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Release Unicode"


"$(INTDIR)\gen_helpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\main.cpp

!IF  "$(CFG)" == "AdvancedAutoAway - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug Unicode"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Release Unicode"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\msgoptions.cpp

!IF  "$(CFG)" == "AdvancedAutoAway - Win32 Release"


"$(INTDIR)\msgoptions.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug"


"$(INTDIR)\msgoptions.obj"	"$(INTDIR)\msgoptions.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug Unicode"


"$(INTDIR)\msgoptions.obj"	"$(INTDIR)\msgoptions.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Release Unicode"


"$(INTDIR)\msgoptions.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\options.cpp

!IF  "$(CFG)" == "AdvancedAutoAway - Win32 Release"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug Unicode"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Release Unicode"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\trigger_aaa.cpp

!IF  "$(CFG)" == "AdvancedAutoAway - Win32 Release"


"$(INTDIR)\trigger_aaa.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug"


"$(INTDIR)\trigger_aaa.obj"	"$(INTDIR)\trigger_aaa.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Debug Unicode"


"$(INTDIR)\trigger_aaa.obj"	"$(INTDIR)\trigger_aaa.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "AdvancedAutoAway - Win32 Release Unicode"


"$(INTDIR)\trigger_aaa.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\vc6.rc

"$(INTDIR)\vc6.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

