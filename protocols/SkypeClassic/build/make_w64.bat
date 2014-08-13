call f:\PlatformSDK\SetEnv /X64 /RETAIL
pushd ..
rem There is no ANSI x64 Miranda build
rem nmake -f Skype_protocol.mak  CFG="Win64 Release" clean
rem nmake -f Skype_protocol.mak  CFG="Win64 Release"
nmake -f Skype_protocol.mak  CFG="Win64 UNICODE Release" clean
nmake -f Skype_protocol.mak  CFG="Win64 UNICODE Release"
popd
