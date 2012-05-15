# Microsoft Developer Studio Project File - Name="IRC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=IRC - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IRC.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IRC.mak" CFG="IRC - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IRC - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IRC - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IRC - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IRC - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IRC - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRC_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRC_EXPORTS" /Yu"irc.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib wsock32.lib /nologo /base:"0x54010000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/IRC.dll" /filealign:512
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRC_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRC_EXPORTS" /Yu"irc.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib wsock32.lib /nologo /base:"0x54010000" /dll /map /debug /machine:I386 /out:"../../bin/debug/plugins/IRC.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "IRC___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "IRC___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRC_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "IRC_EXPORTS" /FR /Yu"irc.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib wsock32.lib /nologo /base:"0x54010000" /dll /map /debug /machine:I386 /out:"../../bin/debug/plugins/IRC.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /incremental:no
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib wsock32.lib /nologo /base:"0x54010000" /dll /map /debug /machine:I386 /out:"../../bin/debug Unicode/plugins/IRC.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "IRC___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "IRC___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRC_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "IRC_EXPORTS" /Yu"irc.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib wsock32.lib /nologo /base:"0x54010000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/IRC.dll"
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib wsock32.lib /nologo /base:"0x54010000" /dll /map /debug /machine:I386 /out:"../../bin/release unicode/plugins/IRC.dll" /filealign:512
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "IRC - Win32 Release"
# Name "IRC - Win32 Debug"
# Name "IRC - Win32 Debug Unicode"
# Name "IRC - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\clist.cpp
# End Source File
# Begin Source File

SOURCE=.\commandmonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\input.cpp
# End Source File
# Begin Source File

SOURCE=.\irclib.cpp
# End Source File
# Begin Source File

SOURCE=.\ircproto.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# ADD CPP /Yc"irc.h"
# End Source File
# Begin Source File

SOURCE=.\MString.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\output.cpp
# End Source File
# Begin Source File

SOURCE=.\scripting.cpp
# End Source File
# Begin Source File

SOURCE=.\services.cpp
# End Source File
# Begin Source File

SOURCE=.\tools.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\userinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\windows.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "External"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AggressiveOptimize.h
# End Source File
# Begin Source File

SOURCE=.\IcoLib.h
# End Source File
# Begin Source File

SOURCE="..\..\plugins\chat unstableTab\m_chat.h"
# End Source File
# Begin Source File

SOURCE=.\m_ircscript.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\commandmonitor.h
# End Source File
# Begin Source File

SOURCE=.\irc.h
# End Source File
# Begin Source File

SOURCE=.\irclib.h
# End Source File
# Begin Source File

SOURCE=.\MString.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
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

SOURCE=.\Icons\add.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\apply.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\block.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\dcc.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\delete.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\edit.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\go.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\hide.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\irc.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\join.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\list.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\manager.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\nick.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\part.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\question.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\quick.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\rename.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\Icons\server.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\show.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\whois.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\world.ico
# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter ".txt"
# Begin Source File

SOURCE=.\Docs\IRC_license.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\IRC_Readme.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\IRC_Translate.txt
# End Source File
# Begin Source File

SOURCE="..\..\..\CVS upload\to do.txt"
# End Source File
# End Group
# Begin Source File

SOURCE=.\Docs\IRC_servers.ini
# End Source File
# End Target
# End Project
