# Microsoft Developer Studio Project File - Name="PopUp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PopUp - Win32 Debug Unicode
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "PopUp.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "PopUp.mak" CFG="PopUp - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "PopUp - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PopUp - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PopUp - Win32 Debug Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PopUp - Win32 Release Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PopUp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release/Obj"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUP_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W2 /GX /Zi /O2 /I "../../include" /I "../../include_API" /I "./include_API" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUP_EXPORTS" /Yu"headers.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
# SUBTRACT RSC /x
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib gdiplus.lib comdlg32.lib comctl32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msimg32.lib /nologo /base:"0x55550000" /dll /debug /machine:I386 /nodefaultlib:"LIBC"
# SUBTRACT LINK32 /pdb:none /map

!ELSEIF  "$(CFG)" == "PopUp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug/Obj"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUP_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W2 /Gm /Gi /GX /ZI /Od /I "../../include" /I "../../include_API" /I "./include_API" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUP_EXPORTS" /Fr /Yu"headers.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug/obj/PopUp.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 msimg32.lib kernel32.lib user32.lib gdi32.lib gdiplus.lib comdlg32.lib comctl32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msimg32.lib /nologo /base:"0x55550000" /dll /debug /machine:I386 /pdbtype:sept /mapinfo:exports /mapinfo:lines
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib

!ELSEIF  "$(CFG)" == "PopUp - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "PopUp___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "PopUp___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode/Obj"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W2 /Gm /Gi /GX /ZI /Od /I "../../include" /I "../../include_API" /I "./include_API" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUP_EXPORTS" /Fr /Yu"headers.h" /FD /c
# ADD CPP /nologo /MTd /W2 /Gm /Gi /GX /ZI /Od /I "../../include" /I "../../include_API" /I "./include_API" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUP_EXPORTS" /D "_UNICODE" /D "UNICODE" /Fr /Yu"headers.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /D "_UNICODE" /D "UNICODE" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_UNICODE" /d "UNICODE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Debug/obj/PopUp.bsc"
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 msimg32.lib kernel32.lib user32.lib gdi32.lib gdiplus.lib comdlg32.lib comctl32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x55550000" /dll /debug /machine:I386 /pdbtype:sept /mapinfo:exports /mapinfo:lines
# SUBTRACT BASE LINK32 /pdb:none /map /nodefaultlib
# ADD LINK32 msimg32.lib kernel32.lib user32.lib gdi32.lib gdiplus.lib comdlg32.lib comctl32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msimg32.lib /nologo /base:"0x55550000" /dll /debug /machine:I386 /out:"Debug_Unicode/PopUpW.dll" /pdbtype:sept /mapinfo:exports /mapinfo:lines
# SUBTRACT LINK32 /pdb:none /map /nodefaultlib

!ELSEIF  "$(CFG)" == "PopUp - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "PopUp___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "PopUp___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode/Obj"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W2 /GX /Zi /O2 /I "../../include" /I "../../include_API" /I "./include_API" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUP_EXPORTS" /Yu"headers.h" /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W2 /GX /Zi /O2 /I "../../include" /I "../../include_API" /I "./include_API" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "POPUP_EXPORTS" /D "_UNICODE" /D "UNICODE" /Fr /Yu"headers.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# SUBTRACT BASE MTL /mktyplib203
# ADD MTL /nologo /D "NDEBUG" /D "_UNICODE" /D "UNICODE" /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# SUBTRACT BASE RSC /x
# ADD RSC /l 0x409 /d "NDEBUG" /d "_UNICODE" /d "UNICODE"
# SUBTRACT RSC /x
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib gdiplus.lib comdlg32.lib comctl32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x55550000" /dll /debug /machine:I386 /nodefaultlib:"LIBC"
# SUBTRACT BASE LINK32 /pdb:none /map
# ADD LINK32 kernel32.lib user32.lib gdi32.lib gdiplus.lib comdlg32.lib comctl32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib msimg32.lib /nologo /base:"0x55550000" /dll /debug /machine:I386 /nodefaultlib:"LIBC" /out:"Release_Unicode/PopUpW.dll"
# SUBTRACT LINK32 /pdb:none /map

!ENDIF 

# Begin Target

