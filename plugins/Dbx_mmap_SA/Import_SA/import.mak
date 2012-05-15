# Microsoft Developer Studio Generated NMAKE File, Based on import.dsp
!IF "$(CFG)" == ""
CFG=import - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to import - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "import - Win32 Release" && "$(CFG)" != "import - Win32 Debug" && "$(CFG)" != "import - Win32 Release Unicode" && "$(CFG)" != "import - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "import.mak" CFG="import - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "import - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "import - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "import - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "import - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "import - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\release\plugins\import.dll"


CLEAN :
	-@erase "$(INTDIR)\ICQserver.obj"
	-@erase "$(INTDIR)\import.pch"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mirabilis.obj"
	-@erase "$(INTDIR)\miranda.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wizard.obj"
	-@erase "$(OUTDIR)\import.exp"
	-@erase "$(OUTDIR)\import.map"
	-@erase "$(OUTDIR)\import.pdb"
	-@erase "..\..\bin\release\plugins\import.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IMPORT_EXPORTS" /Fp"$(INTDIR)\import.pch" /Yu"import.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\import.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x22000000" /dll /incremental:no /pdb:"$(OUTDIR)\import.pdb" /map:"$(INTDIR)\import.map" /debug /machine:I386 /out:"../../bin/release/plugins/import.dll" /implib:"$(OUTDIR)\import.lib" /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\ICQserver.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\mirabilis.obj" \
	"$(INTDIR)\miranda.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\wizard.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\plugins\import.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "import - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\bin\debug\plugins\import.dll" "$(OUTDIR)\import.bsc"


CLEAN :
	-@erase "$(INTDIR)\ICQserver.obj"
	-@erase "$(INTDIR)\ICQserver.sbr"
	-@erase "$(INTDIR)\import.pch"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\mirabilis.obj"
	-@erase "$(INTDIR)\mirabilis.sbr"
	-@erase "$(INTDIR)\miranda.obj"
	-@erase "$(INTDIR)\miranda.sbr"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\progress.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wizard.obj"
	-@erase "$(INTDIR)\wizard.sbr"
	-@erase "$(OUTDIR)\import.bsc"
	-@erase "$(OUTDIR)\import.exp"
	-@erase "$(OUTDIR)\import.lib"
	-@erase "..\..\bin\debug\plugins\import.dll"
	-@erase "..\..\bin\debug\plugins\import.ilk"
	-@erase "..\..\Bin\Debug\Plugins\import.map"
	-@erase "..\..\Bin\Debug\Plugins\import.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IMPORT_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\import.pch" /Yu"import.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\import.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ICQserver.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\mirabilis.sbr" \
	"$(INTDIR)\miranda.sbr" \
	"$(INTDIR)\progress.sbr" \
	"$(INTDIR)\wizard.sbr"

"$(OUTDIR)\import.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x22000000" /dll /incremental:yes /pdb:"../../Bin/Debug/Plugins/import.pdb" /map:"../../Bin/Debug/Plugins/import.map" /debug /machine:I386 /out:"../../bin/debug/plugins/import.dll" /implib:"$(OUTDIR)\import.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\ICQserver.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\mirabilis.obj" \
	"$(INTDIR)\miranda.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\wizard.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug\plugins\import.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "import - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\release unicode\plugins\import.dll"


CLEAN :
	-@erase "$(INTDIR)\ICQserver.obj"
	-@erase "$(INTDIR)\import.pch"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mirabilis.obj"
	-@erase "$(INTDIR)\miranda.obj"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wizard.obj"
	-@erase "$(OUTDIR)\import.exp"
	-@erase "$(OUTDIR)\import.map"
	-@erase "$(OUTDIR)\import.pdb"
	-@erase "..\..\bin\release unicode\plugins\import.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "IMPORT_EXPORTS" /Fp"$(INTDIR)\import.pch" /Yu"import.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\import.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x22000000" /dll /incremental:no /pdb:"$(OUTDIR)\import.pdb" /map:"$(INTDIR)\import.map" /debug /machine:I386 /out:"../../bin/release unicode/plugins/import.dll" /implib:"$(OUTDIR)\import.lib" /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\ICQserver.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\mirabilis.obj" \
	"$(INTDIR)\miranda.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\wizard.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release unicode\plugins\import.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "import - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\bin\debug unicode\plugins\import.dll" "$(OUTDIR)\import.bsc"


