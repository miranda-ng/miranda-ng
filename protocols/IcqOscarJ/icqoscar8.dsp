# Microsoft Developer Studio Project File - Name="icqoscar8" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=icqoscar8 - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "icqoscar8.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "icqoscar8.mak" CFG="icqoscar8 - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "icqoscar8 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "icqoscar8 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "icqoscar8 - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "icqoscar8 - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "icqoscar8 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "icqoscar8_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "icqoscar8_EXPORTS" /FR /Yu"icqoscar.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x417 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/ICQ.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "icqoscar8 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "icqoscar8_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "icqoscar8_EXPORTS" /FR /Yu"icqoscar.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x25000000" /dll /map /debug /debugtype:both /machine:I386 /out:"../../bin/debug/plugins/ICQ.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "icqoscar8 - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "icqoscar8___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "icqoscar8___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "icqoscar8_EXPORTS" /FR /Yu"icqoscar.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "icqoscar8_EXPORTS" /FR /Yu"icqoscar.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x25000000" /dll /map /debug /debugtype:both /machine:I386 /out:"../../bin/debug/plugins/ICQ.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /incremental:no
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x25000000" /dll /map /debug /debugtype:both /machine:I386 /out:"../../bin/debug Unicode/plugins/ICQ.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "icqoscar8 - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "icqoscar8___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "icqoscar8___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "icqoscar8_EXPORTS" /FR /Yu"icqoscar.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "icqoscar8_EXPORTS" /FR /Yu"icqoscar.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/ICQ.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comctl32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/release unicode/plugins/ICQ.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "icqoscar8 - Win32 Release"
# Name "icqoscar8 - Win32 Debug"
# Name "icqoscar8 - Win32 Debug Unicode"
# Name "icqoscar8 - Win32 Release Unicode"
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\UI\askauthentication.h
# End Source File
# Begin Source File

SOURCE=.\capabilities.h
# End Source File
# Begin Source File

SOURCE=.\channels.h
# End Source File
# Begin Source File

SOURCE=.\cookies.h
# End Source File
# Begin Source File

SOURCE=.\directpackets.h
# End Source File
# Begin Source File

SOURCE=.\families.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\guids.h
# End Source File
# Begin Source File

SOURCE=.\i18n.h
# End Source File
# Begin Source File

SOURCE=.\iconlib.h
# End Source File
# Begin Source File

SOURCE=.\icq_advsearch.h
# End Source File
# Begin Source File

SOURCE=.\icq_avatar.h
# End Source File
# Begin Source File

SOURCE=.\icq_constants.h
# End Source File
# Begin Source File

SOURCE=.\icq_db.h
# End Source File
# Begin Source File

SOURCE=.\icq_direct.h
# End Source File
# Begin Source File

SOURCE=.\icq_fieldnames.h
# End Source File
# Begin Source File

SOURCE=.\icq_http.h
# End Source File
# Begin Source File

SOURCE=.\icq_infoupdate.h
# End Source File
# Begin Source File

SOURCE=.\icq_opts.h
# End Source File
# Begin Source File

SOURCE=.\icq_packet.h
# End Source File
# Begin Source File

SOURCE=.\icq_popups.h
# End Source File
# Begin Source File

SOURCE=.\icq_proto.h
# End Source File
# Begin Source File

SOURCE=.\icq_rates.h
# End Source File
# Begin Source File

SOURCE=.\icq_server.h
# End Source File
# Begin Source File

SOURCE=.\icq_servlist.h
# End Source File
# Begin Source File

SOURCE=.\icq_uploadui.h
# End Source File
# Begin Source File

SOURCE=.\icq_xtraz.h
# End Source File
# Begin Source File

SOURCE=.\icqosc_svcs.h
# End Source File
# Begin Source File

SOURCE=.\icqoscar.h
# End Source File
# Begin Source File

SOURCE=.\init.h
# End Source File
# Begin Source File

SOURCE=.\log.h
# End Source File
# Begin Source File

SOURCE=.\UI\loginpassword.h
# End Source File
# Begin Source File

SOURCE=.\oscar_filetransfer.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\stdpackets.h
# End Source File
# Begin Source File

SOURCE=.\tlv.h
# End Source File
# Begin Source File

SOURCE=.\UI\userinfotab.h
# End Source File
# Begin Source File

SOURCE=.\utilities.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icos\auth_ask.ico
# End Source File
# Begin Source File

SOURCE=.\icos\auth_grant.ico
# End Source File
# Begin Source File

SOURCE=.\icos\auth_revoke.ico
# End Source File
# Begin Source File

SOURCE=.\changeinfo\expandst.ico
# End Source File
# Begin Source File

SOURCE=.\icos\icq.ico
# End Source File
# Begin Source File

SOURCE=.\resources.rc
# End Source File
# Begin Source File

SOURCE=.\icos\srvlist_add.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus.bmp
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus01.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus02.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus03.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus04.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus05.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus06.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus07.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus08.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus09.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus10.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus11.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus12.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus13.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus14.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus15.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus16.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus17.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus18.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus19.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus20.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus21.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus22.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus23.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus24.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus25.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus26.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus27.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus28.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus29.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus30.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus31.ico
# End Source File
# Begin Source File

SOURCE=.\icos\xstatus32.ico
# End Source File
# End Group
# Begin Group "FLAP Channels"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\chan_01login.cpp
# End Source File
# Begin Source File

SOURCE=.\chan_02data.cpp
# End Source File
# Begin Source File

SOURCE=.\chan_03error.cpp
# End Source File
# Begin Source File

SOURCE=.\chan_04close.cpp
# End Source File
# Begin Source File

SOURCE=.\chan_05ping.cpp
# End Source File
# End Group
# Begin Group "SNAC Families"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\fam_01service.cpp
# End Source File
# Begin Source File

SOURCE=.\fam_02location.cpp
# End Source File
# Begin Source File

SOURCE=.\fam_03buddy.cpp
# End Source File
# Begin Source File

SOURCE=.\fam_04message.cpp
# End Source File
# Begin Source File

SOURCE=.\fam_09bos.cpp
# End Source File
# Begin Source File

SOURCE=.\fam_0alookup.cpp
# End Source File
# Begin Source File

SOURCE=.\fam_0bstatus.cpp
# End Source File
# Begin Source File

SOURCE=.\fam_13servclist.cpp
# End Source File
# Begin Source File

SOURCE=.\fam_15icqserver.cpp
# End Source File
# Begin Source File

SOURCE=.\fam_17signon.cpp
# End Source File
# End Group
# Begin Group "Direct Connection"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\directpackets.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_direct.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_directmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_filerequests.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_filetransfer.cpp
# End Source File
# End Group
# Begin Group "Miranda Bits"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\icq_advsearch.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_db.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_proto.cpp
# End Source File
# Begin Source File

SOURCE=.\icqosc_svcs.cpp
# End Source File
# Begin Source File

SOURCE=.\init.cpp
# End Source File
# Begin Source File

SOURCE=.\log.cpp
# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter ".txt"
# Begin Source File

SOURCE=".\docs\icq-readme.txt"
# End Source File
# Begin Source File

SOURCE=".\docs\IcqOscarJ-db settings.txt"
# End Source File
# Begin Source File

SOURCE=".\docs\IcqOscarJ-translation.txt"
# End Source File
# End Group
# Begin Group "UI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\UI\askauthentication.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_firstrun.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_opts.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_popups.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_uploadui.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\loginpassword.cpp
# End Source File
# Begin Source File

SOURCE=.\UI\userinfotab.cpp
# End Source File
# End Group
# Begin Group "Change Info"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\changeinfo\changeinfo.h
# End Source File
# Begin Source File

SOURCE=.\changeinfo\constants.cpp
# End Source File
# Begin Source File

SOURCE=.\changeinfo\db.cpp
# End Source File
# Begin Source File

SOURCE=.\changeinfo\dlgproc.cpp
# End Source File
# Begin Source File

SOURCE=.\changeinfo\editlist.cpp
# End Source File
# Begin Source File

SOURCE=.\changeinfo\editstring.cpp
# End Source File
# Begin Source File

SOURCE=.\changeinfo\main.cpp
# End Source File
# Begin Source File

SOURCE=.\changeinfo\upload.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\capabilities.cpp
# End Source File
# Begin Source File

SOURCE=.\cookies.cpp
# End Source File
# Begin Source File

SOURCE=.\i18n.cpp
# End Source File
# Begin Source File

SOURCE=.\iconlib.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_avatar.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_clients.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_fieldnames.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_http.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_infoupdate.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_packet.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_rates.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_server.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_servlist.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_xstatus.cpp
# End Source File
# Begin Source File

SOURCE=.\icq_xtraz.cpp
# End Source File
# Begin Source File

SOURCE=.\icqoscar.cpp
# ADD CPP /Yc"icqoscar.h"
# End Source File
# Begin Source File

SOURCE=.\oscar_filetransfer.cpp
# End Source File
# Begin Source File

SOURCE=.\stdpackets.cpp
# End Source File
# Begin Source File

SOURCE=.\tlv.cpp
# End Source File
# Begin Source File

SOURCE=.\utilities.cpp
# End Source File
# End Target
# End Project
