# Microsoft Developer Studio Project File - Name="tabSRMM" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=tabSRMM - Win32 Release Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tabsrmm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tabsrmm.mak" CFG="tabSRMM - Win32 Release Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tabSRMM - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tabSRMM - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release_Unicode"
# PROP BASE Intermediate_Dir ".\Release_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release_Unicode"
# PROP Intermediate_Dir ".\Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /D "_MBCS" /Fp".\Release_Unicode/srmm.pch" /YX /GF /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /Fp".\Release_Unicode/srmm.pch" /Yu"commonheaders.h" /GF /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG" /d "UNICODE"
# ADD RSC /l 0x809 /fo".\tabsrmm_private.res" /d "NDEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x6a540000" /subsystem:windows /dll /machine:IX86 /out:"..\..\Bin\Release\Plugins\tabsrmm_unicode.dll" /implib:".\Release_Unicode/srmm.lib" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msimg32.lib shlwapi.lib /nologo /base:"0x6a540000" /subsystem:windows /dll /map /debug /machine:IX86 /out:"..\..\Bin\Release Unicode\Plugins\tabsrmm.dll" /implib:".\Release_Unicode/srmm.lib" /opt:NOWIN98
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug_Unicode"
# PROP BASE Intermediate_Dir ".\Debug_Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug_Unicode"
# PROP Intermediate_Dir ".\Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /D "_MBCS" /FR /Fp".\Debug_Unicode/srmm.pch" /YX /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "UNICODE" /GZ /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:".\Debug_Unicode\srmm.pdb" /debug /machine:IX86 /out:"..\..\Bin\Debug\Plugins\tabsrmm_unicode.dll" /implib:".\Debug_Unicode/srmm.lib" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib comctl32.lib msimg32.lib shlwapi.lib /nologo /base:"0x6a540000" /stack:0xfa00 /subsystem:windows /dll /incremental:no /map /debug /machine:IX86 /out:"..\..\Bin\Debug Unicode\Plugins\tabsrmm.dll" /implib:".\Debug_Unicode/srmm.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "tabSRMM - Win32 Release Unicode"
# Name "tabSRMM - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Chat"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\chat\chat.h
# End Source File
# Begin Source File

SOURCE=.\chat\chat_resource.h
# End Source File
# Begin Source File

SOURCE=.\chat\chatprototypes.h
# End Source File
# Begin Source File

SOURCE=.\chat\clist.cpp
DEP_CPP_CLIST=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\colorchooser.cpp
DEP_CPP_COLOR=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\log.cpp
DEP_CPP_LOG_C=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\main.cpp
DEP_CPP_MAIN_=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\manager.cpp
DEP_CPP_MANAG=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\message.cpp
DEP_CPP_MESSA=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\muchighlight.cpp
DEP_CPP_MUCHI=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	

!IF  "$(CFG)" == "tabSRMM - Win32 Release Unicode"

# ADD CPP /O2
# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "tabSRMM - Win32 Debug Unicode"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\chat\muchighlight.h
# End Source File
# Begin Source File

SOURCE=.\chat\options.cpp
DEP_CPP_OPTIO=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\services.cpp
DEP_CPP_SERVI=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\tools.cpp
DEP_CPP_TOOLS=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\chat\window.cpp
DEP_CPP_WINDO=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"tom.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# End Group
# Begin Group "API"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\API\m_buttonbar.h
# End Source File
# Begin Source File

SOURCE=.\API\m_cln_skinedit.h
# End Source File
# Begin Source File

SOURCE=.\API\m_fingerprint.h
# End Source File
# Begin Source File

SOURCE=.\API\m_flash.h
# End Source File
# Begin Source File

SOURCE=.\API\m_folders.h
# End Source File
# Begin Source File

SOURCE=.\API\m_historyevents.h
# End Source File
# Begin Source File

SOURCE=.\API\m_ieview.h
# End Source File
# Begin Source File

SOURCE=.\API\m_mathmodule.h
# End Source File
# Begin Source File

SOURCE=.\API\m_metacontacts.h
# End Source File
# Begin Source File

SOURCE=.\API\m_msg_buttonsbar.h
# End Source File
# Begin Source File

SOURCE=.\API\m_nudge.h
# End Source File
# Begin Source File

SOURCE=.\API\m_popup2.h
# End Source File
# Begin Source File

SOURCE=.\API\m_smileyadd.h
# End Source File
# Begin Source File

SOURCE=.\API\m_spellchecker.h
# End Source File
# Begin Source File

SOURCE=.\API\m_timezones.h
# End Source File
# Begin Source File

SOURCE=.\API\m_updater.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\buttonsbar.cpp
DEP_CPP_BUTTO=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\contactcache.cpp
DEP_CPP_CONTA=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\container.cpp
DEP_CPP_CONTAI=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\containeroptions.cpp
DEP_CPP_CONTAIN=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\controls.cpp
DEP_CPP_CONTR=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\eventpopups.cpp
DEP_CPP_EVENT=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\generic_msghandlers.cpp
DEP_CPP_GENER=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\globals.cpp
DEP_CPP_GLOBA=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\hotkeyhandler.cpp
DEP_CPP_HOTKE=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\ImageDataObject.cpp
DEP_CPP_IMAGE=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\ImageDataObject.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\src\infopanel.cpp
DEP_CPP_INFOP=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\mim.cpp
DEP_CPP_MIM_C=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tabmodplus\modplus.cpp
DEP_CPP_MODPL=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\src\msgdialog.cpp
DEP_CPP_MSGDI=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\msgdlgutils.cpp
DEP_CPP_MSGDL=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\msglog.cpp
DEP_CPP_MSGLO=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\msgoptions.cpp
DEP_CPP_MSGOP=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_modernopt.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tabmodplus\msgoptions_plus.cpp
DEP_CPP_MSGOPT=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yu"../src/commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\src\msgs.cpp
DEP_CPP_MSGS_=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\selectcontainer.cpp
DEP_CPP_SELEC=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\sendlater.cpp
DEP_CPP_SENDL=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\sendqueue.cpp
DEP_CPP_SENDQ=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\sidebar.cpp
DEP_CPP_SIDEB=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\srmm.cpp
DEP_CPP_SRMM_=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# ADD CPP /Yc"commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\src\tabctrl.cpp
DEP_CPP_TABCT=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\taskbar.cpp
DEP_CPP_TASKB=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\templates.cpp
DEP_CPP_TEMPL=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\themeio.cpp
DEP_CPP_THEME=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\themes.cpp
DEP_CPP_THEMES=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\translator.cpp
DEP_CPP_TRANS=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\trayicon.cpp
DEP_CPP_TRAYI=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\TSButton.cpp
DEP_CPP_TSBUT=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\typingnotify.cpp
DEP_CPP_TYPIN=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\userprefs.cpp
DEP_CPP_USERP=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# Begin Source File

SOURCE=.\src\utils.cpp
DEP_CPP_UTILS=\
	"..\..\include\m_acc.h"\
	"..\..\include\m_addcontact.h"\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_chat.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_file.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_freeimage.h"\
	"..\..\include\m_genmenu.h"\
	"..\..\include\m_history.h"\
	"..\..\include\m_hotkeys.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\m_stdhdr.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_userinfo.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\msapi\vsstyle.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	"..\..\include\win2k.h"\
	".\API\m_buttonbar.h"\
	".\API\m_cln_skinedit.h"\
	".\API\m_fingerprint.h"\
	".\API\m_flash.h"\
	".\API\m_folders.h"\
	".\API\m_historyevents.h"\
	".\API\m_ieview.h"\
	".\API\m_mathmodule.h"\
	".\API\m_metacontacts.h"\
	".\API\m_msg_buttonsbar.h"\
	".\API\m_nudge.h"\
	".\API\m_popup2.h"\
	".\API\m_smileyadd.h"\
	".\API\m_spellchecker.h"\
	".\API\m_timezones.h"\
	".\API\m_updater.h"\
	".\chat\chat.h"\
	".\chat\chatprototypes.h"\
	".\chat\muchighlight.h"\
	".\include\contactcache.h"\
	".\include\controls.h"\
	".\include\functions.h"\
	".\include\generic_msghandlers.h"\
	".\include\globals.h"\
	".\include\infopanel.h"\
	".\include\mim.h"\
	".\include\msgdlgutils.h"\
	".\include\msgs.h"\
	".\include\nen.h"\
	".\include\sendlater.h"\
	".\include\sendqueue.h"\
	".\include\sidebar.h"\
	".\include\taskbar.h"\
	".\include\templates.h"\
	".\include\themes.h"\
	".\include\translator.h"\
	".\include\typingnotify.h"\
	".\include\utils.h"\
	".\include\version.h"\
	".\src\commonheaders.h"\
	{$(INCLUDE)}"uxtheme.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\include\contactcache.h
# End Source File
# Begin Source File

SOURCE=.\include\controls.h
# End Source File
# Begin Source File

SOURCE=.\include\functions.h
# End Source File
# Begin Source File

SOURCE=.\include\generic_msghandlers.h
# End Source File
# Begin Source File

SOURCE=.\include\globals.h
# End Source File
# Begin Source File

SOURCE=.\include\ImageDataObject.h
# End Source File
# Begin Source File

SOURCE=.\include\infopanel.h
# End Source File
# Begin Source File

SOURCE=.\include\mim.h
# End Source File
# Begin Source File

SOURCE=.\include\msgdlgutils.h
# End Source File
# Begin Source File

SOURCE=.\include\msgs.h
# End Source File
# Begin Source File

SOURCE=.\include\nen.h
# End Source File
# Begin Source File

SOURCE=.\include\resource.h
# End Source File
# Begin Source File

SOURCE=.\include\sendlater.h
# End Source File
# Begin Source File

SOURCE=.\include\sendqueue.h
# End Source File
# Begin Source File

SOURCE=.\include\sidebar.h
# End Source File
# Begin Source File

SOURCE=.\include\taskbar.h
# End Source File
# Begin Source File

SOURCE=.\include\templates.h
# End Source File
# Begin Source File

SOURCE=.\include\themes.h
# End Source File
# Begin Source File

SOURCE=.\include\translator.h
# End Source File
# Begin Source File

SOURCE=.\include\typingnotify.h
# End Source File
# Begin Source File

SOURCE=.\include\utils.h
# End Source File
# Begin Source File

SOURCE=.\include\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\tabsrmm_private.rc
# End Source File
# End Group
# Begin Group "Misc Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\docs\changelog.txt
# End Source File
# Begin Source File

SOURCE=langpacks\langpack_tabsrmm_german.txt
# End Source File
# Begin Source File

SOURCE=.\Makefile.ansi
# End Source File
# Begin Source File

SOURCE=MAKEFILE.W32
# End Source File
# Begin Source File

SOURCE=.\docs\MetaContacts.TXT
# End Source File
# Begin Source File

SOURCE=.\docs\Popups.txt
# End Source File
# Begin Source File

SOURCE=.\docs\Readme.icons
# End Source File
# Begin Source File

SOURCE=.\docs\readme.txt
# End Source File
# End Group
# End Target
# End Project
