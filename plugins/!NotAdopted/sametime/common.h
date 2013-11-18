#ifndef _COMMON_H
#define _COMMON_H

#ifdef SAMETIME_EXPORTS
#define SAMETIME_API __declspec(dllexport)
#else
#define SAMETIME_API __declspec(dllimport)
#endif
#define _WIN32_WINNT 0x400			// for QueueUserAPC

#pragma warning( disable : 4503 4786 )

#include <windows.h>
//#include <winsock2.h>
#include <commctrl.h>
#include <process.h>
#include <malloc.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include <newpluginapi.h>
#include <statusmodes.h>
#include <m_options.h>
#include <m_langpack.h>
#include <m_system.h>
#include <m_skin.h>
#include <m_netlib.h>
#include <m_database.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_ignore.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_clc.h>
#include <m_utils.h>
#include <m_message.h>
#include <m_idle.h>
#include <m_addcontact.h>
#include <m_popup.h>

#include <m_updater.h>

#include <m_chat.h>

#include "resource.h"

// sametime stuff
extern "C" {
#include <mw_session.h>
#include <mw_cipher.h>
#include <mw_st_list.h>
//#include <mw_util.h>
#include <mw_service.h>
#include <mw_channel.h>
#include <mw_srvc_im.h>
#include <mw_srvc_aware.h>
#include <mw_srvc_resolve.h>
#include <mw_srvc_store.h>
#include <mw_srvc_place.h>
#include <mw_srvc_ft.h>
#include <mw_srvc_conf.h>
#include <mw_error.h>
#include <mw_message.h>
};

// globals

extern char PROTO[64];
extern char PROTO_GROUPS[128];

extern HINSTANCE hInst;

extern PLUGINLINK *pluginLink;
extern PLUGININFOEX pluginInfo;
extern MM_INTERFACE mmi;
extern UTF8_INTERFACE utfi;

extern HANDLE mainThread;
extern DWORD mainThreadId;

extern "C" SAMETIME_API PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion);
extern "C" SAMETIME_API int Load(PLUGINLINK *link);
extern "C" SAMETIME_API int Unload(void);

extern HANDLE hNetlibUser;

static inline void NLog(char *msg) {
	CallService(MS_NETLIB_LOG, (WPARAM)hNetlibUser, (LPARAM)msg);
}

extern int previous_status, current_status;

extern bool is_idle;

int DBGetContactSettingUtf(HANDLE hContact, char *module, char *setting, DBVARIANT *dbv);

void SetAllOffline();

extern bool unicode_chat;
extern int code_page;

#define MAX_MESSAGE_SIZE		(10 * 1024) // verified limit in official client, thx Periferral

#endif
