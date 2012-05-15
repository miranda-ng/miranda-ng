# Microsoft Developer Studio Generated NMAKE File, Based on cryptlib.dsp
!IF "$(CFG)" == ""
CFG=cryptlib - Win32 Debug
!MESSAGE No configuration specified. Defaulting to cryptlib - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "cryptlib - Win32 DLL-Import Release" && "$(CFG)" != "cryptlib - Win32 DLL-Import Debug" && "$(CFG)" != "cryptlib - Win32 Release" && "$(CFG)" != "cryptlib - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "cryptlib.mak" CFG="cryptlib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "cryptlib - Win32 DLL-Import Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cryptlib - Win32 DLL-Import Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "cryptlib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "cryptlib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "cryptlib - Win32 DLL-Import Release"

OUTDIR=.\DLL_Import_Release
INTDIR=.\DLL_Import_Release
# Begin Custom Macros
OutDir=.\DLL_Import_Release
# End Custom Macros

ALL : ".\adhoc.cpp.copied" "$(OUTDIR)\cryptlib.lib"


CLEAN :
	-@erase "$(INTDIR)\3way.obj"
	-@erase "$(INTDIR)\adler32.obj"
	-@erase "$(INTDIR)\algebra.obj"
	-@erase "$(INTDIR)\algparam.obj"
	-@erase "$(INTDIR)\arc4.obj"
	-@erase "$(INTDIR)\asn.obj"
	-@erase "$(INTDIR)\base32.obj"
	-@erase "$(INTDIR)\base64.obj"
	-@erase "$(INTDIR)\basecode.obj"
	-@erase "$(INTDIR)\bfinit.obj"
	-@erase "$(INTDIR)\blowfish.obj"
	-@erase "$(INTDIR)\blumshub.obj"
	-@erase "$(INTDIR)\camellia.obj"
	-@erase "$(INTDIR)\cast.obj"
	-@erase "$(INTDIR)\casts.obj"
	-@erase "$(INTDIR)\cbcmac.obj"
	-@erase "$(INTDIR)\channels.obj"
	-@erase "$(INTDIR)\cpu.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\cryptlib.obj"
	-@erase "$(INTDIR)\cryptlib.pch"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\des.obj"
	-@erase "$(INTDIR)\dessp.obj"
	-@erase "$(INTDIR)\dh.obj"
	-@erase "$(INTDIR)\dh2.obj"
	-@erase "$(INTDIR)\dll.obj"
	-@erase "$(INTDIR)\dsa.obj"
	-@erase "$(INTDIR)\ec2n.obj"
	-@erase "$(INTDIR)\eccrypto.obj"
	-@erase "$(INTDIR)\ecp.obj"
	-@erase "$(INTDIR)\elgamal.obj"
	-@erase "$(INTDIR)\emsa2.obj"
	-@erase "$(INTDIR)\eprecomp.obj"
	-@erase "$(INTDIR)\esign.obj"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\filters.obj"
	-@erase "$(INTDIR)\fips140.obj"
	-@erase "$(INTDIR)\fipstest.obj"
	-@erase "$(INTDIR)\gf256.obj"
	-@erase "$(INTDIR)\gf2_32.obj"
	-@erase "$(INTDIR)\gf2n.obj"
	-@erase "$(INTDIR)\gfpcrypt.obj"
	-@erase "$(INTDIR)\gost.obj"
	-@erase "$(INTDIR)\gzip.obj"
	-@erase "$(INTDIR)\hex.obj"
	-@erase "$(INTDIR)\hmac.obj"
	-@erase "$(INTDIR)\hrtimer.obj"
	-@erase "$(INTDIR)\ida.obj"
	-@erase "$(INTDIR)\idea.obj"
	-@erase "$(INTDIR)\integer.obj"
	-@erase "$(INTDIR)\iterhash.obj"
	-@erase "$(INTDIR)\luc.obj"
	-@erase "$(INTDIR)\mars.obj"
	-@erase "$(INTDIR)\marss.obj"
	-@erase "$(INTDIR)\md2.obj"
	-@erase "$(INTDIR)\md4.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\modes.obj"
	-@erase "$(INTDIR)\mqueue.obj"
	-@erase "$(INTDIR)\mqv.obj"
	-@erase "$(INTDIR)\nbtheory.obj"
	-@erase "$(INTDIR)\network.obj"
	-@erase "$(INTDIR)\oaep.obj"
	-@erase "$(INTDIR)\osrng.obj"
	-@erase "$(INTDIR)\panama.obj"
	-@erase "$(INTDIR)\pch.obj"
	-@erase "$(INTDIR)\pkcspad.obj"
	-@erase "$(INTDIR)\polynomi.obj"
	-@erase "$(INTDIR)\pssr.obj"
	-@erase "$(INTDIR)\pubkey.obj"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\rabin.obj"
	-@erase "$(INTDIR)\randpool.obj"
	-@erase "$(INTDIR)\rc2.obj"
	-@erase "$(INTDIR)\rc5.obj"
	-@erase "$(INTDIR)\rc6.obj"
	-@erase "$(INTDIR)\rdtables.obj"
	-@erase "$(INTDIR)\rijndael.obj"
	-@erase "$(INTDIR)\ripemd.obj"
	-@erase "$(INTDIR)\rng.obj"
	-@erase "$(INTDIR)\rsa.obj"
	-@erase "$(INTDIR)\rw.obj"
	-@erase "$(INTDIR)\safer.obj"
	-@erase "$(INTDIR)\salsa.obj"
	-@erase "$(INTDIR)\seal.obj"
	-@erase "$(INTDIR)\serpent.obj"
	-@erase "$(INTDIR)\sha.obj"
	-@erase "$(INTDIR)\shacal2.obj"
	-@erase "$(INTDIR)\shark.obj"
	-@erase "$(INTDIR)\sharkbox.obj"
	-@erase "$(INTDIR)\simple.obj"
	-@erase "$(INTDIR)\skipjack.obj"
	-@erase "$(INTDIR)\socketft.obj"
	-@erase "$(INTDIR)\sosemanuk.obj"
	-@erase "$(INTDIR)\square.obj"
	-@erase "$(INTDIR)\squaretb.obj"
	-@erase "$(INTDIR)\strciphr.obj"
	-@erase "$(INTDIR)\tea.obj"
	-@erase "$(INTDIR)\tftables.obj"
	-@erase "$(INTDIR)\tiger.obj"
	-@erase "$(INTDIR)\tigertab.obj"
	-@erase "$(INTDIR)\trdlocal.obj"
	-@erase "$(INTDIR)\ttmac.obj"
	-@erase "$(INTDIR)\twofish.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vmac.obj"
	-@erase "$(INTDIR)\wait.obj"
	-@erase "$(INTDIR)\wake.obj"
	-@erase "$(INTDIR)\whrlpool.obj"
	-@erase "$(INTDIR)\winpipes.obj"
	-@erase "$(INTDIR)\xtr.obj"
	-@erase "$(INTDIR)\xtrcrypt.obj"
	-@erase "$(INTDIR)\zdeflate.obj"
	-@erase "$(INTDIR)\zinflate.obj"
	-@erase "$(INTDIR)\zlib.obj"
	-@erase "$(OUTDIR)\cryptlib.lib"
	-@erase "adhoc.cpp.copied"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /Gz /MT /W3 /GX /Zi /O2 /Ob2 /D "NDEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /D "CRYPTOPP_IMPORTS" /Fp"$(INTDIR)\cryptlib.pch" /Yu"pch.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cryptlib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\cryptlib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\3way.obj" \
	"$(INTDIR)\adler32.obj" \
	"$(INTDIR)\algebra.obj" \
	"$(INTDIR)\algparam.obj" \
	"$(INTDIR)\arc4.obj" \
	"$(INTDIR)\asn.obj" \
	"$(INTDIR)\base32.obj" \
	"$(INTDIR)\base64.obj" \
	"$(INTDIR)\basecode.obj" \
	"$(INTDIR)\bfinit.obj" \
	"$(INTDIR)\blowfish.obj" \
	"$(INTDIR)\blumshub.obj" \
	"$(INTDIR)\camellia.obj" \
	"$(INTDIR)\cast.obj" \
	"$(INTDIR)\casts.obj" \
	"$(INTDIR)\cbcmac.obj" \
	"$(INTDIR)\channels.obj" \
	"$(INTDIR)\cpu.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cryptlib.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\des.obj" \
	"$(INTDIR)\dessp.obj" \
	"$(INTDIR)\dh.obj" \
	"$(INTDIR)\dh2.obj" \
	"$(INTDIR)\dll.obj" \
	"$(INTDIR)\dsa.obj" \
	"$(INTDIR)\ec2n.obj" \
	"$(INTDIR)\eccrypto.obj" \
	"$(INTDIR)\ecp.obj" \
	"$(INTDIR)\elgamal.obj" \
	"$(INTDIR)\emsa2.obj" \
	"$(INTDIR)\eprecomp.obj" \
	"$(INTDIR)\esign.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\filters.obj" \
	"$(INTDIR)\fips140.obj" \
	"$(INTDIR)\fipstest.obj" \
	"$(INTDIR)\gf256.obj" \
	"$(INTDIR)\gf2_32.obj" \
	"$(INTDIR)\gf2n.obj" \
	"$(INTDIR)\gfpcrypt.obj" \
	"$(INTDIR)\gost.obj" \
	"$(INTDIR)\gzip.obj" \
	"$(INTDIR)\hex.obj" \
	"$(INTDIR)\hmac.obj" \
	"$(INTDIR)\hrtimer.obj" \
	"$(INTDIR)\ida.obj" \
	"$(INTDIR)\idea.obj" \
	"$(INTDIR)\integer.obj" \
	"$(INTDIR)\iterhash.obj" \
	"$(INTDIR)\luc.obj" \
	"$(INTDIR)\mars.obj" \
	"$(INTDIR)\marss.obj" \
	"$(INTDIR)\md2.obj" \
	"$(INTDIR)\md4.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\modes.obj" \
	"$(INTDIR)\mqueue.obj" \
	"$(INTDIR)\mqv.obj" \
	"$(INTDIR)\nbtheory.obj" \
	"$(INTDIR)\network.obj" \
	"$(INTDIR)\oaep.obj" \
	"$(INTDIR)\osrng.obj" \
	"$(INTDIR)\panama.obj" \
	"$(INTDIR)\pch.obj" \
	"$(INTDIR)\pkcspad.obj" \
	"$(INTDIR)\polynomi.obj" \
	"$(INTDIR)\pssr.obj" \
	"$(INTDIR)\pubkey.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\rabin.obj" \
	"$(INTDIR)\randpool.obj" \
	"$(INTDIR)\rc2.obj" \
	"$(INTDIR)\rc5.obj" \
	"$(INTDIR)\rc6.obj" \
	"$(INTDIR)\rdtables.obj" \
	"$(INTDIR)\rijndael.obj" \
	"$(INTDIR)\ripemd.obj" \
	"$(INTDIR)\rng.obj" \
	"$(INTDIR)\rsa.obj" \
	"$(INTDIR)\rw.obj" \
	"$(INTDIR)\safer.obj" \
	"$(INTDIR)\salsa.obj" \
	"$(INTDIR)\seal.obj" \
	"$(INTDIR)\serpent.obj" \
	"$(INTDIR)\sha.obj" \
	"$(INTDIR)\shacal2.obj" \
	"$(INTDIR)\shark.obj" \
	"$(INTDIR)\sharkbox.obj" \
	"$(INTDIR)\simple.obj" \
	"$(INTDIR)\skipjack.obj" \
	"$(INTDIR)\socketft.obj" \
	"$(INTDIR)\sosemanuk.obj" \
	"$(INTDIR)\square.obj" \
	"$(INTDIR)\squaretb.obj" \
	"$(INTDIR)\strciphr.obj" \
	"$(INTDIR)\tea.obj" \
	"$(INTDIR)\tftables.obj" \
	"$(INTDIR)\tiger.obj" \
	"$(INTDIR)\tigertab.obj" \
	"$(INTDIR)\trdlocal.obj" \
	"$(INTDIR)\ttmac.obj" \
	"$(INTDIR)\twofish.obj" \
	"$(INTDIR)\vmac.obj" \
	"$(INTDIR)\wait.obj" \
	"$(INTDIR)\wake.obj" \
	"$(INTDIR)\whrlpool.obj" \
	"$(INTDIR)\winpipes.obj" \
	"$(INTDIR)\xtr.obj" \
	"$(INTDIR)\xtrcrypt.obj" \
	"$(INTDIR)\zdeflate.obj" \
	"$(INTDIR)\zinflate.obj" \
	"$(INTDIR)\zlib.obj"

