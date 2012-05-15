# Microsoft Developer Studio Generated NMAKE File, Based on tipper.dsp
!IF "$(CFG)" == ""
CFG=tipper - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to tipper - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "tipper - Win32 Release" && "$(CFG)" != "tipper - Win32 Debug" && "$(CFG)" != "tipper - Win32 Release Unicode" && "$(CFG)" != "tipper - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tipper.mak" CFG="tipper - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tipper - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tipper - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tipper - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tipper - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "tipper - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\Release\Plugins\tipper.dll"


CLEAN :
	-@erase "$(INTDIR)\message_pump.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\popwin.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\str_utils.obj"
	-@erase "$(INTDIR)\subst.obj"
	-@erase "$(INTDIR)\tipper.obj"
	-@erase "$(INTDIR)\tipper.pch"
	-@erase "$(INTDIR)\translations.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\tipper.exp"
	-@erase "$(OUTDIR)\tipper.lib"
	-@erase "$(OUTDIR)\tipper.map"
	-@erase "$(OUTDIR)\tipper.pdb"
	-@erase "..\..\bin\Release\Plugins\tipper.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TIPPER_EXPORTS" /Fp"$(INTDIR)\tipper.pch" /Yu"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tipper.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\tipper.pdb" /map:"$(INTDIR)\tipper.map" /debug /machine:I386 /out:"../../bin/Release/Plugins/tipper.dll" /implib:"$(OUTDIR)\tipper.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\resource.res" \
	"$(INTDIR)\translations.obj" \
	"$(INTDIR)\message_pump.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popwin.obj" \
	"$(INTDIR)\str_utils.obj" \
	"$(INTDIR)\subst.obj" \
	"$(INTDIR)\tipper.obj"

"..\..\bin\Release\Plugins\tipper.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tipper - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\bin\Debug\plugins\tipper.dll" "$(OUTDIR)\tipper.bsc"


CLEAN :
	-@erase "$(INTDIR)\message_pump.obj"
	-@erase "$(INTDIR)\message_pump.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\popwin.obj"
	-@erase "$(INTDIR)\popwin.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\str_utils.obj"
	-@erase "$(INTDIR)\str_utils.sbr"
	-@erase "$(INTDIR)\subst.obj"
	-@erase "$(INTDIR)\subst.sbr"
	-@erase "$(INTDIR)\tipper.obj"
	-@erase "$(INTDIR)\tipper.pch"
	-@erase "$(INTDIR)\tipper.sbr"
	-@erase "$(INTDIR)\translations.obj"
	-@erase "$(INTDIR)\translations.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\tipper.bsc"
	-@erase "$(OUTDIR)\tipper.exp"
	-@erase "$(OUTDIR)\tipper.lib"
	-@erase "$(OUTDIR)\tipper.pdb"
	-@erase "..\..\bin\Debug\plugins\tipper.dll"
	-@erase "..\..\bin\Debug\plugins\tipper.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TIPPER_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\tipper.pch" /Yu"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tipper.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\translations.sbr" \
	"$(INTDIR)\message_pump.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\popwin.sbr" \
	"$(INTDIR)\str_utils.sbr" \
	"$(INTDIR)\subst.sbr" \
	"$(INTDIR)\tipper.sbr"

"$(OUTDIR)\tipper.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\tipper.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/tipper.dll" /implib:"$(OUTDIR)\tipper.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\resource.res" \
	"$(INTDIR)\translations.obj" \
	"$(INTDIR)\message_pump.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popwin.obj" \
	"$(INTDIR)\str_utils.obj" \
	"$(INTDIR)\subst.obj" \
	"$(INTDIR)\tipper.obj"

"..\..\bin\Debug\plugins\tipper.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tipper - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\Release Unicode\Plugins\tipper.dll"


CLEAN :
	-@erase "$(INTDIR)\message_pump.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\popwin.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\str_utils.obj"
	-@erase "$(INTDIR)\subst.obj"
	-@erase "$(INTDIR)\tipper.obj"
	-@erase "$(INTDIR)\tipper.pch"
	-@erase "$(INTDIR)\translations.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\tipper.exp"
	-@erase "$(OUTDIR)\tipper.lib"
	-@erase "$(OUTDIR)\tipper.map"
	-@erase "$(OUTDIR)\tipper.pdb"
	-@erase "..\..\bin\Release Unicode\Plugins\tipper.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "TIPPER_EXPORTS" /Fp"$(INTDIR)\tipper.pch" /Yu"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tipper.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\tipper.pdb" /map:"$(INTDIR)\tipper.map" /debug /machine:I386 /out:"../../bin/Release Unicode/Plugins/tipper.dll" /implib:"$(OUTDIR)\tipper.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\resource.res" \
	"$(INTDIR)\translations.obj" \
	"$(INTDIR)\message_pump.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popwin.obj" \
	"$(INTDIR)\str_utils.obj" \
	"$(INTDIR)\subst.obj" \
	"$(INTDIR)\tipper.obj"

"..\..\bin\Release Unicode\Plugins\tipper.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "tipper - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\bin\Debug Unicode\plugins\tipper.dll" "$(OUTDIR)\tipper.bsc"


