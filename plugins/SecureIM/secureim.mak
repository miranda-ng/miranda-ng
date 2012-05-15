# Microsoft Developer Studio Generated NMAKE File, Based on secureim.dsp
!IF "$(CFG)" == ""
CFG=secureim - Win32 Release
!MESSAGE No configuration specified. Defaulting to secureim - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "secureim - Win32 Release" && "$(CFG)" != "secureim - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "secureim.mak" CFG="secureim - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "secureim - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "secureim - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "secureim - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\secureim.dll"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\crypt_check.obj"
	-@erase "$(INTDIR)\crypt_dll.obj"
	-@erase "$(INTDIR)\crypt_icons.obj"
	-@erase "$(INTDIR)\crypt_lists.obj"
	-@erase "$(INTDIR)\crypt_metacontacts.obj"
	-@erase "$(INTDIR)\crypt_misc.obj"
	-@erase "$(INTDIR)\crypt_popups.obj"
	-@erase "$(INTDIR)\dbevent.obj"
	-@erase "$(INTDIR)\gettime.obj"
	-@erase "$(INTDIR)\images.obj"
	-@erase "$(INTDIR)\language.obj"
	-@erase "$(INTDIR)\loadicons.obj"
	-@erase "$(INTDIR)\loadlib.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mmi.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\popupOptions.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\rtfconv.obj"
	-@erase "$(INTDIR)\secureim.pch"
	-@erase "$(INTDIR)\splitmsg.obj"
	-@erase "$(INTDIR)\svcs_clist.obj"
	-@erase "$(INTDIR)\svcs_menu.obj"
	-@erase "$(INTDIR)\svcs_proto.obj"
	-@erase "$(INTDIR)\svcs_rsa.obj"
	-@erase "$(INTDIR)\svcs_srmm.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\secureim.dll"
	-@erase "$(OUTDIR)\secureim.exp"
	-@erase "$(OUTDIR)\secureim.lib"
	-@erase "$(OUTDIR)\secureim.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O1 /I "\source\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECUREIM_EXPORTS" /Fp"$(INTDIR)\secureim.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\secureim.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib msimg32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib /nologo /base:"0x45000000" /dll /incremental:no /pdb:"$(OUTDIR)\secureim.pdb" /map:"$(INTDIR)\secureim.map" /machine:I386 /out:"$(OUTDIR)\secureim.dll" /implib:"$(OUTDIR)\secureim.lib" 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\crypt_check.obj" \
	"$(INTDIR)\crypt_dll.obj" \
	"$(INTDIR)\crypt_icons.obj" \
	"$(INTDIR)\crypt_lists.obj" \
	"$(INTDIR)\crypt_metacontacts.obj" \
	"$(INTDIR)\crypt_misc.obj" \
	"$(INTDIR)\crypt_popups.obj" \
	"$(INTDIR)\dbevent.obj" \
	"$(INTDIR)\gettime.obj" \
	"$(INTDIR)\images.obj" \
	"$(INTDIR)\language.obj" \
	"$(INTDIR)\loadicons.obj" \
	"$(INTDIR)\loadlib.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\mmi.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popupOptions.obj" \
	"$(INTDIR)\rtfconv.obj" \
	"$(INTDIR)\splitmsg.obj" \
	"$(INTDIR)\svcs_clist.obj" \
	"$(INTDIR)\svcs_menu.obj" \
	"$(INTDIR)\svcs_proto.obj" \
	"$(INTDIR)\svcs_rsa.obj" \
	"$(INTDIR)\svcs_srmm.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\secureim.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "secureim - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\secureim.dll"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\crypt_check.obj"
	-@erase "$(INTDIR)\crypt_dll.obj"
	-@erase "$(INTDIR)\crypt_icons.obj"
	-@erase "$(INTDIR)\crypt_lists.obj"
	-@erase "$(INTDIR)\crypt_metacontacts.obj"
	-@erase "$(INTDIR)\crypt_misc.obj"
	-@erase "$(INTDIR)\crypt_popups.obj"
	-@erase "$(INTDIR)\dbevent.obj"
	-@erase "$(INTDIR)\gettime.obj"
	-@erase "$(INTDIR)\images.obj"
	-@erase "$(INTDIR)\language.obj"
	-@erase "$(INTDIR)\loadicons.obj"
	-@erase "$(INTDIR)\loadlib.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mmi.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\popupOptions.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\rtfconv.obj"
	-@erase "$(INTDIR)\secureim.pch"
	-@erase "$(INTDIR)\splitmsg.obj"
	-@erase "$(INTDIR)\svcs_clist.obj"
	-@erase "$(INTDIR)\svcs_menu.obj"
	-@erase "$(INTDIR)\svcs_proto.obj"
	-@erase "$(INTDIR)\svcs_rsa.obj"
	-@erase "$(INTDIR)\svcs_srmm.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\secureim.dll"
	-@erase "$(OUTDIR)\secureim.exp"
	-@erase "$(OUTDIR)\secureim.lib"
	-@erase "$(OUTDIR)\secureim.map"
	-@erase "$(OUTDIR)\secureim.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "\source\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECUREIM_EXPORTS" /Fp"$(INTDIR)\secureim.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
