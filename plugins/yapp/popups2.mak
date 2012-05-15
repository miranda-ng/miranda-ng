# Microsoft Developer Studio Generated NMAKE File, Based on popups2.dsp
!IF "$(CFG)" == ""
CFG=yapp - Win32 Release Unicode
!MESSAGE No configuration specified. Defaulting to yapp - Win32 Release Unicode.
!ENDIF

!IF "$(CFG)" != "yapp - Win32 Release" && "$(CFG)" != "yapp - Win32 Debug" && "$(CFG)" != "yapp - Win32 Release Unicode" && "$(CFG)" != "yapp - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "popups2.mak" CFG="yapp - Win32 Debug Unicode"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "yapp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "yapp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "yapp - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "yapp - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "yapp - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\Release\Plugins\yapp.dll"


CLEAN :
	-@erase "$(INTDIR)\message_pump.obj"
	-@erase "$(INTDIR)\notify_imp.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\popup_history.obj"
	-@erase "$(INTDIR)\popup_history_dlg.obj"
	-@erase "$(INTDIR)\popups2.obj"
	-@erase "$(INTDIR)\popups2.pch"
	-@erase "$(INTDIR)\popwin.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\str_utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\yapp.exp"
	-@erase "$(OUTDIR)\yapp.lib"
	-@erase "$(OUTDIR)\yapp.map"
	-@erase "$(OUTDIR)\yapp.pdb"
	-@erase "..\..\bin\Release\Plugins\yapp.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUPS2_EXPORTS" /Fp"$(INTDIR)\popups2.pch" /Yu"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\resource.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\popups2.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\yapp.pdb" /map:"$(INTDIR)\yapp.map" /debug /machine:I386 /out:"../../bin/Release/Plugins/yapp.dll" /implib:"$(OUTDIR)\yapp.lib" /filealign:512
LINK32_OBJS= \
	"$(INTDIR)\message_pump.obj" \
	"$(INTDIR)\notify_imp.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popup_history.obj" \
	"$(INTDIR)\popup_history_dlg.obj" \
	"$(INTDIR)\popups2.obj" \
	"$(INTDIR)\popwin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\str_utils.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Release\Plugins\yapp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "yapp - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\bin\Debug\plugins\yapp.dll" "$(OUTDIR)\popups2.bsc"


CLEAN :
	-@erase "$(INTDIR)\message_pump.obj"
	-@erase "$(INTDIR)\message_pump.sbr"
	-@erase "$(INTDIR)\notify_imp.obj"
	-@erase "$(INTDIR)\notify_imp.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\popup_history.obj"
	-@erase "$(INTDIR)\popup_history.sbr"
	-@erase "$(INTDIR)\popup_history_dlg.obj"
	-@erase "$(INTDIR)\popup_history_dlg.sbr"
	-@erase "$(INTDIR)\popups2.obj"
	-@erase "$(INTDIR)\popups2.pch"
	-@erase "$(INTDIR)\popups2.sbr"
	-@erase "$(INTDIR)\popwin.obj"
	-@erase "$(INTDIR)\popwin.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\str_utils.obj"
	-@erase "$(INTDIR)\str_utils.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\popups2.bsc"
	-@erase "$(OUTDIR)\yapp.exp"
	-@erase "$(OUTDIR)\yapp.lib"
	-@erase "$(OUTDIR)\yapp.pdb"
	-@erase "..\..\bin\Debug\plugins\yapp.dll"
	-@erase "..\..\bin\Debug\plugins\yapp.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUPS2_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\popups2.pch" /Yu"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\resource.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\popups2.bsc"
BSC32_SBRS= \
	"$(INTDIR)\message_pump.sbr" \
	"$(INTDIR)\notify_imp.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\popup_history.sbr" \
	"$(INTDIR)\popup_history_dlg.sbr" \
	"$(INTDIR)\popups2.sbr" \
	"$(INTDIR)\popwin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\str_utils.sbr"

