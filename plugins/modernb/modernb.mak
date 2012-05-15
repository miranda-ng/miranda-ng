# Microsoft Developer Studio Generated NMAKE File, Based on modernb.dsp
!IF "$(CFG)" == ""
CFG=modernb - Win32 Release Unicode
!MESSAGE No configuration specified. Defaulting to modernb - Win32 Release Unicode.
!ENDIF

!IF "$(CFG)" != "modernb - Win32 Release" && "$(CFG)" != "modernb - Win32 Debug" && "$(CFG)" != "modernb - Win32 Release Unicode" && "$(CFG)" != "modernb - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "modernb.mak" CFG="modernb - Win32 Debug Unicode"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "modernb - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "modernb - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "modernb - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "modernb - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "modernb - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\release\plugins\clist_modern.dll"


CLEAN :
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\modern_aniavatars.obj"
	-@erase "$(INTDIR)\modern_awaymsg.obj"
	-@erase "$(INTDIR)\modern_cachefuncs.obj"
	-@erase "$(INTDIR)\modern_clc.obj"
	-@erase "$(INTDIR)\modern_clcidents.obj"
	-@erase "$(INTDIR)\modern_clcitems.obj"
	-@erase "$(INTDIR)\modern_clcmsgs.obj"
	-@erase "$(INTDIR)\modern_clcopts.obj"
	-@erase "$(INTDIR)\modern_clcpaint.obj"
	-@erase "$(INTDIR)\modern_clcutils.obj"
	-@erase "$(INTDIR)\modern_clistevents.obj"
	-@erase "$(INTDIR)\modern_clistmenus.obj"
	-@erase "$(INTDIR)\modern_clistmod.obj"
	-@erase "$(INTDIR)\modern_clistopts.obj"
	-@erase "$(INTDIR)\modern_clistsettings.obj"
	-@erase "$(INTDIR)\modern_clisttray.obj"
	-@erase "$(INTDIR)\modern_clui.obj"
	-@erase "$(INTDIR)\modern_cluiframes.obj"
	-@erase "$(INTDIR)\modern_cluiservices.obj"
	-@erase "$(INTDIR)\modern_commonheaders.obj"
	-@erase "$(INTDIR)\modern_contact.obj"
	-@erase "$(INTDIR)\modern_docking.obj"
	-@erase "$(INTDIR)\modern_extraimage.obj"
	-@erase "$(INTDIR)\modern_framesmenu.obj"
	-@erase "$(INTDIR)\modern_gdiplus.obj"
	-@erase "$(INTDIR)\modern_gettextasync.obj"
	-@erase "$(INTDIR)\modern_global.obj"
	-@erase "$(INTDIR)\modern_groupmenu.obj"
	-@erase "$(INTDIR)\modern_image_array.obj"
	-@erase "$(INTDIR)\modern_keyboard.obj"
	-@erase "$(INTDIR)\modern_log.obj"
	-@erase "$(INTDIR)\modern_newrowopts.obj"
	-@erase "$(INTDIR)\modern_popup.obj"
	-@erase "$(INTDIR)\modern_row.obj"
	-@erase "$(INTDIR)\modern_rowheight_funcs.obj"
	-@erase "$(INTDIR)\modern_rowtemplateopt.obj"
	-@erase "$(INTDIR)\modern_skinbutton.obj"
	-@erase "$(INTDIR)\modern_skineditor.obj"
	-@erase "$(INTDIR)\modern_skinengine.obj"
	-@erase "$(INTDIR)\modern_skinopt.obj"
	-@erase "$(INTDIR)\modern_skinselector.obj"
	-@erase "$(INTDIR)\modern_statusbar.obj"
	-@erase "$(INTDIR)\modern_statusbar_options.obj"
	-@erase "$(INTDIR)\modern_sync.obj"
	-@erase "$(INTDIR)\modern_tbbutton.obj"
	-@erase "$(INTDIR)\modern_toolbar.obj"
	-@erase "$(INTDIR)\modern_viewmodebar.obj"
	-@erase "$(INTDIR)\modern_xptheme.obj"
	-@erase "$(INTDIR)\modernb.pch"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\clist_modern.exp"
	-@erase "$(OUTDIR)\clist_modern.map"
	-@erase "$(OUTDIR)\clist_modern.pdb"
	-@erase "..\..\bin\release\plugins\clist_modern.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fp"$(INTDIR)\modernb.pch" /Yu"hdr/modern_commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\msvc6.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\modernb.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib delayimp.lib gdiplus.lib msimg32.lib shlwapi.lib /nologo /base:"0x6590000" /dll /incremental:no /pdb:"$(OUTDIR)\clist_modern.pdb" /map:"$(INTDIR)\clist_modern.map" /debug /machine:I386 /out:"../../bin/release/plugins/clist_modern.dll" /implib:"$(OUTDIR)\clist_modern.lib" /delayload:gdiplus.dll
