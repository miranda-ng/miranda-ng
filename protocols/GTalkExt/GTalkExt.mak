# Microsoft Developer Studio Generated NMAKE File, Based on GTalkExt.dsp
!IF "$(CFG)" == ""
CFG=GTalkExt - Win32 Release Unicode
!MESSAGE No configuration specified. Defaulting to GTalkExt - Win32 Release Unicode.
!ENDIF

!IF "$(CFG)" != "GTalkExt - Win32 Release" && "$(CFG)" != "GTalkExt - Win32 Debug" && "$(CFG)" != "GTalkExt - Win32 Release Unicode" && "$(CFG)" != "GTalkExt - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "GTalkExt.mak" CFG="GTalkExt - Win32 Debug Unicode"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "GTalkExt - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GTalkExt - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GTalkExt - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GTalkExt - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "GTalkExt - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\release\plugins\GTalkExt.dll"


CLEAN :
	-@erase "$(INTDIR)\avatar.obj"
	-@erase "$(INTDIR)\db.obj"
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\GTalkExt.obj"
	-@erase "$(INTDIR)\GTalkExt.pch"
	-@erase "$(INTDIR)\handlers.obj"
	-@erase "$(INTDIR)\inbox.obj"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\notifications.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\popups.obj"
	-@erase "$(INTDIR)\settings.res"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\tipper_items.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\GTalkExt.exp"
	-@erase "$(OUTDIR)\GTalkExt.lib"
	-@erase "$(OUTDIR)\GTalkExt.map"
	-@erase "$(OUTDIR)\GTalkExt.pdb"
	-@erase "..\..\bin\release\plugins\GTalkExt.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GTalkExt_EXPORTS" /Fp"$(INTDIR)\GTalkExt.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\settings.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GTalkExt.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x5130000" /dll /incremental:no /pdb:"$(OUTDIR)\GTalkExt.pdb" /map:"$(INTDIR)\GTalkExt.map" /debug /machine:I386 /out:"../../bin/release/plugins/GTalkExt.dll" /implib:"$(OUTDIR)\GTalkExt.lib" /IGNORE:4089
LINK32_OBJS= \
	"$(INTDIR)\tipper_items.obj" \
	"$(INTDIR)\avatar.obj" \
	"$(INTDIR)\db.obj" \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\GTalkExt.obj" \
	"$(INTDIR)\handlers.obj" \
	"$(INTDIR)\inbox.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\notifications.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popups.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\settings.res"

"..\..\bin\release\plugins\GTalkExt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\bin\debug\plugins\GTalkExt.dll" "$(OUTDIR)\GTalkExt.bsc"


CLEAN :
	-@erase "$(INTDIR)\avatar.obj"
	-@erase "$(INTDIR)\avatar.sbr"
	-@erase "$(INTDIR)\db.obj"
	-@erase "$(INTDIR)\db.sbr"
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\dllmain.sbr"
	-@erase "$(INTDIR)\GTalkExt.obj"
	-@erase "$(INTDIR)\GTalkExt.pch"
	-@erase "$(INTDIR)\GTalkExt.sbr"
	-@erase "$(INTDIR)\handlers.obj"
	-@erase "$(INTDIR)\handlers.sbr"
	-@erase "$(INTDIR)\inbox.obj"
	-@erase "$(INTDIR)\inbox.sbr"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\menu.sbr"
	-@erase "$(INTDIR)\notifications.obj"
	-@erase "$(INTDIR)\notifications.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\popups.obj"
	-@erase "$(INTDIR)\popups.sbr"
	-@erase "$(INTDIR)\settings.res"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\stdafx.sbr"
	-@erase "$(INTDIR)\tipper_items.obj"
	-@erase "$(INTDIR)\tipper_items.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\GTalkExt.exp"
	-@erase "$(OUTDIR)\GTalkExt.lib"
	-@erase "$(OUTDIR)\GTalkExt.map"
	-@erase "$(OUTDIR)\GTalkExt.pdb"
	-@erase "$(OUTDIR)\GTalkExt.bsc"
	-@erase "..\..\bin\debug\plugins\GTalkExt.dll"
	-@erase "..\..\bin\debug\plugins\GTalkExt.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GTalkExt_EXPORTS" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\GTalkExt.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\settings.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GTalkExt.bsc"
BSC32_SBRS= \
	"$(INTDIR)\tipper_items.sbr" \
	"$(INTDIR)\avatar.sbr" \
	"$(INTDIR)\db.sbr" \
	"$(INTDIR)\dllmain.sbr" \
	"$(INTDIR)\GTalkExt.sbr" \
	"$(INTDIR)\handlers.sbr" \
	"$(INTDIR)\inbox.sbr" \
	"$(INTDIR)\menu.sbr" \
	"$(INTDIR)\notifications.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\popups.sbr" \
	"$(INTDIR)\stdafx.sbr"

