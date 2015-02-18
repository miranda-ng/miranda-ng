#ifndef _COMMON_H_
#define _COMMON_H_

#include <winsock2.h>
#include <windows.h>
#include <windns.h>
#include <time.h>
#include <commctrl.h>
#include <Shlwapi.h>

#include <string>
#include <sstream>
#include <iomanip>
//#include <vector>
#include <regex>
#include <map>

#include <newpluginapi.h>

#include <m_protoint.h>
#include <m_protomod.h>
#include <m_protosvc.h>

#include <m_database.h>
#include <m_langpack.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_netlib.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <m_userinfo.h>
#include <m_addcontact.h>
#include <m_message.h>
#include <m_avatars.h>
#include <m_skin.h>

#include <tox.h>
#include <toxdns.h>
#include <toxencryptsave.h>

#include "version.h"
#include "resource.h"
#include "tox_address.h"
#include "tox_transfer.h"
#include "tox_proto.h"

extern HINSTANCE g_hInstance;

#define MODULE "TOX"

#define TOX_ERROR -1

#define TOX_MAX_DISCONNECT_RETRIES 100

#define TOX_SETTINGS_ID "ToxID"
#define TOX_SETTINGS_DNS "DnsID"
#define TOX_SETTINGS_GROUP "DefaultGroup"
#define TOX_SETTINGS_AVATAR_HASH "AvatarHash"

#define TOX_SETTINGS_NODE_IPV4 "Node_%d_IPv4"
#define TOX_SETTINGS_NODE_IPV6 "Node_%d_IPv6"
#define TOX_SETTINGS_NODE_PORT "Node_%d_Port"
#define TOX_SETTINGS_NODE_PKEY "Node_%d_PubKey"
#define TOX_SETTINGS_NODE_COUNT "NodeCount"

#define TOX_DB_EVENT_TYPE_ACTION 10001

#define TOX_FILE_BLOCK_SIZE 1024 * 1024

extern HMODULE g_hToxLibrary;

template<typename T>
T CreateFunction(LPCSTR functionName)
{
	if (g_hToxLibrary == NULL)
	{
		g_hToxLibrary = LoadLibrary(L"libtox.dll");
	}
	return reinterpret_cast<T>(GetProcAddress(g_hToxLibrary, functionName));
}

int OptInit(WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ToxNodesOptionsProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif //_COMMON_H_