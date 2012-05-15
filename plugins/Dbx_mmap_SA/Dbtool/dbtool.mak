# Microsoft Developer Studio Generated NMAKE File, Based on dbtool.dsp
!IF "$(CFG)" == ""
CFG=dbtool - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to dbtool - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "dbtool - Win32 Release" && "$(CFG)" != "dbtool - Win32 Debug" && "$(CFG)" != "dbtool - Win32 Debug Unicode" && "$(CFG)" != "dbtool - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "dbtool.mak" CFG="dbtool - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "dbtool - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "dbtool - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "dbtool - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE "dbtool - Win32 Release Unicode" (based on "Win32 (x86) Application")
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

!IF  "$(CFG)" == "dbtool - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\miranda\bin\release\dbtool.exe"


CLEAN :
	-@erase "$(INTDIR)\aggressive.obj"
	-@erase "$(INTDIR)\cleaning.obj"
	-@erase "$(INTDIR)\contactchain.obj"
	-@erase "$(INTDIR)\dbtool.pch"
	-@erase "$(INTDIR)\disk.obj"
	-@erase "$(INTDIR)\eventchain.obj"
	-@erase "$(INTDIR)\fileaccess.obj"
	-@erase "$(INTDIR)\finaltasks.obj"
	-@erase "$(INTDIR)\finished.obj"
	-@erase "$(INTDIR)\initialchecks.obj"
	-@erase "$(INTDIR)\langpack.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\modulechain.obj"
	-@erase "$(INTDIR)\openerror.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\selectdb.obj"
	-@erase "$(INTDIR)\settingschain.obj"
	-@erase "$(INTDIR)\user.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\welcome.obj"
	-@erase "$(INTDIR)\wizard.obj"
	-@erase "$(INTDIR)\worker.obj"
	-@erase "$(OUTDIR)\dbtool.map"
	-@erase "..\..\miranda\bin\release\dbtool.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DATABASE_INDEPENDANT" /Fp"$(INTDIR)\dbtool.pch" /Yu"dbtool.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbtool.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\dbtool.pdb" /map:"$(INTDIR)\dbtool.map" /machine:I386 /out:"../../miranda/bin/release/dbtool.exe" 
LINK32_OBJS= \
	"$(INTDIR)\aggressive.obj" \
	"$(INTDIR)\contactchain.obj" \
	"$(INTDIR)\eventchain.obj" \
	"$(INTDIR)\finaltasks.obj" \
	"$(INTDIR)\initialchecks.obj" \
	"$(INTDIR)\modulechain.obj" \
	"$(INTDIR)\settingschain.obj" \
	"$(INTDIR)\user.obj" \
	"$(INTDIR)\cleaning.obj" \
	"$(INTDIR)\fileaccess.obj" \
	"$(INTDIR)\finished.obj" \
	"$(INTDIR)\openerror.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\selectdb.obj" \
	"$(INTDIR)\welcome.obj" \
	"$(INTDIR)\disk.obj" \
	"$(INTDIR)\langpack.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\wizard.obj" \
	"$(INTDIR)\worker.obj" \
	"$(INTDIR)\resource.res"

"..\..\miranda\bin\release\dbtool.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dbtool - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\miranda\bin\debug\dbtool.exe"