"$(OUTDIR)\cryptlib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cryptlib - Win32 DLL-Import Debug"

OUTDIR=.\DLL_Import_Debug
INTDIR=.\DLL_Import_Debug
# Begin Custom Macros
OutDir=.\DLL_Import_Debug
# End Custom Macros

ALL : ".\adhoc.cpp.copied" "$(OUTDIR)\cryptlib.lib"


CLEAN :
	-@erase "$(INTDIR)\3way.obj"
	-@erase "$(INTDIR)\adler32.obj"
	-@erase "$(INTDIR)\algebra.obj"
	-@erase "$(INTDIR)\algparam.obj"
	-@erase "$(INTDIR)\arc4.obj"
	-@erase "$(INTDIR)\asn.obj"
	-@erase "$(INTDIR)\base32.obj"
	-@erase "$(INTDIR)\base64.obj"
	-@erase "$(INTDIR)\basecode.obj"
	-@erase "$(INTDIR)\bfinit.obj"
	-@erase "$(INTDIR)\blowfish.obj"
	-@erase "$(INTDIR)\blumshub.obj"
	-@erase "$(INTDIR)\camellia.obj"
	-@erase "$(INTDIR)\cast.obj"
	-@erase "$(INTDIR)\casts.obj"
	-@erase "$(INTDIR)\cbcmac.obj"
	-@erase "$(INTDIR)\channels.obj"
	-@erase "$(INTDIR)\cpu.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\cryptlib.obj"
	-@erase "$(INTDIR)\cryptlib.pch"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\des.obj"
	-@erase "$(INTDIR)\dessp.obj"
	-@erase "$(INTDIR)\dh.obj"
	-@erase "$(INTDIR)\dh2.obj"
	-@erase "$(INTDIR)\dll.obj"
	-@erase "$(INTDIR)\dsa.obj"
	-@erase "$(INTDIR)\ec2n.obj"
	-@erase "$(INTDIR)\eccrypto.obj"
	-@erase "$(INTDIR)\ecp.obj"
	-@erase "$(INTDIR)\elgamal.obj"
	-@erase "$(INTDIR)\emsa2.obj"
	-@erase "$(INTDIR)\eprecomp.obj"
	-@erase "$(INTDIR)\esign.obj"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\filters.obj"
	-@erase "$(INTDIR)\fips140.obj"
	-@erase "$(INTDIR)\fipstest.obj"
	-@erase "$(INTDIR)\gf256.obj"
	-@erase "$(INTDIR)\gf2_32.obj"
	-@erase "$(INTDIR)\gf2n.obj"
	-@erase "$(INTDIR)\gfpcrypt.obj"
	-@erase "$(INTDIR)\gost.obj"
	-@erase "$(INTDIR)\gzip.obj"
	-@erase "$(INTDIR)\hex.obj"
	-@erase "$(INTDIR)\hmac.obj"
	-@erase "$(INTDIR)\hrtimer.obj"
	-@erase "$(INTDIR)\ida.obj"
	-@erase "$(INTDIR)\idea.obj"
	-@erase "$(INTDIR)\integer.obj"
	-@erase "$(INTDIR)\iterhash.obj"
	-@erase "$(INTDIR)\luc.obj"
	-@erase "$(INTDIR)\mars.obj"
	-@erase "$(INTDIR)\marss.obj"
	-@erase "$(INTDIR)\md2.obj"
	-@erase "$(INTDIR)\md4.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\modes.obj"
	-@erase "$(INTDIR)\mqueue.obj"
	-@erase "$(INTDIR)\mqv.obj"
	-@erase "$(INTDIR)\nbtheory.obj"
	-@erase "$(INTDIR)\network.obj"
	-@erase "$(INTDIR)\oaep.obj"
	-@erase "$(INTDIR)\osrng.obj"
	-@erase "$(INTDIR)\panama.obj"
	-@erase "$(INTDIR)\pch.obj"
	-@erase "$(INTDIR)\pkcspad.obj"
	-@erase "$(INTDIR)\polynomi.obj"
	-@erase "$(INTDIR)\pssr.obj"
	-@erase "$(INTDIR)\pubkey.obj"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\rabin.obj"
	-@erase "$(INTDIR)\randpool.obj"
	-@erase "$(INTDIR)\rc2.obj"
	-@erase "$(INTDIR)\rc5.obj"
	-@erase "$(INTDIR)\rc6.obj"
	-@erase "$(INTDIR)\rdtables.obj"
	-@erase "$(INTDIR)\rijndael.obj"
	-@erase "$(INTDIR)\ripemd.obj"
	-@erase "$(INTDIR)\rng.obj"
	-@erase "$(INTDIR)\rsa.obj"
	-@erase "$(INTDIR)\rw.obj"
	-@erase "$(INTDIR)\safer.obj"
	-@erase "$(INTDIR)\salsa.obj"
	-@erase "$(INTDIR)\seal.obj"
	-@erase "$(INTDIR)\serpent.obj"
	-@erase "$(INTDIR)\sha.obj"
	-@erase "$(INTDIR)\shacal2.obj"
	-@erase "$(INTDIR)\shark.obj"
	-@erase "$(INTDIR)\sharkbox.obj"
	-@erase "$(INTDIR)\simple.obj"
	-@erase "$(INTDIR)\skipjack.obj"
	-@erase "$(INTDIR)\socketft.obj"
	-@erase "$(INTDIR)\sosemanuk.obj"
	-@erase "$(INTDIR)\square.obj"
	-@erase "$(INTDIR)\squaretb.obj"
	-@erase "$(INTDIR)\strciphr.obj"
	-@erase "$(INTDIR)\tea.obj"
	-@erase "$(INTDIR)\tftables.obj"
	-@erase "$(INTDIR)\tiger.obj"
	-@erase "$(INTDIR)\tigertab.obj"
	-@erase "$(INTDIR)\trdlocal.obj"
	-@erase "$(INTDIR)\ttmac.obj"
	-@erase "$(INTDIR)\twofish.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vmac.obj"
	-@erase "$(INTDIR)\wait.obj"
	-@erase "$(INTDIR)\wake.obj"
	-@erase "$(INTDIR)\whrlpool.obj"
	-@erase "$(INTDIR)\winpipes.obj"
	-@erase "$(INTDIR)\xtr.obj"
	-@erase "$(INTDIR)\xtrcrypt.obj"
	-@erase "$(INTDIR)\zdeflate.obj"
	-@erase "$(INTDIR)\zinflate.obj"
	-@erase "$(INTDIR)\zlib.obj"
	-@erase "$(OUTDIR)\cryptlib.lib"
	-@erase "adhoc.cpp.copied"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /Gz /MTd /W3 /GX /Zi /Oi /D "_DEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /D "CRYPTOPP_IMPORTS" /Fp"$(INTDIR)\cryptlib.pch" /Yu"pch.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cryptlib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\cryptlib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\3way.obj" \
	"$(INTDIR)\adler32.obj" \
	"$(INTDIR)\algebra.obj" \
	"$(INTDIR)\algparam.obj" \
	"$(INTDIR)\arc4.obj" \
	"$(INTDIR)\asn.obj" \
	"$(INTDIR)\base32.obj" \
	"$(INTDIR)\base64.obj" \
	"$(INTDIR)\basecode.obj" \
	"$(INTDIR)\bfinit.obj" \
	"$(INTDIR)\blowfish.obj" \
	"$(INTDIR)\blumshub.obj" \
	"$(INTDIR)\camellia.obj" \
	"$(INTDIR)\cast.obj" \
	"$(INTDIR)\casts.obj" \
	"$(INTDIR)\cbcmac.obj" \
	"$(INTDIR)\channels.obj" \
	"$(INTDIR)\cpu.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cryptlib.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\des.obj" \
	"$(INTDIR)\dessp.obj" \
	"$(INTDIR)\dh.obj" \
	"$(INTDIR)\dh2.obj" \
	"$(INTDIR)\dll.obj" \
	"$(INTDIR)\dsa.obj" \
	"$(INTDIR)\ec2n.obj" \
	"$(INTDIR)\eccrypto.obj" \
	"$(INTDIR)\ecp.obj" \
	"$(INTDIR)\elgamal.obj" \
	"$(INTDIR)\emsa2.obj" \
	"$(INTDIR)\eprecomp.obj" \
	"$(INTDIR)\esign.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\filters.obj" \
	"$(INTDIR)\fips140.obj" \
	"$(INTDIR)\fipstest.obj" \
	"$(INTDIR)\gf256.obj" \
	"$(INTDIR)\gf2_32.obj" \
	"$(INTDIR)\gf2n.obj" \
	"$(INTDIR)\gfpcrypt.obj" \
	"$(INTDIR)\gost.obj" \
	"$(INTDIR)\gzip.obj" \
	"$(INTDIR)\hex.obj" \
	"$(INTDIR)\hmac.obj" \
	"$(INTDIR)\hrtimer.obj" \
	"$(INTDIR)\ida.obj" \
	"$(INTDIR)\idea.obj" \
	"$(INTDIR)\integer.obj" \
	"$(INTDIR)\iterhash.obj" \
	"$(INTDIR)\luc.obj" \
	"$(INTDIR)\mars.obj" \
	"$(INTDIR)\marss.obj" \
	"$(INTDIR)\md2.obj" \
	"$(INTDIR)\md4.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\modes.obj" \
	"$(INTDIR)\mqueue.obj" \
	"$(INTDIR)\mqv.obj" \
	"$(INTDIR)\nbtheory.obj" \
	"$(INTDIR)\network.obj" \
	"$(INTDIR)\oaep.obj" \
	"$(INTDIR)\osrng.obj" \
	"$(INTDIR)\panama.obj" \
	"$(INTDIR)\pch.obj" \
	"$(INTDIR)\pkcspad.obj" \
	"$(INTDIR)\polynomi.obj" \
	"$(INTDIR)\pssr.obj" \
	"$(INTDIR)\pubkey.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\rabin.obj" \
	"$(INTDIR)\randpool.obj" \
	"$(INTDIR)\rc2.obj" \
	"$(INTDIR)\rc5.obj" \
	"$(INTDIR)\rc6.obj" \
	"$(INTDIR)\rdtables.obj" \
	"$(INTDIR)\rijndael.obj" \
	"$(INTDIR)\ripemd.obj" \
	"$(INTDIR)\rng.obj" \
	"$(INTDIR)\rsa.obj" \
	"$(INTDIR)\rw.obj" \
	"$(INTDIR)\safer.obj" \
	"$(INTDIR)\salsa.obj" \
	"$(INTDIR)\seal.obj" \
	"$(INTDIR)\serpent.obj" \
	"$(INTDIR)\sha.obj" \
	"$(INTDIR)\shacal2.obj" \
	"$(INTDIR)\shark.obj" \
	"$(INTDIR)\sharkbox.obj" \
	"$(INTDIR)\simple.obj" \
	"$(INTDIR)\skipjack.obj" \
	"$(INTDIR)\socketft.obj" \
	"$(INTDIR)\sosemanuk.obj" \
	"$(INTDIR)\square.obj" \
	"$(INTDIR)\squaretb.obj" \
	"$(INTDIR)\strciphr.obj" \
	"$(INTDIR)\tea.obj" \
	"$(INTDIR)\tftables.obj" \
	"$(INTDIR)\tiger.obj" \
	"$(INTDIR)\tigertab.obj" \
	"$(INTDIR)\trdlocal.obj" \
	"$(INTDIR)\ttmac.obj" \
	"$(INTDIR)\twofish.obj" \
	"$(INTDIR)\vmac.obj" \
	"$(INTDIR)\wait.obj" \
	"$(INTDIR)\wake.obj" \
	"$(INTDIR)\whrlpool.obj" \
	"$(INTDIR)\winpipes.obj" \
	"$(INTDIR)\xtr.obj" \
	"$(INTDIR)\xtrcrypt.obj" \
	"$(INTDIR)\zdeflate.obj" \
	"$(INTDIR)\zinflate.obj" \
	"$(INTDIR)\zlib.obj"