LINK32_OBJS= \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\modern_aniavatars.obj" \
	"$(INTDIR)\modern_awaymsg.obj" \
	"$(INTDIR)\modern_cachefuncs.obj" \
	"$(INTDIR)\modern_clc.obj" \
	"$(INTDIR)\modern_clcidents.obj" \
	"$(INTDIR)\modern_clcitems.obj" \
	"$(INTDIR)\modern_clcmsgs.obj" \
	"$(INTDIR)\modern_clcopts.obj" \
	"$(INTDIR)\modern_clcpaint.obj" \
	"$(INTDIR)\modern_clcutils.obj" \
	"$(INTDIR)\modern_clistevents.obj" \
	"$(INTDIR)\modern_clistmenus.obj" \
	"$(INTDIR)\modern_clistmod.obj" \
	"$(INTDIR)\modern_clistopts.obj" \
	"$(INTDIR)\modern_clistsettings.obj" \
	"$(INTDIR)\modern_clisttray.obj" \
	"$(INTDIR)\modern_clui.obj" \
	"$(INTDIR)\modern_cluiframes.obj" \
	"$(INTDIR)\modern_cluiservices.obj" \
	"$(INTDIR)\modern_commonheaders.obj" \
	"$(INTDIR)\modern_contact.obj" \
	"$(INTDIR)\modern_docking.obj" \
	"$(INTDIR)\modern_extraimage.obj" \
	"$(INTDIR)\modern_framesmenu.obj" \
	"$(INTDIR)\modern_gdiplus.obj" \
	"$(INTDIR)\modern_gettextasync.obj" \
	"$(INTDIR)\modern_global.obj" \
	"$(INTDIR)\modern_groupmenu.obj" \
	"$(INTDIR)\modern_image_array.obj" \
	"$(INTDIR)\modern_keyboard.obj" \
	"$(INTDIR)\modern_log.obj" \
	"$(INTDIR)\modern_newrowopts.obj" \
	"$(INTDIR)\modern_popup.obj" \
	"$(INTDIR)\modern_row.obj" \
	"$(INTDIR)\modern_rowheight_funcs.obj" \
	"$(INTDIR)\modern_rowtemplateopt.obj" \
	"$(INTDIR)\modern_skinbutton.obj" \
	"$(INTDIR)\modern_skineditor.obj" \
	"$(INTDIR)\modern_skinengine.obj" \
	"$(INTDIR)\modern_skinopt.obj" \
	"$(INTDIR)\modern_skinselector.obj" \
	"$(INTDIR)\modern_statusbar.obj" \
	"$(INTDIR)\modern_statusbar_options.obj" \
	"$(INTDIR)\modern_sync.obj" \
	"$(INTDIR)\modern_tbbutton.obj" \
	"$(INTDIR)\modern_toolbar.obj" \
	"$(INTDIR)\modern_viewmodebar.obj" \
	"$(INTDIR)\modern_xptheme.obj" \
	"$(INTDIR)\msvc6.res"

"..\..\bin\release\plugins\clist_modern.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\bin\debug\plugins\clist_modern.dll"


