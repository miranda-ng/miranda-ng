# Microsoft Developer Studio Project File - Name="speak" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=speak - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "speak.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "speak.mak" CFG="speak - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "speak - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "speak - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/miranda/plugins/speak/speak", OOBAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "speak - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TESTPLUG_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /Ob2 /I ".\\" /I "D:\PROJECT\MIRANDA\MIRANDA0100" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SPEAK_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x43500000" /dll /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:yes /debug /nodefaultlib

!ELSEIF  "$(CFG)" == "speak - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TESTPLUG_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Zi /Od /I ".\\" /I "D:\PROJECT\MIRANDA\MIRANDA0100" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SPEAK_EXPORTS" /FD /GZ /c
# SUBTRACT CPP /X /Fr /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x43500000" /dll /incremental:no /debug /machine:I386 /out:"../../miranda0100/debug/plugins/speak.dll" /pdbtype:sept
# SUBTRACT LINK32 /verbose /nodefaultlib

!ENDIF 

# Begin Target

# Name "speak - Win32 Release"
# Name "speak - Win32 Debug"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\checked.ico
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\speak.rc
# End Source File
# Begin Source File

SOURCE=.\unchecked.ico
# End Source File
# End Group
# Begin Group "Source"

# PROP Default_Filter ""
# Begin Group "config"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\config\config_database.cpp
# End Source File
# Begin Source File

SOURCE=.\config\config_database.h
# End Source File
# Begin Source File

SOURCE=.\config\dialog_config_active.cpp
# End Source File
# Begin Source File

SOURCE=.\config\dialog_config_active.h
# End Source File
# Begin Source File

SOURCE=.\config\dialog_config_engine.cpp
# End Source File
# Begin Source File

SOURCE=.\config\dialog_config_engine.h
# End Source File
# Begin Source File

SOURCE=.\config\speak_config.cpp
# End Source File
# Begin Source File

SOURCE=.\config\speak_config.h
# End Source File
# Begin Source File

SOURCE=.\config\speech_interface.cpp
# End Source File
# Begin Source File

SOURCE=.\config\speech_interface.h
# End Source File
# End Group
# Begin Group "announce"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\announce\announce_database.cpp
# End Source File
# Begin Source File

SOURCE=.\announce\announce_database.h
# End Source File
# Begin Source File

SOURCE=.\announce\dialog_announce.cpp
# End Source File
# Begin Source File

SOURCE=.\announce\dialog_announce.h
# End Source File
# Begin Source File

SOURCE=.\announce\event_information.cpp
# End Source File
# Begin Source File

SOURCE=.\announce\event_information.h
# End Source File
# Begin Source File

SOURCE=.\announce\protocol_information.cpp
# End Source File
# Begin Source File

SOURCE=.\announce\protocol_information.h
# End Source File
# Begin Source File

SOURCE=.\announce\speak_announce.cpp
# End Source File
# Begin Source File

SOURCE=.\announce\speak_announce.h
# End Source File
# End Group
# Begin Group "dialog"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\dialog\miranda_dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\dialog\miranda_dialog.h
# End Source File
# End Group
# Begin Group "defs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\defs\voice_desc.h
# End Source File
# End Group
# Begin Group "user"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\user\user_information.cpp
# End Source File
# Begin Source File

SOURCE=.\user\user_information.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\m_speak.h
# End Source File
# Begin Source File

SOURCE=.\speak.cpp
# End Source File
# Begin Source File

SOURCE=.\speak.h
# End Source File
# End Group
# Begin Group "Package"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\package\langpack_speak.txt
# End Source File
# Begin Source File

SOURCE=.\package\langpack_speak_german.txt
# End Source File
# Begin Source File

SOURCE=.\package\langpack_speak_italian.txt
# End Source File
# Begin Source File

SOURCE=.\package\langpack_speak_russian.txt
# End Source File
# Begin Source File

SOURCE=.\package\Speak.txt
# End Source File
# End Group
# End Target
# End Project
