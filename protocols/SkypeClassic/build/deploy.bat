@echo off
set PATH=%PATH%;C:\Programme\WinRAR
echo Deploying..
pushd ..
winrar a -apPlugins -ep1 \\linux01\leecher\webserver\Skype\Skype_protocol.zip Release\skype.dll
rem There is no ANSI x64 Build
rem winrar a -apPlugins -ep1 \\linux01\leecher\webserver\Skype\Skype_protocol_x64.zip Release64\skype.dll
winrar a -apPlugins -ep1 \\linux01\leecher\webserver\Skype\Skype_protocol_unicode.zip Release-UNICODE\skype.dll
winrar a -apPlugins -ep1 \\linux01\leecher\webserver\Skype\Skype_protocol_unicode_x64.zip Release64-UNICODE\skype.dll

winrar a -apDebug -ep1 \\linux01\leecher\webserver\Skype\Skype_protocol_debug.zip Debug\skype.pdb
winrar a -apDebug\Plugins -ep1 \\linux01\leecher\webserver\Skype\Skype_protocol_debug.zip Debug\skype.dll
winrar a -apDebug64-UNICODE -ep1 \\linux01\leecher\webserver\Skype\Skype_protocol_debug.zip Debug64-UNICODE\skype.pdb
winrar a -apDebug64-UNICODE\Plugins -ep1 \\linux01\leecher\webserver\Skype\Skype_protocol_debug.zip Debug64-UNICODE\skype.dll
winrar a -apDebug-UNICODE -ep1 \\linux01\leecher\webserver\Skype\Skype_protocol_debug.zip Debug-UNICODE\skype.pdb
winrar a -apDebug-UNICODE\Plugins -ep1 \\linux01\leecher\webserver\Skype\Skype_protocol_debug.zip Debug-UNICODE\skype.dll
popd