"$(OUTDIR)\cryptlib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cryptlib - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : ".\adhoc.cpp.copied" "$(OUTDIR)\cryptlib.lib"


CLEAN :
	-@erase "$(INTDIR)\3way.obj"
	-@erase "$(INTDIR)\adler32.obj"
	-@erase "$(INTDIR)\algebra.obj"
	-@erase "$(INTDIR)\algparam.obj"
	-@erase "$(INTDIR)\arc4.obj"
	-@erase "$(INTDIR)\asn.obj"
	-@erase "$(INTDIR)\base32.obj"
	-@erase "$(INTDIR)\base64.obj"
	-@erase "$(INTDIR)\basecode.obj"
	-@erase "$(INTDIR)\bfinit.obj"
	-@erase "$(INTDIR)\blowfish.obj"
	-@erase "$(INTDIR)\blumshub.obj"
	-@erase "$(INTDIR)\camellia.obj"
	-@erase "$(INTDIR)\cast.obj"
	-@erase "$(INTDIR)\casts.obj"
	-@erase "$(INTDIR)\cbcmac.obj"
	-@erase "$(INTDIR)\channels.obj"
	-@erase "$(INTDIR)\cpu.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\cryptlib.obj"
	-@erase "$(INTDIR)\cryptlib.pch"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\des.obj"
	-@erase "$(INTDIR)\dessp.obj"
	-@erase "$(INTDIR)\dh.obj"
	-@erase "$(INTDIR)\dh2.obj"
	-@erase "$(INTDIR)\dll.obj"
	-@erase "$(INTDIR)\dsa.obj"
	-@erase "$(INTDIR)\ec2n.obj"
	-@erase "$(INTDIR)\eccrypto.obj"
	-@erase "$(INTDIR)\ecp.obj"
	-@erase "$(INTDIR)\elgamal.obj"
	-@erase "$(INTDIR)\emsa2.obj"
	-@erase "$(INTDIR)\eprecomp.obj"
	-@erase "$(INTDIR)\esign.obj"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\filters.obj"
	-@erase "$(INTDIR)\fips140.obj"
	-@erase "$(INTDIR)\fipstest.obj"
	-@erase "$(INTDIR)\gf256.obj"
	-@erase "$(INTDIR)\gf2_32.obj"
	-@erase "$(INTDIR)\gf2n.obj"
	-@erase "$(INTDIR)\gfpcrypt.obj"
	-@erase "$(INTDIR)\gost.obj"
	-@erase "$(INTDIR)\gzip.obj"
	-@erase "$(INTDIR)\hex.obj"
	-@erase "$(INTDIR)\hmac.obj"
	-@erase "$(INTDIR)\hrtimer.obj"
	-@erase "$(INTDIR)\ida.obj"
	-@erase "$(INTDIR)\idea.obj"
	-@erase "$(INTDIR)\integer.obj"
	-@erase "$(INTDIR)\iterhash.obj"
	-@erase "$(INTDIR)\luc.obj"
	-@erase "$(INTDIR)\mars.obj"
	-@erase "$(INTDIR)\marss.obj"
	-@erase "$(INTDIR)\md2.obj"
	-@erase "$(INTDIR)\md4.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\modes.obj"
	-@erase "$(INTDIR)\mqueue.obj"
	-@erase "$(INTDIR)\mqv.obj"
	-@erase "$(INTDIR)\nbtheory.obj"
	-@erase "$(INTDIR)\network.obj"
	-@erase "$(INTDIR)\oaep.obj"
	-@erase "$(INTDIR)\osrng.obj"
	-@erase "$(INTDIR)\panama.obj"
	-@erase "$(INTDIR)\pch.obj"
	-@erase "$(INTDIR)\pkcspad.obj"
	-@erase "$(INTDIR)\polynomi.obj"
	-@erase "$(INTDIR)\pssr.obj"
	-@erase "$(INTDIR)\pubkey.obj"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\rabin.obj"
	-@erase "$(INTDIR)\randpool.obj"
	-@erase "$(INTDIR)\rc2.obj"
	-@erase "$(INTDIR)\rc5.obj"
	-@erase "$(INTDIR)\rc6.obj"
	-@erase "$(INTDIR)\rdtables.obj"
	-@erase "$(INTDIR)\rijndael.obj"
	-@erase "$(INTDIR)\ripemd.obj"
	-@erase "$(INTDIR)\rng.obj"
	-@erase "$(INTDIR)\rsa.obj"
	-@erase "$(INTDIR)\rw.obj"
	-@erase "$(INTDIR)\safer.obj"
	-@erase "$(INTDIR)\salsa.obj"
	-@erase "$(INTDIR)\seal.obj"
	-@erase "$(INTDIR)\serpent.obj"
	-@erase "$(INTDIR)\sha.obj"
	-@erase "$(INTDIR)\shacal2.obj"
	-@erase "$(INTDIR)\shark.obj"
	-@erase "$(INTDIR)\sharkbox.obj"
	-@erase "$(INTDIR)\simple.obj"
	-@erase "$(INTDIR)\skipjack.obj"
	-@erase "$(INTDIR)\socketft.obj"
	-@erase "$(INTDIR)\sosemanuk.obj"
	-@erase "$(INTDIR)\square.obj"
	-@erase "$(INTDIR)\squaretb.obj"
	-@erase "$(INTDIR)\strciphr.obj"
	-@erase "$(INTDIR)\tea.obj"
	-@erase "$(INTDIR)\tftables.obj"
	-@erase "$(INTDIR)\tiger.obj"
	-@erase "$(INTDIR)\tigertab.obj"
	-@erase "$(INTDIR)\trdlocal.obj"
	-@erase "$(INTDIR)\ttmac.obj"
	-@erase "$(INTDIR)\twofish.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vmac.obj"
	-@erase "$(INTDIR)\wait.obj"
	-@erase "$(INTDIR)\wake.obj"
	-@erase "$(INTDIR)\whrlpool.obj"
	-@erase "$(INTDIR)\winpipes.obj"
	-@erase "$(INTDIR)\xtr.obj"
	-@erase "$(INTDIR)\xtrcrypt.obj"
	-@erase "$(INTDIR)\zdeflate.obj"
	-@erase "$(INTDIR)\zinflate.obj"
	-@erase "$(INTDIR)\zlib.obj"
	-@erase "$(OUTDIR)\cryptlib.lib"
	-@erase "adhoc.cpp.copied"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /Zi /O1 /D "NDEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /Fp"$(INTDIR)\cryptlib.pch" /Yu"pch.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cryptlib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\cryptlib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\3way.obj" \
	"$(INTDIR)\adler32.obj" \
	"$(INTDIR)\algebra.obj" \
	"$(INTDIR)\algparam.obj" \
	"$(INTDIR)\arc4.obj" \
	"$(INTDIR)\asn.obj" \
	"$(INTDIR)\base32.obj" \
	"$(INTDIR)\base64.obj" \
	"$(INTDIR)\basecode.obj" \
	"$(INTDIR)\bfinit.obj" \
	"$(INTDIR)\blowfish.obj" \
	"$(INTDIR)\blumshub.obj" \
	"$(INTDIR)\camellia.obj" \
	"$(INTDIR)\cast.obj" \
	"$(INTDIR)\casts.obj" \
	"$(INTDIR)\cbcmac.obj" \
	"$(INTDIR)\channels.obj" \
	"$(INTDIR)\cpu.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cryptlib.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\des.obj" \
	"$(INTDIR)\dessp.obj" \
	"$(INTDIR)\dh.obj" \
	"$(INTDIR)\dh2.obj" \
	"$(INTDIR)\dll.obj" \
	"$(INTDIR)\dsa.obj" \
	"$(INTDIR)\ec2n.obj" \
	"$(INTDIR)\eccrypto.obj" \
	"$(INTDIR)\ecp.obj" \
	"$(INTDIR)\elgamal.obj" \
	"$(INTDIR)\emsa2.obj" \
	"$(INTDIR)\eprecomp.obj" \
	"$(INTDIR)\esign.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\filters.obj" \
	"$(INTDIR)\fips140.obj" \
	"$(INTDIR)\fipstest.obj" \
	"$(INTDIR)\gf256.obj" \
	"$(INTDIR)\gf2_32.obj" \
	"$(INTDIR)\gf2n.obj" \
	"$(INTDIR)\gfpcrypt.obj" \
	"$(INTDIR)\gost.obj" \
	"$(INTDIR)\gzip.obj" \
	"$(INTDIR)\hex.obj" \
	"$(INTDIR)\hmac.obj" \
	"$(INTDIR)\hrtimer.obj" \
	"$(INTDIR)\ida.obj" \
	"$(INTDIR)\idea.obj" \
	"$(INTDIR)\integer.obj" \
	"$(INTDIR)\iterhash.obj" \
	"$(INTDIR)\luc.obj" \
	"$(INTDIR)\mars.obj" \
	"$(INTDIR)\marss.obj" \
	"$(INTDIR)\md2.obj" \
	"$(INTDIR)\md4.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\modes.obj" \
	"$(INTDIR)\mqueue.obj" \
	"$(INTDIR)\mqv.obj" \
	"$(INTDIR)\nbtheory.obj" \
	"$(INTDIR)\network.obj" \
	"$(INTDIR)\oaep.obj" \
	"$(INTDIR)\osrng.obj" \
	"$(INTDIR)\panama.obj" \
	"$(INTDIR)\pch.obj" \
	"$(INTDIR)\pkcspad.obj" \
	"$(INTDIR)\polynomi.obj" \
	"$(INTDIR)\pssr.obj" \
	"$(INTDIR)\pubkey.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\rabin.obj" \
	"$(INTDIR)\randpool.obj" \
	"$(INTDIR)\rc2.obj" \
	"$(INTDIR)\rc5.obj" \
	"$(INTDIR)\rc6.obj" \
	"$(INTDIR)\rdtables.obj" \
	"$(INTDIR)\rijndael.obj" \
	"$(INTDIR)\ripemd.obj" \
	"$(INTDIR)\rng.obj" \
	"$(INTDIR)\rsa.obj" \
	"$(INTDIR)\rw.obj" \
	"$(INTDIR)\safer.obj" \
	"$(INTDIR)\salsa.obj" \
	"$(INTDIR)\seal.obj" \
	"$(INTDIR)\serpent.obj" \
	"$(INTDIR)\sha.obj" \
	"$(INTDIR)\shacal2.obj" \
	"$(INTDIR)\shark.obj" \
	"$(INTDIR)\sharkbox.obj" \
	"$(INTDIR)\simple.obj" \
	"$(INTDIR)\skipjack.obj" \
	"$(INTDIR)\socketft.obj" \
	"$(INTDIR)\sosemanuk.obj" \
	"$(INTDIR)\square.obj" \
	"$(INTDIR)\squaretb.obj" \
	"$(INTDIR)\strciphr.obj" \
	"$(INTDIR)\tea.obj" \
	"$(INTDIR)\tftables.obj" \
	"$(INTDIR)\tiger.obj" \
	"$(INTDIR)\tigertab.obj" \
	"$(INTDIR)\trdlocal.obj" \
	"$(INTDIR)\ttmac.obj" \
	"$(INTDIR)\twofish.obj" \
	"$(INTDIR)\vmac.obj" \
	"$(INTDIR)\wait.obj" \
	"$(INTDIR)\wake.obj" \
	"$(INTDIR)\whrlpool.obj" \
	"$(INTDIR)\winpipes.obj" \
	"$(INTDIR)\xtr.obj" \
	"$(INTDIR)\xtrcrypt.obj" \
	"$(INTDIR)\zdeflate.obj" \
	"$(INTDIR)\zinflate.obj" \
	"$(INTDIR)\zlib.obj"

