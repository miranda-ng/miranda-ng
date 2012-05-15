@echo off
pushd ".\bin8"
cls
echo Leere Zwischenverzeichnis
if not errorlevel 1 (
	attrib /s -s -r -h *.* > NUL
	del /s /f /q *.* > NUL
	pushd ..
	rd /s /q bin8 > NUL
)

attrib /s -s -r -h *.aps > NUL
del /s /f /q *.aps > NUL

attrib /s -s -r -h *.user > NUL
del /s /f /q *.user > NUL

attrib /s -s -r -h *.suo > NUL
del /s /f /q *.suo > NUL

attrib /s -s -r -h *.ncb > NUL
del /s /f /q *.ncb > NUL