"$(OUTDIR)\popups2.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\yapp.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/yapp.dll" /implib:"$(OUTDIR)\yapp.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\message_pump.obj" \
	"$(INTDIR)\notify_imp.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popup_history.obj" \
	"$(INTDIR)\popup_history_dlg.obj" \
	"$(INTDIR)\popups2.obj" \
	"$(INTDIR)\popwin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\str_utils.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Debug\plugins\yapp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "yapp - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\Release Unicode\Plugins\yapp.dll"


CLEAN :
	-@erase "$(INTDIR)\message_pump.obj"
	-@erase "$(INTDIR)\notify_imp.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\popup_history.obj"
	-@erase "$(INTDIR)\popup_history_dlg.obj"
	-@erase "$(INTDIR)\popups2.obj"
	-@erase "$(INTDIR)\popups2.pch"
	-@erase "$(INTDIR)\popwin.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\str_utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\yapp.exp"
	-@erase "$(OUTDIR)\yapp.lib"
	-@erase "$(OUTDIR)\yapp.map"
	-@erase "$(OUTDIR)\yapp.pdb"
	-@erase "..\..\bin\Release Unicode\Plugins\yapp.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "POPUPS2_EXPORTS" /Fp"$(INTDIR)\popups2.pch" /Yu"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\resource.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\popups2.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\yapp.pdb" /map:"$(INTDIR)\yapp.map" /debug /machine:I386 /out:"../../bin/Release Unicode/Plugins/yapp.dll" /implib:"$(OUTDIR)\yapp.lib" /filealign:512
LINK32_OBJS= \
	"$(INTDIR)\message_pump.obj" \
	"$(INTDIR)\notify_imp.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popup_history.obj" \
	"$(INTDIR)\popup_history_dlg.obj" \
	"$(INTDIR)\popups2.obj" \
	"$(INTDIR)\popwin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\str_utils.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Release Unicode\Plugins\yapp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "yapp - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\bin\Debug Unicode\plugins\yapp.dll" "$(OUTDIR)\popups2.bsc"


CLEAN :
	-@erase "$(INTDIR)\message_pump.obj"
	-@erase "$(INTDIR)\message_pump.sbr"
	-@erase "$(INTDIR)\notify_imp.obj"
	-@erase "$(INTDIR)\notify_imp.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\popup_history.obj"
	-@erase "$(INTDIR)\popup_history.sbr"
	-@erase "$(INTDIR)\popup_history_dlg.obj"
	-@erase "$(INTDIR)\popup_history_dlg.sbr"
	-@erase "$(INTDIR)\popups2.obj"
	-@erase "$(INTDIR)\popups2.pch"
	-@erase "$(INTDIR)\popups2.sbr"
	-@erase "$(INTDIR)\popwin.obj"
	-@erase "$(INTDIR)\popwin.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\str_utils.obj"
	-@erase "$(INTDIR)\str_utils.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\popups2.bsc"
	-@erase "$(OUTDIR)\yapp.exp"
	-@erase "$(OUTDIR)\yapp.lib"
	-@erase "$(OUTDIR)\yapp.pdb"
	-@erase "..\..\bin\Debug Unicode\plugins\yapp.dll"
	-@erase "..\..\bin\Debug Unicode\plugins\yapp.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "POPUPS2_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\popups2.pch" /Yu"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\resource.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\popups2.bsc"
BSC32_SBRS= \
	"$(INTDIR)\message_pump.sbr" \
	"$(INTDIR)\notify_imp.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\popup_history.sbr" \
	"$(INTDIR)\popup_history_dlg.sbr" \
	"$(INTDIR)\popups2.sbr" \
	"$(INTDIR)\popwin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\str_utils.sbr"

"$(OUTDIR)\popups2.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\yapp.pdb" /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/yapp.dll" /implib:"$(OUTDIR)\yapp.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\message_pump.obj" \
	"$(INTDIR)\notify_imp.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popup_history.obj" \
	"$(INTDIR)\popup_history_dlg.obj" \
	"$(INTDIR)\popups2.obj" \
	"$(INTDIR)\popwin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\str_utils.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Debug Unicode\plugins\yapp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("popups2.dep")
