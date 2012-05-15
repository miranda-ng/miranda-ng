# Microsoft Developer Studio Generated NMAKE File, Based on Yahoo.dsp
!IF "$(CFG)" == ""
CFG=Yahoo - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to Yahoo - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "Yahoo - Win32 Release" && "$(CFG)" != "Yahoo - Win32 Debug" && "$(CFG)" != "Yahoo - Win32 Debug Unicode" && "$(CFG)" != "Yahoo - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Yahoo.mak" CFG="Yahoo - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Yahoo - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Yahoo - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Yahoo - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Yahoo - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "Yahoo - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\Bin\Release\Plugins\Yahoo.dll"


CLEAN :
	-@erase "$(INTDIR)\avatar.obj"
	-@erase "$(INTDIR)\chat.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\file_transfer.obj"
	-@erase "$(INTDIR)\http_gateway.obj"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\ignore.obj"
	-@erase "$(INTDIR)\im.obj"
	-@erase "$(INTDIR)\libyahoo2.obj"
	-@erase "$(INTDIR)\links.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\proto.obj"
	-@erase "$(INTDIR)\search.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\user_info.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\webcam.obj"
	-@erase "$(INTDIR)\yahoo.obj"
	-@erase "$(INTDIR)\Yahoo.pch"
	-@erase "$(INTDIR)\Yahoo.res"
	-@erase "$(INTDIR)\yahoo_httplib.obj"
	-@erase "$(INTDIR)\yahoo_list.obj"
	-@erase "$(INTDIR)\yahoo_util.obj"
	-@erase "$(OUTDIR)\Yahoo.exp"
	-@erase "$(OUTDIR)\Yahoo.lib"
	-@erase "$(OUTDIR)\Yahoo.map"
	-@erase "$(OUTDIR)\Yahoo.pdb"
	-@erase "..\..\Bin\Release\Plugins\Yahoo.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yu"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\Yahoo.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Yahoo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\Yahoo.pdb" /map:"$(INTDIR)\Yahoo.map" /debug /machine:I386 /out:"../../Bin/Release/Plugins/Yahoo.dll" /implib:"$(OUTDIR)\Yahoo.lib" 
LINK32_OBJS= \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\libyahoo2.obj" \
	"$(INTDIR)\yahoo_httplib.obj" \
	"$(INTDIR)\yahoo_list.obj" \
	"$(INTDIR)\yahoo_util.obj" \
	"$(INTDIR)\avatar.obj" \
	"$(INTDIR)\chat.obj" \
	"$(INTDIR)\file_transfer.obj" \
	"$(INTDIR)\http_gateway.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\ignore.obj" \
	"$(INTDIR)\im.obj" \
	"$(INTDIR)\links.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\proto.obj" \
	"$(INTDIR)\search.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\user_info.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\webcam.obj" \
	"$(INTDIR)\yahoo.obj" \
	"$(INTDIR)\Yahoo.res"

"..\..\Bin\Release\Plugins\Yahoo.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "..\..\Bin\Debug\Plugins\Yahoo.dll"


CLEAN :
	-@erase "$(INTDIR)\avatar.obj"
	-@erase "$(INTDIR)\chat.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\file_transfer.obj"
	-@erase "$(INTDIR)\http_gateway.obj"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\ignore.obj"
	-@erase "$(INTDIR)\im.obj"
	-@erase "$(INTDIR)\libyahoo2.obj"
	-@erase "$(INTDIR)\links.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\proto.obj"
	-@erase "$(INTDIR)\search.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\user_info.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\webcam.obj"
	-@erase "$(INTDIR)\yahoo.obj"
	-@erase "$(INTDIR)\Yahoo.pch"
	-@erase "$(INTDIR)\Yahoo.res"
	-@erase "$(INTDIR)\yahoo_httplib.obj"
	-@erase "$(INTDIR)\yahoo_list.obj"
	-@erase "$(INTDIR)\yahoo_util.obj"
	-@erase "$(OUTDIR)\Yahoo.exp"
	-@erase "$(OUTDIR)\Yahoo.lib"
	-@erase "$(OUTDIR)\Yahoo.pdb"
	-@erase "..\..\Bin\Debug\Plugins\Yahoo.dll"
	-@erase "..\..\Bin\Debug\Plugins\Yahoo.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yu"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\Yahoo.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Yahoo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\Yahoo.pdb" /debug /machine:I386 /out:"../../Bin/Debug/Plugins/Yahoo.dll" /implib:"$(OUTDIR)\Yahoo.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\libyahoo2.obj" \
	"$(INTDIR)\yahoo_httplib.obj" \
	"$(INTDIR)\yahoo_list.obj" \
	"$(INTDIR)\yahoo_util.obj" \
	"$(INTDIR)\avatar.obj" \
	"$(INTDIR)\chat.obj" \
	"$(INTDIR)\file_transfer.obj" \
	"$(INTDIR)\http_gateway.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\ignore.obj" \
	"$(INTDIR)\im.obj" \
	"$(INTDIR)\links.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\proto.obj" \
	"$(INTDIR)\search.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\user_info.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\webcam.obj" \
	"$(INTDIR)\yahoo.obj" \
	"$(INTDIR)\Yahoo.res"

