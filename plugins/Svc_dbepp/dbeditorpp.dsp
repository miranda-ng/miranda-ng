# Microsoft Developer Studio Project File - Name="dbeditorpp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=dbeditorpp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dbeditorpp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dbeditorpp.mak" CFG="dbeditorpp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dbeditorpp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dbeditorpp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "dbeditorpp - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dbeditorpp - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX- /O1 /I "../../include" /I "../ExternalAPI" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /FR /Yu"headers.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 gdi32.lib winspool.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib comdlg32.lib advapi32.lib shell32.lib shlwapi.lib comctl32.lib /nologo /dll /machine:I386 /out:"../../bin/release/plugins/svc_dbepp.dll"

!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX- /ZI /Od /I "../../include" /I "../ExternalAPI" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /Yu"headers.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 gdi32.lib winspool.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib comdlg32.lib advapi32.lib shell32.lib shlwapi.lib comctl32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/plugins/svc_dbepp.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "dbeditorpp - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "dbeditorpp___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "dbeditorpp___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release_Unicode"
# PROP Intermediate_Dir ".\Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX- /Ox /Ot /Og /Os /I "../../include" /I "../ExternalAPI" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DBEDITORPP_EXPORTS" /FR /Yu"headers.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386 /out:"../../bin/release/plugins/dbeditorpp.dll"
# ADD LINK32 kernel32.lib user32.lib comdlg32.lib advapi32.lib shell32.lib shlwapi.lib comctl32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/release unicode/plugins/svc_dbepp.dll"

!ENDIF 

# Begin Target

# Name "dbeditorpp - Win32 Release"
# Name "dbeditorpp - Win32 Debug"
# Name "dbeditorpp - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ModSetting Enuming"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\modsettingenum.cpp
# End Source File
# Begin Source File

SOURCE=.\modsettingenum.h
# End Source File
# End Group
# Begin Group "dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\addeditsettingsdlg.cpp
# End Source File
# Begin Source File

SOURCE=.\copymodule.cpp
# End Source File
# Begin Source File

SOURCE=.\deletemodule.cpp
# End Source File
# Begin Source File

SOURCE=.\findwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\main_window.cpp
# End Source File
# Begin Source File

SOURCE=.\moduletree.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\settinglist.cpp
# End Source File
# Begin Source File

SOURCE=.\watchedvars.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\exportimport.cpp
# End Source File
# Begin Source File

SOURCE=.\icons.cpp
# End Source File
# Begin Source File

SOURCE=.\knownmodules.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# ADD CPP /Yc"headers.h"
# End Source File
# Begin Source File

SOURCE=.\modules.cpp
# End Source File
# Begin Source File

SOURCE=.\threads.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\headers.h
# End Source File
# Begin Source File

SOURCE=.\IcoLib.h
# End Source File
# Begin Source File

SOURCE=.\icons.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\Contacts.ico
# End Source File
# Begin Source File

SOURCE=.\dbeditorpp_readme.txt
# End Source File
# Begin Source File

SOURCE=.\dbeditorpp_translation.txt
# End Source File
# Begin Source File

SOURCE=.\res\Icon_1.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icon_14.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icon_15.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icon_16.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icon_17.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icon_18.ico
# End Source File
# Begin Source File

SOURCE=.\res\Icon_4.ico
# End Source File
# Begin Source File

SOURCE=.\res\offline2.ico
# End Source File
# Begin Source File

SOURCE=.\res\online2.ico
# End Source File
# Begin Source File

SOURCE=.\res\Red.ico
# End Source File
# Begin Source File

SOURCE=.\res\Red_open.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\res\Yellow.ico
# End Source File
# Begin Source File

SOURCE=.\res\Yellow_open.ico
# End Source File
# End Group
# End Target
# End Project
