# Microsoft Developer Studio Generated NMAKE File, Based on FreeImage.dsp
!IF "$(CFG)" == ""
CFG=FreeImage - Win32 Debug
!MESSAGE No configuration specified. Defaulting to FreeImage - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "FreeImage - Win32 Release" && "$(CFG)" != "FreeImage - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "FreeImage.mak" CFG="FreeImage - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "FreeImage - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "FreeImage - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "FreeImage - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : "..\..\bin\release\plugins\advaimg.dll"

!ELSE 

ALL : "zlib - Win32 Release" "..\..\bin\release\plugins\advaimg.dll"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"zlib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\BitmapAccess.obj"
	-@erase "$(INTDIR)\BSplineRotate.obj"
	-@erase "$(INTDIR)\CacheFile.obj"
	-@erase "$(INTDIR)\Channels.obj"
	-@erase "$(INTDIR)\ClassicRotate.obj"
	-@erase "$(INTDIR)\ColorLookup.obj"
	-@erase "$(INTDIR)\Colors.obj"
	-@erase "$(INTDIR)\Conversion.obj"
	-@erase "$(INTDIR)\Conversion16_555.obj"
	-@erase "$(INTDIR)\Conversion16_565.obj"
	-@erase "$(INTDIR)\Conversion24.obj"
	-@erase "$(INTDIR)\Conversion32.obj"
	-@erase "$(INTDIR)\Conversion4.obj"
	-@erase "$(INTDIR)\Conversion8.obj"
	-@erase "$(INTDIR)\ConversionFloat.obj"
	-@erase "$(INTDIR)\ConversionRGB16.obj"
	-@erase "$(INTDIR)\ConversionRGBF.obj"
	-@erase "$(INTDIR)\ConversionType.obj"
	-@erase "$(INTDIR)\ConversionUINT16.obj"
	-@erase "$(INTDIR)\CopyPaste.obj"
	-@erase "$(INTDIR)\Display.obj"
	-@erase "$(INTDIR)\Exif.obj"
	-@erase "$(INTDIR)\FIRational.obj"
	-@erase "$(INTDIR)\Flip.obj"
	-@erase "$(INTDIR)\FreeImage.obj"
	-@erase "$(INTDIR)\FreeImage.res"
	-@erase "$(INTDIR)\FreeImageC.obj"
	-@erase "$(INTDIR)\FreeImageIO.obj"
	-@erase "$(INTDIR)\FreeImageTag.obj"
	-@erase "$(INTDIR)\GetType.obj"
	-@erase "$(INTDIR)\Halftoning.obj"
	-@erase "$(INTDIR)\IPTC.obj"
	-@erase "$(INTDIR)\jaricom.obj"
	-@erase "$(INTDIR)\jcapimin.obj"
	-@erase "$(INTDIR)\jcapistd.obj"
	-@erase "$(INTDIR)\jcarith.obj"
	-@erase "$(INTDIR)\jccoefct.obj"
	-@erase "$(INTDIR)\jccolor.obj"
	-@erase "$(INTDIR)\jcdctmgr.obj"
	-@erase "$(INTDIR)\jchuff.obj"
	-@erase "$(INTDIR)\jcinit.obj"
	-@erase "$(INTDIR)\jcmainct.obj"
	-@erase "$(INTDIR)\jcmarker.obj"
	-@erase "$(INTDIR)\jcmaster.obj"
	-@erase "$(INTDIR)\jcomapi.obj"
	-@erase "$(INTDIR)\jcparam.obj"
	-@erase "$(INTDIR)\jcprepct.obj"
	-@erase "$(INTDIR)\jcsample.obj"
	-@erase "$(INTDIR)\jctrans.obj"
	-@erase "$(INTDIR)\jdapimin.obj"
	-@erase "$(INTDIR)\jdapistd.obj"
	-@erase "$(INTDIR)\jdarith.obj"
	-@erase "$(INTDIR)\jdatadst.obj"
	-@erase "$(INTDIR)\jdatasrc.obj"
	-@erase "$(INTDIR)\jdcoefct.obj"
	-@erase "$(INTDIR)\jdcolor.obj"
	-@erase "$(INTDIR)\jddctmgr.obj"
	-@erase "$(INTDIR)\jdhuff.obj"
	-@erase "$(INTDIR)\jdinput.obj"
	-@erase "$(INTDIR)\jdmainct.obj"
	-@erase "$(INTDIR)\jdmarker.obj"
	-@erase "$(INTDIR)\jdmaster.obj"
	-@erase "$(INTDIR)\jdmerge.obj"
	-@erase "$(INTDIR)\jdpostct.obj"
	-@erase "$(INTDIR)\jdsample.obj"
	-@erase "$(INTDIR)\jdtrans.obj"
	-@erase "$(INTDIR)\jerror.obj"
	-@erase "$(INTDIR)\jfdctflt.obj"
	-@erase "$(INTDIR)\jfdctfst.obj"
	-@erase "$(INTDIR)\jfdctint.obj"
	-@erase "$(INTDIR)\jidctflt.obj"
	-@erase "$(INTDIR)\jidctfst.obj"
	-@erase "$(INTDIR)\jidctint.obj"
	-@erase "$(INTDIR)\jmemmgr.obj"
	-@erase "$(INTDIR)\jmemnobs.obj"
	-@erase "$(INTDIR)\JPEGTransform.obj"
	-@erase "$(INTDIR)\jquant1.obj"
	-@erase "$(INTDIR)\jquant2.obj"
	-@erase "$(INTDIR)\jutils.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\MemoryIO.obj"
	-@erase "$(INTDIR)\MultigridPoissonSolver.obj"
	-@erase "$(INTDIR)\MultiPage.obj"
	-@erase "$(INTDIR)\NNQuantizer.obj"
	-@erase "$(INTDIR)\PixelAccess.obj"
	-@erase "$(INTDIR)\Plugin.obj"
	-@erase "$(INTDIR)\PluginBMP.obj"
	-@erase "$(INTDIR)\PluginCUT.obj"
	-@erase "$(INTDIR)\PluginGIF.obj"
	-@erase "$(INTDIR)\PluginICO.obj"
	-@erase "$(INTDIR)\PluginJPEG.obj"
	-@erase "$(INTDIR)\PluginPNG.obj"
	-@erase "$(INTDIR)\png.obj"
	-@erase "$(INTDIR)\pngerror.obj"
	-@erase "$(INTDIR)\pngget.obj"
	-@erase "$(INTDIR)\pngmem.obj"
	-@erase "$(INTDIR)\pngpread.obj"
	-@erase "$(INTDIR)\pngread.obj"
	-@erase "$(INTDIR)\pngrio.obj"
	-@erase "$(INTDIR)\pngrtran.obj"
	-@erase "$(INTDIR)\pngrutil.obj"
	-@erase "$(INTDIR)\pngset.obj"
	-@erase "$(INTDIR)\pngtrans.obj"
	-@erase "$(INTDIR)\pngwio.obj"
	-@erase "$(INTDIR)\pngwrite.obj"
	-@erase "$(INTDIR)\pngwtran.obj"
	-@erase "$(INTDIR)\pngwutil.obj"
	-@erase "$(INTDIR)\Rescale.obj"
	-@erase "$(INTDIR)\Resize.obj"
	-@erase "$(INTDIR)\TagConversion.obj"
	-@erase "$(INTDIR)\TagLib.obj"
	-@erase "$(INTDIR)\tmoColorConvert.obj"
	-@erase "$(INTDIR)\tmoDrago03.obj"
	-@erase "$(INTDIR)\tmoFattal02.obj"
	-@erase "$(INTDIR)\tmoReinhard05.obj"
	-@erase "$(INTDIR)\ToneMapping.obj"
	-@erase "$(INTDIR)\transupp.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WuQuantizer.obj"
	-@erase "$(INTDIR)\ZLibInterface.obj"
	-@erase "$(OUTDIR)\advaimg.exp"
	-@erase "$(OUTDIR)\advaimg.pdb"
	-@erase "..\..\bin\release\plugins\advaimg.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /Zi /O1 /I "Source" /I "Source\ZLib" /I "..\..\include" /I "..\zlib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FREEIMAGE_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\FreeImage.res" /i "..\..\include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\FreeImage.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=..\zlib\Release\zlib.lib kernel32.lib user32.lib gdi32.lib /nologo /base:"0x5130000" /dll /incremental:no /pdb:"$(OUTDIR)\advaimg.pdb" /debug /machine:I386 /out:"../../bin/release/plugins/advaimg.dll" /implib:"$(OUTDIR)\advaimg.lib" /opt:NOWIN98 
