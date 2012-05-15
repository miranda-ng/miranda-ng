# Microsoft Developer Studio Project File - Name="crshdmp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=crshdmp - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "crshdmp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "crshdmp.mak" CFG="crshdmp - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "crshdmp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "crshdmp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "crshdmp - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "crshdmp - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "crshdmp - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "crshdmp_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O1 /I "..\..\include" /I "vc6" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /FAcs /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib shell32.lib advapi32.lib delayimp.lib dbghelp.lib /nologo /dll /pdb:none /map /machine:I386 /out:"Release/svc_crshdmp.dll" /DELAYLOAD:dbghelp.dll /DELAYLOAD:advapi32.dll /DELAY:UNLOAD /filealign:0x200
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "crshdmp - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "crshdmp_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /I "..\..\include" /I "vc6" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib shell32.lib advapi32.lib delayimp.lib dbghelp.lib /nologo /dll /debug /machine:I386 /out:"Debug/svc_crshdmp.dll" /pdbtype:sept /DELAYLOAD:dbghelp.dll /DELAYLOAD:advapi32.dll /DELAY:UNLOAD /filealign:0x200
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "crshdmp - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "crshdmp___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "crshdmp___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release Unicode"
# PROP Intermediate_Dir "Release Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O1 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /FAcs /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /O1 /Ob1 /I "..\..\include" /I "vc6" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "_UNICODE" /D "UNICODE" /FR /FD /c
# SUBTRACT CPP /Z<none> /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib delayimp.lib dbghelp.lib /nologo /dll /map /machine:I386 /out:"Release/svc_crshdmp.dll" /DELAYLOAD:dbghelp.dll /DELAYLOAD:advapi32.dll /DELAY:UNLOAD /filealign:0x200
# SUBTRACT BASE LINK32 /pdb:none /debug
# ADD LINK32 kernel32.lib user32.lib shell32.lib advapi32.lib delayimp.lib dbghelp.lib /nologo /dll /pdb:none /map /machine:I386 /out:"Release Unicode/svc_crshdmp.dll" /DELAYLOAD:dbghelp.dll /DELAYLOAD:advapi32.dll /DELAY:UNLOAD /filealign:0x200
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "crshdmp - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "crshdmp___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "crshdmp___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "crshdmp___Win32_Debug_Unicode"
# PROP Intermediate_Dir "crshdmp___Win32_Debug_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "crshdmp_EXPORTS" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /I "..\..\include" /I "vc6" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib advapi32.lib delayimp.lib dbghelp.lib /nologo /dll /debug /machine:I386 /out:"Debug/svc_crshdmp.dll" /pdbtype:sept /DELAYLOAD:dbghelp.dll /DELAY:UNLOAD /filealign:0x200
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib shell32.lib advapi32.lib delayimp.lib dbghelp.lib /nologo /dll /debug /machine:I386 /out:"Debug Unicode/svc_crshdmp.dll" /pdbtype:sept /DELAYLOAD:dbghelp.dll /DELAY:UNLOAD /filealign:0x200
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "crshdmp - Win32 Release"
# Name "crshdmp - Win32 Debug"
# Name "crshdmp - Win32 Release Unicode"
# Name "crshdmp - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bkstring.cpp
# End Source File
# Begin Source File

SOURCE=.\crshdmp.cpp
# End Source File
# Begin Source File

SOURCE=.\crshdmp_icons.cpp
# End Source File
# Begin Source File

SOURCE=.\dumper.cpp
# End Source File
# Begin Source File

SOURCE=.\exhndlr.cpp
# End Source File
# Begin Source File

SOURCE=.\ui.cpp
# End Source File
# Begin Source File

SOURCE=.\upload.cpp
# End Source File
# Begin Source File

SOURCE=.\utils.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\bkstring.h
# End Source File
# Begin Source File

SOURCE=.\m_folders.h
# End Source File
# Begin Source File

SOURCE=.\m_updater.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\sdkstuff.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\crshdmp.rc
# End Source File
# Begin Source File

SOURCE=".\Vi show.ico"
# End Source File
# Begin Source File

SOURCE=".\Vi to clipboard.ico"
# End Source File
# Begin Source File

SOURCE=".\Vi to file.ico"
# End Source File
# Begin Source File

SOURCE=".\Vi upload.ico"
# End Source File
# Begin Source File

SOURCE=.\Vi.ico
# End Source File
# End Group
# End Target
# End Project
