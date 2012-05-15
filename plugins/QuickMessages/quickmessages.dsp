# Microsoft Developer Studio Project File - Name="quickmessages" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=quickmessages - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "quickmessages.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "quickmessages.mak" CFG="quickmessages - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "quickmessages - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "quickmessages - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "quickmessages - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "quickmessages - Win32 Unicode Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "quickmessages - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /Ob2 /GF PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MD /W3 /GX /O1 /Ob2 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msimg32.lib /nologo /base:"0x67100000" /subsystem:windows /dll /map /machine:IX86 /out:".\$(ConfigurationName)\quickmessages.dll" /pdbtype:sept /filealign:0x200
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msimg32.lib /nologo /base:"0x67100000" /subsystem:windows /dll /map /machine:IX86 /pdbtype:sept /filealign:0x200
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "quickmessages - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\debug"
# PROP BASE Intermediate_Dir ".\debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /ZI /Od /FR /GZ PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MD /W3 /GX /ZI /Od /FR /GZ PRECOMP_VC7_TOBEREMOVED /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msimg32.lib /nologo /base:"0x67100000" /subsystem:windows /dll /incremental:no /pdb:".\release\quickmessages.pdb" /map /debug /machine:IX86 /out:"$(ConfigurationName)\quickmessages.dll" /implib:".\release/quickmessages.lib" /pdbtype:sept /filealign:0x200
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msimg32.lib /nologo /base:"0x67100000" /subsystem:windows /dll /incremental:no /pdb:".\release\quickmessages.pdb" /map /debug /machine:IX86 /out:"$(ConfigurationName)\quickmessages.dll" /implib:".\release/quickmessages.lib" /pdbtype:sept /filealign:0x200
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "quickmessages - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\UNICODE_RELEASE"
# PROP Intermediate_Dir ".\UNICODE_RELEASE"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /Ob2 /D "_UNICODE" /Fo".\Release/" /GF PRECOMP_VC7_TOBEREMOVED /c
# ADD CPP /nologo /MD /W3 /GX /O2 /Ob0 /D "_UNICODE" /D "UNICODE" /c
# SUBTRACT CPP /WX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msimg32.lib /nologo /base:"0x67100000" /subsystem:windows /dll /pdb:".\Release\quickmessages.pdb" /map:".\Release\quickmessages.map" /machine:IX86 /out:".\$(ConfigurationName)\quickmessages.dll" /implib:".\Release/quickmessages.lib" /pdbtype:sept /filealign:0x200
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msimg32.lib /nologo /base:"0x67100000" /subsystem:windows /dll /map /machine:IX86 /pdbtype:sept /filealign:0x200
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "quickmessages - Win32 Unicode Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\UNICODE_DEBUG"
# PROP Intermediate_Dir ".\UNICODE_DEBUG"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /ZI /Od /D "_UNICODE" /D "PRECOMP_VC7_TOBEREMOVED" /FR /Fo".\debug/" /Fd".\debug/" /GZ "DEBUG" /c
# ADD CPP /nologo /MD /W3 /GX /ZI /Od /D "_UNICODE" /D "UNICODE" /FR /GZ "UNICODE_DEBUG" /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x67100000" /subsystem:windows /dll /incremental:no /pdb:".\release\quickmessages.pdb" /map:".\debug\quickmessages.map" /debug /machine:IX86 /implib:".\release/quickmessages.lib" /pdbtype:sept /filealign:0x200
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x67100000" /subsystem:windows /dll /incremental:no /map /debug /debugtype:both /machine:IX86 /pdbtype:sept /filealign:0x200
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "quickmessages - Win32 Release"
# Name "quickmessages - Win32 Debug"
# Name "quickmessages - Win32 Unicode Release"
# Name "quickmessages - Win32 Unicode Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=main.c
DEP_CPP_MAIN_=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\m_msg_buttonsbar.h"\
	".\m_updater.h"\
	".\quickmessages.h"\
	".\Utils.h"\
	

!IF  "$(CFG)" == "quickmessages - Win32 Release"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "quickmessages - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "quickmessages - Win32 Unicode Release"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "quickmessages - Win32 Unicode Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=options.c
DEP_CPP_OPTIO=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\m_msg_buttonsbar.h"\
	".\m_updater.h"\
	".\quickmessages.h"\
	".\Utils.h"\
	

!IF  "$(CFG)" == "quickmessages - Win32 Release"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "quickmessages - Win32 Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ELSEIF  "$(CFG)" == "quickmessages - Win32 Unicode Release"

# ADD CPP /nologo /GX /O1

!ELSEIF  "$(CFG)" == "quickmessages - Win32 Unicode Debug"

# ADD CPP /nologo /GX /Od /FR /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Utils.c
DEP_CPP_UTILS=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_message.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\m_msg_buttonsbar.h"\
	".\m_updater.h"\
	".\quickmessages.h"\
	".\Utils.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\m_msg_buttonsbar.h
# End Source File
# Begin Source File

SOURCE=.\m_updater.h
# End Source File
# Begin Source File

SOURCE=.\quickmessages.h
# End Source File
# Begin Source File

SOURCE=resource.h
# End Source File
# Begin Source File

SOURCE=.\Utils.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icons\qicon.ico
# End Source File
# Begin Source File

SOURCE=.\quickmessages.rc
# End Source File
# End Group
# End Target
# End Project
