# Microsoft Developer Studio Project File - Name="shutdown" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=shutdown - Win32 Release
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "shutdown.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "shutdown.mak" CFG="shutdown - Win32 Release"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "shutdown - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "shutdown - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "shutdown - Win32 Release Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "shutdown - Win32 Debug Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "shutdown - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "release/ANSI"
# PROP BASE Intermediate_Dir "temp/Release/ANSI"
# PROP BASE Target_Dir "release/ANSI"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "temp/Release/ANSI"
# PROP Intermediate_Dir "temp/Release/ANSI"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir "release/ANSI"
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SHUTDOWN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /I ".\include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STRICT" /D "SHUTDOWN_EXPORTS" /U "NO_STRICT" /FR /FD /opt:nowin98 /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203 /Oicf
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x409 /i ".\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib comctl32.lib gdi32.lib user32.lib shell32.lib advapi32.lib /nologo /base:"0x11070000" /dll /pdb:"./release/ANSI/shutdown.pdb" /map:"release/ANSI/shutdown.map" /debug /machine:I386 /nodefaultlib:"uuid.lib" /nodefaultlib:"OLDNAMES" /def:"shutdown.def" /out:"./release/ANSI/shutdown.dll" /mapinfo:lines /opt:nowin98 /ignore:4078 /RELEASE
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "shutdown - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "temp/Debug/ANSI"
# PROP BASE Intermediate_Dir "temp/Debug/ANSI"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "temp/Debug/ANSI"
# PROP Intermediate_Dir "temp/Debug/ANSI"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SHUTDOWN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /Gi /GR /GX /ZI /Od /I ".\include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STRICT" /D "SHUTDOWN_EXPORTS" /U "NO_STRICT" /FR /FD /GZ /c
# SUBTRACT CPP /WX /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# SUBTRACT MTL /Oicf
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x409 /i ".\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib comctl32.lib gdi32.lib user32.lib shell32.lib advapi32.lib /nologo /base:"0x11070000" /dll /pdb:"D:/Miranda IM/ANSI/Plugins/shutdown.pdb" /map /debug /machine:I386 /nodefaultlib:"uuid.lib" /nodefaultlib:"OLDNAMES" /def:"shutdown.def" /out:"D:/Miranda IM/ANSI/Plugins/shutdown.dll" /pdbtype:sept
# SUBTRACT LINK32 /verbose /pdb:none

!ELSEIF  "$(CFG)" == "shutdown - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "release/Unicode"
# PROP BASE Intermediate_Dir "temp/Release/Unicode"
# PROP BASE Target_Dir "release/Unicode"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "temp/Release/Unicode"
# PROP Intermediate_Dir "temp/Release/Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir "release/Unicode"
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SHUTDOWN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W4 /GX /O2 /I ".\include" /D "NDEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "STRICT" /D "SHUTDOWN_EXPORTS" /U "_MBCS" /U "NO_STRICT" /FR /FD /opt:nowin98 /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203 /Oicf
# ADD BASE RSC /l 0x1009 /d "NDEBUG" /d "_UNICODE" /d "UNICODE"
# ADD RSC /l 0x409 /i ".\include" /d "NDEBUG" /d "_UNICODE" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib comctl32.lib gdi32.lib user32.lib shell32.lib advapi32.lib /nologo /base:"0x11070000" /dll /pdb:"./release/Unicode/shutdown.pdb" /map:"release/Unicode/shutdown.map" /debug /machine:I386 /nodefaultlib:"uuid.lib" /nodefaultlib:"OLDNAMES" /def:"shutdown.def" /out:"./release/Unicode/shutdown.dll" /mapinfo:lines /opt:nowin98 /ignore:4078 /RELEASE
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "shutdown - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "temp/Debug/Unicode"
# PROP BASE Intermediate_Dir "temp/Debug/Unicode"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "temp/Debug/Unicode"
# PROP Intermediate_Dir "temp/Debug/Unicode"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SHUTDOWN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W4 /Gm /Gi /GR /GX /ZI /Od /I ".\include" /D "_DEBUG" /D "UNICODE" /D "_UNICODE" /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "STRICT" /D "SHUTDOWN_EXPORTS" /U "_MBCS" /U "NO_STRICT" /FR /FD /GZ /c
# SUBTRACT CPP /WX /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# SUBTRACT MTL /Oicf
# ADD BASE RSC /l 0x1009 /d "_DEBUG" /d "_UNICODE" /d "UNICODE"
# ADD RSC /l 0x409 /i ".\include" /d "_DEBUG" /d "_UNICODE" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib comctl32.lib gdi32.lib user32.lib shell32.lib advapi32.lib /nologo /base:"0x11070000" /dll /pdb:"D:/Miranda IM/Unicode/Plugins/shutdown.pdb" /map /debug /machine:I386 /nodefaultlib:"uuid.lib" /nodefaultlib:"OLDNAMES" /def:"shutdown.def" /out:"D:/Miranda IM/Unicode/Plugins/shutdown.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none /nodefaultlib