"$(OUTDIR)\cryptlib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "cryptlib - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : ".\adhoc.cpp.copied" "$(OUTDIR)\cryptlib.lib"


CLEAN :
	-@erase "$(INTDIR)\3way.obj"
	-@erase "$(INTDIR)\adler32.obj"
	-@erase "$(INTDIR)\algebra.obj"
	-@erase "$(INTDIR)\algparam.obj"
	-@erase "$(INTDIR)\arc4.obj"
	-@erase "$(INTDIR)\asn.obj"
	-@erase "$(INTDIR)\base32.obj"
	-@erase "$(INTDIR)\base64.obj"
	-@erase "$(INTDIR)\basecode.obj"
	-@erase "$(INTDIR)\bfinit.obj"
	-@erase "$(INTDIR)\blowfish.obj"
	-@erase "$(INTDIR)\blumshub.obj"
	-@erase "$(INTDIR)\camellia.obj"
	-@erase "$(INTDIR)\cast.obj"
	-@erase "$(INTDIR)\casts.obj"
	-@erase "$(INTDIR)\cbcmac.obj"
	-@erase "$(INTDIR)\channels.obj"
	-@erase "$(INTDIR)\cpu.obj"
	-@erase "$(INTDIR)\crc.obj"
	-@erase "$(INTDIR)\cryptlib.obj"
	-@erase "$(INTDIR)\cryptlib.pch"
	-@erase "$(INTDIR)\default.obj"
	-@erase "$(INTDIR)\des.obj"
	-@erase "$(INTDIR)\dessp.obj"
	-@erase "$(INTDIR)\dh.obj"
	-@erase "$(INTDIR)\dh2.obj"
	-@erase "$(INTDIR)\dll.obj"
	-@erase "$(INTDIR)\dsa.obj"
	-@erase "$(INTDIR)\ec2n.obj"
	-@erase "$(INTDIR)\eccrypto.obj"
	-@erase "$(INTDIR)\ecp.obj"
	-@erase "$(INTDIR)\elgamal.obj"
	-@erase "$(INTDIR)\emsa2.obj"
	-@erase "$(INTDIR)\eprecomp.obj"
	-@erase "$(INTDIR)\esign.obj"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\filters.obj"
	-@erase "$(INTDIR)\fips140.obj"
	-@erase "$(INTDIR)\fipstest.obj"
	-@erase "$(INTDIR)\gf256.obj"
	-@erase "$(INTDIR)\gf2_32.obj"
	-@erase "$(INTDIR)\gf2n.obj"
	-@erase "$(INTDIR)\gfpcrypt.obj"
	-@erase "$(INTDIR)\gost.obj"
	-@erase "$(INTDIR)\gzip.obj"
	-@erase "$(INTDIR)\hex.obj"
	-@erase "$(INTDIR)\hmac.obj"
	-@erase "$(INTDIR)\hrtimer.obj"
	-@erase "$(INTDIR)\ida.obj"
	-@erase "$(INTDIR)\idea.obj"
	-@erase "$(INTDIR)\integer.obj"
	-@erase "$(INTDIR)\iterhash.obj"
	-@erase "$(INTDIR)\luc.obj"
	-@erase "$(INTDIR)\mars.obj"
	-@erase "$(INTDIR)\marss.obj"
	-@erase "$(INTDIR)\md2.obj"
	-@erase "$(INTDIR)\md4.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\modes.obj"
	-@erase "$(INTDIR)\mqueue.obj"
	-@erase "$(INTDIR)\mqv.obj"
	-@erase "$(INTDIR)\nbtheory.obj"
	-@erase "$(INTDIR)\network.obj"
	-@erase "$(INTDIR)\oaep.obj"
	-@erase "$(INTDIR)\osrng.obj"
	-@erase "$(INTDIR)\panama.obj"
	-@erase "$(INTDIR)\pch.obj"
	-@erase "$(INTDIR)\pkcspad.obj"
	-@erase "$(INTDIR)\polynomi.obj"
	-@erase "$(INTDIR)\pssr.obj"
	-@erase "$(INTDIR)\pubkey.obj"
	-@erase "$(INTDIR)\queue.obj"
	-@erase "$(INTDIR)\rabin.obj"
	-@erase "$(INTDIR)\randpool.obj"
	-@erase "$(INTDIR)\rc2.obj"
	-@erase "$(INTDIR)\rc5.obj"
	-@erase "$(INTDIR)\rc6.obj"
	-@erase "$(INTDIR)\rdtables.obj"
	-@erase "$(INTDIR)\rijndael.obj"
	-@erase "$(INTDIR)\ripemd.obj"
	-@erase "$(INTDIR)\rng.obj"
	-@erase "$(INTDIR)\rsa.obj"
	-@erase "$(INTDIR)\rw.obj"
	-@erase "$(INTDIR)\safer.obj"
	-@erase "$(INTDIR)\salsa.obj"
	-@erase "$(INTDIR)\seal.obj"
	-@erase "$(INTDIR)\serpent.obj"
	-@erase "$(INTDIR)\sha.obj"
	-@erase "$(INTDIR)\shacal2.obj"
	-@erase "$(INTDIR)\shark.obj"
	-@erase "$(INTDIR)\sharkbox.obj"
	-@erase "$(INTDIR)\simple.obj"
	-@erase "$(INTDIR)\skipjack.obj"
	-@erase "$(INTDIR)\socketft.obj"
	-@erase "$(INTDIR)\sosemanuk.obj"
	-@erase "$(INTDIR)\square.obj"
	-@erase "$(INTDIR)\squaretb.obj"
	-@erase "$(INTDIR)\strciphr.obj"
	-@erase "$(INTDIR)\tea.obj"
	-@erase "$(INTDIR)\tftables.obj"
	-@erase "$(INTDIR)\tiger.obj"
	-@erase "$(INTDIR)\tigertab.obj"
	-@erase "$(INTDIR)\trdlocal.obj"
	-@erase "$(INTDIR)\ttmac.obj"
	-@erase "$(INTDIR)\twofish.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vmac.obj"
	-@erase "$(INTDIR)\wait.obj"
	-@erase "$(INTDIR)\wake.obj"
	-@erase "$(INTDIR)\whrlpool.obj"
	-@erase "$(INTDIR)\winpipes.obj"
	-@erase "$(INTDIR)\xtr.obj"
	-@erase "$(INTDIR)\xtrcrypt.obj"
	-@erase "$(INTDIR)\zdeflate.obj"
	-@erase "$(INTDIR)\zinflate.obj"
	-@erase "$(INTDIR)\zlib.obj"
	-@erase "$(OUTDIR)\cryptlib.lib"
	-@erase "adhoc.cpp.copied"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Zi /Oi /D "_DEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /Fp"$(INTDIR)\cryptlib.pch" /Yu"pch.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\cryptlib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\cryptlib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\3way.obj" \
	"$(INTDIR)\adler32.obj" \
	"$(INTDIR)\algebra.obj" \
	"$(INTDIR)\algparam.obj" \
	"$(INTDIR)\arc4.obj" \
	"$(INTDIR)\asn.obj" \
	"$(INTDIR)\base32.obj" \
	"$(INTDIR)\base64.obj" \
	"$(INTDIR)\basecode.obj" \
	"$(INTDIR)\bfinit.obj" \
	"$(INTDIR)\blowfish.obj" \
	"$(INTDIR)\blumshub.obj" \
	"$(INTDIR)\camellia.obj" \
	"$(INTDIR)\cast.obj" \
	"$(INTDIR)\casts.obj" \
	"$(INTDIR)\cbcmac.obj" \
	"$(INTDIR)\channels.obj" \
	"$(INTDIR)\cpu.obj" \
	"$(INTDIR)\crc.obj" \
	"$(INTDIR)\cryptlib.obj" \
	"$(INTDIR)\default.obj" \
	"$(INTDIR)\des.obj" \
	"$(INTDIR)\dessp.obj" \
	"$(INTDIR)\dh.obj" \
	"$(INTDIR)\dh2.obj" \
	"$(INTDIR)\dll.obj" \
	"$(INTDIR)\dsa.obj" \
	"$(INTDIR)\ec2n.obj" \
	"$(INTDIR)\eccrypto.obj" \
	"$(INTDIR)\ecp.obj" \
	"$(INTDIR)\elgamal.obj" \
	"$(INTDIR)\emsa2.obj" \
	"$(INTDIR)\eprecomp.obj" \
	"$(INTDIR)\esign.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\filters.obj" \
	"$(INTDIR)\fips140.obj" \
	"$(INTDIR)\fipstest.obj" \
	"$(INTDIR)\gf256.obj" \
	"$(INTDIR)\gf2_32.obj" \
	"$(INTDIR)\gf2n.obj" \
	"$(INTDIR)\gfpcrypt.obj" \
	"$(INTDIR)\gost.obj" \
	"$(INTDIR)\gzip.obj" \
	"$(INTDIR)\hex.obj" \
	"$(INTDIR)\hmac.obj" \
	"$(INTDIR)\hrtimer.obj" \
	"$(INTDIR)\ida.obj" \
	"$(INTDIR)\idea.obj" \
	"$(INTDIR)\integer.obj" \
	"$(INTDIR)\iterhash.obj" \
	"$(INTDIR)\luc.obj" \
	"$(INTDIR)\mars.obj" \
	"$(INTDIR)\marss.obj" \
	"$(INTDIR)\md2.obj" \
	"$(INTDIR)\md4.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\modes.obj" \
	"$(INTDIR)\mqueue.obj" \
	"$(INTDIR)\mqv.obj" \
	"$(INTDIR)\nbtheory.obj" \
	"$(INTDIR)\network.obj" \
	"$(INTDIR)\oaep.obj" \
	"$(INTDIR)\osrng.obj" \
	"$(INTDIR)\panama.obj" \
	"$(INTDIR)\pch.obj" \
	"$(INTDIR)\pkcspad.obj" \
	"$(INTDIR)\polynomi.obj" \
	"$(INTDIR)\pssr.obj" \
	"$(INTDIR)\pubkey.obj" \
	"$(INTDIR)\queue.obj" \
	"$(INTDIR)\rabin.obj" \
	"$(INTDIR)\randpool.obj" \
	"$(INTDIR)\rc2.obj" \
	"$(INTDIR)\rc5.obj" \
	"$(INTDIR)\rc6.obj" \
	"$(INTDIR)\rdtables.obj" \
	"$(INTDIR)\rijndael.obj" \
	"$(INTDIR)\ripemd.obj" \
	"$(INTDIR)\rng.obj" \
	"$(INTDIR)\rsa.obj" \
	"$(INTDIR)\rw.obj" \
	"$(INTDIR)\safer.obj" \
	"$(INTDIR)\salsa.obj" \
	"$(INTDIR)\seal.obj" \
	"$(INTDIR)\serpent.obj" \
	"$(INTDIR)\sha.obj" \
	"$(INTDIR)\shacal2.obj" \
	"$(INTDIR)\shark.obj" \
	"$(INTDIR)\sharkbox.obj" \
	"$(INTDIR)\simple.obj" \
	"$(INTDIR)\skipjack.obj" \
	"$(INTDIR)\socketft.obj" \
	"$(INTDIR)\sosemanuk.obj" \
	"$(INTDIR)\square.obj" \
	"$(INTDIR)\squaretb.obj" \
	"$(INTDIR)\strciphr.obj" \
	"$(INTDIR)\tea.obj" \
	"$(INTDIR)\tftables.obj" \
	"$(INTDIR)\tiger.obj" \
	"$(INTDIR)\tigertab.obj" \
	"$(INTDIR)\trdlocal.obj" \
	"$(INTDIR)\ttmac.obj" \
	"$(INTDIR)\twofish.obj" \
	"$(INTDIR)\vmac.obj" \
	"$(INTDIR)\wait.obj" \
	"$(INTDIR)\wake.obj" \
	"$(INTDIR)\whrlpool.obj" \
	"$(INTDIR)\winpipes.obj" \
	"$(INTDIR)\xtr.obj" \
	"$(INTDIR)\xtrcrypt.obj" \
	"$(INTDIR)\zdeflate.obj" \
	"$(INTDIR)\zinflate.obj" \
	"$(INTDIR)\zlib.obj"

