# Microsoft Developer Studio Generated NMAKE File, Based on db3x.dsp
!IF "$(CFG)" == ""
CFG=db3x - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to db3x - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "db3x - Win32 Release" && "$(CFG)" != "db3x - Win32 Debug" && "$(CFG)" != "db3x - Win32 Release Unicode" && "$(CFG)" != "db3x - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "db3x.mak" CFG="db3x - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "db3x - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "db3x - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "db3x - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "db3x - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "db3x - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\release\plugins\dbx_3x.dll"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\database.obj"
	-@erase "$(INTDIR)\db3x.pch"
	-@erase "$(INTDIR)\dbcache.obj"
	-@erase "$(INTDIR)\dbcontacts.obj"
	-@erase "$(INTDIR)\dbevents.obj"
	-@erase "$(INTDIR)\dbheaders.obj"
	-@erase "$(INTDIR)\dbmodulechain.obj"
	-@erase "$(INTDIR)\dbsettings.obj"
	-@erase "$(INTDIR)\encrypt.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\dbx_3x.exp"
	-@erase "$(OUTDIR)\dbx_3x.lib"
	-@erase "$(OUTDIR)\dbx_3x.map"
	-@erase "$(OUTDIR)\dbx_3x.pdb"
	-@erase "..\..\bin\release\plugins\dbx_3x.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DB3X_EXPORTS" /Fp"$(INTDIR)\db3x.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\db3x.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x5130000" /dll /incremental:no /pdb:"$(OUTDIR)\dbx_3x.pdb" /map:"$(INTDIR)\dbx_3x.map" /debug /machine:I386 /out:"../../bin/release/plugins/dbx_3x.dll" /implib:"$(OUTDIR)\dbx_3x.lib" /IGNORE:4089 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\database.obj" \
	"$(INTDIR)\dbcache.obj" \
	"$(INTDIR)\dbcontacts.obj" \
	"$(INTDIR)\dbevents.obj" \
	"$(INTDIR)\dbheaders.obj" \
	"$(INTDIR)\dbmodulechain.obj" \
	"$(INTDIR)\dbsettings.obj" \
	"$(INTDIR)\encrypt.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\plugins\dbx_3x.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\bin\debug\plugins\dbx_3x.dll" "$(OUTDIR)\db3x.bsc"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\commonheaders.sbr"
	-@erase "$(INTDIR)\database.obj"
	-@erase "$(INTDIR)\database.sbr"
	-@erase "$(INTDIR)\db3x.pch"
	-@erase "$(INTDIR)\dbcache.obj"
	-@erase "$(INTDIR)\dbcache.sbr"
	-@erase "$(INTDIR)\dbcontacts.obj"
	-@erase "$(INTDIR)\dbcontacts.sbr"
	-@erase "$(INTDIR)\dbevents.obj"
	-@erase "$(INTDIR)\dbevents.sbr"
	-@erase "$(INTDIR)\dbheaders.obj"
	-@erase "$(INTDIR)\dbheaders.sbr"
	-@erase "$(INTDIR)\dbmodulechain.obj"
	-@erase "$(INTDIR)\dbmodulechain.sbr"
	-@erase "$(INTDIR)\dbsettings.obj"
	-@erase "$(INTDIR)\dbsettings.sbr"
	-@erase "$(INTDIR)\encrypt.obj"
	-@erase "$(INTDIR)\encrypt.sbr"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\init.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\db3x.bsc"
	-@erase "$(OUTDIR)\dbx_3x.exp"
	-@erase "$(OUTDIR)\dbx_3x.lib"
	-@erase "$(OUTDIR)\dbx_3x.map"
	-@erase "$(OUTDIR)\dbx_3x.pdb"
	-@erase "..\..\bin\debug\plugins\dbx_3x.dll"
	-@erase "..\..\bin\debug\plugins\dbx_3x.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DB3X_EXPORTS" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\db3x.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\db3x.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commonheaders.sbr" \
	"$(INTDIR)\database.sbr" \
	"$(INTDIR)\dbcache.sbr" \
	"$(INTDIR)\dbcontacts.sbr" \
	"$(INTDIR)\dbevents.sbr" \
	"$(INTDIR)\dbheaders.sbr" \
	"$(INTDIR)\dbmodulechain.sbr" \
	"$(INTDIR)\dbsettings.sbr" \
	"$(INTDIR)\encrypt.sbr" \
	"$(INTDIR)\init.sbr"