LINK32_OBJS= \
	"$(INTDIR)\Plugin.obj" \
	"$(INTDIR)\PluginBMP.obj" \
	"$(INTDIR)\PluginCUT.obj" \
	"$(INTDIR)\PluginGIF.obj" \
	"$(INTDIR)\PluginICO.obj" \
	"$(INTDIR)\PluginJPEG.obj" \
	"$(INTDIR)\PluginPNG.obj" \
	"$(INTDIR)\Conversion.obj" \
	"$(INTDIR)\Conversion16_555.obj" \
	"$(INTDIR)\Conversion16_565.obj" \
	"$(INTDIR)\Conversion24.obj" \
	"$(INTDIR)\Conversion32.obj" \
	"$(INTDIR)\Conversion4.obj" \
	"$(INTDIR)\Conversion8.obj" \
	"$(INTDIR)\ConversionFloat.obj" \
	"$(INTDIR)\ConversionRGB16.obj" \
	"$(INTDIR)\ConversionRGBF.obj" \
	"$(INTDIR)\ConversionType.obj" \
	"$(INTDIR)\ConversionUINT16.obj" \
	"$(INTDIR)\Halftoning.obj" \
	"$(INTDIR)\MultigridPoissonSolver.obj" \
	"$(INTDIR)\tmoColorConvert.obj" \
	"$(INTDIR)\tmoDrago03.obj" \
	"$(INTDIR)\tmoFattal02.obj" \
	"$(INTDIR)\tmoReinhard05.obj" \
	"$(INTDIR)\ToneMapping.obj" \
	"$(INTDIR)\NNQuantizer.obj" \
	"$(INTDIR)\WuQuantizer.obj" \
	"$(INTDIR)\CacheFile.obj" \
	"$(INTDIR)\MultiPage.obj" \
	"$(INTDIR)\ZLibInterface.obj" \
	"$(INTDIR)\Exif.obj" \
	"$(INTDIR)\FIRational.obj" \
	"$(INTDIR)\FreeImageTag.obj" \
	"$(INTDIR)\IPTC.obj" \
	"$(INTDIR)\TagConversion.obj" \
	"$(INTDIR)\TagLib.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\jaricom.obj" \
	"$(INTDIR)\jcapimin.obj" \
	"$(INTDIR)\jcapistd.obj" \
	"$(INTDIR)\jcarith.obj" \
	"$(INTDIR)\jccoefct.obj" \
	"$(INTDIR)\jccolor.obj" \
	"$(INTDIR)\jcdctmgr.obj" \
	"$(INTDIR)\jchuff.obj" \
	"$(INTDIR)\jcinit.obj" \
	"$(INTDIR)\jcmainct.obj" \
	"$(INTDIR)\jcmarker.obj" \
	"$(INTDIR)\jcmaster.obj" \
	"$(INTDIR)\jcomapi.obj" \
	"$(INTDIR)\jcparam.obj" \
	"$(INTDIR)\jcprepct.obj" \
	"$(INTDIR)\jcsample.obj" \
	"$(INTDIR)\jctrans.obj" \
	"$(INTDIR)\jdapimin.obj" \
	"$(INTDIR)\jdapistd.obj" \
	"$(INTDIR)\jdarith.obj" \
	"$(INTDIR)\jdatadst.obj" \
	"$(INTDIR)\jdatasrc.obj" \
	"$(INTDIR)\jdcoefct.obj" \
	"$(INTDIR)\jdcolor.obj" \
	"$(INTDIR)\jddctmgr.obj" \
	"$(INTDIR)\jdhuff.obj" \
	"$(INTDIR)\jdinput.obj" \
	"$(INTDIR)\jdmainct.obj" \
	"$(INTDIR)\jdmarker.obj" \
	"$(INTDIR)\jdmaster.obj" \
	"$(INTDIR)\jdmerge.obj" \
	"$(INTDIR)\jdpostct.obj" \
	"$(INTDIR)\jdsample.obj" \
	"$(INTDIR)\jdtrans.obj" \
	"$(INTDIR)\jerror.obj" \
	"$(INTDIR)\jfdctflt.obj" \
	"$(INTDIR)\jfdctfst.obj" \
	"$(INTDIR)\jfdctint.obj" \
	"$(INTDIR)\jidctflt.obj" \
	"$(INTDIR)\jidctfst.obj" \
	"$(INTDIR)\jidctint.obj" \
	"$(INTDIR)\jmemmgr.obj" \
	"$(INTDIR)\jmemnobs.obj" \
	"$(INTDIR)\jquant1.obj" \
	"$(INTDIR)\jquant2.obj" \
	"$(INTDIR)\jutils.obj" \
	"$(INTDIR)\transupp.obj" \
	"$(INTDIR)\png.obj" \
	"$(INTDIR)\pngerror.obj" \
	"$(INTDIR)\pngget.obj" \
	"$(INTDIR)\pngmem.obj" \
	"$(INTDIR)\pngpread.obj" \
	"$(INTDIR)\pngread.obj" \
	"$(INTDIR)\pngrio.obj" \
	"$(INTDIR)\pngrtran.obj" \
	"$(INTDIR)\pngrutil.obj" \
	"$(INTDIR)\pngset.obj" \
	"$(INTDIR)\pngtrans.obj" \
	"$(INTDIR)\pngwio.obj" \
	"$(INTDIR)\pngwrite.obj" \
	"$(INTDIR)\pngwtran.obj" \
	"$(INTDIR)\pngwutil.obj" \
	"$(INTDIR)\BitmapAccess.obj" \
	"$(INTDIR)\ColorLookup.obj" \
	"$(INTDIR)\FreeImage.obj" \
	"$(INTDIR)\FreeImageC.obj" \
	"$(INTDIR)\FreeImageIO.obj" \
	"$(INTDIR)\GetType.obj" \
	"$(INTDIR)\MemoryIO.obj" \
	"$(INTDIR)\PixelAccess.obj" \
	"$(INTDIR)\BSplineRotate.obj" \
	"$(INTDIR)\Channels.obj" \
	"$(INTDIR)\ClassicRotate.obj" \
	"$(INTDIR)\Colors.obj" \
	"$(INTDIR)\CopyPaste.obj" \
	"$(INTDIR)\Display.obj" \
	"$(INTDIR)\Flip.obj" \
	"$(INTDIR)\JPEGTransform.obj" \
	"$(INTDIR)\Rescale.obj" \
	"$(INTDIR)\Resize.obj" \
	"$(INTDIR)\FreeImage.res" \
	"..\zlib\Release\zlib.lib"

