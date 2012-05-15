# Microsoft Developer Studio Generated NMAKE File, Based on miranda32.dsp
!IF "$(CFG)" == ""
CFG=miranda32 - Win32 Debug Unicode
!MESSAGE No configuration specified. Defaulting to miranda32 - Win32 Debug Unicode.
!ENDIF 

!IF "$(CFG)" != "miranda32 - Win32 Release" && "$(CFG)" != "miranda32 - Win32 Debug" && "$(CFG)" != "miranda32 - Win32 Release Unicode" && "$(CFG)" != "miranda32 - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "miranda32.mak" CFG="miranda32 - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "miranda32 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "miranda32 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "miranda32 - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE "miranda32 - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "miranda32 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "..\bin\release\miranda32.exe" "$(OUTDIR)\miranda32.bsc"

!ELSE 

ALL : "zlib - Win32 Release" "..\bin\release\miranda32.exe" "$(OUTDIR)\miranda32.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"zlib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\addcontact.obj"
	-@erase "$(INTDIR)\addcontact.sbr"
	-@erase "$(INTDIR)\auth.obj"
	-@erase "$(INTDIR)\auth.sbr"
	-@erase "$(INTDIR)\authdialogs.obj"
	-@erase "$(INTDIR)\authdialogs.sbr"
	-@erase "$(INTDIR)\autoaway.obj"
	-@erase "$(INTDIR)\autoaway.sbr"
	-@erase "$(INTDIR)\awaymsg.obj"
	-@erase "$(INTDIR)\awaymsg.sbr"
	-@erase "$(INTDIR)\bmpfilter.obj"
	-@erase "$(INTDIR)\bmpfilter.sbr"
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\button.sbr"
	-@erase "$(INTDIR)\clc.obj"
	-@erase "$(INTDIR)\clc.sbr"
	-@erase "$(INTDIR)\clcfiledrop.obj"
	-@erase "$(INTDIR)\clcfiledrop.sbr"
	-@erase "$(INTDIR)\clcidents.obj"
	-@erase "$(INTDIR)\clcidents.sbr"
	-@erase "$(INTDIR)\clcitems.obj"
	-@erase "$(INTDIR)\clcitems.sbr"
	-@erase "$(INTDIR)\clcmsgs.obj"
	-@erase "$(INTDIR)\clcmsgs.sbr"
	-@erase "$(INTDIR)\clcutils.obj"
	-@erase "$(INTDIR)\clcutils.sbr"
	-@erase "$(INTDIR)\clistcore.obj"
	-@erase "$(INTDIR)\clistcore.sbr"
	-@erase "$(INTDIR)\clistevents.obj"
	-@erase "$(INTDIR)\clistevents.sbr"
	-@erase "$(INTDIR)\clistmenus.obj"
	-@erase "$(INTDIR)\clistmenus.sbr"
	-@erase "$(INTDIR)\clistmod.obj"
	-@erase "$(INTDIR)\clistmod.sbr"
	-@erase "$(INTDIR)\clistsettings.obj"
	-@erase "$(INTDIR)\clistsettings.sbr"
	-@erase "$(INTDIR)\clisttray.obj"
	-@erase "$(INTDIR)\clisttray.sbr"
	-@erase "$(INTDIR)\clui.obj"
	-@erase "$(INTDIR)\clui.sbr"
	-@erase "$(INTDIR)\cluiservices.obj"
	-@erase "$(INTDIR)\cluiservices.sbr"
	-@erase "$(INTDIR)\colourpicker.obj"
	-@erase "$(INTDIR)\colourpicker.sbr"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\commonheaders.sbr"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\contact.sbr"
	-@erase "$(INTDIR)\contactinfo.obj"
	-@erase "$(INTDIR)\contactinfo.sbr"
	-@erase "$(INTDIR)\contacts.obj"
	-@erase "$(INTDIR)\contacts.sbr"
	-@erase "$(INTDIR)\database.obj"
	-@erase "$(INTDIR)\database.sbr"
	-@erase "$(INTDIR)\dbini.obj"
	-@erase "$(INTDIR)\dbini.sbr"
	-@erase "$(INTDIR)\dblists.obj"
	-@erase "$(INTDIR)\dblists.sbr"
	-@erase "$(INTDIR)\dbutils.obj"
	-@erase "$(INTDIR)\dbutils.sbr"
	-@erase "$(INTDIR)\descbutton.obj"
	-@erase "$(INTDIR)\descbutton.sbr"
	-@erase "$(INTDIR)\Docking.obj"
	-@erase "$(INTDIR)\Docking.sbr"
	-@erase "$(INTDIR)\email.obj"
	-@erase "$(INTDIR)\email.sbr"
	-@erase "$(INTDIR)\extracticon.obj"
	-@erase "$(INTDIR)\extracticon.sbr"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\file.sbr"
	-@erase "$(INTDIR)\fileexistsdlg.obj"
	-@erase "$(INTDIR)\fileexistsdlg.sbr"
	-@erase "$(INTDIR)\fileopts.obj"
	-@erase "$(INTDIR)\fileopts.sbr"
	-@erase "$(INTDIR)\filerecvdlg.obj"
	-@erase "$(INTDIR)\filerecvdlg.sbr"
	-@erase "$(INTDIR)\filesenddlg.obj"
	-@erase "$(INTDIR)\filesenddlg.sbr"
	-@erase "$(INTDIR)\filexferdlg.obj"
	-@erase "$(INTDIR)\filexferdlg.sbr"
	-@erase "$(INTDIR)\filter.obj"
	-@erase "$(INTDIR)\filter.sbr"
	-@erase "$(INTDIR)\findadd.obj"
	-@erase "$(INTDIR)\findadd.sbr"
	-@erase "$(INTDIR)\FontOptions.obj"
	-@erase "$(INTDIR)\FontOptions.sbr"
	-@erase "$(INTDIR)\FontService.obj"
	-@erase "$(INTDIR)\FontService.sbr"
	-@erase "$(INTDIR)\ftmanager.obj"
	-@erase "$(INTDIR)\ftmanager.sbr"
	-@erase "$(INTDIR)\genmenu.obj"
	-@erase "$(INTDIR)\genmenu.sbr"
	-@erase "$(INTDIR)\genmenuopt.obj"
	-@erase "$(INTDIR)\genmenuopt.sbr"
	-@erase "$(INTDIR)\groups.obj"
	-@erase "$(INTDIR)\groups.sbr"
	-@erase "$(INTDIR)\headerbar.obj"
	-@erase "$(INTDIR)\headerbar.sbr"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\help.sbr"
	-@erase "$(INTDIR)\history.obj"
	-@erase "$(INTDIR)\history.sbr"
	-@erase "$(INTDIR)\hotkeys.obj"
	-@erase "$(INTDIR)\hotkeys.sbr"
	-@erase "$(INTDIR)\hyperlink.obj"
	-@erase "$(INTDIR)\hyperlink.sbr"
	-@erase "$(INTDIR)\iconheader.obj"
	-@erase "$(INTDIR)\iconheader.sbr"
	-@erase "$(INTDIR)\idle.obj"
	-@erase "$(INTDIR)\idle.sbr"
	-@erase "$(INTDIR)\ignore.obj"
	-@erase "$(INTDIR)\ignore.sbr"
	-@erase "$(INTDIR)\imgconv.obj"
	-@erase "$(INTDIR)\imgconv.sbr"
	-@erase "$(INTDIR)\keyboard.obj"
	-@erase "$(INTDIR)\keyboard.sbr"
	-@erase "$(INTDIR)\langpack.obj"
	-@erase "$(INTDIR)\langpack.sbr"
	-@erase "$(INTDIR)\lpservices.obj"
	-@erase "$(INTDIR)\lpservices.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\memory.obj"
	-@erase "$(INTDIR)\memory.sbr"
	-@erase "$(INTDIR)\miranda.obj"
	-@erase "$(INTDIR)\miranda.sbr"
	-@erase "$(INTDIR)\miranda32.pch"
	-@erase "$(INTDIR)\modules.obj"
	-@erase "$(INTDIR)\modules.sbr"
	-@erase "$(INTDIR)\movetogroup.obj"
	-@erase "$(INTDIR)\movetogroup.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\netlibautoproxy.obj"
	-@erase "$(INTDIR)\netlibautoproxy.sbr"
	-@erase "$(INTDIR)\netlibbind.obj"
	-@erase "$(INTDIR)\netlibbind.sbr"
	-@erase "$(INTDIR)\netlibhttp.obj"
	-@erase "$(INTDIR)\netlibhttp.sbr"
	-@erase "$(INTDIR)\netlibhttpproxy.obj"
	-@erase "$(INTDIR)\netlibhttpproxy.sbr"
	-@erase "$(INTDIR)\netliblog.obj"
	-@erase "$(INTDIR)\netliblog.sbr"
	-@erase "$(INTDIR)\netlibopenconn.obj"
	-@erase "$(INTDIR)\netlibopenconn.sbr"
	-@erase "$(INTDIR)\netlibopts.obj"
	-@erase "$(INTDIR)\netlibopts.sbr"
	-@erase "$(INTDIR)\netlibpktrecver.obj"
	-@erase "$(INTDIR)\netlibpktrecver.sbr"
	-@erase "$(INTDIR)\netlibsecurity.obj"
	-@erase "$(INTDIR)\netlibsecurity.sbr"
	-@erase "$(INTDIR)\netlibsock.obj"
	-@erase "$(INTDIR)\netlibsock.sbr"
	-@erase "$(INTDIR)\netlibssl.obj"
	-@erase "$(INTDIR)\netlibssl.sbr"
	-@erase "$(INTDIR)\netlibupnp.obj"
	-@erase "$(INTDIR)\netlibupnp.sbr"
	-@erase "$(INTDIR)\newplugins.obj"
	-@erase "$(INTDIR)\newplugins.sbr"
	-@erase "$(INTDIR)\openurl.obj"
	-@erase "$(INTDIR)\openurl.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\path.obj"
	-@erase "$(INTDIR)\path.sbr"
	-@erase "$(INTDIR)\profilemanager.obj"
	-@erase "$(INTDIR)\profilemanager.sbr"
	-@erase "$(INTDIR)\protoaccs.obj"
	-@erase "$(INTDIR)\protoaccs.sbr"
	-@erase "$(INTDIR)\protochains.obj"
	-@erase "$(INTDIR)\protochains.sbr"
	-@erase "$(INTDIR)\protocolorder.obj"
	-@erase "$(INTDIR)\protocolorder.sbr"
	-@erase "$(INTDIR)\protocols.obj"
	-@erase "$(INTDIR)\protocols.sbr"
	-@erase "$(INTDIR)\protoint.obj"
	-@erase "$(INTDIR)\protoint.sbr"
	-@erase "$(INTDIR)\protoopts.obj"
	-@erase "$(INTDIR)\protoopts.sbr"
	-@erase "$(INTDIR)\resizer.obj"
	-@erase "$(INTDIR)\resizer.sbr"
	-@erase "$(INTDIR)\searchresults.obj"
	-@erase "$(INTDIR)\searchresults.sbr"
	-@erase "$(INTDIR)\sendmsg.obj"
	-@erase "$(INTDIR)\sendmsg.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\sha1.sbr"
	-@erase "$(INTDIR)\skin2icons.obj"
	-@erase "$(INTDIR)\skin2icons.sbr"
	-@erase "$(INTDIR)\skinicons.obj"
	-@erase "$(INTDIR)\skinicons.sbr"
	-@erase "$(INTDIR)\sounds.obj"
	-@erase "$(INTDIR)\sounds.sbr"
	-@erase "$(INTDIR)\stdinfo.obj"
	-@erase "$(INTDIR)\stdinfo.sbr"
	-@erase "$(INTDIR)\timeutils.obj"
	-@erase "$(INTDIR)\timeutils.sbr"
	-@erase "$(INTDIR)\timezones.obj"
	-@erase "$(INTDIR)\timezones.sbr"
	-@erase "$(INTDIR)\updatenotify.obj"
	-@erase "$(INTDIR)\updatenotify.sbr"
	-@erase "$(INTDIR)\url.obj"
	-@erase "$(INTDIR)\url.sbr"
	-@erase "$(INTDIR)\urldialogs.obj"
	-@erase "$(INTDIR)\urldialogs.sbr"
	-@erase "$(INTDIR)\userinfo.obj"
	-@erase "$(INTDIR)\userinfo.sbr"
	-@erase "$(INTDIR)\useronline.obj"
	-@erase "$(INTDIR)\useronline.sbr"
	-@erase "$(INTDIR)\utf.obj"
	-@erase "$(INTDIR)\utf.sbr"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\utils.sbr"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\visibility.obj"
	-@erase "$(INTDIR)\visibility.sbr"
	-@erase "$(INTDIR)\windowlist.obj"
	-@erase "$(INTDIR)\windowlist.sbr"
	-@erase "$(INTDIR)\xmlApi.obj"
	-@erase "$(INTDIR)\xmlApi.sbr"
	-@erase "$(INTDIR)\xmlParser.obj"
	-@erase "$(INTDIR)\xmlParser.sbr"
	-@erase "$(OUTDIR)\miranda32.bsc"
	-@erase "$(OUTDIR)\miranda32.pdb"
	-@erase "..\bin\release\miranda32.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../include" /I "../include/msapi" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /i "../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\miranda32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commonheaders.sbr" \
	"$(INTDIR)\memory.sbr" \
	"$(INTDIR)\miranda.sbr" \
	"$(INTDIR)\modules.sbr" \
	"$(INTDIR)\addcontact.sbr" \
	"$(INTDIR)\autoaway.sbr" \
	"$(INTDIR)\button.sbr" \
	"$(INTDIR)\contacts.sbr" \
	"$(INTDIR)\database.sbr" \
	"$(INTDIR)\dbini.sbr" \
	"$(INTDIR)\dblists.sbr" \
	"$(INTDIR)\dbutils.sbr" \
	"$(INTDIR)\profilemanager.sbr" \
	"$(INTDIR)\findadd.sbr" \
	"$(INTDIR)\searchresults.sbr" \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\help.sbr" \
	"$(INTDIR)\history.sbr" \
	"$(INTDIR)\idle.sbr" \
	"$(INTDIR)\ignore.sbr" \
	"$(INTDIR)\langpack.sbr" \
	"$(INTDIR)\lpservices.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\netlibautoproxy.sbr" \
	"$(INTDIR)\netlibbind.sbr" \
	"$(INTDIR)\netlibhttp.sbr" \
	"$(INTDIR)\netlibhttpproxy.sbr" \
	"$(INTDIR)\netliblog.sbr" \
	"$(INTDIR)\netlibopenconn.sbr" \
	"$(INTDIR)\netlibopts.sbr" \
	"$(INTDIR)\netlibpktrecver.sbr" \
	"$(INTDIR)\netlibsecurity.sbr" \
	"$(INTDIR)\netlibsock.sbr" \
	"$(INTDIR)\netlibssl.sbr" \
	"$(INTDIR)\netlibupnp.sbr" \
	"$(INTDIR)\descbutton.sbr" \
	"$(INTDIR)\filter.sbr" \
	"$(INTDIR)\headerbar.sbr" \
	"$(INTDIR)\iconheader.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\newplugins.sbr" \
	"$(INTDIR)\protoaccs.sbr" \
	"$(INTDIR)\protochains.sbr" \
	"$(INTDIR)\protocols.sbr" \
	"$(INTDIR)\protoint.sbr" \
	"$(INTDIR)\protoopts.sbr" \
	"$(INTDIR)\hotkeys.sbr" \
	"$(INTDIR)\skinicons.sbr" \
	"$(INTDIR)\sounds.sbr" \
	"$(INTDIR)\auth.sbr" \
	"$(INTDIR)\authdialogs.sbr" \
	"$(INTDIR)\awaymsg.sbr" \
	"$(INTDIR)\sendmsg.sbr" \
	"$(INTDIR)\email.sbr" \
	"$(INTDIR)\file.sbr" \
	"$(INTDIR)\fileexistsdlg.sbr" \
	"$(INTDIR)\fileopts.sbr" \
	"$(INTDIR)\filerecvdlg.sbr" \
	"$(INTDIR)\filesenddlg.sbr" \
	"$(INTDIR)\filexferdlg.sbr" \
	"$(INTDIR)\ftmanager.sbr" \
	"$(INTDIR)\url.sbr" \
	"$(INTDIR)\urldialogs.sbr" \
	"$(INTDIR)\contactinfo.sbr" \
	"$(INTDIR)\stdinfo.sbr" \
	"$(INTDIR)\userinfo.sbr" \
	"$(INTDIR)\useronline.sbr" \
	"$(INTDIR)\bmpfilter.sbr" \
	"$(INTDIR)\colourpicker.sbr" \
	"$(INTDIR)\hyperlink.sbr" \
	"$(INTDIR)\imgconv.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\openurl.sbr" \
	"$(INTDIR)\path.sbr" \
	"$(INTDIR)\resizer.sbr" \
	"$(INTDIR)\sha1.sbr" \
	"$(INTDIR)\timeutils.sbr" \
	"$(INTDIR)\timezones.sbr" \
	"$(INTDIR)\utf.sbr" \
	"$(INTDIR)\utils.sbr" \
	"$(INTDIR)\windowlist.sbr" \
	"$(INTDIR)\visibility.sbr" \
	"$(INTDIR)\clc.sbr" \
	"$(INTDIR)\clcfiledrop.sbr" \
	"$(INTDIR)\clcidents.sbr" \
	"$(INTDIR)\clcitems.sbr" \
	"$(INTDIR)\clcmsgs.sbr" \
	"$(INTDIR)\clcutils.sbr" \
	"$(INTDIR)\clistcore.sbr" \
	"$(INTDIR)\clistevents.sbr" \
	"$(INTDIR)\clistmenus.sbr" \
	"$(INTDIR)\clistmod.sbr" \
	"$(INTDIR)\clistsettings.sbr" \
	"$(INTDIR)\clisttray.sbr" \
	"$(INTDIR)\clui.sbr" \
	"$(INTDIR)\cluiservices.sbr" \
	"$(INTDIR)\contact.sbr" \
	"$(INTDIR)\Docking.sbr" \
	"$(INTDIR)\genmenu.sbr" \
	"$(INTDIR)\genmenuopt.sbr" \
	"$(INTDIR)\groups.sbr" \
	"$(INTDIR)\keyboard.sbr" \
	"$(INTDIR)\movetogroup.sbr" \
	"$(INTDIR)\protocolorder.sbr" \
	"$(INTDIR)\FontOptions.sbr" \
	"$(INTDIR)\FontService.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\extracticon.sbr" \
	"$(INTDIR)\skin2icons.sbr" \
	"$(INTDIR)\updatenotify.sbr" \
	"$(INTDIR)\xmlApi.sbr" \
	"$(INTDIR)\xmlParser.sbr"

