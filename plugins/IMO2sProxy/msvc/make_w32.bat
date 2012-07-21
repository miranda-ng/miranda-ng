call "F:\Programme\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"
nmake -f imoproxy.mak CFG="Win32 Release" clean
nmake -f imoskype.mak CFG="Win32 Release" clean
nmake -f imoproxy.mak CFG="Win32 Release"
nmake -f imoskype.mak CFG="Win32 Release"
nmake -f imoproxy.mak CFG="Win32 Debug"
nmake -f imoskype.mak CFG="Win32 Debug"
