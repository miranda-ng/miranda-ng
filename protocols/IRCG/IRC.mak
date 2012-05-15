# Microsoft Developer Studio Generated NMAKE File, Based on IRC.dsp
!IF "$(CFG)" == ""
CFG=IRC - Win32 Release Unicode
!MESSAGE No configuration specified. Defaulting to IRC - Win32 Release Unicode.
!ENDIF 

!IF "$(CFG)" != "IRC - Win32 Release" && "$(CFG)" != "IRC - Win32 Debug" && "$(CFG)" != "IRC - Win32 Debug Unicode" && "$(CFG)" != "IRC - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IRC.mak" CFG="IRC - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IRC - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IRC - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IRC - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "IRC - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "IRC - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\release\plugins\IRC.dll"


CLEAN :
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\commandmonitor.obj"
	-@erase "$(INTDIR)\input.obj"
	-@erase "$(INTDIR)\IRC.pch"
	-@erase "$(INTDIR)\irclib.obj"
	-@erase "$(INTDIR)\ircproto.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\MString.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\output.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\scripting.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\ui_utils.obj"
	-@erase "$(INTDIR)\userinfo.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\windows.obj"
	-@erase "$(OUTDIR)\IRC.exp"
	-@erase "$(OUTDIR)\IRC.lib"
	-@erase "$(OUTDIR)\IRC.map"
	-@erase "$(OUTDIR)\IRC.pdb"
	-@erase "..\..\bin\release\plugins\IRC.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRC_EXPORTS" /Fp"$(INTDIR)\IRC.pch" /Yu"irc.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\resource.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\IRC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib wsock32.lib /nologo /base:"0x54010000" /dll /incremental:no /pdb:"$(OUTDIR)\IRC.pdb" /map:"$(INTDIR)\IRC.map" /debug /machine:I386 /out:"../../bin/release/plugins/IRC.dll" /implib:"$(OUTDIR)\IRC.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\commandmonitor.obj" \
	"$(INTDIR)\input.obj" \
	"$(INTDIR)\irclib.obj" \
	"$(INTDIR)\ircproto.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\MString.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\output.obj" \
	"$(INTDIR)\scripting.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\ui_utils.obj" \
	"$(INTDIR)\userinfo.obj" \
	"$(INTDIR)\windows.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\plugins\IRC.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\bin\debug\plugins\IRC.dll"


CLEAN :
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\commandmonitor.obj"
	-@erase "$(INTDIR)\input.obj"
	-@erase "$(INTDIR)\IRC.pch"
	-@erase "$(INTDIR)\irclib.obj"
	-@erase "$(INTDIR)\ircproto.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\MString.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\output.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\scripting.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\ui_utils.obj"
	-@erase "$(INTDIR)\userinfo.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\windows.obj"
	-@erase "$(OUTDIR)\IRC.exp"
	-@erase "$(OUTDIR)\IRC.lib"
	-@erase "$(OUTDIR)\IRC.map"
	-@erase "$(OUTDIR)\IRC.pdb"
	-@erase "..\..\bin\debug\plugins\IRC.dll"
	-@erase "..\..\bin\debug\plugins\IRC.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRC_EXPORTS" /Fp"$(INTDIR)\IRC.pch" /Yu"irc.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\resource.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\IRC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib wsock32.lib /nologo /base:"0x54010000" /dll /incremental:yes /pdb:"$(OUTDIR)\IRC.pdb" /map:"$(INTDIR)\IRC.map" /debug /machine:I386 /out:"../../bin/debug/plugins/IRC.dll" /implib:"$(OUTDIR)\IRC.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\commandmonitor.obj" \
	"$(INTDIR)\input.obj" \
	"$(INTDIR)\irclib.obj" \
	"$(INTDIR)\ircproto.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\MString.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\output.obj" \
	"$(INTDIR)\scripting.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\ui_utils.obj" \
	"$(INTDIR)\userinfo.obj" \
	"$(INTDIR)\windows.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug\plugins\IRC.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\bin\debug Unicode\plugins\IRC.dll" "$(OUTDIR)\IRC.bsc"


CLEAN :
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\clist.sbr"
	-@erase "$(INTDIR)\commandmonitor.obj"
	-@erase "$(INTDIR)\commandmonitor.sbr"
	-@erase "$(INTDIR)\input.obj"
	-@erase "$(INTDIR)\input.sbr"
	-@erase "$(INTDIR)\IRC.pch"
	-@erase "$(INTDIR)\irclib.obj"
	-@erase "$(INTDIR)\irclib.sbr"
	-@erase "$(INTDIR)\ircproto.obj"
	-@erase "$(INTDIR)\ircproto.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\MString.obj"
	-@erase "$(INTDIR)\MString.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\output.obj"
	-@erase "$(INTDIR)\output.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\scripting.obj"
	-@erase "$(INTDIR)\scripting.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\tools.sbr"
	-@erase "$(INTDIR)\ui_utils.obj"
	-@erase "$(INTDIR)\ui_utils.sbr"
	-@erase "$(INTDIR)\userinfo.obj"
	-@erase "$(INTDIR)\userinfo.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\windows.obj"
	-@erase "$(INTDIR)\windows.sbr"
	-@erase "$(OUTDIR)\IRC.bsc"
	-@erase "$(OUTDIR)\IRC.exp"
	-@erase "$(OUTDIR)\IRC.lib"
	-@erase "$(OUTDIR)\IRC.map"
	-@erase "$(OUTDIR)\IRC.pdb"
	-@erase "..\..\bin\debug Unicode\plugins\IRC.dll"
	-@erase "..\..\bin\debug Unicode\plugins\IRC.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "IRC_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\IRC.pch" /Yu"irc.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\resource.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\IRC.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\clist.sbr" \
	"$(INTDIR)\commandmonitor.sbr" \
	"$(INTDIR)\input.sbr" \
	"$(INTDIR)\irclib.sbr" \
	"$(INTDIR)\ircproto.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\MString.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\output.sbr" \
	"$(INTDIR)\scripting.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\tools.sbr" \
	"$(INTDIR)\ui_utils.sbr" \
	"$(INTDIR)\userinfo.sbr" \
	"$(INTDIR)\windows.sbr"

