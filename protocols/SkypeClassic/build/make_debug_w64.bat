call f:\PlatformSDK\SetEnv /X64 /RETAIL
pushd ..
nmake -f Skype_protocol.mak CFG="Win64 UNICODE Debug" clean
nmake -f Skype_protocol.mak CFG="Win64 UNICODE Debug"
popd
