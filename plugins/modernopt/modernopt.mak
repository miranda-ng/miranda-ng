# Microsoft Developer Studio Generated NMAKE File, Based on modernopt.dsp
!IF "$(CFG)" == ""
CFG=modernopt - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to modernopt - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "modernopt - Win32 Release" && "$(CFG)" != "modernopt - Win32 Debug" && "$(CFG)" != "modernopt - Win32 Debug Unicode" && "$(CFG)" != "modernopt - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "modernopt.mak" CFG="modernopt - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "modernopt - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "modernopt - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "modernopt - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "modernopt - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "modernopt - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "..\..\bin\release\plugins\modernopt.dll" "$(OUTDIR)\modernopt.bsc"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\commonheaders.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\modernopt.obj"
	-@erase "$(INTDIR)\modernopt.pch"
	-@erase "$(INTDIR)\modernopt.sbr"
	-@erase "$(INTDIR)\mopt_home.obj"
	-@erase "$(INTDIR)\mopt_home.sbr"
	-@erase "$(INTDIR)\mopt_ignore.obj"
	-@erase "$(INTDIR)\mopt_ignore.sbr"
	-@erase "$(INTDIR)\mopt_selector.obj"
	-@erase "$(INTDIR)\mopt_selector.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\modernopt.bsc"
	-@erase "$(OUTDIR)\modernopt.exp"
	-@erase "$(OUTDIR)\modernopt.lib"
	-@erase "$(OUTDIR)\modernopt.pdb"
	-@erase "..\..\bin\release\plugins\modernopt.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernopt_EXPORTS" /D "_UNICODE" /D "UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\modernopt.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\modernopt.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commonheaders.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\modernopt.sbr" \
	"$(INTDIR)\mopt_home.sbr" \
	"$(INTDIR)\mopt_ignore.sbr" \
	"$(INTDIR)\mopt_selector.sbr"

"$(OUTDIR)\modernopt.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ws2_32.lib msvcrt.lib comctl32.lib kernel32.lib user32.lib shell32.lib comdlg32.lib gdi32.lib rpcrt4.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\modernopt.pdb" /debug /machine:I386 /nodefaultlib /out:"../../bin/release/plugins/modernopt.dll" /implib:"$(OUTDIR)\modernopt.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\modernopt.obj" \
	"$(INTDIR)\mopt_home.obj" \
	"$(INTDIR)\mopt_ignore.obj" \
	"$(INTDIR)\mopt_selector.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\plugins\modernopt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "modernopt - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\modernopt.dll" "$(OUTDIR)\modernopt.bsc"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\commonheaders.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\modernopt.obj"
	-@erase "$(INTDIR)\modernopt.pch"
	-@erase "$(INTDIR)\modernopt.sbr"
	-@erase "$(INTDIR)\mopt_home.obj"
	-@erase "$(INTDIR)\mopt_home.sbr"
	-@erase "$(INTDIR)\mopt_ignore.obj"
	-@erase "$(INTDIR)\mopt_ignore.sbr"
	-@erase "$(INTDIR)\mopt_selector.obj"
	-@erase "$(INTDIR)\mopt_selector.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\modernopt.bsc"
	-@erase "$(OUTDIR)\modernopt.dll"
	-@erase "$(OUTDIR)\modernopt.exp"
	-@erase "$(OUTDIR)\modernopt.ilk"
	-@erase "$(OUTDIR)\modernopt.lib"
	-@erase "$(OUTDIR)\modernopt.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernopt_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\modernopt.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\modernopt.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commonheaders.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\modernopt.sbr" \
	"$(INTDIR)\mopt_home.sbr" \
	"$(INTDIR)\mopt_ignore.sbr" \
	"$(INTDIR)\mopt_selector.sbr"

"$(OUTDIR)\modernopt.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=winspool.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib comctl32.lib shell32.lib comdlg32.lib gdi32.lib rpcrt4.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\modernopt.pdb" /debug /machine:I386 /out:"$(OUTDIR)\modernopt.dll" /implib:"$(OUTDIR)\modernopt.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\modernopt.obj" \
	"$(INTDIR)\mopt_home.obj" \
	"$(INTDIR)\mopt_ignore.obj" \
	"$(INTDIR)\mopt_selector.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\modernopt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "modernopt - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "$(OUTDIR)\modernopt.dll" "$(OUTDIR)\modernopt.bsc"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\commonheaders.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\modernopt.obj"
	-@erase "$(INTDIR)\modernopt.pch"
	-@erase "$(INTDIR)\modernopt.sbr"
	-@erase "$(INTDIR)\mopt_home.obj"
	-@erase "$(INTDIR)\mopt_home.sbr"
	-@erase "$(INTDIR)\mopt_ignore.obj"
	-@erase "$(INTDIR)\mopt_ignore.sbr"
	-@erase "$(INTDIR)\mopt_selector.obj"
	-@erase "$(INTDIR)\mopt_selector.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\modernopt.bsc"
	-@erase "$(OUTDIR)\modernopt.dll"
	-@erase "$(OUTDIR)\modernopt.exp"
	-@erase "$(OUTDIR)\modernopt.ilk"
	-@erase "$(OUTDIR)\modernopt.lib"
	-@erase "$(OUTDIR)\modernopt.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernopt_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\modernopt.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\modernopt.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commonheaders.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\modernopt.sbr" \
	"$(INTDIR)\mopt_home.sbr" \
	"$(INTDIR)\mopt_ignore.sbr" \
	"$(INTDIR)\mopt_selector.sbr"