"$(OUTDIR)\GTalkExt.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\GTalkExt.pdb" /map:"$(INTDIR)\GTalkExt.map" /debug /machine:I386 /out:"../../bin/debug/plugins/GTalkExt.dll" /implib:"$(OUTDIR)\GTalkExt.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\tipper_items.obj" \
	"$(INTDIR)\avatar.obj" \
	"$(INTDIR)\db.obj" \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\GTalkExt.obj" \
	"$(INTDIR)\handlers.obj" \
	"$(INTDIR)\inbox.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\notifications.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popups.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\settings.res"

"..\..\bin\debug\plugins\GTalkExt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\release unicode\plugins\GTalkExt.dll"


CLEAN :
	-@erase "$(INTDIR)\avatar.obj"
	-@erase "$(INTDIR)\db.obj"
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\GTalkExt.obj"
	-@erase "$(INTDIR)\GTalkExt.pch"
	-@erase "$(INTDIR)\handlers.obj"
	-@erase "$(INTDIR)\inbox.obj"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\notifications.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\popups.obj"
	-@erase "$(INTDIR)\settings.res"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\tipper_items.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\GTalkExt.exp"
	-@erase "$(OUTDIR)\GTalkExt.lib"
	-@erase "$(OUTDIR)\GTalkExt.map"
	-@erase "$(OUTDIR)\GTalkExt.pdb"
	-@erase "..\..\bin\release unicode\plugins\GTalkExt.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "GTalkExt_EXPORTS" /Fp"$(INTDIR)\GTalkExt.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\settings.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GTalkExt.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x5130000" /dll /incremental:no /pdb:"$(OUTDIR)\GTalkExt.pdb" /map:"$(INTDIR)\GTalkExt.map" /debug /machine:I386 /out:"../../bin/release unicode/plugins/GTalkExt.dll" /implib:"$(OUTDIR)\GTalkExt.lib" /IGNORE:4089
LINK32_OBJS= \
	"$(INTDIR)\tipper_items.obj" \
	"$(INTDIR)\avatar.obj" \
	"$(INTDIR)\db.obj" \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\GTalkExt.obj" \
	"$(INTDIR)\handlers.obj" \
	"$(INTDIR)\inbox.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\notifications.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popups.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\settings.res"

"..\..\bin\release unicode\plugins\GTalkExt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\bin\debug Unicode\plugins\GTalkExt.dll" "$(OUTDIR)\GTalkExt.bsc"


CLEAN :
	-@erase "$(INTDIR)\avatar.obj"
	-@erase "$(INTDIR)\avatar.sbr"
	-@erase "$(INTDIR)\db.obj"
	-@erase "$(INTDIR)\db.sbr"
	-@erase "$(INTDIR)\dllmain.obj"
	-@erase "$(INTDIR)\dllmain.sbr"
	-@erase "$(INTDIR)\GTalkExt.obj"
	-@erase "$(INTDIR)\GTalkExt.pch"
	-@erase "$(INTDIR)\GTalkExt.sbr"
	-@erase "$(INTDIR)\handlers.obj"
	-@erase "$(INTDIR)\handlers.sbr"
	-@erase "$(INTDIR)\inbox.obj"
	-@erase "$(INTDIR)\inbox.sbr"
	-@erase "$(INTDIR)\menu.obj"
	-@erase "$(INTDIR)\menu.sbr"
	-@erase "$(INTDIR)\notifications.obj"
	-@erase "$(INTDIR)\notifications.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\popups.obj"
	-@erase "$(INTDIR)\popups.sbr"
	-@erase "$(INTDIR)\settings.res"
	-@erase "$(INTDIR)\stdafx.obj"
	-@erase "$(INTDIR)\stdafx.sbr"
	-@erase "$(INTDIR)\tipper_items.obj"
	-@erase "$(INTDIR)\tipper_items.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\GTalkExt.exp"
	-@erase "$(OUTDIR)\GTalkExt.lib"
	-@erase "$(OUTDIR)\GTalkExt.map"
	-@erase "$(OUTDIR)\GTalkExt.pdb"
	-@erase "$(OUTDIR)\GTalkExt.bsc"
	-@erase "..\..\bin\debug Unicode\plugins\GTalkExt.dll"
	-@erase "..\..\bin\debug Unicode\plugins\GTalkExt.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "GTalkExt_EXPORTS" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\GTalkExt.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\settings.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GTalkExt.bsc"
