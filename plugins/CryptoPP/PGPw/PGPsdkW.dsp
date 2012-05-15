# Microsoft Developer Studio Project File - Name="PGPsdkW" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PGPsdkW - Win32 Release6
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PGPsdkW.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PGPsdkW.mak" CFG="PGPsdkW - Win32 Release6"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PGPsdkW - Win32 Release6" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PGPsdkW - Win32 Release8" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PGPsdkW - Win32 Release6"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PGPsdkW___Win32_Release6"
# PROP BASE Intermediate_Dir "PGPsdkW___Win32_Release6"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release6"
# PROP Intermediate_Dir "Release6"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /O1 /I "sdk6/include" /D PGP_WIN32=0x658 /YX"commonheaders.h" /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O1 /I "sdk6/include" /D PGP_WIN32=0x658 /Yu"commonheaders.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 delayimp.lib pgp_sdk.lib pgpsdknl.lib pgpsdkui.lib kernel32.lib user32.lib shell32.lib gdi32.lib msimg32.lib comdlg32.lib msvcrt.lib /nologo /base:"0x46600000" /dll /map /machine:I386 /pdb:"Release6/PGPsdkW6.pdb" /out:"Release6/PGPsdkW6.dll" /libpath:"sdk6/lib" /filealign:0x200 /delayload:pgp_sdk.dll /delayload:pgpsdkui.dll
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 delayimp.lib pgp_sdk.lib pgpsdknl.lib pgpsdkui.lib kernel32.lib user32.lib shell32.lib gdi32.lib msimg32.lib comdlg32.lib msvcrt.lib /nologo /base:"0x46600000" /dll /map /machine:I386 /pdb:"Release6/PGPsdkW6.pdb" /out:"Release6/PGPsdkW6.dll" /libpath:"sdk6/lib" /filealign:0x200 /delayload:pgp_sdk.dll /delayload:pgpsdkui.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "PGPsdkW - Win32 Release8"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PGPsdkW___Win32_Release8"
# PROP BASE Intermediate_Dir "PGPsdkW___Win32_Release8"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release8"
# PROP Intermediate_Dir "Release8"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /Gi /O1 /I "sdk8/include" /D PGP_WIN32=0x800 /YX"commonheaders.h" /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O1 /I "sdk8/include" /D PGP_WIN32=0x800 /Yu"commonheaders.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 delayimp.lib pgpsdk.lib pgpsdknl.lib pgpsdkui.lib kernel32.lib user32.lib shell32.lib gdi32.lib msimg32.lib comdlg32.lib msvcrt.lib /nologo /base:"0x46800000" /dll /map /machine:I386 /pdb:"Release8/PGPsdkW8.pdb" /out:"Release8/PGPsdkW8.dll" /libpath:"sdk8/lib" /filealign:0x200 /delayload:pgp_sdk.dll /delayload:pgpsdkui.dll
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 delayimp.lib pgpsdk.lib pgpsdknl.lib pgpsdkui.lib kernel32.lib user32.lib shell32.lib gdi32.lib msimg32.lib comdlg32.lib msvcrt.lib /nologo /base:"0x46800000" /dll /map /machine:I386 /pdb:"Release8/PGPsdkW8.pdb" /out:"Release8/PGPsdkW8.dll" /libpath:"sdk8/lib" /filealign:0x200 /delayload:pgpsdk.dll /delayload:pgpsdkui.dll
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "PGPsdkW - Win32 Release6"
# Name "PGPsdkW - Win32 Release8"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\commonheaders.cpp
# ADD CPP /Yc"commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\commonheaders.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
