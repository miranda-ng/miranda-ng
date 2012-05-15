# Microsoft Developer Studio Project File - Name="smileyadd" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=smileyadd - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "smileyadd.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "smileyadd.mak" CFG="smileyadd - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "smileyadd - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "smileyadd - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "smileyadd - Win32 Unicode Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "smileyadd - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "smileyadd___Win32_Release"
# PROP BASE Intermediate_Dir "smileyadd___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "smileyadd___Win32_Release"
# PROP Intermediate_Dir "smileyadd___Win32_Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SMILEYADD_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O1 /I "..\..\include" /I "regexp" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D _WIN32_WINDOWS=0x0401 /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 advapi32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib ole32.lib uuid.lib comctl32.lib delayimp.lib gdiplus.lib version.lib /nologo /version:1.1011 /dll /map /machine:I386 /DELAYLOAD:gdiplus.dll /DELAY:UNLOAD /filealign:0x200
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "smileyadd - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "smileyadd___Win32_Debug"
# PROP BASE Intermediate_Dir "smileyadd___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "smileyadd___Win32_Debug"
# PROP Intermediate_Dir "smileyadd___Win32_Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SMILEYADD_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "regexp" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D _WIN32_WINDOWS=0x0401 /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 advapi32.lib oleaut32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib ole32.lib uuid.lib comctl32.lib delayimp.lib gdiplus.lib /nologo /version:1.1011 /dll /debug /machine:I386 /pdbtype:sept /DELAYLOAD:gdiplus.dll /DELAY:UNLOAD
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "smileyadd - Win32 Unicode Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "smileyadd___Win32_Unicode_Release"
# PROP BASE Intermediate_Dir "smileyadd___Win32_Unicode_Release"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "smileyadd___Win32_Unicode_Release"
# PROP Intermediate_Dir "smileyadd___Win32_Unicode_Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D REGEX_FOLD_INSTANTIATIONS=1 /FD /c
# SUBTRACT BASE CPP /YX /Yc /Yu
# ADD CPP /nologo /MD /W3 /O1 /I "..\..\include" /I "regexp" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "UNICODE" /D "_USRDLL" /D _WIN32_WINDOWS=0x0401 /FAcs /FR /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winspool.lib advapi32.lib oleaut32.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib shell32.lib ole32.lib uuid.lib comctl32.lib delayimp.lib gdiplus.lib /nologo /version:1.72 /dll /machine:I386 /DELAYLOAD:gdiplus.dll
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 oleaut32.lib kernel32.lib shell32.lib user32.lib gdi32.lib comdlg32.lib comctl32.lib ole32.lib uuid.lib delayimp.lib gdiplus.lib /nologo /version:1.1011 /dll /map /machine:I386 /out:"smileyadd___Win32_Unicode_Release/smileyaddw.dll" /DELAYLOAD:gdiplus.dll /DELAY:UNLOAD /filealign:0x200
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "smileyadd - Win32 Release"
# Name "smileyadd - Win32 Debug"
# Name "smileyadd - Win32 Unicode Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\anim.cpp
# End Source File
# Begin Source File

SOURCE=.\AniSmileyObject.cpp
# End Source File
# Begin Source File

SOURCE=.\bkstring.cpp
# End Source File
# Begin Source File

SOURCE=.\customsmiley.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgboxsubclass.cpp
# End Source File
# Begin Source File

SOURCE=.\download.cpp
# End Source File
# Begin Source File

SOURCE=.\general.cpp
# End Source File
# Begin Source File

SOURCE=.\imagecache.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\options.cpp
# End Source File
# Begin Source File

SOURCE=.\richcall.cpp
# End Source File
# Begin Source File

SOURCE=.\services.cpp
# End Source File
# Begin Source File

SOURCE=.\SmileyBase.cpp
# End Source File
# Begin Source File

SOURCE=.\smileyroutines.cpp
# End Source File
# Begin Source File

SOURCE=.\smileys.cpp
# End Source File
# Begin Source File

SOURCE=.\smltool.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\anim.h
# End Source File
# Begin Source File

SOURCE=.\bkstring.h
# End Source File
# Begin Source File

SOURCE=.\customsmiley.h
# End Source File
# Begin Source File

SOURCE=.\download.h
# End Source File
# Begin Source File

SOURCE=.\general.h
# End Source File
# Begin Source File

SOURCE=.\m_smileyadd.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\services.h
# End Source File
# Begin Source File

SOURCE=.\SmileyBase.h
# End Source File
# Begin Source File

SOURCE=.\smileyroutines.h
# End Source File
# Begin Source File

SOURCE=.\smileys.h
# End Source File
# Begin Source File

SOURCE=.\smltool.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\Res\smiley.ico
# End Source File
# End Group
# End Target
# End Project
