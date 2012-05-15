# Microsoft Developer Studio Generated NMAKE File, Based on Gadu-Gadu.dsp
!IF "$(CFG)" == ""
CFG=GG - Win32 Release
!MESSAGE No configuration specified. Defaulting to GG - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "GG - Win32 Release" && "$(CFG)" != "GG - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Gadu-Gadu.mak" CFG="GG - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GG - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "GG - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "GG - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "..\..\bin\release\plugins\GG.dll" "$(OUTDIR)\Gadu-Gadu.pch"


CLEAN :
	-@erase "$(INTDIR)\avatar.obj"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\core.obj"
	-@erase "$(INTDIR)\dcc.obj"
	-@erase "$(INTDIR)\dcc7.obj"
	-@erase "$(INTDIR)\dialogs.obj"
	-@erase "$(INTDIR)\dynstuff.obj"
	-@erase "$(INTDIR)\events.obj"
	-@erase "$(INTDIR)\filetransfer.obj"
	-@erase "$(INTDIR)\Gadu-Gadu.pch"
	-@erase "$(INTDIR)\gg.obj"
	-@erase "$(INTDIR)\groupchat.obj"
	-@erase "$(INTDIR)\http.obj"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\image.obj"
	-@erase "$(INTDIR)\import.obj"
	-@erase "$(INTDIR)\keepalive.obj"
	-@erase "$(INTDIR)\libgadu.obj"
	-@erase "$(INTDIR)\links.obj"
	-@erase "$(INTDIR)\oauth.obj"
	-@erase "$(INTDIR)\ownerinfo.obj"
	-@erase "$(INTDIR)\popups.obj"
	-@erase "$(INTDIR)\pthread.obj"
	-@erase "$(INTDIR)\pubdir.obj"
	-@erase "$(INTDIR)\pubdir50.obj"
	-@erase "$(INTDIR)\resolver.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\sessions.obj"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\token.obj"
	-@erase "$(INTDIR)\userutils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(OUTDIR)\GG.exp"
	-@erase "$(OUTDIR)\GG.lib"
	-@erase "$(OUTDIR)\GG.map"
	-@erase "$(OUTDIR)\GG.pdb"
	-@erase "..\..\bin\release\plugins\GG.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\Gadu-Gadu.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Gadu-Gadu.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /incremental:no /pdb:"$(OUTDIR)\GG.pdb" /map:"$(INTDIR)\GG.map" /debug /machine:I386 /out:"../../bin/release/plugins/GG.dll" /implib:"$(OUTDIR)\GG.lib" /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\dcc.obj" \
	"$(INTDIR)\dcc7.obj" \
	"$(INTDIR)\events.obj" \
	"$(INTDIR)\http.obj" \
	"$(INTDIR)\libgadu.obj" \
	"$(INTDIR)\pthread.obj" \
	"$(INTDIR)\pubdir.obj" \
	"$(INTDIR)\pubdir50.obj" \
	"$(INTDIR)\resolver.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\avatar.obj" \
	"$(INTDIR)\core.obj" \
	"$(INTDIR)\dialogs.obj" \
	"$(INTDIR)\dynstuff.obj" \
	"$(INTDIR)\filetransfer.obj" \
	"$(INTDIR)\gg.obj" \
	"$(INTDIR)\groupchat.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\image.obj" \
	"$(INTDIR)\import.obj" \
	"$(INTDIR)\keepalive.obj" \
	"$(INTDIR)\links.obj" \
	"$(INTDIR)\oauth.obj" \
	"$(INTDIR)\ownerinfo.obj" \
	"$(INTDIR)\popups.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\sessions.obj" \
	"$(INTDIR)\token.obj" \
	"$(INTDIR)\userutils.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\plugins\GG.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\bin\debug\plugins\GG.dll" "$(OUTDIR)\Gadu-Gadu.pch"