CLEAN :
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\modern_aniavatars.obj"
	-@erase "$(INTDIR)\modern_awaymsg.obj"
	-@erase "$(INTDIR)\modern_cachefuncs.obj"
	-@erase "$(INTDIR)\modern_clc.obj"
	-@erase "$(INTDIR)\modern_clcidents.obj"
	-@erase "$(INTDIR)\modern_clcitems.obj"
	-@erase "$(INTDIR)\modern_clcmsgs.obj"
	-@erase "$(INTDIR)\modern_clcopts.obj"
	-@erase "$(INTDIR)\modern_clcpaint.obj"
	-@erase "$(INTDIR)\modern_clcutils.obj"
	-@erase "$(INTDIR)\modern_clistevents.obj"
	-@erase "$(INTDIR)\modern_clistmenus.obj"
	-@erase "$(INTDIR)\modern_clistmod.obj"
	-@erase "$(INTDIR)\modern_clistopts.obj"
	-@erase "$(INTDIR)\modern_clistsettings.obj"
	-@erase "$(INTDIR)\modern_clisttray.obj"
	-@erase "$(INTDIR)\modern_clui.obj"
	-@erase "$(INTDIR)\modern_cluiframes.obj"
	-@erase "$(INTDIR)\modern_cluiservices.obj"
	-@erase "$(INTDIR)\modern_commonheaders.obj"
	-@erase "$(INTDIR)\modern_contact.obj"
	-@erase "$(INTDIR)\modern_docking.obj"
	-@erase "$(INTDIR)\modern_extraimage.obj"
	-@erase "$(INTDIR)\modern_framesmenu.obj"
	-@erase "$(INTDIR)\modern_gdiplus.obj"
	-@erase "$(INTDIR)\modern_gettextasync.obj"
	-@erase "$(INTDIR)\modern_global.obj"
	-@erase "$(INTDIR)\modern_groupmenu.obj"
	-@erase "$(INTDIR)\modern_image_array.obj"
	-@erase "$(INTDIR)\modern_keyboard.obj"
	-@erase "$(INTDIR)\modern_log.obj"
	-@erase "$(INTDIR)\modern_newrowopts.obj"
	-@erase "$(INTDIR)\modern_popup.obj"
	-@erase "$(INTDIR)\modern_row.obj"
	-@erase "$(INTDIR)\modern_rowheight_funcs.obj"
	-@erase "$(INTDIR)\modern_rowtemplateopt.obj"
	-@erase "$(INTDIR)\modern_skinbutton.obj"
	-@erase "$(INTDIR)\modern_skineditor.obj"
	-@erase "$(INTDIR)\modern_skinengine.obj"
	-@erase "$(INTDIR)\modern_skinopt.obj"
	-@erase "$(INTDIR)\modern_skinselector.obj"
	-@erase "$(INTDIR)\modern_statusbar.obj"
	-@erase "$(INTDIR)\modern_statusbar_options.obj"
	-@erase "$(INTDIR)\modern_sync.obj"
	-@erase "$(INTDIR)\modern_tbbutton.obj"
	-@erase "$(INTDIR)\modern_toolbar.obj"
	-@erase "$(INTDIR)\modern_viewmodebar.obj"
	-@erase "$(INTDIR)\modern_xptheme.obj"
	-@erase "$(INTDIR)\modernb.pch"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\clist_modern.exp"
	-@erase "$(OUTDIR)\clist_modern.pdb"
	-@erase "..\..\bin\debug\plugins\clist_modern.dll"
	-@erase "..\..\bin\debug\plugins\clist_modern.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fp"$(INTDIR)\modernb.pch" /Yu"hdr/modern_commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\msvc6.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\modernb.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib delayimp.lib gdiplus.lib msimg32.lib shlwapi.lib /nologo /base:"0x6590000" /dll /incremental:yes /pdb:"$(OUTDIR)\clist_modern.pdb" /debug /machine:I386 /out:"../../bin/debug/plugins/clist_modern.dll" /implib:"$(OUTDIR)\clist_modern.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\modern_aniavatars.obj" \
	"$(INTDIR)\modern_awaymsg.obj" \
	"$(INTDIR)\modern_cachefuncs.obj" \
	"$(INTDIR)\modern_clc.obj" \
	"$(INTDIR)\modern_clcidents.obj" \
	"$(INTDIR)\modern_clcitems.obj" \
	"$(INTDIR)\modern_clcmsgs.obj" \
	"$(INTDIR)\modern_clcopts.obj" \
	"$(INTDIR)\modern_clcpaint.obj" \
	"$(INTDIR)\modern_clcutils.obj" \
	"$(INTDIR)\modern_clistevents.obj" \
	"$(INTDIR)\modern_clistmenus.obj" \
	"$(INTDIR)\modern_clistmod.obj" \
	"$(INTDIR)\modern_clistopts.obj" \
	"$(INTDIR)\modern_clistsettings.obj" \
	"$(INTDIR)\modern_clisttray.obj" \
	"$(INTDIR)\modern_clui.obj" \
	"$(INTDIR)\modern_cluiframes.obj" \
	"$(INTDIR)\modern_cluiservices.obj" \
	"$(INTDIR)\modern_commonheaders.obj" \
	"$(INTDIR)\modern_contact.obj" \
	"$(INTDIR)\modern_docking.obj" \
	"$(INTDIR)\modern_extraimage.obj" \
	"$(INTDIR)\modern_framesmenu.obj" \
	"$(INTDIR)\modern_gdiplus.obj" \
	"$(INTDIR)\modern_gettextasync.obj" \
	"$(INTDIR)\modern_global.obj" \
	"$(INTDIR)\modern_groupmenu.obj" \
	"$(INTDIR)\modern_image_array.obj" \
	"$(INTDIR)\modern_keyboard.obj" \
	"$(INTDIR)\modern_log.obj" \
	"$(INTDIR)\modern_newrowopts.obj" \
	"$(INTDIR)\modern_popup.obj" \
	"$(INTDIR)\modern_row.obj" \
	"$(INTDIR)\modern_rowheight_funcs.obj" \
	"$(INTDIR)\modern_rowtemplateopt.obj" \
	"$(INTDIR)\modern_skinbutton.obj" \
	"$(INTDIR)\modern_skineditor.obj" \
	"$(INTDIR)\modern_skinengine.obj" \
	"$(INTDIR)\modern_skinopt.obj" \
	"$(INTDIR)\modern_skinselector.obj" \
	"$(INTDIR)\modern_statusbar.obj" \
	"$(INTDIR)\modern_statusbar_options.obj" \
	"$(INTDIR)\modern_sync.obj" \
	"$(INTDIR)\modern_tbbutton.obj" \
	"$(INTDIR)\modern_toolbar.obj" \
	"$(INTDIR)\modern_viewmodebar.obj" \
	"$(INTDIR)\modern_xptheme.obj" \
	"$(INTDIR)\msvc6.res"

