call "F:\Programme\Microsoft Visual Studio\VC98\Bin\VCVARS32.BAT"
pushd ..
nmake -f Skype_protocol.mak CFG="Win32 Release" clean
nmake -f Skype_protocol.mak CFG="Win32 Release"
nmake -f Skype_protocol.mak CFG="Win32 UNICODE Release" clean
nmake -f Skype_protocol.mak CFG="Win32 UNICODE Release"
popd
