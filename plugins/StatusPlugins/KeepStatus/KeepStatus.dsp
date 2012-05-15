# Microsoft Developer Studio Project File - Name="KeepStatus" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=KeepStatus - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "KeepStatus.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "KeepStatus.mak" CFG="KeepStatus - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "KeepStatus - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "KeepStatus - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "KeepStatus - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "KeepStatus - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "KeepStatus - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /Yu"../commonstatus.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msvcrt.lib wininet.lib Delayimp.lib ws2_32.lib /nologo /base:"0x11cd0000" /dll /debug /machine:I386 /nodefaultlib /out:"../../../bin/release/plugins/KeepStatus.dll" /delayload:wininet.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /FR /Yu"../commonstatus.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib wininet.lib Delayimp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /debug /machine:I386 /out:"../../../bin/debug/plugins/KeepStatus.dll" /pdbtype:sept /DelayLoad:wininet.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "KeepStatus___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "KeepStatus___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /FR /Yu"../commonstatus.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /FR /Yu"../commonstatus.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib wininet.lib Delayimp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /debug /machine:I386 /out:"../../../bin/debug/plugins/KeepStatus.dll" /pdbtype:sept /DelayLoad:wininet.dll
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 comctl32.lib wininet.lib Delayimp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /debug /machine:I386 /out:"../../../bin/Debug Unicode/plugins/KeepStatus.dll" /pdbtype:sept /DelayLoad:wininet.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "KeepStatus___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "KeepStatus___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEEPSTATUS_EXPORTS" /Yu"../commonstatus.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /Yu"../commonstatus.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msvcrt.lib wininet.lib Delayimp.lib ws2_32.lib /nologo /base:"0x11cd0000" /dll /debug /machine:I386 /nodefaultlib /out:"../../../bin/release/plugins/KeepStatus.dll" /delayload:wininet.dll
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msvcrt.lib wininet.lib Delayimp.lib ws2_32.lib /nologo /base:"0x11cd0000" /dll /debug /machine:I386 /nodefaultlib /out:"../../../bin/Release Unicode/plugins/KeepStatus.dll" /delayload:wininet.dll
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "KeepStatus - Win32 Release"
# Name "KeepStatus - Win32 Debug"
# Name "KeepStatus - Win32 Debug Unicode"
# Name "KeepStatus - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\action_keepstatus.cpp
# ADD CPP /Yu"../commonstatus.h"
# End Source File
# Begin Source File

SOURCE=..\commonstatus.cpp
# ADD CPP /Yu"commonstatus.h"
# End Source File
# Begin Source File

SOURCE=..\confirmdialog.cpp
# ADD CPP /Yu"commonstatus.h"
# End Source File
# Begin Source File

SOURCE=..\..\helpers\gen_helpers.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\keepstatus.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# ADD CPP /Yc"../commonstatus.h"
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\trigger_keepstatus.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\commonstatus.h
# End Source File
# Begin Source File

SOURCE=.\keepstatus.h
# End Source File
# Begin Source File

SOURCE=..\m_statusplugins.h
# End Source File
# Begin Source File

SOURCE=..\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\vc6.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\Release\KeepStatus.txt

!IF  "$(CFG)" == "KeepStatus - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Debug Unicode"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "KeepStatus - Win32 Release Unicode"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Target
# End Project