"$(OUTDIR)\cryptlib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("cryptlib.dep")
!INCLUDE "cryptlib.dep"
!ELSE 
!MESSAGE Warning: cannot find "cryptlib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "cryptlib - Win32 DLL-Import Release" || "$(CFG)" == "cryptlib - Win32 DLL-Import Debug" || "$(CFG)" == "cryptlib - Win32 Release" || "$(CFG)" == "cryptlib - Win32 Debug"
SOURCE=.\3way.cpp

"$(INTDIR)\3way.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\adhoc.cpp.proto

!IF  "$(CFG)" == "cryptlib - Win32 DLL-Import Release"

InputPath=.\adhoc.cpp.proto

".\adhoc.cpp.copied" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	if not exist adhoc.cpp copy "$(InputPath)" adhoc.cpp 
	echo: >> adhoc.cpp.copied 
<< 
	

!ELSEIF  "$(CFG)" == "cryptlib - Win32 DLL-Import Debug"

InputPath=.\adhoc.cpp.proto

".\adhoc.cpp.copied" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	if not exist adhoc.cpp copy "$(InputPath)" adhoc.cpp 
	echo: >> adhoc.cpp.copied 
<< 
	

!ELSEIF  "$(CFG)" == "cryptlib - Win32 Release"

InputPath=.\adhoc.cpp.proto

