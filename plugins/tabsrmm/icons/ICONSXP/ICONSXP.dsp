# Microsoft Developer Studio Project File - Name="ICONSXP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ICONSXP - Win32 Release
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "ICONSXP.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "ICONSXP.mak" CFG="ICONSXP - Win32 Release"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "ICONSXP - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
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
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ICONSXP_EXPORTS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /Zi /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ICONSXP_EXPORTS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"Release\tabsrmm_icons.dll" /implib:"$(OutDir)/ICONSXP.lib" /pdbtype:sept /opt:ref /opt:icf /noentry
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:IX86 /nodefaultlib /out:"Release\tabsrmm_icons.dll" /implib:"$(OutDir)/ICONSXP.lib" /pdbtype:sept /opt:ref /opt:icf /noentry
# SUBTRACT LINK32 /debug
# Begin Target

# Name "ICONSXP - Win32 Release"
# Begin Group "Headerdateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx"
# Begin Source File

SOURCE=.\ICONSXP.rc
# End Source File
# End Group
# End Target
# End Project
