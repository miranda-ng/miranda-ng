call "F:\Programme\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT" 
pushd ..
nmake -f Skype_protocol.mak CFG="Win32 UNICODE Debug" clean
nmake -f Skype_protocol.mak CFG="Win32 UNICODE Debug"
nmake -f Skype_protocol.mak CFG="Win32 Debug" clean
nmake -f Skype_protocol.mak CFG="Win32 Debug"
popd
