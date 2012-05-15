# Microsoft Developer Studio Generated NMAKE File, Based on YAMN.dsp
!IF "$(CFG)" == ""
CFG=YAMN - Win32 Release Win2in1
!MESSAGE No configuration specified. Defaulting to YAMN - Win32 Release Win2in1.
!ENDIF 

!IF "$(CFG)" != "YAMN - Win32 Release Win2in1" && "$(CFG)" != "YAMN - Win32 Debug Win2in1" && "$(CFG)" != "YAMN - Win32 Release" && "$(CFG)" != "YAMN - Win32 Debug" && "$(CFG)" != "YAMN - Win32 Release Win9x" && "$(CFG)" != "YAMN - Win32 Debug Win9x"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "YAMN.mak" CFG="YAMN - Win32 Release Win2in1"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "YAMN - Win32 Release Win2in1" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Debug Win2in1" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Release Win9x" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "YAMN - Win32 Debug Win9x" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"

OUTDIR=.\Release\Win2in1
INTDIR=.\Release\Win2in1
# Begin Custom Macros
OutDir=.\Release\Win2in1
# End Custom Macros

ALL : "..\..\bin\release\plugins\yamn.dll"



CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\account.sbr"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\badconnect.sbr"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\debug.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\filterplugin.sbr"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mailbrowser.sbr"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\mails.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\mime.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3.sbr"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3comm.sbr"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\pop3opt.sbr"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\protoplugin.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\ssl.obj"
	-@erase "$(INTDIR)\ssl.sbr"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\synchro.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(INTDIR)\yamn.sbr"
	-@erase "$(OUTDIR)\YAMN.bsc"
	-@erase "$(OUTDIR)\yamn.exp"
	-@erase "..\..\bin\release\plugins\yamn.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN2IN1" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\badconnect.sbr" \
	"$(INTDIR)\mailbrowser.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\mails.sbr" \
	"$(INTDIR)\mime.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\pop3.sbr" \
	"$(INTDIR)\pop3comm.sbr" \
	"$(INTDIR)\pop3opt.sbr" \
	"$(INTDIR)\ssl.sbr" \
	"$(INTDIR)\account.sbr" \
	"$(INTDIR)\debug.sbr" \
	"$(INTDIR)\filterplugin.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\protoplugin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\synchro.sbr" \
	"$(INTDIR)\yamn.sbr"

"$(OUTDIR)\YAMN.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\yamn.pdb" /machine:I386 /out:"../../bin/release/plugins/yamn.dll" /implib:"$(OUTDIR)\yamn.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\release\plugins\yamn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"

OUTDIR=.\Debug\Win2in1
INTDIR=.\Debug\Win2in1
# Begin Custom Macros
OutDir=.\Debug\Win2in1
# End Custom Macros

ALL : "..\..\bin\Debug\plugins\YAMN.dll" "$(OUTDIR)\YAMN.bsc"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\account.sbr"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\badconnect.sbr"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\debug.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\filterplugin.sbr"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mailbrowser.sbr"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\mails.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\mime.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3.sbr"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3comm.sbr"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\pop3opt.sbr"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\protoplugin.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\synchro.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(INTDIR)\yamn.sbr"
	-@erase "$(OUTDIR)\YAMN.bsc"
	-@erase "$(OUTDIR)\YAMN.exp"
	-@erase "$(OUTDIR)\YAMN.pdb"
	-@erase "..\..\bin\Debug\plugins\YAMN.dll"
	-@erase "..\..\bin\Debug\plugins\YAMN.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN2IN1" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\badconnect.sbr" \
	"$(INTDIR)\mailbrowser.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\mails.sbr" \
	"$(INTDIR)\mime.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\pop3.sbr" \
	"$(INTDIR)\pop3comm.sbr" \
	"$(INTDIR)\pop3opt.sbr" \
	"$(INTDIR)\account.sbr" \
	"$(INTDIR)\debug.sbr" \
	"$(INTDIR)\filterplugin.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\protoplugin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\synchro.sbr" \
	"$(INTDIR)\yamn.sbr"

"$(OUTDIR)\YAMN.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\YAMN.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN.dll" /implib:"$(OUTDIR)\YAMN.lib" 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\Debug\plugins\YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"

OUTDIR=.\Release/WinNT
INTDIR=.\Release/WinNT

