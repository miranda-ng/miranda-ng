# Microsoft Developer Studio Generated NMAKE File, Based on mtextcontrol.dsp
!IF "$(CFG)" == ""
CFG=mtextcontrol - Win32 Debug Unicode
!MESSAGE Keine Konfiguration angegeben. mtextcontrol - Win32 Debug Unicode wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "mtextcontrol - Win32 Release" && "$(CFG)" != "mtextcontrol - Win32 Debug" && "$(CFG)" != "mtextcontrol - Win32 Int Release" && "$(CFG)" != "mtextcontrol - Win32 Debug Unicode" && "$(CFG)" != "mtextcontrol - Win32 Release Unicode"
!MESSAGE UngÅltige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie kînnen beim AusfÅhren von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "mtextcontrol.mak" CFG="mtextcontrol - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE FÅr die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "mtextcontrol - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mtextcontrol - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mtextcontrol - Win32 Int Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mtextcontrol - Win32 Debug Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "mtextcontrol - Win32 Release Unicode" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR Eine ungÅltige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release\Obj
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\mtextcontrol.dll"


CLEAN :
	-@erase "$(INTDIR)\dataobject.obj"
	-@erase "$(INTDIR)\enumformat.obj"
	-@erase "$(INTDIR)\fancy_rtf.obj"
	-@erase "$(INTDIR)\FormattedTextDraw.obj"
	-@erase "$(INTDIR)\ImageDataObjectHlp.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mtextcontrol.pch"
	-@erase "$(INTDIR)\richeditutils.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\textcontrol.obj"
	-@erase "$(INTDIR)\textusers.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\version.res"
	-@erase "$(OUTDIR)\mtextcontrol.dll"
	-@erase "$(OUTDIR)\mtextcontrol.exp"
	-@erase "$(OUTDIR)\mtextcontrol.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\version.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mtextcontrol.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib riched20.lib /nologo /base:"0x25a20000" /dll /incremental:no /pdb:"$(OUTDIR)\mtextcontrol.pdb" /debug /machine:I386 /out:"$(OUTDIR)\mtextcontrol.dll" /implib:"$(OUTDIR)\mtextcontrol.lib" 
LINK32_OBJS= \
	"$(INTDIR)\dataobject.obj" \
	"$(INTDIR)\enumformat.obj" \
	"$(INTDIR)\fancy_rtf.obj" \
	"$(INTDIR)\FormattedTextDraw.obj" \
	"$(INTDIR)\ImageDataObjectHlp.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\richeditutils.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\textcontrol.obj" \
	"$(INTDIR)\textusers.obj" \
	"$(INTDIR)\version.res"

"$(OUTDIR)\mtextcontrol.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug\Obj
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\mtextcontrol.dll"


CLEAN :
	-@erase "$(INTDIR)\dataobject.obj"
	-@erase "$(INTDIR)\enumformat.obj"
	-@erase "$(INTDIR)\fancy_rtf.obj"
	-@erase "$(INTDIR)\FormattedTextDraw.obj"
	-@erase "$(INTDIR)\ImageDataObjectHlp.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mtextcontrol.pch"
	-@erase "$(INTDIR)\richeditutils.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\textcontrol.obj"
	-@erase "$(INTDIR)\textusers.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\version.res"
	-@erase "$(OUTDIR)\mtextcontrol.dll"
	-@erase "$(OUTDIR)\mtextcontrol.exp"
	-@erase "$(OUTDIR)\mtextcontrol.ilk"
	-@erase "$(OUTDIR)\mtextcontrol.pdb"
	-@erase "$(OUTDIR)\Obj\mtextcontrol.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\version.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mtextcontrol.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib riched20.lib /nologo /base:"0x25a20000" /dll /incremental:yes /pdb:"$(OUTDIR)\mtextcontrol.pdb" /map:"$(INTDIR)\mtextcontrol.map" /debug /machine:I386 /out:"$(OUTDIR)\mtextcontrol.dll" /implib:"$(OUTDIR)\mtextcontrol.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\dataobject.obj" \
	"$(INTDIR)\enumformat.obj" \
	"$(INTDIR)\fancy_rtf.obj" \
	"$(INTDIR)\FormattedTextDraw.obj" \
	"$(INTDIR)\ImageDataObjectHlp.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\richeditutils.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\textcontrol.obj" \
	"$(INTDIR)\textusers.obj" \
	"$(INTDIR)\version.res"

