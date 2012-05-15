# Microsoft Developer Studio Generated NMAKE File, Based on tabsrmm.dsp
!IF "$(CFG)" == ""
CFG=tabSRMM - Win32 Debug
!MESSAGE No configuration specified. Defaulting to tabSRMM - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "tabSRMM - Win32 Debug" && "$(CFG)" != "tabSRMM - Win32 Release Unicode" && "$(CFG)" != "tabSRMM - Win32 Release" && "$(CFG)" != "tabSRMM - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tabsrmm.mak" CFG="tabSRMM - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tabSRMM - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tabSRMM - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tabSRMM - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tabSRMM - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\Bin\Debug\Plugins\tabsrmm.dll"


CLEAN :
	-@erase "$(INTDIR)\buttonsbar.obj"
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\colorchooser.obj"
	-@erase "$(INTDIR)\contactcache.obj"
	-@erase "$(INTDIR)\container.obj"
	-@erase "$(INTDIR)\containeroptions.obj"
	-@erase "$(INTDIR)\controls.obj"
	-@erase "$(INTDIR)\eventpopups.obj"
	-@erase "$(INTDIR)\generic_msghandlers.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\hotkeyhandler.obj"
	-@erase "$(INTDIR)\ImageDataObject.obj"
	-@erase "$(INTDIR)\infopanel.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\manager.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\mim.obj"
	-@erase "$(INTDIR)\modplus.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msgdlgutils.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgoptions_plus.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\muchighlight.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\selectcontainer.obj"
	-@erase "$(INTDIR)\sendlater.obj"
	-@erase "$(INTDIR)\sendqueue.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\sidebar.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\srmm.pch"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabsrmm_private.res"
	-@erase "$(INTDIR)\taskbar.obj"
	-@erase "$(INTDIR)\templates.obj"
	-@erase "$(INTDIR)\themeio.obj"
	-@erase "$(INTDIR)\themes.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\translator.obj"
	-@erase "$(INTDIR)\trayicon.obj"
	-@erase "$(INTDIR)\TSButton.obj"
	-@erase "$(INTDIR)\typingnotify.obj"
	-@erase "$(INTDIR)\userprefs.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(OUTDIR)\srmm.exp"
	-@erase "$(OUTDIR)\srmm.pdb"
	-@erase "..\..\Bin\Debug\Plugins\tabsrmm.dll"
	-@erase "..\..\Bin\Debug\Plugins\tabsrmm.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\tabsrmm_private.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tabsrmm.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msimg32.lib shlwapi.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\srmm.pdb" /debug /machine:IX86 /out:"..\..\Bin\Debug\Plugins\tabsrmm.dll" /implib:"$(OUTDIR)\srmm.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\colorchooser.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\muchighlight.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\buttonsbar.obj" \
	"$(INTDIR)\contactcache.obj" \
	"$(INTDIR)\container.obj" \
	"$(INTDIR)\containeroptions.obj" \
	"$(INTDIR)\controls.obj" \
	"$(INTDIR)\eventpopups.obj" \
	"$(INTDIR)\generic_msghandlers.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\hotkeyhandler.obj" \
	"$(INTDIR)\ImageDataObject.obj" \
	"$(INTDIR)\infopanel.obj" \
	"$(INTDIR)\mim.obj" \
	"$(INTDIR)\modplus.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msgdlgutils.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgoptions_plus.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\selectcontainer.obj" \
	"$(INTDIR)\sendlater.obj" \
	"$(INTDIR)\sendqueue.obj" \
	"$(INTDIR)\sidebar.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\taskbar.obj" \
	"$(INTDIR)\templates.obj" \
	"$(INTDIR)\themeio.obj" \
	"$(INTDIR)\themes.obj" \
	"$(INTDIR)\translator.obj" \
	"$(INTDIR)\trayicon.obj" \
	"$(INTDIR)\TSButton.obj" \
	"$(INTDIR)\typingnotify.obj" \
	"$(INTDIR)\userprefs.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\tabsrmm_private.res"

