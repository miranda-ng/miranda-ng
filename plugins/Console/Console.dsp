# Microsoft Developer Studio Project File - Name="Console" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Console - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Console.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Console.mak" CFG="Console - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Console - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Console - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Console - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Console - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Console - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Zi /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "Console_EXPORTS" /D "_MBCS" /FR /Yu"Commonheaders.h" /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "Console_EXPORTS" /D "_MBCS" /FR /Yu"Commonheaders.h" /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /map /debug /machine:IX86 /out:"..\..\Bin\Debug\plugins\Console.dll" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /map /debug /machine:IX86 /out:"..\..\Bin\Debug\plugins\Console.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Console - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /Ox /Ob2 /Gy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "Console_EXPORTS" /D "_MBCS" /FR /Yu"Commonheaders.h" /GF /c
# ADD CPP /nologo /MD /W3 /Zi /Ox /Ot /Os /Ob2 /Gf /Gy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "Console_EXPORTS" /D "_MBCS" /FR /Yu"Commonheaders.h" /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\..\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /map /machine:IX86 /out:"..\..\bin\release\plugins\Console.dll" /pdbtype:sept /ALIGN:4096
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /map /machine:IX86 /out:"..\..\bin\release\plugins\Console.dll" /pdbtype:sept /ALIGN:4096
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Console - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Zi /Od /I "../../include" /D "_UNICODE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "Console_EXPORTS" /FR /Fp".\Debug/Console.pch" /Yu"Commonheaders.h" /Fo".\Debug/" /Fd".\Debug/" /GZ /c
# ADD CPP /nologo /MDd /W3 /GX /Zi /Od /I "../../include" /D "_UNICODE" /D "UNICODE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "Console_EXPORTS" /FR /Fp".\Debug/Console.pch" /Yu"Commonheaders.h" /Fo".\Debug/" /Fd".\Debug/" /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\include" /d "_DEBUG"
# ADD RSC /l 0x409 /i "..\..\include" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:".\Debug\Console.pdb" /map:".\Debug\Console.map" /debug /machine:IX86 /out:"..\..\Bin\Debug Unicode\plugins\Console.dll" /implib:".\Debug/Console.lib" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:".\Debug\Console.pdb" /map:".\Debug\Console.map" /debug /machine:IX86 /out:"..\..\Bin\Debug Unicode\plugins\Console.dll" /implib:".\Debug/Console.lib" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Console - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Ox /Ob2 /Gy /I "../../include" /D "_UNICODE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "Console_EXPORTS" /FR /Fo".\Release/" /GF /c
# ADD CPP /nologo /MD /W3 /Zi /Ox /Og /Gf /Gy /I "../../include" /D "_UNICODE" /D "UNICODE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "Console_EXPORTS" /Fr /Fo".\Release/" /Fd".\Release/" /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /i "..\..\include" /d "NDEBUG"
# ADD RSC /l 0x409 /i "..\..\include" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:".\Release\Console.pdb" /map /machine:IX86 /out:"..\..\bin\release unicode\plugins\Console.dll" /implib:".\Release/Console.lib" /pdbtype:sept /ALIGN:4096
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /map /debug /machine:IX86 /out:"Release_Unicode\Plugins\Console.dll" /implib:".\Release/Console.lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Console - Win32 Debug"
# Name "Console - Win32 Release"
# Name "Console - Win32 Debug Unicode"
# Name "Console - Win32 Release Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=Commonheaders.c
DEP_CPP_COMMO=\
	".\Commonheaders.h"\
	".\m_toolbar.h"\
	".\m_toptoolbar.h"\
	".\version.h"\
	{$(INCLUDE)}"m_button.h"\
	{$(INCLUDE)}"m_clist.h"\
	{$(INCLUDE)}"m_clui.h"\
	{$(INCLUDE)}"m_database.h"\
	{$(INCLUDE)}"m_fontservice.h"\
	{$(INCLUDE)}"m_hotkeys.h"\
	{$(INCLUDE)}"m_icolib.h"\
	{$(INCLUDE)}"m_langpack.h"\
	{$(INCLUDE)}"m_netlib.h"\
	{$(INCLUDE)}"m_options.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"newpluginapi.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"win2k.h"\
	

!IF  "$(CFG)" == "Console - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /Yc"Commonheaders.h" /GZ

!ELSEIF  "$(CFG)" == "Console - Win32 Release"

# ADD CPP /nologo /GX /O2 /FR /Yc"Commonheaders.h"

!ELSEIF  "$(CFG)" == "Console - Win32 Debug Unicode"

# ADD CPP /nologo /GX /Od /FR /Yc"Commonheaders.h" /GZ

!ELSEIF  "$(CFG)" == "Console - Win32 Release Unicode"

# ADD CPP /nologo /GX /O2 /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=Console.c
DEP_CPP_CONSO=\
	".\Commonheaders.h"\
	".\m_toolbar.h"\
	".\m_toptoolbar.h"\
	".\version.h"\
	{$(INCLUDE)}"m_button.h"\
	{$(INCLUDE)}"m_clist.h"\
	{$(INCLUDE)}"m_clui.h"\
	{$(INCLUDE)}"m_database.h"\
	{$(INCLUDE)}"m_fontservice.h"\
	{$(INCLUDE)}"m_hotkeys.h"\
	{$(INCLUDE)}"m_icolib.h"\
	{$(INCLUDE)}"m_langpack.h"\
	{$(INCLUDE)}"m_netlib.h"\
	{$(INCLUDE)}"m_options.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"newpluginapi.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"win2k.h"\
	

!IF  "$(CFG)" == "Console - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "Console - Win32 Release"

# ADD CPP /nologo /GX /O2 /FR

!ELSEIF  "$(CFG)" == "Console - Win32 Debug Unicode"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "Console - Win32 Release Unicode"

# ADD CPP /nologo /GX /O2 /FR

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\init.c
DEP_CPP_INIT_=\
	".\Commonheaders.h"\
	".\m_toolbar.h"\
	".\m_toptoolbar.h"\
	".\version.h"\
	{$(INCLUDE)}"m_button.h"\
	{$(INCLUDE)}"m_clist.h"\
	{$(INCLUDE)}"m_clui.h"\
	{$(INCLUDE)}"m_database.h"\
	{$(INCLUDE)}"m_fontservice.h"\
	{$(INCLUDE)}"m_hotkeys.h"\
	{$(INCLUDE)}"m_icolib.h"\
	{$(INCLUDE)}"m_langpack.h"\
	{$(INCLUDE)}"m_netlib.h"\
	{$(INCLUDE)}"m_options.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"newpluginapi.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"win2k.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=Commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\btn_dn.ico
# End Source File
# Begin Source File

SOURCE=.\res\btn_up.ico
# End Source File
# Begin Source File

SOURCE=.\res\close.ico
# End Source File
# Begin Source File

SOURCE=Res\console.ico
# End Source File
# Begin Source File

SOURCE=Res\console_dn.bmp
# End Source File
# Begin Source File

SOURCE=Res\console_up.bmp
# End Source File
# Begin Source File

SOURCE=.\Res\copy.ico
# End Source File
# Begin Source File

SOURCE=.\Res\delete.ico
# End Source File
# Begin Source File

SOURCE=.\res\noscroll.ico
# End Source File
# Begin Source File

SOURCE=.\Res\options.ico
# End Source File
# Begin Source File

SOURCE=.\res\pause.ico
# End Source File
# Begin Source File

SOURCE=.\res\paused.ico
# End Source File
# Begin Source File

SOURCE=resource.rc
# End Source File
# Begin Source File

SOURCE=.\Res\Save.ico
# End Source File
# Begin Source File

SOURCE=.\res\scroll.ico
# End Source File
# Begin Source File

SOURCE=.\res\start.ico
# End Source File
# Begin Source File

SOURCE=.\res\started.ico
# End Source File
# End Group
# End Target
# End Project