# Name "PopUp - Win32 Release"
# Name "PopUp - Win32 Debug"
# Name "PopUp - Win32 Debug Unicode"
# Name "PopUp - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\actions.cpp
# End Source File
# Begin Source File

SOURCE=.\src\avatars.cpp
# End Source File
# Begin Source File

SOURCE=.\src\avatars_flash.cpp
# End Source File
# Begin Source File

SOURCE=.\src\avatars_gif.cpp
# End Source File
# Begin Source File

SOURCE=.\src\avatars_simple.cpp
# End Source File
# Begin Source File

SOURCE=.\src\bitmap_funcs.cpp
# End Source File
# Begin Source File

SOURCE=.\src\config.cpp
# End Source File
# Begin Source File

SOURCE=.\src\effects.cpp
# End Source File
# Begin Source File

SOURCE=.\src\font.cpp
# End Source File
# Begin Source File

SOURCE=.\src\formula.cpp
# End Source File
# Begin Source File

SOURCE=.\src\headers.cpp
# ADD CPP /Yc"headers.h"
# End Source File
# Begin Source File

SOURCE=.\src\history.cpp
# End Source File
# Begin Source File

SOURCE=.\src\icons.cpp
# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\notifications.cpp
# End Source File
# Begin Source File

SOURCE=.\src\popup_gdiplus.cpp
# End Source File
# Begin Source File

SOURCE=.\src\popup_thread.cpp
# End Source File
# Begin Source File

SOURCE=.\src\popup_wnd2.cpp
# End Source File
# Begin Source File

SOURCE=.\src\services.cpp
# End Source File
# Begin Source File

SOURCE=.\src\skin.cpp
# End Source File
# Begin Source File

SOURCE=.\src\srmm_menu.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\actions.h
# End Source File
# Begin Source File

SOURCE=.\src\avatars.h
# End Source File
# Begin Source File

SOURCE=.\src\avatars_flash.h
# End Source File
# Begin Source File

SOURCE=.\src\avatars_gif.h
# End Source File
# Begin Source File

SOURCE=.\src\avatars_simple.h
# End Source File
# Begin Source File

SOURCE=.\src\bitmap_funcs.h
# End Source File
# Begin Source File

SOURCE=.\src\effects.h
# End Source File
# Begin Source File

SOURCE=.\src\font.h
# End Source File
# Begin Source File

SOURCE=.\src\formula.h
# End Source File
# Begin Source File

SOURCE=.\src\history.h
# End Source File
# Begin Source File

SOURCE=.\src\icons.h
# End Source File
# Begin Source File

SOURCE=.\src\notifications.h
# End Source File
# Begin Source File

SOURCE=.\src\popup_gdiplus.h
# End Source File
# Begin Source File

SOURCE=.\src\popup_thread.h
# End Source File
# Begin Source File

SOURCE=.\src\popup_wnd2.h
# End Source File
# Begin Source File

SOURCE=.\src\services.h
# End Source File
# Begin Source File

SOURCE=.\src\skin.h
# End Source File
# Begin Source File

SOURCE=.\src\srmm_menu.h
# End Source File
# End Group
# Begin Group "defs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\config.h
# End Source File
# Begin Source File

SOURCE=.\src\def_settings.h
# End Source File
# Begin Source File

SOURCE=.\src\defs.h
# End Source File
# Begin Source File

SOURCE=.\src\headers.h
# End Source File
# Begin Source File

SOURCE=..\..\include\m_avatars.h
# End Source File
# Begin Source File

SOURCE=..\..\include_API\m_folders.h
# End Source File
# Begin Source File

SOURCE=..\..\include\m_fontservice.h
# End Source File
# Begin Source File

SOURCE=..\..\include_API\m_ieview.h
# End Source File
# Begin Source File

SOURCE=.\api\m_MathModule.h
# End Source File
# Begin Source File

SOURCE=..\..\include_API\m_nconvers.h
# End Source File
# Begin Source File

SOURCE=..\..\include_API\m_notify_popup.h
# End Source File
# Begin Source File

SOURCE=..\..\include\m_popup.h
# End Source File
# Begin Source File

SOURCE=.\api\m_popup2.h
# End Source File
# Begin Source File

