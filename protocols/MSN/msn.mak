# Microsoft Developer Studio Generated NMAKE File, Based on msn.dsp
!IF "$(CFG)" == ""
CFG=msn - Win32 Release Unicode
!MESSAGE No configuration specified. Defaulting to msn - Win32 Release Unicode.
!ENDIF

!IF "$(CFG)" != "msn - Win32 Release" && "$(CFG)" != "msn - Win32 Debug" && "$(CFG)" != "msn - Win32 Release Unicode" && "$(CFG)" != "msn - Win32 Debug Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "msn.mak" CFG="msn - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "msn - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "msn - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "msn - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "msn - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "msn - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "..\..\bin\release\plugins\msn.dll" "$(OUTDIR)\msn.pch"


CLEAN :
	-@erase "$(INTDIR)\des.obj"
	-@erase "$(INTDIR)\ezxml.obj"
	-@erase "$(INTDIR)\msn.obj"
	-@erase "$(INTDIR)\msn.pch"
	-@erase "$(INTDIR)\msn_auth.obj"
	-@erase "$(INTDIR)\msn_chat.obj"
	-@erase "$(INTDIR)\msn_commands.obj"
	-@erase "$(INTDIR)\msn_contact.obj"
	-@erase "$(INTDIR)\msn_errors.obj"
	-@erase "$(INTDIR)\msn_ftold.obj"
	-@erase "$(INTDIR)\msn_http.obj"
	-@erase "$(INTDIR)\msn_libstr.obj"
	-@erase "$(INTDIR)\msn_links.obj"
	-@erase "$(INTDIR)\msn_lists.obj"
	-@erase "$(INTDIR)\msn_mail.obj"
	-@erase "$(INTDIR)\msn_menu.obj"
	-@erase "$(INTDIR)\msn_mime.obj"
	-@erase "$(INTDIR)\msn_misc.obj"
	-@erase "$(INTDIR)\msn_msgqueue.obj"
	-@erase "$(INTDIR)\msn_msgsplit.obj"
	-@erase "$(INTDIR)\msn_natdetect.obj"
	-@erase "$(INTDIR)\msn_opts.obj"
	-@erase "$(INTDIR)\msn_p2p.obj"
	-@erase "$(INTDIR)\msn_p2ps.obj"
	-@erase "$(INTDIR)\msn_proto.obj"
	-@erase "$(INTDIR)\msn_soapab.obj"
	-@erase "$(INTDIR)\msn_soapstore.obj"
	-@erase "$(INTDIR)\msn_srv.obj"
	-@erase "$(INTDIR)\msn_ssl.obj"
	-@erase "$(INTDIR)\msn_std.obj"
	-@erase "$(INTDIR)\msn_svcs.obj"
	-@erase "$(INTDIR)\msn_switchboard.obj"
	-@erase "$(INTDIR)\msn_threads.obj"
	-@erase "$(INTDIR)\msn_useropts.obj"
	-@erase "$(INTDIR)\msn_ws.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\msn.exp"
	-@erase "$(OUTDIR)\msn.map"
	-@erase "$(OUTDIR)\msn.pdb"
	-@erase "..\..\bin\release\plugins\msn.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\msn.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /base:"0x19000000" /dll /incremental:no /pdb:"$(OUTDIR)\msn.pdb" /map:"$(INTDIR)\msn.map" /debug /machine:I386 /out:"../../bin/release/plugins/msn.dll" /implib:"$(OUTDIR)\msn.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\des.obj" \
	"$(INTDIR)\ezxml.obj" \
	"$(INTDIR)\msn.obj" \
	"$(INTDIR)\msn_auth.obj" \
	"$(INTDIR)\msn_chat.obj" \
	"$(INTDIR)\msn_commands.obj" \
	"$(INTDIR)\msn_contact.obj" \
	"$(INTDIR)\msn_errors.obj" \
	"$(INTDIR)\msn_ftold.obj" \
	"$(INTDIR)\msn_http.obj" \
	"$(INTDIR)\msn_libstr.obj" \
	"$(INTDIR)\msn_links.obj" \
	"$(INTDIR)\msn_lists.obj" \
	"$(INTDIR)\msn_mail.obj" \
	"$(INTDIR)\msn_menu.obj" \
	"$(INTDIR)\msn_mime.obj" \
	"$(INTDIR)\msn_misc.obj" \
	"$(INTDIR)\msn_msgqueue.obj" \
	"$(INTDIR)\msn_msgsplit.obj" \
	"$(INTDIR)\msn_natdetect.obj" \
	"$(INTDIR)\msn_opts.obj" \
	"$(INTDIR)\msn_p2p.obj" \
	"$(INTDIR)\msn_p2ps.obj" \
	"$(INTDIR)\msn_proto.obj" \
	"$(INTDIR)\msn_soapab.obj" \
	"$(INTDIR)\msn_soapstore.obj" \
	"$(INTDIR)\msn_srv.obj" \
	"$(INTDIR)\msn_ssl.obj" \
	"$(INTDIR)\msn_std.obj" \
	"$(INTDIR)\msn_svcs.obj" \
	"$(INTDIR)\msn_switchboard.obj" \
	"$(INTDIR)\msn_threads.obj" \
	"$(INTDIR)\msn_useropts.obj" \
	"$(INTDIR)\msn_ws.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\release\plugins\msn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "msn - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\bin\debug\plugins\msn.dll" "$(OUTDIR)\msn.pch" "$(OUTDIR)\msn.bsc"


