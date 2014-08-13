# Microsoft Developer Studio Project File - Name="Skype_protocol" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Skype_protocol - Win32 Debug UNICODE
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "Skype_protocol.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "Skype_protocol.mak" CFG="Skype_protocol - Win32 Debug UNICODE"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Skype_protocol - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Skype_protocol - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Skype_protocol - Win32 Debug UNICODE" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Skype_protocol - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYPE_PROTOCOL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "src\ng-compat\\" /I "..\..\include" /I "src\sdk\\" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYPE_PROTOCOL_EXPORTS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /dll /machine:I386 /out:"../../bin/release/plugins/skype.dll"

!ELSEIF  "$(CFG)" == "Skype_protocol - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYPE_PROTOCOL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "src\ng-compat\\" /I "..\..\include" /I "src\sdk\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYPE_PROTOCOL_EXPORTS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "_DEBUG"
# ADD RSC /l 0xc07 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/plugins/Skype.dll" /pdbtype:sept
# SUBTRACT LINK32 /map

!ELSEIF  "$(CFG)" == "Skype_protocol - Win32 Debug UNICODE"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Skype_protocol___Win32_Debug_UNICODE"
# PROP BASE Intermediate_Dir "Skype_protocol___Win32_Debug_UNICODE"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Skype_protocol___Win32_Debug_UNICODE"
# PROP Intermediate_Dir "Skype_protocol___Win32_Debug_UNICODE"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYPE_PROTOCOL_EXPORTS" /YX /FD /GZ /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "src\ng-compat\\" /I "..\..\include" /I "src\sdk\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SKYPE_PROTOCOL_EXPORTS" /D "UNICODE" /D "_UNICODE" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc07 /d "_DEBUG"
# ADD RSC /l 0xc07 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug/plugins/Skype.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /dll /debug /machine:I386 /out:"../../bin/debug Unicode/plugins/Skype.dll" /pdbtype:sept
# SUBTRACT LINK32 /map

!ENDIF 

# Begin Target

# Name "Skype_protocol - Win32 Release"
# Name "Skype_protocol - Win32 Debug"
# Name "Skype_protocol - Win32 Debug UNICODE"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\alogon.c
# End Source File
# Begin Source File

SOURCE=.\src\contacts.c
# End Source File
# Begin Source File

SOURCE=.\src\debug.c
# End Source File
# Begin Source File

SOURCE=.\src\gchat.c
# End Source File
# Begin Source File

SOURCE=.\src\memlist.c
# End Source File
# Begin Source File

SOURCE=.\src\msglist.c
# End Source File
# Begin Source File

SOURCE=.\src\msgq.c
# End Source File
# Begin Source File

SOURCE=.\src\pthread.c
# End Source File
# Begin Source File

SOURCE=.\src\skype.c
# End Source File
# Begin Source File

SOURCE=.\src\skypeapi.c
# End Source File
# Begin Source File

SOURCE=.\src\skypeopt.c
# End Source File
# Begin Source File

SOURCE=.\src\skypeprofile.c
# End Source File
# Begin Source File

SOURCE=.\src\skypesvc.c
# End Source File
# Begin Source File

SOURCE=.\src\utf8.c
# End Source File
# Begin Source File

SOURCE=.\src\util.c
# End Source File
# Begin Source File

SOURCE=.\src\voiceservice.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\alogon.h
# End Source File
# Begin Source File

SOURCE=.\src\contacts.h
# End Source File
# Begin Source File

SOURCE=.\src\debug.h
# End Source File
# Begin Source File

SOURCE=.\src\gchat.h
# End Source File
# Begin Source File

SOURCE=.\src\m_popup.h
# End Source File
# Begin Source File

SOURCE=.\src\m_toptoolbar.h
# End Source File
# Begin Source File

SOURCE=.\src\memlist.h
# End Source File
# Begin Source File

SOURCE=.\src\msglist.h
# End Source File
# Begin Source File

SOURCE=.\src\msgq.h
# End Source File
# Begin Source File

SOURCE=.\src\pthread.h
# End Source File
# Begin Source File

SOURCE=.\src\skype.h
# End Source File
# Begin Source File

SOURCE=.\src\skypeapi.h
# End Source File
# Begin Source File

SOURCE=.\src\skypeopt.h
# End Source File
# Begin Source File

SOURCE=.\src\skypeprofile.h
# End Source File
# Begin Source File

SOURCE=.\src\skypesvc.h
# End Source File
# Begin Source File

SOURCE=.\src\utf8.h
# End Source File
# Begin Source File

SOURCE=.\src\util.h
# End Source File
# Begin Source File

SOURCE=.\src\voiceservice.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\add.ico
# End Source File
# Begin Source File

SOURCE=.\res\away.ico
# End Source File
# Begin Source File

SOURCE=.\res\call.bmp
# End Source File
# Begin Source File

SOURCE=.\res\call.ico
# End Source File
# Begin Source File

SOURCE=.\res\chat.ico
# End Source File
# Begin Source File

SOURCE=.\res\DND.ico
# End Source File
# Begin Source File

SOURCE=.\res\error.ico
# End Source File
# Begin Source File

SOURCE=.\res\hang_up.ico
# End Source File
# Begin Source File

SOURCE=.\res\hold.ico
# End Source File
# Begin Source File

SOURCE=.\res\import.ico
# End Source File
# Begin Source File

SOURCE=.\res\invisible.ico
# End Source File
# Begin Source File

SOURCE=.\res\Invite.ico
# End Source File
# Begin Source File

SOURCE=.\res\message.ico
# End Source File
# Begin Source File

SOURCE=.\res\NA.ico
# End Source File
# Begin Source File

SOURCE=.\res\occupied.ico
# End Source File
# Begin Source File

SOURCE=.\res\offline.ico
# End Source File
# Begin Source File

SOURCE=.\res\online.ico
# End Source File
# Begin Source File

SOURCE=.\res\phone.ico
# End Source File
# Begin Source File

SOURCE=.\res\Resource.rc
# End Source File
# Begin Source File

SOURCE=.\res\resume.ico
# End Source File
# Begin Source File

SOURCE=.\res\skype.ico
# End Source File
# Begin Source File

SOURCE=.\res\skypeout.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\changelog.txt
# End Source File
# End Target
# End Project
