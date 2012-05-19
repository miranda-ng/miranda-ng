#ifndef _COMMON_INC
#define _COMMON_INC


#define _WIN32_WINNT 	0x0500
#define _WIN32_IE 		0x0400

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define VC_EXTRALEAN

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <commctrl.h>

#include "resource.h"

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_options.h>
#include <m_skin.h>
#include <m_contacts.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_popup.h>
#include <m_utils.h>
#include <m_idle.h>

#include <m_clist.h>
#include <m_clui.h>
#include <m_clc.h>

#include <m_cluiframes.h>
#include <m_genmenu.h>

#include <m_toptoolbar.h>
#include <m_trigger.h>

#include <m_fontservice.h>
#include <m_updater.h>

#include <m_alarms.h>

#include <m_icolib.h>

#define MODULE	"Alarm"

extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;

extern HANDLE hTopToolbarButton;

typedef struct ALARM_tag {
	unsigned short id;			
	char *szTitle;
	char *szDesc;
	Occurrence occurrence;
	BOOL snoozer;				
	SYSTEMTIME time;			
	unsigned short action;		
	char *szCommand;			
	char *szCommandParams;		
	BYTE sound_num;				
	int flags;
	DWORD trigger_id;			
} ALARM;

#ifndef MIID_ALARMS
#define MIID_ALARMS		{0x60ebaad1, 0x8d95, 0x4966, { 0x8b, 0xe3, 0xb1, 0xed, 0xaf, 0xa2, 0x11, 0xab}}
#endif

#endif