CLEAN :
	-@erase "$(INTDIR)\des.obj"
	-@erase "$(INTDIR)\des.sbr"
	-@erase "$(INTDIR)\ezxml.obj"
	-@erase "$(INTDIR)\ezxml.sbr"
	-@erase "$(INTDIR)\msn.obj"
	-@erase "$(INTDIR)\msn.pch"
	-@erase "$(INTDIR)\msn.sbr"
	-@erase "$(INTDIR)\msn_auth.obj"
	-@erase "$(INTDIR)\msn_auth.sbr"
	-@erase "$(INTDIR)\msn_chat.obj"
	-@erase "$(INTDIR)\msn_chat.sbr"
	-@erase "$(INTDIR)\msn_commands.obj"
	-@erase "$(INTDIR)\msn_commands.sbr"
	-@erase "$(INTDIR)\msn_contact.obj"
	-@erase "$(INTDIR)\msn_contact.sbr"
	-@erase "$(INTDIR)\msn_errors.obj"
	-@erase "$(INTDIR)\msn_errors.sbr"
	-@erase "$(INTDIR)\msn_ftold.obj"
	-@erase "$(INTDIR)\msn_ftold.sbr"
	-@erase "$(INTDIR)\msn_http.obj"
	-@erase "$(INTDIR)\msn_http.sbr"
	-@erase "$(INTDIR)\msn_libstr.obj"
	-@erase "$(INTDIR)\msn_libstr.sbr"
	-@erase "$(INTDIR)\msn_links.obj"
	-@erase "$(INTDIR)\msn_links.sbr"
	-@erase "$(INTDIR)\msn_lists.obj"
	-@erase "$(INTDIR)\msn_lists.sbr"
	-@erase "$(INTDIR)\msn_mail.obj"
	-@erase "$(INTDIR)\msn_mail.sbr"
	-@erase "$(INTDIR)\msn_menu.obj"
	-@erase "$(INTDIR)\msn_menu.sbr"
	-@erase "$(INTDIR)\msn_mime.obj"
	-@erase "$(INTDIR)\msn_mime.sbr"
	-@erase "$(INTDIR)\msn_misc.obj"
	-@erase "$(INTDIR)\msn_misc.sbr"
	-@erase "$(INTDIR)\msn_msgqueue.obj"
	-@erase "$(INTDIR)\msn_msgqueue.sbr"
	-@erase "$(INTDIR)\msn_msgsplit.obj"
	-@erase "$(INTDIR)\msn_msgsplit.sbr"
	-@erase "$(INTDIR)\msn_natdetect.obj"
	-@erase "$(INTDIR)\msn_natdetect.sbr"
	-@erase "$(INTDIR)\msn_opts.obj"
	-@erase "$(INTDIR)\msn_opts.sbr"
	-@erase "$(INTDIR)\msn_p2p.obj"
	-@erase "$(INTDIR)\msn_p2p.sbr"
	-@erase "$(INTDIR)\msn_p2ps.obj"
	-@erase "$(INTDIR)\msn_p2ps.sbr"
	-@erase "$(INTDIR)\msn_proto.obj"
	-@erase "$(INTDIR)\msn_proto.sbr"
	-@erase "$(INTDIR)\msn_soapab.obj"
	-@erase "$(INTDIR)\msn_soapab.sbr"
	-@erase "$(INTDIR)\msn_soapstore.obj"
	-@erase "$(INTDIR)\msn_soapstore.sbr"
	-@erase "$(INTDIR)\msn_srv.obj"
	-@erase "$(INTDIR)\msn_srv.sbr"
	-@erase "$(INTDIR)\msn_ssl.obj"
	-@erase "$(INTDIR)\msn_ssl.sbr"
	-@erase "$(INTDIR)\msn_std.obj"
	-@erase "$(INTDIR)\msn_std.sbr"
	-@erase "$(INTDIR)\msn_svcs.obj"
	-@erase "$(INTDIR)\msn_svcs.sbr"
	-@erase "$(INTDIR)\msn_switchboard.obj"
	-@erase "$(INTDIR)\msn_switchboard.sbr"
	-@erase "$(INTDIR)\msn_threads.obj"
	-@erase "$(INTDIR)\msn_threads.sbr"
	-@erase "$(INTDIR)\msn_useropts.obj"
	-@erase "$(INTDIR)\msn_useropts.sbr"
	-@erase "$(INTDIR)\msn_ws.obj"
	-@erase "$(INTDIR)\msn_ws.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\msn.bsc"
	-@erase "$(OUTDIR)\msn.exp"
	-@erase "$(OUTDIR)\msn.pdb"
	-@erase "..\..\bin\debug\plugins\msn.dll"
	-@erase "..\..\bin\debug\plugins\msn.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\msn.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\des.sbr" \
	"$(INTDIR)\ezxml.sbr" \
	"$(INTDIR)\msn.sbr" \
	"$(INTDIR)\msn_auth.sbr" \
	"$(INTDIR)\msn_chat.sbr" \
	"$(INTDIR)\msn_commands.sbr" \
	"$(INTDIR)\msn_contact.sbr" \
	"$(INTDIR)\msn_errors.sbr" \
	"$(INTDIR)\msn_ftold.sbr" \
	"$(INTDIR)\msn_http.sbr" \
	"$(INTDIR)\msn_libstr.sbr" \
	"$(INTDIR)\msn_links.sbr" \
	"$(INTDIR)\msn_lists.sbr" \
	"$(INTDIR)\msn_mail.sbr" \
	"$(INTDIR)\msn_menu.sbr" \
	"$(INTDIR)\msn_mime.sbr" \
	"$(INTDIR)\msn_misc.sbr" \
	"$(INTDIR)\msn_msgqueue.sbr" \
	"$(INTDIR)\msn_msgsplit.sbr" \
	"$(INTDIR)\msn_natdetect.sbr" \
	"$(INTDIR)\msn_opts.sbr" \
	"$(INTDIR)\msn_p2p.sbr" \
	"$(INTDIR)\msn_p2ps.sbr" \
	"$(INTDIR)\msn_proto.sbr" \
	"$(INTDIR)\msn_soapab.sbr" \
	"$(INTDIR)\msn_soapstore.sbr" \
	"$(INTDIR)\msn_srv.sbr" \
	"$(INTDIR)\msn_ssl.sbr" \
	"$(INTDIR)\msn_std.sbr" \
	"$(INTDIR)\msn_svcs.sbr" \
	"$(INTDIR)\msn_switchboard.sbr" \
	"$(INTDIR)\msn_threads.sbr" \
	"$(INTDIR)\msn_useropts.sbr" \
	"$(INTDIR)\msn_ws.sbr"