CLEAN :
	-@erase "$(INTDIR)\avatar.obj"
	-@erase "$(INTDIR)\common.obj"
	-@erase "$(INTDIR)\core.obj"
	-@erase "$(INTDIR)\dcc.obj"
	-@erase "$(INTDIR)\dcc7.obj"
	-@erase "$(INTDIR)\dialogs.obj"
	-@erase "$(INTDIR)\dynstuff.obj"
	-@erase "$(INTDIR)\events.obj"
	-@erase "$(INTDIR)\filetransfer.obj"
	-@erase "$(INTDIR)\Gadu-Gadu.pch"
	-@erase "$(INTDIR)\gg.obj"
	-@erase "$(INTDIR)\groupchat.obj"
	-@erase "$(INTDIR)\http.obj"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\image.obj"
	-@erase "$(INTDIR)\import.obj"
	-@erase "$(INTDIR)\keepalive.obj"
	-@erase "$(INTDIR)\libgadu.obj"
	-@erase "$(INTDIR)\links.obj"
	-@erase "$(INTDIR)\oauth.obj"
	-@erase "$(INTDIR)\ownerinfo.obj"
	-@erase "$(INTDIR)\popups.obj"
	-@erase "$(INTDIR)\pthread.obj"
	-@erase "$(INTDIR)\pubdir.obj"
	-@erase "$(INTDIR)\pubdir50.obj"
	-@erase "$(INTDIR)\resolver.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\sessions.obj"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\token.obj"
	-@erase "$(INTDIR)\userutils.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(OUTDIR)\GG.exp"
	-@erase "$(OUTDIR)\GG.lib"
	-@erase "$(OUTDIR)\GG.map"
	-@erase "$(OUTDIR)\GG.pdb"
	-@erase "..\..\bin\debug\plugins\GG.dll"
	-@erase "..\..\bin\debug\plugins\GG.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\Gadu-Gadu.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Gadu-Gadu.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib version.lib /nologo /base:"0x32500000" /dll /incremental:yes /pdb:"$(OUTDIR)\GG.pdb" /map:"$(INTDIR)\GG.map" /debug /machine:I386 /out:"../../bin/debug/plugins/GG.dll" /implib:"$(OUTDIR)\GG.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\common.obj" \
	"$(INTDIR)\dcc.obj" \
	"$(INTDIR)\dcc7.obj" \
	"$(INTDIR)\events.obj" \
	"$(INTDIR)\http.obj" \
	"$(INTDIR)\libgadu.obj" \
	"$(INTDIR)\pthread.obj" \
	"$(INTDIR)\pubdir.obj" \
	"$(INTDIR)\pubdir50.obj" \
	"$(INTDIR)\resolver.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\avatar.obj" \
	"$(INTDIR)\core.obj" \
	"$(INTDIR)\dialogs.obj" \
	"$(INTDIR)\dynstuff.obj" \
	"$(INTDIR)\filetransfer.obj" \
	"$(INTDIR)\gg.obj" \
	"$(INTDIR)\groupchat.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\image.obj" \
	"$(INTDIR)\import.obj" \
	"$(INTDIR)\keepalive.obj" \
	"$(INTDIR)\links.obj" \
	"$(INTDIR)\oauth.obj" \
	"$(INTDIR)\ownerinfo.obj" \
	"$(INTDIR)\popups.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\sessions.obj" \
	"$(INTDIR)\token.obj" \
	"$(INTDIR)\userutils.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug\plugins\GG.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Gadu-Gadu.dep")
!INCLUDE "Gadu-Gadu.dep"
!ELSE 
!MESSAGE Warning: cannot find "Gadu-Gadu.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "GG - Win32 Release" || "$(CFG)" == "GG - Win32 Debug"
SOURCE=.\libgadu\common.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\common.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\common.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\dcc.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dcc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\dcc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\dcc7.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\dcc7.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\dcc7.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\events.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\events.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\events.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\http.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\http.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\http.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\libgadu.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\libgadu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\libgadu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\pthread.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\pthread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\pthread.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\pubdir.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\pubdir.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\pubdir.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\pubdir50.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\pubdir50.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\pubdir50.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\resolver.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\resolver.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\resolver.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\sha1.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\sha1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\sha1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libgadu\win32.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\avatar.c

"$(INTDIR)\avatar.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\core.c

"$(INTDIR)\core.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dialogs.c

"$(INTDIR)\dialogs.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dynstuff.c

"$(INTDIR)\dynstuff.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\filetransfer.c

"$(INTDIR)\filetransfer.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\gg.c

!IF  "$(CFG)" == "GG - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /I "libgadu" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\Gadu-Gadu.pch" /Yc"gg.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\gg.obj"	"$(INTDIR)\Gadu-Gadu.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "GG - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "libgadu" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "GG_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\Gadu-Gadu.pch" /Yc"gg.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\gg.obj"	"$(INTDIR)\Gadu-Gadu.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\groupchat.c

"$(INTDIR)\groupchat.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\icolib.c

"$(INTDIR)\icolib.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\image.c

"$(INTDIR)\image.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\import.c

"$(INTDIR)\import.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\keepalive.c

"$(INTDIR)\keepalive.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\links.c

"$(INTDIR)\links.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\oauth.c

"$(INTDIR)\oauth.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ownerinfo.c

"$(INTDIR)\ownerinfo.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\popups.c

"$(INTDIR)\popups.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\services.c

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\sessions.c

"$(INTDIR)\sessions.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\token.c

"$(INTDIR)\token.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\userutils.c

"$(INTDIR)\userutils.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