"$(OUTDIR)\db3x.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\dbx_3x.pdb" /map:"$(INTDIR)\dbx_3x.map" /debug /machine:I386 /out:"../../bin/debug/plugins/dbx_3x.dll" /implib:"$(OUTDIR)\dbx_3x.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\database.obj" \
	"$(INTDIR)\dbcache.obj" \
	"$(INTDIR)\dbcontacts.obj" \
	"$(INTDIR)\dbevents.obj" \
	"$(INTDIR)\dbheaders.obj" \
	"$(INTDIR)\dbmodulechain.obj" \
	"$(INTDIR)\dbsettings.obj" \
	"$(INTDIR)\encrypt.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug\plugins\dbx_3x.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\release unicode\plugins\dbx_3x.dll"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\database.obj"
	-@erase "$(INTDIR)\db3x.pch"
	-@erase "$(INTDIR)\dbcache.obj"
	-@erase "$(INTDIR)\dbcontacts.obj"
	-@erase "$(INTDIR)\dbevents.obj"
	-@erase "$(INTDIR)\dbheaders.obj"
	-@erase "$(INTDIR)\dbmodulechain.obj"
	-@erase "$(INTDIR)\dbsettings.obj"
	-@erase "$(INTDIR)\encrypt.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\dbx_3x.exp"
	-@erase "$(OUTDIR)\dbx_3x.lib"
	-@erase "$(OUTDIR)\dbx_3x.map"
	-@erase "$(OUTDIR)\dbx_3x.pdb"
	-@erase "..\..\bin\release unicode\plugins\dbx_3x.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "DB3X_EXPORTS" /Fp"$(INTDIR)\db3x.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\db3x.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /base:"0x5130000" /dll /incremental:no /pdb:"$(OUTDIR)\dbx_3x.pdb" /map:"$(INTDIR)\dbx_3x.map" /debug /machine:I386 /out:"../../bin/release unicode/plugins/dbx_3x.dll" /implib:"$(OUTDIR)\dbx_3x.lib" /IGNORE:4089 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\database.obj" \
	"$(INTDIR)\dbcache.obj" \
	"$(INTDIR)\dbcontacts.obj" \
	"$(INTDIR)\dbevents.obj" \
	"$(INTDIR)\dbheaders.obj" \
	"$(INTDIR)\dbmodulechain.obj" \
	"$(INTDIR)\dbsettings.obj" \
	"$(INTDIR)\encrypt.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release unicode\plugins\dbx_3x.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\bin\debug Unicode\plugins\dbx_3x.dll" "$(OUTDIR)\db3x.bsc"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\commonheaders.sbr"
	-@erase "$(INTDIR)\database.obj"
	-@erase "$(INTDIR)\database.sbr"
	-@erase "$(INTDIR)\db3x.pch"
	-@erase "$(INTDIR)\dbcache.obj"
	-@erase "$(INTDIR)\dbcache.sbr"
	-@erase "$(INTDIR)\dbcontacts.obj"
	-@erase "$(INTDIR)\dbcontacts.sbr"
	-@erase "$(INTDIR)\dbevents.obj"
	-@erase "$(INTDIR)\dbevents.sbr"
	-@erase "$(INTDIR)\dbheaders.obj"
	-@erase "$(INTDIR)\dbheaders.sbr"
	-@erase "$(INTDIR)\dbmodulechain.obj"
	-@erase "$(INTDIR)\dbmodulechain.sbr"
	-@erase "$(INTDIR)\dbsettings.obj"
	-@erase "$(INTDIR)\dbsettings.sbr"
	-@erase "$(INTDIR)\encrypt.obj"
	-@erase "$(INTDIR)\encrypt.sbr"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\init.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\db3x.bsc"
	-@erase "$(OUTDIR)\dbx_3x.exp"
	-@erase "$(OUTDIR)\dbx_3x.lib"
	-@erase "$(OUTDIR)\dbx_3x.map"
	-@erase "$(OUTDIR)\dbx_3x.pdb"
	-@erase "..\..\bin\debug Unicode\plugins\dbx_3x.dll"
	-@erase "..\..\bin\debug Unicode\plugins\dbx_3x.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "DB3X_EXPORTS" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\db3x.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\db3x.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commonheaders.sbr" \
	"$(INTDIR)\database.sbr" \
	"$(INTDIR)\dbcache.sbr" \
	"$(INTDIR)\dbcontacts.sbr" \
	"$(INTDIR)\dbevents.sbr" \
	"$(INTDIR)\dbheaders.sbr" \
	"$(INTDIR)\dbmodulechain.sbr" \
	"$(INTDIR)\dbsettings.sbr" \
	"$(INTDIR)\encrypt.sbr" \
	"$(INTDIR)\init.sbr"

"$(OUTDIR)\db3x.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\dbx_3x.pdb" /map:"$(INTDIR)\dbx_3x.map" /debug /machine:I386 /out:"../../bin/debug Unicode/plugins/dbx_3x.dll" /implib:"$(OUTDIR)\dbx_3x.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\database.obj" \
	"$(INTDIR)\dbcache.obj" \
	"$(INTDIR)\dbcontacts.obj" \
	"$(INTDIR)\dbevents.obj" \
	"$(INTDIR)\dbheaders.obj" \
	"$(INTDIR)\dbmodulechain.obj" \
	"$(INTDIR)\dbsettings.obj" \
	"$(INTDIR)\encrypt.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug Unicode\plugins\dbx_3x.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("db3x.dep")