"$(OUTDIR)\mtextcontrol.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

OUTDIR=.\Int_Release
INTDIR=.\Int_Release\Obj
# Begin Custom Macros
OutDir=.\Int_Release
# End Custom Macros

ALL : "$(OUTDIR)\mtextcontrolW.dll"


CLEAN :
	-@erase "$(INTDIR)\dataobject.obj"
	-@erase "$(INTDIR)\enumformat.obj"
	-@erase "$(INTDIR)\fancy_rtf.obj"
	-@erase "$(INTDIR)\FormattedTextDraw.obj"
	-@erase "$(INTDIR)\ImageDataObjectHlp.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mtextcontrol.pch"
	-@erase "$(INTDIR)\richeditutils.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\textcontrol.obj"
	-@erase "$(INTDIR)\textusers.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\version.res"
	-@erase "$(OUTDIR)\mtextcontrolW.dll"
	-@erase "$(OUTDIR)\mtextcontrolW.exp"
	-@erase "$(OUTDIR)\mtextcontrolW.pdb"
	-@erase "$(OUTDIR)\Obj\mtextcontrolW.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\version.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mtextcontrolW.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib riched20.lib /nologo /base:"0x25a20000" /dll /incremental:no /pdb:"$(OUTDIR)\mtextcontrolW.pdb" /map:"$(INTDIR)\mtextcontrolW.map" /debug /machine:I386 /out:"$(OUTDIR)\mtextcontrolW.dll" /implib:"$(OUTDIR)\mtextcontrolW.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\dataobject.obj" \
	"$(INTDIR)\enumformat.obj" \
	"$(INTDIR)\fancy_rtf.obj" \
	"$(INTDIR)\FormattedTextDraw.obj" \
	"$(INTDIR)\ImageDataObjectHlp.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\richeditutils.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\textcontrol.obj" \
	"$(INTDIR)\textusers.obj" \
	"$(INTDIR)\version.res"

"$(OUTDIR)\mtextcontrolW.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode\Obj
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\mtextcontrolW.dll"


CLEAN :
	-@erase "$(INTDIR)\dataobject.obj"
	-@erase "$(INTDIR)\enumformat.obj"
	-@erase "$(INTDIR)\fancy_rtf.obj"
	-@erase "$(INTDIR)\FormattedTextDraw.obj"
	-@erase "$(INTDIR)\ImageDataObjectHlp.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mtextcontrol.pch"
	-@erase "$(INTDIR)\richeditutils.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\textcontrol.obj"
	-@erase "$(INTDIR)\textusers.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\version.res"
	-@erase "$(OUTDIR)\mtextcontrolW.dll"
	-@erase "$(OUTDIR)\mtextcontrolW.exp"
	-@erase "$(OUTDIR)\mtextcontrolW.ilk"
	-@erase "$(OUTDIR)\mtextcontrolW.pdb"
	-@erase "$(OUTDIR)\Obj\mtextcontrolW.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\version.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mtextcontrolW.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib riched20.lib /nologo /base:"0x25a20000" /dll /incremental:yes /pdb:"$(OUTDIR)\mtextcontrolW.pdb" /map:"$(INTDIR)\mtextcontrolW.map" /debug /machine:I386 /out:"$(OUTDIR)\mtextcontrolW.dll" /implib:"$(OUTDIR)\mtextcontrolW.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\dataobject.obj" \
	"$(INTDIR)\enumformat.obj" \
	"$(INTDIR)\fancy_rtf.obj" \
	"$(INTDIR)\FormattedTextDraw.obj" \
	"$(INTDIR)\ImageDataObjectHlp.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\richeditutils.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\textcontrol.obj" \
	"$(INTDIR)\textusers.obj" \
	"$(INTDIR)\version.res"

"$(OUTDIR)\mtextcontrolW.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode\Obj
# Begin Custom Macros
OutDir=.\Release_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\mtextcontrolW.dll"


