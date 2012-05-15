# Microsoft Developer Studio Generated NMAKE File, Based on splashscreen.dsp
!IF "$(CFG)" == ""
CFG=AdvSplashScreen - Win32 Debug
!MESSAGE No configuration specified. Defaulting to AdvSplashScreen - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "AdvSplashScreen - Win32 Release" && "$(CFG)" != "AdvSplashScreen - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "splashscreen.mak" CFG="AdvSplashScreen - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AdvSplashScreen - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "AdvSplashScreen - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "AdvSplashScreen - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\release\plugins\AdvSplashScreen.dll"


CLEAN :
	-@erase "$(INTDIR)\bitmap_funcs.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\splash.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\AdvSplashScreen.exp"
	-@erase "..\..\bin\release\plugins\AdvSplashScreen.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O1 /I "../../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FINGER_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /I "../../include" /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\splash.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\splashscreen.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /verbose /dll /pdb:none /machine:I386 /out:"../../bin/release/plugins/AdvSplashScreen.dll" /implib:"$(OUTDIR)\AdvSplashScreen.lib" 
LINK32_OBJS= \
	"$(INTDIR)\bitmap_funcs.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\splash.res"

"..\..\bin\release\plugins\AdvSplashScreen.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AdvSplashScreen - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\splashscreen.dll"


CLEAN :
	-@erase "$(INTDIR)\bitmap_funcs.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\splash.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\splashscreen.dll"
	-@erase "$(OUTDIR)\splashscreen.exp"
	-@erase "$(OUTDIR)\splashscreen.ilk"
	-@erase "$(OUTDIR)\splashscreen.lib"
	-@erase "$(OUTDIR)\splashscreen.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "FINGER_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
MTL_PROJ=/nologo /I "../../../include" /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\splash.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\splashscreen.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\splashscreen.pdb" /debug /machine:I386 /out:"$(OUTDIR)\splashscreen.dll" /implib:"$(OUTDIR)\splashscreen.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\bitmap_funcs.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\splash.res"

"$(OUTDIR)\splashscreen.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("splashscreen.dep")
!INCLUDE "splashscreen.dep"
!ELSE 
!MESSAGE Warning: cannot find "splashscreen.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "AdvSplashScreen - Win32 Release" || "$(CFG)" == "AdvSplashScreen - Win32 Debug"
SOURCE=.\src\bitmap_funcs.cpp

"$(INTDIR)\bitmap_funcs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\options.cpp

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\src\splash.rc

!IF  "$(CFG)" == "AdvSplashScreen - Win32 Release"


"$(INTDIR)\splash.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x809 /fo"$(INTDIR)\splash.res" /i "src" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "AdvSplashScreen - Win32 Debug"


"$(INTDIR)\splash.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x419 /fo"$(INTDIR)\splash.res" /i "src" /d "_DEBUG" $(SOURCE)


!ENDIF 

SOURCE=.\src\version.rc

!ENDIF 

