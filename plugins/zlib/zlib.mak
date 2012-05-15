# Microsoft Developer Studio Generated NMAKE File, Based on zlib.dsp
!IF "$(CFG)" == ""
CFG=zlib - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to zlib - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "zlib - Win32 Release" && "$(CFG)" != "zlib - Win32 Debug" && "$(CFG)" != "zlib - Win32 Release Unicode" && "$(CFG)" != "zlib - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak" CFG="zlib - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zlib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zlib - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zlib - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "zlib - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\Release\zlib.dll"


CLEAN :
	-@erase "$(INTDIR)\adler32.obj"
	-@erase "$(INTDIR)\compress.obj"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\deflate.obj"
	-@erase "$(INTDIR)\gzclose.obj"
	-@erase "$(INTDIR)\gzlib.obj"
	-@erase "$(INTDIR)\gzread.obj"
	-@erase "$(INTDIR)\gzwrite.obj"
	-@erase "$(INTDIR)\infback.obj"
	-@erase "$(INTDIR)\inffast.obj"
	-@erase "$(INTDIR)\inflate.obj"
	-@erase "$(INTDIR)\inftrees.obj"
	-@erase "$(INTDIR)\trees.obj"
	-@erase "$(INTDIR)\uncompr.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\zlib.res"
	-@erase "$(INTDIR)\zutil.obj"
	-@erase "$(OUTDIR)\zlib.exp"
	-@erase "$(OUTDIR)\zlib.lib"
	-@erase "$(OUTDIR)\zlib.map"
	-@erase "$(OUTDIR)\zlib.pdb"
	-@erase "..\..\bin\Release\zlib.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\zlib.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zlib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\zlib.pdb" /map:"$(INTDIR)\zlib.map" /debug /machine:I386 /def:".\zlib.def" /out:"../../bin/Release/zlib.dll" /implib:"$(OUTDIR)\zlib.lib" /filealign:512 
DEF_FILE= \
	".\zlib.def"
LINK32_OBJS= \
	"$(INTDIR)\adler32.obj" \
	"$(INTDIR)\compress.obj" \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\deflate.obj" \
	"$(INTDIR)\gzclose.obj" \
	"$(INTDIR)\gzlib.obj" \
	"$(INTDIR)\gzread.obj" \
	"$(INTDIR)\gzwrite.obj" \
	"$(INTDIR)\infback.obj" \
	"$(INTDIR)\inffast.obj" \
	"$(INTDIR)\inflate.obj" \
	"$(INTDIR)\inftrees.obj" \
	"$(INTDIR)\trees.obj" \
	"$(INTDIR)\uncompr.obj" \
	"$(INTDIR)\zutil.obj" \
	"$(INTDIR)\zlib.res"

"..\..\bin\Release\zlib.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\bin\Debug\zlib.dll" "$(OUTDIR)\zlib.bsc"