"$(OUTDIR)\IRC.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib wsock32.lib /nologo /base:"0x54010000" /dll /incremental:yes /pdb:"$(OUTDIR)\IRC.pdb" /map:"$(INTDIR)\IRC.map" /debug /machine:I386 /out:"../../bin/debug Unicode/plugins/IRC.dll" /implib:"$(OUTDIR)\IRC.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\commandmonitor.obj" \
	"$(INTDIR)\input.obj" \
	"$(INTDIR)\irclib.obj" \
	"$(INTDIR)\ircproto.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\MString.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\output.obj" \
	"$(INTDIR)\scripting.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\ui_utils.obj" \
	"$(INTDIR)\userinfo.obj" \
	"$(INTDIR)\windows.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug Unicode\plugins\IRC.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\release unicode\plugins\IRC.dll"


CLEAN :
	-@erase "$(INTDIR)\clist.obj"
	-@erase "$(INTDIR)\commandmonitor.obj"
	-@erase "$(INTDIR)\input.obj"
	-@erase "$(INTDIR)\IRC.pch"
	-@erase "$(INTDIR)\irclib.obj"
	-@erase "$(INTDIR)\ircproto.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\MString.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\output.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\scripting.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\ui_utils.obj"
	-@erase "$(INTDIR)\userinfo.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\windows.obj"
	-@erase "$(OUTDIR)\IRC.exp"
	-@erase "$(OUTDIR)\IRC.map"
	-@erase "$(OUTDIR)\IRC.pdb"
	-@erase "..\..\bin\release unicode\plugins\IRC.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "IRC_EXPORTS" /Fp"$(INTDIR)\IRC.pch" /Yu"irc.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x41d /fo"$(INTDIR)\resource.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\IRC.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib wsock32.lib /nologo /base:"0x54010000" /dll /incremental:no /pdb:"$(OUTDIR)\IRC.pdb" /map:"$(INTDIR)\IRC.map" /debug /machine:I386 /out:"../../bin/release unicode/plugins/IRC.dll" /implib:"$(OUTDIR)\IRC.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\clist.obj" \
	"$(INTDIR)\commandmonitor.obj" \
	"$(INTDIR)\input.obj" \
	"$(INTDIR)\irclib.obj" \
	"$(INTDIR)\ircproto.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\MString.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\output.obj" \
	"$(INTDIR)\scripting.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\ui_utils.obj" \
	"$(INTDIR)\userinfo.obj" \
	"$(INTDIR)\windows.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release unicode\plugins\IRC.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("IRC.dep")
!INCLUDE "IRC.dep"
!ELSE 
!MESSAGE Warning: cannot find "IRC.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "IRC - Win32 Release" || "$(CFG)" == "IRC - Win32 Debug" || "$(CFG)" == "IRC - Win32 Debug Unicode" || "$(CFG)" == "IRC - Win32 Release Unicode"
SOURCE=.\clist.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\clist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\clist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\clist.obj"	"$(INTDIR)\clist.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\clist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\commandmonitor.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\commandmonitor.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\commandmonitor.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\commandmonitor.obj"	"$(INTDIR)\commandmonitor.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\commandmonitor.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\input.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\input.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\input.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\input.obj"	"$(INTDIR)\input.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\input.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\irclib.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\irclib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\irclib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\irclib.obj"	"$(INTDIR)\irclib.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\irclib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\ircproto.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\ircproto.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\ircproto.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\ircproto.obj"	"$(INTDIR)\ircproto.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\ircproto.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\main.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRC_EXPORTS" /Fp"$(INTDIR)\IRC.pch" /Yc"irc.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\IRC.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IRC_EXPORTS" /Fp"$(INTDIR)\IRC.pch" /Yc"irc.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\IRC.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "IRC_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\IRC.pch" /Yc"irc.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr"	"$(INTDIR)\IRC.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "IRC_EXPORTS" /Fp"$(INTDIR)\IRC.pch" /Yc"irc.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj"	"$(INTDIR)\IRC.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\MString.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\MString.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\MString.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\MString.obj"	"$(INTDIR)\MString.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\MString.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\options.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\output.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\output.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\output.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\output.obj"	"$(INTDIR)\output.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\output.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\scripting.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\scripting.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\scripting.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\scripting.obj"	"$(INTDIR)\scripting.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\scripting.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\services.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\services.obj"	"$(INTDIR)\services.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\tools.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\tools.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\tools.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\tools.obj"	"$(INTDIR)\tools.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\tools.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\ui_utils.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\ui_utils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\ui_utils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\ui_utils.obj"	"$(INTDIR)\ui_utils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\ui_utils.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\userinfo.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\userinfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\userinfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\userinfo.obj"	"$(INTDIR)\userinfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\userinfo.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\windows.cpp

!IF  "$(CFG)" == "IRC - Win32 Release"


"$(INTDIR)\windows.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug"


"$(INTDIR)\windows.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Debug Unicode"


"$(INTDIR)\windows.obj"	"$(INTDIR)\windows.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ELSEIF  "$(CFG)" == "IRC - Win32 Release Unicode"


"$(INTDIR)\windows.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\IRC.pch"


!ENDIF 

SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

