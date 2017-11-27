@echo off
set p1=%1
set p2=%2
set p3="%~2\libtox.dll"
copy /y bin\%p1%\libtox.dll %p2%
tools\cv2pdb.exe %p3%
exit 0
