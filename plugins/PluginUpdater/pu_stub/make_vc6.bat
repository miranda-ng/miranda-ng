:: set path to your Visual studio folder here!
:: Root of Visual Developer Studio.
set VSDir=%programfiles%\Microsoft Visual Studio
:: ----------------------------------------------


:: Root of Visual Developer Studio Common files.
set VSCommonDir=%VSDir%\Common

:: Root of Visual Developer Studio installed files.
set MSDevDir=%VSDir%\Common\msdev98

:: Root of Visual C++ installed files.
set MSVCDir=%VSDir%\VC98

:: Setting environment for using Microsoft Visual C++ tools.
set INCLUDE=%MSVCDir%\ATL\INCLUDE;%MSVCDir%\INCLUDE;%MSVCDir%\MFC\INCLUDE;%INCLUDE%
set LIB=%MSVCDir%\LIB;%MSVCDir%\MFC\LIB;%LIB%
set PATH=%MSDevDir%\BIN;%MSVCDir%\BIN;%VSCommonDir%\TOOLS\WINNT;%VSCommonDir%\TOOLS;%PATH%

cl /O1 /MD -D_UNICODE -DUNICODE pu_stub.cpp

del /f /q *.obj