CLEAN :
	-@erase "$(INTDIR)\dataobject.obj"
	-@erase "$(INTDIR)\enumformat.obj"
	-@erase "$(INTDIR)\fancy_rtf.obj"
	-@erase "$(INTDIR)\FormattedTextDraw.obj"
	-@erase "$(INTDIR)\ImageDataObjectHlp.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mtextcontrol.pch"
	-@erase "$(INTDIR)\richeditutils.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\textcontrol.obj"
	-@erase "$(INTDIR)\textusers.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\version.res"
	-@erase "$(OUTDIR)\mtextcontrolW.dll"
	-@erase "$(OUTDIR)\mtextcontrolW.exp"
	-@erase "$(OUTDIR)\mtextcontrolW.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\version.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\mtextcontrolW.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib riched20.lib /nologo /base:"0x25a20000" /dll /incremental:no /pdb:"$(OUTDIR)\mtextcontrolW.pdb" /debug /machine:I386 /out:"$(OUTDIR)\mtextcontrolW.dll" /implib:"$(OUTDIR)\mtextcontrolW.lib" 
LINK32_OBJS= \
	"$(INTDIR)\dataobject.obj" \
	"$(INTDIR)\enumformat.obj" \
	"$(INTDIR)\fancy_rtf.obj" \
	"$(INTDIR)\FormattedTextDraw.obj" \
	"$(INTDIR)\ImageDataObjectHlp.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\richeditutils.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\textcontrol.obj" \
	"$(INTDIR)\textusers.obj" \
	"$(INTDIR)\version.res"

"$(OUTDIR)\mtextcontrolW.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("mtextcontrol.dep")
!INCLUDE "mtextcontrol.dep"
!ELSE 
!MESSAGE Warning: cannot find "mtextcontrol.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "mtextcontrol - Win32 Release" || "$(CFG)" == "mtextcontrol - Win32 Debug" || "$(CFG)" == "mtextcontrol - Win32 Int Release" || "$(CFG)" == "mtextcontrol - Win32 Debug Unicode" || "$(CFG)" == "mtextcontrol - Win32 Release Unicode"
SOURCE=.\src\dataobject.cpp

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dataobject.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\dataobject.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

CPP_SWITCHES=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dataobject.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\dataobject.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dataobject.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\enumformat.cpp

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\enumformat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\enumformat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

CPP_SWITCHES=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\enumformat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\enumformat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\enumformat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\fancy_rtf.cpp

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\fancy_rtf.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\fancy_rtf.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

CPP_SWITCHES=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\fancy_rtf.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\fancy_rtf.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\fancy_rtf.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\FormattedTextDraw.cpp

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\FormattedTextDraw.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\FormattedTextDraw.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

CPP_SWITCHES=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\FormattedTextDraw.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\FormattedTextDraw.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\FormattedTextDraw.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\ImageDataObjectHlp.cpp

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\ImageDataObjectHlp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\ImageDataObjectHlp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

CPP_SWITCHES=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\ImageDataObjectHlp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\ImageDataObjectHlp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\ImageDataObjectHlp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\main.cpp

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yc"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\mtextcontrol.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yc"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\mtextcontrol.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

CPP_SWITCHES=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yc"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\mtextcontrol.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yc"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\mtextcontrol.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yc"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\mtextcontrol.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\richeditutils.cpp

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\richeditutils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\richeditutils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

CPP_SWITCHES=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\richeditutils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\richeditutils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\richeditutils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\services.cpp

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

CPP_SWITCHES=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\textcontrol.cpp

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\textcontrol.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\textcontrol.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

CPP_SWITCHES=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\textcontrol.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\textcontrol.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\textcontrol.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\src\textusers.cpp

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\textusers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\textusers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"

CPP_SWITCHES=/nologo /MTd /W3 /ZI /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\textusers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /ZI /Od /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\textusers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O2 /I "../../include" /I "../../include_API" /I "./api" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MTEXTCONTROL_EXPORTS" /Fp"$(INTDIR)\mtextcontrol.pch" /Yu"headers.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\textusers.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\mtextcontrol.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\res\version.rc

!IF  "$(CFG)" == "mtextcontrol - Win32 Release"


"$(INTDIR)\version.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\version.res" /i "res" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug"


"$(INTDIR)\version.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\version.res" /i "res" /d "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Int Release"


"$(INTDIR)\version.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\version.res" /i "res" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Debug Unicode"


"$(INTDIR)\version.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\version.res" /i "res" /d "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "mtextcontrol - Win32 Release Unicode"


"$(INTDIR)\version.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x409 /fo"$(INTDIR)\version.res" /i "res" /d "NDEBUG" $(SOURCE)


!ENDIF 


!ENDIF 

