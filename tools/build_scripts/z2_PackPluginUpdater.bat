title MirandaNG packer
rem Packing compiled MirandaNG
rem use with param 32 or 64
rem set target platform %tp% from first parameter
set tp=%1
set dp=%2
if "%tp%"=="" (echo "please specify target platform 32 or 64!"&&pause&&goto :EOF)

set comp=%3
if "%comp%"=="" (echo "please specify target compiler folder!" && pause && goto :EOF)

call a_SetVar%tp%.bat

if exist %Arch% rd /Q /S "%Arch%" >nul

rem making temp dirs
mkdir %Arch%

if not exist "%comp%\Release%tp%" goto EOF

:FILECHECK
pushd "%comp%\Release%tp%"
if exist %Errors% del /F /Q %Errors%
if exist %NoErrors% del /F /Q %NoErrors%
for /f %%a in (..\..\%comp%\z2_PackPluginUpdater_x%tp%.txt) do (if not exist %%a echo %%a >> %Errors%)

:COMPRESS
REM Core
%ZipIt% "%Arch%\miranda%tp%.zip" "Miranda%tp%.exe" -i@..\..\z2_PackPluginUpdater.txt
REM end Core

REM Langpacks
for /f %%a in ('dir Languages\lang*.txt /B /L') do (%ZipIt% "%Arch%\Languages\%%~na.zip" "Languages\%%~nXa")
REM end Langpacks

REM Protocols
REM for /f %%a in ('dir Protocols\*.dll /B /L') do (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Plugins\%%~nXa")
REM end Protocols

REM Plugins
for /f %%a in ('dir plugins\*.dll /B /L') do (
	rem check for special plugins and pack additional files first.
	if /I "%%a"=="Actman.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Plugins\services.ini")
	if /I "%%a"=="BASS_interface.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Plugins\BASS\*.dll")
	if /I "%%a"=="Boltun.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Plugins\boltun.mindw")
	if /I "%%a"=="Clist_modern.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Plugins\AVS.dll" "Icons\Toolbar_icons.dll")
	if /I "%%a"=="CmdLine.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "MimCmd.exe")
	if /I "%%a"=="CurrencyRates.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_CurrencyRates.dll" "Plugins\CurrencyRates\*.xml" "Plugins\CurrencyRates\CurrencyRatesChart.exe")
	if /I "%%a"=="Dbx_mdbx.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "mdbx_chk.exe" "mdbx_dump.exe" "mdbx_load.exe" "Libs\libmdbx.mir")
	if /I "%%a"=="Dummy.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Dummy.dll")
	if /I "%%a"=="Facebook.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Facebook.dll")
	if /I "%%a"=="FavContacts.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Gadgets\favcontacts.gadget")
	if /I "%%a"=="Fingerprint.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Fp_icons.dll")
	if /I "%%a"=="FTPFile.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Libs\libcurl.mir" "Libs\libssh2.mir")
	if /I "%%a"=="GG.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_GG.dll")
	if /I "%%a"=="HistoryPP.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\HistoryPP_icons.dll")
	if /I "%%a"=="ICQ.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_ICQ.dll" "Icons\Proto_MRA.dll")
	if /I "%%a"=="IRC.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_IRC.dll")
	if /I "%%a"=="Import.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Plugins\Import\*.ini")
	if /I "%%a"=="Jabber.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Jabber.dll" "Icons\xStatus_Jabber.dll" "Libs\libsignal.mir")
	if /I "%%a"=="Listeningto.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Plugins\Listeningto\*.dll")
	if /I "%%a"=="MinecraftDynmap.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_MinecraftDynmap.dll")
	if /I "%%a"=="MirLua.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Scripts\*.dll" "Libs\lua*.dll")
	if /I "%%a"=="mRadio.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_mRadio.dll" "Plugins\BASS\*.dll")
	if /I "%%a"=="NewsAggregator.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_NewsAggregator.dll")
	if /I "%%a"=="NotifyAnything.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "SendLog*.exe")
	if /I "%%a"=="PluginUpdater.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "pu_stub.exe")
	if /I "%%a"=="PopupPlus.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "libs\libTextControl.mir")
	if /I "%%a"=="RemovePersonalSettings.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "RemovePersonalSettings.ini")
	if /I "%%a"=="Sametime.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Sametime.dll")
	if /I "%%a"=="SecureIM.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Plugins\CryptoPP.dll" "Icons\SecureIM_icons.dll")
	if /I "%%a"=="SkypeWeb.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Skype.dll")
	if /I "%%a"=="SpellChecker.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Flags_icons.dll" "Libs\hunspell.mir")
	if /I "%%a"=="Steam.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Steam.dll")
	if /I "%%a"=="TabSRMM.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Plugins\NewEventNotify.dll" "Icons\TabSRMM_icons.dll")
	if /I "%%a"=="Telegram.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Telegram.dll")
	if /I "%%a"=="Tox.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Tox.dll" "Libs\libtox.mir" "Libs\libsodium.mir" "Libs\pthreads.mir")
	if /I "%%a"=="Twitter.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Twitter.dll")
	if /I "%%a"=="UInfoEx.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\UInfoEx_icons.dll")
	if /I "%%a"=="VKontakte.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_VKontakte.dll")
	if /I "%%a"=="Watrack.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Watrack_buttons.dll" "Icons\Watrack_icons.dll" "Plugins\player.ini")
	if /I "%%a"=="Weather.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Weather.dll" "Plugins\Weather\*.ini")
	if /I "%%a"=="WhatsApp.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Libs\libsignal.mir" "libs\libqrencode.mir")
	if /I "%%a"=="YAMN.dll" (%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_YAMN.dll")

	if /I "%%a"=="Discord.dll" (
		xcopy /S /V /Y "../../redist/x%tp%/gstreamer/opus-0.dll" "Libs"
		%ZipIt% "%Arch%\Plugins\%%~na.zip" "Icons\Proto_Discord.dll" "Libs\libsodium.mir" "Libs\opus-0.dll"
		del "Libs/opus-0.dll"
	)

	if /I "%%a"=="Jingle.dll" (
		ren Libs Libs2
		mkdir Libs
		xcopy /S /V /Y "../../redist/x%tp%/gstreamer" "Libs"
		%ZipIt% "%Arch%\Plugins\%%~na.zip" "Libs\*.dll" "Plugins\VoiceService.dll"
		rd /S /Q Libs
		ren Libs2 Libs
	)
	rem now adding plugin itself into archive
	%ZipIt% "%Arch%\Plugins\%%~na.zip" %%a
)
REM end Plugins

REM Icons
for /f %%a in ('dir .\Icons\*.dll /B /L') do (%ZipIt% "%Arch%\Icons\%%~na.zip" "-ir-!Icons\%%~nXa")
REM end Icons

REM CRC32
copy /y /v hashes.txt %Arch%\hash.txt
pushd %Arch%

if /i '%dp%' == 'DEV_N_STABLE' (
  echo ;DoNotSwitchToStable > hashes.txt
) else if /i '%dp%' == 'DEV_ONLY' (
  echo DoNotSwitchToStable > hashes.txt
) else (
  echo ;StableAllowed > hashes.txt
)

for /f "tokens=1,2 delims= " %%i in (hash.txt) do call :WriteLine %%i %%j
%ZipIt% "hashes.zip" hashes.txt
del /f /q hash*.txt
popd
REM end CRC32

if not exist %Errors% echo "No errors!" >> %NoErrors%

cd ..

:EOF
popd
goto :eof
:: END OF WORK

:WriteLine
for /F "tokens=1,2 delims=:" %%i in ('%SourceDir%\tools\crc32.exe %~dpn1.zip') do echo %1 %2 %%j >> hashes.txt
goto :eof