"..\..\bin\release\plugins\advaimg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "..\..\bin\debug\plugins\advaimg.dll" "$(OUTDIR)\FreeImage.bsc"

!ELSE 

ALL : "zlib - Win32 Debug" "..\..\bin\debug\plugins\advaimg.dll" "$(OUTDIR)\FreeImage.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"zlib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\BitmapAccess.obj"
	-@erase "$(INTDIR)\BitmapAccess.sbr"
	-@erase "$(INTDIR)\BSplineRotate.obj"
	-@erase "$(INTDIR)\BSplineRotate.sbr"
	-@erase "$(INTDIR)\CacheFile.obj"
	-@erase "$(INTDIR)\CacheFile.sbr"
	-@erase "$(INTDIR)\Channels.obj"
	-@erase "$(INTDIR)\Channels.sbr"
	-@erase "$(INTDIR)\ClassicRotate.obj"
	-@erase "$(INTDIR)\ClassicRotate.sbr"
	-@erase "$(INTDIR)\ColorLookup.obj"
	-@erase "$(INTDIR)\ColorLookup.sbr"
	-@erase "$(INTDIR)\Colors.obj"
	-@erase "$(INTDIR)\Colors.sbr"
	-@erase "$(INTDIR)\Conversion.obj"
	-@erase "$(INTDIR)\Conversion.sbr"
	-@erase "$(INTDIR)\Conversion16_555.obj"
	-@erase "$(INTDIR)\Conversion16_555.sbr"
	-@erase "$(INTDIR)\Conversion16_565.obj"
	-@erase "$(INTDIR)\Conversion16_565.sbr"
	-@erase "$(INTDIR)\Conversion24.obj"
	-@erase "$(INTDIR)\Conversion24.sbr"
	-@erase "$(INTDIR)\Conversion32.obj"
	-@erase "$(INTDIR)\Conversion32.sbr"
	-@erase "$(INTDIR)\Conversion4.obj"
	-@erase "$(INTDIR)\Conversion4.sbr"
	-@erase "$(INTDIR)\Conversion8.obj"
	-@erase "$(INTDIR)\Conversion8.sbr"
	-@erase "$(INTDIR)\ConversionFloat.obj"
	-@erase "$(INTDIR)\ConversionFloat.sbr"
	-@erase "$(INTDIR)\ConversionRGB16.obj"
	-@erase "$(INTDIR)\ConversionRGB16.sbr"
	-@erase "$(INTDIR)\ConversionRGBF.obj"
	-@erase "$(INTDIR)\ConversionRGBF.sbr"
	-@erase "$(INTDIR)\ConversionType.obj"
	-@erase "$(INTDIR)\ConversionType.sbr"
	-@erase "$(INTDIR)\ConversionUINT16.obj"
	-@erase "$(INTDIR)\ConversionUINT16.sbr"
	-@erase "$(INTDIR)\CopyPaste.obj"
	-@erase "$(INTDIR)\CopyPaste.sbr"
	-@erase "$(INTDIR)\Display.obj"
	-@erase "$(INTDIR)\Display.sbr"
	-@erase "$(INTDIR)\Exif.obj"
	-@erase "$(INTDIR)\Exif.sbr"
	-@erase "$(INTDIR)\FIRational.obj"
	-@erase "$(INTDIR)\FIRational.sbr"
	-@erase "$(INTDIR)\Flip.obj"
	-@erase "$(INTDIR)\Flip.sbr"
	-@erase "$(INTDIR)\FreeImage.obj"
	-@erase "$(INTDIR)\FreeImage.res"
	-@erase "$(INTDIR)\FreeImage.sbr"
	-@erase "$(INTDIR)\FreeImageC.obj"
	-@erase "$(INTDIR)\FreeImageC.sbr"
	-@erase "$(INTDIR)\FreeImageIO.obj"
	-@erase "$(INTDIR)\FreeImageIO.sbr"
	-@erase "$(INTDIR)\FreeImageTag.obj"
	-@erase "$(INTDIR)\FreeImageTag.sbr"
	-@erase "$(INTDIR)\GetType.obj"
	-@erase "$(INTDIR)\GetType.sbr"
	-@erase "$(INTDIR)\Halftoning.obj"
	-@erase "$(INTDIR)\Halftoning.sbr"
	-@erase "$(INTDIR)\IPTC.obj"
	-@erase "$(INTDIR)\IPTC.sbr"
	-@erase "$(INTDIR)\jaricom.obj"
	-@erase "$(INTDIR)\jaricom.sbr"
	-@erase "$(INTDIR)\jcapimin.obj"
	-@erase "$(INTDIR)\jcapimin.sbr"
	-@erase "$(INTDIR)\jcapistd.obj"
	-@erase "$(INTDIR)\jcapistd.sbr"
	-@erase "$(INTDIR)\jcarith.obj"
	-@erase "$(INTDIR)\jcarith.sbr"
	-@erase "$(INTDIR)\jccoefct.obj"
	-@erase "$(INTDIR)\jccoefct.sbr"
	-@erase "$(INTDIR)\jccolor.obj"
	-@erase "$(INTDIR)\jccolor.sbr"
	-@erase "$(INTDIR)\jcdctmgr.obj"
	-@erase "$(INTDIR)\jcdctmgr.sbr"
	-@erase "$(INTDIR)\jchuff.obj"
	-@erase "$(INTDIR)\jchuff.sbr"
	-@erase "$(INTDIR)\jcinit.obj"
	-@erase "$(INTDIR)\jcinit.sbr"
	-@erase "$(INTDIR)\jcmainct.obj"
	-@erase "$(INTDIR)\jcmainct.sbr"
	-@erase "$(INTDIR)\jcmarker.obj"
	-@erase "$(INTDIR)\jcmarker.sbr"
	-@erase "$(INTDIR)\jcmaster.obj"
	-@erase "$(INTDIR)\jcmaster.sbr"
	-@erase "$(INTDIR)\jcomapi.obj"
	-@erase "$(INTDIR)\jcomapi.sbr"
	-@erase "$(INTDIR)\jcparam.obj"
	-@erase "$(INTDIR)\jcparam.sbr"
	-@erase "$(INTDIR)\jcprepct.obj"
	-@erase "$(INTDIR)\jcprepct.sbr"
	-@erase "$(INTDIR)\jcsample.obj"
	-@erase "$(INTDIR)\jcsample.sbr"
	-@erase "$(INTDIR)\jctrans.obj"
	-@erase "$(INTDIR)\jctrans.sbr"
	-@erase "$(INTDIR)\jdapimin.obj"
	-@erase "$(INTDIR)\jdapimin.sbr"
	-@erase "$(INTDIR)\jdapistd.obj"
	-@erase "$(INTDIR)\jdapistd.sbr"
	-@erase "$(INTDIR)\jdarith.obj"
	-@erase "$(INTDIR)\jdarith.sbr"
	-@erase "$(INTDIR)\jdatadst.obj"
	-@erase "$(INTDIR)\jdatadst.sbr"
	-@erase "$(INTDIR)\jdatasrc.obj"
	-@erase "$(INTDIR)\jdatasrc.sbr"
	-@erase "$(INTDIR)\jdcoefct.obj"
	-@erase "$(INTDIR)\jdcoefct.sbr"
	-@erase "$(INTDIR)\jdcolor.obj"
	-@erase "$(INTDIR)\jdcolor.sbr"
	-@erase "$(INTDIR)\jddctmgr.obj"
	-@erase "$(INTDIR)\jddctmgr.sbr"
	-@erase "$(INTDIR)\jdhuff.obj"
	-@erase "$(INTDIR)\jdhuff.sbr"
	-@erase "$(INTDIR)\jdinput.obj"
	-@erase "$(INTDIR)\jdinput.sbr"
	-@erase "$(INTDIR)\jdmainct.obj"
	-@erase "$(INTDIR)\jdmainct.sbr"
	-@erase "$(INTDIR)\jdmarker.obj"
	-@erase "$(INTDIR)\jdmarker.sbr"
	-@erase "$(INTDIR)\jdmaster.obj"
	-@erase "$(INTDIR)\jdmaster.sbr"
	-@erase "$(INTDIR)\jdmerge.obj"
	-@erase "$(INTDIR)\jdmerge.sbr"
	-@erase "$(INTDIR)\jdpostct.obj"
	-@erase "$(INTDIR)\jdpostct.sbr"
	-@erase "$(INTDIR)\jdsample.obj"
	-@erase "$(INTDIR)\jdsample.sbr"
	-@erase "$(INTDIR)\jdtrans.obj"
	-@erase "$(INTDIR)\jdtrans.sbr"
	-@erase "$(INTDIR)\jerror.obj"
	-@erase "$(INTDIR)\jerror.sbr"
	-@erase "$(INTDIR)\jfdctflt.obj"
	-@erase "$(INTDIR)\jfdctflt.sbr"
	-@erase "$(INTDIR)\jfdctfst.obj"
	-@erase "$(INTDIR)\jfdctfst.sbr"
	-@erase "$(INTDIR)\jfdctint.obj"
	-@erase "$(INTDIR)\jfdctint.sbr"
	-@erase "$(INTDIR)\jidctflt.obj"
	-@erase "$(INTDIR)\jidctflt.sbr"
	-@erase "$(INTDIR)\jidctfst.obj"
	-@erase "$(INTDIR)\jidctfst.sbr"
	-@erase "$(INTDIR)\jidctint.obj"
	-@erase "$(INTDIR)\jidctint.sbr"
	-@erase "$(INTDIR)\jmemmgr.obj"
	-@erase "$(INTDIR)\jmemmgr.sbr"
	-@erase "$(INTDIR)\jmemnobs.obj"
	-@erase "$(INTDIR)\jmemnobs.sbr"
	-@erase "$(INTDIR)\JPEGTransform.obj"
	-@erase "$(INTDIR)\JPEGTransform.sbr"
	-@erase "$(INTDIR)\jquant1.obj"
	-@erase "$(INTDIR)\jquant1.sbr"
	-@erase "$(INTDIR)\jquant2.obj"
	-@erase "$(INTDIR)\jquant2.sbr"
	-@erase "$(INTDIR)\jutils.obj"
	-@erase "$(INTDIR)\jutils.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\MemoryIO.obj"
	-@erase "$(INTDIR)\MemoryIO.sbr"
	-@erase "$(INTDIR)\MultigridPoissonSolver.obj"
	-@erase "$(INTDIR)\MultigridPoissonSolver.sbr"
	-@erase "$(INTDIR)\MultiPage.obj"
	-@erase "$(INTDIR)\MultiPage.sbr"
	-@erase "$(INTDIR)\NNQuantizer.obj"
	-@erase "$(INTDIR)\NNQuantizer.sbr"
	-@erase "$(INTDIR)\PixelAccess.obj"
	-@erase "$(INTDIR)\PixelAccess.sbr"
	-@erase "$(INTDIR)\Plugin.obj"
	-@erase "$(INTDIR)\Plugin.sbr"
	-@erase "$(INTDIR)\PluginBMP.obj"
	-@erase "$(INTDIR)\PluginBMP.sbr"
	-@erase "$(INTDIR)\PluginCUT.obj"
	-@erase "$(INTDIR)\PluginCUT.sbr"
	-@erase "$(INTDIR)\PluginGIF.obj"
	-@erase "$(INTDIR)\PluginGIF.sbr"
	-@erase "$(INTDIR)\PluginICO.obj"
	-@erase "$(INTDIR)\PluginICO.sbr"
	-@erase "$(INTDIR)\PluginJPEG.obj"
	-@erase "$(INTDIR)\PluginJPEG.sbr"
	-@erase "$(INTDIR)\PluginPNG.obj"
	-@erase "$(INTDIR)\PluginPNG.sbr"
	-@erase "$(INTDIR)\png.obj"
	-@erase "$(INTDIR)\png.sbr"
	-@erase "$(INTDIR)\pngerror.obj"
	-@erase "$(INTDIR)\pngerror.sbr"
	-@erase "$(INTDIR)\pngget.obj"
	-@erase "$(INTDIR)\pngget.sbr"
	-@erase "$(INTDIR)\pngmem.obj"
	-@erase "$(INTDIR)\pngmem.sbr"
	-@erase "$(INTDIR)\pngpread.obj"
	-@erase "$(INTDIR)\pngpread.sbr"
	-@erase "$(INTDIR)\pngread.obj"
	-@erase "$(INTDIR)\pngread.sbr"
	-@erase "$(INTDIR)\pngrio.obj"
	-@erase "$(INTDIR)\pngrio.sbr"
	-@erase "$(INTDIR)\pngrtran.obj"
	-@erase "$(INTDIR)\pngrtran.sbr"
	-@erase "$(INTDIR)\pngrutil.obj"
	-@erase "$(INTDIR)\pngrutil.sbr"
	-@erase "$(INTDIR)\pngset.obj"
	-@erase "$(INTDIR)\pngset.sbr"
	-@erase "$(INTDIR)\pngtrans.obj"
	-@erase "$(INTDIR)\pngtrans.sbr"
	-@erase "$(INTDIR)\pngwio.obj"
	-@erase "$(INTDIR)\pngwio.sbr"
	-@erase "$(INTDIR)\pngwrite.obj"
	-@erase "$(INTDIR)\pngwrite.sbr"
	-@erase "$(INTDIR)\pngwtran.obj"
	-@erase "$(INTDIR)\pngwtran.sbr"
	-@erase "$(INTDIR)\pngwutil.obj"
	-@erase "$(INTDIR)\pngwutil.sbr"
	-@erase "$(INTDIR)\Rescale.obj"
	-@erase "$(INTDIR)\Rescale.sbr"
	-@erase "$(INTDIR)\Resize.obj"
	-@erase "$(INTDIR)\Resize.sbr"
	-@erase "$(INTDIR)\TagConversion.obj"
	-@erase "$(INTDIR)\TagConversion.sbr"
	-@erase "$(INTDIR)\TagLib.obj"
	-@erase "$(INTDIR)\TagLib.sbr"
	-@erase "$(INTDIR)\tmoColorConvert.obj"
	-@erase "$(INTDIR)\tmoColorConvert.sbr"
	-@erase "$(INTDIR)\tmoDrago03.obj"
	-@erase "$(INTDIR)\tmoDrago03.sbr"
	-@erase "$(INTDIR)\tmoFattal02.obj"
	-@erase "$(INTDIR)\tmoFattal02.sbr"
	-@erase "$(INTDIR)\tmoReinhard05.obj"
	-@erase "$(INTDIR)\tmoReinhard05.sbr"
	-@erase "$(INTDIR)\ToneMapping.obj"
	-@erase "$(INTDIR)\ToneMapping.sbr"
	-@erase "$(INTDIR)\transupp.obj"
	-@erase "$(INTDIR)\transupp.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\WuQuantizer.obj"
	-@erase "$(INTDIR)\WuQuantizer.sbr"
	-@erase "$(INTDIR)\ZLibInterface.obj"
	-@erase "$(INTDIR)\ZLibInterface.sbr"
	-@erase "$(OUTDIR)\advaimg.exp"
	-@erase "$(OUTDIR)\advaimg.lib"
	-@erase "$(OUTDIR)\advaimg.pdb"
	-@erase "$(OUTDIR)\FreeImage.bsc"
	-@erase "..\..\bin\debug\plugins\advaimg.dll"
	-@erase "..\..\bin\debug\plugins\advaimg.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "Source" /I "Source\ZLib" /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FREEIMAGE_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\FreeImage.res" /i "..\..\include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\FreeImage.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Plugin.sbr" \
	"$(INTDIR)\PluginBMP.sbr" \
	"$(INTDIR)\PluginCUT.sbr" \
	"$(INTDIR)\PluginGIF.sbr" \
	"$(INTDIR)\PluginICO.sbr" \
	"$(INTDIR)\PluginJPEG.sbr" \
	"$(INTDIR)\PluginPNG.sbr" \
	"$(INTDIR)\Conversion.sbr" \
	"$(INTDIR)\Conversion16_555.sbr" \
	"$(INTDIR)\Conversion16_565.sbr" \
	"$(INTDIR)\Conversion24.sbr" \
	"$(INTDIR)\Conversion32.sbr" \
	"$(INTDIR)\Conversion4.sbr" \
	"$(INTDIR)\Conversion8.sbr" \
	"$(INTDIR)\ConversionFloat.sbr" \
	"$(INTDIR)\ConversionRGB16.sbr" \
	"$(INTDIR)\ConversionRGBF.sbr" \
	"$(INTDIR)\ConversionType.sbr" \
	"$(INTDIR)\ConversionUINT16.sbr" \
	"$(INTDIR)\Halftoning.sbr" \
	"$(INTDIR)\MultigridPoissonSolver.sbr" \
	"$(INTDIR)\tmoColorConvert.sbr" \
	"$(INTDIR)\tmoDrago03.sbr" \
	"$(INTDIR)\tmoFattal02.sbr" \
	"$(INTDIR)\tmoReinhard05.sbr" \
	"$(INTDIR)\ToneMapping.sbr" \
	"$(INTDIR)\NNQuantizer.sbr" \
	"$(INTDIR)\WuQuantizer.sbr" \
	"$(INTDIR)\CacheFile.sbr" \
	"$(INTDIR)\MultiPage.sbr" \
	"$(INTDIR)\ZLibInterface.sbr" \
	"$(INTDIR)\Exif.sbr" \
	"$(INTDIR)\FIRational.sbr" \
	"$(INTDIR)\FreeImageTag.sbr" \
	"$(INTDIR)\IPTC.sbr" \
	"$(INTDIR)\TagConversion.sbr" \
	"$(INTDIR)\TagLib.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\jaricom.sbr" \
	"$(INTDIR)\jcapimin.sbr" \
	"$(INTDIR)\jcapistd.sbr" \
	"$(INTDIR)\jcarith.sbr" \
	"$(INTDIR)\jccoefct.sbr" \
	"$(INTDIR)\jccolor.sbr" \
	"$(INTDIR)\jcdctmgr.sbr" \
	"$(INTDIR)\jchuff.sbr" \
	"$(INTDIR)\jcinit.sbr" \
	"$(INTDIR)\jcmainct.sbr" \
	"$(INTDIR)\jcmarker.sbr" \
	"$(INTDIR)\jcmaster.sbr" \
	"$(INTDIR)\jcomapi.sbr" \
	"$(INTDIR)\jcparam.sbr" \
	"$(INTDIR)\jcprepct.sbr" \
	"$(INTDIR)\jcsample.sbr" \
	"$(INTDIR)\jctrans.sbr" \
	"$(INTDIR)\jdapimin.sbr" \
	"$(INTDIR)\jdapistd.sbr" \
	"$(INTDIR)\jdarith.sbr" \
	"$(INTDIR)\jdatadst.sbr" \
	"$(INTDIR)\jdatasrc.sbr" \
	"$(INTDIR)\jdcoefct.sbr" \
	"$(INTDIR)\jdcolor.sbr" \
	"$(INTDIR)\jddctmgr.sbr" \
	"$(INTDIR)\jdhuff.sbr" \
	"$(INTDIR)\jdinput.sbr" \
	"$(INTDIR)\jdmainct.sbr" \
	"$(INTDIR)\jdmarker.sbr" \
	"$(INTDIR)\jdmaster.sbr" \
	"$(INTDIR)\jdmerge.sbr" \
	"$(INTDIR)\jdpostct.sbr" \
	"$(INTDIR)\jdsample.sbr" \
	"$(INTDIR)\jdtrans.sbr" \
	"$(INTDIR)\jerror.sbr" \
	"$(INTDIR)\jfdctflt.sbr" \
	"$(INTDIR)\jfdctfst.sbr" \
	"$(INTDIR)\jfdctint.sbr" \
	"$(INTDIR)\jidctflt.sbr" \
	"$(INTDIR)\jidctfst.sbr" \
	"$(INTDIR)\jidctint.sbr" \
	"$(INTDIR)\jmemmgr.sbr" \
	"$(INTDIR)\jmemnobs.sbr" \
	"$(INTDIR)\jquant1.sbr" \
	"$(INTDIR)\jquant2.sbr" \
	"$(INTDIR)\jutils.sbr" \
	"$(INTDIR)\transupp.sbr" \
	"$(INTDIR)\png.sbr" \
	"$(INTDIR)\pngerror.sbr" \
	"$(INTDIR)\pngget.sbr" \
	"$(INTDIR)\pngmem.sbr" \
	"$(INTDIR)\pngpread.sbr" \
	"$(INTDIR)\pngread.sbr" \
	"$(INTDIR)\pngrio.sbr" \
	"$(INTDIR)\pngrtran.sbr" \
	"$(INTDIR)\pngrutil.sbr" \
	"$(INTDIR)\pngset.sbr" \
	"$(INTDIR)\pngtrans.sbr" \
	"$(INTDIR)\pngwio.sbr" \
	"$(INTDIR)\pngwrite.sbr" \
	"$(INTDIR)\pngwtran.sbr" \
	"$(INTDIR)\pngwutil.sbr" \
	"$(INTDIR)\BitmapAccess.sbr" \
	"$(INTDIR)\ColorLookup.sbr" \
	"$(INTDIR)\FreeImage.sbr" \
	"$(INTDIR)\FreeImageC.sbr" \
	"$(INTDIR)\FreeImageIO.sbr" \
	"$(INTDIR)\GetType.sbr" \
	"$(INTDIR)\MemoryIO.sbr" \
	"$(INTDIR)\PixelAccess.sbr" \
	"$(INTDIR)\BSplineRotate.sbr" \
	"$(INTDIR)\Channels.sbr" \
	"$(INTDIR)\ClassicRotate.sbr" \
	"$(INTDIR)\Colors.sbr" \
	"$(INTDIR)\CopyPaste.sbr" \
	"$(INTDIR)\Display.sbr" \
	"$(INTDIR)\Flip.sbr" \
	"$(INTDIR)\JPEGTransform.sbr" \
	"$(INTDIR)\Rescale.sbr" \
	"$(INTDIR)\Resize.sbr"

