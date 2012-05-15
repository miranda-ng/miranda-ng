# Microsoft Developer Studio Generated NMAKE File, Based on xml.dsp
!IF "$(CFG)" == ""
CFG=xml - Win32 Release Unicode
!MESSAGE No configuration specified. Defaulting to xml - Win32 Release Unicode.
!ENDIF

!IF "$(CFG)" != "xml - Win32 Release" && "$(CFG)" != "xml - Win32 Debug" && "$(CFG)" != "xml - Win32 Debug Unicode" && "$(CFG)" != "xml - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "xml.mak" CFG="xml - Win32 Debug Unicode"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "xml - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "xml - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "xml - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "xml - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

!IF  "$(CFG)" == "xml - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\xml.dll" "$(OUTDIR)\xml.bsc"


CLEAN :
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\xml.obj"
	-@erase "$(INTDIR)\xml.pch"
	-@erase "$(INTDIR)\xml.sbr"
	-@erase "$(INTDIR)\xmlApi.obj"
	-@erase "$(INTDIR)\xmlApi.sbr"
	-@erase "$(INTDIR)\xmlParser.obj"
	-@erase "$(INTDIR)\xmlParser.sbr"
	-@erase "$(OUTDIR)\xml.bsc"
	-@erase "$(OUTDIR)\xml.dll"
	-@erase "$(OUTDIR)\xml.exp"
	-@erase "$(OUTDIR)\xml.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "xml_EXPORTS" /D "_UNICODE" /D "UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\xml.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

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
RSC_PROJ=/l 0x80c /fo"$(INTDIR)\resource.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xml.bsc"
BSC32_SBRS= \
	"$(INTDIR)\xml.sbr" \
	"$(INTDIR)\xmlApi.sbr" \
	"$(INTDIR)\xmlParser.sbr"

"$(OUTDIR)\xml.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=user32.lib advapi32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\xml.pdb" /machine:I386 /out:"$(OUTDIR)\xml.dll" /implib:"$(OUTDIR)\xml.lib" /filealign:512
LINK32_OBJS= \
	"$(INTDIR)\xml.obj" \
	"$(INTDIR)\xmlApi.obj" \
	"$(INTDIR)\xmlParser.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\xml.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xml - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\xml.dll" "$(OUTDIR)\xml.bsc"


CLEAN :
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\xml.obj"
	-@erase "$(INTDIR)\xml.pch"
	-@erase "$(INTDIR)\xml.sbr"
	-@erase "$(INTDIR)\xmlApi.obj"
	-@erase "$(INTDIR)\xmlApi.sbr"
	-@erase "$(INTDIR)\xmlParser.obj"
	-@erase "$(INTDIR)\xmlParser.sbr"
	-@erase "$(OUTDIR)\xml.bsc"
	-@erase "$(OUTDIR)\xml.dll"
	-@erase "$(OUTDIR)\xml.exp"
	-@erase "$(OUTDIR)\xml.ilk"
	-@erase "$(OUTDIR)\xml.lib"
	-@erase "$(OUTDIR)\xml.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "xml_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\xml.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

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
RSC_PROJ=/l 0x80c /fo"$(INTDIR)\resource.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xml.bsc"
BSC32_SBRS= \
	"$(INTDIR)\xml.sbr" \
	"$(INTDIR)\xmlApi.sbr" \
	"$(INTDIR)\xmlParser.sbr"

"$(OUTDIR)\xml.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=winspool.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib shell32.lib comdlg32.lib gdi32.lib rpcrt4.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\xml.pdb" /debug /machine:I386 /out:"$(OUTDIR)\xml.dll" /implib:"$(OUTDIR)\xml.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\xml.obj" \
	"$(INTDIR)\xmlApi.obj" \
	"$(INTDIR)\xmlParser.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\xml.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xml - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\xml.dll" "$(OUTDIR)\xml.bsc"