"..\..\Bin\Debug\Plugins\tabsrmm.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\Bin\Release Unicode\Plugins\tabsrmm.dll"


CLEAN :
	-@erase "$(INTDIR)\buttonsbar.obj"
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\colorchooser.obj"
	-@erase "$(INTDIR)\contactcache.obj"
	-@erase "$(INTDIR)\container.obj"
	-@erase "$(INTDIR)\containeroptions.obj"
	-@erase "$(INTDIR)\controls.obj"
	-@erase "$(INTDIR)\eventpopups.obj"
	-@erase "$(INTDIR)\generic_msghandlers.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\hotkeyhandler.obj"
	-@erase "$(INTDIR)\ImageDataObject.obj"
	-@erase "$(INTDIR)\infopanel.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\manager.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\mim.obj"
	-@erase "$(INTDIR)\modplus.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msgdlgutils.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgoptions_plus.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\muchighlight.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\selectcontainer.obj"
	-@erase "$(INTDIR)\sendlater.obj"
	-@erase "$(INTDIR)\sendqueue.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\sidebar.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\srmm.pch"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\taskbar.obj"
	-@erase "$(INTDIR)\templates.obj"
	-@erase "$(INTDIR)\themeio.obj"
	-@erase "$(INTDIR)\themes.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\translator.obj"
	-@erase "$(INTDIR)\trayicon.obj"
	-@erase "$(INTDIR)\TSButton.obj"
	-@erase "$(INTDIR)\typingnotify.obj"
	-@erase "$(INTDIR)\userprefs.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(OUTDIR)\srmm.exp"
	-@erase "$(OUTDIR)\tabsrmm.map"
	-@erase "$(OUTDIR)\tabsrmm.pdb"
	-@erase "..\..\Bin\Release Unicode\Plugins\tabsrmm.dll"
	-@erase ".\tabsrmm_private.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

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
RSC_PROJ=/l 0x809 /fo".\tabsrmm_private.res" /d "NDEBUG" /d "UNICODE" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tabsrmm.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msimg32.lib shlwapi.lib /nologo /base:"0x6a540000" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\tabsrmm.pdb" /map:"$(INTDIR)\tabsrmm.map" /debug /machine:IX86 /out:"..\..\Bin\Release Unicode\Plugins\tabsrmm.dll" /implib:"$(OUTDIR)\srmm.lib" /opt:NOWIN98 
LINK32_OBJS= \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\colorchooser.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\muchighlight.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\buttonsbar.obj" \
	"$(INTDIR)\contactcache.obj" \
	"$(INTDIR)\container.obj" \
	"$(INTDIR)\containeroptions.obj" \
	"$(INTDIR)\controls.obj" \
	"$(INTDIR)\eventpopups.obj" \
	"$(INTDIR)\generic_msghandlers.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\hotkeyhandler.obj" \
	"$(INTDIR)\ImageDataObject.obj" \
	"$(INTDIR)\infopanel.obj" \
	"$(INTDIR)\mim.obj" \
	"$(INTDIR)\modplus.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msgdlgutils.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgoptions_plus.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\selectcontainer.obj" \
	"$(INTDIR)\sendlater.obj" \
	"$(INTDIR)\sendqueue.obj" \
	"$(INTDIR)\sidebar.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\taskbar.obj" \
	"$(INTDIR)\templates.obj" \
	"$(INTDIR)\themeio.obj" \
	"$(INTDIR)\themes.obj" \
	"$(INTDIR)\translator.obj" \
	"$(INTDIR)\trayicon.obj" \
	"$(INTDIR)\TSButton.obj" \
	"$(INTDIR)\typingnotify.obj" \
	"$(INTDIR)\userprefs.obj" \
	"$(INTDIR)\utils.obj" \
	".\tabsrmm_private.res"