CLEAN :
	-@erase "$(INTDIR)\adler32.obj"
	-@erase "$(INTDIR)\adler32.sbr"
	-@erase "$(INTDIR)\compress.obj"
	-@erase "$(INTDIR)\compress.sbr"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\crc32.sbr"
	-@erase "$(INTDIR)\deflate.obj"
	-@erase "$(INTDIR)\deflate.sbr"
	-@erase "$(INTDIR)\gzclose.obj"
	-@erase "$(INTDIR)\gzclose.sbr"
	-@erase "$(INTDIR)\gzlib.obj"
	-@erase "$(INTDIR)\gzlib.sbr"
	-@erase "$(INTDIR)\gzread.obj"
	-@erase "$(INTDIR)\gzread.sbr"
	-@erase "$(INTDIR)\gzwrite.obj"
	-@erase "$(INTDIR)\gzwrite.sbr"
	-@erase "$(INTDIR)\infback.obj"
	-@erase "$(INTDIR)\infback.sbr"
	-@erase "$(INTDIR)\inffast.obj"
	-@erase "$(INTDIR)\inffast.sbr"
	-@erase "$(INTDIR)\inflate.obj"
	-@erase "$(INTDIR)\inflate.sbr"
	-@erase "$(INTDIR)\inftrees.obj"
	-@erase "$(INTDIR)\inftrees.sbr"
	-@erase "$(INTDIR)\trees.obj"
	-@erase "$(INTDIR)\trees.sbr"
	-@erase "$(INTDIR)\uncompr.obj"
	-@erase "$(INTDIR)\uncompr.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\zlib.res"
	-@erase "$(INTDIR)\zutil.obj"
	-@erase "$(INTDIR)\zutil.sbr"
	-@erase "$(OUTDIR)\zlib.bsc"
	-@erase "$(OUTDIR)\zlib.exp"
	-@erase "$(OUTDIR)\zlib.lib"
	-@erase "$(OUTDIR)\zlib.pdb"
	-@erase "..\..\bin\Debug\zlib.dll"
	-@erase "..\..\bin\Debug\zlib.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\zlib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\zlib.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zlib.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\adler32.sbr" \
	"$(INTDIR)\compress.sbr" \
	"$(INTDIR)\crc32.sbr" \
	"$(INTDIR)\deflate.sbr" \
	"$(INTDIR)\gzclose.sbr" \
	"$(INTDIR)\gzlib.sbr" \
	"$(INTDIR)\gzread.sbr" \
	"$(INTDIR)\gzwrite.sbr" \
	"$(INTDIR)\infback.sbr" \
	"$(INTDIR)\inffast.sbr" \
	"$(INTDIR)\inflate.sbr" \
	"$(INTDIR)\inftrees.sbr" \
	"$(INTDIR)\trees.sbr" \
	"$(INTDIR)\uncompr.sbr" \
	"$(INTDIR)\zutil.sbr"

"$(OUTDIR)\zlib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\zlib.pdb" /debug /machine:I386 /def:".\zlib.def" /out:"../../bin/Debug/zlib.dll" /implib:"$(OUTDIR)\zlib.lib" /pdbtype:sept 
DEF_FILE= \
	".\zlib.def"
LINK32_OBJS= \
	"$(INTDIR)\adler32.obj" \
	"$(INTDIR)\compress.obj" \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\deflate.obj" \
	"$(INTDIR)\gzclose.obj" \
	"$(INTDIR)\gzlib.obj" \
	"$(INTDIR)\gzread.obj" \
	"$(INTDIR)\gzwrite.obj" \
	"$(INTDIR)\infback.obj" \
	"$(INTDIR)\inffast.obj" \
	"$(INTDIR)\inflate.obj" \
	"$(INTDIR)\inftrees.obj" \
	"$(INTDIR)\trees.obj" \
	"$(INTDIR)\uncompr.obj" \
	"$(INTDIR)\zutil.obj" \
	"$(INTDIR)\zlib.res"

"..\..\bin\Debug\zlib.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\Release Unicode\zlib.dll"


CLEAN :
	-@erase "$(INTDIR)\adler32.obj"
	-@erase "$(INTDIR)\compress.obj"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\deflate.obj"
	-@erase "$(INTDIR)\gzclose.obj"
	-@erase "$(INTDIR)\gzlib.obj"
	-@erase "$(INTDIR)\gzread.obj"
	-@erase "$(INTDIR)\gzwrite.obj"
	-@erase "$(INTDIR)\infback.obj"
	-@erase "$(INTDIR)\inffast.obj"
	-@erase "$(INTDIR)\inflate.obj"
	-@erase "$(INTDIR)\inftrees.obj"
	-@erase "$(INTDIR)\trees.obj"
	-@erase "$(INTDIR)\uncompr.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\zlib.res"
	-@erase "$(INTDIR)\zutil.obj"
	-@erase "$(OUTDIR)\zlib.exp"
	-@erase "$(OUTDIR)\zlib.lib"
	-@erase "$(OUTDIR)\zlib.map"
	-@erase "$(OUTDIR)\zlib.pdb"
	-@erase "..\..\bin\Release Unicode\zlib.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\zlib.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zlib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\zlib.pdb" /map:"$(INTDIR)\zlib.map" /debug /machine:I386 /def:".\zlib.def" /out:"../../bin/Release Unicode/zlib.dll" /implib:"$(OUTDIR)\zlib.lib" /filealign:512 
DEF_FILE= \
	".\zlib.def"
