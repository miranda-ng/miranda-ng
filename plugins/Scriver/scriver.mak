# Microsoft Developer Studio Generated NMAKE File, Based on scriver.dsp
!IF "$(CFG)" == ""
CFG=scriver - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to scriver - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "scriver - Win32 Release" && "$(CFG)" != "scriver - Win32 Debug" && "$(CFG)" != "scriver - Win32 Release Unicode" && "$(CFG)" != "scriver - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "scriver.mak" CFG="scriver - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "scriver - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "scriver - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "scriver - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "scriver - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "scriver - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\Release\plugins\scriver.dll"


CLEAN :
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\cmdlist.obj"
	-@erase "$(INTDIR)\colorchooser.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\infobar.obj"
	-@erase "$(INTDIR)\input.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\manager.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\msgtimedout.obj"
	-@erase "$(INTDIR)\msgwindow.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\richutil.obj"
	-@erase "$(INTDIR)\sendqueue.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\statusicon.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(OUTDIR)\scriver.exp"
	-@erase "$(OUTDIR)\scriver.pdb"
	-@erase "..\..\bin\Release\plugins\scriver.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /I "../../include/msapi" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\scriver.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\scriver.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib shlwapi.lib Version.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\scriver.pdb" /debug /machine:I386 /out:"../../bin/Release/plugins/scriver.dll" /implib:"$(OUTDIR)\scriver.lib" 
LINK32_OBJS= \
	"$(INTDIR)\cmdlist.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\infobar.obj" \
	"$(INTDIR)\input.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\msgtimedout.obj" \
	"$(INTDIR)\msgwindow.obj" \
	"$(INTDIR)\richutil.obj" \
	"$(INTDIR)\sendqueue.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\statusicon.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\colorchooser.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Release\plugins\scriver.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "scriver - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\bin\Debug\plugins\scriver.dll"


CLEAN :
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\cmdlist.obj"
	-@erase "$(INTDIR)\colorchooser.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\infobar.obj"
	-@erase "$(INTDIR)\input.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\manager.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\msgtimedout.obj"
	-@erase "$(INTDIR)\msgwindow.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\richutil.obj"
	-@erase "$(INTDIR)\sendqueue.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\statusicon.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(OUTDIR)\scriver.exp"
	-@erase "$(OUTDIR)\scriver.pdb"
	-@erase "..\..\bin\Debug\plugins\scriver.dll"
	-@erase "..\..\bin\Debug\plugins\scriver.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\scriver.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\scriver.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib shlwapi.lib Version.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\scriver.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/scriver.dll" /implib:"$(OUTDIR)\scriver.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\cmdlist.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\infobar.obj" \
	"$(INTDIR)\input.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\msgtimedout.obj" \
	"$(INTDIR)\msgwindow.obj" \
	"$(INTDIR)\richutil.obj" \
	"$(INTDIR)\sendqueue.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\statusicon.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\colorchooser.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Debug\plugins\scriver.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "scriver - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\Release Unicode\plugins\scriver.dll"


CLEAN :
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\cmdlist.obj"
	-@erase "$(INTDIR)\colorchooser.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\infobar.obj"
	-@erase "$(INTDIR)\input.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\manager.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\msgtimedout.obj"
	-@erase "$(INTDIR)\msgwindow.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\richutil.obj"
	-@erase "$(INTDIR)\sendqueue.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\statusicon.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(OUTDIR)\scriver.exp"
	-@erase "$(OUTDIR)\scriver.pdb"
	-@erase "..\..\bin\Release Unicode\plugins\scriver.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include/msapi" /I "../../include" /D "UNICODE" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /D "_WIN32_IE 0x0500" /Fp"$(INTDIR)\scriver.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "NDEBUG" /d "UNICODE" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\scriver.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib shlwapi.lib Version.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\scriver.pdb" /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/scriver.dll" /implib:"$(OUTDIR)\scriver.lib" 
LINK32_OBJS= \
	"$(INTDIR)\cmdlist.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\infobar.obj" \
	"$(INTDIR)\input.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\msgtimedout.obj" \
	"$(INTDIR)\msgwindow.obj" \
	"$(INTDIR)\richutil.obj" \
	"$(INTDIR)\sendqueue.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\statusicon.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\colorchooser.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Release Unicode\plugins\scriver.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "scriver - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode

ALL : "..\..\bin\Debug Unicode\plugins\scriver.dll"


CLEAN :
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\cmdlist.obj"
	-@erase "$(INTDIR)\colorchooser.obj"
	-@erase "$(INTDIR)\globals.obj"
	-@erase "$(INTDIR)\infobar.obj"
	-@erase "$(INTDIR)\input.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\manager.obj"
	-@erase "$(INTDIR)\message.obj"
	-@erase "$(INTDIR)\msgdialog.obj"
	-@erase "$(INTDIR)\msglog.obj"
	-@erase "$(INTDIR)\msgoptions.obj"
	-@erase "$(INTDIR)\msgs.obj"
	-@erase "$(INTDIR)\msgtimedout.obj"
	-@erase "$(INTDIR)\msgwindow.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\richutil.obj"
	-@erase "$(INTDIR)\sendqueue.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\srmm.obj"
	-@erase "$(INTDIR)\statusicon.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\window.obj"
	-@erase "$(OUTDIR)\scriver.exp"
	-@erase "$(OUTDIR)\scriver.pdb"
	-@erase "..\..\bin\Debug Unicode\plugins\scriver.dll"
	-@erase "..\..\bin\Debug Unicode\plugins\scriver.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "../../include/msapi" /D "UNICODE" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SRMM_EXPORTS" /Fp"$(INTDIR)\scriver.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "_DEBUG" /d "UNICODE" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\scriver.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=comctl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib ole32.lib comdlg32.lib shlwapi.lib Version.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\scriver.pdb" /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/scriver.dll" /implib:"$(OUTDIR)\scriver.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\cmdlist.obj" \
	"$(INTDIR)\globals.obj" \
	"$(INTDIR)\infobar.obj" \
	"$(INTDIR)\input.obj" \
	"$(INTDIR)\msgdialog.obj" \
	"$(INTDIR)\msglog.obj" \
	"$(INTDIR)\msgoptions.obj" \
	"$(INTDIR)\msgs.obj" \
	"$(INTDIR)\msgtimedout.obj" \
	"$(INTDIR)\msgwindow.obj" \
	"$(INTDIR)\richutil.obj" \
	"$(INTDIR)\sendqueue.obj" \
	"$(INTDIR)\srmm.obj" \
	"$(INTDIR)\statusicon.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\colorchooser.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\manager.obj" \
	"$(INTDIR)\message.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\window.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Debug Unicode\plugins\scriver.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

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


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("scriver.dep")
!INCLUDE "scriver.dep"
!ELSE 
!MESSAGE Warning: cannot find "scriver.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "scriver - Win32 Release" || "$(CFG)" == "scriver - Win32 Debug" || "$(CFG)" == "scriver - Win32 Release Unicode" || "$(CFG)" == "scriver - Win32 Debug Unicode"
SOURCE=.\cmdlist.c

"$(INTDIR)\cmdlist.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\globals.c

"$(INTDIR)\globals.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\infobar.c

"$(INTDIR)\infobar.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\input.c

"$(INTDIR)\input.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\msgdialog.c

"$(INTDIR)\msgdialog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\msglog.c

"$(INTDIR)\msglog.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\msgoptions.c

"$(INTDIR)\msgoptions.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\msgs.c

"$(INTDIR)\msgs.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\msgtimedout.c

"$(INTDIR)\msgtimedout.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\msgwindow.c

"$(INTDIR)\msgwindow.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\richutil.c

"$(INTDIR)\richutil.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sendqueue.c

"$(INTDIR)\sendqueue.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\srmm.c

"$(INTDIR)\srmm.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\statusicon.c

"$(INTDIR)\statusicon.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\utils.c

"$(INTDIR)\utils.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\chat\clist.c

"$(INTDIR)\clist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\chat\colorchooser.c

"$(INTDIR)\colorchooser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\chat\log.c

"$(INTDIR)\log.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\chat\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\chat\manager.c

"$(INTDIR)\manager.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\chat\message.c

"$(INTDIR)\message.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\chat\options.c

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\chat\services.c

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\chat\tools.c

"$(INTDIR)\tools.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\chat\window.c

"$(INTDIR)\window.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