"$(OUTDIR)\msn.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\msn.pdb" /debug /machine:I386 /out:"../../bin/debug/plugins/msn.dll" /implib:"$(OUTDIR)\msn.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\des.obj" \
	"$(INTDIR)\ezxml.obj" \
	"$(INTDIR)\msn.obj" \
	"$(INTDIR)\msn_auth.obj" \
	"$(INTDIR)\msn_chat.obj" \
	"$(INTDIR)\msn_commands.obj" \
	"$(INTDIR)\msn_contact.obj" \
	"$(INTDIR)\msn_errors.obj" \
	"$(INTDIR)\msn_ftold.obj" \
	"$(INTDIR)\msn_http.obj" \
	"$(INTDIR)\msn_libstr.obj" \
	"$(INTDIR)\msn_links.obj" \
	"$(INTDIR)\msn_lists.obj" \
	"$(INTDIR)\msn_mail.obj" \
	"$(INTDIR)\msn_menu.obj" \
	"$(INTDIR)\msn_mime.obj" \
	"$(INTDIR)\msn_misc.obj" \
	"$(INTDIR)\msn_msgqueue.obj" \
	"$(INTDIR)\msn_msgsplit.obj" \
	"$(INTDIR)\msn_natdetect.obj" \
	"$(INTDIR)\msn_opts.obj" \
	"$(INTDIR)\msn_p2p.obj" \
	"$(INTDIR)\msn_p2ps.obj" \
	"$(INTDIR)\msn_proto.obj" \
	"$(INTDIR)\msn_soapab.obj" \
	"$(INTDIR)\msn_soapstore.obj" \
	"$(INTDIR)\msn_srv.obj" \
	"$(INTDIR)\msn_ssl.obj" \
	"$(INTDIR)\msn_std.obj" \
	"$(INTDIR)\msn_svcs.obj" \
	"$(INTDIR)\msn_switchboard.obj" \
	"$(INTDIR)\msn_threads.obj" \
	"$(INTDIR)\msn_useropts.obj" \
	"$(INTDIR)\msn_ws.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\debug\plugins\msn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\Release Unicode\plugins\msn.dll"


