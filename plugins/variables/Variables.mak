# Microsoft Developer Studio Generated NMAKE File, Based on Variables.dsp
!IF "$(CFG)" == ""
CFG=Variables - Win32 Release Unicode
!MESSAGE No configuration specified. Defaulting to Variables - Win32 Release Unicode.
!ENDIF

!IF "$(CFG)" != "Variables - Win32 Release" && "$(CFG)" != "Variables - Win32 Debug" && "$(CFG)" != "Variables - Win32 Debug Unicode" && "$(CFG)" != "Variables - Win32 Release Unicode"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "Variables.mak" CFG="Variables - Win32 Debug Unicode"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "Variables - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Variables - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Variables - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Variables - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
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

!IF  "$(CFG)" == "Variables - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\..\bin\Release\plugins\Variables.dll"


CLEAN :
	-@erase "$(INTDIR)\action_variables.obj"
	-@erase "$(INTDIR)\condition_variables.obj"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\db_helpers.obj"
	-@erase "$(INTDIR)\enumprocs.obj"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\lookup3.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\parse_alias.obj"
	-@erase "$(INTDIR)\parse_external.obj"
	-@erase "$(INTDIR)\parse_inet.obj"
	-@erase "$(INTDIR)\parse_logic.obj"
	-@erase "$(INTDIR)\parse_math.obj"
	-@erase "$(INTDIR)\parse_metacontacts.obj"
	-@erase "$(INTDIR)\parse_miranda.obj"
	-@erase "$(INTDIR)\parse_regexp.obj"
	-@erase "$(INTDIR)\parse_str.obj"
	-@erase "$(INTDIR)\parse_system.obj"
	-@erase "$(INTDIR)\parse_variables.obj"
	-@erase "$(INTDIR)\parse_xml.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\tokenregister.obj"
	-@erase "$(INTDIR)\trigger_variables.obj"
	-@erase "$(INTDIR)\variables.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Variables.exp"
	-@erase "$(OUTDIR)\Variables.lib"
	-@erase "..\..\bin\Release\plugins\Variables.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O1 /I "../../include" /I "./pcre/include" /I "./" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VARIABLES_EXPORTS" /D _WIN32_WINNT=0x0502 /Fp"$(INTDIR)\Variables.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Variables.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pdh.lib Ws2_32.lib Delayimp.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\Variables.pdb" /machine:I386 /out:"../../bin/Release/plugins/Variables.dll" /implib:"$(OUTDIR)\Variables.lib" /DELAYLOAD:pdh.dll /DELAYLOAD:Ws2_32.dll
LINK32_OBJS= \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\lookup3.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\tokenregister.obj" \
	"$(INTDIR)\variables.obj" \
	"$(INTDIR)\enumprocs.obj" \
	"$(INTDIR)\parse_alias.obj" \
	"$(INTDIR)\parse_external.obj" \
	"$(INTDIR)\parse_inet.obj" \
	"$(INTDIR)\parse_logic.obj" \
	"$(INTDIR)\parse_math.obj" \
	"$(INTDIR)\parse_miranda.obj" \
	"$(INTDIR)\parse_regexp.obj" \
	"$(INTDIR)\parse_str.obj" \
	"$(INTDIR)\parse_system.obj" \
	"$(INTDIR)\parse_variables.obj" \
	"$(INTDIR)\parse_xml.obj" \
	"$(INTDIR)\action_variables.obj" \
	"$(INTDIR)\condition_variables.obj" \
	"$(INTDIR)\trigger_variables.obj" \
	"$(INTDIR)\db_helpers.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\resource.res" \
	"$(INTDIR)\parse_metacontacts.obj"

"..\..\bin\Release\plugins\Variables.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "..\..\bin\Debug\plugins\Variables.dll" "$(OUTDIR)\Variables.bsc"


CLEAN :
	-@erase "$(INTDIR)\action_variables.obj"
	-@erase "$(INTDIR)\action_variables.sbr"
	-@erase "$(INTDIR)\condition_variables.obj"
	-@erase "$(INTDIR)\condition_variables.sbr"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\contact.sbr"
	-@erase "$(INTDIR)\db_helpers.obj"
	-@erase "$(INTDIR)\db_helpers.sbr"
	-@erase "$(INTDIR)\enumprocs.obj"
	-@erase "$(INTDIR)\enumprocs.sbr"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\gen_helpers.sbr"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\help.sbr"
	-@erase "$(INTDIR)\lookup3.obj"
	-@erase "$(INTDIR)\lookup3.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\parse_alias.obj"
	-@erase "$(INTDIR)\parse_alias.sbr"
	-@erase "$(INTDIR)\parse_external.obj"
	-@erase "$(INTDIR)\parse_external.sbr"
	-@erase "$(INTDIR)\parse_inet.obj"
	-@erase "$(INTDIR)\parse_inet.sbr"
	-@erase "$(INTDIR)\parse_logic.obj"
	-@erase "$(INTDIR)\parse_logic.sbr"
	-@erase "$(INTDIR)\parse_math.obj"
	-@erase "$(INTDIR)\parse_math.sbr"
	-@erase "$(INTDIR)\parse_metacontacts.obj"
	-@erase "$(INTDIR)\parse_metacontacts.sbr"
	-@erase "$(INTDIR)\parse_miranda.obj"
	-@erase "$(INTDIR)\parse_miranda.sbr"
	-@erase "$(INTDIR)\parse_regexp.obj"
	-@erase "$(INTDIR)\parse_regexp.sbr"
	-@erase "$(INTDIR)\parse_str.obj"
	-@erase "$(INTDIR)\parse_str.sbr"
	-@erase "$(INTDIR)\parse_system.obj"
	-@erase "$(INTDIR)\parse_system.sbr"
	-@erase "$(INTDIR)\parse_variables.obj"
	-@erase "$(INTDIR)\parse_variables.sbr"
	-@erase "$(INTDIR)\parse_xml.obj"
	-@erase "$(INTDIR)\parse_xml.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\tokenregister.obj"
	-@erase "$(INTDIR)\tokenregister.sbr"
	-@erase "$(INTDIR)\trigger_variables.obj"
	-@erase "$(INTDIR)\trigger_variables.sbr"
	-@erase "$(INTDIR)\variables.obj"
	-@erase "$(INTDIR)\variables.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Variables.bsc"
	-@erase "$(OUTDIR)\Variables.exp"
	-@erase "$(OUTDIR)\Variables.lib"
	-@erase "$(OUTDIR)\Variables.pdb"
	-@erase "..\..\bin\Debug\plugins\Variables.dll"
	-@erase "..\..\bin\Debug\plugins\Variables.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "./pcre/include" /I "./" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VARIABLES_EXPORTS" /D _WIN32_WINNT=0x0502 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Variables.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /D /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Variables.bsc"
BSC32_SBRS= \
	"$(INTDIR)\contact.sbr" \
	"$(INTDIR)\help.sbr" \
	"$(INTDIR)\lookup3.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\tokenregister.sbr" \
	"$(INTDIR)\variables.sbr" \
	"$(INTDIR)\enumprocs.sbr" \
	"$(INTDIR)\parse_alias.sbr" \
	"$(INTDIR)\parse_external.sbr" \
	"$(INTDIR)\parse_inet.sbr" \
	"$(INTDIR)\parse_logic.sbr" \
	"$(INTDIR)\parse_math.sbr" \
	"$(INTDIR)\parse_miranda.sbr" \
	"$(INTDIR)\parse_regexp.sbr" \
	"$(INTDIR)\parse_str.sbr" \
	"$(INTDIR)\parse_system.sbr" \
	"$(INTDIR)\parse_variables.sbr" \
	"$(INTDIR)\parse_xml.sbr" \
	"$(INTDIR)\action_variables.sbr" \
	"$(INTDIR)\condition_variables.sbr" \
	"$(INTDIR)\trigger_variables.sbr" \
	"$(INTDIR)\db_helpers.sbr" \
	"$(INTDIR)\gen_helpers.sbr" \
	"$(INTDIR)\parse_metacontacts.sbr"

"$(OUTDIR)\Variables.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Pdh.lib Ws2_32.lib Delayimp.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\Variables.pdb" /debug /machine:I386 /out:"../../bin/Debug/plugins/Variables.dll" /implib:"$(OUTDIR)\Variables.lib" /pdbtype:sept /DELAYLOAD:pdh.dll /DELAYLOAD:Ws2_32.dll
LINK32_OBJS= \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\lookup3.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\tokenregister.obj" \
	"$(INTDIR)\variables.obj" \
	"$(INTDIR)\enumprocs.obj" \
	"$(INTDIR)\parse_alias.obj" \
	"$(INTDIR)\parse_external.obj" \
	"$(INTDIR)\parse_inet.obj" \
	"$(INTDIR)\parse_logic.obj" \
	"$(INTDIR)\parse_math.obj" \
	"$(INTDIR)\parse_miranda.obj" \
	"$(INTDIR)\parse_regexp.obj" \
	"$(INTDIR)\parse_str.obj" \
	"$(INTDIR)\parse_system.obj" \
	"$(INTDIR)\parse_variables.obj" \
	"$(INTDIR)\parse_xml.obj" \
	"$(INTDIR)\action_variables.obj" \
	"$(INTDIR)\condition_variables.obj" \
	"$(INTDIR)\trigger_variables.obj" \
	"$(INTDIR)\db_helpers.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\resource.res" \
	"$(INTDIR)\parse_metacontacts.obj"

"..\..\bin\Debug\plugins\Variables.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"

OUTDIR=.\Debug_Unicode
INTDIR=.\Debug_Unicode
# Begin Custom Macros
OutDir=.\Debug_Unicode
# End Custom Macros

ALL : "..\..\bin\Debug unicode\plugins\Variables.dll" "$(OUTDIR)\Variables.bsc"


CLEAN :
	-@erase "$(INTDIR)\action_variables.obj"
	-@erase "$(INTDIR)\action_variables.sbr"
	-@erase "$(INTDIR)\condition_variables.obj"
	-@erase "$(INTDIR)\condition_variables.sbr"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\contact.sbr"
	-@erase "$(INTDIR)\db_helpers.obj"
	-@erase "$(INTDIR)\db_helpers.sbr"
	-@erase "$(INTDIR)\enumprocs.obj"
	-@erase "$(INTDIR)\enumprocs.sbr"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\gen_helpers.sbr"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\help.sbr"
	-@erase "$(INTDIR)\lookup3.obj"
	-@erase "$(INTDIR)\lookup3.sbr"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\main.sbr"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\options.sbr"
	-@erase "$(INTDIR)\parse_alias.obj"
	-@erase "$(INTDIR)\parse_alias.sbr"
	-@erase "$(INTDIR)\parse_external.obj"
	-@erase "$(INTDIR)\parse_external.sbr"
	-@erase "$(INTDIR)\parse_inet.obj"
	-@erase "$(INTDIR)\parse_inet.sbr"
	-@erase "$(INTDIR)\parse_logic.obj"
	-@erase "$(INTDIR)\parse_logic.sbr"
	-@erase "$(INTDIR)\parse_math.obj"
	-@erase "$(INTDIR)\parse_math.sbr"
	-@erase "$(INTDIR)\parse_metacontacts.obj"
	-@erase "$(INTDIR)\parse_metacontacts.sbr"
	-@erase "$(INTDIR)\parse_miranda.obj"
	-@erase "$(INTDIR)\parse_miranda.sbr"
	-@erase "$(INTDIR)\parse_regexp.obj"
	-@erase "$(INTDIR)\parse_regexp.sbr"
	-@erase "$(INTDIR)\parse_str.obj"
	-@erase "$(INTDIR)\parse_str.sbr"
	-@erase "$(INTDIR)\parse_system.obj"
	-@erase "$(INTDIR)\parse_system.sbr"
	-@erase "$(INTDIR)\parse_variables.obj"
	-@erase "$(INTDIR)\parse_variables.sbr"
	-@erase "$(INTDIR)\parse_xml.obj"
	-@erase "$(INTDIR)\parse_xml.sbr"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\tokenregister.obj"
	-@erase "$(INTDIR)\tokenregister.sbr"
	-@erase "$(INTDIR)\trigger_variables.obj"
	-@erase "$(INTDIR)\trigger_variables.sbr"
	-@erase "$(INTDIR)\variables.obj"
	-@erase "$(INTDIR)\variables.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Variables.bsc"
	-@erase "$(OUTDIR)\Variables.exp"
	-@erase "$(OUTDIR)\Variables.lib"
	-@erase "$(OUTDIR)\Variables.pdb"
	-@erase "..\..\bin\Debug unicode\plugins\Variables.dll"
	-@erase "..\..\bin\Debug unicode\plugins\Variables.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../../include" /I "./pcre/include" /I "./" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VARIABLES_EXPORTS" /D "UNICODE" /D _WIN32_WINNT=0x0502 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Variables.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /D /GZ /c
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /i "../../include/" /d "_DEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Variables.bsc"
BSC32_SBRS= \
	"$(INTDIR)\contact.sbr" \
	"$(INTDIR)\help.sbr" \
	"$(INTDIR)\lookup3.sbr" \
	"$(INTDIR)\main.sbr" \
	"$(INTDIR)\options.sbr" \
	"$(INTDIR)\tokenregister.sbr" \
	"$(INTDIR)\variables.sbr" \
	"$(INTDIR)\enumprocs.sbr" \
	"$(INTDIR)\parse_alias.sbr" \
	"$(INTDIR)\parse_external.sbr" \
	"$(INTDIR)\parse_inet.sbr" \
	"$(INTDIR)\parse_logic.sbr" \
	"$(INTDIR)\parse_math.sbr" \
	"$(INTDIR)\parse_miranda.sbr" \
	"$(INTDIR)\parse_regexp.sbr" \
	"$(INTDIR)\parse_str.sbr" \
	"$(INTDIR)\parse_system.sbr" \
	"$(INTDIR)\parse_variables.sbr" \
	"$(INTDIR)\parse_xml.sbr" \
	"$(INTDIR)\action_variables.sbr" \
	"$(INTDIR)\condition_variables.sbr" \
	"$(INTDIR)\trigger_variables.sbr" \
	"$(INTDIR)\db_helpers.sbr" \
	"$(INTDIR)\gen_helpers.sbr" \
	"$(INTDIR)\parse_metacontacts.sbr"

"$(OUTDIR)\Variables.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Pdh.lib Ws2_32.lib Delayimp.lib /nologo /dll /incremental:yes /pdb:"$(OUTDIR)\Variables.pdb" /debug /machine:I386 /out:"../../bin/Debug unicode/plugins/Variables.dll" /implib:"$(OUTDIR)\Variables.lib" /pdbtype:sept /DELAYLOAD:pdh.dll /DELAYLOAD:Ws2_32.dll
LINK32_OBJS= \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\lookup3.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\tokenregister.obj" \
	"$(INTDIR)\variables.obj" \
	"$(INTDIR)\enumprocs.obj" \
	"$(INTDIR)\parse_alias.obj" \
	"$(INTDIR)\parse_external.obj" \
	"$(INTDIR)\parse_inet.obj" \
	"$(INTDIR)\parse_logic.obj" \
	"$(INTDIR)\parse_math.obj" \
	"$(INTDIR)\parse_miranda.obj" \
	"$(INTDIR)\parse_regexp.obj" \
	"$(INTDIR)\parse_str.obj" \
	"$(INTDIR)\parse_system.obj" \
	"$(INTDIR)\parse_variables.obj" \
	"$(INTDIR)\parse_xml.obj" \
	"$(INTDIR)\action_variables.obj" \
	"$(INTDIR)\condition_variables.obj" \
	"$(INTDIR)\trigger_variables.obj" \
	"$(INTDIR)\db_helpers.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\resource.res" \
	"$(INTDIR)\parse_metacontacts.obj"

"..\..\bin\Debug unicode\plugins\Variables.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"

OUTDIR=.\Release_Unicode
INTDIR=.\Release_Unicode

ALL : "..\..\bin\Release Unicode\plugins\Variables.dll"


CLEAN :
	-@erase "$(INTDIR)\action_variables.obj"
	-@erase "$(INTDIR)\condition_variables.obj"
	-@erase "$(INTDIR)\contact.obj"
	-@erase "$(INTDIR)\db_helpers.obj"
	-@erase "$(INTDIR)\enumprocs.obj"
	-@erase "$(INTDIR)\gen_helpers.obj"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\lookup3.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\options.obj"
	-@erase "$(INTDIR)\parse_alias.obj"
	-@erase "$(INTDIR)\parse_external.obj"
	-@erase "$(INTDIR)\parse_inet.obj"
	-@erase "$(INTDIR)\parse_logic.obj"
	-@erase "$(INTDIR)\parse_math.obj"
	-@erase "$(INTDIR)\parse_metacontacts.obj"
	-@erase "$(INTDIR)\parse_miranda.obj"
	-@erase "$(INTDIR)\parse_regexp.obj"
	-@erase "$(INTDIR)\parse_str.obj"
	-@erase "$(INTDIR)\parse_system.obj"
	-@erase "$(INTDIR)\parse_variables.obj"
	-@erase "$(INTDIR)\parse_xml.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\tokenregister.obj"
	-@erase "$(INTDIR)\trigger_variables.obj"
	-@erase "$(INTDIR)\variables.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Variables.exp"
	-@erase "$(OUTDIR)\Variables.lib"
	-@erase "..\..\bin\Release Unicode\plugins\Variables.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /O1 /I "../../include" /I "./pcre/include" /I "./" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VARIABLES_EXPORTS" /D "UNICODE" /D _WIN32_WINNT=0x0502 /Fp"$(INTDIR)\Variables.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "NDEBUG"
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Variables.bsc"
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pdh.lib Ws2_32.lib Delayimp.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\Variables.pdb" /machine:I386 /out:"../../bin/Release Unicode/plugins/Variables.dll" /implib:"$(OUTDIR)\Variables.lib" /DELAYLOAD:pdh.dll /DELAYLOAD:Ws2_32.dll
LINK32_OBJS= \
	"$(INTDIR)\contact.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\lookup3.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\options.obj" \
	"$(INTDIR)\tokenregister.obj" \
	"$(INTDIR)\variables.obj" \
	"$(INTDIR)\enumprocs.obj" \
	"$(INTDIR)\parse_alias.obj" \
	"$(INTDIR)\parse_external.obj" \
	"$(INTDIR)\parse_inet.obj" \
	"$(INTDIR)\parse_logic.obj" \
	"$(INTDIR)\parse_math.obj" \
	"$(INTDIR)\parse_miranda.obj" \
	"$(INTDIR)\parse_regexp.obj" \
	"$(INTDIR)\parse_str.obj" \
	"$(INTDIR)\parse_system.obj" \
	"$(INTDIR)\parse_variables.obj" \
	"$(INTDIR)\parse_xml.obj" \
	"$(INTDIR)\action_variables.obj" \
	"$(INTDIR)\condition_variables.obj" \
	"$(INTDIR)\trigger_variables.obj" \
	"$(INTDIR)\db_helpers.obj" \
	"$(INTDIR)\gen_helpers.obj" \
	"$(INTDIR)\resource.res" \
	"$(INTDIR)\parse_metacontacts.obj"

"..\..\bin\Release Unicode\plugins\Variables.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("Variables.dep")
!INCLUDE "Variables.dep"
!ELSE
!MESSAGE Warning: cannot find "Variables.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "Variables - Win32 Release" || "$(CFG)" == "Variables - Win32 Debug" || "$(CFG)" == "Variables - Win32 Debug Unicode" || "$(CFG)" == "Variables - Win32 Release Unicode"
SOURCE=.\contact.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\contact.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\contact.obj"	"$(INTDIR)\contact.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\contact.obj"	"$(INTDIR)\contact.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\contact.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\help.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\help.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\help.obj"	"$(INTDIR)\help.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\help.obj"	"$(INTDIR)\help.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\help.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\lookup3.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\lookup3.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\lookup3.obj"	"$(INTDIR)\lookup3.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\lookup3.obj"	"$(INTDIR)\lookup3.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\lookup3.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\main.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\main.obj"	"$(INTDIR)\main.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\options.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\options.obj"	"$(INTDIR)\options.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\options.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\tokenregister.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\tokenregister.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\tokenregister.obj"	"$(INTDIR)\tokenregister.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\tokenregister.obj"	"$(INTDIR)\tokenregister.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\tokenregister.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\variables.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\variables.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\variables.obj"	"$(INTDIR)\variables.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\variables.obj"	"$(INTDIR)\variables.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\variables.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\variables.rc
SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\enumprocs.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\enumprocs.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\enumprocs.obj"	"$(INTDIR)\enumprocs.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\enumprocs.obj"	"$(INTDIR)\enumprocs.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\enumprocs.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_alias.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_alias.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_alias.obj"	"$(INTDIR)\parse_alias.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_alias.obj"	"$(INTDIR)\parse_alias.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_alias.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_external.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_external.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_external.obj"	"$(INTDIR)\parse_external.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_external.obj"	"$(INTDIR)\parse_external.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_external.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_inet.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_inet.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_inet.obj"	"$(INTDIR)\parse_inet.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_inet.obj"	"$(INTDIR)\parse_inet.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_inet.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_logic.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_logic.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_logic.obj"	"$(INTDIR)\parse_logic.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_logic.obj"	"$(INTDIR)\parse_logic.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_logic.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_math.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_math.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_math.obj"	"$(INTDIR)\parse_math.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_math.obj"	"$(INTDIR)\parse_math.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_math.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_metacontacts.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_metacontacts.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_metacontacts.obj"	"$(INTDIR)\parse_metacontacts.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_metacontacts.obj"	"$(INTDIR)\parse_metacontacts.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_metacontacts.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_miranda.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_miranda.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_miranda.obj"	"$(INTDIR)\parse_miranda.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_miranda.obj"	"$(INTDIR)\parse_miranda.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_miranda.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_regexp.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_regexp.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_regexp.obj"	"$(INTDIR)\parse_regexp.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_regexp.obj"	"$(INTDIR)\parse_regexp.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_regexp.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_str.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_str.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_str.obj"	"$(INTDIR)\parse_str.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_str.obj"	"$(INTDIR)\parse_str.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_str.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_system.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_system.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_system.obj"	"$(INTDIR)\parse_system.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_system.obj"	"$(INTDIR)\parse_system.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_system.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_variables.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_variables.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_variables.obj"	"$(INTDIR)\parse_variables.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_variables.obj"	"$(INTDIR)\parse_variables.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_variables.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\parse_xml.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\parse_xml.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\parse_xml.obj"	"$(INTDIR)\parse_xml.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\parse_xml.obj"	"$(INTDIR)\parse_xml.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\parse_xml.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\action_variables.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\action_variables.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\action_variables.obj"	"$(INTDIR)\action_variables.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\action_variables.obj"	"$(INTDIR)\action_variables.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\action_variables.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\condition_variables.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\condition_variables.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\condition_variables.obj"	"$(INTDIR)\condition_variables.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\condition_variables.obj"	"$(INTDIR)\condition_variables.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\condition_variables.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=.\trigger_variables.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\trigger_variables.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\trigger_variables.obj"	"$(INTDIR)\trigger_variables.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\trigger_variables.obj"	"$(INTDIR)\trigger_variables.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\trigger_variables.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF

SOURCE=..\helpers\db_helpers.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\db_helpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\db_helpers.obj"	"$(INTDIR)\db_helpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\db_helpers.obj"	"$(INTDIR)\db_helpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\db_helpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

SOURCE=..\helpers\gen_helpers.cpp

!IF  "$(CFG)" == "Variables - Win32 Release"


"$(INTDIR)\gen_helpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug"


"$(INTDIR)\gen_helpers.obj"	"$(INTDIR)\gen_helpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Variables - Win32 Debug Unicode"


"$(INTDIR)\gen_helpers.obj"	"$(INTDIR)\gen_helpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "Variables - Win32 Release Unicode"


"$(INTDIR)\gen_helpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF


!ENDIF
