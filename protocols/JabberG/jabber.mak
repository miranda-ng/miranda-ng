# Microsoft Developer Studio Generated NMAKE File, Based on jabber.dsp
!IF "$(CFG)" == ""
CFG=jabberg - Win32 Release Unicode
!MESSAGE No configuration specified. Defaulting to jabberg - Win32 Release Unicode.
!ENDIF

!IF "$(CFG)" != "jabberg - Win32 Release" && "$(CFG)" != "jabberg - Win32 Debug" && "$(CFG)" != "jabberg - Win32 Release Unicode" && "$(CFG)" != "jabberg - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "jabber.mak" CFG="jabberg - Win32 Debug Unicode"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "jabberg - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jabberg - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jabberg - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "jabberg - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "jabberg - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0"

ALL : "..\..\bin\release\plugins\jabber.dll"

!ELSE

ALL : "zlib - Win32 Release" "..\..\bin\release\plugins\jabber.dll"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"zlib - Win32 ReleaseCLEAN"
!ELSE
CLEAN :
!ENDIF
	-@erase "$(INTDIR)\jabber.obj"
	-@erase "$(INTDIR)\jabber.pch"
	-@erase "$(INTDIR)\jabber_adhoc.obj"
	-@erase "$(INTDIR)\jabber_agent.obj"
	-@erase "$(INTDIR)\jabber_bookmarks.obj"
	-@erase "$(INTDIR)\jabber_byte.obj"
	-@erase "$(INTDIR)\jabber_caps.obj"
	-@erase "$(INTDIR)\jabber_captcha.obj"
	-@erase "$(INTDIR)\jabber_chat.obj"
	-@erase "$(INTDIR)\jabber_console.obj"
	-@erase "$(INTDIR)\jabber_disco.obj"
	-@erase "$(INTDIR)\jabber_events.obj"
	-@erase "$(INTDIR)\jabber_file.obj"
	-@erase "$(INTDIR)\jabber_form.obj"
	-@erase "$(INTDIR)\jabber_ft.obj"
	-@erase "$(INTDIR)\jabber_groupchat.obj"
	-@erase "$(INTDIR)\jabber_ibb.obj"
	-@erase "$(INTDIR)\jabber_icolib.obj"
	-@erase "$(INTDIR)\jabber_iq.obj"
	-@erase "$(INTDIR)\jabber_iq_handlers.obj"
	-@erase "$(INTDIR)\jabber_iqid.obj"
	-@erase "$(INTDIR)\jabber_iqid_muc.obj"
	-@erase "$(INTDIR)\jabber_libstr.obj"
	-@erase "$(INTDIR)\jabber_list.obj"
	-@erase "$(INTDIR)\jabber_menu.obj"
	-@erase "$(INTDIR)\jabber_message_handlers.obj"
	-@erase "$(INTDIR)\jabber_message_manager.obj"
	-@erase "$(INTDIR)\jabber_misc.obj"
	-@erase "$(INTDIR)\jabber_notes.obj"
	-@erase "$(INTDIR)\jabber_opt.obj"
	-@erase "$(INTDIR)\jabber_opttree.obj"
	-@erase "$(INTDIR)\jabber_password.obj"
	-@erase "$(INTDIR)\jabber_presence_manager.obj"
	-@erase "$(INTDIR)\jabber_privacy.obj"
	-@erase "$(INTDIR)\jabber_proto.obj"
	-@erase "$(INTDIR)\jabber_proxy.obj"
	-@erase "$(INTDIR)\jabber_rc.obj"
	-@erase "$(INTDIR)\jabber_search.obj"
	-@erase "$(INTDIR)\jabber_secur.obj"
	-@erase "$(INTDIR)\jabber_send_manager.obj"
	-@erase "$(INTDIR)\jabber_std.obj"
	-@erase "$(INTDIR)\jabber_svc.obj"
	-@erase "$(INTDIR)\jabber_thread.obj"
	-@erase "$(INTDIR)\jabber_treelist.obj"
	-@erase "$(INTDIR)\jabber_userinfo.obj"
	-@erase "$(INTDIR)\jabber_util.obj"
	-@erase "$(INTDIR)\jabber_vcard.obj"
	-@erase "$(INTDIR)\jabber_ws.obj"
	-@erase "$(INTDIR)\jabber_xml.obj"
	-@erase "$(INTDIR)\jabber_xstatus.obj"
	-@erase "$(INTDIR)\jabber_zstream.obj"
	-@erase "$(INTDIR)\MString.obj"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\ui_utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\jabber.exp"
	-@erase "$(OUTDIR)\jabber.map"
	-@erase "$(OUTDIR)\jabber.pdb"
	-@erase "..\..\bin\release\plugins\jabber.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\msvc6.res" /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jabber.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib comctl32.lib /nologo /base:"0x32500000" /dll /incremental:no /pdb:"$(OUTDIR)\jabber.pdb" /map:"$(INTDIR)\jabber.map" /debug /machine:I386 /out:"../../bin/release/plugins/jabber.dll" /implib:"$(OUTDIR)\jabber.lib" /ALIGN:4096 /ignore:4108
LINK32_OBJS= \
	"$(INTDIR)\jabber.obj" \
	"$(INTDIR)\jabber_adhoc.obj" \
	"$(INTDIR)\jabber_agent.obj" \
	"$(INTDIR)\jabber_bookmarks.obj" \
	"$(INTDIR)\jabber_byte.obj" \
	"$(INTDIR)\jabber_caps.obj" \
	"$(INTDIR)\jabber_chat.obj" \
	"$(INTDIR)\jabber_console.obj" \
	"$(INTDIR)\jabber_disco.obj" \
	"$(INTDIR)\jabber_events.obj" \
	"$(INTDIR)\jabber_file.obj" \
	"$(INTDIR)\jabber_form.obj" \
	"$(INTDIR)\jabber_ft.obj" \
	"$(INTDIR)\jabber_groupchat.obj" \
	"$(INTDIR)\jabber_ibb.obj" \
	"$(INTDIR)\jabber_icolib.obj" \
	"$(INTDIR)\jabber_iq.obj" \
	"$(INTDIR)\jabber_iq_handlers.obj" \
	"$(INTDIR)\jabber_iqid.obj" \
	"$(INTDIR)\jabber_iqid_muc.obj" \
	"$(INTDIR)\jabber_libstr.obj" \
	"$(INTDIR)\jabber_list.obj" \
	"$(INTDIR)\jabber_menu.obj" \
	"$(INTDIR)\jabber_message_handlers.obj" \
	"$(INTDIR)\jabber_message_manager.obj" \
	"$(INTDIR)\jabber_misc.obj" \
	"$(INTDIR)\jabber_notes.obj" \
	"$(INTDIR)\jabber_opt.obj" \
	"$(INTDIR)\jabber_opttree.obj" \
	"$(INTDIR)\jabber_password.obj" \
	"$(INTDIR)\jabber_presence_manager.obj" \
	"$(INTDIR)\jabber_privacy.obj" \
	"$(INTDIR)\jabber_proto.obj" \
	"$(INTDIR)\jabber_proxy.obj" \
	"$(INTDIR)\jabber_rc.obj" \
	"$(INTDIR)\jabber_search.obj" \
	"$(INTDIR)\jabber_secur.obj" \
	"$(INTDIR)\jabber_send_manager.obj" \
	"$(INTDIR)\jabber_std.obj" \
	"$(INTDIR)\jabber_svc.obj" \
	"$(INTDIR)\jabber_thread.obj" \
	"$(INTDIR)\jabber_treelist.obj" \
	"$(INTDIR)\jabber_userinfo.obj" \
	"$(INTDIR)\jabber_util.obj" \
	"$(INTDIR)\jabber_vcard.obj" \
	"$(INTDIR)\jabber_ws.obj" \
	"$(INTDIR)\jabber_xml.obj" \
	"$(INTDIR)\jabber_xstatus.obj" \
	"$(INTDIR)\jabber_zstream.obj" \
	"$(INTDIR)\MString.obj" \
	"$(INTDIR)\ui_utils.obj" \
	"$(INTDIR)\msvc6.res" \
	"$(INTDIR)\jabber_captcha.obj" \
	"..\..\plugins\zlib\Release\zlib.lib"

"..\..\bin\release\plugins\jabber.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "..\..\bin\debug\plugins\jabber.dll" "$(OUTDIR)\jabber.bsc"

!ELSE

ALL : "zlib - Win32 Debug" "..\..\bin\debug\plugins\jabber.dll" "$(OUTDIR)\jabber.bsc"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"zlib - Win32 DebugCLEAN"
!ELSE
CLEAN :
!ENDIF
	-@erase "$(INTDIR)\jabber.obj"
	-@erase "$(INTDIR)\jabber.pch"
	-@erase "$(INTDIR)\jabber.sbr"
	-@erase "$(INTDIR)\jabber_adhoc.obj"
	-@erase "$(INTDIR)\jabber_adhoc.sbr"
	-@erase "$(INTDIR)\jabber_agent.obj"
	-@erase "$(INTDIR)\jabber_agent.sbr"
	-@erase "$(INTDIR)\jabber_bookmarks.obj"
	-@erase "$(INTDIR)\jabber_bookmarks.sbr"
	-@erase "$(INTDIR)\jabber_byte.obj"
	-@erase "$(INTDIR)\jabber_byte.sbr"
	-@erase "$(INTDIR)\jabber_caps.obj"
	-@erase "$(INTDIR)\jabber_caps.sbr"
	-@erase "$(INTDIR)\jabber_captcha.obj"
	-@erase "$(INTDIR)\jabber_captcha.sbr"
	-@erase "$(INTDIR)\jabber_chat.obj"
	-@erase "$(INTDIR)\jabber_chat.sbr"
	-@erase "$(INTDIR)\jabber_console.obj"
	-@erase "$(INTDIR)\jabber_console.sbr"
	-@erase "$(INTDIR)\jabber_disco.obj"
	-@erase "$(INTDIR)\jabber_disco.sbr"
	-@erase "$(INTDIR)\jabber_events.obj"
	-@erase "$(INTDIR)\jabber_events.sbr"
	-@erase "$(INTDIR)\jabber_file.obj"
	-@erase "$(INTDIR)\jabber_file.sbr"
	-@erase "$(INTDIR)\jabber_form.obj"
	-@erase "$(INTDIR)\jabber_form.sbr"
	-@erase "$(INTDIR)\jabber_ft.obj"
	-@erase "$(INTDIR)\jabber_ft.sbr"
	-@erase "$(INTDIR)\jabber_groupchat.obj"
	-@erase "$(INTDIR)\jabber_groupchat.sbr"
	-@erase "$(INTDIR)\jabber_ibb.obj"
	-@erase "$(INTDIR)\jabber_ibb.sbr"
	-@erase "$(INTDIR)\jabber_icolib.obj"
	-@erase "$(INTDIR)\jabber_icolib.sbr"
	-@erase "$(INTDIR)\jabber_iq.obj"
	-@erase "$(INTDIR)\jabber_iq.sbr"
	-@erase "$(INTDIR)\jabber_iq_handlers.obj"
	-@erase "$(INTDIR)\jabber_iq_handlers.sbr"
	-@erase "$(INTDIR)\jabber_iqid.obj"
	-@erase "$(INTDIR)\jabber_iqid.sbr"
	-@erase "$(INTDIR)\jabber_iqid_muc.obj"
	-@erase "$(INTDIR)\jabber_iqid_muc.sbr"
	-@erase "$(INTDIR)\jabber_libstr.obj"
	-@erase "$(INTDIR)\jabber_libstr.sbr"
	-@erase "$(INTDIR)\jabber_list.obj"
	-@erase "$(INTDIR)\jabber_list.sbr"
	-@erase "$(INTDIR)\jabber_menu.obj"
	-@erase "$(INTDIR)\jabber_menu.sbr"
	-@erase "$(INTDIR)\jabber_message_handlers.obj"
	-@erase "$(INTDIR)\jabber_message_handlers.sbr"
	-@erase "$(INTDIR)\jabber_message_manager.obj"
	-@erase "$(INTDIR)\jabber_message_manager.sbr"
	-@erase "$(INTDIR)\jabber_misc.obj"
	-@erase "$(INTDIR)\jabber_misc.sbr"
	-@erase "$(INTDIR)\jabber_notes.obj"
	-@erase "$(INTDIR)\jabber_notes.sbr"
	-@erase "$(INTDIR)\jabber_opt.obj"
	-@erase "$(INTDIR)\jabber_opt.sbr"
	-@erase "$(INTDIR)\jabber_opttree.obj"
	-@erase "$(INTDIR)\jabber_opttree.sbr"
	-@erase "$(INTDIR)\jabber_password.obj"
	-@erase "$(INTDIR)\jabber_password.sbr"
	-@erase "$(INTDIR)\jabber_presence_manager.obj"
	-@erase "$(INTDIR)\jabber_presence_manager.sbr"
	-@erase "$(INTDIR)\jabber_privacy.obj"
	-@erase "$(INTDIR)\jabber_privacy.sbr"
	-@erase "$(INTDIR)\jabber_proto.obj"
	-@erase "$(INTDIR)\jabber_proto.sbr"
	-@erase "$(INTDIR)\jabber_proxy.obj"
	-@erase "$(INTDIR)\jabber_proxy.sbr"
	-@erase "$(INTDIR)\jabber_rc.obj"
	-@erase "$(INTDIR)\jabber_rc.sbr"
	-@erase "$(INTDIR)\jabber_search.obj"
	-@erase "$(INTDIR)\jabber_search.sbr"
	-@erase "$(INTDIR)\jabber_secur.obj"
	-@erase "$(INTDIR)\jabber_secur.sbr"
	-@erase "$(INTDIR)\jabber_send_manager.obj"
	-@erase "$(INTDIR)\jabber_send_manager.sbr"
	-@erase "$(INTDIR)\jabber_std.obj"
	-@erase "$(INTDIR)\jabber_std.sbr"
	-@erase "$(INTDIR)\jabber_svc.obj"
	-@erase "$(INTDIR)\jabber_svc.sbr"
	-@erase "$(INTDIR)\jabber_thread.obj"
	-@erase "$(INTDIR)\jabber_thread.sbr"
	-@erase "$(INTDIR)\jabber_treelist.obj"
	-@erase "$(INTDIR)\jabber_treelist.sbr"
	-@erase "$(INTDIR)\jabber_userinfo.obj"
	-@erase "$(INTDIR)\jabber_userinfo.sbr"
	-@erase "$(INTDIR)\jabber_util.obj"
	-@erase "$(INTDIR)\jabber_util.sbr"
	-@erase "$(INTDIR)\jabber_vcard.obj"
	-@erase "$(INTDIR)\jabber_vcard.sbr"
	-@erase "$(INTDIR)\jabber_ws.obj"
	-@erase "$(INTDIR)\jabber_ws.sbr"
	-@erase "$(INTDIR)\jabber_xml.obj"
	-@erase "$(INTDIR)\jabber_xml.sbr"
	-@erase "$(INTDIR)\jabber_xstatus.obj"
	-@erase "$(INTDIR)\jabber_xstatus.sbr"
	-@erase "$(INTDIR)\jabber_zstream.obj"
	-@erase "$(INTDIR)\jabber_zstream.sbr"
	-@erase "$(INTDIR)\MString.obj"
	-@erase "$(INTDIR)\MString.sbr"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\ui_utils.obj"
	-@erase "$(INTDIR)\ui_utils.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\jabber.bsc"
	-@erase "$(OUTDIR)\jabber.exp"
	-@erase "$(OUTDIR)\jabber.map"
	-@erase "$(OUTDIR)\jabber.pdb"
	-@erase "..\..\bin\debug\plugins\jabber.dll"
	-@erase "..\..\bin\debug\plugins\jabber.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\msvc6.res" /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jabber.bsc"
BSC32_SBRS= \
	"$(INTDIR)\jabber.sbr" \
	"$(INTDIR)\jabber_adhoc.sbr" \
	"$(INTDIR)\jabber_agent.sbr" \
	"$(INTDIR)\jabber_bookmarks.sbr" \
	"$(INTDIR)\jabber_byte.sbr" \
	"$(INTDIR)\jabber_caps.sbr" \
	"$(INTDIR)\jabber_chat.sbr" \
	"$(INTDIR)\jabber_console.sbr" \
	"$(INTDIR)\jabber_disco.sbr" \
	"$(INTDIR)\jabber_events.sbr" \
	"$(INTDIR)\jabber_file.sbr" \
	"$(INTDIR)\jabber_form.sbr" \
	"$(INTDIR)\jabber_ft.sbr" \
	"$(INTDIR)\jabber_groupchat.sbr" \
	"$(INTDIR)\jabber_ibb.sbr" \
	"$(INTDIR)\jabber_icolib.sbr" \
	"$(INTDIR)\jabber_iq.sbr" \
	"$(INTDIR)\jabber_iq_handlers.sbr" \
	"$(INTDIR)\jabber_iqid.sbr" \
	"$(INTDIR)\jabber_iqid_muc.sbr" \
	"$(INTDIR)\jabber_libstr.sbr" \
	"$(INTDIR)\jabber_list.sbr" \
	"$(INTDIR)\jabber_menu.sbr" \
	"$(INTDIR)\jabber_message_handlers.sbr" \
	"$(INTDIR)\jabber_message_manager.sbr" \
	"$(INTDIR)\jabber_misc.sbr" \
	"$(INTDIR)\jabber_notes.sbr" \
	"$(INTDIR)\jabber_opt.sbr" \
	"$(INTDIR)\jabber_opttree.sbr" \
	"$(INTDIR)\jabber_password.sbr" \
	"$(INTDIR)\jabber_presence_manager.sbr" \
	"$(INTDIR)\jabber_privacy.sbr" \
	"$(INTDIR)\jabber_proto.sbr" \
	"$(INTDIR)\jabber_proxy.sbr" \
	"$(INTDIR)\jabber_rc.sbr" \
	"$(INTDIR)\jabber_search.sbr" \
	"$(INTDIR)\jabber_secur.sbr" \
	"$(INTDIR)\jabber_send_manager.sbr" \
	"$(INTDIR)\jabber_std.sbr" \
	"$(INTDIR)\jabber_svc.sbr" \
	"$(INTDIR)\jabber_thread.sbr" \
	"$(INTDIR)\jabber_treelist.sbr" \
	"$(INTDIR)\jabber_userinfo.sbr" \
	"$(INTDIR)\jabber_util.sbr" \
	"$(INTDIR)\jabber_vcard.sbr" \
	"$(INTDIR)\jabber_ws.sbr" \
	"$(INTDIR)\jabber_xml.sbr" \
	"$(INTDIR)\jabber_xstatus.sbr" \
	"$(INTDIR)\jabber_zstream.sbr" \
	"$(INTDIR)\MString.sbr" \
	"$(INTDIR)\ui_utils.sbr" \
	"$(INTDIR)\jabber_captcha.sbr"

"$(OUTDIR)\jabber.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib comctl32.lib /nologo /base:"0x32500000" /dll /incremental:yes /pdb:"$(OUTDIR)\jabber.pdb" /map:"$(INTDIR)\jabber.map" /debug /machine:I386 /out:"../../bin/debug/plugins/jabber.dll" /implib:"$(OUTDIR)\jabber.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\jabber.obj" \
	"$(INTDIR)\jabber_adhoc.obj" \
	"$(INTDIR)\jabber_agent.obj" \
	"$(INTDIR)\jabber_bookmarks.obj" \
	"$(INTDIR)\jabber_byte.obj" \
	"$(INTDIR)\jabber_caps.obj" \
	"$(INTDIR)\jabber_chat.obj" \
	"$(INTDIR)\jabber_console.obj" \
	"$(INTDIR)\jabber_disco.obj" \
	"$(INTDIR)\jabber_events.obj" \
	"$(INTDIR)\jabber_file.obj" \
	"$(INTDIR)\jabber_form.obj" \
	"$(INTDIR)\jabber_ft.obj" \
	"$(INTDIR)\jabber_groupchat.obj" \
	"$(INTDIR)\jabber_ibb.obj" \
	"$(INTDIR)\jabber_icolib.obj" \
	"$(INTDIR)\jabber_iq.obj" \
	"$(INTDIR)\jabber_iq_handlers.obj" \
	"$(INTDIR)\jabber_iqid.obj" \
	"$(INTDIR)\jabber_iqid_muc.obj" \
	"$(INTDIR)\jabber_libstr.obj" \
	"$(INTDIR)\jabber_list.obj" \
	"$(INTDIR)\jabber_menu.obj" \
	"$(INTDIR)\jabber_message_handlers.obj" \
	"$(INTDIR)\jabber_message_manager.obj" \
	"$(INTDIR)\jabber_misc.obj" \
	"$(INTDIR)\jabber_notes.obj" \
	"$(INTDIR)\jabber_opt.obj" \
	"$(INTDIR)\jabber_opttree.obj" \
	"$(INTDIR)\jabber_password.obj" \
	"$(INTDIR)\jabber_presence_manager.obj" \
	"$(INTDIR)\jabber_privacy.obj" \
	"$(INTDIR)\jabber_proto.obj" \
	"$(INTDIR)\jabber_proxy.obj" \
	"$(INTDIR)\jabber_rc.obj" \
	"$(INTDIR)\jabber_search.obj" \
	"$(INTDIR)\jabber_secur.obj" \
	"$(INTDIR)\jabber_send_manager.obj" \
	"$(INTDIR)\jabber_std.obj" \
	"$(INTDIR)\jabber_svc.obj" \
	"$(INTDIR)\jabber_thread.obj" \
	"$(INTDIR)\jabber_treelist.obj" \
	"$(INTDIR)\jabber_userinfo.obj" \
	"$(INTDIR)\jabber_util.obj" \
	"$(INTDIR)\jabber_vcard.obj" \
	"$(INTDIR)\jabber_ws.obj" \
	"$(INTDIR)\jabber_xml.obj" \
	"$(INTDIR)\jabber_xstatus.obj" \
	"$(INTDIR)\jabber_zstream.obj" \
	"$(INTDIR)\MString.obj" \
	"$(INTDIR)\ui_utils.obj" \
	"$(INTDIR)\msvc6.res" \
	"$(INTDIR)\jabber_captcha.obj" \
	"..\..\plugins\zlib\Debug\zlib.lib"

"..\..\bin\debug\plugins\jabber.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

!IF "$(RECURSE)" == "0"

ALL : "..\..\bin\Release Unicode\plugins\jabber.dll"

!ELSE

ALL : "zlib - Win32 Release Unicode" "..\..\bin\Release Unicode\plugins\jabber.dll"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"zlib - Win32 Release UnicodeCLEAN"
!ELSE
CLEAN :
!ENDIF
	-@erase "$(INTDIR)\jabber.obj"
	-@erase "$(INTDIR)\jabber.pch"
	-@erase "$(INTDIR)\jabber_adhoc.obj"
	-@erase "$(INTDIR)\jabber_agent.obj"
	-@erase "$(INTDIR)\jabber_bookmarks.obj"
	-@erase "$(INTDIR)\jabber_byte.obj"
	-@erase "$(INTDIR)\jabber_caps.obj"
	-@erase "$(INTDIR)\jabber_captcha.obj"
	-@erase "$(INTDIR)\jabber_chat.obj"
	-@erase "$(INTDIR)\jabber_console.obj"
	-@erase "$(INTDIR)\jabber_disco.obj"
	-@erase "$(INTDIR)\jabber_events.obj"
	-@erase "$(INTDIR)\jabber_file.obj"
	-@erase "$(INTDIR)\jabber_form.obj"
	-@erase "$(INTDIR)\jabber_ft.obj"
	-@erase "$(INTDIR)\jabber_groupchat.obj"
	-@erase "$(INTDIR)\jabber_ibb.obj"
	-@erase "$(INTDIR)\jabber_icolib.obj"
	-@erase "$(INTDIR)\jabber_iq.obj"
	-@erase "$(INTDIR)\jabber_iq_handlers.obj"
	-@erase "$(INTDIR)\jabber_iqid.obj"
	-@erase "$(INTDIR)\jabber_iqid_muc.obj"
	-@erase "$(INTDIR)\jabber_libstr.obj"
	-@erase "$(INTDIR)\jabber_list.obj"
	-@erase "$(INTDIR)\jabber_menu.obj"
	-@erase "$(INTDIR)\jabber_message_handlers.obj"
	-@erase "$(INTDIR)\jabber_message_manager.obj"
	-@erase "$(INTDIR)\jabber_misc.obj"
	-@erase "$(INTDIR)\jabber_notes.obj"
	-@erase "$(INTDIR)\jabber_opt.obj"
	-@erase "$(INTDIR)\jabber_opttree.obj"
	-@erase "$(INTDIR)\jabber_password.obj"
	-@erase "$(INTDIR)\jabber_presence_manager.obj"
	-@erase "$(INTDIR)\jabber_privacy.obj"
	-@erase "$(INTDIR)\jabber_proto.obj"
	-@erase "$(INTDIR)\jabber_proxy.obj"
	-@erase "$(INTDIR)\jabber_rc.obj"
	-@erase "$(INTDIR)\jabber_search.obj"
	-@erase "$(INTDIR)\jabber_secur.obj"
	-@erase "$(INTDIR)\jabber_send_manager.obj"
	-@erase "$(INTDIR)\jabber_std.obj"
	-@erase "$(INTDIR)\jabber_svc.obj"
	-@erase "$(INTDIR)\jabber_thread.obj"
	-@erase "$(INTDIR)\jabber_treelist.obj"
	-@erase "$(INTDIR)\jabber_userinfo.obj"
	-@erase "$(INTDIR)\jabber_util.obj"
	-@erase "$(INTDIR)\jabber_vcard.obj"
	-@erase "$(INTDIR)\jabber_ws.obj"
	-@erase "$(INTDIR)\jabber_xml.obj"
	-@erase "$(INTDIR)\jabber_xstatus.obj"
	-@erase "$(INTDIR)\jabber_zstream.obj"
	-@erase "$(INTDIR)\MString.obj"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\ui_utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\jabber.exp"
	-@erase "$(OUTDIR)\jabber.map"
	-@erase "$(OUTDIR)\jabber.pdb"
	-@erase "..\..\bin\Release Unicode\plugins\jabber.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\msvc6.res" /i "../../include" /d "NDEBUG" /d "UNICODE"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jabber.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib comctl32.lib /nologo /base:"0x32500000" /dll /incremental:no /pdb:"$(OUTDIR)\jabber.pdb" /map:"$(INTDIR)\jabber.map" /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/jabber.dll" /implib:"$(OUTDIR)\jabber.lib" /ALIGN:4096 /ignore:4108
LINK32_OBJS= \
	"$(INTDIR)\jabber.obj" \
	"$(INTDIR)\jabber_adhoc.obj" \
	"$(INTDIR)\jabber_agent.obj" \
	"$(INTDIR)\jabber_bookmarks.obj" \
	"$(INTDIR)\jabber_byte.obj" \
	"$(INTDIR)\jabber_caps.obj" \
	"$(INTDIR)\jabber_chat.obj" \
	"$(INTDIR)\jabber_console.obj" \
	"$(INTDIR)\jabber_disco.obj" \
	"$(INTDIR)\jabber_events.obj" \
	"$(INTDIR)\jabber_file.obj" \
	"$(INTDIR)\jabber_form.obj" \
	"$(INTDIR)\jabber_ft.obj" \
	"$(INTDIR)\jabber_groupchat.obj" \
	"$(INTDIR)\jabber_ibb.obj" \
	"$(INTDIR)\jabber_icolib.obj" \
	"$(INTDIR)\jabber_iq.obj" \
	"$(INTDIR)\jabber_iq_handlers.obj" \
	"$(INTDIR)\jabber_iqid.obj" \
	"$(INTDIR)\jabber_iqid_muc.obj" \
	"$(INTDIR)\jabber_libstr.obj" \
	"$(INTDIR)\jabber_list.obj" \
	"$(INTDIR)\jabber_menu.obj" \
	"$(INTDIR)\jabber_message_handlers.obj" \
	"$(INTDIR)\jabber_message_manager.obj" \
	"$(INTDIR)\jabber_misc.obj" \
	"$(INTDIR)\jabber_notes.obj" \
	"$(INTDIR)\jabber_opt.obj" \
	"$(INTDIR)\jabber_opttree.obj" \
	"$(INTDIR)\jabber_password.obj" \
	"$(INTDIR)\jabber_presence_manager.obj" \
	"$(INTDIR)\jabber_privacy.obj" \
	"$(INTDIR)\jabber_proto.obj" \
	"$(INTDIR)\jabber_proxy.obj" \
	"$(INTDIR)\jabber_rc.obj" \
	"$(INTDIR)\jabber_search.obj" \
	"$(INTDIR)\jabber_secur.obj" \
	"$(INTDIR)\jabber_send_manager.obj" \
	"$(INTDIR)\jabber_std.obj" \
	"$(INTDIR)\jabber_svc.obj" \
	"$(INTDIR)\jabber_thread.obj" \
	"$(INTDIR)\jabber_treelist.obj" \
	"$(INTDIR)\jabber_userinfo.obj" \
	"$(INTDIR)\jabber_util.obj" \
	"$(INTDIR)\jabber_vcard.obj" \
	"$(INTDIR)\jabber_ws.obj" \
	"$(INTDIR)\jabber_xml.obj" \
	"$(INTDIR)\jabber_xstatus.obj" \
	"$(INTDIR)\jabber_zstream.obj" \
	"$(INTDIR)\MString.obj" \
	"$(INTDIR)\ui_utils.obj" \
	"$(INTDIR)\msvc6.res" \
	"$(INTDIR)\jabber_captcha.obj" \
	"..\..\plugins\zlib\Release_Unicode\zlib.lib"

"..\..\bin\Release Unicode\plugins\jabber.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"

OUTDIR=.\./Debug_Unicode
INTDIR=.\./Debug_Unicode
# Begin Custom Macros
OutDir=.\./Debug_Unicode
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "..\..\bin\debug unicode\plugins\jabber.dll" "$(OUTDIR)\jabber.bsc"

!ELSE

ALL : "zlib - Win32 Debug Unicode" "..\..\bin\debug unicode\plugins\jabber.dll" "$(OUTDIR)\jabber.bsc"

!ENDIF

!IF "$(RECURSE)" == "1"
CLEAN :"zlib - Win32 Debug UnicodeCLEAN"
!ELSE
CLEAN :
!ENDIF
	-@erase "$(INTDIR)\jabber.obj"
	-@erase "$(INTDIR)\jabber.pch"
	-@erase "$(INTDIR)\jabber.sbr"
	-@erase "$(INTDIR)\jabber_adhoc.obj"
	-@erase "$(INTDIR)\jabber_adhoc.sbr"
	-@erase "$(INTDIR)\jabber_agent.obj"
	-@erase "$(INTDIR)\jabber_agent.sbr"
	-@erase "$(INTDIR)\jabber_bookmarks.obj"
	-@erase "$(INTDIR)\jabber_bookmarks.sbr"
	-@erase "$(INTDIR)\jabber_byte.obj"
	-@erase "$(INTDIR)\jabber_byte.sbr"
	-@erase "$(INTDIR)\jabber_caps.obj"
	-@erase "$(INTDIR)\jabber_caps.sbr"
	-@erase "$(INTDIR)\jabber_captcha.obj"
	-@erase "$(INTDIR)\jabber_captcha.sbr"
	-@erase "$(INTDIR)\jabber_chat.obj"
	-@erase "$(INTDIR)\jabber_chat.sbr"
	-@erase "$(INTDIR)\jabber_console.obj"
	-@erase "$(INTDIR)\jabber_console.sbr"
	-@erase "$(INTDIR)\jabber_disco.obj"
	-@erase "$(INTDIR)\jabber_disco.sbr"
	-@erase "$(INTDIR)\jabber_events.obj"
	-@erase "$(INTDIR)\jabber_events.sbr"
	-@erase "$(INTDIR)\jabber_file.obj"
	-@erase "$(INTDIR)\jabber_file.sbr"
	-@erase "$(INTDIR)\jabber_form.obj"
	-@erase "$(INTDIR)\jabber_form.sbr"
	-@erase "$(INTDIR)\jabber_ft.obj"
	-@erase "$(INTDIR)\jabber_ft.sbr"
	-@erase "$(INTDIR)\jabber_groupchat.obj"
	-@erase "$(INTDIR)\jabber_groupchat.sbr"
	-@erase "$(INTDIR)\jabber_ibb.obj"
	-@erase "$(INTDIR)\jabber_ibb.sbr"
	-@erase "$(INTDIR)\jabber_icolib.obj"
	-@erase "$(INTDIR)\jabber_icolib.sbr"
	-@erase "$(INTDIR)\jabber_iq.obj"
	-@erase "$(INTDIR)\jabber_iq.sbr"
	-@erase "$(INTDIR)\jabber_iq_handlers.obj"
	-@erase "$(INTDIR)\jabber_iq_handlers.sbr"
	-@erase "$(INTDIR)\jabber_iqid.obj"
	-@erase "$(INTDIR)\jabber_iqid.sbr"
	-@erase "$(INTDIR)\jabber_iqid_muc.obj"
	-@erase "$(INTDIR)\jabber_iqid_muc.sbr"
	-@erase "$(INTDIR)\jabber_libstr.obj"
	-@erase "$(INTDIR)\jabber_libstr.sbr"
	-@erase "$(INTDIR)\jabber_list.obj"
	-@erase "$(INTDIR)\jabber_list.sbr"
	-@erase "$(INTDIR)\jabber_menu.obj"
	-@erase "$(INTDIR)\jabber_menu.sbr"
	-@erase "$(INTDIR)\jabber_message_handlers.obj"
	-@erase "$(INTDIR)\jabber_message_handlers.sbr"
	-@erase "$(INTDIR)\jabber_message_manager.obj"
	-@erase "$(INTDIR)\jabber_message_manager.sbr"
	-@erase "$(INTDIR)\jabber_misc.obj"
	-@erase "$(INTDIR)\jabber_misc.sbr"
	-@erase "$(INTDIR)\jabber_notes.obj"
	-@erase "$(INTDIR)\jabber_notes.sbr"
	-@erase "$(INTDIR)\jabber_opt.obj"
	-@erase "$(INTDIR)\jabber_opt.sbr"
	-@erase "$(INTDIR)\jabber_opttree.obj"
	-@erase "$(INTDIR)\jabber_opttree.sbr"
	-@erase "$(INTDIR)\jabber_password.obj"
	-@erase "$(INTDIR)\jabber_password.sbr"
	-@erase "$(INTDIR)\jabber_presence_manager.obj"
	-@erase "$(INTDIR)\jabber_presence_manager.sbr"
	-@erase "$(INTDIR)\jabber_privacy.obj"
	-@erase "$(INTDIR)\jabber_privacy.sbr"
	-@erase "$(INTDIR)\jabber_proto.obj"
	-@erase "$(INTDIR)\jabber_proto.sbr"
	-@erase "$(INTDIR)\jabber_proxy.obj"
	-@erase "$(INTDIR)\jabber_proxy.sbr"
	-@erase "$(INTDIR)\jabber_rc.obj"
	-@erase "$(INTDIR)\jabber_rc.sbr"
	-@erase "$(INTDIR)\jabber_search.obj"
	-@erase "$(INTDIR)\jabber_search.sbr"
	-@erase "$(INTDIR)\jabber_secur.obj"
	-@erase "$(INTDIR)\jabber_secur.sbr"
	-@erase "$(INTDIR)\jabber_send_manager.obj"
	-@erase "$(INTDIR)\jabber_send_manager.sbr"
	-@erase "$(INTDIR)\jabber_std.obj"
	-@erase "$(INTDIR)\jabber_std.sbr"
	-@erase "$(INTDIR)\jabber_svc.obj"
	-@erase "$(INTDIR)\jabber_svc.sbr"
	-@erase "$(INTDIR)\jabber_thread.obj"
	-@erase "$(INTDIR)\jabber_thread.sbr"
	-@erase "$(INTDIR)\jabber_treelist.obj"
	-@erase "$(INTDIR)\jabber_treelist.sbr"
	-@erase "$(INTDIR)\jabber_userinfo.obj"
	-@erase "$(INTDIR)\jabber_userinfo.sbr"
	-@erase "$(INTDIR)\jabber_util.obj"
	-@erase "$(INTDIR)\jabber_util.sbr"
	-@erase "$(INTDIR)\jabber_vcard.obj"
	-@erase "$(INTDIR)\jabber_vcard.sbr"
	-@erase "$(INTDIR)\jabber_ws.obj"
	-@erase "$(INTDIR)\jabber_ws.sbr"
	-@erase "$(INTDIR)\jabber_xml.obj"
	-@erase "$(INTDIR)\jabber_xml.sbr"
	-@erase "$(INTDIR)\jabber_xstatus.obj"
	-@erase "$(INTDIR)\jabber_xstatus.sbr"
	-@erase "$(INTDIR)\jabber_zstream.obj"
	-@erase "$(INTDIR)\jabber_zstream.sbr"
	-@erase "$(INTDIR)\MString.obj"
	-@erase "$(INTDIR)\MString.sbr"
	-@erase "$(INTDIR)\msvc6.res"
	-@erase "$(INTDIR)\ui_utils.obj"
	-@erase "$(INTDIR)\ui_utils.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\jabber.bsc"
	-@erase "$(OUTDIR)\jabber.exp"
	-@erase "$(OUTDIR)\jabber.map"
	-@erase "$(OUTDIR)\jabber.pdb"
	-@erase "..\..\bin\debug unicode\plugins\jabber.dll"
	-@erase "..\..\bin\debug unicode\plugins\jabber.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yu"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\msvc6.res" /i "../../include" /d "_DEBUG" /d "UNICODE"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\jabber.bsc"