"$(OUTDIR)\FreeImage.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=..\zlib\Release\zlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\advaimg.pdb" /debug /machine:I386 /out:"../../bin/debug/plugins/advaimg.dll" /implib:"$(OUTDIR)\advaimg.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\Plugin.obj" \
	"$(INTDIR)\PluginBMP.obj" \
	"$(INTDIR)\PluginCUT.obj" \
	"$(INTDIR)\PluginGIF.obj" \
	"$(INTDIR)\PluginICO.obj" \
	"$(INTDIR)\PluginJPEG.obj" \
	"$(INTDIR)\PluginPNG.obj" \
	"$(INTDIR)\Conversion.obj" \
	"$(INTDIR)\Conversion16_555.obj" \
	"$(INTDIR)\Conversion16_565.obj" \
	"$(INTDIR)\Conversion24.obj" \
	"$(INTDIR)\Conversion32.obj" \
	"$(INTDIR)\Conversion4.obj" \
	"$(INTDIR)\Conversion8.obj" \
	"$(INTDIR)\ConversionFloat.obj" \
	"$(INTDIR)\ConversionRGB16.obj" \
	"$(INTDIR)\ConversionRGBF.obj" \
	"$(INTDIR)\ConversionType.obj" \
	"$(INTDIR)\ConversionUINT16.obj" \
	"$(INTDIR)\Halftoning.obj" \
	"$(INTDIR)\MultigridPoissonSolver.obj" \
	"$(INTDIR)\tmoColorConvert.obj" \
	"$(INTDIR)\tmoDrago03.obj" \
	"$(INTDIR)\tmoFattal02.obj" \
	"$(INTDIR)\tmoReinhard05.obj" \
	"$(INTDIR)\ToneMapping.obj" \
	"$(INTDIR)\NNQuantizer.obj" \
	"$(INTDIR)\WuQuantizer.obj" \
	"$(INTDIR)\CacheFile.obj" \
	"$(INTDIR)\MultiPage.obj" \
	"$(INTDIR)\ZLibInterface.obj" \
	"$(INTDIR)\Exif.obj" \
	"$(INTDIR)\FIRational.obj" \
	"$(INTDIR)\FreeImageTag.obj" \
	"$(INTDIR)\IPTC.obj" \
	"$(INTDIR)\TagConversion.obj" \
	"$(INTDIR)\TagLib.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\jaricom.obj" \
	"$(INTDIR)\jcapimin.obj" \
	"$(INTDIR)\jcapistd.obj" \
	"$(INTDIR)\jcarith.obj" \
	"$(INTDIR)\jccoefct.obj" \
	"$(INTDIR)\jccolor.obj" \
	"$(INTDIR)\jcdctmgr.obj" \
	"$(INTDIR)\jchuff.obj" \
	"$(INTDIR)\jcinit.obj" \
	"$(INTDIR)\jcmainct.obj" \
	"$(INTDIR)\jcmarker.obj" \
	"$(INTDIR)\jcmaster.obj" \
	"$(INTDIR)\jcomapi.obj" \
	"$(INTDIR)\jcparam.obj" \
	"$(INTDIR)\jcprepct.obj" \
	"$(INTDIR)\jcsample.obj" \
	"$(INTDIR)\jctrans.obj" \
	"$(INTDIR)\jdapimin.obj" \
	"$(INTDIR)\jdapistd.obj" \
	"$(INTDIR)\jdarith.obj" \
	"$(INTDIR)\jdatadst.obj" \
	"$(INTDIR)\jdatasrc.obj" \
	"$(INTDIR)\jdcoefct.obj" \
	"$(INTDIR)\jdcolor.obj" \
	"$(INTDIR)\jddctmgr.obj" \
	"$(INTDIR)\jdhuff.obj" \
	"$(INTDIR)\jdinput.obj" \
	"$(INTDIR)\jdmainct.obj" \
	"$(INTDIR)\jdmarker.obj" \
	"$(INTDIR)\jdmaster.obj" \
	"$(INTDIR)\jdmerge.obj" \
	"$(INTDIR)\jdpostct.obj" \
	"$(INTDIR)\jdsample.obj" \
	"$(INTDIR)\jdtrans.obj" \
	"$(INTDIR)\jerror.obj" \
	"$(INTDIR)\jfdctflt.obj" \
	"$(INTDIR)\jfdctfst.obj" \
	"$(INTDIR)\jfdctint.obj" \
	"$(INTDIR)\jidctflt.obj" \
	"$(INTDIR)\jidctfst.obj" \
	"$(INTDIR)\jidctint.obj" \
	"$(INTDIR)\jmemmgr.obj" \
	"$(INTDIR)\jmemnobs.obj" \
	"$(INTDIR)\jquant1.obj" \
	"$(INTDIR)\jquant2.obj" \
	"$(INTDIR)\jutils.obj" \
	"$(INTDIR)\transupp.obj" \
	"$(INTDIR)\png.obj" \
	"$(INTDIR)\pngerror.obj" \
	"$(INTDIR)\pngget.obj" \
	"$(INTDIR)\pngmem.obj" \
	"$(INTDIR)\pngpread.obj" \
	"$(INTDIR)\pngread.obj" \
	"$(INTDIR)\pngrio.obj" \
	"$(INTDIR)\pngrtran.obj" \
	"$(INTDIR)\pngrutil.obj" \
	"$(INTDIR)\pngset.obj" \
	"$(INTDIR)\pngtrans.obj" \
	"$(INTDIR)\pngwio.obj" \
	"$(INTDIR)\pngwrite.obj" \
	"$(INTDIR)\pngwtran.obj" \
	"$(INTDIR)\pngwutil.obj" \
	"$(INTDIR)\BitmapAccess.obj" \
	"$(INTDIR)\ColorLookup.obj" \
	"$(INTDIR)\FreeImage.obj" \
	"$(INTDIR)\FreeImageC.obj" \
	"$(INTDIR)\FreeImageIO.obj" \
	"$(INTDIR)\GetType.obj" \
	"$(INTDIR)\MemoryIO.obj" \
	"$(INTDIR)\PixelAccess.obj" \
	"$(INTDIR)\BSplineRotate.obj" \
	"$(INTDIR)\Channels.obj" \
	"$(INTDIR)\ClassicRotate.obj" \
	"$(INTDIR)\Colors.obj" \
	"$(INTDIR)\CopyPaste.obj" \
	"$(INTDIR)\Display.obj" \
	"$(INTDIR)\Flip.obj" \
	"$(INTDIR)\JPEGTransform.obj" \
	"$(INTDIR)\Rescale.obj" \
	"$(INTDIR)\Resize.obj" \
	"$(INTDIR)\FreeImage.res" \
	"..\zlib\Debug\zlib.lib"