CLEAN :
	-@erase "$(INTDIR)\ICQserver.obj"
	-@erase "$(INTDIR)\ICQserver.sbr"
	-@erase "$(INTDIR)\import.pch"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\mirabilis.obj"
	-@erase "$(INTDIR)\mirabilis.sbr"
	-@erase "$(INTDIR)\miranda.obj"
	-@erase "$(INTDIR)\miranda.sbr"
	-@erase "$(INTDIR)\progress.obj"
	-@erase "$(INTDIR)\progress.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wizard.obj"
	-@erase "$(INTDIR)\wizard.sbr"
	-@erase "$(OUTDIR)\import.bsc"
	-@erase "$(OUTDIR)\import.exp"
	-@erase "$(OUTDIR)\import.lib"
	-@erase "..\..\bin\debug unicode\plugins\import.dll"
	-@erase "..\..\bin\debug unicode\plugins\import.ilk"
	-@erase "..\..\Bin\Debug\Plugins\import.map"
	-@erase "..\..\Bin\Debug\Plugins\import.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "IMPORT_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\import.pch" /Yu"import.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\import.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ICQserver.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\mirabilis.sbr" \
	"$(INTDIR)\miranda.sbr" \
	"$(INTDIR)\progress.sbr" \
	"$(INTDIR)\wizard.sbr"

"$(OUTDIR)\import.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x22000000" /dll /incremental:yes /pdb:"../../Bin/Debug/Plugins/import.pdb" /map:"../../Bin/Debug/Plugins/import.map" /debug /machine:I386 /out:"../../bin/debug unicode/plugins/import.dll" /implib:"$(OUTDIR)\import.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\ICQserver.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\mirabilis.obj" \
	"$(INTDIR)\miranda.obj" \
	"$(INTDIR)\progress.obj" \
	"$(INTDIR)\wizard.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug unicode\plugins\import.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("import.dep")
!INCLUDE "import.dep"
!ELSE 
!MESSAGE Warning: cannot find "import.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "import - Win32 Release" || "$(CFG)" == "import - Win32 Debug" || "$(CFG)" == "import - Win32 Release Unicode" || "$(CFG)" == "import - Win32 Debug Unicode"
SOURCE=.\ICQserver.c

!IF  "$(CFG)" == "import - Win32 Release"


"$(INTDIR)\ICQserver.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Debug"


"$(INTDIR)\ICQserver.obj"	"$(INTDIR)\ICQserver.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Release Unicode"


"$(INTDIR)\ICQserver.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Debug Unicode"


"$(INTDIR)\ICQserver.obj"	"$(INTDIR)\ICQserver.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ENDIF 

SOURCE=.\main.c

!IF  "$(CFG)" == "import - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IMPORT_EXPORTS" /Fp"$(INTDIR)\import.pch" /Yc"import.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\import.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "import - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IMPORT_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\import.pch" /Yc"import.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr"	"$(INTDIR)\import.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "import - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "IMPORT_EXPORTS" /Fp"$(INTDIR)\import.pch" /Yc"import.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\import.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "import - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "IMPORT_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\import.pch" /Yc"import.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr"	"$(INTDIR)\import.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\mirabilis.c

!IF  "$(CFG)" == "import - Win32 Release"


"$(INTDIR)\mirabilis.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Debug"


"$(INTDIR)\mirabilis.obj"	"$(INTDIR)\mirabilis.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Release Unicode"


"$(INTDIR)\mirabilis.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Debug Unicode"


"$(INTDIR)\mirabilis.obj"	"$(INTDIR)\mirabilis.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ENDIF 

SOURCE=.\miranda.c

!IF  "$(CFG)" == "import - Win32 Release"


"$(INTDIR)\miranda.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Debug"


"$(INTDIR)\miranda.obj"	"$(INTDIR)\miranda.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Release Unicode"


"$(INTDIR)\miranda.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Debug Unicode"


"$(INTDIR)\miranda.obj"	"$(INTDIR)\miranda.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ENDIF 

SOURCE=.\progress.c

!IF  "$(CFG)" == "import - Win32 Release"


"$(INTDIR)\progress.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Debug"


"$(INTDIR)\progress.obj"	"$(INTDIR)\progress.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Release Unicode"


"$(INTDIR)\progress.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Debug Unicode"


"$(INTDIR)\progress.obj"	"$(INTDIR)\progress.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ENDIF 

SOURCE=.\wizard.c

!IF  "$(CFG)" == "import - Win32 Release"


"$(INTDIR)\wizard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Debug"


"$(INTDIR)\wizard.obj"	"$(INTDIR)\wizard.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Release Unicode"


"$(INTDIR)\wizard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ELSEIF  "$(CFG)" == "import - Win32 Debug Unicode"


"$(INTDIR)\wizard.obj"	"$(INTDIR)\wizard.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\import.pch"


!ENDIF 

SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 


