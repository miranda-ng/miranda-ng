# Microsoft Developer Studio Project File - Name="mtextcontrol" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=mtextcontrol - Win32 Debug Unicode
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "mtextcontrol.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "mtextcontrol.mak" CFG="mtextcontrol - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "mtextcontrol - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mtextcontrol - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mtextcontrol - Win32 Int Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mtextcontrol - Win32 Debug Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mtextcontrol - Win32 Release Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Obj"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Yu"headers.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x422 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib riched20.lib /nologo /base:"0x25a20000" /dll /debug /machine:I386
# SUBTRACT LINK32 /map /nodefaultlib

!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\Obj"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Yu"headers.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x422 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib riched20.lib /nologo /base:"0x25a20000" /dll /map /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "mtextcontrol___Win32_Int_Release"
# PROP BASE Intermediate_Dir "mtextcontrol___Win32_Int_Release"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Int_Release"
# PROP Intermediate_Dir "Int_Release\Obj"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Yu"headers.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x422 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Int_Release/mtextcontrolW.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x25a20000" /dll /map /machine:I386
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib riched20.lib /nologo /base:"0x25a20000" /dll /map /debug /machine:I386 /out:"Int_Release/mtextcontrolW.dll" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "mtextcontrol___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "mtextcontrol___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode\Obj"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Yu"headers.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x422 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "UNICODE" /d "_UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug_Unicode/mtextcontrolW.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x25a20000" /dll /map /debug /machine:I386 /pdbtype:sept
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib riched20.lib /nologo /base:"0x25a20000" /dll /map /debug /machine:I386 /out:"Debug_Unicode/mtextcontrolW.dll" /pdbtype:sept
# SUBTRACT LINK32 /nodefaultlib

!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "mtextcontrol___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "mtextcontrol___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode\Obj"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Yu"headers.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x422 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "UNICODE" /d "_UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release_Unicode/mtextcontrolW.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x25a20000" /dll /map /machine:I386
# SUBTRACT BASE LINK32 /nodefaultlib
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib riched20.lib /nologo /base:"0x25a20000" /dll /debug /machine:I386 /out:"Release_Unicode/mtextcontrolW.dll"
# SUBTRACT LINK32 /map /nodefaultlib

!ENDIF 

# Begin Target

# Name "mtextcontrol - Win32 Release"
# Name "mtextcontrol - Win32 Debug"
# Name "mtextcontrol - Win32 Int Release"
# Name "mtextcontrol - Win32 Debug Unicode"
# Name "mtextcontrol - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\dataobject.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\src\enumformat.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\src\fancy_rtf.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\src\FormattedTextDraw.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\src\ImageDataObjectHlp.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# ADD CPP /Yc"headers.h"
# End Source File
# Begin Source File

SOURCE=.\src\richeditutils.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\src\services.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\src\textcontrol.cpp
# ADD CPP /Yu
# End Source File
# Begin Source File

SOURCE=.\src\textusers.cpp
# ADD CPP /Yu
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\fancy_rtf.h
# End Source File
# Begin Source File

SOURCE=.\src\FormattedTextDraw.h
# End Source File
# Begin Source File

SOURCE=.\src\headers.h
# End Source File
# Begin Source File

SOURCE=.\src\ImageDataObjectHlp.h
# End Source File
# Begin Source File

SOURCE=.\src\richeditutils.h
# End Source File
# Begin Source File

SOURCE=.\src\services.h
# End Source File
# Begin Source File

SOURCE=.\src\textcontrol.h
# End Source File
# Begin Source File

SOURCE=.\src\textusers.h
# End Source File
# Begin Source File

SOURCE=.\src\version.h
# End Source File
# End Group
# Begin Group "api"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\api\m_text.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\version.rc
# End Source File
# End Group
# End Target
# End Project