"..\..\bin\debug\plugins\advaimg.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("FreeImage.dep")
!INCLUDE "FreeImage.dep"
!ELSE 
!MESSAGE Warning: cannot find "FreeImage.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "FreeImage - Win32 Release" || "$(CFG)" == "FreeImage - Win32 Debug"
SOURCE=.\Source\FreeImage\Plugin.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Plugin.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Plugin.obj"	"$(INTDIR)\Plugin.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\PluginBMP.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\PluginBMP.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\PluginBMP.obj"	"$(INTDIR)\PluginBMP.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\PluginCUT.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\PluginCUT.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\PluginCUT.obj"	"$(INTDIR)\PluginCUT.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\PluginGIF.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\PluginGIF.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\PluginGIF.obj"	"$(INTDIR)\PluginGIF.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\PluginICO.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\PluginICO.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\PluginICO.obj"	"$(INTDIR)\PluginICO.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\PluginJPEG.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\PluginJPEG.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\PluginJPEG.obj"	"$(INTDIR)\PluginJPEG.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\PluginPNG.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\PluginPNG.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\PluginPNG.obj"	"$(INTDIR)\PluginPNG.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\Conversion.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Conversion.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Conversion.obj"	"$(INTDIR)\Conversion.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\Conversion16_555.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Conversion16_555.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Conversion16_555.obj"	"$(INTDIR)\Conversion16_555.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\Conversion16_565.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Conversion16_565.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Conversion16_565.obj"	"$(INTDIR)\Conversion16_565.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\Conversion24.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Conversion24.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Conversion24.obj"	"$(INTDIR)\Conversion24.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\Conversion32.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Conversion32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Conversion32.obj"	"$(INTDIR)\Conversion32.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\Conversion4.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Conversion4.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Conversion4.obj"	"$(INTDIR)\Conversion4.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\Conversion8.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Conversion8.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Conversion8.obj"	"$(INTDIR)\Conversion8.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\ConversionFloat.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\ConversionFloat.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\ConversionFloat.obj"	"$(INTDIR)\ConversionFloat.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\ConversionRGB16.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\ConversionRGB16.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\ConversionRGB16.obj"	"$(INTDIR)\ConversionRGB16.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\ConversionRGBF.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\ConversionRGBF.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\ConversionRGBF.obj"	"$(INTDIR)\ConversionRGBF.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\ConversionType.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\ConversionType.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\ConversionType.obj"	"$(INTDIR)\ConversionType.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\ConversionUINT16.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\ConversionUINT16.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\ConversionUINT16.obj"	"$(INTDIR)\ConversionUINT16.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\Halftoning.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Halftoning.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Halftoning.obj"	"$(INTDIR)\Halftoning.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\MultigridPoissonSolver.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\MultigridPoissonSolver.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\MultigridPoissonSolver.obj"	"$(INTDIR)\MultigridPoissonSolver.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\tmoColorConvert.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\tmoColorConvert.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\tmoColorConvert.obj"	"$(INTDIR)\tmoColorConvert.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\tmoDrago03.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\tmoDrago03.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\tmoDrago03.obj"	"$(INTDIR)\tmoDrago03.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\tmoFattal02.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\tmoFattal02.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\tmoFattal02.obj"	"$(INTDIR)\tmoFattal02.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\tmoReinhard05.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\tmoReinhard05.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\tmoReinhard05.obj"	"$(INTDIR)\tmoReinhard05.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\ToneMapping.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\ToneMapping.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\ToneMapping.obj"	"$(INTDIR)\ToneMapping.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\NNQuantizer.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\NNQuantizer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\NNQuantizer.obj"	"$(INTDIR)\NNQuantizer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\WuQuantizer.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\WuQuantizer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\WuQuantizer.obj"	"$(INTDIR)\WuQuantizer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\CacheFile.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\CacheFile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\CacheFile.obj"	"$(INTDIR)\CacheFile.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\MultiPage.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\MultiPage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\MultiPage.obj"	"$(INTDIR)\MultiPage.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\ZLibInterface.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\ZLibInterface.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\ZLibInterface.obj"	"$(INTDIR)\ZLibInterface.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\Metadata\Exif.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Exif.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Exif.obj"	"$(INTDIR)\Exif.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\Metadata\FIRational.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\FIRational.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\FIRational.obj"	"$(INTDIR)\FIRational.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\Metadata\FreeImageTag.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\FreeImageTag.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\FreeImageTag.obj"	"$(INTDIR)\FreeImageTag.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\Metadata\IPTC.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\IPTC.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\IPTC.obj"	"$(INTDIR)\IPTC.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\Metadata\TagConversion.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\TagConversion.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\TagConversion.obj"	"$(INTDIR)\TagConversion.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\Metadata\TagLib.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\TagLib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\TagLib.obj"	"$(INTDIR)\TagLib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Miranda\main.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jaricom.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jaricom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jaricom.obj"	"$(INTDIR)\jaricom.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcapimin.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcapimin.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcapimin.obj"	"$(INTDIR)\jcapimin.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcapistd.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcapistd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcapistd.obj"	"$(INTDIR)\jcapistd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcarith.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcarith.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcarith.obj"	"$(INTDIR)\jcarith.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jccoefct.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jccoefct.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jccoefct.obj"	"$(INTDIR)\jccoefct.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jccolor.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jccolor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jccolor.obj"	"$(INTDIR)\jccolor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcdctmgr.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcdctmgr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcdctmgr.obj"	"$(INTDIR)\jcdctmgr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jchuff.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jchuff.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jchuff.obj"	"$(INTDIR)\jchuff.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcinit.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcinit.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcinit.obj"	"$(INTDIR)\jcinit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcmainct.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcmainct.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcmainct.obj"	"$(INTDIR)\jcmainct.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcmarker.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcmarker.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcmarker.obj"	"$(INTDIR)\jcmarker.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcmaster.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcmaster.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcmaster.obj"	"$(INTDIR)\jcmaster.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcomapi.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcomapi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcomapi.obj"	"$(INTDIR)\jcomapi.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcparam.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcparam.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcparam.obj"	"$(INTDIR)\jcparam.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcprepct.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcprepct.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcprepct.obj"	"$(INTDIR)\jcprepct.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jcsample.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jcsample.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jcsample.obj"	"$(INTDIR)\jcsample.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jctrans.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jctrans.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jctrans.obj"	"$(INTDIR)\jctrans.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdapimin.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdapimin.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdapimin.obj"	"$(INTDIR)\jdapimin.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdapistd.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdapistd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdapistd.obj"	"$(INTDIR)\jdapistd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdarith.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdarith.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdarith.obj"	"$(INTDIR)\jdarith.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdatadst.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdatadst.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdatadst.obj"	"$(INTDIR)\jdatadst.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdatasrc.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdatasrc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdatasrc.obj"	"$(INTDIR)\jdatasrc.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdcoefct.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdcoefct.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdcoefct.obj"	"$(INTDIR)\jdcoefct.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdcolor.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdcolor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdcolor.obj"	"$(INTDIR)\jdcolor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jddctmgr.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jddctmgr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jddctmgr.obj"	"$(INTDIR)\jddctmgr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdhuff.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdhuff.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdhuff.obj"	"$(INTDIR)\jdhuff.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdinput.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdinput.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdinput.obj"	"$(INTDIR)\jdinput.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdmainct.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdmainct.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdmainct.obj"	"$(INTDIR)\jdmainct.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdmarker.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdmarker.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdmarker.obj"	"$(INTDIR)\jdmarker.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdmaster.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdmaster.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdmaster.obj"	"$(INTDIR)\jdmaster.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdmerge.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdmerge.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdmerge.obj"	"$(INTDIR)\jdmerge.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdpostct.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdpostct.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdpostct.obj"	"$(INTDIR)\jdpostct.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdsample.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdsample.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdsample.obj"	"$(INTDIR)\jdsample.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jdtrans.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jdtrans.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jdtrans.obj"	"$(INTDIR)\jdtrans.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jerror.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jerror.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jerror.obj"	"$(INTDIR)\jerror.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jfdctflt.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jfdctflt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jfdctflt.obj"	"$(INTDIR)\jfdctflt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jfdctfst.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jfdctfst.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jfdctfst.obj"	"$(INTDIR)\jfdctfst.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jfdctint.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jfdctint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jfdctint.obj"	"$(INTDIR)\jfdctint.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jidctflt.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jidctflt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jidctflt.obj"	"$(INTDIR)\jidctflt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jidctfst.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jidctfst.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jidctfst.obj"	"$(INTDIR)\jidctfst.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jidctint.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jidctint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jidctint.obj"	"$(INTDIR)\jidctint.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jmemmgr.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jmemmgr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jmemmgr.obj"	"$(INTDIR)\jmemmgr.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jmemnobs.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jmemnobs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jmemnobs.obj"	"$(INTDIR)\jmemnobs.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jquant1.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jquant1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jquant1.obj"	"$(INTDIR)\jquant1.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jquant2.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jquant2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jquant2.obj"	"$(INTDIR)\jquant2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\jutils.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\jutils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\jutils.obj"	"$(INTDIR)\jutils.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibJPEG\transupp.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\transupp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\transupp.obj"	"$(INTDIR)\transupp.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\png.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\png.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\png.obj"	"$(INTDIR)\png.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngerror.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngerror.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngerror.obj"	"$(INTDIR)\pngerror.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngget.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngget.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngget.obj"	"$(INTDIR)\pngget.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngmem.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngmem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngmem.obj"	"$(INTDIR)\pngmem.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngpread.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngpread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngpread.obj"	"$(INTDIR)\pngpread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngread.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngread.obj"	"$(INTDIR)\pngread.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngrio.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngrio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngrio.obj"	"$(INTDIR)\pngrio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngrtran.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngrtran.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngrtran.obj"	"$(INTDIR)\pngrtran.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngrutil.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngrutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngrutil.obj"	"$(INTDIR)\pngrutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngset.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngset.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngset.obj"	"$(INTDIR)\pngset.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngtrans.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngtrans.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngtrans.obj"	"$(INTDIR)\pngtrans.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngwio.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngwio.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngwio.obj"	"$(INTDIR)\pngwio.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngwrite.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngwrite.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngwrite.obj"	"$(INTDIR)\pngwrite.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngwtran.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngwtran.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngwtran.obj"	"$(INTDIR)\pngwtran.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\LibPNG\pngwutil.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\pngwutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\pngwutil.obj"	"$(INTDIR)\pngwutil.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\BitmapAccess.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\BitmapAccess.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\BitmapAccess.obj"	"$(INTDIR)\BitmapAccess.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\ColorLookup.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\ColorLookup.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\ColorLookup.obj"	"$(INTDIR)\ColorLookup.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\FreeImage.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\FreeImage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\FreeImage.obj"	"$(INTDIR)\FreeImage.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\FreeImage.rc