BSC32_SBRS= \
	"$(INTDIR)\jabber.sbr" \
	"$(INTDIR)\jabber_adhoc.sbr" \
	"$(INTDIR)\jabber_agent.sbr" \
	"$(INTDIR)\jabber_bookmarks.sbr" \
	"$(INTDIR)\jabber_byte.sbr" \
	"$(INTDIR)\jabber_caps.sbr" \
	"$(INTDIR)\jabber_chat.sbr" \
	"$(INTDIR)\jabber_console.sbr" \
	"$(INTDIR)\jabber_disco.sbr" \
	"$(INTDIR)\jabber_events.sbr" \
	"$(INTDIR)\jabber_file.sbr" \
	"$(INTDIR)\jabber_form.sbr" \
	"$(INTDIR)\jabber_ft.sbr" \
	"$(INTDIR)\jabber_groupchat.sbr" \
	"$(INTDIR)\jabber_ibb.sbr" \
	"$(INTDIR)\jabber_icolib.sbr" \
	"$(INTDIR)\jabber_iq.sbr" \
	"$(INTDIR)\jabber_iq_handlers.sbr" \
	"$(INTDIR)\jabber_iqid.sbr" \
	"$(INTDIR)\jabber_iqid_muc.sbr" \
	"$(INTDIR)\jabber_libstr.sbr" \
	"$(INTDIR)\jabber_list.sbr" \
	"$(INTDIR)\jabber_menu.sbr" \
	"$(INTDIR)\jabber_message_handlers.sbr" \
	"$(INTDIR)\jabber_message_manager.sbr" \
	"$(INTDIR)\jabber_misc.sbr" \
	"$(INTDIR)\jabber_notes.sbr" \
	"$(INTDIR)\jabber_opt.sbr" \
	"$(INTDIR)\jabber_opttree.sbr" \
	"$(INTDIR)\jabber_password.sbr" \
	"$(INTDIR)\jabber_presence_manager.sbr" \
	"$(INTDIR)\jabber_privacy.sbr" \
	"$(INTDIR)\jabber_proto.sbr" \
	"$(INTDIR)\jabber_proxy.sbr" \
	"$(INTDIR)\jabber_rc.sbr" \
	"$(INTDIR)\jabber_search.sbr" \
	"$(INTDIR)\jabber_secur.sbr" \
	"$(INTDIR)\jabber_send_manager.sbr" \
	"$(INTDIR)\jabber_std.sbr" \
	"$(INTDIR)\jabber_svc.sbr" \
	"$(INTDIR)\jabber_thread.sbr" \
	"$(INTDIR)\jabber_treelist.sbr" \
	"$(INTDIR)\jabber_userinfo.sbr" \
	"$(INTDIR)\jabber_util.sbr" \
	"$(INTDIR)\jabber_vcard.sbr" \
	"$(INTDIR)\jabber_ws.sbr" \
	"$(INTDIR)\jabber_xml.sbr" \
	"$(INTDIR)\jabber_xstatus.sbr" \
	"$(INTDIR)\jabber_zstream.sbr" \
	"$(INTDIR)\MString.sbr" \
	"$(INTDIR)\ui_utils.sbr" \
	"$(INTDIR)\jabber_captcha.sbr"

