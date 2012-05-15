@echo off

for /F "tokens=1,2,3 delims= " %%i in (build.no) do call :WriteVer %%i %%j %%k

if not exist Release md Release
if not exist "Release/Icons" md "Release/Icons"
if not exist "Release/Plugins" md "Release/Plugins"

rem ---------------------------------------------------------------------------
rem Main modules
rem ---------------------------------------------------------------------------

pushd ..\src
call :Nmake miranda32.mak "miranda32 - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda-tools\dbtool
call :Nmake dbtool.mak "dbtool - Win32 Release"
popd
if errorlevel 1 goto :Error

rem ---------------------------------------------------------------------------
rem Protocols
rem ---------------------------------------------------------------------------

pushd ..\..\miranda\protocols\IcqOscarJ
call :Nmake IcqOscar8.mak "icqoscar8 - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\protocols\MSN
call :Nmake MSN.mak "msn - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\protocols\JabberG
call :Nmake jabber.mak "jabberg - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\protocols\AimOscar
call :Nmake aimoscar.mak "aim - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\protocols\YAHOO
call :Nmake Yahoo.mak "Yahoo - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\protocols\IRCG
call :Nmake IRC.mak "IRC - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\protocols\Gadu-Gadu
call :Nmake Gadu-Gadu.mak "GG - Win32 Release"
popd
if errorlevel 1 goto :Error

rem ---------------------------------------------------------------------------
rem Plugins
rem ---------------------------------------------------------------------------

pushd ..\..\miranda\plugins\zlib
call :Nmake zlib.mak "zlib - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\avs
call :Nmake avs.mak "avs - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\chat
call :Nmake chat.mak "chat - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\clist
call :Nmake clist.mak "clist - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\db3x
call :Nmake db3x.mak "db3x - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\db3x_mmap
call :Nmake db3x_mmap.mak "db3x_mmap - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\freeimage
call :Nmake FreeImage.mak "FreeImage - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\import
call :Nmake import.mak "import - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\modernb
call :Nmake modernb.mak "modernb - Win32 Release"
popd
if errorlevel 1 goto :Error

rem pushd ..\..\miranda\plugins\modernopt
rem call :Nmake modernopt.mak "modernopt - Win32 Release"
rem popd
rem if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\mwclist
call :Nmake mwclist.mak "mwclist - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\scriver
call :Nmake scriver.mak "scriver - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\srmm
call :Nmake srmm.mak "srmm - Win32 Release"
popd
if errorlevel 1 goto :Error

rem ---------------------------------------------------------------------------
rem Icons
rem ---------------------------------------------------------------------------

pushd ..\..\miranda\protocols\IcqOscarJ\icons_pack
call :Nmake ICONS.mak "ICONS - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\protocols\JabberG\jabber_xstatus
call :Nmake JABBER_XSTATUS.mak "JABBER_XSTATUS - Win32 Release"
popd
if errorlevel 1 goto :Error

pushd ..\..\miranda\plugins\modernb\icons_pack\
call :Nmake ICONS_MODERN.mak "ICONS_MODERN - Win32 Release"
popd
if errorlevel 1 goto :Error

rem ---------------------------------------------------------------------------
rem Zip it
rem ---------------------------------------------------------------------------

pushd Release

dir /B /S *.dll | ..\rebaser

for /F "tokens=1,2,3 delims= " %%i in (..\build.no) do call :Pack %%i %%j %%k

popd
goto :eof

:Nmake
echo.
echo ===========================================================================
echo Building %1
echo ===========================================================================
nmake /NOLOGO /f %1 CFG=%2
goto :eof

:WriteVer
set /A Version = %1
set /A SubVersion = %2
call :WriteVer2 %Version% %SubVersion% %3
goto :eof

:WriteVer2
copy m_version.h.in ..\include\m_version.h

echo #define MIRANDA_VERSION_FILEVERSION 0,%1,%2,%3                                >>..\include\m_version.h
echo #define MIRANDA_VERSION_STRING      "0.%1.%2.%3"                              >>..\include\m_version.h
echo #define MIRANDA_VERSION_DISPLAY     "0.%1.%2 alpha build #%3"                 >>..\include\m_version.h
echo #define MIRANDA_VERSION_DWORD       MIRANDA_MAKE_VERSION(0, %1, %2, %3)       >>..\include\m_version.h
echo #define MIRANDA_VERSION_CORE        MIRANDA_MAKE_VERSION(0, %1, %2, 0)        >>..\include\m_version.h
echo #define MIRANDA_VERSION_CORE_STRING "0.%1.%2.0"                               >>..\include\m_version.h
echo.                                                                              >>..\include\m_version.h
echo #endif // M_VERSION_H__                                                       >>..\include\m_version.h

for /F "delims=-/. tokens=1,2,3" %%i in ('date /T') do call :SetBuildDate %%i %%j %%k
for /F "delims=:/. tokens=1,2" %%i in ('time /T') do call :SetBuildTime %%i %%j

echo ^<?xml version="1.0" ?^>                                                      >%temp%\index.xml
echo ^<rss version="2.0"^>                                                         >>%temp%\index.xml
echo      ^<channel^>                                                              >>%temp%\index.xml
echo           ^<title^>Miranda IM Alpha Builds^</title^>                          >>%temp%\index.xml
echo           ^<link^>http://files.miranda-im.org/builds/^</link^>                >>%temp%\index.xml
echo           ^<language^>en-us^</language^>                                      >>%temp%\index.xml
echo           ^<lastBuildDate^>%yy%-%mm%-%dd% %hh%:%mn%^</lastBuildDate^>         >>%temp%\index.xml
echo           ^<item^>                                                            >>%temp%\index.xml
echo                ^<title^>Miranda 0.%1.%2 alpha %3^</title^>                    >>%temp%\index.xml
echo         ^<link^>http://files.miranda-im.org/builds/?%yy%%mm%%dd%%hh%%mn%^</link^> >>%temp%\index.xml
echo                ^<description^>                                                >>%temp%\index.xml
echo                     Miranda 0.%1.%2 alpha %3 is now available at http://files.miranda-im.org/builds/miranda-v%1a%3.zip >>%temp%\index.xml
echo                ^</description^>                                               >>%temp%\index.xml
echo                ^<pubDate^>%yy%-%mm%-%dd% %hh%:%mn%^</pubDate^>                >>%temp%\index.xml
echo                ^<category^>Nightly Builds^</category^>                        >>%temp%\index.xml
echo                ^<author^>Miranda IM Development Team^</author^>               >>%temp%\index.xml
echo           ^</item^>                                                           >>%temp%\index.xml
echo      ^</channel^>                                                             >>%temp%\index.xml
echo ^</rss^>                                                                      >>%temp%\index.xml
goto :eof

:SetBuildDate
set dd=%1
set mm=%2
set yy=%3
goto :eof

:SetBuildTime
set hh=%1
set mn=%2
goto :eof

:Pack
if %2 == 00 (
   set FileVer=v0%1a%3.7z
) else (
   set FileVer=v0%1%2a%3.7z
)

if exist "%Temp%\miranda-%FileVer%" del /Q /F "%Temp%\miranda-%FileVer%"
"%PROGRAMFILES%\7-zip\7z.exe" a -r -mx=9 "%Temp%\miranda-%FileVer%" ./* ..\ChangeLog.txt

if exist %Temp%\pdba rd /Q /S %Temp%\pdba >nul
md %Temp%\pdba
md %Temp%\pdba\plugins

copy ..\..\src\Release\miranda32.pdb                   %Temp%\pdba
copy ..\..\..\miranda-tools\dbtool\Release\dbtool.pdb  %Temp%\pdba
rem  Protocols
copy ..\..\protocols\AimOscar\Release\Aim.pdb          %Temp%\pdba\plugins
copy ..\..\protocols\IcqOscarJ\Release\ICQ.pdb         %Temp%\pdba\plugins
copy ..\..\protocols\IRCG\Release\IRC.pdb              %Temp%\pdba\plugins
copy ..\..\protocols\JabberG\Release\jabber.pdb        %Temp%\pdba\plugins
copy ..\..\protocols\MSN\Release\MSN.pdb               %Temp%\pdba\plugins
copy ..\..\protocols\Yahoo\Release\Yahoo.pdb           %Temp%\pdba\plugins
copy ..\..\protocols\Gadu-Gadu\Release\GG.pdb          %Temp%\pdba\plugins
rem  Plugins
copy ..\..\plugins\avs\Release\avs.pdb                 %Temp%\pdba\plugins
copy ..\..\plugins\chat\Release\chat.pdb               %Temp%\pdba\plugins
copy ..\..\plugins\clist\Release\clist_classic.pdb     %Temp%\pdba\plugins
copy ..\..\plugins\db3x\Release\dbx_3x.pdb             %Temp%\pdba\plugins
copy ..\..\plugins\db3x_mmap\Release\dbx_mmap.pdb      %Temp%\pdba\plugins
copy ..\..\plugins\freeimage\Release\advaimg.pdb       %Temp%\pdba\plugins
copy ..\..\plugins\import\Release\import.pdb           %Temp%\pdba\plugins
copy ..\..\plugins\modernb\Release\clist_modern.pdb    %Temp%\pdba\plugins
copy ..\..\plugins\mwclist\Release\clist_mw.pdb        %Temp%\pdba\plugins
copy ..\..\plugins\scriver\Release\scriver.pdb         %Temp%\pdba\plugins
copy ..\..\plugins\srmm\Release\srmm.pdb               %Temp%\pdba\plugins

if exist "%Temp%\miranda-pdb-%FileVer%" del /Q /F "%Temp%\miranda-pdb-%FileVer%"
"%PROGRAMFILES%\7-zip\7z.exe" a -r -mx=9 "%Temp%\miranda-pdb-%FileVer%" %Temp%\pdba/*
rd /Q /S %Temp%\pdba
goto :eof

:Error
echo Make failed
pause
goto :eof