"$(INTDIR)\FreeImage.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\Source\FreeImage\FreeImageC.c

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\FreeImageC.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\FreeImageC.obj"	"$(INTDIR)\FreeImageC.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\FreeImageIO.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\FreeImageIO.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\FreeImageIO.obj"	"$(INTDIR)\FreeImageIO.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\GetType.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\GetType.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\GetType.obj"	"$(INTDIR)\GetType.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\MemoryIO.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\MemoryIO.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\MemoryIO.obj"	"$(INTDIR)\MemoryIO.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImage\PixelAccess.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\PixelAccess.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\PixelAccess.obj"	"$(INTDIR)\PixelAccess.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\BSplineRotate.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\BSplineRotate.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\BSplineRotate.obj"	"$(INTDIR)\BSplineRotate.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\Channels.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Channels.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Channels.obj"	"$(INTDIR)\Channels.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\ClassicRotate.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\ClassicRotate.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\ClassicRotate.obj"	"$(INTDIR)\ClassicRotate.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\Colors.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Colors.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Colors.obj"	"$(INTDIR)\Colors.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\CopyPaste.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\CopyPaste.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\CopyPaste.obj"	"$(INTDIR)\CopyPaste.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\Display.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Display.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Display.obj"	"$(INTDIR)\Display.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\Flip.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Flip.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Flip.obj"	"$(INTDIR)\Flip.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\JPEGTransform.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\JPEGTransform.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\JPEGTransform.obj"	"$(INTDIR)\JPEGTransform.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\Rescale.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Rescale.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Rescale.obj"	"$(INTDIR)\Rescale.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\Source\FreeImageToolkit\Resize.cpp

!IF  "$(CFG)" == "FreeImage - Win32 Release"


"$(INTDIR)\Resize.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"


"$(INTDIR)\Resize.obj"	"$(INTDIR)\Resize.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

!IF  "$(CFG)" == "FreeImage - Win32 Release"

"zlib - Win32 Release" : 
   cd "..\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release" 
   cd "..\freeimage"

"zlib - Win32 ReleaseCLEAN" : 
   cd "..\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release" RECURSE=1 CLEAN 
   cd "..\freeimage"

!ELSEIF  "$(CFG)" == "FreeImage - Win32 Debug"

"zlib - Win32 Debug" : 
   cd "..\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug" 
   cd "..\freeimage"

"zlib - Win32 DebugCLEAN" : 
   cd "..\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\freeimage"

!ENDIF 


!ENDIF 