BSC32_SBRS= \
	"$(INTDIR)\tipper_items.sbr" \
	"$(INTDIR)\avatar.sbr" \
	"$(INTDIR)\db.sbr" \
	"$(INTDIR)\dllmain.sbr" \
	"$(INTDIR)\GTalkExt.sbr" \
	"$(INTDIR)\handlers.sbr" \
	"$(INTDIR)\inbox.sbr" \
	"$(INTDIR)\menu.sbr" \
	"$(INTDIR)\notifications.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\popups.sbr" \
	"$(INTDIR)\stdafx.sbr"

"$(OUTDIR)\GTalkExt.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\GTalkExt.pdb" /map:"$(INTDIR)\GTalkExt.map" /debug /machine:I386 /out:"../../bin/debug Unicode/plugins/GTalkExt.dll" /implib:"$(OUTDIR)\GTalkExt.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\tipper_items.obj" \
	"$(INTDIR)\avatar.obj" \
	"$(INTDIR)\db.obj" \
	"$(INTDIR)\dllmain.obj" \
	"$(INTDIR)\GTalkExt.obj" \
	"$(INTDIR)\handlers.obj" \
	"$(INTDIR)\inbox.obj" \
	"$(INTDIR)\menu.obj" \
	"$(INTDIR)\notifications.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popups.obj" \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\settings.res"

"..\..\bin\debug Unicode\plugins\GTalkExt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("GTalkExt.dep")
!INCLUDE "GTalkExt.dep"
!ELSE
!MESSAGE Warning: cannot find "GTalkExt.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "GTalkExt - Win32 Release" || "$(CFG)" == "GTalkExt - Win32 Debug" || "$(CFG)" == "GTalkExt - Win32 Release Unicode" || "$(CFG)" == "GTalkExt - Win32 Debug Unicode"
SOURCE=.\avatar.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\avatar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\avatar.obj"	"$(INTDIR)\avatar.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\avatar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\avatar.obj"	"$(INTDIR)\avatar.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\db.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\db.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\db.obj"	"$(INTDIR)\db.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\db.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\db.obj"	"$(INTDIR)\db.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\dllmain.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\dllmain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\dllmain.obj"	"$(INTDIR)\dllmain.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\dllmain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\dllmain.obj"	"$(INTDIR)\dllmain.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\GTalkExt.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\GTalkExt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\GTalkExt.obj"	"$(INTDIR)\GTalkExt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\GTalkExt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\GTalkExt.obj"	"$(INTDIR)\GTalkExt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\handlers.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\handlers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\handlers.obj"	"$(INTDIR)\handlers.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\handlers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\handlers.obj"	"$(INTDIR)\handlers.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\inbox.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\inbox.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\inbox.obj"	"$(INTDIR)\inbox.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\inbox.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\inbox.obj"	"$(INTDIR)\inbox.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\menu.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\menu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\menu.obj"	"$(INTDIR)\menu.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\menu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\menu.obj"	"$(INTDIR)\menu.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\notifications.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\notifications.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\notifications.obj"	"$(INTDIR)\notifications.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\notifications.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\notifications.obj"	"$(INTDIR)\notifications.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\options.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\popups.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\popups.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\popups.obj"	"$(INTDIR)\popups.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\popups.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\popups.obj"	"$(INTDIR)\popups.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\stdafx.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GTalkExt_EXPORTS" /Fp"$(INTDIR)\GTalkExt.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\stdafx.obj"	"$(INTDIR)\GTalkExt.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GTalkExt_EXPORTS" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\GTalkExt.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\stdafx.obj"	"$(INTDIR)\stdafx.sbr"	"$(INTDIR)\GTalkExt.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "GTalkExt_EXPORTS" /Fp"$(INTDIR)\GTalkExt.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\stdafx.obj"	"$(INTDIR)\GTalkExt.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "GTalkExt_EXPORTS" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\GTalkExt.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\stdafx.obj"	"$(INTDIR)\stdafx.sbr"	"$(INTDIR)\GTalkExt.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\tipper_items.cpp

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\tipper_items.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\tipper_items.obj"	"$(INTDIR)\tipper_items.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\tipper_items.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\tipper_items.obj"	"$(INTDIR)\tipper_items.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GTalkExt.pch"


!ENDIF

SOURCE=.\res\settings.rc

!IF  "$(CFG)" == "GTalkExt - Win32 Release"


"$(INTDIR)\settings.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x809 /fo"$(INTDIR)\settings.res" /i "res" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug"


"$(INTDIR)\settings.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x809 /fo"$(INTDIR)\settings.res" /i "res" /d "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Release Unicode"


"$(INTDIR)\settings.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x809 /fo"$(INTDIR)\settings.res" /i "res" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "GTalkExt - Win32 Debug Unicode"


"$(INTDIR)\settings.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x809 /fo"$(INTDIR)\settings.res" /i "res" /d "_DEBUG" $(SOURCE)


!ENDIF


!ENDIF
