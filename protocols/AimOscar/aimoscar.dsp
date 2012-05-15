# Microsoft Developer Studio Project File - Name="aim" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=aim - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "aimoscar.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "aimoscar.mak" CFG="aim - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "aim - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "aim - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "aim - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "aim - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "aim - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "AIM_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/Aim.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "aim - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "AIM_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W2 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "AIM_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/plugins/Aim.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "aim - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "aim___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "aim___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "AIM_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/Aim.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/release unicode/plugins/Aim.dll" /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "aim - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "aim___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "aim___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W2 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "AIM_EXPORTS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W2 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "AIM_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/plugins/Aim.dll" /pdbtype:sept
# ADD LINK32 wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug unicode/plugins/Aim.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "aim - Win32 Release"
# Name "aim - Win32 Debug"
# Name "aim - Win32 Release Unicode"
# Name "aim - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\aim.cpp
# End Source File
# Begin Source File

SOURCE=.\avatars.cpp
# End Source File
# Begin Source File

SOURCE=.\away.cpp
# End Source File
# Begin Source File

SOURCE=.\chat.cpp
# End Source File
# Begin Source File

SOURCE=.\client.cpp
# End Source File
# Begin Source File

SOURCE=.\connection.cpp
# End Source File
# Begin Source File

SOURCE=.\conv.cpp
# End Source File
# Begin Source File

SOURCE=.\direct_connect.cpp
# End Source File
# Begin Source File

SOURCE=.\error.cpp
# End Source File
# Begin Source File

SOURCE=.\file.cpp
# End Source File
# Begin Source File

SOURCE=.\flap.cpp
# End Source File
# Begin Source File

SOURCE=.\links.cpp
# End Source File
# Begin Source File

SOURCE=.\packets.cpp
# End Source File
# Begin Source File

SOURCE=.\popup.cpp
# End Source File
# Begin Source File

SOURCE=.\proto.cpp
# End Source File
# Begin Source File

SOURCE=.\proxy.cpp
# End Source File
# Begin Source File

SOURCE=.\server.cpp
# End Source File
# Begin Source File

SOURCE=.\services.cpp
# End Source File
# Begin Source File

SOURCE=.\snac.cpp
# End Source File
# Begin Source File

SOURCE=.\theme.cpp
# End Source File
# Begin Source File

SOURCE=.\thread.cpp
# End Source File
# Begin Source File

SOURCE=.\tlv.cpp
# End Source File
# Begin Source File

SOURCE=.\ui.cpp
# End Source File
# Begin Source File

SOURCE=.\utility.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\aim.h
# End Source File
# Begin Source File

SOURCE=.\avatars.h
# End Source File
# Begin Source File

SOURCE=.\chat.h
# End Source File
# Begin Source File

SOURCE=.\client.h
# End Source File
# Begin Source File

SOURCE=.\conv.h
# End Source File
# Begin Source File

SOURCE=.\defines.h
# End Source File
# Begin Source File

SOURCE=.\direct_connect.h
# End Source File
# Begin Source File

SOURCE=.\error.h
# End Source File
# Begin Source File

SOURCE=.\file.h
# End Source File
# Begin Source File

SOURCE=.\flap.h
# End Source File
# Begin Source File

SOURCE=.\links.h
# End Source File
# Begin Source File

SOURCE=.\packets.h
# End Source File
# Begin Source File

SOURCE=.\popup.h
# End Source File
# Begin Source File

SOURCE=.\proto.h
# End Source File
# Begin Source File

SOURCE=.\proxy.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\server.h
# End Source File
# Begin Source File

SOURCE=.\services.h
# End Source File
# Begin Source File

SOURCE=.\snac.h
# End Source File
# Begin Source File

SOURCE=.\strl.h
# End Source File
# Begin Source File

SOURCE=.\theme.h
# End Source File
# Begin Source File

SOURCE=.\tlv.h
# End Source File
# Begin Source File

SOURCE=.\ui.h
# End Source File
# Begin Source File

SOURCE=.\utility.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icons\add.ico
# End Source File
# Begin Source File

SOURCE=.\icons\admin.ico
# End Source File
# Begin Source File

SOURCE=.\aim.rc
# End Source File
# Begin Source File

SOURCE=.\icons\aol.ico
# End Source File
# Begin Source File

SOURCE=.\icons\away.ico
# End Source File
# Begin Source File

SOURCE=.\icons\backgroundcolor.ico
# End Source File
# Begin Source File

SOURCE=.\icons\block.ico
# End Source File
# Begin Source File

SOURCE=.\icons\bold.ico
# End Source File
# Begin Source File

SOURCE=.\icons\bot.ico
# End Source File
# Begin Source File

SOURCE=.\icons\confirmed.ico
# End Source File
# Begin Source File

SOURCE=.\icons\foregroundcolor.ico
# End Source File
# Begin Source File

SOURCE=.\icons\hiptop.ico
# End Source File
# Begin Source File

SOURCE=.\icons\icq.ico
# End Source File
# Begin Source File

SOURCE=.\icons\idle.ico
# End Source File
# Begin Source File

SOURCE=.\icons\inbox.ico
# End Source File
# Begin Source File

SOURCE=.\icons\italic.ico
# End Source File
# Begin Source File

SOURCE=.\icons\link.ico
# End Source File
# Begin Source File

SOURCE=.\icons\nbold.ico
# End Source File
# Begin Source File

SOURCE=.\icons\nitalic.ico
# End Source File
# Begin Source File

SOURCE=.\icons\nnormalscript.ico
# End Source File
# Begin Source File

SOURCE=.\icons\normalscript.ico
# End Source File
# Begin Source File

SOURCE=.\icons\nsubscript.ico
# End Source File
# Begin Source File

SOURCE=.\icons\nsuperscript.ico
# End Source File
# Begin Source File

SOURCE=.\icons\nunderline.ico
# End Source File
# Begin Source File

SOURCE=.\icons\oscar.ico
# End Source File
# Begin Source File

SOURCE=.\icons\profile.ico
# End Source File
# Begin Source File

SOURCE=.\icons\subscript.ico
# End Source File
# Begin Source File

SOURCE=.\icons\superscript.ico
# End Source File
# Begin Source File

SOURCE=.\icons\unconfirmed.ico
# End Source File
# Begin Source File

SOURCE=.\icons\underline.ico
# End Source File
# End Group
# End Target
# End Project
