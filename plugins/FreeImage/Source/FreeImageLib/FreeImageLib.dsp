# Microsoft Developer Studio Project File - Name="FreeImageLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=FreeImageLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "FreeImageLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FreeImageLib.mak" CFG="FreeImageLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FreeImageLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "FreeImageLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "FreeImageLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\\" /I "..\ZLib" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "FREEIMAGE_LIB" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\FreeImage.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release\FreeImage.lib ..\..\Dist	copy ..\FreeImage.h ..\..\Dist
# End Special Build Tool

!ELSEIF  "$(CFG)" == "FreeImageLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\\" /I "..\ZLib" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "FREEIMAGE_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\FreeImaged.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Debug\FreeImaged.lib ..\..\Dist	copy ..\FreeImage.h ..\..\Dist
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "FreeImageLib - Win32 Release"
# Name "FreeImageLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Quantizers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\FreeImage\NNQuantizer.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\WuQuantizer.cpp
# End Source File
# End Group
# Begin Group "Conversion"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\FreeImage\Conversion.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\Conversion16_555.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\Conversion16_565.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\Conversion24.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\Conversion32.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\Conversion4.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\Conversion8.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\ConversionRGBF.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\ConversionType.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\Halftoning.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\tmoColorConvert.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\tmoDrago03.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\tmoReinhard05.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\ToneMapping.cpp
# End Source File
# End Group
# Begin Group "Plugins"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\FreeImage\Plugin.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginBMP.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginCUT.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginDDS.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginG3.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginGIF.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginHDR.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginICO.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginIFF.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginJPEG.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginKOALA.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginMNG.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginPCD.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginPCX.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginPNG.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginPNM.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginPSD.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginRAS.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginSGI.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginTARGA.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginTIFF.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginWBMP.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginXBM.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PluginXPM.cpp
# End Source File
# End Group
# Begin Group "DeprecationMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\DeprecationManager\DeprecationMgr.cpp
# End Source File
# End Group
# Begin Group "MultiPaging"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\FreeImage\CacheFile.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\MultiPage.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\ZLibInterface.cpp
# End Source File
# End Group
# Begin Group "Metadata"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Metadata\Exif.cpp
# End Source File
# Begin Source File

SOURCE=..\Metadata\FIRational.cpp
# End Source File
# Begin Source File

SOURCE=..\Metadata\FreeImageTag.cpp
# End Source File
# Begin Source File

SOURCE=..\Metadata\IPTC.cpp
# End Source File
# Begin Source File

SOURCE=..\Metadata\TagConversion.cpp
# End Source File
# Begin Source File

SOURCE=..\Metadata\TagLib.cpp
# End Source File
# Begin Source File

SOURCE=..\Metadata\XTIFF.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\FreeImage\BitmapAccess.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\ColorLookup.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\FreeImage.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\FreeImageIO.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\GetType.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\MemoryIO.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImage\PixelAccess.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\CacheFile.h
# End Source File
# Begin Source File

SOURCE=..\DeprecationManager\DeprecationMgr.h
# End Source File
# Begin Source File

SOURCE=..\Metadata\FIRational.h
# End Source File
# Begin Source File

SOURCE=..\FreeImage.h
# End Source File
# Begin Source File

SOURCE=..\FreeImageIO.h
# End Source File
# Begin Source File

SOURCE=..\Metadata\FreeImageTag.h
# End Source File
# Begin Source File

SOURCE=..\Plugin.h
# End Source File
# Begin Source File

SOURCE=..\Quantizers.h
# End Source File
# Begin Source File

SOURCE=..\ToneMapping.h
# End Source File
# Begin Source File

SOURCE=..\Utilities.h
# End Source File
# End Group
# Begin Group "Toolkit Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\FreeImageToolkit\BSplineRotate.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImageToolkit\Channels.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImageToolkit\ClassicRotate.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImageToolkit\Colors.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImageToolkit\CopyPaste.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImageToolkit\Display.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImageToolkit\Flip.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImageToolkit\JPEGTransform.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImageToolkit\Rescale.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImageToolkit\Resize.cpp
# End Source File
# Begin Source File

SOURCE=..\FreeImageToolkit\Resize.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Whatsnew.txt
# End Source File
# End Target
# End Project