ALL : "..\..\bin\release\plugins\YAMN-NT\yamn.dll"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(OUTDIR)\yamn.exp"
	-@erase "..\..\bin\release\plugins\YAMN-NT\yamn.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\yamn.pdb" /machine:I386 /out:"../../bin/release/plugins/YAMN-NT/yamn.dll" /implib:"$(OUTDIR)\yamn.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\release\plugins\YAMN-NT\yamn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"

OUTDIR=.\Debug/WinNT
INTDIR=.\Debug/WinNT
# Begin Custom Macros
OutDir=.\Debug/WinNT
# End Custom Macros

ALL : "..\..\bin\Debug\plugins\YAMN-NT\YAMN.dll" "$(OUTDIR)\YAMN.bsc"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\account.sbr"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\badconnect.sbr"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\debug.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\filterplugin.sbr"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mailbrowser.sbr"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\mails.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\mime.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3.sbr"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3comm.sbr"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\pop3opt.sbr"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\protoplugin.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\synchro.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(INTDIR)\yamn.sbr"
	-@erase "$(OUTDIR)\YAMN.bsc"
	-@erase "$(OUTDIR)\YAMN.exp"
	-@erase "$(OUTDIR)\YAMN.pdb"
	-@erase "..\..\bin\Debug\plugins\YAMN-NT\YAMN.dll"
	-@erase "..\..\bin\Debug\plugins\YAMN-NT\YAMN.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\badconnect.sbr" \
	"$(INTDIR)\mailbrowser.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\mails.sbr" \
	"$(INTDIR)\mime.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\pop3.sbr" \
	"$(INTDIR)\pop3comm.sbr" \
	"$(INTDIR)\pop3opt.sbr" \
	"$(INTDIR)\account.sbr" \
	"$(INTDIR)\debug.sbr" \
	"$(INTDIR)\filterplugin.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\protoplugin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\synchro.sbr" \
	"$(INTDIR)\yamn.sbr"

"$(OUTDIR)\YAMN.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\YAMN.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN-NT/YAMN.dll" /implib:"$(OUTDIR)\YAMN.lib" 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\Debug\plugins\YAMN-NT\YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"

OUTDIR=.\Release\Win9x
INTDIR=.\Release\Win9x

ALL : "..\..\bin\Release\plugins\YAMN-9x\YAMN.dll"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(OUTDIR)\YAMN.exp"
	-@erase "..\..\bin\Release\plugins\YAMN-9x\YAMN.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G4 /Zp4 /MD /W3 /GX /O1 /Ob0 /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "WIN9X" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\YAMN.pdb" /machine:I386 /out:"../../bin/Release/plugins/YAMN-9x/YAMN.dll" /implib:"$(OUTDIR)\YAMN.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\Release\plugins\YAMN-9x\YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"

OUTDIR=.\Debug\Win9x
INTDIR=.\Debug\Win9x
# Begin Custom Macros
OutDir=.\Debug\Win9x
# End Custom Macros

ALL : "..\..\bin\Debug\plugins\YAMN-9x\YAMN.dll" "$(OUTDIR)\YAMN.bsc"


CLEAN :
	-@erase "$(INTDIR)\account.obj"
	-@erase "$(INTDIR)\account.sbr"
	-@erase "$(INTDIR)\badconnect.obj"
	-@erase "$(INTDIR)\badconnect.sbr"
	-@erase "$(INTDIR)\debug.obj"
	-@erase "$(INTDIR)\debug.sbr"
	-@erase "$(INTDIR)\decode.obj"
	-@erase "$(INTDIR)\decode.sbr"
	-@erase "$(INTDIR)\filterplugin.obj"
	-@erase "$(INTDIR)\filterplugin.sbr"
	-@erase "$(INTDIR)\mailbrowser.obj"
	-@erase "$(INTDIR)\mailbrowser.sbr"
	-@erase "$(INTDIR)\mails.obj"
	-@erase "$(INTDIR)\mails.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\mime.obj"
	-@erase "$(INTDIR)\mime.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\pop3.obj"
	-@erase "$(INTDIR)\pop3.sbr"
	-@erase "$(INTDIR)\pop3comm.obj"
	-@erase "$(INTDIR)\pop3comm.sbr"
	-@erase "$(INTDIR)\pop3opt.obj"
	-@erase "$(INTDIR)\pop3opt.sbr"
	-@erase "$(INTDIR)\protoplugin.obj"
	-@erase "$(INTDIR)\protoplugin.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\synchro.obj"
	-@erase "$(INTDIR)\synchro.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\yamn.obj"
	-@erase "$(INTDIR)\YAMN.res"
	-@erase "$(INTDIR)\yamn.sbr"
	-@erase "$(OUTDIR)\YAMN.bsc"
	-@erase "$(OUTDIR)\YAMN.exp"
	-@erase "$(OUTDIR)\YAMN.pdb"
	-@erase "..\..\bin\Debug\plugins\YAMN-9x\YAMN.dll"
	-@erase "..\..\bin\Debug\plugins\YAMN-9x\YAMN.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G4 /Zp4 /MDd /W3 /Gm /Gi /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "WIN9X" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\YAMN.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