"..\..\bin\debug\plugins\clist_modern.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

OUTDIR=.\./Release_Unicode
INTDIR=.\./Release_Unicode

ALL : "..\..\bin\release Unicode\plugins\clist_modern.dll"


CLEAN :
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\modern_aniavatars.obj"
	-@erase "$(INTDIR)\modern_awaymsg.obj"
	-@erase "$(INTDIR)\modern_cachefuncs.obj"
	-@erase "$(INTDIR)\modern_clc.obj"
	-@erase "$(INTDIR)\modern_clcidents.obj"
	-@erase "$(INTDIR)\modern_clcitems.obj"
	-@erase "$(INTDIR)\modern_clcmsgs.obj"
	-@erase "$(INTDIR)\modern_clcopts.obj"
	-@erase "$(INTDIR)\modern_clcpaint.obj"
	-@erase "$(INTDIR)\modern_clcutils.obj"
	-@erase "$(INTDIR)\modern_clistevents.obj"
	-@erase "$(INTDIR)\modern_clistmenus.obj"
	-@erase "$(INTDIR)\modern_clistmod.obj"
	-@erase "$(INTDIR)\modern_clistopts.obj"
	-@erase "$(INTDIR)\modern_clistsettings.obj"
	-@erase "$(INTDIR)\modern_clisttray.obj"
	-@erase "$(INTDIR)\modern_clui.obj"
	-@erase "$(INTDIR)\modern_cluiframes.obj"
	-@erase "$(INTDIR)\modern_cluiservices.obj"
	-@erase "$(INTDIR)\modern_commonheaders.obj"
	-@erase "$(INTDIR)\modern_contact.obj"
	-@erase "$(INTDIR)\modern_docking.obj"
	-@erase "$(INTDIR)\modern_extraimage.obj"
	-@erase "$(INTDIR)\modern_framesmenu.obj"
	-@erase "$(INTDIR)\modern_gdiplus.obj"
	-@erase "$(INTDIR)\modern_gettextasync.obj"
	-@erase "$(INTDIR)\modern_global.obj"
	-@erase "$(INTDIR)\modern_groupmenu.obj"
	-@erase "$(INTDIR)\modern_image_array.obj"
	-@erase "$(INTDIR)\modern_keyboard.obj"
	-@erase "$(INTDIR)\modern_log.obj"
	-@erase "$(INTDIR)\modern_newrowopts.obj"
	-@erase "$(INTDIR)\modern_popup.obj"
	-@erase "$(INTDIR)\modern_row.obj"
	-@erase "$(INTDIR)\modern_rowheight_funcs.obj"
	-@erase "$(INTDIR)\modern_rowtemplateopt.obj"
	-@erase "$(INTDIR)\modern_skinbutton.obj"
	-@erase "$(INTDIR)\modern_skineditor.obj"
	-@erase "$(INTDIR)\modern_skinengine.obj"
	-@erase "$(INTDIR)\modern_skinopt.obj"
	-@erase "$(INTDIR)\modern_skinselector.obj"
	-@erase "$(INTDIR)\modern_statusbar.obj"
	-@erase "$(INTDIR)\modern_statusbar_options.obj"
	-@erase "$(INTDIR)\modern_sync.obj"
	-@erase "$(INTDIR)\modern_tbbutton.obj"
	-@erase "$(INTDIR)\modern_toolbar.obj"
	-@erase "$(INTDIR)\modern_viewmodebar.obj"
	-@erase "$(INTDIR)\modern_xptheme.obj"
	-@erase "$(INTDIR)\modernb.pch"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\clist_modern.exp"
	-@erase "$(OUTDIR)\clist_modern.map"
	-@erase "$(OUTDIR)\clist_modern.pdb"
	-@erase "..\..\bin\release Unicode\plugins\clist_modern.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fp"$(INTDIR)\modernb.pch" /Yu"hdr/modern_commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\msvc6.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\modernb.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib delayimp.lib gdiplus.lib msimg32.lib shlwapi.lib /nologo /base:"0x6590000" /dll /incremental:no /pdb:"$(OUTDIR)\clist_modern.pdb" /map:"$(INTDIR)\clist_modern.map" /debug /machine:I386 /out:"../../bin/release Unicode/plugins/clist_modern.dll" /implib:"$(OUTDIR)\clist_modern.lib" /delayload:gdiplus.dll