".\adhoc.cpp.copied" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	if not exist adhoc.cpp copy "$(InputPath)" adhoc.cpp 
	echo: >> adhoc.cpp.copied 
<< 
	

!ELSEIF  "$(CFG)" == "cryptlib - Win32 Debug"

InputPath=.\adhoc.cpp.proto

".\adhoc.cpp.copied" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	if not exist adhoc.cpp copy "$(InputPath)" adhoc.cpp 
	echo: >> adhoc.cpp.copied 
<< 
	

!ENDIF 

SOURCE=.\adler32.cpp

"$(INTDIR)\adler32.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\algebra.cpp

"$(INTDIR)\algebra.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\algparam.cpp

"$(INTDIR)\algparam.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\arc4.cpp

"$(INTDIR)\arc4.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\asn.cpp

"$(INTDIR)\asn.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\base32.cpp

"$(INTDIR)\base32.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\base64.cpp

"$(INTDIR)\base64.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\basecode.cpp

"$(INTDIR)\basecode.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\bfinit.cpp

"$(INTDIR)\bfinit.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\blowfish.cpp

"$(INTDIR)\blowfish.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\blumshub.cpp

"$(INTDIR)\blumshub.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\camellia.cpp

"$(INTDIR)\camellia.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\cast.cpp

"$(INTDIR)\cast.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\casts.cpp

