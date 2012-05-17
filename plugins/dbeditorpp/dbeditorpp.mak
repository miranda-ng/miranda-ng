# Microsoft Developer Studio Generated NMAKE File, Based on dbeditorpp.dsp
!IF "$(CFG)" == ""
CFG=dbeditorpp - Win32 Release Unicode
!MESSAGE No configuration specified. Defaulting to dbeditorpp - Win32 Release Unicode.
!ENDIF

!IF "$(CFG)" != "dbeditorpp - Win32 Release" && "$(CFG)" != "dbeditorpp - Win32 Debug" && "$(CFG)" != "dbeditorpp - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "dbeditorpp.mak" CFG="dbeditorpp - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "dbeditorpp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dbeditorpp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dbeditorpp - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "..\..\bin\release\plugins\svc_dbepp.dll" "$(OUTDIR)\dbeditorpp.bsc"


CLEAN :
	-@erase "$(INTDIR)\addeditsettingsdlg.obj"
	-@erase "$(INTDIR)\addeditsettingsdlg.sbr"
	-@erase "$(INTDIR)\copymodule.obj"
	-@erase "$(INTDIR)\copymodule.sbr"
	-@erase "$(INTDIR)\dbeditorpp.pch"
	-@erase "$(INTDIR)\deletemodule.obj"
	-@erase "$(INTDIR)\deletemodule.sbr"
	-@erase "$(INTDIR)\exportimport.obj"
	-@erase "$(INTDIR)\exportimport.sbr"
	-@erase "$(INTDIR)\findwindow.obj"
	-@erase "$(INTDIR)\findwindow.sbr"
	-@erase "$(INTDIR)\icons.obj"
	-@erase "$(INTDIR)\icons.sbr"
	-@erase "$(INTDIR)\knownmodules.obj"
	-@erase "$(INTDIR)\knownmodules.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\main_window.obj"
	-@erase "$(INTDIR)\main_window.sbr"
	-@erase "$(INTDIR)\modsettingenum.obj"
	-@erase "$(INTDIR)\modsettingenum.sbr"
	-@erase "$(INTDIR)\modules.obj"
	-@erase "$(INTDIR)\modules.sbr"
	-@erase "$(INTDIR)\moduletree.obj"
	-@erase "$(INTDIR)\moduletree.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\settinglist.obj"
	-@erase "$(INTDIR)\settinglist.sbr"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\threads.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\watchedvars.obj"
	-@erase "$(INTDIR)\watchedvars.sbr"
	-@erase "$(OUTDIR)\dbeditorpp.bsc"
	-@erase "$(OUTDIR)\svc_dbepp.exp"
	-@erase "$(OUTDIR)\svc_dbepp.lib"
	-@erase "..\..\bin\release\plugins\svc_dbepp.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX- /O1 /I "../../include" /I "../ExternalAPI" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\dbeditorpp.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbeditorpp.bsc"
BSC32_SBRS= \
	"$(INTDIR)\modsettingenum.sbr" \
	"$(INTDIR)\addeditsettingsdlg.sbr" \
	"$(INTDIR)\copymodule.sbr" \
	"$(INTDIR)\deletemodule.sbr" \
	"$(INTDIR)\findwindow.sbr" \
	"$(INTDIR)\main_window.sbr" \
	"$(INTDIR)\moduletree.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\settinglist.sbr" \
	"$(INTDIR)\watchedvars.sbr" \
	"$(INTDIR)\exportimport.sbr" \
	"$(INTDIR)\icons.sbr" \
	"$(INTDIR)\knownmodules.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\modules.sbr" \
	"$(INTDIR)\threads.sbr"

"$(OUTDIR)\dbeditorpp.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=gdi32.lib winspool.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib comdlg32.lib advapi32.lib shell32.lib shlwapi.lib comctl32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\svc_dbepp.pdb" /machine:I386 /out:"../../bin/release/plugins/svc_dbepp.dll" /implib:"$(OUTDIR)\svc_dbepp.lib"
LINK32_OBJS= \
	"$(INTDIR)\modsettingenum.obj" \
	"$(INTDIR)\addeditsettingsdlg.obj" \
	"$(INTDIR)\copymodule.obj" \
	"$(INTDIR)\deletemodule.obj" \
	"$(INTDIR)\findwindow.obj" \
	"$(INTDIR)\main_window.obj" \
	"$(INTDIR)\moduletree.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\settinglist.obj" \
	"$(INTDIR)\watchedvars.obj" \
	"$(INTDIR)\exportimport.obj" \
	"$(INTDIR)\icons.obj" \
	"$(INTDIR)\knownmodules.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\modules.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\plugins\svc_dbepp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\bin\debug\plugins\svc_dbepp.dll"


