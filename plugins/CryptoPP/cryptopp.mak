# Microsoft Developer Studio Generated NMAKE File, Based on cryptopp.dsp
!IF "$(CFG)" == ""
CFG=cryptopp - Win32 Release
!MESSAGE No configuration specified. Defaulting to cryptopp - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "cryptopp - Win32 Release" && "$(CFG)" != "cryptopp - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cryptopp.mak" CFG="cryptopp - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cryptopp - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "cryptopp - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "cryptopp - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\cryptopp.dll"

!ELSE 

ALL : "cryptlib - Win32 Release" "$(OUTDIR)\cryptopp.dll"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"cryptlib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\base16.obj"
	-@erase "$(INTDIR)\base64.obj"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\cpp_cntx.obj"
	-@erase "$(INTDIR)\cpp_gpgw.obj"
	-@erase "$(INTDIR)\cpp_gzip.obj"
	-@erase "$(INTDIR)\cpp_keys.obj"
	-@erase "$(INTDIR)\cpp_misc.obj"
	-@erase "$(INTDIR)\cpp_pgpw.obj"
	-@erase "$(INTDIR)\cpp_rsam.obj"
	-@erase "$(INTDIR)\cpp_rsau.obj"
	-@erase "$(INTDIR)\cpp_svcs.obj"
	-@erase "$(INTDIR)\cryptopp.pch"
	-@erase "$(INTDIR)\dllloader.obj"
	-@erase "$(INTDIR)\gettime.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mmi.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\utf8.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\cryptopp.dll"
	-@erase "$(OUTDIR)\cryptopp.exp"
	-@erase "$(OUTDIR)\cryptopp.lib"
	-@erase "$(OUTDIR)\cryptopp.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O1 /I "\source\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\cryptopp.pch" /Yu"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cryptopp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib /nologo /base:"0x46000000" /dll /incremental:no /pdb:"$(OUTDIR)\cryptopp.pdb" /map:"$(INTDIR)\cryptopp.map" /machine:I386 /out:"$(OUTDIR)\cryptopp.dll" /implib:"$(OUTDIR)\cryptopp.lib" 
LINK32_OBJS= \
	"$(INTDIR)\base16.obj" \
	"$(INTDIR)\base64.obj" \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\cpp_cntx.obj" \
	"$(INTDIR)\cpp_gpgw.obj" \
	"$(INTDIR)\cpp_gzip.obj" \
	"$(INTDIR)\cpp_keys.obj" \
	"$(INTDIR)\cpp_misc.obj" \
	"$(INTDIR)\cpp_pgpw.obj" \
	"$(INTDIR)\cpp_rsam.obj" \
	"$(INTDIR)\cpp_rsau.obj" \
	"$(INTDIR)\cpp_svcs.obj" \
	"$(INTDIR)\dllloader.obj" \
	"$(INTDIR)\gettime.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\mmi.obj" \
	"$(INTDIR)\utf8.obj" \
	"$(INTDIR)\resource.res" \
	".\crypto\Release\cryptlib.lib"

"$(OUTDIR)\cryptopp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cryptopp - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\cryptopp.dll"

!ELSE 

ALL : "cryptlib - Win32 Debug" "$(OUTDIR)\cryptopp.dll"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"cryptlib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\base16.obj"
	-@erase "$(INTDIR)\base64.obj"
	-@erase "$(INTDIR)\commonheaders.obj"
	-@erase "$(INTDIR)\cpp_cntx.obj"
	-@erase "$(INTDIR)\cpp_gpgw.obj"
	-@erase "$(INTDIR)\cpp_gzip.obj"
	-@erase "$(INTDIR)\cpp_keys.obj"
	-@erase "$(INTDIR)\cpp_misc.obj"
	-@erase "$(INTDIR)\cpp_pgpw.obj"
	-@erase "$(INTDIR)\cpp_rsam.obj"
	-@erase "$(INTDIR)\cpp_rsau.obj"
	-@erase "$(INTDIR)\cpp_svcs.obj"
	-@erase "$(INTDIR)\dllloader.obj"
	-@erase "$(INTDIR)\gettime.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\mmi.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\utf8.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\cryptopp.dll"
	-@erase "$(OUTDIR)\cryptopp.exp"
	-@erase "$(OUTDIR)\cryptopp.ilk"
	-@erase "$(OUTDIR)\cryptopp.lib"
	-@erase "$(OUTDIR)\cryptopp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "\source\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CRYPTOPP_EXPORTS" /Fp"$(INTDIR)\cryptopp.pch" /YX"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cryptopp.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib uuid.lib /nologo /base:"0x46000000" /dll /incremental:yes /pdb:"$(OUTDIR)\cryptopp.pdb" /debug /machine:I386 /out:"$(OUTDIR)\cryptopp.dll" /implib:"$(OUTDIR)\cryptopp.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\base16.obj" \
	"$(INTDIR)\base64.obj" \
	"$(INTDIR)\commonheaders.obj" \
	"$(INTDIR)\cpp_cntx.obj" \
	"$(INTDIR)\cpp_gpgw.obj" \
	"$(INTDIR)\cpp_gzip.obj" \
	"$(INTDIR)\cpp_keys.obj" \
	"$(INTDIR)\cpp_misc.obj" \
	"$(INTDIR)\cpp_pgpw.obj" \
	"$(INTDIR)\cpp_rsam.obj" \
	"$(INTDIR)\cpp_rsau.obj" \
	"$(INTDIR)\cpp_svcs.obj" \
	"$(INTDIR)\dllloader.obj" \
	"$(INTDIR)\gettime.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\mmi.obj" \
	"$(INTDIR)\utf8.obj" \
	"$(INTDIR)\resource.res" \
	".\crypto\Debug\cryptlib.lib"

"$(OUTDIR)\cryptopp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("cryptopp.dep")
!INCLUDE "cryptopp.dep"
!ELSE 
!MESSAGE Warning: cannot find "cryptopp.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "cryptopp - Win32 Release" || "$(CFG)" == "cryptopp - Win32 Debug"
SOURCE=.\base16.cpp

"$(INTDIR)\base16.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\base64.cpp

"$(INTDIR)\base64.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\commonheaders.cpp

!IF  "$(CFG)" == "cryptopp - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O1 /I "\source\include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\cryptopp.pch" /Yc"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\commonheaders.obj"	"$(INTDIR)\cryptopp.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "cryptopp - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "\source\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CRYPTOPP_EXPORTS" /Fp"$(INTDIR)\cryptopp.pch" /YX"commonheaders.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\commonheaders.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\cpp_cntx.cpp

"$(INTDIR)\cpp_cntx.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\cpp_gpgw.cpp

"$(INTDIR)\cpp_gpgw.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\cpp_gzip.cpp

"$(INTDIR)\cpp_gzip.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\cpp_keys.cpp

"$(INTDIR)\cpp_keys.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\cpp_misc.cpp

"$(INTDIR)\cpp_misc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\cpp_pgpw.cpp

"$(INTDIR)\cpp_pgpw.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\cpp_rsam.cpp

"$(INTDIR)\cpp_rsam.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\cpp_rsau.cpp

"$(INTDIR)\cpp_rsau.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\cpp_svcs.cpp

"$(INTDIR)\cpp_svcs.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\dllloader.cpp

"$(INTDIR)\dllloader.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\gettime.cpp

"$(INTDIR)\gettime.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\mmi.cpp

"$(INTDIR)\mmi.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\utf8.cpp

"$(INTDIR)\utf8.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptopp.pch"


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


!IF  "$(CFG)" == "cryptopp - Win32 Release"

"cryptlib - Win32 Release" : 
   cd ".\crypto"
   $(MAKE) /$(MAKEFLAGS) /F .\cryptlib.mak CFG="cryptlib - Win32 Release" 
   cd ".."

"cryptlib - Win32 ReleaseCLEAN" : 
   cd ".\crypto"
   $(MAKE) /$(MAKEFLAGS) /F .\cryptlib.mak CFG="cryptlib - Win32 Release" RECURSE=1 CLEAN 
   cd ".."

!ELSEIF  "$(CFG)" == "cryptopp - Win32 Debug"

"cryptlib - Win32 Debug" : 
   cd ".\crypto"
   $(MAKE) /$(MAKEFLAGS) /F .\cryptlib.mak CFG="cryptlib - Win32 Debug" 
   cd ".."

"cryptlib - Win32 DebugCLEAN" : 
   cd ".\crypto"
   $(MAKE) /$(MAKEFLAGS) /F .\cryptlib.mak CFG="cryptlib - Win32 Debug" RECURSE=1 CLEAN 
   cd ".."

!ENDIF 


!ENDIF 