MTL_PROJ=
RSC=rc.exe
RSC_PROJ=/l 0x417 /fo"$(INTDIR)\YAMN.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\YAMN.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\badconnect.sbr" \
	"$(INTDIR)\mailbrowser.sbr" \
	"$(INTDIR)\decode.sbr" \
	"$(INTDIR)\mails.sbr" \
	"$(INTDIR)\mime.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\pop3.sbr" \
	"$(INTDIR)\pop3comm.sbr" \
	"$(INTDIR)\pop3opt.sbr" \
	"$(INTDIR)\account.sbr" \
	"$(INTDIR)\debug.sbr" \
	"$(INTDIR)\filterplugin.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\protoplugin.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\synchro.sbr" \
	"$(INTDIR)\yamn.sbr"

"$(OUTDIR)\YAMN.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=libs/unicows.lib kernel32.lib user32.lib gdi32.lib comctl32.lib comdlg32.lib shell32.lib wsock32.lib /nologo /base:"0x60010000" /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\YAMN.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/YAMN-9x/YAMN.dll" /implib:"$(OUTDIR)\YAMN.lib" 
LINK32_OBJS= \
	"$(INTDIR)\badconnect.obj" \
	"$(INTDIR)\mailbrowser.obj" \
	"$(INTDIR)\decode.obj" \
	"$(INTDIR)\mails.obj" \
	"$(INTDIR)\mime.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\pop3.obj" \
	"$(INTDIR)\pop3comm.obj" \
	"$(INTDIR)\pop3opt.obj" \
	"$(INTDIR)\account.obj" \
	"$(INTDIR)\debug.obj" \
	"$(INTDIR)\filterplugin.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\protoplugin.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\synchro.obj" \
	"$(INTDIR)\yamn.obj" \
	"$(INTDIR)\YAMN.res"

