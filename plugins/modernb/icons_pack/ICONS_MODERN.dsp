# Microsoft Developer Studio Project File - Name="ICONS_MODERN" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ICONS_MODERN - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ICONS_MODERN.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ICONS_MODERN.mak" CFG="ICONS_MODERN - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ICONS_MODERN - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ICONS_EXPORTS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /Za /W3 /GX- /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ICONS_EXPORTS" /D "_MBCS" /Yu"stdafx.h" /c
# SUBTRACT CPP /Z<none>
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"..\..\..\bin\Release\Icons\toolbar_icons.dll" /pdbtype:sept /noentry /ALIGN:4096 /ignore:4108
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:none /machine:IX86 /nodefaultlib /out:"..\..\..\bin\Release\Icons\toolbar_icons.dll" /noentry /ALIGN:4096 /ignore:4108
# SUBTRACT LINK32 /debug
# Begin Target

# Name "ICONS_MODERN - Win32 Release"
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx"
# Begin Source File

SOURCE=.\icon_resource.h
# End Source File
# Begin Source File

SOURCE=ICONS.rc
# End Source File
# End Group
# End Target
# End Project