CLEAN :
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\xml.obj"
	-@erase "$(INTDIR)\xml.pch"
	-@erase "$(INTDIR)\xml.sbr"
	-@erase "$(INTDIR)\xmlApi.obj"
	-@erase "$(INTDIR)\xmlApi.sbr"
	-@erase "$(INTDIR)\xmlParser.obj"
	-@erase "$(INTDIR)\xmlParser.sbr"
	-@erase "$(OUTDIR)\xml.bsc"
	-@erase "$(OUTDIR)\xml.dll"
	-@erase "$(OUTDIR)\xml.exp"
	-@erase "$(OUTDIR)\xml.ilk"
	-@erase "$(OUTDIR)\xml.lib"
	-@erase "$(OUTDIR)\xml.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "xml_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\xml.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

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
RSC_PROJ=/l 0x80c /fo"$(INTDIR)\resource.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xml.bsc"
BSC32_SBRS= \
	"$(INTDIR)\xml.sbr" \
	"$(INTDIR)\xmlApi.sbr" \
	"$(INTDIR)\xmlParser.sbr"

"$(OUTDIR)\xml.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=winspool.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib shell32.lib comdlg32.lib gdi32.lib rpcrt4.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\xml.pdb" /debug /machine:I386 /out:"$(OUTDIR)\xml.dll" /implib:"$(OUTDIR)\xml.lib" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\xml.obj" \
	"$(INTDIR)\xmlApi.obj" \
	"$(INTDIR)\xmlParser.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\xml.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xml - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode
# Begin Custom Macros
OutDir=.\Release_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\xml.dll" "$(OUTDIR)\xml.bsc"


CLEAN :
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\xml.obj"
	-@erase "$(INTDIR)\xml.pch"
	-@erase "$(INTDIR)\xml.sbr"
	-@erase "$(INTDIR)\xmlApi.obj"
	-@erase "$(INTDIR)\xmlApi.sbr"
	-@erase "$(INTDIR)\xmlParser.obj"
	-@erase "$(INTDIR)\xmlParser.sbr"
	-@erase "$(OUTDIR)\xml.bsc"
	-@erase "$(OUTDIR)\xml.dll"
	-@erase "$(OUTDIR)\xml.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "xml_EXPORTS" /D "_UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\xml.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

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
RSC_PROJ=/l 0x80c /fo"$(INTDIR)\resource.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xml.bsc"
BSC32_SBRS= \
	"$(INTDIR)\xml.sbr" \
	"$(INTDIR)\xmlApi.sbr" \
	"$(INTDIR)\xmlParser.sbr"

"$(OUTDIR)\xml.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=advapi32.lib user32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\xml.pdb" /machine:I386 /out:"$(OUTDIR)\xml.dll" /implib:"$(OUTDIR)\xml.lib" /filealign:512
LINK32_OBJS= \
	"$(INTDIR)\xml.obj" \
	"$(INTDIR)\xmlApi.obj" \
	"$(INTDIR)\xmlParser.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\xml.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("xml.dep")
!INCLUDE "xml.dep"
!ELSE
!MESSAGE Warning: cannot find "xml.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "xml - Win32 Release" || "$(CFG)" == "xml - Win32 Debug" || "$(CFG)" == "xml - Win32 Debug Unicode" || "$(CFG)" == "xml - Win32 Release Unicode"
SOURCE=.\xml.cpp

!IF  "$(CFG)" == "xml - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "xml_EXPORTS" /D "_UNICODE" /D "UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\xml.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\xml.obj"	"$(INTDIR)\xml.sbr"	"$(INTDIR)\xml.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "xml - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "xml_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\xml.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\xml.obj"	"$(INTDIR)\xml.sbr"	"$(INTDIR)\xml.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "xml - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "xml_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\xml.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c

"$(INTDIR)\xml.obj"	"$(INTDIR)\xml.sbr"	"$(INTDIR)\xml.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "xml - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "xml_EXPORTS" /D "_UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\xml.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

"$(INTDIR)\xml.obj"	"$(INTDIR)\xml.sbr"	"$(INTDIR)\xml.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF

SOURCE=..\..\src\modules\xml\xmlApi.cpp

"$(INTDIR)\xmlApi.obj"	"$(INTDIR)\xmlApi.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\xml.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\..\src\modules\xml\xmlParser.cpp

"$(INTDIR)\xmlParser.obj"	"$(INTDIR)\xmlParser.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\xml.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF
