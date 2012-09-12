# Microsoft Developer Studio Project File - Name="NoHistory" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=NoHistory - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NoHistory_8.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NoHistory_8.mak" CFG="NoHistory - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NoHistory - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "NoHistory - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NoHistory - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "$(SolutionDir)$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "$(SolutionDir)$(ConfigurationName)"
# PROP Intermediate_Dir "$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "NOHISTORY_EXPORTS" /D "_UNICODE" /YX /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "NOHISTORY_EXPORTS" /D "_UNICODE" /YX /GZ /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"..\..\bin\$(ConfigurationName)\plugins\$(ProjectName).dll" /implib:"$(IntDir)/$(TargetName).lib" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"..\..\bin\$(ConfigurationName)\plugins\$(ProjectName).dll" /implib:"$(IntDir)/$(TargetName).lib" /pdbtype:sept

!ELSEIF  "$(CFG)" == "NoHistory - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "$(SolutionDir)$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release/"
# PROP Intermediate_Dir "Release/"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "NOHISTORY_EXPORTS" /D "_UNICODE" /YX /c
# ADD CPP /nologo /MT /W3 /GX /Zi /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "NOHISTORY_EXPORTS" /D "_MBCS" /YX /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"..\..\bin\$(ConfigurationName)\plugins\$(ProjectName).dll" /implib:"$(IntDir)/$(TargetName).lib" /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"../../bin/Release/plugins/nohistory.dll" /pdbtype:sept /opt:ref /opt:icf
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "NoHistory - Win32 Debug"
# Name "NoHistory - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=.\dllmain.cpp
DEP_CPP_DLLMA=\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_updater.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\common.h"\
	".\icons.h"\
	".\options.h"\
	".\private.h"\
	
NODEP_CPP_DLLMA=\
	".\_plugins.h"\
	".\_system.h"\
	".\_utils.h"\
	".\tatusmodes.h"\
	

!IF  "$(CFG)" == "NoHistory - Win32 Debug"

# ADD CPP /nologo /GX /Yc"common.h" /GZ

!ELSEIF  "$(CFG)" == "NoHistory - Win32 Release"

# ADD CPP /nologo /GX /Yc"common.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\icons.cpp
DEP_CPP_ICONS=\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_updater.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\common.h"\
	".\icons.h"\
	
NODEP_CPP_ICONS=\
	".\_plugins.h"\
	".\_system.h"\
	".\_utils.h"\
	".\tatusmodes.h"\
	
# End Source File
# Begin Source File

SOURCE=.\options.cpp
DEP_CPP_OPTIO=\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_updater.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\common.h"\
	".\icons.h"\
	".\options.h"\
	
NODEP_CPP_OPTIO=\
	".\_plugins.h"\
	".\_system.h"\
	".\_utils.h"\
	".\tatusmodes.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\icons.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\private.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav"
# Begin Source File

SOURCE=.\history_clear.ico
# End Source File
# Begin Source File

SOURCE=.\history_disabled.ico
# End Source File
# Begin Source File

SOURCE=.\history_enabled.ico
# End Source File
# Begin Source File

SOURCE=.\NoHistory.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\private.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# End Target
# End Project
