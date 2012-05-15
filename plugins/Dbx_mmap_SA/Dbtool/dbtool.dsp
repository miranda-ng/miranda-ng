# Microsoft Developer Studio Project File - Name="dbtool" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=dbtool - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "dbtool.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dbtool.mak" CFG="dbtool - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dbtool - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "dbtool - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "dbtool - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE "dbtool - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "dbtool - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DATABASE_INDEPENDANT" /Yu"dbtool.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /map /machine:I386 /out:"../../miranda/bin/release/dbtool.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "dbtool - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DATABASE_INDEPENDANT" /Yu"dbtool.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../../miranda/bin/debug/dbtool.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "dbtool - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "dbtool___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "dbtool___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DATABASE_INDEPENDANT" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "DATABASE_INDEPENDANT" /Yu"dbtool.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../../miranda/bin/debug/dbtool.exe" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../../miranda/bin/debug unicode/dbtool.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "dbtool - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "dbtool___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "dbtool___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DATABASE_INDEPENDANT" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "DATABASE_INDEPENDANT" /Yu"dbtool.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /map /machine:I386 /out:"../../miranda/bin/release/dbtool.exe" /ALIGN:4096 /ALIGN:4096
# SUBTRACT BASE LINK32 /pdb:none /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /map /machine:I386 /out:"../../miranda/bin/release unicode/dbtool.exe"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "dbtool - Win32 Release"
# Name "dbtool - Win32 Debug"
# Name "dbtool - Win32 Debug Unicode"
# Name "dbtool - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Workers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\aggressive.cpp
# End Source File
# Begin Source File

SOURCE=.\contactchain.cpp
# End Source File
# Begin Source File

SOURCE=.\eventchain.cpp
# End Source File
# Begin Source File

SOURCE=.\finaltasks.cpp
# End Source File
# Begin Source File

SOURCE=.\initialchecks.cpp
# End Source File
# Begin Source File

SOURCE=.\modulechain.cpp
# End Source File
# Begin Source File

SOURCE=.\settingschain.cpp
# End Source File
# Begin Source File

SOURCE=.\user.cpp
# End Source File
# End Group
# Begin Group "Wizard Pages"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cleaning.cpp
# End Source File
# Begin Source File

SOURCE=.\fileaccess.cpp
# End Source File
# Begin Source File

SOURCE=.\finished.cpp
# End Source File
# Begin Source File

SOURCE=.\openerror.cpp
# End Source File
# Begin Source File

SOURCE=.\progress.cpp
# End Source File
# Begin Source File

SOURCE=.\selectdb.cpp
# End Source File
# Begin Source File

SOURCE=.\welcome.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\disk.cpp
# End Source File
# Begin Source File

SOURCE=.\langpack.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# ADD CPP /Yc"dbtool.h"
# End Source File
# Begin Source File

SOURCE=.\wizard.cpp
# End Source File
# Begin Source File

SOURCE=.\worker.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\database.h
# End Source File
# Begin Source File

SOURCE=.\dbtool.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\dbthdr.emf
# End Source File
# Begin Source File

SOURCE=.\res\dbtool.emf
# End Source File
# Begin Source File

SOURCE=.\res\dbtool.ico
# End Source File
# Begin Source File

SOURCE=.\res\profileg.ico
# End Source File
# Begin Source File

SOURCE=.\res\profiler.ico
# End Source File
# Begin Source File

SOURCE=.\res\profiley.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# Begin Source File

SOURCE=.\dbtool.exe.manifest
# End Source File
# End Target
# End Project
