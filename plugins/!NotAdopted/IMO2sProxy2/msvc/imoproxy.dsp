# Microsoft Developer Studio Project File - Name="imoproxy" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=imoproxy - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "imoproxy.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "imoproxy.mak" CFG="imoproxy - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "imoproxy - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "imoproxy - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "imoproxy - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "imoproxy_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\src\common" /I "..\src\imolib" /I "..\src\imo2skype" /I "F:\miranda09_src\miranda" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "imoproxy_EXPORTS" /FR /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "imoproxy - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "imoproxy_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\src\common" /I "..\src\imolib" /I "..\src\imo2skype" /I "F:\miranda09_src\miranda" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "imoproxy_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "_DEBUG"
# ADD RSC /l 0xc07 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"F:\miranda09_src\miranda\bin\debug\Plugins\imoproxy.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "imoproxy - Win32 Release"
# Name "imoproxy - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\imo2skype\avatarlist.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\buddylist.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\callqueue.c
# End Source File
# Begin Source File

SOURCE=..\src\common\cJSON.c
# End Source File
# Begin Source File

SOURCE=..\src\common\fifo.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\imo2skypeapi.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\imo2sproxy.c
# End Source File
# Begin Source File

SOURCE=..\src\imolib\imo_request.c
# End Source File
# Begin Source File

SOURCE=..\src\imolib\imo_skype.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\miranda\imoproxy.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\miranda\io_layer_netlib.c
# End Source File
# Begin Source File

SOURCE=..\src\imolib\io_layer_win32.c
# End Source File
# Begin Source File

SOURCE=..\src\common\memlist.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\msgqueue.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\queue.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\miranda\skypepluginlink.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\socksproxy.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\w32browser.c
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\w32skypeemu.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\imo2skype\buddylist.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\callqueue.h
# End Source File
# Begin Source File

SOURCE=..\src\common\cJSON.h
# End Source File
# Begin Source File

SOURCE=..\src\common\fifo.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\imo2skypeapi.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\imo2sproxy.h
# End Source File
# Begin Source File

SOURCE=..\src\imolib\imo_request.h
# End Source File
# Begin Source File

SOURCE=..\src\imolib\imo_skype.h
# End Source File
# Begin Source File

SOURCE=..\src\imolib\io_layer.h
# End Source File
# Begin Source File

SOURCE=..\src\common\memlist.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\msgqueue.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\queue.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\miranda\skypepluginlink.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\socksproxy.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\w32browser.h
# End Source File
# Begin Source File

SOURCE=..\src\imo2skype\w32skypeemu.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\src\imo2skype\miranda\res.rc
# End Source File
# End Group
# End Target
# End Project