LINK32_OBJS= \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\modern_aniavatars.obj" \
	"$(INTDIR)\modern_awaymsg.obj" \
	"$(INTDIR)\modern_cachefuncs.obj" \
	"$(INTDIR)\modern_clc.obj" \
	"$(INTDIR)\modern_clcidents.obj" \
	"$(INTDIR)\modern_clcitems.obj" \
	"$(INTDIR)\modern_clcmsgs.obj" \
	"$(INTDIR)\modern_clcopts.obj" \
	"$(INTDIR)\modern_clcpaint.obj" \
	"$(INTDIR)\modern_clcutils.obj" \
	"$(INTDIR)\modern_clistevents.obj" \
	"$(INTDIR)\modern_clistmenus.obj" \
	"$(INTDIR)\modern_clistmod.obj" \
	"$(INTDIR)\modern_clistopts.obj" \
	"$(INTDIR)\modern_clistsettings.obj" \
	"$(INTDIR)\modern_clisttray.obj" \
	"$(INTDIR)\modern_clui.obj" \
	"$(INTDIR)\modern_cluiframes.obj" \
	"$(INTDIR)\modern_cluiservices.obj" \
	"$(INTDIR)\modern_commonheaders.obj" \
	"$(INTDIR)\modern_contact.obj" \
	"$(INTDIR)\modern_docking.obj" \
	"$(INTDIR)\modern_extraimage.obj" \
	"$(INTDIR)\modern_framesmenu.obj" \
	"$(INTDIR)\modern_gdiplus.obj" \
	"$(INTDIR)\modern_gettextasync.obj" \
	"$(INTDIR)\modern_global.obj" \
	"$(INTDIR)\modern_groupmenu.obj" \
	"$(INTDIR)\modern_image_array.obj" \
	"$(INTDIR)\modern_keyboard.obj" \
	"$(INTDIR)\modern_log.obj" \
	"$(INTDIR)\modern_newrowopts.obj" \
	"$(INTDIR)\modern_popup.obj" \
	"$(INTDIR)\modern_row.obj" \
	"$(INTDIR)\modern_rowheight_funcs.obj" \
	"$(INTDIR)\modern_rowtemplateopt.obj" \
	"$(INTDIR)\modern_skinbutton.obj" \
	"$(INTDIR)\modern_skineditor.obj" \
	"$(INTDIR)\modern_skinengine.obj" \
	"$(INTDIR)\modern_skinopt.obj" \
	"$(INTDIR)\modern_skinselector.obj" \
	"$(INTDIR)\modern_statusbar.obj" \
	"$(INTDIR)\modern_statusbar_options.obj" \
	"$(INTDIR)\modern_sync.obj" \
	"$(INTDIR)\modern_tbbutton.obj" \
	"$(INTDIR)\modern_toolbar.obj" \
	"$(INTDIR)\modern_viewmodebar.obj" \
	"$(INTDIR)\modern_xptheme.obj" \
	"$(INTDIR)\msvc6.res"

