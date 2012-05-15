# Microsoft Developer Studio Generated NMAKE File, Based on GnuPGw.dsp
!IF "$(CFG)" == ""
CFG=GnuPGw - Win32 Release
!MESSAGE No configuration specified. Defaulting to GnuPGw - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "GnuPGw - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GnuPGw.mak" CFG="GnuPGw - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GnuPGw - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\GnuPGw.dll"


CLEAN :
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\GnuPGw.pch"
	-@erase "$(INTDIR)\gpg.obj"
	-@erase "$(INTDIR)\keys.obj"
	-@erase "$(INTDIR)\language.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\passdialog.obj"
	-@erase "$(INTDIR)\passphrases.obj"
	-@erase "$(INTDIR)\pipeexec.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\tools.obj"
	-@erase "$(INTDIR)\userdialog.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\GnuPGw.dll"
	-@erase "$(OUTDIR)\GnuPGw.exp"
	-@erase "$(OUTDIR)\GnuPGw.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O1 /Ob2 /Fp"$(INTDIR)\GnuPGw.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\GnuPGw.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib shell32.lib gdi32.lib msimg32.lib comdlg32.lib advapi32.lib msvcrt.lib /nologo /base:"0x46100000" /dll /incremental:no /pdb:"$(OUTDIR)\GnuPGw.pdb" /map:"$(INTDIR)\GnuPGw.map" /machine:I386 /out:"$(OUTDIR)\GnuPGw.dll" /implib:"$(OUTDIR)\GnuPGw.lib" /filealign:0x200 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\gpg.obj" \
	"$(INTDIR)\keys.obj" \
	"$(INTDIR)\language.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\passdialog.obj" \
	"$(INTDIR)\passphrases.obj" \
	"$(INTDIR)\pipeexec.obj" \
	"$(INTDIR)\tools.obj" \
	"$(INTDIR)\userdialog.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\GnuPGw.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("GnuPGw.dep")
!INCLUDE "GnuPGw.dep"
!ELSE 
!MESSAGE Warning: cannot find "GnuPGw.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "GnuPGw - Win32 Release"
SOURCE=.\commonheaders.c
CPP_SWITCHES=/nologo /MD /W3 /GX /O1 /Ob2 /Fp"$(INTDIR)\GnuPGw.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\GnuPGw.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


SOURCE=.\gpg.c

"$(INTDIR)\gpg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GnuPGw.pch"


SOURCE=.\keys.c

"$(INTDIR)\keys.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GnuPGw.pch"


SOURCE=.\language.c

"$(INTDIR)\language.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GnuPGw.pch"


SOURCE=.\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GnuPGw.pch"


SOURCE=.\passdialog.c

"$(INTDIR)\passdialog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GnuPGw.pch"


SOURCE=.\passphrases.c

"$(INTDIR)\passphrases.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GnuPGw.pch"


SOURCE=.\pipeexec.c

"$(INTDIR)\pipeexec.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GnuPGw.pch"


SOURCE=.\tools.c

"$(INTDIR)\tools.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GnuPGw.pch"


SOURCE=.\userdialog.c

"$(INTDIR)\userdialog.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\GnuPGw.pch"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