CLEAN :
	-@erase "$(INTDIR)\addeditsettingsdlg.obj"
	-@erase "$(INTDIR)\copymodule.obj"
	-@erase "$(INTDIR)\dbeditorpp.pch"
	-@erase "$(INTDIR)\deletemodule.obj"
	-@erase "$(INTDIR)\exportimport.obj"
	-@erase "$(INTDIR)\findwindow.obj"
	-@erase "$(INTDIR)\icons.obj"
	-@erase "$(INTDIR)\knownmodules.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main_window.obj"
	-@erase "$(INTDIR)\modsettingenum.obj"
	-@erase "$(INTDIR)\modules.obj"
	-@erase "$(INTDIR)\moduletree.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\settinglist.obj"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\watchedvars.obj"
	-@erase "$(OUTDIR)\svc_dbepp.exp"
	-@erase "$(OUTDIR)\svc_dbepp.lib"
	-@erase "$(OUTDIR)\svc_dbepp.pdb"
	-@erase "..\..\bin\debug\plugins\svc_dbepp.dll"
	-@erase "..\..\bin\debug\plugins\svc_dbepp.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX- /ZI /Od /I "../../include" /I "../ExternalAPI" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /Fp"$(INTDIR)\dbeditorpp.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbeditorpp.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=gdi32.lib winspool.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib comdlg32.lib advapi32.lib shell32.lib shlwapi.lib comctl32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\svc_dbepp.pdb" /debug /machine:I386 /out:"../../bin/debug/plugins/svc_dbepp.dll" /implib:"$(OUTDIR)\svc_dbepp.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\modsettingenum.obj" \
	"$(INTDIR)\addeditsettingsdlg.obj" \
	"$(INTDIR)\copymodule.obj" \
	"$(INTDIR)\deletemodule.obj" \
	"$(INTDIR)\findwindow.obj" \
	"$(INTDIR)\main_window.obj" \
	"$(INTDIR)\moduletree.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\settinglist.obj" \
	"$(INTDIR)\watchedvars.obj" \
	"$(INTDIR)\exportimport.obj" \
	"$(INTDIR)\icons.obj" \
	"$(INTDIR)\knownmodules.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\modules.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug\plugins\svc_dbepp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode
# Begin Custom Macros
OutDir=.\Release_Unicode
# End Custom Macros

ALL : "..\..\bin\release unicode\plugins\svc_dbepp.dll" "$(OUTDIR)\dbeditorpp.bsc"


CLEAN :
	-@erase "$(INTDIR)\addeditsettingsdlg.obj"
	-@erase "$(INTDIR)\addeditsettingsdlg.sbr"
	-@erase "$(INTDIR)\copymodule.obj"
	-@erase "$(INTDIR)\copymodule.sbr"
	-@erase "$(INTDIR)\dbeditorpp.pch"
	-@erase "$(INTDIR)\deletemodule.obj"
	-@erase "$(INTDIR)\deletemodule.sbr"
	-@erase "$(INTDIR)\exportimport.obj"
	-@erase "$(INTDIR)\exportimport.sbr"
	-@erase "$(INTDIR)\findwindow.obj"
	-@erase "$(INTDIR)\findwindow.sbr"
	-@erase "$(INTDIR)\icons.obj"
	-@erase "$(INTDIR)\icons.sbr"
	-@erase "$(INTDIR)\knownmodules.obj"
	-@erase "$(INTDIR)\knownmodules.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\main_window.obj"
	-@erase "$(INTDIR)\main_window.sbr"
	-@erase "$(INTDIR)\modsettingenum.obj"
	-@erase "$(INTDIR)\modsettingenum.sbr"
	-@erase "$(INTDIR)\modules.obj"
	-@erase "$(INTDIR)\modules.sbr"
	-@erase "$(INTDIR)\moduletree.obj"
	-@erase "$(INTDIR)\moduletree.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\settinglist.obj"
	-@erase "$(INTDIR)\settinglist.sbr"
	-@erase "$(INTDIR)\threads.obj"
	-@erase "$(INTDIR)\threads.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\watchedvars.obj"
	-@erase "$(INTDIR)\watchedvars.sbr"
	-@erase "$(OUTDIR)\dbeditorpp.bsc"
	-@erase "$(OUTDIR)\svc_dbepp.exp"
	-@erase "$(OUTDIR)\svc_dbepp.lib"
	-@erase "$(OUTDIR)\svc_dbepp.pdb"
	-@erase "..\..\bin\release unicode\plugins\svc_dbepp.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX- /Ox /Ot /Og /Os /I "../../include" /I "../ExternalAPI" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\dbeditorpp.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbeditorpp.bsc"
BSC32_SBRS= \
	"$(INTDIR)\modsettingenum.sbr" \
	"$(INTDIR)\addeditsettingsdlg.sbr" \
	"$(INTDIR)\copymodule.sbr" \
	"$(INTDIR)\deletemodule.sbr" \
	"$(INTDIR)\findwindow.sbr" \
	"$(INTDIR)\main_window.sbr" \
	"$(INTDIR)\moduletree.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\settinglist.sbr" \
	"$(INTDIR)\watchedvars.sbr" \
	"$(INTDIR)\exportimport.sbr" \
	"$(INTDIR)\icons.sbr" \
	"$(INTDIR)\knownmodules.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\modules.sbr" \
	"$(INTDIR)\threads.sbr"