"..\..\bin\release Unicode\plugins\clist_modern.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

OUTDIR=.\./Debug_Unicode
INTDIR=.\./Debug_Unicode

ALL : "..\..\bin\debug Unicode\plugins\clist_modern.dll"


CLEAN :
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\modern_aniavatars.obj"
	-@erase "$(INTDIR)\modern_awaymsg.obj"
	-@erase "$(INTDIR)\modern_cachefuncs.obj"
	-@erase "$(INTDIR)\modern_clc.obj"
	-@erase "$(INTDIR)\modern_clcidents.obj"
	-@erase "$(INTDIR)\modern_clcitems.obj"
	-@erase "$(INTDIR)\modern_clcmsgs.obj"
	-@erase "$(INTDIR)\modern_clcopts.obj"
	-@erase "$(INTDIR)\modern_clcpaint.obj"
	-@erase "$(INTDIR)\modern_clcutils.obj"
	-@erase "$(INTDIR)\modern_clistevents.obj"
	-@erase "$(INTDIR)\modern_clistmenus.obj"
	-@erase "$(INTDIR)\modern_clistmod.obj"
	-@erase "$(INTDIR)\modern_clistopts.obj"
	-@erase "$(INTDIR)\modern_clistsettings.obj"
	-@erase "$(INTDIR)\modern_clisttray.obj"
	-@erase "$(INTDIR)\modern_clui.obj"
	-@erase "$(INTDIR)\modern_cluiframes.obj"
	-@erase "$(INTDIR)\modern_cluiservices.obj"
	-@erase "$(INTDIR)\modern_commonheaders.obj"
	-@erase "$(INTDIR)\modern_contact.obj"
	-@erase "$(INTDIR)\modern_docking.obj"
	-@erase "$(INTDIR)\modern_extraimage.obj"
	-@erase "$(INTDIR)\modern_framesmenu.obj"
	-@erase "$(INTDIR)\modern_gdiplus.obj"
	-@erase "$(INTDIR)\modern_gettextasync.obj"
	-@erase "$(INTDIR)\modern_global.obj"
	-@erase "$(INTDIR)\modern_groupmenu.obj"
	-@erase "$(INTDIR)\modern_image_array.obj"
	-@erase "$(INTDIR)\modern_keyboard.obj"
	-@erase "$(INTDIR)\modern_log.obj"
	-@erase "$(INTDIR)\modern_newrowopts.obj"
	-@erase "$(INTDIR)\modern_popup.obj"
	-@erase "$(INTDIR)\modern_row.obj"
	-@erase "$(INTDIR)\modern_rowheight_funcs.obj"
	-@erase "$(INTDIR)\modern_rowtemplateopt.obj"
	-@erase "$(INTDIR)\modern_skinbutton.obj"
	-@erase "$(INTDIR)\modern_skineditor.obj"
	-@erase "$(INTDIR)\modern_skinengine.obj"
	-@erase "$(INTDIR)\modern_skinopt.obj"
	-@erase "$(INTDIR)\modern_skinselector.obj"
	-@erase "$(INTDIR)\modern_statusbar.obj"
	-@erase "$(INTDIR)\modern_statusbar_options.obj"
	-@erase "$(INTDIR)\modern_sync.obj"
	-@erase "$(INTDIR)\modern_tbbutton.obj"
	-@erase "$(INTDIR)\modern_toolbar.obj"
	-@erase "$(INTDIR)\modern_viewmodebar.obj"
	-@erase "$(INTDIR)\modern_xptheme.obj"
	-@erase "$(INTDIR)\modernb.pch"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\clist_modern.exp"
	-@erase "$(OUTDIR)\clist_modern.pdb"
	-@erase "..\..\bin\debug Unicode\plugins\clist_modern.dll"
	-@erase "..\..\bin\debug Unicode\plugins\clist_modern.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fp"$(INTDIR)\modernb.pch" /Yu"hdr/modern_commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\msvc6.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\modernb.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib delayimp.lib gdiplus.lib msimg32.lib shlwapi.lib /nologo /base:"0x6590000" /dll /incremental:yes /pdb:"$(OUTDIR)\clist_modern.pdb" /debug /machine:I386 /out:"../../bin/debug Unicode/plugins/clist_modern.dll" /implib:"$(OUTDIR)\clist_modern.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\modern_aniavatars.obj" \
	"$(INTDIR)\modern_awaymsg.obj" \
	"$(INTDIR)\modern_cachefuncs.obj" \
	"$(INTDIR)\modern_clc.obj" \
	"$(INTDIR)\modern_clcidents.obj" \
	"$(INTDIR)\modern_clcitems.obj" \
	"$(INTDIR)\modern_clcmsgs.obj" \
	"$(INTDIR)\modern_clcopts.obj" \
	"$(INTDIR)\modern_clcpaint.obj" \
	"$(INTDIR)\modern_clcutils.obj" \
	"$(INTDIR)\modern_clistevents.obj" \
	"$(INTDIR)\modern_clistmenus.obj" \
	"$(INTDIR)\modern_clistmod.obj" \
	"$(INTDIR)\modern_clistopts.obj" \
	"$(INTDIR)\modern_clistsettings.obj" \
	"$(INTDIR)\modern_clisttray.obj" \
	"$(INTDIR)\modern_clui.obj" \
	"$(INTDIR)\modern_cluiframes.obj" \
	"$(INTDIR)\modern_cluiservices.obj" \
	"$(INTDIR)\modern_commonheaders.obj" \
	"$(INTDIR)\modern_contact.obj" \
	"$(INTDIR)\modern_docking.obj" \
	"$(INTDIR)\modern_extraimage.obj" \
	"$(INTDIR)\modern_framesmenu.obj" \
	"$(INTDIR)\modern_gdiplus.obj" \
	"$(INTDIR)\modern_gettextasync.obj" \
	"$(INTDIR)\modern_global.obj" \
	"$(INTDIR)\modern_groupmenu.obj" \
	"$(INTDIR)\modern_image_array.obj" \
	"$(INTDIR)\modern_keyboard.obj" \
	"$(INTDIR)\modern_log.obj" \
	"$(INTDIR)\modern_newrowopts.obj" \
	"$(INTDIR)\modern_popup.obj" \
	"$(INTDIR)\modern_row.obj" \
	"$(INTDIR)\modern_rowheight_funcs.obj" \
	"$(INTDIR)\modern_rowtemplateopt.obj" \
	"$(INTDIR)\modern_skinbutton.obj" \
	"$(INTDIR)\modern_skineditor.obj" \
	"$(INTDIR)\modern_skinengine.obj" \
	"$(INTDIR)\modern_skinopt.obj" \
	"$(INTDIR)\modern_skinselector.obj" \
	"$(INTDIR)\modern_statusbar.obj" \
	"$(INTDIR)\modern_statusbar_options.obj" \
	"$(INTDIR)\modern_sync.obj" \
	"$(INTDIR)\modern_tbbutton.obj" \
	"$(INTDIR)\modern_toolbar.obj" \
	"$(INTDIR)\modern_viewmodebar.obj" \
	"$(INTDIR)\modern_xptheme.obj" \
	"$(INTDIR)\msvc6.res"

