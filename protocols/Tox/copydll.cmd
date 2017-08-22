echo off
cd /d ~dp%0
copy /y bin\%1\libtox.* "%2"
tools\cv2pdb.exe "%2\libtox.dll"
exit 0