"$(OUTDIR)\miranda32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ws2_32.lib kernel32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib winmm.lib version.lib crypt32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\miranda32.pdb" /debug /machine:I386 /out:"../bin/release/miranda32.exe" /fixed /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\memory.obj" \
	"$(INTDIR)\miranda.obj" \
	"$(INTDIR)\modules.obj" \
	"$(INTDIR)\addcontact.obj" \
	"$(INTDIR)\autoaway.obj" \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\contacts.obj" \
	"$(INTDIR)\database.obj" \
	"$(INTDIR)\dbini.obj" \
	"$(INTDIR)\dblists.obj" \
	"$(INTDIR)\dbutils.obj" \
	"$(INTDIR)\profilemanager.obj" \
	"$(INTDIR)\findadd.obj" \
	"$(INTDIR)\searchresults.obj" \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\history.obj" \
	"$(INTDIR)\idle.obj" \
	"$(INTDIR)\ignore.obj" \
	"$(INTDIR)\langpack.obj" \
	"$(INTDIR)\lpservices.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\netlibautoproxy.obj" \
	"$(INTDIR)\netlibbind.obj" \
	"$(INTDIR)\netlibhttp.obj" \
	"$(INTDIR)\netlibhttpproxy.obj" \
	"$(INTDIR)\netliblog.obj" \
	"$(INTDIR)\netlibopenconn.obj" \
	"$(INTDIR)\netlibopts.obj" \
	"$(INTDIR)\netlibpktrecver.obj" \
	"$(INTDIR)\netlibsecurity.obj" \
	"$(INTDIR)\netlibsock.obj" \
	"$(INTDIR)\netlibssl.obj" \
	"$(INTDIR)\netlibupnp.obj" \
	"$(INTDIR)\descbutton.obj" \
	"$(INTDIR)\filter.obj" \
	"$(INTDIR)\headerbar.obj" \
	"$(INTDIR)\iconheader.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\newplugins.obj" \
	"$(INTDIR)\protoaccs.obj" \
	"$(INTDIR)\protochains.obj" \
	"$(INTDIR)\protocols.obj" \
	"$(INTDIR)\protoint.obj" \
	"$(INTDIR)\protoopts.obj" \
	"$(INTDIR)\hotkeys.obj" \
	"$(INTDIR)\skinicons.obj" \
	"$(INTDIR)\sounds.obj" \
	"$(INTDIR)\auth.obj" \
	"$(INTDIR)\authdialogs.obj" \
	"$(INTDIR)\awaymsg.obj" \
	"$(INTDIR)\sendmsg.obj" \
	"$(INTDIR)\email.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\fileexistsdlg.obj" \
	"$(INTDIR)\fileopts.obj" \
	"$(INTDIR)\filerecvdlg.obj" \
	"$(INTDIR)\filesenddlg.obj" \
	"$(INTDIR)\filexferdlg.obj" \
	"$(INTDIR)\ftmanager.obj" \
	"$(INTDIR)\url.obj" \
	"$(INTDIR)\urldialogs.obj" \
	"$(INTDIR)\contactinfo.obj" \
	"$(INTDIR)\stdinfo.obj" \
	"$(INTDIR)\userinfo.obj" \
	"$(INTDIR)\useronline.obj" \
	"$(INTDIR)\bmpfilter.obj" \
	"$(INTDIR)\colourpicker.obj" \
	"$(INTDIR)\hyperlink.obj" \
	"$(INTDIR)\imgconv.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\openurl.obj" \
	"$(INTDIR)\path.obj" \
	"$(INTDIR)\resizer.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\timeutils.obj" \
	"$(INTDIR)\timezones.obj" \
	"$(INTDIR)\utf.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\windowlist.obj" \
	"$(INTDIR)\visibility.obj" \
	"$(INTDIR)\clc.obj" \
	"$(INTDIR)\clcfiledrop.obj" \
	"$(INTDIR)\clcidents.obj" \
	"$(INTDIR)\clcitems.obj" \
	"$(INTDIR)\clcmsgs.obj" \
	"$(INTDIR)\clcutils.obj" \
	"$(INTDIR)\clistcore.obj" \
	"$(INTDIR)\clistevents.obj" \
	"$(INTDIR)\clistmenus.obj" \
	"$(INTDIR)\clistmod.obj" \
	"$(INTDIR)\clistsettings.obj" \
	"$(INTDIR)\clisttray.obj" \
	"$(INTDIR)\clui.obj" \
	"$(INTDIR)\cluiservices.obj" \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\Docking.obj" \
	"$(INTDIR)\genmenu.obj" \
	"$(INTDIR)\genmenuopt.obj" \
	"$(INTDIR)\groups.obj" \
	"$(INTDIR)\keyboard.obj" \
	"$(INTDIR)\movetogroup.obj" \
	"$(INTDIR)\protocolorder.obj" \
	"$(INTDIR)\FontOptions.obj" \
	"$(INTDIR)\FontService.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\extracticon.obj" \
	"$(INTDIR)\skin2icons.obj" \
	"$(INTDIR)\updatenotify.obj" \
	"$(INTDIR)\xmlApi.obj" \
	"$(INTDIR)\xmlParser.obj" \
	"$(INTDIR)\vc6.res" \
	"..\plugins\zlib\Release\zlib.lib"

"..\bin\release\miranda32.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "miranda32 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "..\bin\debug\miranda32.exe" "$(OUTDIR)\miranda32.bsc"

!ELSE 

