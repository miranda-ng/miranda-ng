call f:\PlatformSDK\SetEnv /X64 /RETAIL
nmake -f imoproxy.mak  CFG="Win64 Release" clean
nmake -f imoskype.mak  CFG="Win64 Release" clean
nmake -f imoproxy.mak  CFG="Win64 Release"
nmake -f imoskype.mak  CFG="Win64 Release"
