@echo off
rem Download tools needed for compiling installer
mkdir util

if defined ProgramFiles(x86) (
	wget -O util\7z920-x64.msi http://downloads.sourceforge.net/sevenzip/7z920-x64.msi
) else (
	wget -O util\7z920.msi http://downloads.sourceforge.net/sevenzip/7z920.msi
)

wget -P util http://www.jrsoftware.org/download.php/ispack-unicode.exe
rem end