SOURCE=..\..\include_API\m_smileyadd.h
# End Source File
# Begin Source File

SOURCE=..\..\include_API\m_text.h
# End Source File
# Begin Source File

SOURCE=..\..\include_API\m_ticker.h
# End Source File
# Begin Source File

SOURCE=..\..\include_API\m_toolbar.h
# End Source File
# Begin Source File

SOURCE=..\..\include_API\m_toptoolbar.h
# End Source File
# Begin Source File

SOURCE=..\..\include_API\m_updater.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\add.ico
# End Source File
# Begin Source File

SOURCE=.\res\auto.ico
# End Source File
# Begin Source File

SOURCE=.\res\avatar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\block.ico
# End Source File
# Begin Source File

SOURCE=.\res\Check_off.ico
# End Source File
# Begin Source File

SOURCE=.\res\Check_on.ico
# End Source File
# Begin Source File

SOURCE=.\res\classic.popupskin
# End Source File
# Begin Source File

SOURCE=.\res\close.ico
# End Source File
# Begin Source File

SOURCE=.\res\error.ico
# End Source File
# Begin Source File

SOURCE=.\res\favorite.ico
# End Source File
# Begin Source File

SOURCE=.\res\fullscreen.ico
# End Source File
# Begin Source File

SOURCE=.\res\img.ico
# End Source File
# Begin Source File

SOURCE=.\res\info.ico
# End Source File
# Begin Source File

SOURCE=.\res\menu.ico
# End Source File
# Begin Source File

SOURCE=.\res\message.ico
# End Source File
# Begin Source File

SOURCE=.\res\notify.ico
# End Source File
# Begin Source File

SOURCE=.\res\ok.ico
# End Source File
# Begin Source File

SOURCE=.\res\pin.ico
# End Source File
# Begin Source File

SOURCE=.\res\pinned.ico
# End Source File
# Begin Source File

SOURCE=.\res\popup.ico
# End Source File
# Begin Source File

SOURCE=.\res\popup_fav.ico
# End Source File
# Begin Source File

SOURCE=.\res\popup_fs.ico
# End Source File
# Begin Source File

SOURCE=.\res\popup_group.ico
# End Source File
# Begin Source File

SOURCE=.\res\popup_history.ico
# End Source File
# Begin Source File

SOURCE=.\res\popup_no.ico
# End Source File
# Begin Source File

SOURCE=.\res\popup_plus.ico
# End Source File
# Begin Source File

SOURCE=.\res\Radio_off.ico
# End Source File
# Begin Source File

SOURCE=.\res\Radio_on.ico
# End Source File
# Begin Source File

SOURCE=.\res\reload.ico
# End Source File
# Begin Source File

SOURCE=.\res\reply.ico
# End Source File
# Begin Source File

SOURCE=.\res_vc6.rc
# End Source File
# Begin Source File

SOURCE=.\res\resize.ico
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\res\warning.ico
# End Source File
# End Group
# Begin Group "docs"

# PROP Default_Filter "txt"
# Begin Source File

SOURCE=.\docs\Changelog.txt
# End Source File
# Begin Source File

SOURCE=.\res\whatsnew.txt
# End Source File
# End Group
# Begin Group "Option Pages"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\opt_adv.cpp
# End Source File
# Begin Source File

SOURCE=.\src\opt_adv.h
# End Source File
# Begin Source File

SOURCE=.\src\opt_class.cpp
# End Source File
# Begin Source File

SOURCE=.\src\opt_class.h
# End Source File
# Begin Source File

SOURCE=.\src\opt_contacts.cpp
# End Source File
# Begin Source File

SOURCE=.\src\opt_contacts.h
# End Source File
# Begin Source File

SOURCE=.\src\opt_gen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\opt_gen.h
# End Source File
# Begin Source File

SOURCE=.\src\opt_old.cpp
# End Source File
# Begin Source File

SOURCE=.\src\opt_old.h
# End Source File
# Begin Source File

SOURCE=.\src\opt_skins.cpp
# End Source File
# Begin Source File

SOURCE=.\src\opt_skins.h
# End Source File
# Begin Source File

SOURCE=.\src\opttree.cpp
# End Source File
# Begin Source File

SOURCE=.\src\opttree.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\common.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Target
# End Project
