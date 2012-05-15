# Microsoft Developer Studio Project File - Name="updater" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=updater - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "updater.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "updater.mak" CFG="updater - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "updater - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "updater - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "updater - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "updater - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "updater - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UPDATER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../../include" /I "zbin" /I "zbin/minizip" /I "bzip2-1.0.3" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BZ_NO_STDIO" /D "UPDATER_EXPORTS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 comctl32.lib kernel32.lib user32.lib shell32.lib ole32.lib zlib.lib DelayImp.lib /nologo /base:"0x22000000" /dll /machine:I386 /out:"../../bin/release/plugins/updater.dll" /libpath:"zbin\x32" /filealign:0x200 /delayload:"zlib.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "updater - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UPDATER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /I "../../../include" /I "zbin" /I "zbin/minizip" /I "bzip2-1.0.3" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BZ_NO_STDIO" /D "UPDATER_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib shell32.lib ole32.lib zlib.lib DelayImp.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/plugins/updater.dll" /pdbtype:sept /libpath:"zbin\x32" /delayload:"zlib.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "updater - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "updater___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "updater___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UPDATER_EXPORTS" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../../include" /I "zbin" /I "zbin/minizip" /I "bzip2-1.0.3" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "BZ_NO_STDIO" /D "_UNICODE" /D "UNICODE" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG" /d "_UNICODE" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib /nologo /base:"0x22000000" /dll /machine:I386 /out:"../../bin/release/plugins/updater.dll"
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 DelayImp.lib comctl32.lib kernel32.lib user32.lib shell32.lib ole32.lib zlib.lib /nologo /base:"0x22000000" /dll /machine:I386 /out:"../../bin/Release Unicode/plugins/updater.dll" /libpath:"zbin\x32" /filealign:0x200 /delayload:"zlib.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "updater - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "updater___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "updater___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "updater___Win32_Debug_Unicode"
# PROP Intermediate_Dir "updater___Win32_Debug_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "UPDATER_EXPORTS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /I "../../../include" /I "zbin" /I "zbin/minizip" /I "bzip2-1.0.3" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "BZ_NO_STDIO" /D "UPDATER_EXPORTS" /D "_UNICODE" /D "UNICODE" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG" /d "_UNICODE" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/plugins/updater.dll" /pdbtype:sept
# ADD LINK32 comctl32.lib kernel32.lib user32.lib shell32.lib ole32.lib zlib.lib DelayImp.lib /nologo /dll /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/updater.dll" /pdbtype:sept /libpath:"zbin\x32" /delayload:"zlib.dll"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "updater - Win32 Release"
# Name "updater - Win32 Debug"
# Name "updater - Win32 Release Unicode"
# Name "updater - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\allocations.cpp
# End Source File
# Begin Source File

SOURCE=.\conf_comp_dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\conf_dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\extern.cpp
# End Source File
# Begin Source File

SOURCE=.\ezxml.c
# End Source File
# Begin Source File

SOURCE=.\icons.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\popups.cpp
# End Source File
# Begin Source File

SOURCE=.\progress_dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\scan.cpp
# End Source File
# Begin Source File

SOURCE=.\services.cpp
# End Source File
# Begin Source File

SOURCE=.\socket.cpp
# End Source File
# Begin Source File

SOURCE=.\unzipfile.cpp
# End Source File
# Begin Source File

SOURCE=.\updater.cpp
# End Source File
# Begin Source File

SOURCE=.\updater.def
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# Begin Source File

SOURCE=.\xmldata.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\allocations.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\conf_comp_dialog.h
# End Source File
# Begin Source File

SOURCE=.\conf_dialog.h
# End Source File
# Begin Source File

SOURCE=.\extern.h
# End Source File
# Begin Source File

SOURCE=.\ezxml.h
# End Source File
# Begin Source File

SOURCE=.\icons.h
# End Source File
# Begin Source File

SOURCE=.\m_updater.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\popups.h
# End Source File
# Begin Source File

SOURCE=.\progress_dialog.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\scan.h
# End Source File
# Begin Source File

SOURCE=.\services.h
# End Source File
# Begin Source File

SOURCE=.\socket.h
# End Source File
# Begin Source File

SOURCE=.\updater.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# Begin Source File

SOURCE=.\xmldata.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\notick.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\tick.ico
# End Source File
# Begin Source File

SOURCE=.\updater.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\version.rc
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Group "bzip-1.0.3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\bzip2-1.0.3\blocksort.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.3\bzlib.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.3\bzlib.h"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.3\bzlib_private.h"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.3\compress.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.3\crctable.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.3\decompress.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.3\huffman.c"
# End Source File
# Begin Source File

SOURCE=".\bzip2-1.0.3\randtable.c"
# End Source File
# End Group
# Begin Group "minizip"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\zbin\minizip\ioapi.c
# End Source File
# Begin Source File

SOURCE=.\zbin\minizip\ioapi_mem.c
# End Source File
# Begin Source File

SOURCE=.\zbin\minizip\unzip.c
# End Source File
# End Group
# End Target
# End Project