"$(OUTDIR)\dbeditorpp.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib comdlg32.lib advapi32.lib shell32.lib shlwapi.lib comctl32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\svc_dbepp.pdb" /debug /machine:I386 /out:"../../bin/release unicode/plugins/svc_dbepp.dll" /implib:"$(OUTDIR)\svc_dbepp.lib"
LINK32_OBJS= \
	"$(INTDIR)\modsettingenum.obj" \
	"$(INTDIR)\addeditsettingsdlg.obj" \
	"$(INTDIR)\copymodule.obj" \
	"$(INTDIR)\deletemodule.obj" \
	"$(INTDIR)\findwindow.obj" \
	"$(INTDIR)\main_window.obj" \
	"$(INTDIR)\moduletree.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\settinglist.obj" \
	"$(INTDIR)\watchedvars.obj" \
	"$(INTDIR)\exportimport.obj" \
	"$(INTDIR)\icons.obj" \
	"$(INTDIR)\knownmodules.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\modules.obj" \
	"$(INTDIR)\threads.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release unicode\plugins\svc_dbepp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("dbeditorpp.dep")
!INCLUDE "dbeditorpp.dep"
!ELSE
!MESSAGE Warning: cannot find "dbeditorpp.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "dbeditorpp - Win32 Release" || "$(CFG)" == "dbeditorpp - Win32 Debug" || "$(CFG)" == "dbeditorpp - Win32 Release Unicode"
SOURCE=.\modsettingenum.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\modsettingenum.obj"	"$(INTDIR)\modsettingenum.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\modsettingenum.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\modsettingenum.obj"	"$(INTDIR)\modsettingenum.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\addeditsettingsdlg.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\addeditsettingsdlg.obj"	"$(INTDIR)\addeditsettingsdlg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\addeditsettingsdlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\addeditsettingsdlg.obj"	"$(INTDIR)\addeditsettingsdlg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\copymodule.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\copymodule.obj"	"$(INTDIR)\copymodule.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\copymodule.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\copymodule.obj"	"$(INTDIR)\copymodule.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\deletemodule.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\deletemodule.obj"	"$(INTDIR)\deletemodule.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\deletemodule.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\deletemodule.obj"	"$(INTDIR)\deletemodule.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\findwindow.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\findwindow.obj"	"$(INTDIR)\findwindow.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\findwindow.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\findwindow.obj"	"$(INTDIR)\findwindow.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\main_window.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\main_window.obj"	"$(INTDIR)\main_window.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\main_window.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\main_window.obj"	"$(INTDIR)\main_window.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\moduletree.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\moduletree.obj"	"$(INTDIR)\moduletree.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\moduletree.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\moduletree.obj"	"$(INTDIR)\moduletree.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\options.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\settinglist.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\settinglist.obj"	"$(INTDIR)\settinglist.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\settinglist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\settinglist.obj"	"$(INTDIR)\settinglist.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\watchedvars.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\watchedvars.obj"	"$(INTDIR)\watchedvars.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\watchedvars.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\watchedvars.obj"	"$(INTDIR)\watchedvars.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\exportimport.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\exportimport.obj"	"$(INTDIR)\exportimport.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\exportimport.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\exportimport.obj"	"$(INTDIR)\exportimport.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\icons.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\icons.obj"	"$(INTDIR)\icons.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\icons.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\icons.obj"	"$(INTDIR)\icons.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\knownmodules.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\knownmodules.obj"	"$(INTDIR)\knownmodules.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\knownmodules.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\knownmodules.obj"	"$(INTDIR)\knownmodules.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\main.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX- /O1 /I "../../include" /I "../ExternalAPI" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\dbeditorpp.pch" /Yc"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr"	"$(INTDIR)\dbeditorpp.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX- /ZI /Od /I "../../include" /I "../ExternalAPI" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /Fp"$(INTDIR)\dbeditorpp.pch" /Yc"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\main.obj"	"$(INTDIR)\dbeditorpp.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX- /Ox /Ot /Og /Os /I "../../include" /I "../ExternalAPI" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\dbeditorpp.pch" /Yc"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr"	"$(INTDIR)\dbeditorpp.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modules.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\modules.obj"	"$(INTDIR)\modules.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\modules.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\modules.obj"	"$(INTDIR)\modules.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\threads.cpp

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"


"$(INTDIR)\threads.obj"	"$(INTDIR)\threads.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"


"$(INTDIR)\threads.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"


"$(INTDIR)\threads.obj"	"$(INTDIR)\threads.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbeditorpp.pch"


!ENDIF

SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF
