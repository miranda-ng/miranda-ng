!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE Keine Konfiguration angegeben. Win32 Debug wird als Standard verwendet.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug" && "$(CFG)" != "Win64 Release" && "$(CFG)" != "Win64 Debug"
!MESSAGE Ungültige Konfiguration "$(CFG)" angegeben.
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "imoproxy.mak" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "Win32 Release"
!MESSAGE "Win32 Debug"
!MESSAGE "Win64 Release"
!MESSAGE "Win64 Debug"
!MESSAGE 
!ERROR Eine ungültige Konfiguration wurde angegeben.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
LINK32=link.exe

LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /dll /pdb:"$(INTDIR)\imoproxy.pdb" /out:"$(OUTDIR)\imoproxy.dll" /implib:"$(INTDIR)\imoproxy.lib" 
INCLUDES=/I "../src/common" /I "../src/imo2skype" /I "../src/imolib" /I "F:\miranda09_src\miranda" 

!IF  "$(CFG)" == "Win64 Release"
OUTDIR=..\bin64
INTDIR=.\Release64
CPP_PROJ=/nologo /MT /W3 /O2 $(INCLUDES) /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "imoproxy_EXPORTS" /Fp"$(INTDIR)\imoproxy.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
LINK32_FLAGS=$(LINK32_FLAGS) /incremental:no 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc07 /fo"$(INTDIR)\res.res" /d "NDEBUG" 
!ELSEIF  "$(CFG)" == "Win32 Release"
OUTDIR=..\bin
INTDIR=.\Release
CPP_PROJ=/nologo /MT /W3 /O2 $(INCLUDES) /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "imoproxy_EXPORTS" /Fp"$(INTDIR)\imoproxy.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
LINK32_FLAGS=$(LINK32_FLAGS) /incremental:no 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc07 /fo"$(INTDIR)\res.res" /d "NDEBUG" 
!ELSEIF  "$(CFG)" == "Win64 Debug"
OUTDIR=.\Debug64
INTDIR=$(OUTDIR)
CPP_PROJ=/nologo /MTd /W3 /Gm /ZI /Od $(INCLUDES) /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "imoproxy_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\imoproxy.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
LINK32_FLAGS=$(LINK32_FLAGS) /incremental:yes /debug /pdbtype:sept 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc07 /fo"$(INTDIR)\res.res" /d "_DEBUG" 
!ELSEIF  "$(CFG)" == "Win32 Debug"
OUTDIR=.\Debug
INTDIR=$(OUTDIR)
CPP_PROJ=/nologo /MTd /W3 /Gm /ZI /Od $(INCLUDES) /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "imoproxy_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\imoproxy.pch" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
LINK32_FLAGS=$(LINK32_FLAGS) /incremental:yes /debug /pdbtype:sept 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0xc07 /fo"$(INTDIR)\res.res" /d "_DEBUG" 
!ENDIF

LINK32_OBJS= \
	"$(INTDIR)\avatarlist.obj" \
	"$(INTDIR)\buddylist.obj" \
	"$(INTDIR)\callqueue.obj" \
	"$(INTDIR)\cJSON.obj" \
	"$(INTDIR)\fifo.obj" \
	"$(INTDIR)\imo2skypeapi.obj" \
	"$(INTDIR)\imo2sproxy.obj" \
	"$(INTDIR)\imo_request.obj" \
	"$(INTDIR)\imo_skype.obj" \
	"$(INTDIR)\imoproxy.obj" \
	"$(INTDIR)\io_layer_win32.obj" \
	"$(INTDIR)\io_layer_netlib.obj" \
	"$(INTDIR)\memlist.obj" \
	"$(INTDIR)\msgqueue.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\socksproxy.obj" \
	"$(INTDIR)\w32browser.obj" \
	"$(INTDIR)\w32skypeemu.obj" \
	"$(INTDIR)\res.res" \
	"$(INTDIR)\skypepluginlink.obj"


ALL : "$(OUTDIR)\imoproxy.dll"


CLEAN :
	-@erase "$(INTDIR)\avatarlist.obj"
	-@erase "$(INTDIR)\buddylist.obj"
	-@erase "$(INTDIR)\callqueue.obj"
	-@erase "$(INTDIR)\cJSON.obj"
	-@erase "$(INTDIR)\fifo.obj"
	-@erase "$(INTDIR)\imo2skypeapi.obj"
	-@erase "$(INTDIR)\imo2sproxy.obj"
	-@erase "$(INTDIR)\imo_request.obj"
	-@erase "$(INTDIR)\imo_skype.obj"
	-@erase "$(INTDIR)\imoproxy.obj"
	-@erase "$(INTDIR)\io_layer_win32.obj"
	-@erase "$(INTDIR)\io_layer_netlib.obj"
	-@erase "$(INTDIR)\memlist.obj"
	-@erase "$(INTDIR)\msgqueue.obj"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\res.res"
	-@erase "$(INTDIR)\skypepluginlink.obj"
	-@erase "$(INTDIR)\socksproxy.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\w32browser.obj"
	-@erase "$(INTDIR)\w32skypeemu.obj"
	-@erase "$(OUTDIR)\imoproxy.dll"
	-@erase "$(OUTDIR)\imoproxy.exp"
	-@erase "$(OUTDIR)\imoproxy.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(OUTDIR)\imoproxy.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("imoproxy.dep")
!INCLUDE "imoproxy.dep"
!ELSE 
!MESSAGE Warning: cannot find "imoproxy.dep"
!ENDIF 
!ENDIF 


SOURCE=..\src\imo2skype\avatarlist.c
"$(INTDIR)\avatarlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\buddylist.c
"$(INTDIR)\buddylist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\callqueue.c
"$(INTDIR)\callqueue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\common\cJSON.c
"$(INTDIR)\cJSON.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\common\fifo.c
"$(INTDIR)\fifo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\imo2skypeapi.c
"$(INTDIR)\imo2skypeapi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\imo2sproxy.c
"$(INTDIR)\imo2sproxy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imolib\imo_request.c
"$(INTDIR)\imo_request.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imolib\imo_skype.c
"$(INTDIR)\imo_skype.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\miranda\imoproxy.c
"$(INTDIR)\imoproxy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imolib\io_layer_win32.c
"$(INTDIR)\io_layer_win32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\common\memlist.c
"$(INTDIR)\memlist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\msgqueue.c
"$(INTDIR)\msgqueue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\queue.c
"$(INTDIR)\queue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\miranda\skypepluginlink.c
"$(INTDIR)\skypepluginlink.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\miranda\io_layer_netlib.c
"$(INTDIR)\io_layer_netlib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\socksproxy.c
"$(INTDIR)\socksproxy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\w32browser.c
"$(INTDIR)\w32browser.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\src\imo2skype\w32skypeemu.c
"$(INTDIR)\w32skypeemu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\src\imo2skype\miranda\res.rc
"$(INTDIR)\res.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) /l 0xc07 /fo"$(INTDIR)\res.res" /i "..\src\imo2skype\miranda" /d "NDEBUG" $(SOURCE)