ALL : "zlib - Win32 Debug" "..\bin\debug\miranda32.exe" "$(OUTDIR)\miranda32.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"zlib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\addcontact.obj"
	-@erase "$(INTDIR)\addcontact.sbr"
	-@erase "$(INTDIR)\auth.obj"
	-@erase "$(INTDIR)\auth.sbr"
	-@erase "$(INTDIR)\authdialogs.obj"
	-@erase "$(INTDIR)\authdialogs.sbr"
	-@erase "$(INTDIR)\autoaway.obj"
	-@erase "$(INTDIR)\autoaway.sbr"
	-@erase "$(INTDIR)\awaymsg.obj"
	-@erase "$(INTDIR)\awaymsg.sbr"
	-@erase "$(INTDIR)\bmpfilter.obj"
	-@erase "$(INTDIR)\bmpfilter.sbr"
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\button.sbr"
	-@erase "$(INTDIR)\clc.obj"
	-@erase "$(INTDIR)\clc.sbr"
	-@erase "$(INTDIR)\clcfiledrop.obj"
	-@erase "$(INTDIR)\clcfiledrop.sbr"
	-@erase "$(INTDIR)\clcidents.obj"
	-@erase "$(INTDIR)\clcidents.sbr"
	-@erase "$(INTDIR)\clcitems.obj"
	-@erase "$(INTDIR)\clcitems.sbr"
	-@erase "$(INTDIR)\clcmsgs.obj"
	-@erase "$(INTDIR)\clcmsgs.sbr"
	-@erase "$(INTDIR)\clcutils.obj"
	-@erase "$(INTDIR)\clcutils.sbr"
	-@erase "$(INTDIR)\clistcore.obj"
	-@erase "$(INTDIR)\clistcore.sbr"
	-@erase "$(INTDIR)\clistevents.obj"
	-@erase "$(INTDIR)\clistevents.sbr"
	-@erase "$(INTDIR)\clistmenus.obj"
	-@erase "$(INTDIR)\clistmenus.sbr"
	-@erase "$(INTDIR)\clistmod.obj"
	-@erase "$(INTDIR)\clistmod.sbr"
	-@erase "$(INTDIR)\clistsettings.obj"
	-@erase "$(INTDIR)\clistsettings.sbr"
	-@erase "$(INTDIR)\clisttray.obj"
	-@erase "$(INTDIR)\clisttray.sbr"
	-@erase "$(INTDIR)\clui.obj"
	-@erase "$(INTDIR)\clui.sbr"
	-@erase "$(INTDIR)\cluiservices.obj"
	-@erase "$(INTDIR)\cluiservices.sbr"
	-@erase "$(INTDIR)\colourpicker.obj"
	-@erase "$(INTDIR)\colourpicker.sbr"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\commonheaders.sbr"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\contact.sbr"
	-@erase "$(INTDIR)\contactinfo.obj"
	-@erase "$(INTDIR)\contactinfo.sbr"
	-@erase "$(INTDIR)\contacts.obj"
	-@erase "$(INTDIR)\contacts.sbr"
	-@erase "$(INTDIR)\database.obj"
	-@erase "$(INTDIR)\database.sbr"
	-@erase "$(INTDIR)\dbini.obj"
	-@erase "$(INTDIR)\dbini.sbr"
	-@erase "$(INTDIR)\dblists.obj"
	-@erase "$(INTDIR)\dblists.sbr"
	-@erase "$(INTDIR)\dbutils.obj"
	-@erase "$(INTDIR)\dbutils.sbr"
	-@erase "$(INTDIR)\descbutton.obj"
	-@erase "$(INTDIR)\descbutton.sbr"
	-@erase "$(INTDIR)\Docking.obj"
	-@erase "$(INTDIR)\Docking.sbr"
	-@erase "$(INTDIR)\email.obj"
	-@erase "$(INTDIR)\email.sbr"
	-@erase "$(INTDIR)\extracticon.obj"
	-@erase "$(INTDIR)\extracticon.sbr"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\file.sbr"
	-@erase "$(INTDIR)\fileexistsdlg.obj"
	-@erase "$(INTDIR)\fileexistsdlg.sbr"
	-@erase "$(INTDIR)\fileopts.obj"
	-@erase "$(INTDIR)\fileopts.sbr"
	-@erase "$(INTDIR)\filerecvdlg.obj"
	-@erase "$(INTDIR)\filerecvdlg.sbr"
	-@erase "$(INTDIR)\filesenddlg.obj"
	-@erase "$(INTDIR)\filesenddlg.sbr"
	-@erase "$(INTDIR)\filexferdlg.obj"
	-@erase "$(INTDIR)\filexferdlg.sbr"
	-@erase "$(INTDIR)\filter.obj"
	-@erase "$(INTDIR)\filter.sbr"
	-@erase "$(INTDIR)\findadd.obj"
	-@erase "$(INTDIR)\findadd.sbr"
	-@erase "$(INTDIR)\FontOptions.obj"
	-@erase "$(INTDIR)\FontOptions.sbr"
	-@erase "$(INTDIR)\FontService.obj"
	-@erase "$(INTDIR)\FontService.sbr"
	-@erase "$(INTDIR)\ftmanager.obj"
	-@erase "$(INTDIR)\ftmanager.sbr"
	-@erase "$(INTDIR)\genmenu.obj"
	-@erase "$(INTDIR)\genmenu.sbr"
	-@erase "$(INTDIR)\genmenuopt.obj"
	-@erase "$(INTDIR)\genmenuopt.sbr"
	-@erase "$(INTDIR)\groups.obj"
	-@erase "$(INTDIR)\groups.sbr"
	-@erase "$(INTDIR)\headerbar.obj"
	-@erase "$(INTDIR)\headerbar.sbr"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\help.sbr"
	-@erase "$(INTDIR)\history.obj"
	-@erase "$(INTDIR)\history.sbr"
	-@erase "$(INTDIR)\hotkeys.obj"
	-@erase "$(INTDIR)\hotkeys.sbr"
	-@erase "$(INTDIR)\hyperlink.obj"
	-@erase "$(INTDIR)\hyperlink.sbr"
	-@erase "$(INTDIR)\iconheader.obj"
	-@erase "$(INTDIR)\iconheader.sbr"
	-@erase "$(INTDIR)\idle.obj"
	-@erase "$(INTDIR)\idle.sbr"
	-@erase "$(INTDIR)\ignore.obj"
	-@erase "$(INTDIR)\ignore.sbr"
	-@erase "$(INTDIR)\imgconv.obj"
	-@erase "$(INTDIR)\imgconv.sbr"
	-@erase "$(INTDIR)\keyboard.obj"
	-@erase "$(INTDIR)\keyboard.sbr"
	-@erase "$(INTDIR)\langpack.obj"
	-@erase "$(INTDIR)\langpack.sbr"
	-@erase "$(INTDIR)\lpservices.obj"
	-@erase "$(INTDIR)\lpservices.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\memory.obj"
	-@erase "$(INTDIR)\memory.sbr"
	-@erase "$(INTDIR)\miranda.obj"
	-@erase "$(INTDIR)\miranda.sbr"
	-@erase "$(INTDIR)\miranda32.pch"
	-@erase "$(INTDIR)\modules.obj"
	-@erase "$(INTDIR)\modules.sbr"
	-@erase "$(INTDIR)\movetogroup.obj"
	-@erase "$(INTDIR)\movetogroup.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\netlibautoproxy.obj"
	-@erase "$(INTDIR)\netlibautoproxy.sbr"
	-@erase "$(INTDIR)\netlibbind.obj"
	-@erase "$(INTDIR)\netlibbind.sbr"
	-@erase "$(INTDIR)\netlibhttp.obj"
	-@erase "$(INTDIR)\netlibhttp.sbr"
	-@erase "$(INTDIR)\netlibhttpproxy.obj"
	-@erase "$(INTDIR)\netlibhttpproxy.sbr"
	-@erase "$(INTDIR)\netliblog.obj"
	-@erase "$(INTDIR)\netliblog.sbr"
	-@erase "$(INTDIR)\netlibopenconn.obj"
	-@erase "$(INTDIR)\netlibopenconn.sbr"
	-@erase "$(INTDIR)\netlibopts.obj"
	-@erase "$(INTDIR)\netlibopts.sbr"
	-@erase "$(INTDIR)\netlibpktrecver.obj"
	-@erase "$(INTDIR)\netlibpktrecver.sbr"
	-@erase "$(INTDIR)\netlibsecurity.obj"
	-@erase "$(INTDIR)\netlibsecurity.sbr"
	-@erase "$(INTDIR)\netlibsock.obj"
	-@erase "$(INTDIR)\netlibsock.sbr"
	-@erase "$(INTDIR)\netlibssl.obj"
	-@erase "$(INTDIR)\netlibssl.sbr"
	-@erase "$(INTDIR)\netlibupnp.obj"
	-@erase "$(INTDIR)\netlibupnp.sbr"
	-@erase "$(INTDIR)\newplugins.obj"
	-@erase "$(INTDIR)\newplugins.sbr"
	-@erase "$(INTDIR)\openurl.obj"
	-@erase "$(INTDIR)\openurl.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\path.obj"
	-@erase "$(INTDIR)\path.sbr"
	-@erase "$(INTDIR)\profilemanager.obj"
	-@erase "$(INTDIR)\profilemanager.sbr"
	-@erase "$(INTDIR)\protoaccs.obj"
	-@erase "$(INTDIR)\protoaccs.sbr"
	-@erase "$(INTDIR)\protochains.obj"
	-@erase "$(INTDIR)\protochains.sbr"
	-@erase "$(INTDIR)\protocolorder.obj"
	-@erase "$(INTDIR)\protocolorder.sbr"
	-@erase "$(INTDIR)\protocols.obj"
	-@erase "$(INTDIR)\protocols.sbr"
	-@erase "$(INTDIR)\protoint.obj"
	-@erase "$(INTDIR)\protoint.sbr"
	-@erase "$(INTDIR)\protoopts.obj"
	-@erase "$(INTDIR)\protoopts.sbr"
	-@erase "$(INTDIR)\resizer.obj"
	-@erase "$(INTDIR)\resizer.sbr"
	-@erase "$(INTDIR)\searchresults.obj"
	-@erase "$(INTDIR)\searchresults.sbr"
	-@erase "$(INTDIR)\sendmsg.obj"
	-@erase "$(INTDIR)\sendmsg.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\sha1.sbr"
	-@erase "$(INTDIR)\skin2icons.obj"
	-@erase "$(INTDIR)\skin2icons.sbr"
	-@erase "$(INTDIR)\skinicons.obj"
	-@erase "$(INTDIR)\skinicons.sbr"
	-@erase "$(INTDIR)\sounds.obj"
	-@erase "$(INTDIR)\sounds.sbr"
	-@erase "$(INTDIR)\stdinfo.obj"
	-@erase "$(INTDIR)\stdinfo.sbr"
	-@erase "$(INTDIR)\timeutils.obj"
	-@erase "$(INTDIR)\timeutils.sbr"
	-@erase "$(INTDIR)\timezones.obj"
	-@erase "$(INTDIR)\timezones.sbr"
	-@erase "$(INTDIR)\updatenotify.obj"
	-@erase "$(INTDIR)\updatenotify.sbr"
	-@erase "$(INTDIR)\url.obj"
	-@erase "$(INTDIR)\url.sbr"
	-@erase "$(INTDIR)\urldialogs.obj"
	-@erase "$(INTDIR)\urldialogs.sbr"
	-@erase "$(INTDIR)\userinfo.obj"
	-@erase "$(INTDIR)\userinfo.sbr"
	-@erase "$(INTDIR)\useronline.obj"
	-@erase "$(INTDIR)\useronline.sbr"
	-@erase "$(INTDIR)\utf.obj"
	-@erase "$(INTDIR)\utf.sbr"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\utils.sbr"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\visibility.obj"
	-@erase "$(INTDIR)\visibility.sbr"
	-@erase "$(INTDIR)\windowlist.obj"
	-@erase "$(INTDIR)\windowlist.sbr"
	-@erase "$(INTDIR)\xmlApi.obj"
	-@erase "$(INTDIR)\xmlApi.sbr"
	-@erase "$(INTDIR)\xmlParser.obj"
	-@erase "$(INTDIR)\xmlParser.sbr"
	-@erase "$(OUTDIR)\miranda32.bsc"
	-@erase "$(OUTDIR)\miranda32.pdb"
	-@erase "..\bin\debug\miranda32.exe"
	-@erase "..\bin\debug\miranda32.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /ZI /Od /I "../include" /I "../include/msapi" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /i "../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\miranda32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commonheaders.sbr" \
	"$(INTDIR)\memory.sbr" \
	"$(INTDIR)\miranda.sbr" \
	"$(INTDIR)\modules.sbr" \
	"$(INTDIR)\addcontact.sbr" \
	"$(INTDIR)\autoaway.sbr" \
	"$(INTDIR)\button.sbr" \
	"$(INTDIR)\contacts.sbr" \
	"$(INTDIR)\database.sbr" \
	"$(INTDIR)\dbini.sbr" \
	"$(INTDIR)\dblists.sbr" \
	"$(INTDIR)\dbutils.sbr" \
	"$(INTDIR)\profilemanager.sbr" \
	"$(INTDIR)\findadd.sbr" \
	"$(INTDIR)\searchresults.sbr" \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\help.sbr" \
	"$(INTDIR)\history.sbr" \
	"$(INTDIR)\idle.sbr" \
	"$(INTDIR)\ignore.sbr" \
	"$(INTDIR)\langpack.sbr" \
	"$(INTDIR)\lpservices.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\netlibautoproxy.sbr" \
	"$(INTDIR)\netlibbind.sbr" \
	"$(INTDIR)\netlibhttp.sbr" \
	"$(INTDIR)\netlibhttpproxy.sbr" \
	"$(INTDIR)\netliblog.sbr" \
	"$(INTDIR)\netlibopenconn.sbr" \
	"$(INTDIR)\netlibopts.sbr" \
	"$(INTDIR)\netlibpktrecver.sbr" \
	"$(INTDIR)\netlibsecurity.sbr" \
	"$(INTDIR)\netlibsock.sbr" \
	"$(INTDIR)\netlibssl.sbr" \
	"$(INTDIR)\netlibupnp.sbr" \
	"$(INTDIR)\descbutton.sbr" \
	"$(INTDIR)\filter.sbr" \
	"$(INTDIR)\headerbar.sbr" \
	"$(INTDIR)\iconheader.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\newplugins.sbr" \
	"$(INTDIR)\protoaccs.sbr" \
	"$(INTDIR)\protochains.sbr" \
	"$(INTDIR)\protocols.sbr" \
	"$(INTDIR)\protoint.sbr" \
	"$(INTDIR)\protoopts.sbr" \
	"$(INTDIR)\hotkeys.sbr" \
	"$(INTDIR)\skinicons.sbr" \
	"$(INTDIR)\sounds.sbr" \
	"$(INTDIR)\auth.sbr" \
	"$(INTDIR)\authdialogs.sbr" \
	"$(INTDIR)\awaymsg.sbr" \
	"$(INTDIR)\sendmsg.sbr" \
	"$(INTDIR)\email.sbr" \
	"$(INTDIR)\file.sbr" \
	"$(INTDIR)\fileexistsdlg.sbr" \
	"$(INTDIR)\fileopts.sbr" \
	"$(INTDIR)\filerecvdlg.sbr" \
	"$(INTDIR)\filesenddlg.sbr" \
	"$(INTDIR)\filexferdlg.sbr" \
	"$(INTDIR)\ftmanager.sbr" \
	"$(INTDIR)\url.sbr" \
	"$(INTDIR)\urldialogs.sbr" \
	"$(INTDIR)\contactinfo.sbr" \
	"$(INTDIR)\stdinfo.sbr" \
	"$(INTDIR)\userinfo.sbr" \
	"$(INTDIR)\useronline.sbr" \
	"$(INTDIR)\bmpfilter.sbr" \
	"$(INTDIR)\colourpicker.sbr" \
	"$(INTDIR)\hyperlink.sbr" \
	"$(INTDIR)\imgconv.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\openurl.sbr" \
	"$(INTDIR)\path.sbr" \
	"$(INTDIR)\resizer.sbr" \
	"$(INTDIR)\sha1.sbr" \
	"$(INTDIR)\timeutils.sbr" \
	"$(INTDIR)\timezones.sbr" \
	"$(INTDIR)\utf.sbr" \
	"$(INTDIR)\utils.sbr" \
	"$(INTDIR)\windowlist.sbr" \
	"$(INTDIR)\visibility.sbr" \
	"$(INTDIR)\clc.sbr" \
	"$(INTDIR)\clcfiledrop.sbr" \
	"$(INTDIR)\clcidents.sbr" \
	"$(INTDIR)\clcitems.sbr" \
	"$(INTDIR)\clcmsgs.sbr" \
	"$(INTDIR)\clcutils.sbr" \
	"$(INTDIR)\clistcore.sbr" \
	"$(INTDIR)\clistevents.sbr" \
	"$(INTDIR)\clistmenus.sbr" \
	"$(INTDIR)\clistmod.sbr" \
	"$(INTDIR)\clistsettings.sbr" \
	"$(INTDIR)\clisttray.sbr" \
	"$(INTDIR)\clui.sbr" \
	"$(INTDIR)\cluiservices.sbr" \
	"$(INTDIR)\contact.sbr" \
	"$(INTDIR)\Docking.sbr" \
	"$(INTDIR)\genmenu.sbr" \
	"$(INTDIR)\genmenuopt.sbr" \
	"$(INTDIR)\groups.sbr" \
	"$(INTDIR)\keyboard.sbr" \
	"$(INTDIR)\movetogroup.sbr" \
	"$(INTDIR)\protocolorder.sbr" \
	"$(INTDIR)\FontOptions.sbr" \
	"$(INTDIR)\FontService.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\extracticon.sbr" \
	"$(INTDIR)\skin2icons.sbr" \
	"$(INTDIR)\updatenotify.sbr" \
	"$(INTDIR)\xmlApi.sbr" \
	"$(INTDIR)\xmlParser.sbr"