CLEAN :
	-@erase "$(INTDIR)\des.obj"
	-@erase "$(INTDIR)\ezxml.obj"
	-@erase "$(INTDIR)\msn.obj"
	-@erase "$(INTDIR)\msn.pch"
	-@erase "$(INTDIR)\msn_auth.obj"
	-@erase "$(INTDIR)\msn_chat.obj"
	-@erase "$(INTDIR)\msn_commands.obj"
	-@erase "$(INTDIR)\msn_contact.obj"
	-@erase "$(INTDIR)\msn_errors.obj"
	-@erase "$(INTDIR)\msn_ftold.obj"
	-@erase "$(INTDIR)\msn_http.obj"
	-@erase "$(INTDIR)\msn_libstr.obj"
	-@erase "$(INTDIR)\msn_links.obj"
	-@erase "$(INTDIR)\msn_lists.obj"
	-@erase "$(INTDIR)\msn_mail.obj"
	-@erase "$(INTDIR)\msn_menu.obj"
	-@erase "$(INTDIR)\msn_mime.obj"
	-@erase "$(INTDIR)\msn_misc.obj"
	-@erase "$(INTDIR)\msn_msgqueue.obj"
	-@erase "$(INTDIR)\msn_msgsplit.obj"
	-@erase "$(INTDIR)\msn_natdetect.obj"
	-@erase "$(INTDIR)\msn_opts.obj"
	-@erase "$(INTDIR)\msn_p2p.obj"
	-@erase "$(INTDIR)\msn_p2ps.obj"
	-@erase "$(INTDIR)\msn_proto.obj"
	-@erase "$(INTDIR)\msn_soapab.obj"
	-@erase "$(INTDIR)\msn_soapstore.obj"
	-@erase "$(INTDIR)\msn_srv.obj"
	-@erase "$(INTDIR)\msn_ssl.obj"
	-@erase "$(INTDIR)\msn_std.obj"
	-@erase "$(INTDIR)\msn_svcs.obj"
	-@erase "$(INTDIR)\msn_switchboard.obj"
	-@erase "$(INTDIR)\msn_threads.obj"
	-@erase "$(INTDIR)\msn_useropts.obj"
	-@erase "$(INTDIR)\msn_ws.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\msn.exp"
	-@erase "$(OUTDIR)\msn.map"
	-@erase "$(OUTDIR)\msn.pdb"
	-@erase "..\..\bin\Release Unicode\plugins\msn.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\msn.pch" /Yu"msn_global.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\msn.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /base:"0x19000000" /dll /incremental:no /pdb:"$(OUTDIR)\msn.pdb" /map:"$(INTDIR)\msn.map" /debug /machine:I386 /out:"../../bin/Release Unicode/plugins/msn.dll" /implib:"$(OUTDIR)\msn.lib" /filealign:512 
