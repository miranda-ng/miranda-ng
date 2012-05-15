# Microsoft Developer Studio Project File - Name="jabberg" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=jabberg - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "jabber.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "jabberg - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Yu"jabber.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib comctl32.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/jabber.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /FR /Yu"jabber.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib comctl32.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/debug/plugins/jabber.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "jabberg - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "jabberg___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "jabberg___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release_Unicode"
# PROP Intermediate_Dir ".\Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Yu"jabber.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /Yu"jabber.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "NDEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/jabber.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib comctl32.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/jabber.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "jabberg - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "jabberg___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "jabberg___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "./Debug_Unicode"
# PROP Intermediate_Dir "./Debug_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /FR /Yu"jabber.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "JABBER_EXPORTS" /FAcs /FR /Yu"jabber.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /i "../../include" /d "_DEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /incremental:no /map /debug /machine:I386 /out:"../../bin/debug/plugins/jabber.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib comctl32.lib /nologo /base:"0x32500000" /dll /map /debug /machine:I386 /out:"../../bin/debug unicode/plugins/jabber.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "jabberg - Win32 Release"
# Name "jabberg - Win32 Debug"
# Name "jabberg - Win32 Release Unicode"
# Name "jabberg - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\jabber.cpp
# ADD CPP /Yc"jabber.h"
# End Source File
# Begin Source File

SOURCE=.\jabber_adhoc.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_agent.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_bookmarks.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_byte.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_caps.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_captcha.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_chat.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_console.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_disco.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_events.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_file.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_form.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_ft.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_groupchat.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_ibb.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_icolib.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_iq.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_iq_handlers.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_iqid.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_iqid_muc.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_libstr.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_list.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_message_handlers.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_message_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_misc.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_notes.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_opt.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_opttree.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_password.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_presence_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_privacy.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_proto.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_proxy.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_rc.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_search.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_secur.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_send_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_std.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_svc.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_thread.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_treelist.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_userinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_util.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_vcard.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_ws.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_xml.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_xstatus.cpp
# End Source File
# Begin Source File

SOURCE=.\jabber_zstream.cpp
# End Source File
# Begin Source File

SOURCE=.\MString.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\jabber.h
# End Source File
# Begin Source File

SOURCE=.\jabber_byte.h
# End Source File
# Begin Source File

SOURCE=.\jabber_caps.h
# End Source File
# Begin Source File

SOURCE=.\jabber_disco.h
# End Source File
# Begin Source File

SOURCE=.\jabber_ibb.h
# End Source File
# Begin Source File

SOURCE=.\jabber_iq.h
# End Source File
# Begin Source File

SOURCE=.\jabber_iq_handlers.h
# End Source File
# Begin Source File

SOURCE=.\jabber_list.h
# End Source File
# Begin Source File

SOURCE=.\jabber_notes.h
# End Source File
# Begin Source File

SOURCE=.\jabber_opttree.h
# End Source File
# Begin Source File

SOURCE=.\jabber_privacy.h
# End Source File
# Begin Source File

SOURCE=.\jabber_proto.h
# End Source File
# Begin Source File

SOURCE=.\jabber_proxy.h
# End Source File
# Begin Source File

SOURCE=.\jabber_rc.h
# End Source File
# Begin Source File

SOURCE=.\jabber_search.h
# End Source File
# Begin Source File

SOURCE=.\jabber_secur.h
# End Source File
# Begin Source File

SOURCE=.\jabber_xml.h
# End Source File
# Begin Source File

SOURCE=.\MString.h
# End Source File
# Begin Source File

SOURCE=.\ui_utils.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icos\add2roster.ico
# End Source File
# Begin Source File

SOURCE=.\icos\addcontact.ico
# End Source File
# Begin Source File

SOURCE=.\icos\arrow_down.ico
# End Source File
# Begin Source File

SOURCE=.\icos\arrow_up.ico
# End Source File
# Begin Source File

SOURCE=.\icos\auth_revoke.ico
# End Source File
# Begin Source File

SOURCE=.\icos\block.ico
# End Source File
# Begin Source File

SOURCE=.\icos\bookmarks.ico
# End Source File
# Begin Source File

SOURCE=.\icos\command.ico
# End Source File
# Begin Source File

