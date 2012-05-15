@ECHO off
CLS
SET ZIP="D:\Program Files\7-Zip\7z.exe"

CALL "%VS71COMNTOOLS%\vsvars32.bat"
ECHO.
ECHO Compilation in progress...
ECHO.
devenv.exe /rebuild "Release" tipper_ym.vcproj
devenv.exe /rebuild "Release (ansi)" tipper_ym.vcproj

CALL "%VS90COMNTOOLS%\vsvars32.bat"
ECHO.
ECHO Compilation in progress...
ECHO.
devenv.exe tipper_ym_9.vcproj /project "tipper_ym_9.vcproj" /projectconfig "Release|x64" /rebuild

MKDIR ansi
MKDIR x64
COPY /Y "Release\tipper.dll" tipper.dll
COPY /Y "Release64\tipper.dll" "x64\tipper.dll"
COPY /Y "Release (ansi)\tipper.dll" "ansi\tipper.dll"

MKDIR "skins\Tipper\XPMC"
MKDIR "skins\Tipper\Tipper classic"
XCOPY /Y "docs\sample skin" "skins\Tipper\XPMC"
XCOPY /Y "docs\tipper classic" "skins\Tipper\Tipper classic"

RMDIR /S /Q upload
MKDIR "upload\updater"
MKDIR "upload\addons"
%ZIP% a -mx9 upload\tipper-ym.zip tipper.dll ansi\tipper.dll x64\tipper.dll docs\*.txt docs\*.ini docs\*.h skins

MKDIR plugins
COPY /Y "Release\tipper.dll" "plugins\tipper.dll"
%ZIP% a -mx9 upload\updater\tipper-ym.zip plugins\tipper.dll docs\*.txt docs\*.ini docs\*.h

COPY /Y "Release (ansi)\tipper.dll" "plugins\tipper.dll"
%ZIP% a -mx9 upload\updater\tipper-ym_ansi.zip plugins\tipper.dll docs\*.txt docs\*.ini docs\*.h
%ZIP% a -mx9 upload\tipper-ym_src.zip *.cpp *.h *.rc *.vcproj *.ds* docs\*.txt docs\*.ini docs\*.h sdk\*.h res\*.ico

COPY /Y "Release\tipper.dll" "tipper.dll"
%ZIP% a -mx9 upload\addons\tipper-ym.zip tipper.dll docs\*.txt docs\*.ini docs\*.h skins
COPY /Y "Release (ansi)\tipper.dll" "tipper.dll"
%ZIP% a -mx9 upload\addons\tipper-ym_ansi.zip tipper.dll docs\*.txt docs\*.ini docs\*.h
COPY /Y "Release64\tipper.dll" "tipper.dll"
%ZIP% a -mx9 upload\addons\tipper-ym_x64.zip tipper.dll docs\*.txt docs\*.ini docs\*.h skins

DEL tipper.dll
RMDIR /S /Q x64
RMDIR /S /Q ansi
RMDIR /S /Q plugins
RMDIR /S /Q skins
RMDIR /S /Q "Release"
RMDIR /S /Q "Release64"
RMDIR /S /Q "Release (ansi)"

TIMEOUT 10