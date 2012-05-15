# Microsoft Developer Studio Project File - Name="YAMN" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=YAMN - Win32 Release Win2in1
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "YAMN.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "YAMN.mak" CFG="YAMN - Win32 Release Win2in1"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "YAMN - Win32 Release Win2in1" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Debug Win2in1" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Release Win9x" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Debug Win9x" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release\Win2in1"
# PROP BASE Intermediate_Dir "Release\Win2in1"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release\Win2in1"
# PROP Intermediate_Dir "Release\Win2in1"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN2IN1" /YX /FD /c
# ADD CPP /nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /I "../../include/msapi" /I "../../include_API" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN2IN1" /FR /YX /FD /c
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /machine:I386 /out:"../../bin/release/plugins/YAMN.dll" /filealign:512
# ADD LINK32 libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /pdb:"../../bin/release/plugins/yamn.pdb" /debug /machine:I386 /out:"../../bin/release/plugins/yamn.dll" /filealign:512

!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug\Win2in1"
# PROP BASE Intermediate_Dir "Debug\Win2in1"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug\Win2in1"
# PROP Intermediate_Dir "Debug\Win2in1"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN2IN1" /FR /YX /FD /GZ /c
# ADD CPP /nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /I "../../include/msapi" /I "../../include_API" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN2IN1" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x417 /d "_DEBUG"
# ADD RSC /l 0x417 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN.dll"
# ADD LINK32 libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /pdb:"../../bin/Debug/plugins/yamn.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/yamn.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release/WinNT"
# PROP BASE Intermediate_Dir "Release/WinNT"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release/WinNT"
# PROP Intermediate_Dir "Release/WinNT"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /I "../../include/msapi" /I "../../include_API" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /machine:I386 /out:"../../bin/release/plugins/YAMN-NT/YAMN.dll" /filealign:512
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /debug /machine:I386 /out:"../../bin/release/plugins/YAMN-NT/yamn.dll" /filealign:512

!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug/WinNT"
# PROP BASE Intermediate_Dir "Debug/WinNT"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug/WinNT"
# PROP Intermediate_Dir "Debug/WinNT"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /I "../../include/msapi" /I "../../include_API" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x417 /d "_DEBUG"
# ADD RSC /l 0x417 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN-NT/YAMN.dll"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN-NT/YAMN.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release\Win9x"
# PROP BASE Intermediate_Dir "Release\Win9x"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release\Win9x"
# PROP Intermediate_Dir "Release\Win9x"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN9X" /YX /FD /c
# ADD CPP /nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /I "../../include/msapi" /I "../../include_API" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN9X" /YX /FD /c
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /machine:I386 /out:"../../bin/Release/plugins/YAMN-9x/YAMN.dll" /filealign:512
# ADD LINK32 libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /debug /machine:I386 /out:"../../bin/Release/plugins/YAMN-9x/YAMN.dll" /filealign:512

!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug\Win9x"
# PROP BASE Intermediate_Dir "Debug\Win9x"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug\Win9x"
# PROP Intermediate_Dir "Debug\Win9x"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN9X" /FR /YX /FD /GZ /c
# ADD CPP /nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /I "../../include/msapi" /I "../../include_API" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN9X" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x417 /d "_DEBUG"
# ADD RSC /l 0x417 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN-9x/YAMN.dll"
# ADD LINK32 libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN-9x/YAMN.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "YAMN - Win32 Release Win2in1"
# Name "YAMN - Win32 Debug Win2in1"
# Name "YAMN - Win32 Release"
# Name "YAMN - Win32 Debug"
# Name "YAMN - Win32 Release Win9x"
# Name "YAMN - Win32 Debug Win9x"
# Begin Group "YAMN"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Mail browser, dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\browser\badconnect.cpp
# End Source File
# Begin Source File

SOURCE=.\browser\mailbrowser.cpp
# End Source File
# End Group
# Begin Group "Mails"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\mails\decode.cpp
# End Source File
# Begin Source File

SOURCE=.\mails\mails.cpp
# End Source File
# Begin Source File

SOURCE=.\mails\mime.cpp
# End Source File
# End Group
# Begin Group "POP3 plugin"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\proto\md5.c
# End Source File
# Begin Source File

SOURCE=.\proto\netlib.cpp
# End Source File
# Begin Source File

SOURCE=.\proto\pop3\pop3.cpp
# End Source File
# Begin Source File

SOURCE=.\proto\pop3\pop3comm.cpp
# End Source File
# Begin Source File

SOURCE=.\proto\pop3\pop3opt.cpp
# End Source File
# End Group
# Begin Group "Header"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\main.h
# End Source File
# Begin Source File

SOURCE=.\proto\pop3\pop3.h
# End Source File
# Begin Source File

SOURCE=.\proto\pop3\pop3comm.h
# End Source File
# Begin Source File

SOURCE=.\proto\pop3\pop3opt.h
# End Source File
# Begin Source File

SOURCE=.\yamn.h
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\include\IcoLib.h
# End Source File
# Begin Source File

SOURCE=.\include\m_kbdnotify.h
# End Source File
# Begin Source File

SOURCE=.\include\m_popup.h
# End Source File
# Begin Source File

SOURCE=.\include\m_toptoolbar.h
# End Source File
# Begin Source File

SOURCE=.\include\m_uninstaller.h
# End Source File
# Begin Source File

SOURCE=.\include\m_updater.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\account.cpp
# End Source File
# Begin Source File

SOURCE=.\ChangeLog.txt
# End Source File
# Begin Source File

SOURCE=.\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\filterplugin.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\protoplugin.cpp
# End Source File
# Begin Source File

SOURCE=.\services.cpp
# End Source File
# Begin Source File

SOURCE=.\synchro.cpp
# End Source File
# Begin Source File

SOURCE=.\yamn.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\resources\iconeutral.ico
# End Source File
# Begin Source File

SOURCE=.\resources\iconttbdown.ico
# End Source File
# Begin Source File

SOURCE=.\resources\icooffline.ico
# End Source File
# Begin Source File

SOURCE=.\resources\icoyamn3.ico
# End Source File
# Begin Source File

SOURCE=.\resources\resource.h
# End Source File
# Begin Source File

SOURCE=.\resources\yamn.bmp
# End Source File
# Begin Source File

SOURCE=.\resources\YAMN.rc
# End Source File
# End Group
# End Target
# End Project