LINK32_OBJS= \
	"$(INTDIR)\adler32.obj" \
	"$(INTDIR)\compress.obj" \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\deflate.obj" \
	"$(INTDIR)\gzclose.obj" \
	"$(INTDIR)\gzlib.obj" \
	"$(INTDIR)\gzread.obj" \
	"$(INTDIR)\gzwrite.obj" \
	"$(INTDIR)\infback.obj" \
	"$(INTDIR)\inffast.obj" \
	"$(INTDIR)\inflate.obj" \
	"$(INTDIR)\inftrees.obj" \
	"$(INTDIR)\trees.obj" \
	"$(INTDIR)\uncompr.obj" \
	"$(INTDIR)\zutil.obj" \
	"$(INTDIR)\zlib.res"

"..\..\bin\Release Unicode\zlib.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\bin\Debug Unicode\zlib.dll" "$(OUTDIR)\zlib.bsc"


CLEAN :
	-@erase "$(INTDIR)\adler32.obj"
	-@erase "$(INTDIR)\adler32.sbr"
	-@erase "$(INTDIR)\compress.obj"
	-@erase "$(INTDIR)\compress.sbr"
	-@erase "$(INTDIR)\crc32.obj"
	-@erase "$(INTDIR)\crc32.sbr"
	-@erase "$(INTDIR)\deflate.obj"
	-@erase "$(INTDIR)\deflate.sbr"
	-@erase "$(INTDIR)\gzclose.obj"
	-@erase "$(INTDIR)\gzclose.sbr"
	-@erase "$(INTDIR)\gzlib.obj"
	-@erase "$(INTDIR)\gzlib.sbr"
	-@erase "$(INTDIR)\gzread.obj"
	-@erase "$(INTDIR)\gzread.sbr"
	-@erase "$(INTDIR)\gzwrite.obj"
	-@erase "$(INTDIR)\gzwrite.sbr"
	-@erase "$(INTDIR)\infback.obj"
	-@erase "$(INTDIR)\infback.sbr"
	-@erase "$(INTDIR)\inffast.obj"
	-@erase "$(INTDIR)\inffast.sbr"
	-@erase "$(INTDIR)\inflate.obj"
	-@erase "$(INTDIR)\inflate.sbr"
	-@erase "$(INTDIR)\inftrees.obj"
	-@erase "$(INTDIR)\inftrees.sbr"
	-@erase "$(INTDIR)\trees.obj"
	-@erase "$(INTDIR)\trees.sbr"
	-@erase "$(INTDIR)\uncompr.obj"
	-@erase "$(INTDIR)\uncompr.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\zlib.res"
	-@erase "$(INTDIR)\zutil.obj"
	-@erase "$(INTDIR)\zutil.sbr"
	-@erase "$(OUTDIR)\zlib.bsc"
	-@erase "$(OUTDIR)\zlib.exp"
	-@erase "$(OUTDIR)\zlib.lib"
	-@erase "$(OUTDIR)\zlib.pdb"
	-@erase "..\..\bin\Debug Unicode\zlib.dll"
	-@erase "..\..\bin\Debug Unicode\zlib.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\zlib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\zlib.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\zlib.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\adler32.sbr" \
	"$(INTDIR)\compress.sbr" \
	"$(INTDIR)\crc32.sbr" \
	"$(INTDIR)\deflate.sbr" \
	"$(INTDIR)\gzclose.sbr" \
	"$(INTDIR)\gzlib.sbr" \
	"$(INTDIR)\gzread.sbr" \
	"$(INTDIR)\gzwrite.sbr" \
	"$(INTDIR)\infback.sbr" \
	"$(INTDIR)\inffast.sbr" \
	"$(INTDIR)\inflate.sbr" \
	"$(INTDIR)\inftrees.sbr" \
	"$(INTDIR)\trees.sbr" \
	"$(INTDIR)\uncompr.sbr" \
	"$(INTDIR)\zutil.sbr"

"$(OUTDIR)\zlib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\zlib.pdb" /debug /machine:I386 /def:".\zlib.def" /out:"../../bin/Debug Unicode/zlib.dll" /implib:"$(OUTDIR)\zlib.lib" /pdbtype:sept 
DEF_FILE= \
	".\zlib.def"
