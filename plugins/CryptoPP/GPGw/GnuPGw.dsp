# Microsoft Developer Studio Project File - Name="GnuPGw" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GnuPGw - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GnuPGw.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GnuPGw.mak" CFG="GnuPGw - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GnuPGw - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP BASE Output_Dir "GnuPGw___Win32_Release"
# PROP BASE Intermediate_Dir "GnuPGw___Win32_Release"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /GX /O2 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /Ob2 /Yu"commonheaders.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib shell32.lib gdi32.lib msimg32.lib comdlg32.lib advapi32.lib msvcrt.lib /nologo /base:"0x46100000" /dll /map /machine:I386 /filealign:0x200
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib shell32.lib gdi32.lib msimg32.lib comdlg32.lib advapi32.lib msvcrt.lib /nologo /base:"0x46100000" /dll /map /machine:I386 /filealign:0x200
# SUBTRACT LINK32 /pdb:none /debug
# Begin Target

# Name "GnuPGw - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\commonheaders.c
# ADD CPP /Yc"commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\gpg.c
# End Source File
# Begin Source File

SOURCE=.\keys.c
# End Source File
# Begin Source File

SOURCE=.\language.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\passdialog.c
# End Source File
# Begin Source File

SOURCE=.\passphrases.c
# End Source File
# Begin Source File

SOURCE=.\pipeexec.c
# End Source File
# Begin Source File

SOURCE=.\tools.c
# End Source File
# Begin Source File

SOURCE=.\userdialog.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\gpg.h
# End Source File
# Begin Source File

SOURCE=.\keys.h
# End Source File
# Begin Source File

SOURCE=.\language.h
# End Source File
# Begin Source File

SOURCE=.\passdialog.h
# End Source File
# Begin Source File

SOURCE=.\passphrases.h
# End Source File
# Begin Source File

SOURCE=.\pipeexec.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\size.h
# End Source File
# Begin Source File

SOURCE=.\userdialog.h
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
