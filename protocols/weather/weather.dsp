# Microsoft Developer Studio Project File - Name="Weather" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Weather - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "weather.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "weather.mak" CFG="Weather - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Weather - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Weather___Win32"
# PROP BASE Intermediate_Dir "Weather___Win32"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\Obj"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O1 /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W3 /O1 /Ob0 /I "..\..\include" /FAcs /Fr /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x417 /d "NDEBUG"
# ADD RSC /l 0x409 /fo"Release\Obj\weather.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Intermediate/weather.bsc"
LINK32=link.exe
# ADD BASE LINK32 user32.lib shell32.lib gdi32.lib /nologo /base:"0x67100000" /dll /machine:I386 /filealign:0x200
# SUBTRACT BASE LINK32 /pdb:none /map
# ADD LINK32 user32.lib shell32.lib gdi32.lib comdlg32.lib wsock32.lib /nologo /base:"0x72030000" /version:3.64 /dll /pdb:none /map /machine:I386 /filealign:512
# Begin Target

# Name "Weather - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\weather.c
# End Source File
# Begin Source File

SOURCE=.\weather_addstn.c
# End Source File
# Begin Source File

SOURCE=.\weather_contacts.c
# End Source File
# Begin Source File

SOURCE=.\weather_conv.c
# End Source File
# Begin Source File

SOURCE=.\weather_data.c
# End Source File
# Begin Source File

SOURCE=.\weather_http.c
# End Source File
# Begin Source File

SOURCE=.\weather_icons.c
# End Source File
# Begin Source File

SOURCE=.\weather_info.c
# End Source File
# Begin Source File

SOURCE=.\weather_ini.c
# End Source File
# Begin Source File

SOURCE=.\weather_mwin.c
# End Source File
# Begin Source File

SOURCE=.\weather_opt.c
# End Source File
# Begin Source File

SOURCE=.\weather_popup.c
# End Source File
# Begin Source File

SOURCE=.\weather_svcs.c
# End Source File
# Begin Source File

SOURCE=.\weather_update.c
# End Source File
# Begin Source File

SOURCE=.\weather_userinfo.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\m_cluiframes.h
# End Source File
# Begin Source File

SOURCE=.\m_weather.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# Begin Source File

SOURCE=.\weather.h
# End Source File
# End Group
# Begin Group "Documentations"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\copying.txt
# End Source File
# Begin Source File

SOURCE=".\Release\weather-doc\langpack_defweather.txt"
# End Source File
# Begin Source File

SOURCE=".\Release\weather-doc\sample_ini.ini"
# End Source File
# Begin Source File

SOURCE=".\weather-history.txt"
# End Source File
# Begin Source File

SOURCE=".\Release\weather-doc\weather-readme.html"
# End Source File
# Begin Source File

SOURCE=".\Release\weather-doc\weather-translation.txt"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Group "Plugin Icons"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\res\brief.ico
# End Source File
# Begin Source File

SOURCE=.\res\disabled.ico
# End Source File
# Begin Source File

SOURCE=.\res\edit.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon.ico
# End Source File
# Begin Source File

SOURCE=.\res\infologo.ico
# End Source File
# Begin Source File

SOURCE=.\res\log.ico
# End Source File
# Begin Source File

SOURCE=.\res\map.ico
# End Source File
# Begin Source File

SOURCE=.\res\more.ico
# End Source File
# Begin Source File

SOURCE=.\res\popup.ico
# End Source File
# Begin Source File

SOURCE=.\res\popup_no.ico
# End Source File
# Begin Source File

SOURCE=.\res\reload.ico
# End Source File
# Begin Source File

SOURCE=.\res\update.ico
# End Source File
# Begin Source File

SOURCE=.\res\update2.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# End Target
# End Project
