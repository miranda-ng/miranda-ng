# Microsoft Developer Studio Project File - Name="chat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=chat - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "chat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "chat.mak" CFG="chat - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "chat - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "chat - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "chat - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "chat - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "chat - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CHAT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CHAT_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib Version.lib /nologo /base:"0x54110000" /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/chat.dll"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "chat - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CHAT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CHAT_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib Version.lib /nologo /base:"0x54110000" /dll /debug /machine:I386 /out:"../../bin/debug/plugins/chat.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "chat - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "chat___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "chat___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug_Unicode"
# PROP Intermediate_Dir ".\Debug_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CHAT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "CHAT_EXPORTS" /D "UNICODE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "_DEBUG" /d "UNICODE"
# ADD RSC /l 0x41d /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Debug/Plugins/chat.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Version.lib shlwapi.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/chat.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "chat - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "chat___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "chat___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release_Unicode"
# PROP Intermediate_Dir ".\Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CHAT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include/msapi" /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "CHAT_EXPORTS" /D "UNICODE" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x41d /d "NDEBUG"
# ADD RSC /l 0x41d /d "NDEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../bin/Release/Plugins/chat.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Version.lib shlwapi.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/chat.dll"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ENDIF 

# Begin Target

# Name "chat - Win32 Release"
# Name "chat - Win32 Debug"
# Name "chat - Win32 Debug Unicode"
# Name "chat - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\clist.c
# End Source File
# Begin Source File

SOURCE=.\colorchooser.c
# End Source File
# Begin Source File

SOURCE=.\log.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\manager.c
# End Source File
# Begin Source File

SOURCE=.\message.c
# End Source File
# Begin Source File

SOURCE=.\options.c
# End Source File
# Begin Source File

SOURCE=.\richutil.c
# End Source File
# Begin Source File

SOURCE=.\services.c
# End Source File
# Begin Source File

SOURCE=.\tools.c
# End Source File
# Begin Source File

SOURCE=.\window.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "Other"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AggressiveOptimize.h
# End Source File
# Begin Source File

SOURCE=.\IcoLib.h
# End Source File
# Begin Source File

SOURCE=..\..\include\m_popup.h
# End Source File
# Begin Source File

SOURCE=.\m_smileyadd.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\chat.h
# End Source File
# Begin Source File

SOURCE=.\m_chat.h
# End Source File
# Begin Source File

SOURCE=.\richutil.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\Icons\1.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\2.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\3.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\4.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\5.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\6.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\action.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\addmode.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\bkgcolor.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\blank.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\bold.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\chanmgr.ico
# End Source File
# Begin Source File

SOURCE=.\Chat.rc
# End Source File
# Begin Source File

SOURCE=.\Icons\close.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\color.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\filter.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\filter2.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\highlight.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\history.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\hop.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\info.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\italics.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\join.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\junk.bmp
# End Source File
# Begin Source File

SOURCE=.\Icons\kick.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\message.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\messageout.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\nick.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\nicklist.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\nicklist2.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\normal.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\notice.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\Op.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\overlay.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\part.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\quit.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\removestatus.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\smiley.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\tag1.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\tag2.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\topic.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\topicbut.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\underline.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\Voice.ico
# End Source File
# Begin Source File

SOURCE=.\Icons\window.ico
# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter ".txt"
# Begin Source File

SOURCE=.\Docs\Chat_license.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\Chat_Readme.txt
# End Source File
# Begin Source File

SOURCE=.\Docs\Chat_Translate.txt
# End Source File
# Begin Source File

SOURCE="..\..\..\CVS upload\to do.txt"
# End Source File
# End Group
# End Target
# End Project