"..\..\Bin\Debug\Plugins\Yahoo.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode

ALL : "..\..\Bin\Debug Unicode\Plugins\Yahoo.dll"


CLEAN :
	-@erase "$(INTDIR)\avatar.obj"
	-@erase "$(INTDIR)\chat.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\file_transfer.obj"
	-@erase "$(INTDIR)\http_gateway.obj"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\ignore.obj"
	-@erase "$(INTDIR)\im.obj"
	-@erase "$(INTDIR)\libyahoo2.obj"
	-@erase "$(INTDIR)\links.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\proto.obj"
	-@erase "$(INTDIR)\search.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\user_info.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\webcam.obj"
	-@erase "$(INTDIR)\yahoo.obj"
	-@erase "$(INTDIR)\Yahoo.pch"
	-@erase "$(INTDIR)\Yahoo.res"
	-@erase "$(INTDIR)\yahoo_httplib.obj"
	-@erase "$(INTDIR)\yahoo_list.obj"
	-@erase "$(INTDIR)\yahoo_util.obj"
	-@erase "$(OUTDIR)\Yahoo.exp"
	-@erase "$(OUTDIR)\Yahoo.lib"
	-@erase "$(OUTDIR)\Yahoo.pdb"
	-@erase "..\..\Bin\Debug Unicode\Plugins\Yahoo.dll"
	-@erase "..\..\Bin\Debug Unicode\Plugins\Yahoo.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yu"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\Yahoo.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Yahoo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\Yahoo.pdb" /debug /machine:I386 /out:"../../Bin/Debug Unicode/Plugins/Yahoo.dll" /implib:"$(OUTDIR)\Yahoo.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\libyahoo2.obj" \
	"$(INTDIR)\yahoo_httplib.obj" \
	"$(INTDIR)\yahoo_list.obj" \
	"$(INTDIR)\yahoo_util.obj" \
	"$(INTDIR)\avatar.obj" \
	"$(INTDIR)\chat.obj" \
	"$(INTDIR)\file_transfer.obj" \
	"$(INTDIR)\http_gateway.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\ignore.obj" \
	"$(INTDIR)\im.obj" \
	"$(INTDIR)\links.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\proto.obj" \
	"$(INTDIR)\search.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\user_info.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\webcam.obj" \
	"$(INTDIR)\yahoo.obj" \
	"$(INTDIR)\Yahoo.res"

"..\..\Bin\Debug Unicode\Plugins\Yahoo.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Yahoo - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\Bin\Release Unicode\Plugins\Yahoo.dll"


CLEAN :
	-@erase "$(INTDIR)\avatar.obj"
	-@erase "$(INTDIR)\chat.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\file_transfer.obj"
	-@erase "$(INTDIR)\http_gateway.obj"
	-@erase "$(INTDIR)\icolib.obj"
	-@erase "$(INTDIR)\ignore.obj"
	-@erase "$(INTDIR)\im.obj"
	-@erase "$(INTDIR)\libyahoo2.obj"
	-@erase "$(INTDIR)\links.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\proto.obj"
	-@erase "$(INTDIR)\search.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\user_info.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\webcam.obj"
	-@erase "$(INTDIR)\yahoo.obj"
	-@erase "$(INTDIR)\Yahoo.pch"
	-@erase "$(INTDIR)\Yahoo.res"
	-@erase "$(INTDIR)\yahoo_httplib.obj"
	-@erase "$(INTDIR)\yahoo_list.obj"
	-@erase "$(INTDIR)\yahoo_util.obj"
	-@erase "$(OUTDIR)\Yahoo.exp"
	-@erase "$(OUTDIR)\Yahoo.lib"
	-@erase "$(OUTDIR)\Yahoo.map"
	-@erase "$(OUTDIR)\Yahoo.pdb"
	-@erase "..\..\Bin\Release Unicode\Plugins\Yahoo.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yu"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\Yahoo.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Yahoo.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib wsock32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\Yahoo.pdb" /map:"$(INTDIR)\Yahoo.map" /debug /machine:I386 /out:"../../Bin/Release Unicode/Plugins/Yahoo.dll" /implib:"$(OUTDIR)\Yahoo.lib" 