!INCLUDE "popups2.dep"
!ELSE
!MESSAGE Warning: cannot find "popups2.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "yapp - Win32 Release" || "$(CFG)" == "yapp - Win32 Debug" || "$(CFG)" == "yapp - Win32 Release Unicode" || "$(CFG)" == "yapp - Win32 Debug Unicode"
SOURCE=.\message_pump.cpp

!IF  "$(CFG)" == "yapp - Win32 Release"


"$(INTDIR)\message_pump.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug"


"$(INTDIR)\message_pump.obj"	"$(INTDIR)\message_pump.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Release Unicode"


"$(INTDIR)\message_pump.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug Unicode"


"$(INTDIR)\message_pump.obj"	"$(INTDIR)\message_pump.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ENDIF

SOURCE=.\notify_imp.cpp

!IF  "$(CFG)" == "yapp - Win32 Release"


"$(INTDIR)\notify_imp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug"


"$(INTDIR)\notify_imp.obj"	"$(INTDIR)\notify_imp.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Release Unicode"


"$(INTDIR)\notify_imp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug Unicode"


"$(INTDIR)\notify_imp.obj"	"$(INTDIR)\notify_imp.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ENDIF

SOURCE=.\options.cpp

!IF  "$(CFG)" == "yapp - Win32 Release"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Release Unicode"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug Unicode"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ENDIF

SOURCE=.\popup_history.cpp

!IF  "$(CFG)" == "yapp - Win32 Release"


"$(INTDIR)\popup_history.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug"


"$(INTDIR)\popup_history.obj"	"$(INTDIR)\popup_history.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Release Unicode"


"$(INTDIR)\popup_history.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug Unicode"


"$(INTDIR)\popup_history.obj"	"$(INTDIR)\popup_history.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ENDIF

SOURCE=.\popup_history_dlg.cpp

!IF  "$(CFG)" == "yapp - Win32 Release"


"$(INTDIR)\popup_history_dlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug"


"$(INTDIR)\popup_history_dlg.obj"	"$(INTDIR)\popup_history_dlg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Release Unicode"


"$(INTDIR)\popup_history_dlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug Unicode"


"$(INTDIR)\popup_history_dlg.obj"	"$(INTDIR)\popup_history_dlg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ENDIF

SOURCE=.\popups2.cpp

!IF  "$(CFG)" == "yapp - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUPS2_EXPORTS" /Fp"$(INTDIR)\popups2.pch" /Yc"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\popups2.obj"	"$(INTDIR)\popups2.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUPS2_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\popups2.pch" /Yc"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\popups2.obj"	"$(INTDIR)\popups2.sbr"	"$(INTDIR)\popups2.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "yapp - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "POPUPS2_EXPORTS" /Fp"$(INTDIR)\popups2.pch" /Yc"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\popups2.obj"	"$(INTDIR)\popups2.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "POPUPS2_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\popups2.pch" /Yc"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\popups2.obj"	"$(INTDIR)\popups2.sbr"	"$(INTDIR)\popups2.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\popwin.cpp

!IF  "$(CFG)" == "yapp - Win32 Release"


"$(INTDIR)\popwin.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug"


"$(INTDIR)\popwin.obj"	"$(INTDIR)\popwin.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Release Unicode"


"$(INTDIR)\popwin.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug Unicode"


"$(INTDIR)\popwin.obj"	"$(INTDIR)\popwin.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ENDIF

SOURCE=.\services.cpp

!IF  "$(CFG)" == "yapp - Win32 Release"


"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug"


"$(INTDIR)\services.obj"	"$(INTDIR)\services.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Release Unicode"


"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug Unicode"


"$(INTDIR)\services.obj"	"$(INTDIR)\services.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ENDIF

SOURCE=.\str_utils.cpp

!IF  "$(CFG)" == "yapp - Win32 Release"


"$(INTDIR)\str_utils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug"


"$(INTDIR)\str_utils.obj"	"$(INTDIR)\str_utils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Release Unicode"


"$(INTDIR)\str_utils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ELSEIF  "$(CFG)" == "yapp - Win32 Debug Unicode"


"$(INTDIR)\str_utils.obj"	"$(INTDIR)\str_utils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\popups2.pch"


!ENDIF

SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF
