# Microsoft Developer Studio Project File - Name="tipper_ym" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=tipper_ym - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tipper_ym.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tipper_ym.mak" CFG="tipper_ym - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tipper_ym - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tipper_ym - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "tipper_ym - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tipper_ym - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "TIPPER_EXPORTS" /D MICROSOFT_LAYER_FOR_UNICODE=1 /D "_UNICODE" /YX /GZ /c
# ADD CPP /nologo /MTd /Gm /GX /ZI /Od /I "../../include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "TIPPER_EXPORTS" /D MICROSOFT_LAYER_FOR_UNICODE=1 /D "_UNICODE" /YX /GZ /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib unicows.lib comctl32.lib ws2_32.lib msimg32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"../../bin/Debug/Plugins/tipper_ym.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib unicows.lib comctl32.lib ws2_32.lib msimg32.lib /nologo /subsystem:windows /dll /pdb:"../../bin/Debug/Plugins/tipper_ym.pdb" /debug /machine:IX86 /out:"../../bin/Debug/Plugins/tipper_ym.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "tipper_ym - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseW"
# PROP BASE Intermediate_Dir "ReleaseW"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseW"
# PROP Intermediate_Dir "ReleaseW"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /Gm /GX /Zi /Os /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "TIPPER_EXPORTS" /D "_UNICODE" /D "UNICODE" /D MICROSOFT_LAYER_FOR_UNICODE=1 /YX /TP /c
# ADD CPP /nologo /MT /Gm /GX /Zi /Os /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "TIPPER_EXPORTS" /D "_UNICODE" /D "UNICODE" /D MICROSOFT_LAYER_FOR_UNICODE=1 /YX /TP /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib unicows.lib comctl32.lib ws2_32.lib msimg32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /nodefaultlib:"Kernel32.lib Advapi32.lib User32.lib Gdi32.lib Shell32.lib Comdlg32.lib Version.lib Mpr.lib Rasapi32.lib Winmm.lib Winspool.lib Vfw32.lib Secur32.lib Oleacc.lib Oledlg.lib Sensapi.lib" /out:"../../bin/ReleaseW/Plugins/tipper_ym.dll" /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib unicows.lib comctl32.lib ws2_32.lib msimg32.lib /nologo /subsystem:windows /dll /pdb:"../../bin/ReleaseW/Plugins/tipper_ym.pdb" /debug /machine:IX86 /nodefaultlib:"Kernel32.lib Advapi32.lib User32.lib Gdi32.lib Shell32.lib Comdlg32.lib Version.lib Mpr.lib Rasapi32.lib Winmm.lib Winspool.lib Vfw32.lib Secur32.lib Oleacc.lib Oledlg.lib Sensapi.lib" /out:"../../bin/ReleaseW/Plugins/tipper_ym.dll" /pdbtype:sept /opt:ref /opt:icf
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "tipper_ym - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /GX /Zi /Os /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "TIPPER_EXPORTS" /D "_MBCS" /YX /c
# ADD CPP /nologo /MT /GX /Zi /Os /I "../../include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" /D "TIPPER_EXPORTS" /D "_MBCS" /FR /YX /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ws2_32.lib msimg32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /out:"../../bin/Release/Plugins/tipper_ym.dll" /pdbtype:sept /opt:ref /opt:icf
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ws2_32.lib msimg32.lib /nologo /subsystem:windows /dll /pdb:"../../bin/Release/Plugins/tipper_ym.pdb" /debug /machine:IX86 /out:"../../bin/Release/Plugins/tipper_ym.dll" /pdbtype:sept /opt:ref /opt:icf
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "tipper_ym - Win32 Debug"
# Name "tipper_ym - Win32 Release Unicode"
# Name "tipper_ym - Win32 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=.\bitmap_func.cpp
DEP_CPP_BITMA=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\bitmap_func.h"\
	".\common.h"\
	".\message_pump.h"\
	".\mir_dblists.h"\
	".\mir_smileys.h"\
	".\options.h"\
	".\popwin.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_tipper.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	".\translations.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	
# End Source File
# Begin Source File

SOURCE=.\message_pump.cpp
DEP_CPP_MESSA=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\bitmap_func.h"\
	".\common.h"\
	".\message_pump.h"\
	".\mir_dblists.h"\
	".\mir_smileys.h"\
	".\options.h"\
	".\popwin.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_tipper.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	".\str_utils.h"\
	".\subst.h"\
	".\translations.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	

!IF  "$(CFG)" == "tipper_ym - Win32 Debug"

# ADD CPP /nologo /GX /YX /GZ

!ELSEIF  "$(CFG)" == "tipper_ym - Win32 Release Unicode"

# ADD CPP /nologo /GX /YX

!ELSEIF  "$(CFG)" == "tipper_ym - Win32 Release"

# ADD CPP /nologo /GX /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mir_dblists.cpp
DEP_CPP_MIR_D=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\common.h"\
	".\mir_dblists.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	
# End Source File
# Begin Source File

SOURCE=.\mir_smileys.cpp
DEP_CPP_MIR_S=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\bitmap_func.h"\
	".\common.h"\
	".\mir_dblists.h"\
	".\mir_smileys.h"\
	".\options.h"\
	".\popwin.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_tipper.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	".\translations.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	
# End Source File
# Begin Source File

SOURCE=.\options.cpp
DEP_CPP_OPTIO=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\bitmap_func.h"\
	".\common.h"\
	".\message_pump.h"\
	".\mir_dblists.h"\
	".\mir_smileys.h"\
	".\options.h"\
	".\popwin.h"\
	".\preset_items.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_tipper.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	".\str_utils.h"\
	".\translations.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	

!IF  "$(CFG)" == "tipper_ym - Win32 Debug"

# ADD CPP /nologo /GX /YX /GZ

!ELSEIF  "$(CFG)" == "tipper_ym - Win32 Release Unicode"

# ADD CPP /nologo /GX /YX

!ELSEIF  "$(CFG)" == "tipper_ym - Win32 Release"

# ADD CPP /nologo /GX /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\popwin.cpp
DEP_CPP_POPWI=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\bitmap_func.h"\
	".\common.h"\
	".\message_pump.h"\
	".\mir_dblists.h"\
	".\mir_smileys.h"\
	".\options.h"\
	".\popwin.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_tipper.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	".\str_utils.h"\
	".\subst.h"\
	".\translations.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	

!IF  "$(CFG)" == "tipper_ym - Win32 Debug"

!ELSEIF  "$(CFG)" == "tipper_ym - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "tipper_ym - Win32 Release"

# ADD CPP /nologo /GX /YX /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\preset_items.cpp
DEP_CPP_PRESE=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\bitmap_func.h"\
	".\common.h"\
	".\mir_dblists.h"\
	".\mir_smileys.h"\
	".\options.h"\
	".\popwin.h"\
	".\preset_items.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_tipper.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	".\translations.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	
# End Source File
# Begin Source File

SOURCE=.\str_utils.cpp
DEP_CPP_STR_U=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\common.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	".\str_utils.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	
# End Source File
# Begin Source File

SOURCE=.\subst.cpp
DEP_CPP_SUBST=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\bitmap_func.h"\
	".\common.h"\
	".\mir_dblists.h"\
	".\mir_smileys.h"\
	".\options.h"\
	".\popwin.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_tipper.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	".\str_utils.h"\
	".\subst.h"\
	".\translations.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	
# End Source File
# Begin Source File

SOURCE=.\tipper.cpp
DEP_CPP_TIPPE=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\bitmap_func.h"\
	".\common.h"\
	".\message_pump.h"\
	".\mir_dblists.h"\
	".\mir_smileys.h"\
	".\options.h"\
	".\popwin.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_tipper.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	".\str_utils.h"\
	".\tipper.h"\
	".\translations.h"\
	".\version.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	

!IF  "$(CFG)" == "tipper_ym - Win32 Debug"

!ELSEIF  "$(CFG)" == "tipper_ym - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "tipper_ym - Win32 Release"

# ADD CPP /nologo /GX /Yc"common.h" /GZ

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\translations.cpp
DEP_CPP_TRANS=\
	"..\..\include\m_avatars.h"\
	"..\..\include\m_awaymsg.h"\
	"..\..\include\m_button.h"\
	"..\..\include\m_clc.h"\
	"..\..\include\m_clist.h"\
	"..\..\include\m_clui.h"\
	"..\..\include\m_cluiframes.h"\
	"..\..\include\m_contacts.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_fontservice.h"\
	"..\..\include\m_icolib.h"\
	"..\..\include\m_icq.h"\
	"..\..\include\m_idle.h"\
	"..\..\include\m_imgsrvc.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_skin.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\win2k.h"\
	".\common.h"\
	".\sdk\m_ersatz.h"\
	".\sdk\m_fingerprint.h"\
	".\sdk\m_flags.h"\
	".\sdk\m_metacontacts.h"\
	".\sdk\m_newawaysys.h"\
	".\sdk\m_simpleaway.h"\
	".\sdk\m_smileyadd.h"\
	".\sdk\m_tipper.h"\
	".\sdk\m_updater.h"\
	".\sdk\m_variables.h"\
	".\str_utils.h"\
	".\translations.h"\
	{$(INCLUDE)}"inttypes.h"\
	{$(INCLUDE)}"m_freeimage.h"\
	{$(INCLUDE)}"m_plugins.h"\
	{$(INCLUDE)}"m_protocols.h"\
	{$(INCLUDE)}"m_system.h"\
	{$(INCLUDE)}"m_utils.h"\
	{$(INCLUDE)}"statusmodes.h"\
	{$(INCLUDE)}"stdint.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\bitmap_func.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\sdk\m_tipper.h
# End Source File
# Begin Source File

SOURCE=.\message_pump.h
# End Source File
# Begin Source File

SOURCE=.\mir_dblists.h
# End Source File
# Begin Source File

SOURCE=.\mir_smileys.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\popwin.h
# End Source File
# Begin Source File

SOURCE=.\preset_items.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\str_utils.h
# End Source File
# Begin Source File

SOURCE=.\subst.h
# End Source File
# Begin Source File

SOURCE=.\tipper.h
# End Source File
# Begin Source File

SOURCE=.\translations.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav"
# Begin Source File

SOURCE=.\res\copy_all_items.ico
# End Source File
# Begin Source File

SOURCE=.\res\copy_item.ico
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# End Group
# End Target
# End Project