!ENDIF 

# Begin Target

# Name "shutdown - Win32 Release"
# Name "shutdown - Win32 Debug"
# Name "shutdown - Win32 Release Unicode"
# Name "shutdown - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cpuusage.c
# End Source File
# Begin Source File

SOURCE=.\frame.c
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\options.c
# End Source File
# Begin Source File

SOURCE=.\settingsdlg.c
# End Source File
# Begin Source File

SOURCE=.\shutdownsvc.c
# End Source File
# Begin Source File

SOURCE=.\utils.c
# End Source File
# Begin Source File

SOURCE=.\watcher.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\cpuusage.h
# End Source File
# Begin Source File

SOURCE=.\frame.h
# End Source File
# Begin Source File

SOURCE=.\m_shutdown.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\settingsdlg.h
# End Source File
# Begin Source File

SOURCE=.\shutdownsvc.h
# End Source File
# Begin Source File

SOURCE=.\utils.h
# End Source File
# Begin Source File

SOURCE=.\watcher.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\active.ico
# End Source File
# Begin Source File

SOURCE=.\res\header.ico
# End Source File
# Begin Source File

SOURCE=.\res\inactive.ico
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Documentation"

# PROP Default_Filter "txt"
# Begin Source File

SOURCE=.\Extensions\countdown.wav
# End Source File
# Begin Source File

SOURCE=.\m_shutdown.inc
# End Source File
# Begin Source File

SOURCE=".\docs\Shutdown-Developer.txt"
# End Source File
# Begin Source File

SOURCE=".\docs\Shutdown-License.txt"
# End Source File
# Begin Source File

SOURCE=".\docs\Shutdown-Readme.txt"
# End Source File
# Begin Source File

SOURCE=".\docs\Shutdown-Translation.txt"
# End Source File
# End Group
# Begin Group "SDK"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\include\m_autoreplacer.h
# End Source File
# Begin Source File

SOURCE=.\include\m_button.h
# End Source File
# Begin Source File

SOURCE=.\include\m_clc.h
# End Source File
# Begin Source File

SOURCE=.\include\m_clist.h
# End Source File
# Begin Source File

SOURCE=.\include\m_clistint.h
# End Source File
# Begin Source File

SOURCE=.\include\m_clui.h
# End Source File
# Begin Source File

SOURCE=.\include\m_cluiframes.h
# End Source File
# Begin Source File

SOURCE=.\include\m_database.h
# End Source File
# Begin Source File

SOURCE=.\include\m_file.h
# End Source File
# Begin Source File

SOURCE=.\include\m_fontservice.h
# End Source File
# Begin Source File

SOURCE=.\include\m_genmenu.h
# End Source File
# Begin Source File

SOURCE=.\include\m_hddinfo.h
# End Source File
# Begin Source File

SOURCE=.\include\m_hotkey.h
# End Source File
# Begin Source File

SOURCE=.\include\m_hotkeysplus.h
# End Source File
# Begin Source File

SOURCE=.\include\m_hotkeysservice.h
# End Source File
# Begin Source File

SOURCE=.\include\m_icolib.h
# End Source File
# Begin Source File

SOURCE=.\include\m_idle.h
# End Source File
# Begin Source File

SOURCE=.\include\m_langpack.h
# End Source File
# Begin Source File

SOURCE=.\include\m_magneticwindows.h
# End Source File
# Begin Source File

SOURCE=.\include\m_message.h
# End Source File
# Begin Source File

SOURCE=.\include\m_mwclc.h
# End Source File
# Begin Source File

SOURCE=.\include\m_options.h
# End Source File
# Begin Source File

SOURCE=.\include\m_plugins.h
# End Source File
# Begin Source File

SOURCE=.\include\m_protocols.h
# End Source File
# Begin Source File

SOURCE=.\include\m_protomod.h
# End Source File
# Begin Source File

SOURCE=.\include\m_protosvc.h
# End Source File
# Begin Source File

SOURCE=.\include\m_skin.h
# End Source File
# Begin Source File

SOURCE=.\include\m_snappingwindows.h
# End Source File
# Begin Source File

SOURCE=.\include\m_system.h
# End Source File
# Begin Source File

SOURCE=.\include\m_toptoolbar.h
# End Source File
# Begin Source File

SOURCE=.\include\m_trigger.h
# End Source File
# Begin Source File

SOURCE=.\include\m_updater.h
# End Source File
# Begin Source File

SOURCE=.\include\m_utils.h
# End Source File
# Begin Source File

SOURCE=.\include\m_weather.h
# End Source File
# Begin Source File

SOURCE=.\include\newpluginapi.h
# End Source File
# Begin Source File

SOURCE=.\include\statusmodes.h
# End Source File
# Begin Source File

SOURCE=.\include\win2k.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Info_Src.txt
# End Source File
# Begin Source File

SOURCE=.\License_Appendix.txt
# End Source File
# End Target
# End Project
