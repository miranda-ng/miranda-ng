# Microsoft Developer Studio Project File - Name="srmm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=srmm - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "srmm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "srmm.mak" CFG="srmm - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "srmm - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "srmm - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "srmm - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "srmm - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "srmm - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /Yu"commonheaders.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/release/plugins/srmm.dll"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "srmm - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /Yu"commonheaders.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/Plugins/srmm.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "srmm - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "srmm___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "srmm___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release_Unicode"
# PROP Intermediate_Dir ".\Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "UNICODE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /Yu"commonheaders.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /i "../../include" /d "NDEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "NDEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /pdb:none /machine:I386 /out:"../../bin/release/plugins/srmm.dll"
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/srmm.dll"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "srmm - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "srmm___Win32_Unicode_Debug"
# PROP BASE Intermediate_Dir "srmm___Win32_Unicode_Debug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug_Unicode"
# PROP Intermediate_Dir ".\Debug_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "UNICODE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "SRMM_EXPORTS" /Yu"commonheaders.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /i "../../include" /d "_DEBUG"
# ADD RSC /l 0x809 /i "../../include" /d "_DEBUG" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/Plugins/srmm.dll" /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Debug Unicode/Plugins/srmm.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "srmm - Win32 Release"
# Name "srmm - Win32 Debug"
# Name "srmm - Win32 Release Unicode"
# Name "srmm - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cmdlist.c
# End Source File
# Begin Source File

SOURCE=.\globals.c
# End Source File
# Begin Source File

SOURCE=.\msgdialog.c
# End Source File
# Begin Source File

SOURCE=.\msglog.c
# End Source File
# Begin Source File

SOURCE=.\msgoptions.c
# End Source File
# Begin Source File

SOURCE=.\msgs.c
# End Source File
# Begin Source File

SOURCE=.\msgtimedout.c
# End Source File
# Begin Source File

SOURCE=.\richutil.c
# End Source File
# Begin Source File

SOURCE=.\srmm.c
# ADD CPP /Yc"commonheaders.h"
# End Source File
# Begin Source File

SOURCE=.\statusicon.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cmdlist.h
# End Source File
# Begin Source File

SOURCE=.\commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\globals.h
# End Source File
# Begin Source File

SOURCE=.\msgs.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\richutil.h
# End Source File
# Begin Source File

SOURCE=.\statusicon.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\add.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\res\addcontact.ico
# End Source File
# Begin Source File

SOURCE=.\res\clock.ico
# End Source File
# Begin Source File

SOURCE=.\res\details.ico
# End Source File
# Begin Source File

SOURCE=.\res\downarrow.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\res\dragcopy.cur
# End Source File
# Begin Source File

SOURCE=..\..\src\res\dropuser.cur
# End Source File
# Begin Source File

SOURCE=.\res\history.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\res\hyperlin.cur
# End Source File
# Begin Source File

SOURCE=.\res\incoming.ico
# End Source File
# Begin Source File

SOURCE=.\res\notice.ico
# End Source File
# Begin Source File

SOURCE=.\res\outgoing.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\res\typing.ico
# End Source File
# End Group
# End Target
# End Project