"$(OUTDIR)\jabber.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib comctl32.lib /nologo /base:"0x32500000" /dll /incremental:yes /pdb:"$(OUTDIR)\jabber.pdb" /map:"$(INTDIR)\jabber.map" /debug /machine:I386 /out:"../../bin/debug unicode/plugins/jabber.dll" /implib:"$(OUTDIR)\jabber.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\jabber.obj" \
	"$(INTDIR)\jabber_adhoc.obj" \
	"$(INTDIR)\jabber_agent.obj" \
	"$(INTDIR)\jabber_bookmarks.obj" \
	"$(INTDIR)\jabber_byte.obj" \
	"$(INTDIR)\jabber_caps.obj" \
	"$(INTDIR)\jabber_chat.obj" \
	"$(INTDIR)\jabber_console.obj" \
	"$(INTDIR)\jabber_disco.obj" \
	"$(INTDIR)\jabber_events.obj" \
	"$(INTDIR)\jabber_file.obj" \
	"$(INTDIR)\jabber_form.obj" \
	"$(INTDIR)\jabber_ft.obj" \
	"$(INTDIR)\jabber_groupchat.obj" \
	"$(INTDIR)\jabber_ibb.obj" \
	"$(INTDIR)\jabber_icolib.obj" \
	"$(INTDIR)\jabber_iq.obj" \
	"$(INTDIR)\jabber_iq_handlers.obj" \
	"$(INTDIR)\jabber_iqid.obj" \
	"$(INTDIR)\jabber_iqid_muc.obj" \
	"$(INTDIR)\jabber_libstr.obj" \
	"$(INTDIR)\jabber_list.obj" \
	"$(INTDIR)\jabber_menu.obj" \
	"$(INTDIR)\jabber_message_handlers.obj" \
	"$(INTDIR)\jabber_message_manager.obj" \
	"$(INTDIR)\jabber_misc.obj" \
	"$(INTDIR)\jabber_notes.obj" \
	"$(INTDIR)\jabber_opt.obj" \
	"$(INTDIR)\jabber_opttree.obj" \
	"$(INTDIR)\jabber_password.obj" \
	"$(INTDIR)\jabber_presence_manager.obj" \
	"$(INTDIR)\jabber_privacy.obj" \
	"$(INTDIR)\jabber_proto.obj" \
	"$(INTDIR)\jabber_proxy.obj" \
	"$(INTDIR)\jabber_rc.obj" \
	"$(INTDIR)\jabber_search.obj" \
	"$(INTDIR)\jabber_secur.obj" \
	"$(INTDIR)\jabber_send_manager.obj" \
	"$(INTDIR)\jabber_std.obj" \
	"$(INTDIR)\jabber_svc.obj" \
	"$(INTDIR)\jabber_thread.obj" \
	"$(INTDIR)\jabber_treelist.obj" \
	"$(INTDIR)\jabber_userinfo.obj" \
	"$(INTDIR)\jabber_util.obj" \
	"$(INTDIR)\jabber_vcard.obj" \
	"$(INTDIR)\jabber_ws.obj" \
	"$(INTDIR)\jabber_xml.obj" \
	"$(INTDIR)\jabber_xstatus.obj" \
	"$(INTDIR)\jabber_zstream.obj" \
	"$(INTDIR)\MString.obj" \
	"$(INTDIR)\ui_utils.obj" \
	"$(INTDIR)\msvc6.res" \
	"$(INTDIR)\jabber_captcha.obj" \
	"..\..\plugins\zlib\Debug_Unicode\zlib.lib"