"..\..\bin\Debug\plugins\YAMN-9x\YAMN.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("YAMN.dep")
!INCLUDE "YAMN.dep"
!ELSE 
!MESSAGE Warning: cannot find "YAMN.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "YAMN - Win32 Release Win2in1" || "$(CFG)" == "YAMN - Win32 Debug Win2in1" || "$(CFG)" == "YAMN - Win32 Release" || "$(CFG)" == "YAMN - Win32 Debug" || "$(CFG)" == "YAMN - Win32 Release Win9x" || "$(CFG)" == "YAMN - Win32 Debug Win9x"
SOURCE=.\browser\badconnect.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\badconnect.obj"	"$(INTDIR)\badconnect.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\badconnect.obj"	"$(INTDIR)\badconnect.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\badconnect.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\badconnect.obj"	"$(INTDIR)\badconnect.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\badconnect.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\badconnect.obj"	"$(INTDIR)\badconnect.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\browser\mailbrowser.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\mailbrowser.obj"	"$(INTDIR)\mailbrowser.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\mailbrowser.obj"	"$(INTDIR)\mailbrowser.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\mailbrowser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\mailbrowser.obj"	"$(INTDIR)\mailbrowser.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\mailbrowser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\mailbrowser.obj"	"$(INTDIR)\mailbrowser.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\mails\decode.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\decode.obj"	"$(INTDIR)\decode.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\decode.obj"	"$(INTDIR)\decode.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\decode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\decode.obj"	"$(INTDIR)\decode.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\decode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\decode.obj"	"$(INTDIR)\decode.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\mails\mails.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\mails.obj"	"$(INTDIR)\mails.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\mails.obj"	"$(INTDIR)\mails.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\mails.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\mails.obj"	"$(INTDIR)\mails.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\mails.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\mails.obj"	"$(INTDIR)\mails.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\mails\mime.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\mime.obj"	"$(INTDIR)\mime.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\mime.obj"	"$(INTDIR)\mime.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\mime.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\mime.obj"	"$(INTDIR)\mime.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\mime.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\mime.obj"	"$(INTDIR)\mime.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\proto\md5.c

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\proto\netlib.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\netlib.obj"	"$(INTDIR)\netlib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\netlib.obj"	"$(INTDIR)\netlib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\netlib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\netlib.obj"	"$(INTDIR)\netlib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\netlib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\netlib.obj"	"$(INTDIR)\netlib.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\proto\pop3\pop3.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\pop3.obj"	"$(INTDIR)\pop3.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\pop3.obj"	"$(INTDIR)\pop3.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\pop3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\pop3.obj"	"$(INTDIR)\pop3.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\pop3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\pop3.obj"	"$(INTDIR)\pop3.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\proto\pop3\pop3comm.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\pop3comm.obj"	"$(INTDIR)\pop3comm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\pop3comm.obj"	"$(INTDIR)\pop3comm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\pop3comm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\pop3comm.obj"	"$(INTDIR)\pop3comm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\pop3comm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\pop3comm.obj"	"$(INTDIR)\pop3comm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\proto\pop3\pop3opt.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\pop3opt.obj"	"$(INTDIR)\pop3opt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\pop3opt.obj"	"$(INTDIR)\pop3opt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\pop3opt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\pop3opt.obj"	"$(INTDIR)\pop3opt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\pop3opt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\pop3opt.obj"	"$(INTDIR)\pop3opt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\proto\ssl.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\ssl.obj"	"$(INTDIR)\ssl.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\ssl.obj"	"$(INTDIR)\ssl.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\ssl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\ssl.obj"	"$(INTDIR)\ssl.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\ssl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\ssl.obj"	"$(INTDIR)\ssl.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\account.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\account.obj"	"$(INTDIR)\account.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\account.obj"	"$(INTDIR)\account.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\account.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\account.obj"	"$(INTDIR)\account.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\account.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\account.obj"	"$(INTDIR)\account.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\debug.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\debug.obj"	"$(INTDIR)\debug.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\debug.obj"	"$(INTDIR)\debug.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\debug.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\debug.obj"	"$(INTDIR)\debug.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\debug.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\debug.obj"	"$(INTDIR)\debug.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\filterplugin.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\filterplugin.obj"	"$(INTDIR)\filterplugin.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\filterplugin.obj"	"$(INTDIR)\filterplugin.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\filterplugin.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\filterplugin.obj"	"$(INTDIR)\filterplugin.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\filterplugin.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\filterplugin.obj"	"$(INTDIR)\filterplugin.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\main.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\protoplugin.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\protoplugin.obj"	"$(INTDIR)\protoplugin.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\protoplugin.obj"	"$(INTDIR)\protoplugin.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\protoplugin.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\protoplugin.obj"	"$(INTDIR)\protoplugin.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\protoplugin.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\protoplugin.obj"	"$(INTDIR)\protoplugin.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\services.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\services.obj"	"$(INTDIR)\services.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\services.obj"	"$(INTDIR)\services.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\services.obj"	"$(INTDIR)\services.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\services.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\services.obj"	"$(INTDIR)\services.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\synchro.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\synchro.obj"	"$(INTDIR)\synchro.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\synchro.obj"	"$(INTDIR)\synchro.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\synchro.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\synchro.obj"	"$(INTDIR)\synchro.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\synchro.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\synchro.obj"	"$(INTDIR)\synchro.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\yamn.cpp

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\yamn.obj"	"$(INTDIR)\yamn.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\yamn.obj"	"$(INTDIR)\yamn.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\yamn.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\yamn.obj"	"$(INTDIR)\yamn.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\yamn.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\yamn.obj"	"$(INTDIR)\yamn.sbr" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\resources\YAMN.rc

!IF  "$(CFG)" == "YAMN - Win32 Release Win2in1"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win2in1"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "_DEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Release Win9x"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "NDEBUG" $(SOURCE)


!ELSEIF  "$(CFG)" == "YAMN - Win32 Debug Win9x"


"$(INTDIR)\YAMN.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0x417 /fo"$(INTDIR)\YAMN.res" /i "resources" /d "_DEBUG" $(SOURCE)


!ENDIF 


!ENDIF 