LINK32_OBJS= \
	"$(INTDIR)\adler32.obj" \
	"$(INTDIR)\compress.obj" \
	"$(INTDIR)\crc32.obj" \
	"$(INTDIR)\deflate.obj" \
	"$(INTDIR)\gzclose.obj" \
	"$(INTDIR)\gzlib.obj" \
	"$(INTDIR)\gzread.obj" \
	"$(INTDIR)\gzwrite.obj" \
	"$(INTDIR)\infback.obj" \
	"$(INTDIR)\inffast.obj" \
	"$(INTDIR)\inflate.obj" \
	"$(INTDIR)\inftrees.obj" \
	"$(INTDIR)\trees.obj" \
	"$(INTDIR)\uncompr.obj" \
	"$(INTDIR)\zutil.obj" \
	"$(INTDIR)\zlib.res"

"..\..\bin\Debug Unicode\zlib.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("zlib.dep")
!INCLUDE "zlib.dep"
!ELSE 
!MESSAGE Warning: cannot find "zlib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "zlib - Win32 Release" || "$(CFG)" == "zlib - Win32 Debug" || "$(CFG)" == "zlib - Win32 Release Unicode" || "$(CFG)" == "zlib - Win32 Debug Unicode"
SOURCE=.\adler32.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\adler32.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\adler32.obj"	"$(INTDIR)\adler32.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\adler32.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\adler32.obj"	"$(INTDIR)\adler32.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\compress.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\compress.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\compress.obj"	"$(INTDIR)\compress.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\compress.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\compress.obj"	"$(INTDIR)\compress.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\crc32.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\crc32.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\crc32.obj"	"$(INTDIR)\crc32.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\crc32.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\crc32.obj"	"$(INTDIR)\crc32.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\deflate.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\deflate.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\deflate.obj"	"$(INTDIR)\deflate.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\deflate.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\deflate.obj"	"$(INTDIR)\deflate.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\gzclose.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\gzclose.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\gzclose.obj"	"$(INTDIR)\gzclose.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\gzclose.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\gzclose.obj"	"$(INTDIR)\gzclose.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\gzlib.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\gzlib.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\gzlib.obj"	"$(INTDIR)\gzlib.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\gzlib.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\gzlib.obj"	"$(INTDIR)\gzlib.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\gzread.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\gzread.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\gzread.obj"	"$(INTDIR)\gzread.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\gzread.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\gzread.obj"	"$(INTDIR)\gzread.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\gzwrite.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\gzwrite.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\gzwrite.obj"	"$(INTDIR)\gzwrite.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\gzwrite.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\gzwrite.obj"	"$(INTDIR)\gzwrite.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\infback.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\infback.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\infback.obj"	"$(INTDIR)\infback.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\infback.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\infback.obj"	"$(INTDIR)\infback.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\inffast.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\inffast.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\inffast.obj"	"$(INTDIR)\inffast.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\inffast.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\inffast.obj"	"$(INTDIR)\inffast.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\inflate.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\inflate.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\inflate.obj"	"$(INTDIR)\inflate.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\inflate.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\inflate.obj"	"$(INTDIR)\inflate.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\inftrees.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\inftrees.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\inftrees.obj"	"$(INTDIR)\inftrees.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\inftrees.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\inftrees.obj"	"$(INTDIR)\inftrees.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\trees.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\trees.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\trees.obj"	"$(INTDIR)\trees.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\trees.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\trees.obj"	"$(INTDIR)\trees.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\uncompr.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\uncompr.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\uncompr.obj"	"$(INTDIR)\uncompr.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\uncompr.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\uncompr.obj"	"$(INTDIR)\uncompr.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\zutil.c

!IF  "$(CFG)" == "zlib - Win32 Release"


"$(INTDIR)\zutil.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"


"$(INTDIR)\zutil.obj"	"$(INTDIR)\zutil.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Release Unicode"


"$(INTDIR)\zutil.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "zlib - Win32 Debug Unicode"


"$(INTDIR)\zutil.obj"	"$(INTDIR)\zutil.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\zlib.rc

"$(INTDIR)\zlib.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