"$(INTDIR)\casts.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\cbcmac.cpp

"$(INTDIR)\cbcmac.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\channels.cpp

"$(INTDIR)\channels.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\cpu.cpp

"$(INTDIR)\cpu.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\crc.cpp

"$(INTDIR)\crc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\cryptlib.cpp

"$(INTDIR)\cryptlib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\default.cpp

"$(INTDIR)\default.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\des.cpp

"$(INTDIR)\des.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\dessp.cpp

"$(INTDIR)\dessp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\dh.cpp

"$(INTDIR)\dh.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\dh2.cpp

"$(INTDIR)\dh2.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\dll.cpp

!IF  "$(CFG)" == "cryptlib - Win32 DLL-Import Release"

CPP_SWITCHES=/nologo /G5 /Gz /MT /W3 /GX /Zi /O2 /Ob2 /D "NDEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /D "CRYPTOPP_IMPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

"$(INTDIR)\dll.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "cryptlib - Win32 DLL-Import Debug"

CPP_SWITCHES=/nologo /G5 /Gz /MTd /W3 /GX /Zi /Oi /D "_DEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /D "CRYPTOPP_IMPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

"$(INTDIR)\dll.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "cryptlib - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /D "NDEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

"$(INTDIR)\dll.obj" : $(SOURCE) "$(INTDIR)" ".\strciphr.cpp"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "cryptlib - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Oi /D "_DEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

