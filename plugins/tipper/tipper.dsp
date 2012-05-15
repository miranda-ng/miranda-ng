# Microsoft Developer Studio Project File - Name="tipper" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=tipper - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tipper.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tipper.mak" CFG="tipper - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tipper - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tipper - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tipper - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tipper - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tipper - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "tipper_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TIPPER_EXPORTS" /FR /Yu"common.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib /nologo /dll /map /debug /machine:I386 /filealign:512
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "tipper - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "tipper_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TIPPER_EXPORTS" /FR /Yu"common.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Debug/plugins/tipper.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ELSEIF  "$(CFG)" == "tipper - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "tipper___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "tipper___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NO_GZIP" /D "PNG_NO_STDIO" /D "PNG_NO_CONSOLE_IO" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "TIPPER_EXPORTS" /D MICROSOFT_LAYER_FOR_UNICODE=1 /Yu"common.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /debug /machine:I386 /out:"../../bin/Release/Plugins/tipper.dll" /filealign:512
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib /nologo /dll /machine:I386 /filealign:512
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "tipper - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "tipper___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "tipper___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NO_GZIP" /D "PNG_NO_STDIO" /D "PNG_NO_CONSOLE_IO" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "TIPPER_EXPORTS" /FR /Yu"common.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Debug/plugins/tipper.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none /incremental:no
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib unicows.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/tipper.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "tipper - Win32 Release"
# Name "tipper - Win32 Debug"
# Name "tipper - Win32 Release Unicode"
# Name "tipper - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\message_pump.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\popwin.cpp
# End Source File
# Begin Source File

SOURCE=.\str_utils.cpp
# End Source File
# Begin Source File

SOURCE=.\subst.cpp
# End Source File
# Begin Source File

SOURCE=.\tipper.cpp
# ADD CPP /Yc"common.h"
# End Source File
# Begin Source File

SOURCE=.\translations.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\m_tipper.h
# End Source File
# Begin Source File

SOURCE=.\message_pump.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\popwin.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\str_utils.h
# End Source File
# Begin Source File

SOURCE=.\subst.h
# End Source File
# Begin Source File

SOURCE=.\tipper.h
# End Source File
# Begin Source File

SOURCE=.\translations.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# End Target
# End Project