"$(OUTDIR)\miranda32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ws2_32.lib kernel32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib winmm.lib version.lib crypt32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\miranda32.pdb" /debug /machine:I386 /out:"../bin/debug/miranda32.exe" 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\memory.obj" \
	"$(INTDIR)\miranda.obj" \
	"$(INTDIR)\modules.obj" \
	"$(INTDIR)\addcontact.obj" \
	"$(INTDIR)\autoaway.obj" \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\contacts.obj" \
	"$(INTDIR)\database.obj" \
	"$(INTDIR)\dbini.obj" \
	"$(INTDIR)\dblists.obj" \
	"$(INTDIR)\dbutils.obj" \
	"$(INTDIR)\profilemanager.obj" \
	"$(INTDIR)\findadd.obj" \
	"$(INTDIR)\searchresults.obj" \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\history.obj" \
	"$(INTDIR)\idle.obj" \
	"$(INTDIR)\ignore.obj" \
	"$(INTDIR)\langpack.obj" \
	"$(INTDIR)\lpservices.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\netlibautoproxy.obj" \
	"$(INTDIR)\netlibbind.obj" \
	"$(INTDIR)\netlibhttp.obj" \
	"$(INTDIR)\netlibhttpproxy.obj" \
	"$(INTDIR)\netliblog.obj" \
	"$(INTDIR)\netlibopenconn.obj" \
	"$(INTDIR)\netlibopts.obj" \
	"$(INTDIR)\netlibpktrecver.obj" \
	"$(INTDIR)\netlibsecurity.obj" \
	"$(INTDIR)\netlibsock.obj" \
	"$(INTDIR)\netlibssl.obj" \
	"$(INTDIR)\netlibupnp.obj" \
	"$(INTDIR)\descbutton.obj" \
	"$(INTDIR)\filter.obj" \
	"$(INTDIR)\headerbar.obj" \
	"$(INTDIR)\iconheader.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\newplugins.obj" \
	"$(INTDIR)\protoaccs.obj" \
	"$(INTDIR)\protochains.obj" \
	"$(INTDIR)\protocols.obj" \
	"$(INTDIR)\protoint.obj" \
	"$(INTDIR)\protoopts.obj" \
	"$(INTDIR)\hotkeys.obj" \
	"$(INTDIR)\skinicons.obj" \
	"$(INTDIR)\sounds.obj" \
	"$(INTDIR)\auth.obj" \
	"$(INTDIR)\authdialogs.obj" \
	"$(INTDIR)\awaymsg.obj" \
	"$(INTDIR)\sendmsg.obj" \
	"$(INTDIR)\email.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\fileexistsdlg.obj" \
	"$(INTDIR)\fileopts.obj" \
	"$(INTDIR)\filerecvdlg.obj" \
	"$(INTDIR)\filesenddlg.obj" \
	"$(INTDIR)\filexferdlg.obj" \
	"$(INTDIR)\ftmanager.obj" \
	"$(INTDIR)\url.obj" \
	"$(INTDIR)\urldialogs.obj" \
	"$(INTDIR)\contactinfo.obj" \
	"$(INTDIR)\stdinfo.obj" \
	"$(INTDIR)\userinfo.obj" \
	"$(INTDIR)\useronline.obj" \
	"$(INTDIR)\bmpfilter.obj" \
	"$(INTDIR)\colourpicker.obj" \
	"$(INTDIR)\hyperlink.obj" \
	"$(INTDIR)\imgconv.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\openurl.obj" \
	"$(INTDIR)\path.obj" \
	"$(INTDIR)\resizer.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\timeutils.obj" \
	"$(INTDIR)\timezones.obj" \
	"$(INTDIR)\utf.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\windowlist.obj" \
	"$(INTDIR)\visibility.obj" \
	"$(INTDIR)\clc.obj" \
	"$(INTDIR)\clcfiledrop.obj" \
	"$(INTDIR)\clcidents.obj" \
	"$(INTDIR)\clcitems.obj" \
	"$(INTDIR)\clcmsgs.obj" \
	"$(INTDIR)\clcutils.obj" \
	"$(INTDIR)\clistcore.obj" \
	"$(INTDIR)\clistevents.obj" \
	"$(INTDIR)\clistmenus.obj" \
	"$(INTDIR)\clistmod.obj" \
	"$(INTDIR)\clistsettings.obj" \
	"$(INTDIR)\clisttray.obj" \
	"$(INTDIR)\clui.obj" \
	"$(INTDIR)\cluiservices.obj" \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\Docking.obj" \
	"$(INTDIR)\genmenu.obj" \
	"$(INTDIR)\genmenuopt.obj" \
	"$(INTDIR)\groups.obj" \
	"$(INTDIR)\keyboard.obj" \
	"$(INTDIR)\movetogroup.obj" \
	"$(INTDIR)\protocolorder.obj" \
	"$(INTDIR)\FontOptions.obj" \
	"$(INTDIR)\FontService.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\extracticon.obj" \
	"$(INTDIR)\skin2icons.obj" \
	"$(INTDIR)\updatenotify.obj" \
	"$(INTDIR)\xmlApi.obj" \
	"$(INTDIR)\xmlParser.obj" \
	"$(INTDIR)\vc6.res" \
	"..\plugins\zlib\Debug\zlib.lib"

"..\bin\debug\miranda32.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "miranda32 - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode
# Begin Custom Macros
OutDir=.\Release_Unicode
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "..\bin\Release Unicode\miranda32.exe" "$(OUTDIR)\miranda32.bsc"

!ELSE 