"$(INTDIR)\dll.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\dsa.cpp

"$(INTDIR)\dsa.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\ec2n.cpp

"$(INTDIR)\ec2n.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\eccrypto.cpp

"$(INTDIR)\eccrypto.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\ecp.cpp

"$(INTDIR)\ecp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\elgamal.cpp

"$(INTDIR)\elgamal.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\emsa2.cpp

"$(INTDIR)\emsa2.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\eprecomp.cpp

"$(INTDIR)\eprecomp.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\esign.cpp

"$(INTDIR)\esign.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\files.cpp

"$(INTDIR)\files.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\filters.cpp

"$(INTDIR)\filters.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\fips140.cpp

"$(INTDIR)\fips140.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\fipstest.cpp

"$(INTDIR)\fipstest.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\gf256.cpp

"$(INTDIR)\gf256.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\gf2_32.cpp

"$(INTDIR)\gf2_32.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\gf2n.cpp

"$(INTDIR)\gf2n.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\gfpcrypt.cpp

"$(INTDIR)\gfpcrypt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\gost.cpp

"$(INTDIR)\gost.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\gzip.cpp

"$(INTDIR)\gzip.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\hex.cpp

"$(INTDIR)\hex.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\hmac.cpp

"$(INTDIR)\hmac.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\hrtimer.cpp

"$(INTDIR)\hrtimer.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\ida.cpp

"$(INTDIR)\ida.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\idea.cpp

"$(INTDIR)\idea.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\integer.cpp

"$(INTDIR)\integer.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\iterhash.cpp

"$(INTDIR)\iterhash.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\luc.cpp

"$(INTDIR)\luc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\mars.cpp

"$(INTDIR)\mars.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\marss.cpp

"$(INTDIR)\marss.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\md2.cpp

"$(INTDIR)\md2.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\md4.cpp

"$(INTDIR)\md4.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\md5.cpp

"$(INTDIR)\md5.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\misc.cpp

"$(INTDIR)\misc.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\modes.cpp

"$(INTDIR)\modes.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\mqueue.cpp

"$(INTDIR)\mqueue.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\mqv.cpp

"$(INTDIR)\mqv.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\nbtheory.cpp

"$(INTDIR)\nbtheory.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\network.cpp

"$(INTDIR)\network.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\oaep.cpp

"$(INTDIR)\oaep.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\osrng.cpp

"$(INTDIR)\osrng.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\panama.cpp

"$(INTDIR)\panama.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\pch.cpp

!IF  "$(CFG)" == "cryptlib - Win32 DLL-Import Release"

CPP_SWITCHES=/nologo /G5 /Gz /MT /W3 /GX /Zi /O2 /Ob2 /D "NDEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /D "CRYPTOPP_IMPORTS" /Fp"$(INTDIR)\cryptlib.pch" /Yc"pch.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

"$(INTDIR)\pch.obj"	"$(INTDIR)\cryptlib.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "cryptlib - Win32 DLL-Import Debug"

CPP_SWITCHES=/nologo /G5 /Gz /MTd /W3 /GX /Zi /Oi /D "_DEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /D "CRYPTOPP_IMPORTS" /Fp"$(INTDIR)\cryptlib.pch" /Yc"pch.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

"$(INTDIR)\pch.obj"	"$(INTDIR)\cryptlib.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "cryptlib - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /Zi /O1 /D "NDEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /Fp"$(INTDIR)\cryptlib.pch" /Yc"pch.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

"$(INTDIR)\pch.obj"	"$(INTDIR)\cryptlib.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "cryptlib - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /GX /Zi /Oi /D "_DEBUG" /D "_WINDOWS" /D "USE_PRECOMPILED_HEADERS" /D "WIN32" /Fp"$(INTDIR)\cryptlib.pch" /Yc"pch.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Zm400 /c 

"$(INTDIR)\pch.obj"	"$(INTDIR)\cryptlib.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\pkcspad.cpp

"$(INTDIR)\pkcspad.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\polynomi.cpp

"$(INTDIR)\polynomi.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\pssr.cpp

"$(INTDIR)\pssr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\pubkey.cpp

"$(INTDIR)\pubkey.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\queue.cpp

"$(INTDIR)\queue.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\rabin.cpp

"$(INTDIR)\rabin.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\randpool.cpp

"$(INTDIR)\randpool.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\rc2.cpp

"$(INTDIR)\rc2.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\rc5.cpp

"$(INTDIR)\rc5.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\rc6.cpp

"$(INTDIR)\rc6.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\rdtables.cpp

"$(INTDIR)\rdtables.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\rijndael.cpp

"$(INTDIR)\rijndael.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\ripemd.cpp

"$(INTDIR)\ripemd.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\rng.cpp

"$(INTDIR)\rng.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\rsa.cpp

"$(INTDIR)\rsa.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\rw.cpp

"$(INTDIR)\rw.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\safer.cpp

"$(INTDIR)\safer.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\salsa.cpp

"$(INTDIR)\salsa.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\seal.cpp

"$(INTDIR)\seal.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\serpent.cpp

"$(INTDIR)\serpent.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\sha.cpp

"$(INTDIR)\sha.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\shacal2.cpp

"$(INTDIR)\shacal2.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\shark.cpp

"$(INTDIR)\shark.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\sharkbox.cpp

"$(INTDIR)\sharkbox.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\simple.cpp

"$(INTDIR)\simple.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\skipjack.cpp

"$(INTDIR)\skipjack.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\socketft.cpp

"$(INTDIR)\socketft.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\sosemanuk.cpp

"$(INTDIR)\sosemanuk.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\square.cpp

"$(INTDIR)\square.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\squaretb.cpp

"$(INTDIR)\squaretb.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\strciphr.cpp

"$(INTDIR)\strciphr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\tea.cpp

"$(INTDIR)\tea.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\tftables.cpp

"$(INTDIR)\tftables.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\tiger.cpp

"$(INTDIR)\tiger.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\tigertab.cpp

"$(INTDIR)\tigertab.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\trdlocal.cpp

"$(INTDIR)\trdlocal.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\ttmac.cpp

"$(INTDIR)\ttmac.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\twofish.cpp

"$(INTDIR)\twofish.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\vmac.cpp

"$(INTDIR)\vmac.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\wait.cpp

"$(INTDIR)\wait.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\wake.cpp

"$(INTDIR)\wake.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\whrlpool.cpp

"$(INTDIR)\whrlpool.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\winpipes.cpp

"$(INTDIR)\winpipes.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\xtr.cpp

"$(INTDIR)\xtr.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\xtrcrypt.cpp

"$(INTDIR)\xtrcrypt.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\zdeflate.cpp

"$(INTDIR)\zdeflate.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\zinflate.cpp

"$(INTDIR)\zinflate.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"


SOURCE=.\zlib.cpp

"$(INTDIR)\zlib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\cryptlib.pch"



!ENDIF 