LINK32_OBJS= \
	"$(INTDIR)\des.obj" \
	"$(INTDIR)\ezxml.obj" \
	"$(INTDIR)\msn.obj" \
	"$(INTDIR)\msn_auth.obj" \
	"$(INTDIR)\msn_chat.obj" \
	"$(INTDIR)\msn_commands.obj" \
	"$(INTDIR)\msn_contact.obj" \
	"$(INTDIR)\msn_errors.obj" \
	"$(INTDIR)\msn_ftold.obj" \
	"$(INTDIR)\msn_http.obj" \
	"$(INTDIR)\msn_libstr.obj" \
	"$(INTDIR)\msn_links.obj" \
	"$(INTDIR)\msn_lists.obj" \
	"$(INTDIR)\msn_mail.obj" \
	"$(INTDIR)\msn_menu.obj" \
	"$(INTDIR)\msn_mime.obj" \
	"$(INTDIR)\msn_misc.obj" \
	"$(INTDIR)\msn_msgqueue.obj" \
	"$(INTDIR)\msn_msgsplit.obj" \
	"$(INTDIR)\msn_natdetect.obj" \
	"$(INTDIR)\msn_opts.obj" \
	"$(INTDIR)\msn_p2p.obj" \
	"$(INTDIR)\msn_p2ps.obj" \
	"$(INTDIR)\msn_proto.obj" \
	"$(INTDIR)\msn_soapab.obj" \
	"$(INTDIR)\msn_soapstore.obj" \
	"$(INTDIR)\msn_srv.obj" \
	"$(INTDIR)\msn_ssl.obj" \
	"$(INTDIR)\msn_std.obj" \
	"$(INTDIR)\msn_svcs.obj" \
	"$(INTDIR)\msn_switchboard.obj" \
	"$(INTDIR)\msn_threads.obj" \
	"$(INTDIR)\msn_useropts.obj" \
	"$(INTDIR)\msn_ws.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Release Unicode\plugins\msn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"

OUTDIR=.\./Debug_Unicode
INTDIR=.\./Debug_Unicode
# Begin Custom Macros
OutDir=.\./Debug_Unicode
# End Custom Macros

ALL : "..\..\bin\Debug Unicode\plugins\msn.dll" "$(OUTDIR)\msn.bsc"


CLEAN :
	-@erase "$(INTDIR)\des.obj"
	-@erase "$(INTDIR)\des.sbr"
	-@erase "$(INTDIR)\ezxml.obj"
	-@erase "$(INTDIR)\ezxml.sbr"
	-@erase "$(INTDIR)\msn.obj"
	-@erase "$(INTDIR)\msn.pch"
	-@erase "$(INTDIR)\msn.sbr"
	-@erase "$(INTDIR)\msn_auth.obj"
	-@erase "$(INTDIR)\msn_auth.sbr"
	-@erase "$(INTDIR)\msn_chat.obj"
	-@erase "$(INTDIR)\msn_chat.sbr"
	-@erase "$(INTDIR)\msn_commands.obj"
	-@erase "$(INTDIR)\msn_commands.sbr"
	-@erase "$(INTDIR)\msn_contact.obj"
	-@erase "$(INTDIR)\msn_contact.sbr"
	-@erase "$(INTDIR)\msn_errors.obj"
	-@erase "$(INTDIR)\msn_errors.sbr"
	-@erase "$(INTDIR)\msn_ftold.obj"
	-@erase "$(INTDIR)\msn_ftold.sbr"
	-@erase "$(INTDIR)\msn_http.obj"
	-@erase "$(INTDIR)\msn_http.sbr"
	-@erase "$(INTDIR)\msn_libstr.obj"
	-@erase "$(INTDIR)\msn_libstr.sbr"
	-@erase "$(INTDIR)\msn_links.obj"
	-@erase "$(INTDIR)\msn_links.sbr"
	-@erase "$(INTDIR)\msn_lists.obj"
	-@erase "$(INTDIR)\msn_lists.sbr"
	-@erase "$(INTDIR)\msn_mail.obj"
	-@erase "$(INTDIR)\msn_mail.sbr"
	-@erase "$(INTDIR)\msn_menu.obj"
	-@erase "$(INTDIR)\msn_menu.sbr"
	-@erase "$(INTDIR)\msn_mime.obj"
	-@erase "$(INTDIR)\msn_mime.sbr"
	-@erase "$(INTDIR)\msn_misc.obj"
	-@erase "$(INTDIR)\msn_misc.sbr"
	-@erase "$(INTDIR)\msn_msgqueue.obj"
	-@erase "$(INTDIR)\msn_msgqueue.sbr"
	-@erase "$(INTDIR)\msn_msgsplit.obj"
	-@erase "$(INTDIR)\msn_msgsplit.sbr"
	-@erase "$(INTDIR)\msn_natdetect.obj"
	-@erase "$(INTDIR)\msn_natdetect.sbr"
	-@erase "$(INTDIR)\msn_opts.obj"
	-@erase "$(INTDIR)\msn_opts.sbr"
	-@erase "$(INTDIR)\msn_p2p.obj"
	-@erase "$(INTDIR)\msn_p2p.sbr"
	-@erase "$(INTDIR)\msn_p2ps.obj"
	-@erase "$(INTDIR)\msn_p2ps.sbr"
	-@erase "$(INTDIR)\msn_proto.obj"
	-@erase "$(INTDIR)\msn_proto.sbr"
	-@erase "$(INTDIR)\msn_soapab.obj"
	-@erase "$(INTDIR)\msn_soapab.sbr"
	-@erase "$(INTDIR)\msn_soapstore.obj"
	-@erase "$(INTDIR)\msn_soapstore.sbr"
	-@erase "$(INTDIR)\msn_srv.obj"
	-@erase "$(INTDIR)\msn_srv.sbr"
	-@erase "$(INTDIR)\msn_ssl.obj"
	-@erase "$(INTDIR)\msn_ssl.sbr"
	-@erase "$(INTDIR)\msn_std.obj"
	-@erase "$(INTDIR)\msn_std.sbr"
	-@erase "$(INTDIR)\msn_svcs.obj"
	-@erase "$(INTDIR)\msn_svcs.sbr"
	-@erase "$(INTDIR)\msn_switchboard.obj"
	-@erase "$(INTDIR)\msn_switchboard.sbr"
	-@erase "$(INTDIR)\msn_threads.obj"
	-@erase "$(INTDIR)\msn_threads.sbr"
	-@erase "$(INTDIR)\msn_useropts.obj"
	-@erase "$(INTDIR)\msn_useropts.sbr"
	-@erase "$(INTDIR)\msn_ws.obj"
	-@erase "$(INTDIR)\msn_ws.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\msn.bsc"
	-@erase "$(OUTDIR)\msn.exp"
	-@erase "$(OUTDIR)\msn.pdb"
	-@erase "..\..\bin\Debug Unicode\plugins\msn.dll"
	-@erase "..\..\bin\Debug Unicode\plugins\msn.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MSN_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\msn.pch" /Yu"msn_global.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x809 /fo"$(INTDIR)\resource.res" /i "../../include" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\msn.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\des.sbr" \
	"$(INTDIR)\ezxml.sbr" \
	"$(INTDIR)\msn.sbr" \
	"$(INTDIR)\msn_auth.sbr" \
	"$(INTDIR)\msn_chat.sbr" \
	"$(INTDIR)\msn_commands.sbr" \
	"$(INTDIR)\msn_contact.sbr" \
	"$(INTDIR)\msn_errors.sbr" \
	"$(INTDIR)\msn_ftold.sbr" \
	"$(INTDIR)\msn_http.sbr" \
	"$(INTDIR)\msn_libstr.sbr" \
	"$(INTDIR)\msn_links.sbr" \
	"$(INTDIR)\msn_lists.sbr" \
	"$(INTDIR)\msn_mail.sbr" \
	"$(INTDIR)\msn_menu.sbr" \
	"$(INTDIR)\msn_mime.sbr" \
	"$(INTDIR)\msn_misc.sbr" \
	"$(INTDIR)\msn_msgqueue.sbr" \
	"$(INTDIR)\msn_msgsplit.sbr" \
	"$(INTDIR)\msn_natdetect.sbr" \
	"$(INTDIR)\msn_opts.sbr" \
	"$(INTDIR)\msn_p2p.sbr" \
	"$(INTDIR)\msn_p2ps.sbr" \
	"$(INTDIR)\msn_proto.sbr" \
	"$(INTDIR)\msn_soapab.sbr" \
	"$(INTDIR)\msn_soapstore.sbr" \
	"$(INTDIR)\msn_srv.sbr" \
	"$(INTDIR)\msn_ssl.sbr" \
	"$(INTDIR)\msn_std.sbr" \
	"$(INTDIR)\msn_svcs.sbr" \
	"$(INTDIR)\msn_switchboard.sbr" \
	"$(INTDIR)\msn_threads.sbr" \
	"$(INTDIR)\msn_useropts.sbr" \
	"$(INTDIR)\msn_ws.sbr"

"$(OUTDIR)\msn.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib comctl32.lib Rpcrt4.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\msn.pdb" /debug /machine:I386 /out:"../../bin/Debug Unicode/plugins/msn.dll" /implib:"$(OUTDIR)\msn.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\des.obj" \
	"$(INTDIR)\ezxml.obj" \
	"$(INTDIR)\msn.obj" \
	"$(INTDIR)\msn_auth.obj" \
	"$(INTDIR)\msn_chat.obj" \
	"$(INTDIR)\msn_commands.obj" \
	"$(INTDIR)\msn_contact.obj" \
	"$(INTDIR)\msn_errors.obj" \
	"$(INTDIR)\msn_ftold.obj" \
	"$(INTDIR)\msn_http.obj" \
	"$(INTDIR)\msn_libstr.obj" \
	"$(INTDIR)\msn_links.obj" \
	"$(INTDIR)\msn_lists.obj" \
	"$(INTDIR)\msn_mail.obj" \
	"$(INTDIR)\msn_menu.obj" \
	"$(INTDIR)\msn_mime.obj" \
	"$(INTDIR)\msn_misc.obj" \
	"$(INTDIR)\msn_msgqueue.obj" \
	"$(INTDIR)\msn_msgsplit.obj" \
	"$(INTDIR)\msn_natdetect.obj" \
	"$(INTDIR)\msn_opts.obj" \
	"$(INTDIR)\msn_p2p.obj" \
	"$(INTDIR)\msn_p2ps.obj" \
	"$(INTDIR)\msn_proto.obj" \
	"$(INTDIR)\msn_soapab.obj" \
	"$(INTDIR)\msn_soapstore.obj" \
	"$(INTDIR)\msn_srv.obj" \
	"$(INTDIR)\msn_ssl.obj" \
	"$(INTDIR)\msn_std.obj" \
	"$(INTDIR)\msn_svcs.obj" \
	"$(INTDIR)\msn_switchboard.obj" \
	"$(INTDIR)\msn_threads.obj" \
	"$(INTDIR)\msn_useropts.obj" \
	"$(INTDIR)\msn_ws.obj" \
	"$(INTDIR)\resource.res"

"..\..\bin\Debug Unicode\plugins\msn.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("msn.dep")
!INCLUDE "msn.dep"
!ELSE 
!MESSAGE Warning: cannot find "msn.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "msn - Win32 Release" || "$(CFG)" == "msn - Win32 Debug" || "$(CFG)" == "msn - Win32 Release Unicode" || "$(CFG)" == "msn - Win32 Debug Unicode"
SOURCE=.\des.c

!IF  "$(CFG)" == "msn - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\des.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\des.obj"	"$(INTDIR)\des.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\des.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MSN_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\des.obj"	"$(INTDIR)\des.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\ezxml.c

!IF  "$(CFG)" == "msn - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\ezxml.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\ezxml.obj"	"$(INTDIR)\ezxml.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\ezxml.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MSN_EXPORTS" /FR"$(INTDIR)\\" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\ezxml.obj"	"$(INTDIR)\ezxml.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\msn.cpp

!IF  "$(CFG)" == "msn - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\msn.pch" /Yc"msn_global.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\msn.obj"	"$(INTDIR)\msn.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "MSN_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\msn.pch" /Yc"msn_global.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\msn.obj"	"$(INTDIR)\msn.sbr"	"$(INTDIR)\msn.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"