ALL : "zlib - Win32 Release Unicode" "..\bin\Release Unicode\miranda32.exe" "$(OUTDIR)\miranda32.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"zlib - Win32 Release UnicodeCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\addcontact.obj"
	-@erase "$(INTDIR)\addcontact.sbr"
	-@erase "$(INTDIR)\auth.obj"
	-@erase "$(INTDIR)\auth.sbr"
	-@erase "$(INTDIR)\authdialogs.obj"
	-@erase "$(INTDIR)\authdialogs.sbr"
	-@erase "$(INTDIR)\autoaway.obj"
	-@erase "$(INTDIR)\autoaway.sbr"
	-@erase "$(INTDIR)\awaymsg.obj"
	-@erase "$(INTDIR)\awaymsg.sbr"
	-@erase "$(INTDIR)\bmpfilter.obj"
	-@erase "$(INTDIR)\bmpfilter.sbr"
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\button.sbr"
	-@erase "$(INTDIR)\clc.obj"
	-@erase "$(INTDIR)\clc.sbr"
	-@erase "$(INTDIR)\clcfiledrop.obj"
	-@erase "$(INTDIR)\clcfiledrop.sbr"
	-@erase "$(INTDIR)\clcidents.obj"
	-@erase "$(INTDIR)\clcidents.sbr"
	-@erase "$(INTDIR)\clcitems.obj"
	-@erase "$(INTDIR)\clcitems.sbr"
	-@erase "$(INTDIR)\clcmsgs.obj"
	-@erase "$(INTDIR)\clcmsgs.sbr"
	-@erase "$(INTDIR)\clcutils.obj"
	-@erase "$(INTDIR)\clcutils.sbr"
	-@erase "$(INTDIR)\clistcore.obj"
	-@erase "$(INTDIR)\clistcore.sbr"
	-@erase "$(INTDIR)\clistevents.obj"
	-@erase "$(INTDIR)\clistevents.sbr"
	-@erase "$(INTDIR)\clistmenus.obj"
	-@erase "$(INTDIR)\clistmenus.sbr"
	-@erase "$(INTDIR)\clistmod.obj"
	-@erase "$(INTDIR)\clistmod.sbr"
	-@erase "$(INTDIR)\clistsettings.obj"
	-@erase "$(INTDIR)\clistsettings.sbr"
	-@erase "$(INTDIR)\clisttray.obj"
	-@erase "$(INTDIR)\clisttray.sbr"
	-@erase "$(INTDIR)\clui.obj"
	-@erase "$(INTDIR)\clui.sbr"
	-@erase "$(INTDIR)\cluiservices.obj"
	-@erase "$(INTDIR)\cluiservices.sbr"
	-@erase "$(INTDIR)\colourpicker.obj"
	-@erase "$(INTDIR)\colourpicker.sbr"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\commonheaders.sbr"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\contact.sbr"
	-@erase "$(INTDIR)\contactinfo.obj"
	-@erase "$(INTDIR)\contactinfo.sbr"
	-@erase "$(INTDIR)\contacts.obj"
	-@erase "$(INTDIR)\contacts.sbr"
	-@erase "$(INTDIR)\database.obj"
	-@erase "$(INTDIR)\database.sbr"
	-@erase "$(INTDIR)\dbini.obj"
	-@erase "$(INTDIR)\dbini.sbr"
	-@erase "$(INTDIR)\dblists.obj"
	-@erase "$(INTDIR)\dblists.sbr"
	-@erase "$(INTDIR)\dbutils.obj"
	-@erase "$(INTDIR)\dbutils.sbr"
	-@erase "$(INTDIR)\descbutton.obj"
	-@erase "$(INTDIR)\descbutton.sbr"
	-@erase "$(INTDIR)\Docking.obj"
	-@erase "$(INTDIR)\Docking.sbr"
	-@erase "$(INTDIR)\email.obj"
	-@erase "$(INTDIR)\email.sbr"
	-@erase "$(INTDIR)\extracticon.obj"
	-@erase "$(INTDIR)\extracticon.sbr"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\file.sbr"
	-@erase "$(INTDIR)\fileexistsdlg.obj"
	-@erase "$(INTDIR)\fileexistsdlg.sbr"
	-@erase "$(INTDIR)\fileopts.obj"
	-@erase "$(INTDIR)\fileopts.sbr"
	-@erase "$(INTDIR)\filerecvdlg.obj"
	-@erase "$(INTDIR)\filerecvdlg.sbr"
	-@erase "$(INTDIR)\filesenddlg.obj"
	-@erase "$(INTDIR)\filesenddlg.sbr"
	-@erase "$(INTDIR)\filexferdlg.obj"
	-@erase "$(INTDIR)\filexferdlg.sbr"
	-@erase "$(INTDIR)\filter.obj"
	-@erase "$(INTDIR)\filter.sbr"
	-@erase "$(INTDIR)\findadd.obj"
	-@erase "$(INTDIR)\findadd.sbr"
	-@erase "$(INTDIR)\FontOptions.obj"
	-@erase "$(INTDIR)\FontOptions.sbr"
	-@erase "$(INTDIR)\FontService.obj"
	-@erase "$(INTDIR)\FontService.sbr"
	-@erase "$(INTDIR)\ftmanager.obj"
	-@erase "$(INTDIR)\ftmanager.sbr"
	-@erase "$(INTDIR)\genmenu.obj"
	-@erase "$(INTDIR)\genmenu.sbr"
	-@erase "$(INTDIR)\genmenuopt.obj"
	-@erase "$(INTDIR)\genmenuopt.sbr"
	-@erase "$(INTDIR)\groups.obj"
	-@erase "$(INTDIR)\groups.sbr"
	-@erase "$(INTDIR)\headerbar.obj"
	-@erase "$(INTDIR)\headerbar.sbr"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\help.sbr"
	-@erase "$(INTDIR)\history.obj"
	-@erase "$(INTDIR)\history.sbr"
	-@erase "$(INTDIR)\hotkeys.obj"
	-@erase "$(INTDIR)\hotkeys.sbr"
	-@erase "$(INTDIR)\hyperlink.obj"
	-@erase "$(INTDIR)\hyperlink.sbr"
	-@erase "$(INTDIR)\iconheader.obj"
	-@erase "$(INTDIR)\iconheader.sbr"
	-@erase "$(INTDIR)\idle.obj"
	-@erase "$(INTDIR)\idle.sbr"
	-@erase "$(INTDIR)\ignore.obj"
	-@erase "$(INTDIR)\ignore.sbr"
	-@erase "$(INTDIR)\imgconv.obj"
	-@erase "$(INTDIR)\imgconv.sbr"
	-@erase "$(INTDIR)\keyboard.obj"
	-@erase "$(INTDIR)\keyboard.sbr"
	-@erase "$(INTDIR)\langpack.obj"
	-@erase "$(INTDIR)\langpack.sbr"
	-@erase "$(INTDIR)\lpservices.obj"
	-@erase "$(INTDIR)\lpservices.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\memory.obj"
	-@erase "$(INTDIR)\memory.sbr"
	-@erase "$(INTDIR)\miranda.obj"
	-@erase "$(INTDIR)\miranda.sbr"
	-@erase "$(INTDIR)\miranda32.pch"
	-@erase "$(INTDIR)\modules.obj"
	-@erase "$(INTDIR)\modules.sbr"
	-@erase "$(INTDIR)\movetogroup.obj"
	-@erase "$(INTDIR)\movetogroup.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\netlibautoproxy.obj"
	-@erase "$(INTDIR)\netlibautoproxy.sbr"
	-@erase "$(INTDIR)\netlibbind.obj"
	-@erase "$(INTDIR)\netlibbind.sbr"
	-@erase "$(INTDIR)\netlibhttp.obj"
	-@erase "$(INTDIR)\netlibhttp.sbr"
	-@erase "$(INTDIR)\netlibhttpproxy.obj"
	-@erase "$(INTDIR)\netlibhttpproxy.sbr"
	-@erase "$(INTDIR)\netliblog.obj"
	-@erase "$(INTDIR)\netliblog.sbr"
	-@erase "$(INTDIR)\netlibopenconn.obj"
	-@erase "$(INTDIR)\netlibopenconn.sbr"
	-@erase "$(INTDIR)\netlibopts.obj"
	-@erase "$(INTDIR)\netlibopts.sbr"
	-@erase "$(INTDIR)\netlibpktrecver.obj"
	-@erase "$(INTDIR)\netlibpktrecver.sbr"
	-@erase "$(INTDIR)\netlibsecurity.obj"
	-@erase "$(INTDIR)\netlibsecurity.sbr"
	-@erase "$(INTDIR)\netlibsock.obj"
	-@erase "$(INTDIR)\netlibsock.sbr"
	-@erase "$(INTDIR)\netlibssl.obj"
	-@erase "$(INTDIR)\netlibssl.sbr"
	-@erase "$(INTDIR)\netlibupnp.obj"
	-@erase "$(INTDIR)\netlibupnp.sbr"
	-@erase "$(INTDIR)\newplugins.obj"
	-@erase "$(INTDIR)\newplugins.sbr"
	-@erase "$(INTDIR)\openurl.obj"
	-@erase "$(INTDIR)\openurl.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\path.obj"
	-@erase "$(INTDIR)\path.sbr"
	-@erase "$(INTDIR)\profilemanager.obj"
	-@erase "$(INTDIR)\profilemanager.sbr"
	-@erase "$(INTDIR)\protoaccs.obj"
	-@erase "$(INTDIR)\protoaccs.sbr"
	-@erase "$(INTDIR)\protochains.obj"
	-@erase "$(INTDIR)\protochains.sbr"
	-@erase "$(INTDIR)\protocolorder.obj"
	-@erase "$(INTDIR)\protocolorder.sbr"
	-@erase "$(INTDIR)\protocols.obj"
	-@erase "$(INTDIR)\protocols.sbr"
	-@erase "$(INTDIR)\protoint.obj"
	-@erase "$(INTDIR)\protoint.sbr"
	-@erase "$(INTDIR)\protoopts.obj"
	-@erase "$(INTDIR)\protoopts.sbr"
	-@erase "$(INTDIR)\resizer.obj"
	-@erase "$(INTDIR)\resizer.sbr"
	-@erase "$(INTDIR)\searchresults.obj"
	-@erase "$(INTDIR)\searchresults.sbr"
	-@erase "$(INTDIR)\sendmsg.obj"
	-@erase "$(INTDIR)\sendmsg.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\sha1.sbr"
	-@erase "$(INTDIR)\skin2icons.obj"
	-@erase "$(INTDIR)\skin2icons.sbr"
	-@erase "$(INTDIR)\skinicons.obj"
	-@erase "$(INTDIR)\skinicons.sbr"
	-@erase "$(INTDIR)\sounds.obj"
	-@erase "$(INTDIR)\sounds.sbr"
	-@erase "$(INTDIR)\stdinfo.obj"
	-@erase "$(INTDIR)\stdinfo.sbr"
	-@erase "$(INTDIR)\timeutils.obj"
	-@erase "$(INTDIR)\timeutils.sbr"
	-@erase "$(INTDIR)\timezones.obj"
	-@erase "$(INTDIR)\timezones.sbr"
	-@erase "$(INTDIR)\updatenotify.obj"
	-@erase "$(INTDIR)\updatenotify.sbr"
	-@erase "$(INTDIR)\url.obj"
	-@erase "$(INTDIR)\url.sbr"
	-@erase "$(INTDIR)\urldialogs.obj"
	-@erase "$(INTDIR)\urldialogs.sbr"
	-@erase "$(INTDIR)\userinfo.obj"
	-@erase "$(INTDIR)\userinfo.sbr"
	-@erase "$(INTDIR)\useronline.obj"
	-@erase "$(INTDIR)\useronline.sbr"
	-@erase "$(INTDIR)\utf.obj"
	-@erase "$(INTDIR)\utf.sbr"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\utils.sbr"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\visibility.obj"
	-@erase "$(INTDIR)\visibility.sbr"
	-@erase "$(INTDIR)\windowlist.obj"
	-@erase "$(INTDIR)\windowlist.sbr"
	-@erase "$(INTDIR)\xmlApi.obj"
	-@erase "$(INTDIR)\xmlApi.sbr"
	-@erase "$(INTDIR)\xmlParser.obj"
	-@erase "$(INTDIR)\xmlParser.sbr"
	-@erase "$(OUTDIR)\miranda32.bsc"
	-@erase "$(OUTDIR)\miranda32.pdb"
	-@erase "..\bin\Release Unicode\miranda32.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Zi /O1 /I "../include/msapi" /I "../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "UNICODE" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /i "../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\miranda32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commonheaders.sbr" \
	"$(INTDIR)\memory.sbr" \
	"$(INTDIR)\miranda.sbr" \
	"$(INTDIR)\modules.sbr" \
	"$(INTDIR)\addcontact.sbr" \
	"$(INTDIR)\autoaway.sbr" \
	"$(INTDIR)\button.sbr" \
	"$(INTDIR)\contacts.sbr" \
	"$(INTDIR)\database.sbr" \
	"$(INTDIR)\dbini.sbr" \
	"$(INTDIR)\dblists.sbr" \
	"$(INTDIR)\dbutils.sbr" \
	"$(INTDIR)\profilemanager.sbr" \
	"$(INTDIR)\findadd.sbr" \
	"$(INTDIR)\searchresults.sbr" \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\help.sbr" \
	"$(INTDIR)\history.sbr" \
	"$(INTDIR)\idle.sbr" \
	"$(INTDIR)\ignore.sbr" \
	"$(INTDIR)\langpack.sbr" \
	"$(INTDIR)\lpservices.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\netlibautoproxy.sbr" \
	"$(INTDIR)\netlibbind.sbr" \
	"$(INTDIR)\netlibhttp.sbr" \
	"$(INTDIR)\netlibhttpproxy.sbr" \
	"$(INTDIR)\netliblog.sbr" \
	"$(INTDIR)\netlibopenconn.sbr" \
	"$(INTDIR)\netlibopts.sbr" \
	"$(INTDIR)\netlibpktrecver.sbr" \
	"$(INTDIR)\netlibsecurity.sbr" \
	"$(INTDIR)\netlibsock.sbr" \
	"$(INTDIR)\netlibssl.sbr" \
	"$(INTDIR)\netlibupnp.sbr" \
	"$(INTDIR)\descbutton.sbr" \
	"$(INTDIR)\filter.sbr" \
	"$(INTDIR)\headerbar.sbr" \
	"$(INTDIR)\iconheader.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\newplugins.sbr" \
	"$(INTDIR)\protoaccs.sbr" \
	"$(INTDIR)\protochains.sbr" \
	"$(INTDIR)\protocols.sbr" \
	"$(INTDIR)\protoint.sbr" \
	"$(INTDIR)\protoopts.sbr" \
	"$(INTDIR)\hotkeys.sbr" \
	"$(INTDIR)\skinicons.sbr" \
	"$(INTDIR)\sounds.sbr" \
	"$(INTDIR)\auth.sbr" \
	"$(INTDIR)\authdialogs.sbr" \
	"$(INTDIR)\awaymsg.sbr" \
	"$(INTDIR)\sendmsg.sbr" \
	"$(INTDIR)\email.sbr" \
	"$(INTDIR)\file.sbr" \
	"$(INTDIR)\fileexistsdlg.sbr" \
	"$(INTDIR)\fileopts.sbr" \
	"$(INTDIR)\filerecvdlg.sbr" \
	"$(INTDIR)\filesenddlg.sbr" \
	"$(INTDIR)\filexferdlg.sbr" \
	"$(INTDIR)\ftmanager.sbr" \
	"$(INTDIR)\url.sbr" \
	"$(INTDIR)\urldialogs.sbr" \
	"$(INTDIR)\contactinfo.sbr" \
	"$(INTDIR)\stdinfo.sbr" \
	"$(INTDIR)\userinfo.sbr" \
	"$(INTDIR)\useronline.sbr" \
	"$(INTDIR)\bmpfilter.sbr" \
	"$(INTDIR)\colourpicker.sbr" \
	"$(INTDIR)\hyperlink.sbr" \
	"$(INTDIR)\imgconv.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\openurl.sbr" \
	"$(INTDIR)\path.sbr" \
	"$(INTDIR)\resizer.sbr" \
	"$(INTDIR)\sha1.sbr" \
	"$(INTDIR)\timeutils.sbr" \
	"$(INTDIR)\timezones.sbr" \
	"$(INTDIR)\utf.sbr" \
	"$(INTDIR)\utils.sbr" \
	"$(INTDIR)\windowlist.sbr" \
	"$(INTDIR)\visibility.sbr" \
	"$(INTDIR)\clc.sbr" \
	"$(INTDIR)\clcfiledrop.sbr" \
	"$(INTDIR)\clcidents.sbr" \
	"$(INTDIR)\clcitems.sbr" \
	"$(INTDIR)\clcmsgs.sbr" \
	"$(INTDIR)\clcutils.sbr" \
	"$(INTDIR)\clistcore.sbr" \
	"$(INTDIR)\clistevents.sbr" \
	"$(INTDIR)\clistmenus.sbr" \
	"$(INTDIR)\clistmod.sbr" \
	"$(INTDIR)\clistsettings.sbr" \
	"$(INTDIR)\clisttray.sbr" \
	"$(INTDIR)\clui.sbr" \
	"$(INTDIR)\cluiservices.sbr" \
	"$(INTDIR)\contact.sbr" \
	"$(INTDIR)\Docking.sbr" \
	"$(INTDIR)\genmenu.sbr" \
	"$(INTDIR)\genmenuopt.sbr" \
	"$(INTDIR)\groups.sbr" \
	"$(INTDIR)\keyboard.sbr" \
	"$(INTDIR)\movetogroup.sbr" \
	"$(INTDIR)\protocolorder.sbr" \
	"$(INTDIR)\FontOptions.sbr" \
	"$(INTDIR)\FontService.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\extracticon.sbr" \
	"$(INTDIR)\skin2icons.sbr" \
	"$(INTDIR)\updatenotify.sbr" \
	"$(INTDIR)\xmlApi.sbr" \
	"$(INTDIR)\xmlParser.sbr"

"$(OUTDIR)\miranda32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ws2_32.lib kernel32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib winmm.lib version.lib crypt32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\miranda32.pdb" /debug /machine:I386 /out:"../bin/Release Unicode/miranda32.exe" /fixed /ALIGN:4096 /ignore:4108 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\memory.obj" \
	"$(INTDIR)\miranda.obj" \
	"$(INTDIR)\modules.obj" \
	"$(INTDIR)\addcontact.obj" \
	"$(INTDIR)\autoaway.obj" \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\contacts.obj" \
	"$(INTDIR)\database.obj" \
	"$(INTDIR)\dbini.obj" \
	"$(INTDIR)\dblists.obj" \
	"$(INTDIR)\dbutils.obj" \
	"$(INTDIR)\profilemanager.obj" \
	"$(INTDIR)\findadd.obj" \
	"$(INTDIR)\searchresults.obj" \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\history.obj" \
	"$(INTDIR)\idle.obj" \
	"$(INTDIR)\ignore.obj" \
	"$(INTDIR)\langpack.obj" \
	"$(INTDIR)\lpservices.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\netlibautoproxy.obj" \
	"$(INTDIR)\netlibbind.obj" \
	"$(INTDIR)\netlibhttp.obj" \
	"$(INTDIR)\netlibhttpproxy.obj" \
	"$(INTDIR)\netliblog.obj" \
	"$(INTDIR)\netlibopenconn.obj" \
	"$(INTDIR)\netlibopts.obj" \
	"$(INTDIR)\netlibpktrecver.obj" \
	"$(INTDIR)\netlibsecurity.obj" \
	"$(INTDIR)\netlibsock.obj" \
	"$(INTDIR)\netlibssl.obj" \
	"$(INTDIR)\netlibupnp.obj" \
	"$(INTDIR)\descbutton.obj" \
	"$(INTDIR)\filter.obj" \
	"$(INTDIR)\headerbar.obj" \
	"$(INTDIR)\iconheader.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\newplugins.obj" \
	"$(INTDIR)\protoaccs.obj" \
	"$(INTDIR)\protochains.obj" \
	"$(INTDIR)\protocols.obj" \
	"$(INTDIR)\protoint.obj" \
	"$(INTDIR)\protoopts.obj" \
	"$(INTDIR)\hotkeys.obj" \
	"$(INTDIR)\skinicons.obj" \
	"$(INTDIR)\sounds.obj" \
	"$(INTDIR)\auth.obj" \
	"$(INTDIR)\authdialogs.obj" \
	"$(INTDIR)\awaymsg.obj" \
	"$(INTDIR)\sendmsg.obj" \
	"$(INTDIR)\email.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\fileexistsdlg.obj" \
	"$(INTDIR)\fileopts.obj" \
	"$(INTDIR)\filerecvdlg.obj" \
	"$(INTDIR)\filesenddlg.obj" \
	"$(INTDIR)\filexferdlg.obj" \
	"$(INTDIR)\ftmanager.obj" \
	"$(INTDIR)\url.obj" \
	"$(INTDIR)\urldialogs.obj" \
	"$(INTDIR)\contactinfo.obj" \
	"$(INTDIR)\stdinfo.obj" \
	"$(INTDIR)\userinfo.obj" \
	"$(INTDIR)\useronline.obj" \
	"$(INTDIR)\bmpfilter.obj" \
	"$(INTDIR)\colourpicker.obj" \
	"$(INTDIR)\hyperlink.obj" \
	"$(INTDIR)\imgconv.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\openurl.obj" \
	"$(INTDIR)\path.obj" \
	"$(INTDIR)\resizer.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\timeutils.obj" \
	"$(INTDIR)\timezones.obj" \
	"$(INTDIR)\utf.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\windowlist.obj" \
	"$(INTDIR)\visibility.obj" \
	"$(INTDIR)\clc.obj" \
	"$(INTDIR)\clcfiledrop.obj" \
	"$(INTDIR)\clcidents.obj" \
	"$(INTDIR)\clcitems.obj" \
	"$(INTDIR)\clcmsgs.obj" \
	"$(INTDIR)\clcutils.obj" \
	"$(INTDIR)\clistcore.obj" \
	"$(INTDIR)\clistevents.obj" \
	"$(INTDIR)\clistmenus.obj" \
	"$(INTDIR)\clistmod.obj" \
	"$(INTDIR)\clistsettings.obj" \
	"$(INTDIR)\clisttray.obj" \
	"$(INTDIR)\clui.obj" \
	"$(INTDIR)\cluiservices.obj" \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\Docking.obj" \
	"$(INTDIR)\genmenu.obj" \
	"$(INTDIR)\genmenuopt.obj" \
	"$(INTDIR)\groups.obj" \
	"$(INTDIR)\keyboard.obj" \
	"$(INTDIR)\movetogroup.obj" \
	"$(INTDIR)\protocolorder.obj" \
	"$(INTDIR)\FontOptions.obj" \
	"$(INTDIR)\FontService.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\extracticon.obj" \
	"$(INTDIR)\skin2icons.obj" \
	"$(INTDIR)\updatenotify.obj" \
	"$(INTDIR)\xmlApi.obj" \
	"$(INTDIR)\xmlParser.obj" \
	"$(INTDIR)\vc6.res" \
	"..\plugins\zlib\Release_Unicode\zlib.lib"

"..\bin\Release Unicode\miranda32.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "miranda32 - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "..\bin\Debug Unicode\miranda32.exe" "$(OUTDIR)\miranda32.bsc"

!ELSE 

ALL : "zlib - Win32 Debug Unicode" "..\bin\Debug Unicode\miranda32.exe" "$(OUTDIR)\miranda32.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"zlib - Win32 Debug UnicodeCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\about.obj"
	-@erase "$(INTDIR)\about.sbr"
	-@erase "$(INTDIR)\addcontact.obj"
	-@erase "$(INTDIR)\addcontact.sbr"
	-@erase "$(INTDIR)\auth.obj"
	-@erase "$(INTDIR)\auth.sbr"
	-@erase "$(INTDIR)\authdialogs.obj"
	-@erase "$(INTDIR)\authdialogs.sbr"
	-@erase "$(INTDIR)\autoaway.obj"
	-@erase "$(INTDIR)\autoaway.sbr"
	-@erase "$(INTDIR)\awaymsg.obj"
	-@erase "$(INTDIR)\awaymsg.sbr"
	-@erase "$(INTDIR)\bmpfilter.obj"
	-@erase "$(INTDIR)\bmpfilter.sbr"
	-@erase "$(INTDIR)\button.obj"
	-@erase "$(INTDIR)\button.sbr"
	-@erase "$(INTDIR)\clc.obj"
	-@erase "$(INTDIR)\clc.sbr"
	-@erase "$(INTDIR)\clcfiledrop.obj"
	-@erase "$(INTDIR)\clcfiledrop.sbr"
	-@erase "$(INTDIR)\clcidents.obj"
	-@erase "$(INTDIR)\clcidents.sbr"
	-@erase "$(INTDIR)\clcitems.obj"
	-@erase "$(INTDIR)\clcitems.sbr"
	-@erase "$(INTDIR)\clcmsgs.obj"
	-@erase "$(INTDIR)\clcmsgs.sbr"
	-@erase "$(INTDIR)\clcutils.obj"
	-@erase "$(INTDIR)\clcutils.sbr"
	-@erase "$(INTDIR)\clistcore.obj"
	-@erase "$(INTDIR)\clistcore.sbr"
	-@erase "$(INTDIR)\clistevents.obj"
	-@erase "$(INTDIR)\clistevents.sbr"
	-@erase "$(INTDIR)\clistmenus.obj"
	-@erase "$(INTDIR)\clistmenus.sbr"
	-@erase "$(INTDIR)\clistmod.obj"
	-@erase "$(INTDIR)\clistmod.sbr"
	-@erase "$(INTDIR)\clistsettings.obj"
	-@erase "$(INTDIR)\clistsettings.sbr"
	-@erase "$(INTDIR)\clisttray.obj"
	-@erase "$(INTDIR)\clisttray.sbr"
	-@erase "$(INTDIR)\clui.obj"
	-@erase "$(INTDIR)\clui.sbr"
	-@erase "$(INTDIR)\cluiservices.obj"
	-@erase "$(INTDIR)\cluiservices.sbr"
	-@erase "$(INTDIR)\colourpicker.obj"
	-@erase "$(INTDIR)\colourpicker.sbr"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\commonheaders.sbr"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\contact.sbr"
	-@erase "$(INTDIR)\contactinfo.obj"
	-@erase "$(INTDIR)\contactinfo.sbr"
	-@erase "$(INTDIR)\contacts.obj"
	-@erase "$(INTDIR)\contacts.sbr"
	-@erase "$(INTDIR)\database.obj"
	-@erase "$(INTDIR)\database.sbr"
	-@erase "$(INTDIR)\dbini.obj"
	-@erase "$(INTDIR)\dbini.sbr"
	-@erase "$(INTDIR)\dblists.obj"
	-@erase "$(INTDIR)\dblists.sbr"
	-@erase "$(INTDIR)\dbutils.obj"
	-@erase "$(INTDIR)\dbutils.sbr"
	-@erase "$(INTDIR)\descbutton.obj"
	-@erase "$(INTDIR)\descbutton.sbr"
	-@erase "$(INTDIR)\Docking.obj"
	-@erase "$(INTDIR)\Docking.sbr"
	-@erase "$(INTDIR)\email.obj"
	-@erase "$(INTDIR)\email.sbr"
	-@erase "$(INTDIR)\extracticon.obj"
	-@erase "$(INTDIR)\extracticon.sbr"
	-@erase "$(INTDIR)\file.obj"
	-@erase "$(INTDIR)\file.sbr"
	-@erase "$(INTDIR)\fileexistsdlg.obj"
	-@erase "$(INTDIR)\fileexistsdlg.sbr"
	-@erase "$(INTDIR)\fileopts.obj"
	-@erase "$(INTDIR)\fileopts.sbr"
	-@erase "$(INTDIR)\filerecvdlg.obj"
	-@erase "$(INTDIR)\filerecvdlg.sbr"
	-@erase "$(INTDIR)\filesenddlg.obj"
	-@erase "$(INTDIR)\filesenddlg.sbr"
	-@erase "$(INTDIR)\filexferdlg.obj"
	-@erase "$(INTDIR)\filexferdlg.sbr"
	-@erase "$(INTDIR)\filter.obj"
	-@erase "$(INTDIR)\filter.sbr"
	-@erase "$(INTDIR)\findadd.obj"
	-@erase "$(INTDIR)\findadd.sbr"
	-@erase "$(INTDIR)\FontOptions.obj"
	-@erase "$(INTDIR)\FontOptions.sbr"
	-@erase "$(INTDIR)\FontService.obj"
	-@erase "$(INTDIR)\FontService.sbr"
	-@erase "$(INTDIR)\ftmanager.obj"
	-@erase "$(INTDIR)\ftmanager.sbr"
	-@erase "$(INTDIR)\genmenu.obj"
	-@erase "$(INTDIR)\genmenu.sbr"
	-@erase "$(INTDIR)\genmenuopt.obj"
	-@erase "$(INTDIR)\genmenuopt.sbr"
	-@erase "$(INTDIR)\groups.obj"
	-@erase "$(INTDIR)\groups.sbr"
	-@erase "$(INTDIR)\headerbar.obj"
	-@erase "$(INTDIR)\headerbar.sbr"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\help.sbr"
	-@erase "$(INTDIR)\history.obj"
	-@erase "$(INTDIR)\history.sbr"
	-@erase "$(INTDIR)\hotkeys.obj"
	-@erase "$(INTDIR)\hotkeys.sbr"
	-@erase "$(INTDIR)\hyperlink.obj"
	-@erase "$(INTDIR)\hyperlink.sbr"
	-@erase "$(INTDIR)\iconheader.obj"
	-@erase "$(INTDIR)\iconheader.sbr"
	-@erase "$(INTDIR)\idle.obj"
	-@erase "$(INTDIR)\idle.sbr"
	-@erase "$(INTDIR)\ignore.obj"
	-@erase "$(INTDIR)\ignore.sbr"
	-@erase "$(INTDIR)\imgconv.obj"
	-@erase "$(INTDIR)\imgconv.sbr"
	-@erase "$(INTDIR)\keyboard.obj"
	-@erase "$(INTDIR)\keyboard.sbr"
	-@erase "$(INTDIR)\langpack.obj"
	-@erase "$(INTDIR)\langpack.sbr"
	-@erase "$(INTDIR)\lpservices.obj"
	-@erase "$(INTDIR)\lpservices.sbr"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\md5.sbr"
	-@erase "$(INTDIR)\memory.obj"
	-@erase "$(INTDIR)\memory.sbr"
	-@erase "$(INTDIR)\miranda.obj"
	-@erase "$(INTDIR)\miranda.sbr"
	-@erase "$(INTDIR)\miranda32.pch"
	-@erase "$(INTDIR)\modules.obj"
	-@erase "$(INTDIR)\modules.sbr"
	-@erase "$(INTDIR)\movetogroup.obj"
	-@erase "$(INTDIR)\movetogroup.sbr"
	-@erase "$(INTDIR)\netlib.obj"
	-@erase "$(INTDIR)\netlib.sbr"
	-@erase "$(INTDIR)\netlibautoproxy.obj"
	-@erase "$(INTDIR)\netlibautoproxy.sbr"
	-@erase "$(INTDIR)\netlibbind.obj"
	-@erase "$(INTDIR)\netlibbind.sbr"
	-@erase "$(INTDIR)\netlibhttp.obj"
	-@erase "$(INTDIR)\netlibhttp.sbr"
	-@erase "$(INTDIR)\netlibhttpproxy.obj"
	-@erase "$(INTDIR)\netlibhttpproxy.sbr"
	-@erase "$(INTDIR)\netliblog.obj"
	-@erase "$(INTDIR)\netliblog.sbr"
	-@erase "$(INTDIR)\netlibopenconn.obj"
	-@erase "$(INTDIR)\netlibopenconn.sbr"
	-@erase "$(INTDIR)\netlibopts.obj"
	-@erase "$(INTDIR)\netlibopts.sbr"
	-@erase "$(INTDIR)\netlibpktrecver.obj"
	-@erase "$(INTDIR)\netlibpktrecver.sbr"
	-@erase "$(INTDIR)\netlibsecurity.obj"
	-@erase "$(INTDIR)\netlibsecurity.sbr"
	-@erase "$(INTDIR)\netlibsock.obj"
	-@erase "$(INTDIR)\netlibsock.sbr"
	-@erase "$(INTDIR)\netlibssl.obj"
	-@erase "$(INTDIR)\netlibssl.sbr"
	-@erase "$(INTDIR)\netlibupnp.obj"
	-@erase "$(INTDIR)\netlibupnp.sbr"
	-@erase "$(INTDIR)\newplugins.obj"
	-@erase "$(INTDIR)\newplugins.sbr"
	-@erase "$(INTDIR)\openurl.obj"
	-@erase "$(INTDIR)\openurl.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\path.obj"
	-@erase "$(INTDIR)\path.sbr"
	-@erase "$(INTDIR)\profilemanager.obj"
	-@erase "$(INTDIR)\profilemanager.sbr"
	-@erase "$(INTDIR)\protoaccs.obj"
	-@erase "$(INTDIR)\protoaccs.sbr"
	-@erase "$(INTDIR)\protochains.obj"
	-@erase "$(INTDIR)\protochains.sbr"
	-@erase "$(INTDIR)\protocolorder.obj"
	-@erase "$(INTDIR)\protocolorder.sbr"
	-@erase "$(INTDIR)\protocols.obj"
	-@erase "$(INTDIR)\protocols.sbr"
	-@erase "$(INTDIR)\protoint.obj"
	-@erase "$(INTDIR)\protoint.sbr"
	-@erase "$(INTDIR)\protoopts.obj"
	-@erase "$(INTDIR)\protoopts.sbr"
	-@erase "$(INTDIR)\resizer.obj"
	-@erase "$(INTDIR)\resizer.sbr"
	-@erase "$(INTDIR)\searchresults.obj"
	-@erase "$(INTDIR)\searchresults.sbr"
	-@erase "$(INTDIR)\sendmsg.obj"
	-@erase "$(INTDIR)\sendmsg.sbr"
	-@erase "$(INTDIR)\services.obj"
	-@erase "$(INTDIR)\services.sbr"
	-@erase "$(INTDIR)\sha1.obj"
	-@erase "$(INTDIR)\sha1.sbr"
	-@erase "$(INTDIR)\skin2icons.obj"
	-@erase "$(INTDIR)\skin2icons.sbr"
	-@erase "$(INTDIR)\skinicons.obj"
	-@erase "$(INTDIR)\skinicons.sbr"
	-@erase "$(INTDIR)\sounds.obj"
	-@erase "$(INTDIR)\sounds.sbr"
	-@erase "$(INTDIR)\stdinfo.obj"
	-@erase "$(INTDIR)\stdinfo.sbr"
	-@erase "$(INTDIR)\timeutils.obj"
	-@erase "$(INTDIR)\timeutils.sbr"
	-@erase "$(INTDIR)\timezones.obj"
	-@erase "$(INTDIR)\timezones.sbr"
	-@erase "$(INTDIR)\updatenotify.obj"
	-@erase "$(INTDIR)\updatenotify.sbr"
	-@erase "$(INTDIR)\url.obj"
	-@erase "$(INTDIR)\url.sbr"
	-@erase "$(INTDIR)\urldialogs.obj"
	-@erase "$(INTDIR)\urldialogs.sbr"
	-@erase "$(INTDIR)\userinfo.obj"
	-@erase "$(INTDIR)\userinfo.sbr"
	-@erase "$(INTDIR)\useronline.obj"
	-@erase "$(INTDIR)\useronline.sbr"
	-@erase "$(INTDIR)\utf.obj"
	-@erase "$(INTDIR)\utf.sbr"
	-@erase "$(INTDIR)\utils.obj"
	-@erase "$(INTDIR)\utils.sbr"
	-@erase "$(INTDIR)\vc6.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\visibility.obj"
	-@erase "$(INTDIR)\visibility.sbr"
	-@erase "$(INTDIR)\windowlist.obj"
	-@erase "$(INTDIR)\windowlist.sbr"
	-@erase "$(INTDIR)\xmlApi.obj"
	-@erase "$(INTDIR)\xmlApi.sbr"
	-@erase "$(INTDIR)\xmlParser.obj"
	-@erase "$(INTDIR)\xmlParser.sbr"
	-@erase "$(OUTDIR)\miranda32.bsc"
	-@erase "$(OUTDIR)\miranda32.pdb"
	-@erase "..\bin\Debug Unicode\miranda32.exe"
	-@erase "..\bin\Debug Unicode\miranda32.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /ZI /Od /I "../include" /I "../include/msapi" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "UNICODE" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\vc6.res" /i "../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\miranda32.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\commonheaders.sbr" \
	"$(INTDIR)\memory.sbr" \
	"$(INTDIR)\miranda.sbr" \
	"$(INTDIR)\modules.sbr" \
	"$(INTDIR)\addcontact.sbr" \
	"$(INTDIR)\autoaway.sbr" \
	"$(INTDIR)\button.sbr" \
	"$(INTDIR)\contacts.sbr" \
	"$(INTDIR)\database.sbr" \
	"$(INTDIR)\dbini.sbr" \
	"$(INTDIR)\dblists.sbr" \
	"$(INTDIR)\dbutils.sbr" \
	"$(INTDIR)\profilemanager.sbr" \
	"$(INTDIR)\findadd.sbr" \
	"$(INTDIR)\searchresults.sbr" \
	"$(INTDIR)\about.sbr" \
	"$(INTDIR)\help.sbr" \
	"$(INTDIR)\history.sbr" \
	"$(INTDIR)\idle.sbr" \
	"$(INTDIR)\ignore.sbr" \
	"$(INTDIR)\langpack.sbr" \
	"$(INTDIR)\lpservices.sbr" \
	"$(INTDIR)\netlib.sbr" \
	"$(INTDIR)\netlibautoproxy.sbr" \
	"$(INTDIR)\netlibbind.sbr" \
	"$(INTDIR)\netlibhttp.sbr" \
	"$(INTDIR)\netlibhttpproxy.sbr" \
	"$(INTDIR)\netliblog.sbr" \
	"$(INTDIR)\netlibopenconn.sbr" \
	"$(INTDIR)\netlibopts.sbr" \
	"$(INTDIR)\netlibpktrecver.sbr" \
	"$(INTDIR)\netlibsecurity.sbr" \
	"$(INTDIR)\netlibsock.sbr" \
	"$(INTDIR)\netlibssl.sbr" \
	"$(INTDIR)\netlibupnp.sbr" \
	"$(INTDIR)\descbutton.sbr" \
	"$(INTDIR)\filter.sbr" \
	"$(INTDIR)\headerbar.sbr" \
	"$(INTDIR)\iconheader.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\newplugins.sbr" \
	"$(INTDIR)\protoaccs.sbr" \
	"$(INTDIR)\protochains.sbr" \
	"$(INTDIR)\protocols.sbr" \
	"$(INTDIR)\protoint.sbr" \
	"$(INTDIR)\protoopts.sbr" \
	"$(INTDIR)\hotkeys.sbr" \
	"$(INTDIR)\skinicons.sbr" \
	"$(INTDIR)\sounds.sbr" \
	"$(INTDIR)\auth.sbr" \
	"$(INTDIR)\authdialogs.sbr" \
	"$(INTDIR)\awaymsg.sbr" \
	"$(INTDIR)\sendmsg.sbr" \
	"$(INTDIR)\email.sbr" \
	"$(INTDIR)\file.sbr" \
	"$(INTDIR)\fileexistsdlg.sbr" \
	"$(INTDIR)\fileopts.sbr" \
	"$(INTDIR)\filerecvdlg.sbr" \
	"$(INTDIR)\filesenddlg.sbr" \
	"$(INTDIR)\filexferdlg.sbr" \
	"$(INTDIR)\ftmanager.sbr" \
	"$(INTDIR)\url.sbr" \
	"$(INTDIR)\urldialogs.sbr" \
	"$(INTDIR)\contactinfo.sbr" \
	"$(INTDIR)\stdinfo.sbr" \
	"$(INTDIR)\userinfo.sbr" \
	"$(INTDIR)\useronline.sbr" \
	"$(INTDIR)\bmpfilter.sbr" \
	"$(INTDIR)\colourpicker.sbr" \
	"$(INTDIR)\hyperlink.sbr" \
	"$(INTDIR)\imgconv.sbr" \
	"$(INTDIR)\md5.sbr" \
	"$(INTDIR)\openurl.sbr" \
	"$(INTDIR)\path.sbr" \
	"$(INTDIR)\resizer.sbr" \
	"$(INTDIR)\sha1.sbr" \
	"$(INTDIR)\timeutils.sbr" \
	"$(INTDIR)\timezones.sbr" \
	"$(INTDIR)\utf.sbr" \
	"$(INTDIR)\utils.sbr" \
	"$(INTDIR)\windowlist.sbr" \
	"$(INTDIR)\visibility.sbr" \
	"$(INTDIR)\clc.sbr" \
	"$(INTDIR)\clcfiledrop.sbr" \
	"$(INTDIR)\clcidents.sbr" \
	"$(INTDIR)\clcitems.sbr" \
	"$(INTDIR)\clcmsgs.sbr" \
	"$(INTDIR)\clcutils.sbr" \
	"$(INTDIR)\clistcore.sbr" \
	"$(INTDIR)\clistevents.sbr" \
	"$(INTDIR)\clistmenus.sbr" \
	"$(INTDIR)\clistmod.sbr" \
	"$(INTDIR)\clistsettings.sbr" \
	"$(INTDIR)\clisttray.sbr" \
	"$(INTDIR)\clui.sbr" \
	"$(INTDIR)\cluiservices.sbr" \
	"$(INTDIR)\contact.sbr" \
	"$(INTDIR)\Docking.sbr" \
	"$(INTDIR)\genmenu.sbr" \
	"$(INTDIR)\genmenuopt.sbr" \
	"$(INTDIR)\groups.sbr" \
	"$(INTDIR)\keyboard.sbr" \
	"$(INTDIR)\movetogroup.sbr" \
	"$(INTDIR)\protocolorder.sbr" \
	"$(INTDIR)\FontOptions.sbr" \
	"$(INTDIR)\FontService.sbr" \
	"$(INTDIR)\services.sbr" \
	"$(INTDIR)\extracticon.sbr" \
	"$(INTDIR)\skin2icons.sbr" \
	"$(INTDIR)\updatenotify.sbr" \
	"$(INTDIR)\xmlApi.sbr" \
	"$(INTDIR)\xmlParser.sbr"

"$(OUTDIR)\miranda32.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=ws2_32.lib kernel32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib comctl32.lib winmm.lib version.lib crypt32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\miranda32.pdb" /debug /machine:I386 /out:"../bin/Debug Unicode/miranda32.exe" 
LINK32_OBJS= \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\memory.obj" \
	"$(INTDIR)\miranda.obj" \
	"$(INTDIR)\modules.obj" \
	"$(INTDIR)\addcontact.obj" \
	"$(INTDIR)\autoaway.obj" \
	"$(INTDIR)\button.obj" \
	"$(INTDIR)\contacts.obj" \
	"$(INTDIR)\database.obj" \
	"$(INTDIR)\dbini.obj" \
	"$(INTDIR)\dblists.obj" \
	"$(INTDIR)\dbutils.obj" \
	"$(INTDIR)\profilemanager.obj" \
	"$(INTDIR)\findadd.obj" \
	"$(INTDIR)\searchresults.obj" \
	"$(INTDIR)\about.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\history.obj" \
	"$(INTDIR)\idle.obj" \
	"$(INTDIR)\ignore.obj" \
	"$(INTDIR)\langpack.obj" \
	"$(INTDIR)\lpservices.obj" \
	"$(INTDIR)\netlib.obj" \
	"$(INTDIR)\netlibautoproxy.obj" \
	"$(INTDIR)\netlibbind.obj" \
	"$(INTDIR)\netlibhttp.obj" \
	"$(INTDIR)\netlibhttpproxy.obj" \
	"$(INTDIR)\netliblog.obj" \
	"$(INTDIR)\netlibopenconn.obj" \
	"$(INTDIR)\netlibopts.obj" \
	"$(INTDIR)\netlibpktrecver.obj" \
	"$(INTDIR)\netlibsecurity.obj" \
	"$(INTDIR)\netlibsock.obj" \
	"$(INTDIR)\netlibssl.obj" \
	"$(INTDIR)\netlibupnp.obj" \
	"$(INTDIR)\descbutton.obj" \
	"$(INTDIR)\filter.obj" \
	"$(INTDIR)\headerbar.obj" \
	"$(INTDIR)\iconheader.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\newplugins.obj" \
	"$(INTDIR)\protoaccs.obj" \
	"$(INTDIR)\protochains.obj" \
	"$(INTDIR)\protocols.obj" \
	"$(INTDIR)\protoint.obj" \
	"$(INTDIR)\protoopts.obj" \
	"$(INTDIR)\hotkeys.obj" \
	"$(INTDIR)\skinicons.obj" \
	"$(INTDIR)\sounds.obj" \
	"$(INTDIR)\auth.obj" \
	"$(INTDIR)\authdialogs.obj" \
	"$(INTDIR)\awaymsg.obj" \
	"$(INTDIR)\sendmsg.obj" \
	"$(INTDIR)\email.obj" \
	"$(INTDIR)\file.obj" \
	"$(INTDIR)\fileexistsdlg.obj" \
	"$(INTDIR)\fileopts.obj" \
	"$(INTDIR)\filerecvdlg.obj" \
	"$(INTDIR)\filesenddlg.obj" \
	"$(INTDIR)\filexferdlg.obj" \
	"$(INTDIR)\ftmanager.obj" \
	"$(INTDIR)\url.obj" \
	"$(INTDIR)\urldialogs.obj" \
	"$(INTDIR)\contactinfo.obj" \
	"$(INTDIR)\stdinfo.obj" \
	"$(INTDIR)\userinfo.obj" \
	"$(INTDIR)\useronline.obj" \
	"$(INTDIR)\bmpfilter.obj" \
	"$(INTDIR)\colourpicker.obj" \
	"$(INTDIR)\hyperlink.obj" \
	"$(INTDIR)\imgconv.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\openurl.obj" \
	"$(INTDIR)\path.obj" \
	"$(INTDIR)\resizer.obj" \
	"$(INTDIR)\sha1.obj" \
	"$(INTDIR)\timeutils.obj" \
	"$(INTDIR)\timezones.obj" \
	"$(INTDIR)\utf.obj" \
	"$(INTDIR)\utils.obj" \
	"$(INTDIR)\windowlist.obj" \
	"$(INTDIR)\visibility.obj" \
	"$(INTDIR)\clc.obj" \
	"$(INTDIR)\clcfiledrop.obj" \
	"$(INTDIR)\clcidents.obj" \
	"$(INTDIR)\clcitems.obj" \
	"$(INTDIR)\clcmsgs.obj" \
	"$(INTDIR)\clcutils.obj" \
	"$(INTDIR)\clistcore.obj" \
	"$(INTDIR)\clistevents.obj" \
	"$(INTDIR)\clistmenus.obj" \
	"$(INTDIR)\clistmod.obj" \
	"$(INTDIR)\clistsettings.obj" \
	"$(INTDIR)\clisttray.obj" \
	"$(INTDIR)\clui.obj" \
	"$(INTDIR)\cluiservices.obj" \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\Docking.obj" \
	"$(INTDIR)\genmenu.obj" \
	"$(INTDIR)\genmenuopt.obj" \
	"$(INTDIR)\groups.obj" \
	"$(INTDIR)\keyboard.obj" \
	"$(INTDIR)\movetogroup.obj" \
	"$(INTDIR)\protocolorder.obj" \
	"$(INTDIR)\FontOptions.obj" \
	"$(INTDIR)\FontService.obj" \
	"$(INTDIR)\services.obj" \
	"$(INTDIR)\extracticon.obj" \
	"$(INTDIR)\skin2icons.obj" \
	"$(INTDIR)\updatenotify.obj" \
	"$(INTDIR)\xmlApi.obj" \
	"$(INTDIR)\xmlParser.obj" \
	"$(INTDIR)\vc6.res" \
	"..\plugins\zlib\Debug_Unicode\zlib.lib"

"..\bin\Debug Unicode\miranda32.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("miranda32.dep")
!INCLUDE "miranda32.dep"
!ELSE 
!MESSAGE Warning: cannot find "miranda32.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "miranda32 - Win32 Release" || "$(CFG)" == "miranda32 - Win32 Debug" || "$(CFG)" == "miranda32 - Win32 Release Unicode" || "$(CFG)" == "miranda32 - Win32 Debug Unicode"
SOURCE=.\core\commonheaders.cpp

!IF  "$(CFG)" == "miranda32 - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../include" /I "../include/msapi" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\commonheaders.sbr"	"$(INTDIR)\miranda32.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "miranda32 - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../include" /I "../include/msapi" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\commonheaders.sbr"	"$(INTDIR)\miranda32.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "miranda32 - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../include/msapi" /I "../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "UNICODE" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\commonheaders.sbr"	"$(INTDIR)\miranda32.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "miranda32 - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../include" /I "../include/msapi" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "UNICODE" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\commonheaders.sbr"	"$(INTDIR)\miranda32.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\core\memory.cpp

"$(INTDIR)\memory.obj"	"$(INTDIR)\memory.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\core\miranda.cpp

"$(INTDIR)\miranda.obj"	"$(INTDIR)\miranda.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\core\modules.cpp

"$(INTDIR)\modules.obj"	"$(INTDIR)\modules.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\addcontact\addcontact.cpp

"$(INTDIR)\addcontact.obj"	"$(INTDIR)\addcontact.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\autoaway\autoaway.cpp

"$(INTDIR)\autoaway.obj"	"$(INTDIR)\autoaway.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\button\button.cpp

"$(INTDIR)\button.obj"	"$(INTDIR)\button.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\contacts\contacts.cpp

"$(INTDIR)\contacts.obj"	"$(INTDIR)\contacts.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\database\database.cpp

"$(INTDIR)\database.obj"	"$(INTDIR)\database.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\database\dbini.cpp

"$(INTDIR)\dbini.obj"	"$(INTDIR)\dbini.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\database\dblists.cpp

"$(INTDIR)\dblists.obj"	"$(INTDIR)\dblists.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\database\dbutils.cpp

"$(INTDIR)\dbutils.obj"	"$(INTDIR)\dbutils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\database\profilemanager.cpp

"$(INTDIR)\profilemanager.obj"	"$(INTDIR)\profilemanager.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\findadd\findadd.cpp

"$(INTDIR)\findadd.obj"	"$(INTDIR)\findadd.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\findadd\searchresults.cpp

"$(INTDIR)\searchresults.obj"	"$(INTDIR)\searchresults.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\help\about.cpp

"$(INTDIR)\about.obj"	"$(INTDIR)\about.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\help\help.cpp

"$(INTDIR)\help.obj"	"$(INTDIR)\help.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\history\history.cpp

"$(INTDIR)\history.obj"	"$(INTDIR)\history.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\idle\idle.cpp

"$(INTDIR)\idle.obj"	"$(INTDIR)\idle.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\ignore\ignore.cpp

"$(INTDIR)\ignore.obj"	"$(INTDIR)\ignore.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\langpack\langpack.cpp

"$(INTDIR)\langpack.obj"	"$(INTDIR)\langpack.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\langpack\lpservices.cpp

"$(INTDIR)\lpservices.obj"	"$(INTDIR)\lpservices.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlib.cpp

"$(INTDIR)\netlib.obj"	"$(INTDIR)\netlib.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibautoproxy.cpp

"$(INTDIR)\netlibautoproxy.obj"	"$(INTDIR)\netlibautoproxy.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibbind.cpp

"$(INTDIR)\netlibbind.obj"	"$(INTDIR)\netlibbind.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibhttp.cpp

"$(INTDIR)\netlibhttp.obj"	"$(INTDIR)\netlibhttp.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibhttpproxy.cpp

"$(INTDIR)\netlibhttpproxy.obj"	"$(INTDIR)\netlibhttpproxy.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netliblog.cpp

"$(INTDIR)\netliblog.obj"	"$(INTDIR)\netliblog.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibopenconn.cpp

"$(INTDIR)\netlibopenconn.obj"	"$(INTDIR)\netlibopenconn.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibopts.cpp

"$(INTDIR)\netlibopts.obj"	"$(INTDIR)\netlibopts.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibpktrecver.cpp

"$(INTDIR)\netlibpktrecver.obj"	"$(INTDIR)\netlibpktrecver.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibsecurity.cpp

"$(INTDIR)\netlibsecurity.obj"	"$(INTDIR)\netlibsecurity.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibsock.cpp

"$(INTDIR)\netlibsock.obj"	"$(INTDIR)\netlibsock.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibssl.cpp

"$(INTDIR)\netlibssl.obj"	"$(INTDIR)\netlibssl.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\netlib\netlibupnp.cpp

"$(INTDIR)\netlibupnp.obj"	"$(INTDIR)\netlibupnp.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\options\descbutton.cpp

"$(INTDIR)\descbutton.obj"	"$(INTDIR)\descbutton.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\options\filter.cpp

"$(INTDIR)\filter.obj"	"$(INTDIR)\filter.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\options\headerbar.cpp

"$(INTDIR)\headerbar.obj"	"$(INTDIR)\headerbar.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\options\iconheader.cpp

"$(INTDIR)\iconheader.obj"	"$(INTDIR)\iconheader.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\options\options.cpp

"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\plugins\newplugins.cpp

"$(INTDIR)\newplugins.obj"	"$(INTDIR)\newplugins.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\protocols\protoaccs.cpp

"$(INTDIR)\protoaccs.obj"	"$(INTDIR)\protoaccs.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\protocols\protochains.cpp

"$(INTDIR)\protochains.obj"	"$(INTDIR)\protochains.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\protocols\protocols.cpp

"$(INTDIR)\protocols.obj"	"$(INTDIR)\protocols.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\protocols\protoint.cpp

"$(INTDIR)\protoint.obj"	"$(INTDIR)\protoint.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\protocols\protoopts.cpp

"$(INTDIR)\protoopts.obj"	"$(INTDIR)\protoopts.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\skin\hotkeys.cpp

"$(INTDIR)\hotkeys.obj"	"$(INTDIR)\hotkeys.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\skin\skinicons.cpp

"$(INTDIR)\skinicons.obj"	"$(INTDIR)\skinicons.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\skin\sounds.cpp

"$(INTDIR)\sounds.obj"	"$(INTDIR)\sounds.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srauth\auth.cpp

"$(INTDIR)\auth.obj"	"$(INTDIR)\auth.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srauth\authdialogs.cpp

"$(INTDIR)\authdialogs.obj"	"$(INTDIR)\authdialogs.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srawaymsg\awaymsg.cpp

"$(INTDIR)\awaymsg.obj"	"$(INTDIR)\awaymsg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srawaymsg\sendmsg.cpp

"$(INTDIR)\sendmsg.obj"	"$(INTDIR)\sendmsg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\sremail\email.cpp

"$(INTDIR)\email.obj"	"$(INTDIR)\email.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srfile\file.cpp

"$(INTDIR)\file.obj"	"$(INTDIR)\file.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srfile\fileexistsdlg.cpp

"$(INTDIR)\fileexistsdlg.obj"	"$(INTDIR)\fileexistsdlg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srfile\fileopts.cpp

"$(INTDIR)\fileopts.obj"	"$(INTDIR)\fileopts.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srfile\filerecvdlg.cpp

"$(INTDIR)\filerecvdlg.obj"	"$(INTDIR)\filerecvdlg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srfile\filesenddlg.cpp

"$(INTDIR)\filesenddlg.obj"	"$(INTDIR)\filesenddlg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srfile\filexferdlg.cpp

"$(INTDIR)\filexferdlg.obj"	"$(INTDIR)\filexferdlg.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srfile\ftmanager.cpp

"$(INTDIR)\ftmanager.obj"	"$(INTDIR)\ftmanager.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srurl\url.cpp

"$(INTDIR)\url.obj"	"$(INTDIR)\url.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\srurl\urldialogs.cpp

"$(INTDIR)\urldialogs.obj"	"$(INTDIR)\urldialogs.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\userinfo\contactinfo.cpp

"$(INTDIR)\contactinfo.obj"	"$(INTDIR)\contactinfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\userinfo\stdinfo.cpp

"$(INTDIR)\stdinfo.obj"	"$(INTDIR)\stdinfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\userinfo\userinfo.cpp

"$(INTDIR)\userinfo.obj"	"$(INTDIR)\userinfo.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\useronline\useronline.cpp

"$(INTDIR)\useronline.obj"	"$(INTDIR)\useronline.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\bmpfilter.cpp

"$(INTDIR)\bmpfilter.obj"	"$(INTDIR)\bmpfilter.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\colourpicker.cpp

"$(INTDIR)\colourpicker.obj"	"$(INTDIR)\colourpicker.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\hyperlink.cpp

"$(INTDIR)\hyperlink.obj"	"$(INTDIR)\hyperlink.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\imgconv.cpp

"$(INTDIR)\imgconv.obj"	"$(INTDIR)\imgconv.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\md5.cpp

"$(INTDIR)\md5.obj"	"$(INTDIR)\md5.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\openurl.cpp

"$(INTDIR)\openurl.obj"	"$(INTDIR)\openurl.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\path.cpp

"$(INTDIR)\path.obj"	"$(INTDIR)\path.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\resizer.cpp

"$(INTDIR)\resizer.obj"	"$(INTDIR)\resizer.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\sha1.cpp

"$(INTDIR)\sha1.obj"	"$(INTDIR)\sha1.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\timeutils.cpp

"$(INTDIR)\timeutils.obj"	"$(INTDIR)\timeutils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\timezones.cpp

"$(INTDIR)\timezones.obj"	"$(INTDIR)\timezones.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\utf.cpp

"$(INTDIR)\utf.obj"	"$(INTDIR)\utf.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\utils.cpp

"$(INTDIR)\utils.obj"	"$(INTDIR)\utils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\utils\windowlist.cpp

"$(INTDIR)\windowlist.obj"	"$(INTDIR)\windowlist.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\visibility\visibility.cpp

"$(INTDIR)\visibility.obj"	"$(INTDIR)\visibility.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clc.cpp

"$(INTDIR)\clc.obj"	"$(INTDIR)\clc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clcfiledrop.cpp

"$(INTDIR)\clcfiledrop.obj"	"$(INTDIR)\clcfiledrop.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clcidents.cpp

"$(INTDIR)\clcidents.obj"	"$(INTDIR)\clcidents.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clcitems.cpp

"$(INTDIR)\clcitems.obj"	"$(INTDIR)\clcitems.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clcmsgs.cpp

"$(INTDIR)\clcmsgs.obj"	"$(INTDIR)\clcmsgs.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clcutils.cpp

"$(INTDIR)\clcutils.obj"	"$(INTDIR)\clcutils.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clistcore.cpp

"$(INTDIR)\clistcore.obj"	"$(INTDIR)\clistcore.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clistevents.cpp

"$(INTDIR)\clistevents.obj"	"$(INTDIR)\clistevents.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clistmenus.cpp

"$(INTDIR)\clistmenus.obj"	"$(INTDIR)\clistmenus.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clistmod.cpp

"$(INTDIR)\clistmod.obj"	"$(INTDIR)\clistmod.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clistsettings.cpp

"$(INTDIR)\clistsettings.obj"	"$(INTDIR)\clistsettings.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clisttray.cpp

"$(INTDIR)\clisttray.obj"	"$(INTDIR)\clisttray.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\clui.cpp

"$(INTDIR)\clui.obj"	"$(INTDIR)\clui.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\cluiservices.cpp

"$(INTDIR)\cluiservices.obj"	"$(INTDIR)\cluiservices.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\contact.cpp

"$(INTDIR)\contact.obj"	"$(INTDIR)\contact.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\Docking.cpp

"$(INTDIR)\Docking.obj"	"$(INTDIR)\Docking.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\genmenu.cpp

"$(INTDIR)\genmenu.obj"	"$(INTDIR)\genmenu.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\genmenuopt.cpp

"$(INTDIR)\genmenuopt.obj"	"$(INTDIR)\genmenuopt.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\groups.cpp

"$(INTDIR)\groups.obj"	"$(INTDIR)\groups.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\keyboard.cpp

"$(INTDIR)\keyboard.obj"	"$(INTDIR)\keyboard.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\movetogroup.cpp

"$(INTDIR)\movetogroup.obj"	"$(INTDIR)\movetogroup.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\clist\protocolorder.cpp

"$(INTDIR)\protocolorder.obj"	"$(INTDIR)\protocolorder.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\fonts\FontOptions.cpp

"$(INTDIR)\FontOptions.obj"	"$(INTDIR)\FontOptions.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\fonts\FontService.cpp

"$(INTDIR)\FontService.obj"	"$(INTDIR)\FontService.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\fonts\services.cpp

"$(INTDIR)\services.obj"	"$(INTDIR)\services.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\icolib\extracticon.cpp

!IF  "$(CFG)" == "miranda32 - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../include" /I "../include/msapi" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\extracticon.obj"	"$(INTDIR)\extracticon.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "miranda32 - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../include" /I "../include/msapi" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\extracticon.obj"	"$(INTDIR)\extracticon.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "miranda32 - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /I "../include/msapi" /I "../include" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "UNICODE" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\extracticon.obj"	"$(INTDIR)\extracticon.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "miranda32 - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /ZI /Od /I "../include" /I "../include/msapi" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_NOSDK" /D "UNICODE" /D "_STATIC" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\miranda32.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\extracticon.obj"	"$(INTDIR)\extracticon.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\modules\icolib\skin2icons.cpp

"$(INTDIR)\skin2icons.obj"	"$(INTDIR)\skin2icons.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\updatenotify\updatenotify.cpp

"$(INTDIR)\updatenotify.obj"	"$(INTDIR)\updatenotify.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\xml\xmlApi.cpp

"$(INTDIR)\xmlApi.obj"	"$(INTDIR)\xmlApi.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\modules\xml\xmlParser.cpp

"$(INTDIR)\xmlParser.obj"	"$(INTDIR)\xmlParser.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\miranda32.pch"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\vc6.rc

"$(INTDIR)\vc6.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!IF  "$(CFG)" == "miranda32 - Win32 Release"

"zlib - Win32 Release" : 
   cd "..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release" 
   cd "..\..\src"

"zlib - Win32 ReleaseCLEAN" : 
   cd "..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release" RECURSE=1 CLEAN 
   cd "..\..\src"

!ELSEIF  "$(CFG)" == "miranda32 - Win32 Debug"

"zlib - Win32 Debug" : 
   cd "..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug" 
   cd "..\..\src"

"zlib - Win32 DebugCLEAN" : 
   cd "..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\..\src"

!ELSEIF  "$(CFG)" == "miranda32 - Win32 Release Unicode"

"zlib - Win32 Release Unicode" : 
   cd "..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release Unicode" 
   cd "..\..\src"

"zlib - Win32 Release UnicodeCLEAN" : 
   cd "..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Release Unicode" RECURSE=1 CLEAN 
   cd "..\..\src"

!ELSEIF  "$(CFG)" == "miranda32 - Win32 Debug Unicode"

"zlib - Win32 Debug Unicode" : 
   cd "..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug Unicode" 
   cd "..\..\src"

"zlib - Win32 Debug UnicodeCLEAN" : 
   cd "..\plugins\zlib"
   $(MAKE) /$(MAKEFLAGS) /F .\zlib.mak CFG="zlib - Win32 Debug Unicode" RECURSE=1 CLEAN 
   cd "..\..\src"

!ENDIF 


!ENDIF 