"..\..\Bin\Release Unicode\Plugins\tabsrmm.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\Bin\Release\Plugins\tabsrmm.dll"


CLEAN :
	-@erase "$(INTDIR)\buttonsbar.obj"
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\colorchooser.obj"
	-@erase "$(INTDIR)\contactcache.obj"
	-@erase "$(INTDIR)\container.obj"
	-@erase "$(INTDIR)\containeroptions.obj"
	-@erase "$(INTDIR)\controls.obj"
	-@erase "$(INTDIR)\eventpopups.obj"
	-@erase "$(INTDIR)\generic_msghandlers.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\hotkeyhandler.obj"
	-@erase "$(INTDIR)\ImageDataObject.obj"
	-@erase "$(INTDIR)\infopanel.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\manager.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\mim.obj"
	-@erase "$(INTDIR)\modplus.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msgdlgutils.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgoptions_plus.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\muchighlight.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\selectcontainer.obj"
	-@erase "$(INTDIR)\sendlater.obj"
	-@erase "$(INTDIR)\sendqueue.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\sidebar.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\srmm.pch"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabsrmm_private.res"
	-@erase "$(INTDIR)\taskbar.obj"
	-@erase "$(INTDIR)\templates.obj"
	-@erase "$(INTDIR)\themeio.obj"
	-@erase "$(INTDIR)\themes.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\translator.obj"
	-@erase "$(INTDIR)\trayicon.obj"
	-@erase "$(INTDIR)\TSButton.obj"
	-@erase "$(INTDIR)\typingnotify.obj"
	-@erase "$(INTDIR)\userprefs.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(OUTDIR)\srmm.exp"
	-@erase "$(OUTDIR)\tabsrmm.map"
	-@erase "..\..\Bin\Release\Plugins\tabsrmm.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

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
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\tabsrmm_private.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tabsrmm.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msimg32.lib shlwapi.lib /nologo /base:"0x6a540000" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\tabsrmm.pdb" /map:"$(INTDIR)\tabsrmm.map" /machine:IX86 /out:"..\..\Bin\Release\Plugins\tabsrmm.dll" /implib:"$(OUTDIR)\srmm.lib" /opt:NOWIN98 
LINK32_OBJS= \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\colorchooser.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\muchighlight.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\buttonsbar.obj" \
	"$(INTDIR)\contactcache.obj" \
	"$(INTDIR)\container.obj" \
	"$(INTDIR)\containeroptions.obj" \
	"$(INTDIR)\controls.obj" \
	"$(INTDIR)\eventpopups.obj" \
	"$(INTDIR)\generic_msghandlers.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\hotkeyhandler.obj" \
	"$(INTDIR)\ImageDataObject.obj" \
	"$(INTDIR)\infopanel.obj" \
	"$(INTDIR)\mim.obj" \
	"$(INTDIR)\modplus.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msgdlgutils.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgoptions_plus.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\selectcontainer.obj" \
	"$(INTDIR)\sendlater.obj" \
	"$(INTDIR)\sendqueue.obj" \
	"$(INTDIR)\sidebar.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\taskbar.obj" \
	"$(INTDIR)\templates.obj" \
	"$(INTDIR)\themeio.obj" \
	"$(INTDIR)\themes.obj" \
	"$(INTDIR)\translator.obj" \
	"$(INTDIR)\trayicon.obj" \
	"$(INTDIR)\TSButton.obj" \
	"$(INTDIR)\typingnotify.obj" \
	"$(INTDIR)\userprefs.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\tabsrmm_private.res"

"..\..\Bin\Release\Plugins\tabsrmm.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode

ALL : "..\..\Bin\Debug Unicode\Plugins\tabsrmm.dll"


CLEAN :
	-@erase "$(INTDIR)\buttonsbar.obj"
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\colorchooser.obj"
	-@erase "$(INTDIR)\contactcache.obj"
	-@erase "$(INTDIR)\container.obj"
	-@erase "$(INTDIR)\containeroptions.obj"
	-@erase "$(INTDIR)\controls.obj"
	-@erase "$(INTDIR)\eventpopups.obj"
	-@erase "$(INTDIR)\generic_msghandlers.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\hotkeyhandler.obj"
	-@erase "$(INTDIR)\ImageDataObject.obj"
	-@erase "$(INTDIR)\infopanel.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\manager.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\mim.obj"
	-@erase "$(INTDIR)\modplus.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msgdlgutils.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgoptions_plus.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\muchighlight.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\selectcontainer.obj"
	-@erase "$(INTDIR)\sendlater.obj"
	-@erase "$(INTDIR)\sendqueue.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\sidebar.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\tabctrl.obj"
	-@erase "$(INTDIR)\tabsrmm.pch"
	-@erase "$(INTDIR)\tabsrmm_private.res"
	-@erase "$(INTDIR)\taskbar.obj"
	-@erase "$(INTDIR)\templates.obj"
	-@erase "$(INTDIR)\themeio.obj"
	-@erase "$(INTDIR)\themes.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\translator.obj"
	-@erase "$(INTDIR)\trayicon.obj"
	-@erase "$(INTDIR)\TSButton.obj"
	-@erase "$(INTDIR)\typingnotify.obj"
	-@erase "$(INTDIR)\userprefs.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(OUTDIR)\srmm.exp"
	-@erase "$(OUTDIR)\srmm.lib"
	-@erase "$(OUTDIR)\tabsrmm.map"
	-@erase "$(OUTDIR)\tabsrmm.pdb"
	-@erase "..\..\Bin\Debug Unicode\Plugins\tabsrmm.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\tabsrmm_private.res" /d "_DEBUG" /d "UNICODE" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tabsrmm.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib comctl32.lib msimg32.lib shlwapi.lib /nologo /base:"0x6a540000" /stack:0xfa00 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\tabsrmm.pdb" /map:"$(INTDIR)\tabsrmm.map" /debug /machine:IX86 /out:"..\..\Bin\Debug Unicode\Plugins\tabsrmm.dll" /implib:"$(OUTDIR)\srmm.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\colorchooser.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\muchighlight.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\buttonsbar.obj" \
	"$(INTDIR)\contactcache.obj" \
	"$(INTDIR)\container.obj" \
	"$(INTDIR)\containeroptions.obj" \
	"$(INTDIR)\controls.obj" \
	"$(INTDIR)\eventpopups.obj" \
	"$(INTDIR)\generic_msghandlers.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\hotkeyhandler.obj" \
	"$(INTDIR)\ImageDataObject.obj" \
	"$(INTDIR)\infopanel.obj" \
	"$(INTDIR)\mim.obj" \
	"$(INTDIR)\modplus.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msgdlgutils.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgoptions_plus.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\selectcontainer.obj" \
	"$(INTDIR)\sendlater.obj" \
	"$(INTDIR)\sendqueue.obj" \
	"$(INTDIR)\sidebar.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\tabctrl.obj" \
	"$(INTDIR)\taskbar.obj" \
	"$(INTDIR)\templates.obj" \
	"$(INTDIR)\themeio.obj" \
	"$(INTDIR)\themes.obj" \
	"$(INTDIR)\translator.obj" \
	"$(INTDIR)\trayicon.obj" \
	"$(INTDIR)\TSButton.obj" \
	"$(INTDIR)\typingnotify.obj" \
	"$(INTDIR)\userprefs.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\tabsrmm_private.res"

"..\..\Bin\Debug Unicode\Plugins\tabsrmm.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("tabsrmm.dep")
!INCLUDE "tabsrmm.dep"
!ELSE 
!MESSAGE Warning: cannot find "tabsrmm.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "tabSRMM - Win32 Debug" || "$(CFG)" == "tabSRMM - Win32 Release Unicode" || "$(CFG)" == "tabSRMM - Win32 Release" || "$(CFG)" == "tabSRMM - Win32 Debug Unicode"
SOURCE=.\chat\clist.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\clist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\clist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\clist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\clist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\chat\colorchooser.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\colorchooser.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\colorchooser.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\colorchooser.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\colorchooser.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\chat\log.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\log.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\log.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\log.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\log.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\chat\main.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\chat\manager.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\manager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\manager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\manager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\manager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\chat\message.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\message.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\message.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\message.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\message.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\chat\muchighlight.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\muchighlight.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O2 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\muchighlight.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O2 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\muchighlight.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\muchighlight.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\chat\options.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\chat\services.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\chat\tools.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\tools.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\tools.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\tools.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\tools.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\chat\window.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\window.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\window.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\window.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\window.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\buttonsbar.cpp

"$(INTDIR)\buttonsbar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\contactcache.cpp

"$(INTDIR)\contactcache.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\container.cpp

"$(INTDIR)\container.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\containeroptions.cpp

"$(INTDIR)\containeroptions.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\controls.cpp

"$(INTDIR)\controls.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\eventpopups.cpp

"$(INTDIR)\eventpopups.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\generic_msghandlers.cpp

"$(INTDIR)\generic_msghandlers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\globals.cpp

"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\hotkeyhandler.cpp

"$(INTDIR)\hotkeyhandler.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\ImageDataObject.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\ImageDataObject.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\ImageDataObject.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\ImageDataObject.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\ImageDataObject.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\infopanel.cpp

"$(INTDIR)\infopanel.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\mim.cpp

"$(INTDIR)\mim.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\tabmodplus\modplus.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\modplus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\modplus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\modplus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\modplus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\msgdialog.cpp

"$(INTDIR)\msgdialog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\msgdlgutils.cpp

"$(INTDIR)\msgdlgutils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\msglog.cpp

"$(INTDIR)\msglog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\msgoptions.cpp

"$(INTDIR)\msgoptions.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\tabmodplus\msgoptions_plus.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\msgoptions_plus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\msgoptions_plus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\msgoptions_plus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yu"../src/commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\msgoptions_plus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tabsrmm.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\msgs.cpp

"$(INTDIR)\msgs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\selectcontainer.cpp

"$(INTDIR)\selectcontainer.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\sendlater.cpp

"$(INTDIR)\sendlater.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\sendqueue.cpp

"$(INTDIR)\sendqueue.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\sidebar.cpp

"$(INTDIR)\sidebar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\srmm.cpp

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\srmm.obj"	"$(INTDIR)\srmm.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\srmm.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\srmm.obj"	"$(INTDIR)\srmm.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_MBCS" /Fp"$(INTDIR)\srmm.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GF /c 

"$(INTDIR)\srmm.obj"	"$(INTDIR)\srmm.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Od /I "../" /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp"$(INTDIR)\tabsrmm.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /GZ /c 

"$(INTDIR)\srmm.obj"	"$(INTDIR)\tabsrmm.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\tabctrl.cpp

"$(INTDIR)\tabctrl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\taskbar.cpp

"$(INTDIR)\taskbar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\templates.cpp

"$(INTDIR)\templates.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\themeio.cpp

"$(INTDIR)\themeio.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\themes.cpp

"$(INTDIR)\themes.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\translator.cpp

"$(INTDIR)\translator.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\trayicon.cpp

"$(INTDIR)\trayicon.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\TSButton.cpp

"$(INTDIR)\TSButton.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\typingnotify.cpp

"$(INTDIR)\typingnotify.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\userprefs.cpp

"$(INTDIR)\userprefs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\utils.cpp

"$(INTDIR)\utils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\srmm.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\tabsrmm_private.rc

!IF  "$(CFG)" == "tabSRMM - Win32 Debug"


"$(INTDIR)\tabsrmm_private.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"


".\tabsrmm_private.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Release"


"$(INTDIR)\tabsrmm_private.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"


"$(INTDIR)\tabsrmm_private.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