CLEAN :
	-@erase "$(INTDIR)\aggressive.obj"
	-@erase "$(INTDIR)\cleaning.obj"
	-@erase "$(INTDIR)\contactchain.obj"
	-@erase "$(INTDIR)\dbtool.pch"
	-@erase "$(INTDIR)\disk.obj"
	-@erase "$(INTDIR)\eventchain.obj"
	-@erase "$(INTDIR)\fileaccess.obj"
	-@erase "$(INTDIR)\finaltasks.obj"
	-@erase "$(INTDIR)\finished.obj"
	-@erase "$(INTDIR)\initialchecks.obj"
	-@erase "$(INTDIR)\langpack.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\modulechain.obj"
	-@erase "$(INTDIR)\openerror.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\selectdb.obj"
	-@erase "$(INTDIR)\settingschain.obj"
	-@erase "$(INTDIR)\user.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\welcome.obj"
	-@erase "$(INTDIR)\wizard.obj"
	-@erase "$(INTDIR)\worker.obj"
	-@erase "$(OUTDIR)\dbtool.pdb"
	-@erase "..\..\miranda\bin\debug\dbtool.exe"
	-@erase "..\..\miranda\bin\debug\dbtool.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DATABASE_INDEPENDANT" /Fp"$(INTDIR)\dbtool.pch" /Yu"dbtool.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbtool.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\dbtool.pdb" /debug /machine:I386 /out:"../../miranda/bin/debug/dbtool.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\aggressive.obj" \
	"$(INTDIR)\contactchain.obj" \
	"$(INTDIR)\eventchain.obj" \
	"$(INTDIR)\finaltasks.obj" \
	"$(INTDIR)\initialchecks.obj" \
	"$(INTDIR)\modulechain.obj" \
	"$(INTDIR)\settingschain.obj" \
	"$(INTDIR)\user.obj" \
	"$(INTDIR)\cleaning.obj" \
	"$(INTDIR)\fileaccess.obj" \
	"$(INTDIR)\finished.obj" \
	"$(INTDIR)\openerror.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\selectdb.obj" \
	"$(INTDIR)\welcome.obj" \
	"$(INTDIR)\disk.obj" \
	"$(INTDIR)\langpack.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\wizard.obj" \
	"$(INTDIR)\worker.obj" \
	"$(INTDIR)\resource.res"

"..\..\miranda\bin\debug\dbtool.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dbtool - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode

ALL : "..\..\miranda\bin\debug unicode\dbtool.exe"


CLEAN :
	-@erase "$(INTDIR)\aggressive.obj"
	-@erase "$(INTDIR)\cleaning.obj"
	-@erase "$(INTDIR)\contactchain.obj"
	-@erase "$(INTDIR)\dbtool.pch"
	-@erase "$(INTDIR)\disk.obj"
	-@erase "$(INTDIR)\eventchain.obj"
	-@erase "$(INTDIR)\fileaccess.obj"
	-@erase "$(INTDIR)\finaltasks.obj"
	-@erase "$(INTDIR)\finished.obj"
	-@erase "$(INTDIR)\initialchecks.obj"
	-@erase "$(INTDIR)\langpack.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\modulechain.obj"
	-@erase "$(INTDIR)\openerror.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\selectdb.obj"
	-@erase "$(INTDIR)\settingschain.obj"
	-@erase "$(INTDIR)\user.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\welcome.obj"
	-@erase "$(INTDIR)\wizard.obj"
	-@erase "$(INTDIR)\worker.obj"
	-@erase "$(OUTDIR)\dbtool.pdb"
	-@erase "..\..\miranda\bin\debug unicode\dbtool.exe"
	-@erase "..\..\miranda\bin\debug unicode\dbtool.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "DATABASE_INDEPENDANT" /Fp"$(INTDIR)\dbtool.pch" /Yu"dbtool.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbtool.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\dbtool.pdb" /debug /machine:I386 /out:"../../miranda/bin/debug unicode/dbtool.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\aggressive.obj" \
	"$(INTDIR)\contactchain.obj" \
	"$(INTDIR)\eventchain.obj" \
	"$(INTDIR)\finaltasks.obj" \
	"$(INTDIR)\initialchecks.obj" \
	"$(INTDIR)\modulechain.obj" \
	"$(INTDIR)\settingschain.obj" \
	"$(INTDIR)\user.obj" \
	"$(INTDIR)\cleaning.obj" \
	"$(INTDIR)\fileaccess.obj" \
	"$(INTDIR)\finished.obj" \
	"$(INTDIR)\openerror.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\selectdb.obj" \
	"$(INTDIR)\welcome.obj" \
	"$(INTDIR)\disk.obj" \
	"$(INTDIR)\langpack.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\wizard.obj" \
	"$(INTDIR)\worker.obj" \
	"$(INTDIR)\resource.res"

"..\..\miranda\bin\debug unicode\dbtool.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "dbtool - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\miranda\bin\release unicode\dbtool.exe"