LINK32_OBJS= \
	"$(INTDIR)\crypt.obj" \
	"$(INTDIR)\libyahoo2.obj" \
	"$(INTDIR)\yahoo_httplib.obj" \
	"$(INTDIR)\yahoo_list.obj" \
	"$(INTDIR)\yahoo_util.obj" \
	"$(INTDIR)\avatar.obj" \
	"$(INTDIR)\chat.obj" \
	"$(INTDIR)\file_transfer.obj" \
	"$(INTDIR)\http_gateway.obj" \
	"$(INTDIR)\icolib.obj" \
	"$(INTDIR)\ignore.obj" \
	"$(INTDIR)\im.obj" \
	"$(INTDIR)\links.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\proto.obj" \
	"$(INTDIR)\search.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\user_info.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\webcam.obj" \
	"$(INTDIR)\yahoo.obj" \
	"$(INTDIR)\Yahoo.res"

"..\..\Bin\Release Unicode\Plugins\Yahoo.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Yahoo.dep")
!INCLUDE "Yahoo.dep"
!ELSE 
!MESSAGE Warning: cannot find "Yahoo.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Yahoo - Win32 Release" || "$(CFG)" == "Yahoo - Win32 Debug" || "$(CFG)" == "Yahoo - Win32 Debug Unicode" || "$(CFG)" == "Yahoo - Win32 Release Unicode"
SOURCE=.\libyahoo2\crypt.c

!IF  "$(CFG)" == "Yahoo - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\crypt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libyahoo2\libyahoo2.c

!IF  "$(CFG)" == "Yahoo - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\libyahoo2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\libyahoo2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\libyahoo2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\libyahoo2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libyahoo2\yahoo_httplib.c

!IF  "$(CFG)" == "Yahoo - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\yahoo_httplib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\yahoo_httplib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\yahoo_httplib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\yahoo_httplib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libyahoo2\yahoo_list.c

!IF  "$(CFG)" == "Yahoo - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\yahoo_list.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\yahoo_list.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\yahoo_list.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\yahoo_list.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\libyahoo2\yahoo_util.c

!IF  "$(CFG)" == "Yahoo - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\yahoo_util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\yahoo_util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\yahoo_util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\yahoo_util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\avatar.cpp

"$(INTDIR)\avatar.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\chat.cpp

"$(INTDIR)\chat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\file_transfer.cpp

"$(INTDIR)\file_transfer.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\http_gateway.cpp

"$(INTDIR)\http_gateway.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\icolib.cpp

"$(INTDIR)\icolib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\ignore.cpp

"$(INTDIR)\ignore.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\im.cpp

"$(INTDIR)\im.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\links.cpp

"$(INTDIR)\links.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\main.cpp

!IF  "$(CFG)" == "Yahoo - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yu"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yu"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yu"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yu"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\options.cpp

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\proto.cpp

"$(INTDIR)\proto.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\search.cpp

"$(INTDIR)\search.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\server.cpp

"$(INTDIR)\server.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\services.cpp

"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\user_info.cpp

"$(INTDIR)\user_info.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\util.cpp

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\webcam.cpp

"$(INTDIR)\webcam.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\Yahoo.pch"


SOURCE=.\yahoo.cpp

!IF  "$(CFG)" == "Yahoo - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yc"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\yahoo.obj"	"$(INTDIR)\Yahoo.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yc"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\yahoo.obj"	"$(INTDIR)\Yahoo.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../../include" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yc"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\yahoo.obj"	"$(INTDIR)\Yahoo.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "Yahoo - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "YAHOO_EXPORTS" /D "HAVE_CONFIG_H" /Fp"$(INTDIR)\Yahoo.pch" /Yc"yahoo.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\yahoo.obj"	"$(INTDIR)\Yahoo.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\Yahoo.rc

"$(INTDIR)\Yahoo.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