"$(OUTDIR)\modernopt.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=winspool.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib comctl32.lib shell32.lib comdlg32.lib gdi32.lib rpcrt4.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\modernopt.pdb" /debug /machine:I386 /out:"$(OUTDIR)\modernopt.dll" /implib:"$(OUTDIR)\modernopt.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\modernopt.obj" \
	"$(INTDIR)\mopt_home.obj" \
	"$(INTDIR)\mopt_ignore.obj" \
	"$(INTDIR)\mopt_selector.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\modernopt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "modernopt - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode
# Begin Custom Macros
OutDir=.\Release_Unicode
# End Custom Macros

ALL : "..\..\bin\release unicode\plugins\modernopt.dll" "$(OUTDIR)\modernopt.bsc"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\commonheaders.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\modernopt.obj"
	-@erase "$(INTDIR)\modernopt.pch"
	-@erase "$(INTDIR)\modernopt.sbr"
	-@erase "$(INTDIR)\mopt_home.obj"
	-@erase "$(INTDIR)\mopt_home.sbr"
	-@erase "$(INTDIR)\mopt_ignore.obj"
	-@erase "$(INTDIR)\mopt_ignore.sbr"
	-@erase "$(INTDIR)\mopt_selector.obj"
	-@erase "$(INTDIR)\mopt_selector.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\modernopt.bsc"
	-@erase "$(OUTDIR)\modernopt.exp"
	-@erase "$(OUTDIR)\modernopt.lib"
	-@erase "$(OUTDIR)\modernopt.pdb"
	-@erase "..\..\bin\release unicode\plugins\modernopt.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX- /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernopt_EXPORTS" /D "_UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\modernopt.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\modernopt.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commonheaders.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\modernopt.sbr" \
	"$(INTDIR)\mopt_home.sbr" \
	"$(INTDIR)\mopt_ignore.sbr" \
	"$(INTDIR)\mopt_selector.sbr"

"$(OUTDIR)\modernopt.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ws2_32.lib msvcrt.lib comctl32.lib kernel32.lib user32.lib shell32.lib comdlg32.lib gdi32.lib rpcrt4.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\modernopt.pdb" /debug /machine:I386 /nodefaultlib /out:"../../bin/release unicode/plugins/modernopt.dll" /implib:"$(OUTDIR)\modernopt.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\modernopt.obj" \
	"$(INTDIR)\mopt_home.obj" \
	"$(INTDIR)\mopt_ignore.obj" \
	"$(INTDIR)\mopt_selector.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release unicode\plugins\modernopt.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("modernopt.dep")
!INCLUDE "modernopt.dep"
!ELSE 
!MESSAGE Warning: cannot find "modernopt.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "modernopt - Win32 Release" || "$(CFG)" == "modernopt - Win32 Debug" || "$(CFG)" == "modernopt - Win32 Debug Unicode" || "$(CFG)" == "modernopt - Win32 Release Unicode"
SOURCE=.\commonheaders.cpp

!IF  "$(CFG)" == "modernopt - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernopt_EXPORTS" /D "_UNICODE" /D "UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\modernopt.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\commonheaders.sbr"	"$(INTDIR)\modernopt.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernopt - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "modernopt_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\modernopt.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\commonheaders.sbr"	"$(INTDIR)\modernopt.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernopt - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernopt_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\modernopt.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\commonheaders.sbr"	"$(INTDIR)\modernopt.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "modernopt - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX- /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "modernopt_EXPORTS" /D "_UNICODE" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\modernopt.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\commonheaders.sbr"	"$(INTDIR)\modernopt.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\main.cpp

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernopt.pch"


SOURCE=.\modernopt.cpp

"$(INTDIR)\modernopt.obj"	"$(INTDIR)\modernopt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernopt.pch"


SOURCE=.\mopt_home.cpp

"$(INTDIR)\mopt_home.obj"	"$(INTDIR)\mopt_home.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernopt.pch"


SOURCE=.\mopt_ignore.cpp

"$(INTDIR)\mopt_ignore.obj"	"$(INTDIR)\mopt_ignore.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernopt.pch"


SOURCE=.\mopt_selector.cpp

"$(INTDIR)\mopt_selector.obj"	"$(INTDIR)\mopt_selector.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\modernopt.pch"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

