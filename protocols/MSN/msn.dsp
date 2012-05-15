# Microsoft Developer Studio Project File - Name="msn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=msn - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "msn.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "msn.mak" CFG="msn - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "msn - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "msn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "msn - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "msn - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "msn - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G4 /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /base:"0x19000000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/msn.dll" /filealign:512
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "msn - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/plugins/msn.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "msn___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "msn___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release_Unicode"
# PROP Intermediate_Dir ".\Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /Yu"msn_global.h" /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /Yu"msn_global.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /base:"0x19000000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/msn.dll" /filealign:512
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /base:"0x19000000" /dll /map /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/msn.dll" /filealign:512
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "msn___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "msn___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "./Debug_Unicode"
# PROP Intermediate_Dir "./Debug_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FR /Yu"msn_global.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MSN_EXPORTS" /FR /Yu"msn_global.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"../../bin/debug/plugins/msn.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/msn.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "msn - Win32 Release"
# Name "msn - Win32 Debug"
# Name "msn - Win32 Release Unicode"
# Name "msn - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\des.c

!IF  "$(CFG)" == "msn - Win32 Release"

!ELSEIF  "$(CFG)" == "msn - Win32 Debug"

!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ezxml.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\msn.cpp
# ADD CPP /Yc"msn_global.h"
# End Source File
# Begin Source File

SOURCE=.\msn_auth.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_chat.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_commands.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_contact.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_errors.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_ftold.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_global.h
# End Source File
# Begin Source File

SOURCE=.\msn_http.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_libstr.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_links.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_lists.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_mail.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_mime.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_misc.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_msgqueue.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_msgsplit.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_natdetect.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_opts.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_p2p.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_p2ps.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_proto.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_soapab.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_soapstore.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_srv.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_ssl.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_std.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_svcs.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_switchboard.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_threads.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_useropts.cpp
# End Source File
# Begin Source File

SOURCE=.\msn_ws.cpp
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Miranda Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_clist.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_database.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_langpack.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_netlib.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_options.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_plugins.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_popup.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_protocols.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_protomod.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_protosvc.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_skin.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_system.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\m_utils.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\newpluginapi.h
# End Source File
# Begin Source File

SOURCE=..\..\SDK\headers_c\statusmodes.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Icos\avatar.ico
# End Source File
# Begin Source File

SOURCE=.\Icos\inbox.ico
# End Source File
# Begin Source File

SOURCE=.\Icos\invite.ico
# End Source File
# Begin Source File

SOURCE=.\Icos\list_al.ico
# End Source File
# Begin Source File

SOURCE=.\Icos\list_bl.ico
# End Source File
# Begin Source File

SOURCE=.\Icos\list_fl.ico
# End Source File
# Begin Source File

SOURCE=.\Icos\list_rl.ico
# End Source File
# Begin Source File

SOURCE=.\Icos\msn.ico
# End Source File
# Begin Source File

SOURCE=.\Icos\msnblock.ico
# End Source File
# Begin Source File

SOURCE=.\Icos\netmeeting.ico
# End Source File
# Begin Source File

SOURCE=.\Icos\profile.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\Icos\services.ico
# End Source File
# End Group
# End Target
# End Project
