# Microsoft Developer Studio Project File - Name="modernb" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=modernb - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "modernb.mak".
!MESSAGE 
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

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "modernb - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Yu"hdr/modern_commonheaders.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib delayimp.lib gdiplus.lib msimg32.lib shlwapi.lib /nologo /base:"0x6590000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/clist_modern.dll" /delayload:gdiplus.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "modernb - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Yu"hdr/modern_commonheaders.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib delayimp.lib gdiplus.lib msimg32.lib shlwapi.lib /nologo /base:"0x6590000" /dll /debug /machine:I386 /out:"../../bin/debug/plugins/clist_modern.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "modernb - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "modernb___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "modernb___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "./Release_Unicode"
# PROP Intermediate_Dir "./Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Yu"commonheaders.h" /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Yu"hdr/modern_commonheaders.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib /nologo /base:"0x6590000" /dll /machine:I386 /out:"../../bin/release/plugins/clist_mw.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib delayimp.lib gdiplus.lib msimg32.lib shlwapi.lib /nologo /base:"0x6590000" /dll /map /debug /machine:I386 /out:"../../bin/release Unicode/plugins/clist_modern.dll" /delayload:gdiplus.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "modernb - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "modernb___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "modernb___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "./Debug_Unicode"
# PROP Intermediate_Dir "./Debug_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernb_EXPORTS" /Yu"commonheaders.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernb_EXPORTS" /Yu"hdr/modern_commonheaders.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib /nologo /base:"0x6590000" /dll /debug /machine:I386 /out:"../../bin/debug/plugins/clist_mw.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib shell32.lib ole32.lib comdlg32.lib delayimp.lib gdiplus.lib msimg32.lib shlwapi.lib /nologo /base:"0x6590000" /dll /debug /machine:I386 /out:"../../bin/debug Unicode/plugins/clist_modern.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "modernb - Win32 Release"
# Name "modernb - Win32 Debug"
# Name "modernb - Win32 Release Unicode"
# Name "modernb - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\init.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_aniavatars.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_awaymsg.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_cachefuncs.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clc.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_clcidents.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clcitems.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clcmsgs.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clcopts.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clcpaint.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_clcutils.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clistevents.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clistmenus.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clistmod.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clistopts.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clistsettings.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clisttray.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_clui.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_cluiframes.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_cluiservices.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_commonheaders.cpp
# ADD CPP /Yc"hdr/modern_commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\modern_contact.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_docking.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_extraimage.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_framesmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_gdiplus.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_gettextasync.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_global.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_groupmenu.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_image_array.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_keyboard.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_log.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_newrowopts.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_popup.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_row.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_rowheight_funcs.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_rowtemplateopt.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_skinbutton.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_skineditor.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_skinengine.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_skinopt.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_skinselector.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_statusbar.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_statusbar_options.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_sync.cpp
# End Source File
# Begin Source File

SOURCE=.\modern_tbbutton.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_toolbar.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_viewmodebar.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\modern_xptheme.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\hdr\modern_awaymsg.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_cache_funcs.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_clc.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_clist.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_clui.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_cluiframes.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_commonprototypes.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_defsettings.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_effectenum.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_gettextasync.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_global_structure.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_image_array.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_log.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_popup.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_row.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_rowheight_funcs.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_skinengine.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_skinselector.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_statusbar.h
# End Source File
# Begin Source File

SOURCE=.\hdr\modern_statusmodes.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=".\res\&RQ.ico"
# End Source File
# Begin Source File

SOURCE=.\res\1.ico
# End Source File
# Begin Source File

SOURCE=.\res\2.ico
# End Source File
# Begin Source File

SOURCE=.\res\3.ico
# End Source File
# Begin Source File

SOURCE=.\res\4.ico
# End Source File
# Begin Source File

SOURCE=.\res\5.ico
# End Source File
# Begin Source File

SOURCE=.\res\6.ico
# End Source File
# Begin Source File

SOURCE=.\res\7.ico
# End Source File
# Begin Source File

SOURCE=.\res\8.ico
# End Source File
# Begin Source File

SOURCE=.\res\addcontact.ico
# End Source File
# Begin Source File

SOURCE=.\res\addgoupp.ico
# End Source File
# Begin Source File

SOURCE=.\res\AlwaysVis.ico
# End Source File
# Begin Source File

SOURCE=.\res\away.ico
# End Source File
# Begin Source File

SOURCE=.\res\blank.ico
# End Source File
# Begin Source File

SOURCE=.\res\blank1.ico
# End Source File
# Begin Source File

SOURCE=.\res\changefont.ico
# End Source File
# Begin Source File

SOURCE=.\res\chat.ico
# End Source File
# Begin Source File

SOURCE=.\res\Chatchannel.ico
# End Source File
# Begin Source File

SOURCE=.\res\delete.ico
# End Source File
# Begin Source File

SOURCE=.\res\detailsl.ico
# End Source File
# Begin Source File

SOURCE=.\res\dnd.ico
# End Source File
# Begin Source File

SOURCE=.\res\downarrow.ico
# End Source File
# Begin Source File

SOURCE=.\res\dragcopy.cur
# End Source File
# Begin Source File

SOURCE=.\res\dropuser.cur
# End Source File
# Begin Source File

SOURCE=.\res\emptyblo.ico
# End Source File
# Begin Source File

SOURCE=.\res\file.ico
# End Source File
# Begin Source File

SOURCE=.\res\filledbl.ico
# End Source File
# Begin Source File

SOURCE=.\res\finduser.ico
# End Source File
# Begin Source File

SOURCE=.\res\freechat.ico
# End Source File
# Begin Source File

SOURCE=.\res\Gaim.ico
# End Source File
# Begin Source File

SOURCE=.\res\globus.ico
# End Source File
# Begin Source File

SOURCE=.\res\groupope.ico
# End Source File
# Begin Source File

SOURCE=.\res\groupshu.ico
# End Source File
# Begin Source File

SOURCE=.\res\help.ico
# End Source File
# Begin Source File

SOURCE=.\res\hide_avatar.ico
# End Source File
# Begin Source File

SOURCE=.\res\history.ico
# End Source File
# Begin Source File

SOURCE=.\res\hyperlin.cur
# End Source File
# Begin Source File

SOURCE=.\res\Icq2000.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icq2001.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icq2003.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icq2gof.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icq2goJ.ico
# End Source File
# Begin Source File

SOURCE=.\res\IcqL4.ico
# End Source File
# Begin Source File

SOURCE=.\res\IcqL5.ico
# End Source File
# Begin Source File

SOURCE=.\res\IcqLite.ico
# End Source File
# Begin Source File

SOURCE=.\res\IM2.ico
# End Source File
# Begin Source File

SOURCE=.\res\invisible.ico
# End Source File
# Begin Source File

SOURCE=.\res\Kopete.ico
# End Source File
# Begin Source File

SOURCE=.\res\Licq.ico
# End Source File
# Begin Source File

SOURCE=.\res\listening_to.ico
# End Source File
# Begin Source File

SOURCE=.\res\lunch.ico
# End Source File
# Begin Source File

SOURCE=.\res\message.ico
# End Source File
# Begin Source File

SOURCE=.\res\MicroAway.ico
# End Source File
# Begin Source File

SOURCE=.\res\MicroChat.ico
# End Source File
# Begin Source File

SOURCE=.\res\MicroDnd.ico
# End Source File
# Begin Source File

SOURCE=.\res\MicroInvisible.ico
# End Source File
# Begin Source File

SOURCE=.\res\MicroLunch.ico
# End Source File
# Begin Source File

SOURCE=.\res\MicroNa.ico
# End Source File
# Begin Source File

SOURCE=.\res\MicroOccupied.ico
# End Source File
# Begin Source File

SOURCE=.\res\MicroOffline.ico
# End Source File
# Begin Source File

SOURCE=.\res\MicroPhone.ico
# End Source File
# Begin Source File

SOURCE=.\res\miranda.ico
# End Source File
# Begin Source File

SOURCE=.\res\mirandaclassic.ico
# End Source File
# Begin Source File

SOURCE=.\res\mirandaw.ico
# End Source File
# Begin Source File

SOURCE=.\msvc6.rc
# End Source File
# Begin Source File

SOURCE=.\res\multisend.ico
# End Source File
# Begin Source File

SOURCE=.\res\NA.ico
# End Source File
# Begin Source File

SOURCE=.\res\na2.ico
# End Source File
# Begin Source File

SOURCE=.\res\neo_meta_create.cur
# End Source File
# Begin Source File

SOURCE=.\res\neo_meta_default.cur
# End Source File
# Begin Source File

SOURCE=.\res\neo_meta_move.cur
# End Source File
# Begin Source File

SOURCE=.\res\NeverVis.ico
# End Source File
# Begin Source File

SOURCE=.\res\occupied.ico
# End Source File
# Begin Source File

SOURCE=.\res\offline.ico
# End Source File
# Begin Source File

SOURCE=.\res\offline2.ico
# End Source File
# Begin Source File

SOURCE=.\res\online2.ico
# End Source File
# Begin Source File

SOURCE=.\res\onthepho.ico
# End Source File
# Begin Source File

SOURCE=.\res\options.ico
# End Source File
# Begin Source File

SOURCE=.\res\outtolun.ico
# End Source File
# Begin Source File

SOURCE=.\res\phone.ico
# End Source File
# Begin Source File

SOURCE=.\res\QIP.ico
# End Source File
# Begin Source File

SOURCE=.\res\rate_high.ico
# End Source File
# Begin Source File

SOURCE=.\res\rate_low.ico
# End Source File
# Begin Source File

SOURCE=.\res\rate_med.ico
# End Source File
# Begin Source File

SOURCE=.\res\rate_none.ico
# End Source File
# Begin Source File

SOURCE=.\res\rename.ico
# End Source File
# Begin Source File

SOURCE=.\res\reply.ico
# End Source File
# Begin Source File

SOURCE=.\res\reset_view.ico
# End Source File
# Begin Source File

SOURCE=.\res\searchal.ico
# End Source File
# Begin Source File

SOURCE=.\res\sendmail.ico
# End Source File
# Begin Source File

SOURCE=.\res\set_view.ico
# End Source File
# Begin Source File

SOURCE=.\res\show_avatar.ico
# End Source File
# Begin Source File

SOURCE=.\res\Sim.ico
# End Source File
# Begin Source File

SOURCE=.\res\smalldot.ico
# End Source File
# Begin Source File

SOURCE=.\res\sms.ico
# End Source File
# Begin Source File

SOURCE=.\res\sortcold.bmp
# End Source File
# Begin Source File

SOURCE=.\res\sortcolu.bmp
# End Source File
# Begin Source File

SOURCE=.\res\timestamp.ico
# End Source File
# Begin Source File

SOURCE=.\res\Trillian.ico
# End Source File
# Begin Source File

SOURCE=.\res\url.ico
# End Source File
# Begin Source File

SOURCE=.\res\useronli.ico
# End Source File
# Begin Source File

SOURCE=.\res\viewdetails.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\res\back.tga
# End Source File
# Begin Source File

SOURCE=.\res\skin.msf
# End Source File
# End Target
# End Project
