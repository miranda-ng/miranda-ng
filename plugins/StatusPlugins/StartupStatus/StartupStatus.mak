# Microsoft Developer Studio Generated NMAKE File, Based on StartupStatus.dsp
!IF "$(CFG)" == ""
CFG=StartupStatus - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to StartupStatus - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "StartupStatus - Win32 Release" && "$(CFG)" != "StartupStatus - Win32 Debug" && "$(CFG)" != "StartupStatus - Win32 Debug Unicode" && "$(CFG)" != "StartupStatus - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StartupStatus.mak" CFG="StartupStatus - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StartupStatus - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "StartupStatus - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "StartupStatus - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "StartupStatus - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "StartupStatus - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\..\bin\release\plugins\StartupStatus.dll"


CLEAN :
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\profiles.obj"
	-@erase "$(INTDIR)\startupstatus.obj"
	-@erase "$(INTDIR)\toolbars.obj"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\StartupStatus.exp"
	-@erase "$(OUTDIR)\StartupStatus.lib"
	-@erase "$(OUTDIR)\StartupStatus.pdb"
	-@erase "..\..\..\bin\release\plugins\StartupStatus.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STARTUPSTATUS_EXPORTS" /Fp"$(INTDIR)\StartupStatus.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\StartupStatus.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrt.lib wininet.lib Delayimp.lib /nologo /base:"0x11bd0000" /dll /incremental:no /pdb:"$(OUTDIR)\StartupStatus.pdb" /debug /machine:I386 /nodefaultlib /out:"../../../bin/release/plugins/StartupStatus.dll" /implib:"$(OUTDIR)\StartupStatus.lib" /DELAYLOAD:wininet.dll 
LINK32_OBJS= \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\profiles.obj" \
	"$(INTDIR)\startupstatus.obj" \
	"$(INTDIR)\vc6.res" \
	"$(INTDIR)\toolbars.obj"

"..\..\..\bin\release\plugins\StartupStatus.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "StartupStatus - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\..\bin\debug\plugins\StartupStatus.dll"


CLEAN :
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\profiles.obj"
	-@erase "$(INTDIR)\startupstatus.obj"
	-@erase "$(INTDIR)\toolbars.obj"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\StartupStatus.exp"
	-@erase "$(OUTDIR)\StartupStatus.lib"
	-@erase "$(OUTDIR)\StartupStatus.pdb"
	-@erase "..\..\..\bin\debug\plugins\StartupStatus.dll"
	-@erase "..\..\..\bin\debug\plugins\StartupStatus.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "STARTUPSTATUS_EXPORTS" /Fp"$(INTDIR)\StartupStatus.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\StartupStatus.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wininet.lib Delayimp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\StartupStatus.pdb" /debug /machine:I386 /out:"../../../bin/debug/plugins/StartupStatus.dll" /implib:"$(OUTDIR)\StartupStatus.lib" /pdbtype:sept /DELAYLOAD:wininet.dll 
LINK32_OBJS= \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\profiles.obj" \
	"$(INTDIR)\startupstatus.obj" \
	"$(INTDIR)\vc6.res" \
	"$(INTDIR)\toolbars.obj"

"..\..\..\bin\debug\plugins\StartupStatus.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "StartupStatus - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode

ALL : "..\..\..\bin\Debug Unicode\plugins\StartupStatus.dll"


CLEAN :
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\profiles.obj"
	-@erase "$(INTDIR)\startupstatus.obj"
	-@erase "$(INTDIR)\toolbars.obj"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\StartupStatus.exp"
	-@erase "$(OUTDIR)\StartupStatus.lib"
	-@erase "$(OUTDIR)\StartupStatus.pdb"
	-@erase "..\..\..\bin\Debug Unicode\plugins\StartupStatus.dll"
	-@erase "..\..\..\bin\Debug Unicode\plugins\StartupStatus.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../../include" /I "../includes" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /Fp"$(INTDIR)\StartupStatus.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\StartupStatus.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=wininet.lib Delayimp.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\StartupStatus.pdb" /debug /machine:I386 /out:"../../../bin/Debug Unicode/plugins/StartupStatus.dll" /implib:"$(OUTDIR)\StartupStatus.lib" /pdbtype:sept /DELAYLOAD:wininet.dll 
LINK32_OBJS= \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\profiles.obj" \
	"$(INTDIR)\startupstatus.obj" \
	"$(INTDIR)\vc6.res" \
	"$(INTDIR)\toolbars.obj"

"..\..\..\bin\Debug Unicode\plugins\StartupStatus.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "StartupStatus - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\..\bin\Release Unicode\plugins\StartupStatus.dll"


CLEAN :
	-@erase "$(INTDIR)\commonstatus.obj"
	-@erase "$(INTDIR)\confirmdialog.obj"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\profiles.obj"
	-@erase "$(INTDIR)\startupstatus.obj"
	-@erase "$(INTDIR)\toolbars.obj"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\StartupStatus.exp"
	-@erase "$(OUTDIR)\StartupStatus.lib"
	-@erase "$(OUTDIR)\StartupStatus.pdb"
	-@erase "..\..\..\bin\Release Unicode\plugins\StartupStatus.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../../include" /I "../includes" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_UNICODE" /D "_USRDLL" /Fp"$(INTDIR)\StartupStatus.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\StartupStatus.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib msvcrt.lib wininet.lib Delayimp.lib /nologo /base:"0x11bd0000" /dll /incremental:no /pdb:"$(OUTDIR)\StartupStatus.pdb" /debug /machine:I386 /nodefaultlib /out:"../../../bin/Release Unicode/plugins/StartupStatus.dll" /implib:"$(OUTDIR)\StartupStatus.lib" /DELAYLOAD:wininet.dll 
LINK32_OBJS= \
	"$(INTDIR)\commonstatus.obj" \
	"$(INTDIR)\confirmdialog.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\profiles.obj" \
	"$(INTDIR)\startupstatus.obj" \
	"$(INTDIR)\vc6.res" \
	"$(INTDIR)\toolbars.obj"

"..\..\..\bin\Release Unicode\plugins\StartupStatus.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("StartupStatus.dep")
!INCLUDE "StartupStatus.dep"
!ELSE 
!MESSAGE Warning: cannot find "StartupStatus.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "StartupStatus - Win32 Release" || "$(CFG)" == "StartupStatus - Win32 Debug" || "$(CFG)" == "StartupStatus - Win32 Debug Unicode" || "$(CFG)" == "StartupStatus - Win32 Release Unicode"
SOURCE=..\commonstatus.cpp

"$(INTDIR)\commonstatus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\confirmdialog.cpp

"$(INTDIR)\confirmdialog.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\helpers\gen_helpers.cpp

"$(INTDIR)\gen_helpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\options.cpp

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\profiles.cpp

"$(INTDIR)\profiles.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\startupstatus.cpp

"$(INTDIR)\startupstatus.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\toolbars.cpp

"$(INTDIR)\toolbars.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vc6.rc

"$(INTDIR)\vc6.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

