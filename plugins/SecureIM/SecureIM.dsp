# Microsoft Developer Studio Project File - Name="secureim" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=secureim - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "secureim.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "secureim.mak" CFG="secureim - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "secureim - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "secureim - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "secureim - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECUREIM_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "\source\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECUREIM_EXPORTS" /Yu"commonheaders.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib msimg32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x45000000" /dll /map /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "secureim - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECUREIM_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "\source\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECUREIM_EXPORTS" /Yu"commonheaders.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msimg32.lib /nologo /base:"0x45000000" /dll /incremental:no /map /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "secureim - Win32 Release"
# Name "secureim - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\commonheaders.cpp
# ADD CPP /Yc"commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\crypt_check.cpp
# End Source File
# Begin Source File

SOURCE=.\crypt_dll.cpp
# End Source File
# Begin Source File

SOURCE=.\crypt_icons.cpp
# End Source File
# Begin Source File

SOURCE=.\crypt_lists.cpp
# End Source File
# Begin Source File

SOURCE=.\crypt_metacontacts.cpp
# End Source File
# Begin Source File

SOURCE=.\crypt_misc.cpp
# End Source File
# Begin Source File

SOURCE=.\crypt_popups.cpp
# End Source File
# Begin Source File

SOURCE=.\dbevent.cpp
# End Source File
# Begin Source File

SOURCE=.\gettime.cpp
# End Source File
# Begin Source File

SOURCE=.\images.cpp
# End Source File
# Begin Source File

SOURCE=.\language.cpp
# End Source File
# Begin Source File

SOURCE=.\loadicons.cpp
# End Source File
# Begin Source File

SOURCE=.\loadlib.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\mmi.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\popupOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\rtfconv.cpp
# End Source File
# Begin Source File

SOURCE=.\splitmsg.cpp
# End Source File
# Begin Source File

SOURCE=.\svcs_clist.cpp
# End Source File
# Begin Source File

SOURCE=.\svcs_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\svcs_proto.cpp
# End Source File
# Begin Source File

SOURCE=.\svcs_rsa.cpp
# End Source File
# Begin Source File

SOURCE=.\svcs_srmm.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\crypt.h
# End Source File
# Begin Source File

SOURCE=.\cryptopp.h
# End Source File
# Begin Source File

SOURCE=.\dbevent.h
# End Source File
# Begin Source File

SOURCE=.\gettime.h
# End Source File
# Begin Source File

SOURCE=.\images.h
# End Source File
# Begin Source File

SOURCE=.\language.h
# End Source File
# Begin Source File

SOURCE=.\loadicons.h
# End Source File
# Begin Source File

SOURCE=.\loadlib.h
# End Source File
# Begin Source File

SOURCE=.\mmi.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\popupOptions.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\rtfconv.h
# End Source File
# Begin Source File

SOURCE=.\secureim.h
# End Source File
# Begin Source File

SOURCE=.\splitmsg.h
# End Source File
# Begin Source File

SOURCE=.\svcs_clist.h
# End Source File
# Begin Source File

SOURCE=.\svcs_menu.h
# End Source File
# Begin Source File

SOURCE=.\svcs_proto.h
# End Source File
# Begin Source File

SOURCE=.\svcs_rsa.h
# End Source File
# Begin Source File

SOURCE=.\svcs_srmm.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# End Target
# End Project