CPP_SWITCHES=/nologo /MD /W3 /Zi /O1 /Oy /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MSN_EXPORTS" /FAcs /Fa"$(INTDIR)\\" /Fp"$(INTDIR)\msn.pch" /Yc"msn_global.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\msn.obj"	"$(INTDIR)\msn.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Gi /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "UNICODE" /D "_USRDLL" /D "MSN_EXPORTS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\msn.pch" /Yc"msn_global.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\msn.obj"	"$(INTDIR)\msn.sbr"	"$(INTDIR)\msn.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\msn_auth.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_auth.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_auth.obj"	"$(INTDIR)\msn_auth.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_auth.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_auth.obj"	"$(INTDIR)\msn_auth.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_chat.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_chat.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_chat.obj"	"$(INTDIR)\msn_chat.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_chat.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_chat.obj"	"$(INTDIR)\msn_chat.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_commands.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_commands.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_commands.obj"	"$(INTDIR)\msn_commands.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_commands.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_commands.obj"	"$(INTDIR)\msn_commands.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_contact.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_contact.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_contact.obj"	"$(INTDIR)\msn_contact.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_contact.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_contact.obj"	"$(INTDIR)\msn_contact.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_errors.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_errors.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_errors.obj"	"$(INTDIR)\msn_errors.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_errors.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_errors.obj"	"$(INTDIR)\msn_errors.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_ftold.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_ftold.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_ftold.obj"	"$(INTDIR)\msn_ftold.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_ftold.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_ftold.obj"	"$(INTDIR)\msn_ftold.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_http.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_http.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_http.obj"	"$(INTDIR)\msn_http.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_http.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_http.obj"	"$(INTDIR)\msn_http.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_libstr.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_libstr.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_libstr.obj"	"$(INTDIR)\msn_libstr.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_libstr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_libstr.obj"	"$(INTDIR)\msn_libstr.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_links.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_links.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_links.obj"	"$(INTDIR)\msn_links.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_links.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_links.obj"	"$(INTDIR)\msn_links.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_lists.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_lists.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_lists.obj"	"$(INTDIR)\msn_lists.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_lists.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_lists.obj"	"$(INTDIR)\msn_lists.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_mail.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_mail.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_mail.obj"	"$(INTDIR)\msn_mail.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_mail.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_mail.obj"	"$(INTDIR)\msn_mail.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_menu.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_menu.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_menu.obj"	"$(INTDIR)\msn_menu.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_menu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_menu.obj"	"$(INTDIR)\msn_menu.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_mime.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_mime.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_mime.obj"	"$(INTDIR)\msn_mime.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_mime.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_mime.obj"	"$(INTDIR)\msn_mime.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_misc.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_misc.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_misc.obj"	"$(INTDIR)\msn_misc.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_misc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_misc.obj"	"$(INTDIR)\msn_misc.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_msgqueue.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_msgqueue.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_msgqueue.obj"	"$(INTDIR)\msn_msgqueue.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_msgqueue.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_msgqueue.obj"	"$(INTDIR)\msn_msgqueue.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_msgsplit.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_msgsplit.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_msgsplit.obj"	"$(INTDIR)\msn_msgsplit.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_msgsplit.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_msgsplit.obj"	"$(INTDIR)\msn_msgsplit.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_natdetect.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_natdetect.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_natdetect.obj"	"$(INTDIR)\msn_natdetect.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_natdetect.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_natdetect.obj"	"$(INTDIR)\msn_natdetect.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_opts.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_opts.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_opts.obj"	"$(INTDIR)\msn_opts.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_opts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_opts.obj"	"$(INTDIR)\msn_opts.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_p2p.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_p2p.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_p2p.obj"	"$(INTDIR)\msn_p2p.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_p2p.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_p2p.obj"	"$(INTDIR)\msn_p2p.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_p2ps.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_p2ps.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_p2ps.obj"	"$(INTDIR)\msn_p2ps.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_p2ps.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_p2ps.obj"	"$(INTDIR)\msn_p2ps.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_proto.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_proto.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_proto.obj"	"$(INTDIR)\msn_proto.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_proto.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_proto.obj"	"$(INTDIR)\msn_proto.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_soapab.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_soapab.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_soapab.obj"	"$(INTDIR)\msn_soapab.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_soapab.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_soapab.obj"	"$(INTDIR)\msn_soapab.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_soapstore.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_soapstore.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_soapstore.obj"	"$(INTDIR)\msn_soapstore.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_soapstore.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_soapstore.obj"	"$(INTDIR)\msn_soapstore.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_srv.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_srv.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_srv.obj"	"$(INTDIR)\msn_srv.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_srv.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_srv.obj"	"$(INTDIR)\msn_srv.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_ssl.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_ssl.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_ssl.obj"	"$(INTDIR)\msn_ssl.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_ssl.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_ssl.obj"	"$(INTDIR)\msn_ssl.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_std.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_std.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_std.obj"	"$(INTDIR)\msn_std.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_std.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_std.obj"	"$(INTDIR)\msn_std.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_svcs.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_svcs.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_svcs.obj"	"$(INTDIR)\msn_svcs.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_svcs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_svcs.obj"	"$(INTDIR)\msn_svcs.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_switchboard.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_switchboard.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_switchboard.obj"	"$(INTDIR)\msn_switchboard.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_switchboard.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_switchboard.obj"	"$(INTDIR)\msn_switchboard.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_threads.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_threads.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_threads.obj"	"$(INTDIR)\msn_threads.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_threads.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_threads.obj"	"$(INTDIR)\msn_threads.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_useropts.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_useropts.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_useropts.obj"	"$(INTDIR)\msn_useropts.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_useropts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_useropts.obj"	"$(INTDIR)\msn_useropts.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\msn_ws.cpp

!IF  "$(CFG)" == "msn - Win32 Release"


"$(INTDIR)\msn_ws.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug"


"$(INTDIR)\msn_ws.obj"	"$(INTDIR)\msn_ws.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "msn - Win32 Release Unicode"


"$(INTDIR)\msn_ws.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ELSEIF  "$(CFG)" == "msn - Win32 Debug Unicode"


"$(INTDIR)\msn_ws.obj"	"$(INTDIR)\msn_ws.sbr" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\msn.pch"


!ENDIF 

SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

