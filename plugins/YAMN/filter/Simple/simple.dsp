# Microsoft Developer Studio Project File - Name="simple" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=simple - Win32 Release
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "simple.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "simple.mak" CFG="simple - Win32 Release"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "simple - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "simple - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "simple - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G4 /Zp4 /MD /W3 /GX /Zi /O1 /Ob0 /I "../../../../include" /I "../../../../include/msapi" /I "../../../../include_API" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib /nologo /dll /machine:I386 /out:"../../../../bin/release/plugins/YAMN-filter/simple.dll" /filealign:512
# ADD LINK32 kernel32.lib user32.lib /nologo /dll /pdb:"../../../../bin/Release/plugins/YAMN/simple.pdb" /debug /machine:I386 /out:"../../../../bin/Release/plugins/YAMN/simple.dll" /filealign:512

!ELSEIF  "$(CFG)" == "simple - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD CPP /nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../../../include" /I "../../../../include/msapi" /I "../../../../include_API" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x417 /d "_DEBUG"
# ADD RSC /l 0x417 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib /nologo /dll /debug /machine:I386 /out:"../../../../bin/Debug/plugins/YAMN-filter/simple.dll"
# ADD LINK32 kernel32.lib user32.lib /nologo /dll /pdb:"../../../../bin/Debug/plugins/YAMN/simple.pdb" /debug /machine:I386 /out:"../../../../bin/Debug/plugins/YAMN/simple.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "simple - Win32 Release"
# Name "simple - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\maindll.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
