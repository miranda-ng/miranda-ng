# Microsoft Developer Studio Project File - Name="seenplugin" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=seenplugin - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "seenplugin.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "seenplugin.mak" CFG="seenplugin - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "seenplugin - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "seenplugin - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "seenplugin - Win32 PermNSN Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "seenplugin - Win32 PermNSN Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "seenplugin - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "release"
# PROP Intermediate_Dir "release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MP_SEEN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O1 /I "..\..\include\\" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib winmm.lib version.lib /nologo /base:"0x67400000" /dll /machine:I386 /out:"../../bin/upload/seen/seenplugin.dll" /filealign:0x200
# SUBTRACT LINK32 /pdb:none /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=cd ../../bin/upload/	md5 -s -t -oseen/seenplugin.dll.md5 seen/seenplugin.dll
# End Special Build Tool

!ELSEIF  "$(CFG)" == "seenplugin - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MP_SEEN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MP_SEEN_EXPORTS" /FAcs /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib winmm.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib winmm.lib /nologo /base:"0x67400000" /dll /debug /machine:I386 /out:"../../bin/debug/Plugins/seenplugin.dll"
# SUBTRACT LINK32 /profile /pdb:none /incremental:no /map

!ELSEIF  "$(CFG)" == "seenplugin - Win32 PermNSN Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleasePermNSN"
# PROP BASE Intermediate_Dir "ReleasePermNSN"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "releasePermNSN"
# PROP Intermediate_Dir "releasePermNSN"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "PERMITNSN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MP_SEEN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GX /O1 /I "..\..\include\\" /D "PERMITNSN" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib winmm.lib /nologo /dll /machine:I386
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib winmm.lib /nologo /base:"0x67400000" /dll /machine:I386 /out:"../../bin/upload/seen/NSNCompat/seenplugin.dll" /filealign:0x200
# SUBTRACT LINK32 /pdb:none /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=cd ../../bin/upload/	md5 -s -t -oseen/NSNCompat/seenplugin.dll.md5 seen/NSNCompat/seenplugin.dll
# End Special Build Tool

!ELSEIF  "$(CFG)" == "seenplugin - Win32 PermNSN Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugPermNSN"
# PROP BASE Intermediate_Dir "DebugPermNSN"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugPermNSN"
# PROP Intermediate_Dir "DebugPermNSN"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MP_SEEN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "PERMITNSN" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MP_SEEN_EXPORTS" /FAcs /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib winmm.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib advapi32.lib winmm.lib /nologo /base:"0x67400000" /dll /debug /machine:I386 /out:"../../bin/debug/Plugins/NSNCompat/seenplugin.dll"
# SUBTRACT LINK32 /profile /pdb:none /incremental:no /map

!ENDIF 

# Begin Target

# Name "seenplugin - Win32 Release"
# Name "seenplugin - Win32 Debug"
# Name "seenplugin - Win32 PermNSN Release"
# Name "seenplugin - Win32 PermNSN Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\file.c
# End Source File
# Begin Source File

SOURCE=.\history.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\menu.c
# End Source File
# Begin Source File

SOURCE=.\missed.c
# End Source File
# Begin Source File

SOURCE=.\options.c
# End Source File
# Begin Source File

SOURCE=.\userinfo.c
# End Source File
# Begin Source File

SOURCE=.\utils.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\seen.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\seen_info.txt
# End Source File
# Begin Source File

SOURCE=.\seen_langpack.txt
# End Source File
# End Target
# End Project
