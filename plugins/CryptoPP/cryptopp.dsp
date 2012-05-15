# Microsoft Developer Studio Project File - Name="cryptopp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=cryptopp - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "cryptopp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cryptopp.mak" CFG="cryptopp - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cryptopp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cryptopp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "cryptopp - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CRYPTOPP_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O1 /I "\source\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"commonheaders.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib /nologo /base:"0x46000000" /dll /map /machine:I386
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "cryptopp - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CRYPTOPP_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "\source\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CRYPTOPP_EXPORTS" /YX"commonheaders.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib /nologo /base:"0x46000000" /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "cryptopp - Win32 Release"
# Name "cryptopp - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\base16.cpp
# End Source File
# Begin Source File

SOURCE=.\base64.cpp
# End Source File
# Begin Source File

SOURCE=.\commonheaders.cpp

!IF  "$(CFG)" == "cryptopp - Win32 Release"

# ADD CPP /Yc"commonheaders.h"

!ELSEIF  "$(CFG)" == "cryptopp - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cpp_cntx.cpp
# End Source File
# Begin Source File

SOURCE=.\cpp_gpgw.cpp
# End Source File
# Begin Source File

SOURCE=.\cpp_gzip.cpp
# End Source File
# Begin Source File

SOURCE=.\cpp_keys.cpp
# End Source File
# Begin Source File

SOURCE=.\cpp_misc.cpp
# End Source File
# Begin Source File

SOURCE=.\cpp_pgpw.cpp
# End Source File
# Begin Source File

SOURCE=.\cpp_rsam.cpp
# End Source File
# Begin Source File

SOURCE=.\cpp_rsau.cpp
# End Source File
# Begin Source File

SOURCE=.\cpp_svcs.cpp
# End Source File
# Begin Source File

SOURCE=.\dllloader.cpp
# End Source File
# Begin Source File

SOURCE=.\gettime.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\mmi.cpp
# End Source File
# Begin Source File

SOURCE=.\utf8.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\base16.h
# End Source File
# Begin Source File

SOURCE=.\base64.h
# End Source File
# Begin Source File

SOURCE=.\commonheaders.h
# End Source File
# Begin Source File

SOURCE=.\cpp_rsam.h
# End Source File
# Begin Source File

SOURCE=.\cpp_rsau.h
# End Source File
# Begin Source File

SOURCE=.\cryptopp.h
# End Source File
# Begin Source File

SOURCE=.\dllloader.h
# End Source File
# Begin Source File

SOURCE=.\gettime.h
# End Source File
# Begin Source File

SOURCE=.\mmi.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\utf8.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# End Target
# End Project