"..\..\bin\debug unicode\plugins\jabber.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("jabber.dep")
!INCLUDE "jabber.dep"
!ELSE
!MESSAGE Warning: cannot find "jabber.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "jabberg - Win32 Release" || "$(CFG)" == "jabberg - Win32 Debug" || "$(CFG)" == "jabberg - Win32 Release Unicode" || "$(CFG)" == "jabberg - Win32 Debug Unicode"
SOURCE=.\jabber.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yc"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\jabber.obj"	"$(INTDIR)\jabber.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yc"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\jabber.obj"	"$(INTDIR)\jabber.sbr"	"$(INTDIR)\jabber.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yc"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\jabber.obj"	"$(INTDIR)\jabber.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\jabber.pch" /Yc"jabber.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\jabber.obj"	"$(INTDIR)\jabber.sbr"	"$(INTDIR)\jabber.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=.\jabber_adhoc.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_adhoc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_adhoc.obj"	"$(INTDIR)\jabber_adhoc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_adhoc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_adhoc.obj"	"$(INTDIR)\jabber_adhoc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_agent.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_agent.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_agent.obj"	"$(INTDIR)\jabber_agent.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_agent.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_agent.obj"	"$(INTDIR)\jabber_agent.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_bookmarks.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_bookmarks.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_bookmarks.obj"	"$(INTDIR)\jabber_bookmarks.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_bookmarks.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_bookmarks.obj"	"$(INTDIR)\jabber_bookmarks.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_byte.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_byte.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_byte.obj"	"$(INTDIR)\jabber_byte.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_byte.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_byte.obj"	"$(INTDIR)\jabber_byte.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_caps.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_caps.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_caps.obj"	"$(INTDIR)\jabber_caps.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_caps.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_caps.obj"	"$(INTDIR)\jabber_caps.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_captcha.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_captcha.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_captcha.obj"	"$(INTDIR)\jabber_captcha.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_captcha.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_captcha.obj"	"$(INTDIR)\jabber_captcha.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_chat.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_chat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_chat.obj"	"$(INTDIR)\jabber_chat.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_chat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_chat.obj"	"$(INTDIR)\jabber_chat.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_console.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_console.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_console.obj"	"$(INTDIR)\jabber_console.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_console.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_console.obj"	"$(INTDIR)\jabber_console.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_disco.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_disco.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_disco.obj"	"$(INTDIR)\jabber_disco.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_disco.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_disco.obj"	"$(INTDIR)\jabber_disco.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_events.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_events.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_events.obj"	"$(INTDIR)\jabber_events.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_events.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_events.obj"	"$(INTDIR)\jabber_events.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_file.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_file.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_file.obj"	"$(INTDIR)\jabber_file.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_file.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_file.obj"	"$(INTDIR)\jabber_file.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_form.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_form.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_form.obj"	"$(INTDIR)\jabber_form.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_form.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_form.obj"	"$(INTDIR)\jabber_form.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_ft.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_ft.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_ft.obj"	"$(INTDIR)\jabber_ft.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_ft.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_ft.obj"	"$(INTDIR)\jabber_ft.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_groupchat.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_groupchat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_groupchat.obj"	"$(INTDIR)\jabber_groupchat.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_groupchat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_groupchat.obj"	"$(INTDIR)\jabber_groupchat.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_ibb.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_ibb.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_ibb.obj"	"$(INTDIR)\jabber_ibb.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_ibb.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_ibb.obj"	"$(INTDIR)\jabber_ibb.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_icolib.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_icolib.obj"	"$(INTDIR)\jabber_icolib.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_icolib.obj"	"$(INTDIR)\jabber_icolib.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_iq.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_iq.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_iq.obj"	"$(INTDIR)\jabber_iq.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_iq.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_iq.obj"	"$(INTDIR)\jabber_iq.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_iq_handlers.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_iq_handlers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_iq_handlers.obj"	"$(INTDIR)\jabber_iq_handlers.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_iq_handlers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_iq_handlers.obj"	"$(INTDIR)\jabber_iq_handlers.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_iqid.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_iqid.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_iqid.obj"	"$(INTDIR)\jabber_iqid.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_iqid.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_iqid.obj"	"$(INTDIR)\jabber_iqid.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_iqid_muc.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_iqid_muc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_iqid_muc.obj"	"$(INTDIR)\jabber_iqid_muc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_iqid_muc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_iqid_muc.obj"	"$(INTDIR)\jabber_iqid_muc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_libstr.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_libstr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_libstr.obj"	"$(INTDIR)\jabber_libstr.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_libstr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_libstr.obj"	"$(INTDIR)\jabber_libstr.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_list.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_list.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_list.obj"	"$(INTDIR)\jabber_list.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_list.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_list.obj"	"$(INTDIR)\jabber_list.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_menu.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_menu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_menu.obj"	"$(INTDIR)\jabber_menu.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_menu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_menu.obj"	"$(INTDIR)\jabber_menu.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_message_handlers.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_message_handlers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_message_handlers.obj"	"$(INTDIR)\jabber_message_handlers.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_message_handlers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_message_handlers.obj"	"$(INTDIR)\jabber_message_handlers.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_message_manager.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_message_manager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_message_manager.obj"	"$(INTDIR)\jabber_message_manager.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_message_manager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_message_manager.obj"	"$(INTDIR)\jabber_message_manager.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_misc.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_misc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_misc.obj"	"$(INTDIR)\jabber_misc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_misc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_misc.obj"	"$(INTDIR)\jabber_misc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_notes.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_notes.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_notes.obj"	"$(INTDIR)\jabber_notes.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_notes.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_notes.obj"	"$(INTDIR)\jabber_notes.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_opt.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_opt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_opt.obj"	"$(INTDIR)\jabber_opt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_opt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_opt.obj"	"$(INTDIR)\jabber_opt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_opttree.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_opttree.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_opttree.obj"	"$(INTDIR)\jabber_opttree.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_opttree.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_opttree.obj"	"$(INTDIR)\jabber_opttree.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_password.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_password.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_password.obj"	"$(INTDIR)\jabber_password.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_password.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_password.obj"	"$(INTDIR)\jabber_password.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_presence_manager.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_presence_manager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_presence_manager.obj"	"$(INTDIR)\jabber_presence_manager.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_presence_manager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_presence_manager.obj"	"$(INTDIR)\jabber_presence_manager.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_privacy.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_privacy.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_privacy.obj"	"$(INTDIR)\jabber_privacy.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_privacy.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_privacy.obj"	"$(INTDIR)\jabber_privacy.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_proto.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_proto.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_proto.obj"	"$(INTDIR)\jabber_proto.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_proto.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_proto.obj"	"$(INTDIR)\jabber_proto.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_proxy.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_proxy.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_proxy.obj"	"$(INTDIR)\jabber_proxy.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_proxy.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_proxy.obj"	"$(INTDIR)\jabber_proxy.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_rc.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_rc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_rc.obj"	"$(INTDIR)\jabber_rc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_rc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_rc.obj"	"$(INTDIR)\jabber_rc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_search.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_search.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_search.obj"	"$(INTDIR)\jabber_search.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_search.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_search.obj"	"$(INTDIR)\jabber_search.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_secur.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_secur.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_secur.obj"	"$(INTDIR)\jabber_secur.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_secur.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_secur.obj"	"$(INTDIR)\jabber_secur.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_send_manager.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_send_manager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_send_manager.obj"	"$(INTDIR)\jabber_send_manager.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_send_manager.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_send_manager.obj"	"$(INTDIR)\jabber_send_manager.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_std.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_std.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_std.obj"	"$(INTDIR)\jabber_std.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_std.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_std.obj"	"$(INTDIR)\jabber_std.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_svc.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_svc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_svc.obj"	"$(INTDIR)\jabber_svc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_svc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_svc.obj"	"$(INTDIR)\jabber_svc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_thread.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_thread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_thread.obj"	"$(INTDIR)\jabber_thread.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_thread.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_thread.obj"	"$(INTDIR)\jabber_thread.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_treelist.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_treelist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_treelist.obj"	"$(INTDIR)\jabber_treelist.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_treelist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_treelist.obj"	"$(INTDIR)\jabber_treelist.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_userinfo.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_userinfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_userinfo.obj"	"$(INTDIR)\jabber_userinfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_userinfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_userinfo.obj"	"$(INTDIR)\jabber_userinfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_util.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_util.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_util.obj"	"$(INTDIR)\jabber_util.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_util.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_util.obj"	"$(INTDIR)\jabber_util.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_vcard.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_vcard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_vcard.obj"	"$(INTDIR)\jabber_vcard.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_vcard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_vcard.obj"	"$(INTDIR)\jabber_vcard.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_ws.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_ws.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_ws.obj"	"$(INTDIR)\jabber_ws.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_ws.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_ws.obj"	"$(INTDIR)\jabber_ws.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_xml.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_xml.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_xml.obj"	"$(INTDIR)\jabber_xml.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_xml.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_xml.obj"	"$(INTDIR)\jabber_xml.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_xstatus.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_xstatus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_xstatus.obj"	"$(INTDIR)\jabber_xstatus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_xstatus.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_xstatus.obj"	"$(INTDIR)\jabber_xstatus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\jabber_zstream.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\jabber_zstream.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\jabber_zstream.obj"	"$(INTDIR)\jabber_zstream.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\jabber_zstream.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\jabber_zstream.obj"	"$(INTDIR)\jabber_zstream.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\MString.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\MString.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\MString.obj"	"$(INTDIR)\MString.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\MString.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\MString.obj"	"$(INTDIR)\MString.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\ui_utils.cpp

!IF  "$(CFG)" == "jabberg - Win32 Release"


"$(INTDIR)\ui_utils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"


"$(INTDIR)\ui_utils.obj"	"$(INTDIR)\ui_utils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"


"$(INTDIR)\ui_utils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"


"$(INTDIR)\ui_utils.obj"	"$(INTDIR)\ui_utils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\jabber.pch"


!ENDIF

SOURCE=.\msvc6.rc

"$(INTDIR)\msvc6.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!IF  "$(CFG)" == "jabberg - Win32 Release"

"zlib - Win32 Release" :
   cd "..\..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release"
   cd "..\..\protocols\JabberG"

"zlib - Win32 ReleaseCLEAN" :
   cd "..\..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release" RECURSE=1 CLEAN
   cd "..\..\protocols\JabberG"

!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"

"zlib - Win32 Debug" :
   cd "..\..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug"
   cd "..\..\protocols\JabberG"

"zlib - Win32 DebugCLEAN" :
   cd "..\..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug" RECURSE=1 CLEAN
   cd "..\..\protocols\JabberG"

!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"

"zlib - Win32 Release Unicode" :
   cd "..\..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release Unicode"
   cd "..\..\protocols\JabberG"

"zlib - Win32 Release UnicodeCLEAN" :
   cd "..\..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release Unicode" RECURSE=1 CLEAN
   cd "..\..\protocols\JabberG"

!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"

"zlib - Win32 Debug Unicode" :
   cd "..\..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug Unicode"
   cd "..\..\protocols\JabberG"

"zlib - Win32 Debug UnicodeCLEAN" :
   cd "..\..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug Unicode" RECURSE=1 CLEAN
   cd "..\..\protocols\JabberG"

!ENDIF


!ENDIF