CLEAN :
	-@erase "$(INTDIR)\message_pump.obj"
	-@erase "$(INTDIR)\message_pump.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\popwin.obj"
	-@erase "$(INTDIR)\popwin.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\str_utils.obj"
	-@erase "$(INTDIR)\str_utils.sbr"
	-@erase "$(INTDIR)\subst.obj"
	-@erase "$(INTDIR)\subst.sbr"
	-@erase "$(INTDIR)\tipper.obj"
	-@erase "$(INTDIR)\tipper.pch"
	-@erase "$(INTDIR)\tipper.sbr"
	-@erase "$(INTDIR)\translations.obj"
	-@erase "$(INTDIR)\translations.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\tipper.bsc"
	-@erase "$(OUTDIR)\tipper.exp"
	-@erase "$(OUTDIR)\tipper.lib"
	-@erase "$(OUTDIR)\tipper.pdb"
	-@erase "..\..\bin\Debug Unicode\plugins\tipper.dll"
	-@erase "..\..\bin\Debug Unicode\plugins\tipper.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "TIPPER_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\tipper.pch" /Yu"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tipper.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\translations.sbr" \
	"$(INTDIR)\message_pump.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\popwin.sbr" \
	"$(INTDIR)\str_utils.sbr" \
	"$(INTDIR)\subst.sbr" \
	"$(INTDIR)\tipper.sbr"

"$(OUTDIR)\tipper.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\tipper.pdb" /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/tipper.dll" /implib:"$(OUTDIR)\tipper.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\resource.res" \
	"$(INTDIR)\translations.obj" \
	"$(INTDIR)\message_pump.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popwin.obj" \
	"$(INTDIR)\str_utils.obj" \
	"$(INTDIR)\subst.obj" \
	"$(INTDIR)\tipper.obj"

"..\..\bin\Debug Unicode\plugins\tipper.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("tipper.dep")
!INCLUDE "tipper.dep"
!ELSE 
!MESSAGE Warning: cannot find "tipper.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "tipper - Win32 Release" || "$(CFG)" == "tipper - Win32 Debug" || "$(CFG)" == "tipper - Win32 Release Unicode" || "$(CFG)" == "tipper - Win32 Debug Unicode"
SOURCE=.\message_pump.cpp

!IF  "$(CFG)" == "tipper - Win32 Release"


"$(INTDIR)\message_pump.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug"


"$(INTDIR)\message_pump.obj"	"$(INTDIR)\message_pump.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Release Unicode"


"$(INTDIR)\message_pump.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug Unicode"


"$(INTDIR)\message_pump.obj"	"$(INTDIR)\message_pump.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ENDIF 

SOURCE=.\options.cpp

!IF  "$(CFG)" == "tipper - Win32 Release"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Release Unicode"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug Unicode"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ENDIF 

SOURCE=.\popwin.cpp

!IF  "$(CFG)" == "tipper - Win32 Release"


"$(INTDIR)\popwin.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug"


"$(INTDIR)\popwin.obj"	"$(INTDIR)\popwin.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Release Unicode"


"$(INTDIR)\popwin.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug Unicode"


"$(INTDIR)\popwin.obj"	"$(INTDIR)\popwin.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ENDIF 

SOURCE=.\str_utils.cpp

!IF  "$(CFG)" == "tipper - Win32 Release"


"$(INTDIR)\str_utils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug"


"$(INTDIR)\str_utils.obj"	"$(INTDIR)\str_utils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Release Unicode"


"$(INTDIR)\str_utils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug Unicode"


"$(INTDIR)\str_utils.obj"	"$(INTDIR)\str_utils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ENDIF 

SOURCE=.\subst.cpp

!IF  "$(CFG)" == "tipper - Win32 Release"


"$(INTDIR)\subst.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug"


"$(INTDIR)\subst.obj"	"$(INTDIR)\subst.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Release Unicode"


"$(INTDIR)\subst.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug Unicode"


"$(INTDIR)\subst.obj"	"$(INTDIR)\subst.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ENDIF 

SOURCE=.\tipper.cpp

!IF  "$(CFG)" == "tipper - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TIPPER_EXPORTS" /Fp"$(INTDIR)\tipper.pch" /Yc"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\tipper.obj"	"$(INTDIR)\tipper.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TIPPER_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\tipper.pch" /Yc"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\tipper.obj"	"$(INTDIR)\tipper.sbr"	"$(INTDIR)\tipper.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tipper - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "TIPPER_EXPORTS" /Fp"$(INTDIR)\tipper.pch" /Yc"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\tipper.obj"	"$(INTDIR)\tipper.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "TIPPER_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\tipper.pch" /Yc"common.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\tipper.obj"	"$(INTDIR)\tipper.sbr"	"$(INTDIR)\tipper.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\translations.cpp

!IF  "$(CFG)" == "tipper - Win32 Release"


"$(INTDIR)\translations.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug"


"$(INTDIR)\translations.obj"	"$(INTDIR)\translations.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Release Unicode"


"$(INTDIR)\translations.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ELSEIF  "$(CFG)" == "tipper - Win32 Debug Unicode"


"$(INTDIR)\translations.obj"	"$(INTDIR)\translations.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\tipper.pch"


!ENDIF 

SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

