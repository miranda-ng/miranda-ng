# Microsoft Developer Studio Project File - Name="libgpg_error_1_1" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libgpg_error_1_1 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libgpg_error_1_1.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libgpg_error_1_1.mak" CFG="libgpg_error_1_1 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libgpg_error_1_1 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libgpg_error_1_1 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libgpg_error_1_1 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin.vs\libgpg-error-1.1\release\static"
# PROP Intermediate_Dir "..\bin.vs\libgpg-error-1.1\release\static\obj"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /I ".\custom" /I "..\inc.vs" /I "..\..\libgpg-error-1.1\libgpg-error\src" /D "WIN32" /D "_WIN32" /D "_DEBUG" /D "_LIB" /D "HAVE_CONFIG_H" /FR /FD /c
# SUBTRACT CPP /O<none> /YX
# ADD BASE RSC /l 0x410 /d "NDEBUG"
# ADD RSC /l 0x410 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin.vs\libgpg-error-1.1\release\static\libgpg-error.lib"

!ELSEIF  "$(CFG)" == "libgpg_error_1_1 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin.vs\libgpg-error-1.1\debug\static"
# PROP Intermediate_Dir "..\bin.vs\libgpg-error-1.1\debug\static\obj"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /I ".\custom" /I "..\inc.vs" /I "..\..\libgpg-error-1.1\libgpg-error\src" /D "WIN32" /D "_WIN32" /D "_DEBUG" /D "_LIB" /D "HAVE_CONFIG_H" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x410 /d "_DEBUG"
# ADD RSC /l 0x410 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\bin.vs\libgpg-error-1.1\debug\static\libgpg-error.lib"

!ENDIF 

# Begin Target

# Name "libgpg_error_1_1 - Win32 Release"
# Name "libgpg_error_1_1 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\code-from-errno.c"
# End Source File
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\code-to-errno.c"
# End Source File
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\gpg-error.c"
# End Source File
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\strerror-sym.c"
# End Source File
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\strerror.c"
# End Source File
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\strsource-sym.c"
# End Source File
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\strsource.c"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\err-codes.h"
# End Source File
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\err-sources.h"
# End Source File
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\gettext.h"
# End Source File
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\gpg-error.h"
# End Source File
# End Group
# Begin Group "custom"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".\custom\code-from-errno.h"
# End Source File
# Begin Source File

SOURCE=".\custom\code-to-errno.h"
# End Source File
# Begin Source File

SOURCE=.\custom\config.h
# End Source File
# Begin Source File

SOURCE=".\custom\err-codes-sym.h"
# End Source File
# Begin Source File

SOURCE=".\custom\err-sources-sym.h"
# End Source File
# Begin Source File

SOURCE=".\custom\errnos-sym.h"
# End Source File
# Begin Source File

SOURCE=.\custom\mkerrcodes.h
# End Source File
# End Group
# Begin Group "removed"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\libgpg-error-1.1\libgpg-error\src\mkerrcodes.c"
# PROP Exclude_From_Build 1
# End Source File
# End Group
# End Target
# End Project