CLEAN :
	-@erase "$(INTDIR)\aggressive.obj"
	-@erase "$(INTDIR)\cleaning.obj"
	-@erase "$(INTDIR)\contactchain.obj"
	-@erase "$(INTDIR)\dbtool.pch"
	-@erase "$(INTDIR)\disk.obj"
	-@erase "$(INTDIR)\eventchain.obj"
	-@erase "$(INTDIR)\fileaccess.obj"
	-@erase "$(INTDIR)\finaltasks.obj"
	-@erase "$(INTDIR)\finished.obj"
	-@erase "$(INTDIR)\initialchecks.obj"
	-@erase "$(INTDIR)\langpack.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\modulechain.obj"
	-@erase "$(INTDIR)\openerror.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\selectdb.obj"
	-@erase "$(INTDIR)\settingschain.obj"
	-@erase "$(INTDIR)\user.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\welcome.obj"
	-@erase "$(INTDIR)\wizard.obj"
	-@erase "$(INTDIR)\worker.obj"
	-@erase "$(OUTDIR)\dbtool.map"
	-@erase "..\..\miranda\bin\release unicode\dbtool.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "DATABASE_INDEPENDANT" /Fp"$(INTDIR)\dbtool.pch" /Yu"dbtool.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\dbtool.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\dbtool.pdb" /map:"$(INTDIR)\dbtool.map" /machine:I386 /out:"../../miranda/bin/release unicode/dbtool.exe" 
LINK32_OBJS= \
	"$(INTDIR)\aggressive.obj" \
	"$(INTDIR)\contactchain.obj" \
	"$(INTDIR)\eventchain.obj" \
	"$(INTDIR)\finaltasks.obj" \
	"$(INTDIR)\initialchecks.obj" \
	"$(INTDIR)\modulechain.obj" \
	"$(INTDIR)\settingschain.obj" \
	"$(INTDIR)\user.obj" \
	"$(INTDIR)\cleaning.obj" \
	"$(INTDIR)\fileaccess.obj" \
	"$(INTDIR)\finished.obj" \
	"$(INTDIR)\openerror.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\selectdb.obj" \
	"$(INTDIR)\welcome.obj" \
	"$(INTDIR)\disk.obj" \
	"$(INTDIR)\langpack.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\wizard.obj" \
	"$(INTDIR)\worker.obj" \
	"$(INTDIR)\resource.res"

"..\..\miranda\bin\release unicode\dbtool.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("dbtool.dep")
!INCLUDE "dbtool.dep"
!ELSE 
!MESSAGE Warning: cannot find "dbtool.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "dbtool - Win32 Release" || "$(CFG)" == "dbtool - Win32 Debug" || "$(CFG)" == "dbtool - Win32 Debug Unicode" || "$(CFG)" == "dbtool - Win32 Release Unicode"
SOURCE=.\aggressive.cpp

"$(INTDIR)\aggressive.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\contactchain.cpp

"$(INTDIR)\contactchain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\eventchain.cpp

"$(INTDIR)\eventchain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\finaltasks.cpp

"$(INTDIR)\finaltasks.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\initialchecks.cpp

"$(INTDIR)\initialchecks.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\modulechain.cpp

"$(INTDIR)\modulechain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\settingschain.cpp

"$(INTDIR)\settingschain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\user.cpp

"$(INTDIR)\user.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\cleaning.cpp

"$(INTDIR)\cleaning.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\fileaccess.cpp

"$(INTDIR)\fileaccess.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\finished.cpp

"$(INTDIR)\finished.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\openerror.cpp

"$(INTDIR)\openerror.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\progress.cpp

"$(INTDIR)\progress.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\selectdb.cpp

"$(INTDIR)\selectdb.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\welcome.cpp

"$(INTDIR)\welcome.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\disk.cpp

"$(INTDIR)\disk.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\langpack.cpp

"$(INTDIR)\langpack.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\main.cpp

!IF  "$(CFG)" == "dbtool - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DATABASE_INDEPENDANT" /Fp"$(INTDIR)\dbtool.pch" /Yc"dbtool.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\dbtool.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "dbtool - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DATABASE_INDEPENDANT" /Fp"$(INTDIR)\dbtool.pch" /Yc"dbtool.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\dbtool.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "dbtool - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "DATABASE_INDEPENDANT" /Fp"$(INTDIR)\dbtool.pch" /Yc"dbtool.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\dbtool.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "dbtool - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "DATABASE_INDEPENDANT" /Fp"$(INTDIR)\dbtool.pch" /Yc"dbtool.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\dbtool.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\wizard.cpp

"$(INTDIR)\wizard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\worker.cpp

"$(INTDIR)\worker.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\dbtool.pch"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