RSC_PROJ=/l 0x419 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\secureim.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib msimg32.lib /nologo /base:"0x45000000" /dll /incremental:no /pdb:"$(OUTDIR)\secureim.pdb" /map:"$(INTDIR)\secureim.map" /debug /machine:I386 /out:"$(OUTDIR)\secureim.dll" /implib:"$(OUTDIR)\secureim.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\crypt_check.obj" \
	"$(INTDIR)\crypt_dll.obj" \
	"$(INTDIR)\crypt_icons.obj" \
	"$(INTDIR)\crypt_lists.obj" \
	"$(INTDIR)\crypt_metacontacts.obj" \
	"$(INTDIR)\crypt_misc.obj" \
	"$(INTDIR)\crypt_popups.obj" \
	"$(INTDIR)\dbevent.obj" \
	"$(INTDIR)\gettime.obj" \
	"$(INTDIR)\images.obj" \
	"$(INTDIR)\language.obj" \
	"$(INTDIR)\loadicons.obj" \
	"$(INTDIR)\loadlib.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\mmi.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\popupOptions.obj" \
	"$(INTDIR)\rtfconv.obj" \
	"$(INTDIR)\splitmsg.obj" \
	"$(INTDIR)\svcs_clist.obj" \
	"$(INTDIR)\svcs_menu.obj" \
	"$(INTDIR)\svcs_proto.obj" \
	"$(INTDIR)\svcs_rsa.obj" \
	"$(INTDIR)\svcs_srmm.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\secureim.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("secureim.dep")
!INCLUDE "secureim.dep"
!ELSE 
!MESSAGE Warning: cannot find "secureim.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "secureim - Win32 Release" || "$(CFG)" == "secureim - Win32 Debug"
SOURCE=.\commonheaders.cpp

!IF  "$(CFG)" == "secureim - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O1 /I "\source\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECUREIM_EXPORTS" /Fp"$(INTDIR)\secureim.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\secureim.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "secureim - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /Zi /Od /I "\source\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SECUREIM_EXPORTS" /Fp"$(INTDIR)\secureim.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\secureim.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\crypt_check.cpp

"$(INTDIR)\crypt_check.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\crypt_dll.cpp

"$(INTDIR)\crypt_dll.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\crypt_icons.cpp

"$(INTDIR)\crypt_icons.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\crypt_lists.cpp

"$(INTDIR)\crypt_lists.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\crypt_metacontacts.cpp

"$(INTDIR)\crypt_metacontacts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\crypt_misc.cpp

"$(INTDIR)\crypt_misc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\crypt_popups.cpp

"$(INTDIR)\crypt_popups.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\dbevent.cpp

"$(INTDIR)\dbevent.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\gettime.cpp

"$(INTDIR)\gettime.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\images.cpp

"$(INTDIR)\images.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\language.cpp

"$(INTDIR)\language.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\loadicons.cpp

"$(INTDIR)\loadicons.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\loadlib.cpp

"$(INTDIR)\loadlib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\mmi.cpp

"$(INTDIR)\mmi.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\options.cpp

"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\popupOptions.cpp

"$(INTDIR)\popupOptions.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\rtfconv.cpp

"$(INTDIR)\rtfconv.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\splitmsg.cpp

"$(INTDIR)\splitmsg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\svcs_clist.cpp

"$(INTDIR)\svcs_clist.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\svcs_menu.cpp

"$(INTDIR)\svcs_menu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\svcs_proto.cpp

"$(INTDIR)\svcs_proto.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\svcs_rsa.cpp

"$(INTDIR)\svcs_rsa.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\svcs_srmm.cpp

"$(INTDIR)\svcs_srmm.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\secureim.pch"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