"..\..\bin\debug Unicode\plugins\clist_modern.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("modernb.dep")
!INCLUDE "modernb.dep"
!ELSE
!MESSAGE Warning: cannot find "modernb.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "modernb - Win32 Release" || "$(CFG)" == "modernb - Win32 Debug" || "$(CFG)" == "modernb - Win32 Release Unicode" || "$(CFG)" == "modernb - Win32 Debug Unicode"
SOURCE=.\init.cpp

"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_aniavatars.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_aniavatars.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_aniavatars.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_aniavatars.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_aniavatars.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_awaymsg.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_awaymsg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_awaymsg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_awaymsg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_awaymsg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_cachefuncs.cpp

"$(INTDIR)\modern_cachefuncs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clc.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_clc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_clc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_clc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_clc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_clcidents.cpp

"$(INTDIR)\modern_clcidents.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clcitems.cpp

"$(INTDIR)\modern_clcitems.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clcmsgs.cpp

"$(INTDIR)\modern_clcmsgs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clcopts.cpp

"$(INTDIR)\modern_clcopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clcpaint.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_clcpaint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_clcpaint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_clcpaint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_clcpaint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_clcutils.cpp

"$(INTDIR)\modern_clcutils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clistevents.cpp

"$(INTDIR)\modern_clistevents.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clistmenus.cpp