!INCLUDE "db3x.dep"
!ELSE 
!MESSAGE Warning: cannot find "db3x.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "db3x - Win32 Release" || "$(CFG)" == "db3x - Win32 Debug" || "$(CFG)" == "db3x - Win32 Release Unicode" || "$(CFG)" == "db3x - Win32 Debug Unicode"
SOURCE=.\commonheaders.c

!IF  "$(CFG)" == "db3x - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DB3X_EXPORTS" /Fp"$(INTDIR)\db3x.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\db3x.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "DB3X_EXPORTS" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\db3x.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\commonheaders.sbr"	"$(INTDIR)\db3x.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "DB3X_EXPORTS" /Fp"$(INTDIR)\db3x.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\db3x.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "DB3X_EXPORTS" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\db3x.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\commonheaders.sbr"	"$(INTDIR)\db3x.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\database.c

!IF  "$(CFG)" == "db3x - Win32 Release"


"$(INTDIR)\database.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"


"$(INTDIR)\database.obj"	"$(INTDIR)\database.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"


"$(INTDIR)\database.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"


"$(INTDIR)\database.obj"	"$(INTDIR)\database.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ENDIF 

SOURCE=.\dbcache.c

!IF  "$(CFG)" == "db3x - Win32 Release"


"$(INTDIR)\dbcache.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"


"$(INTDIR)\dbcache.obj"	"$(INTDIR)\dbcache.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"


"$(INTDIR)\dbcache.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"


"$(INTDIR)\dbcache.obj"	"$(INTDIR)\dbcache.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ENDIF 

SOURCE=.\dbcontacts.c

!IF  "$(CFG)" == "db3x - Win32 Release"


"$(INTDIR)\dbcontacts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"


"$(INTDIR)\dbcontacts.obj"	"$(INTDIR)\dbcontacts.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"


"$(INTDIR)\dbcontacts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"


"$(INTDIR)\dbcontacts.obj"	"$(INTDIR)\dbcontacts.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ENDIF 

SOURCE=.\dbevents.c

!IF  "$(CFG)" == "db3x - Win32 Release"


"$(INTDIR)\dbevents.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"


"$(INTDIR)\dbevents.obj"	"$(INTDIR)\dbevents.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"


"$(INTDIR)\dbevents.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"


"$(INTDIR)\dbevents.obj"	"$(INTDIR)\dbevents.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ENDIF 

SOURCE=.\dbheaders.c

!IF  "$(CFG)" == "db3x - Win32 Release"


"$(INTDIR)\dbheaders.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"


"$(INTDIR)\dbheaders.obj"	"$(INTDIR)\dbheaders.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"


"$(INTDIR)\dbheaders.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"


"$(INTDIR)\dbheaders.obj"	"$(INTDIR)\dbheaders.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ENDIF 

SOURCE=.\dbmodulechain.c

!IF  "$(CFG)" == "db3x - Win32 Release"


"$(INTDIR)\dbmodulechain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"


"$(INTDIR)\dbmodulechain.obj"	"$(INTDIR)\dbmodulechain.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"


"$(INTDIR)\dbmodulechain.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"


"$(INTDIR)\dbmodulechain.obj"	"$(INTDIR)\dbmodulechain.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ENDIF 

SOURCE=.\dbsettings.c

!IF  "$(CFG)" == "db3x - Win32 Release"


"$(INTDIR)\dbsettings.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"


"$(INTDIR)\dbsettings.obj"	"$(INTDIR)\dbsettings.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"


"$(INTDIR)\dbsettings.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"


"$(INTDIR)\dbsettings.obj"	"$(INTDIR)\dbsettings.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ENDIF 

SOURCE=.\encrypt.c

!IF  "$(CFG)" == "db3x - Win32 Release"


"$(INTDIR)\encrypt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"


"$(INTDIR)\encrypt.obj"	"$(INTDIR)\encrypt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"


"$(INTDIR)\encrypt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"


"$(INTDIR)\encrypt.obj"	"$(INTDIR)\encrypt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ENDIF 

SOURCE=.\init.c

!IF  "$(CFG)" == "db3x - Win32 Release"


"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug"


"$(INTDIR)\init.obj"	"$(INTDIR)\init.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Release Unicode"


"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ELSEIF  "$(CFG)" == "db3x - Win32 Debug Unicode"


"$(INTDIR)\init.obj"	"$(INTDIR)\init.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\db3x.pch"


!ENDIF 

SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