SOURCE=.\icos\console.ico
# End Source File
# Begin Source File

SOURCE=.\icos\delete.ico
# End Source File
# Begin Source File

SOURCE=.\icos\disco_fail.ico
# End Source File
# Begin Source File

SOURCE=.\icos\disco_in_progress.ico
# End Source File
# Begin Source File

SOURCE=.\icos\disco_ok.ico
# End Source File
# Begin Source File

SOURCE=.\icos\filter.ico
# End Source File
# Begin Source File

SOURCE=.\icos\go.ico
# End Source File
# Begin Source File

SOURCE=.\icos\grant.ico
# End Source File
# Begin Source File

SOURCE=.\icos\group.ico
# End Source File
# Begin Source File

SOURCE=.\icos\home.ico
# End Source File
# Begin Source File

SOURCE=.\icos\jabber.ico
# End Source File
# Begin Source File

SOURCE=.\icos\key.ico
# End Source File
# Begin Source File

SOURCE=.\icos\login.ico
# End Source File
# Begin Source File

SOURCE=.\icos\message_allow.ico
# End Source File
# Begin Source File

SOURCE=.\icos\message_deny.ico
# End Source File
# Begin Source File

SOURCE=.\msvc6.rc
# End Source File
# Begin Source File

SOURCE=.\icos\notes.ico
# End Source File
# Begin Source File

SOURCE=.\icos\open.ico
# End Source File
# Begin Source File

SOURCE=.\icos\openid.ico
# End Source File
# Begin Source File

SOURCE=.\icos\pages.ico
# End Source File
# Begin Source File

SOURCE=.\icos\plist_active.ico
# End Source File
# Begin Source File

SOURCE=.\icos\plist_any.ico
# End Source File
# Begin Source File

SOURCE=.\icos\plist_default.ico
# End Source File
# Begin Source File

SOURCE=.\icos\presence_in_allow.ico
# End Source File
# Begin Source File

SOURCE=.\icos\presence_in_deny.ico
# End Source File
# Begin Source File

SOURCE=.\icos\presence_out_allow.ico
# End Source File
# Begin Source File

SOURCE=.\icos\presence_out_deny.ico
# End Source File
# Begin Source File

SOURCE=.\icos\privacy_lists.ico
# End Source File
# Begin Source File

SOURCE=.\icos\query_allow.ico
# End Source File
# Begin Source File

SOURCE=.\icos\query_deny.ico
# End Source File
# Begin Source File

SOURCE=.\icos\refresh.ico
# End Source File
# Begin Source File

SOURCE=.\icos\refresh_node.ico
# End Source File
# Begin Source File

SOURCE=.\icos\rename.ico
# End Source File
# Begin Source File

SOURCE=.\icos\request.ico
# End Source File
# Begin Source File

SOURCE=.\icos\reset_filter.ico
# End Source File
# Begin Source File

SOURCE=.\icos\roster.ico
# End Source File
# Begin Source File

SOURCE=.\icos\rss.ico
# End Source File
# Begin Source File

SOURCE=.\icos\save.ico
# End Source File
# Begin Source File

SOURCE=.\icos\send_note.ico
# End Source File
# Begin Source File

SOURCE=.\icos\server.ico
# End Source File
# Begin Source File

SOURCE=.\icos\service_discovery.ico
# End Source File
# Begin Source File

SOURCE=.\icos\store.ico
# End Source File
# Begin Source File

SOURCE=.\icos\transport.ico
# End Source File
# Begin Source File

SOURCE=.\icos\transport_local.ico
# End Source File
# Begin Source File

SOURCE=.\icos\user2room.ico
# End Source File
# Begin Source File

SOURCE=.\icos\view_as_list.ico
# End Source File
# Begin Source File

SOURCE=.\icos\view_as_tree.ico
# End Source File
# Begin Source File

SOURCE=.\icos\weather.ico
# End Source File
# Begin Source File

SOURCE=.\icos\write.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\docs\changelog_jabber.txt
# End Source File
# Begin Source File

SOURCE=.\docs\readme_jabber.txt
# End Source File
# Begin Source File

SOURCE=.\docs\translation_jabber.txt
# End Source File
# End Target
# End Project