"$(INTDIR)\modern_clistmenus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clistmod.cpp

"$(INTDIR)\modern_clistmod.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clistopts.cpp

"$(INTDIR)\modern_clistopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clistsettings.cpp

"$(INTDIR)\modern_clistsettings.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clisttray.cpp

"$(INTDIR)\modern_clisttray.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_clui.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_clui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_clui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_clui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_clui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_cluiframes.cpp

"$(INTDIR)\modern_cluiframes.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_cluiservices.cpp

"$(INTDIR)\modern_cluiservices.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_commonheaders.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fp"$(INTDIR)\modernb.pch" /Yc"hdr/modern_commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_commonheaders.obj"	"$(INTDIR)\modernb.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fp"$(INTDIR)\modernb.pch" /Yc"hdr/modern_commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_commonheaders.obj"	"$(INTDIR)\modernb.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fp"$(INTDIR)\modernb.pch" /Yc"hdr/modern_commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_commonheaders.obj"	"$(INTDIR)\modernb.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fp"$(INTDIR)\modernb.pch" /Yc"hdr/modern_commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_commonheaders.obj"	"$(INTDIR)\modernb.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_contact.cpp

"$(INTDIR)\modern_contact.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_docking.cpp

"$(INTDIR)\modern_docking.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_extraimage.cpp

"$(INTDIR)\modern_extraimage.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_framesmenu.cpp

"$(INTDIR)\modern_framesmenu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_gdiplus.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_gdiplus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_gdiplus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_gdiplus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_gdiplus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_gettextasync.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_gettextasync.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_gettextasync.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_gettextasync.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_gettextasync.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_global.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_global.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_global.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_global.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_global.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_groupmenu.cpp

"$(INTDIR)\modern_groupmenu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_image_array.cpp

"$(INTDIR)\modern_image_array.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_keyboard.cpp

"$(INTDIR)\modern_keyboard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_log.cpp

"$(INTDIR)\modern_log.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_newrowopts.cpp

"$(INTDIR)\modern_newrowopts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_popup.cpp

"$(INTDIR)\modern_popup.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_row.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_row.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_row.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_row.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_row.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_rowheight_funcs.cpp

"$(INTDIR)\modern_rowheight_funcs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_rowtemplateopt.cpp

"$(INTDIR)\modern_rowtemplateopt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_skinbutton.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_skinbutton.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_skinbutton.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_skinbutton.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_skinbutton.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_skineditor.cpp

"$(INTDIR)\modern_skineditor.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_skinengine.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_skinengine.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_skinengine.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_skinengine.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_skinengine.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_skinopt.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_skinopt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_skinopt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_skinopt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_skinopt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_skinselector.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_skinselector.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_skinselector.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_skinselector.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_skinselector.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_statusbar.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_statusbar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_statusbar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_statusbar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_statusbar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_statusbar_options.cpp

"$(INTDIR)\modern_statusbar_options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_sync.cpp

"$(INTDIR)\modern_sync.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernb.pch"


SOURCE=.\modern_tbbutton.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_tbbutton.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_tbbutton.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_tbbutton.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_tbbutton.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_toolbar.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_toolbar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_toolbar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_toolbar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_toolbar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_viewmodebar.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_viewmodebar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_viewmodebar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_viewmodebar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_viewmodebar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\modern_xptheme.cpp

!IF  "$(CFG)" == "modernb - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_xptheme.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_xptheme.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\modern_xptheme.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\modern_xptheme.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\msvc6.rc

"$(INTDIR)\msvc6.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF
