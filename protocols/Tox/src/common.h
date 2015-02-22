#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <windns.h>
#include <time.h>
#include <commctrl.h>

#include <string>
#include <vector>
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
#include <m_chat.h>
#include <m_genmenu.h>
#include <m_clc.h>
#include <m_clistint.h>

#include <tox.h>
#include <toxdns.h>
#include <toxencryptsave.h>
#include <toxav.h>

#include "version.h"
#include "resource.h"
#include "tox_address.h"
#include "tox_options.h"
#include "tox_transfer.h"
#include "tox_proto.h"

extern HINSTANCE g_hInstance;

#define MODULE "TOX"

#define TOX_ERROR -1

#define TOX_MAX_CONNECT_RETRIES 200
#define TOX_MAX_DISCONNECT_RETRIES 200

#define TOX_INI_PATH "%miranda_path%\\Plugins\\tox.ini"

#define TOX_SETTINGS_ID "ToxID"
#define TOX_SETTINGS_DNS "DnsID"
#define TOX_SETTINGS_CHAT_ID "ChatID"
#define TOX_SETTINGS_GROUP "DefaultGroup"
#define TOX_SETTINGS_AVATAR_HASH "AvatarHash"

#define TOX_SETTINGS_NODE_PREFIX "Node_"
#define TOX_SETTINGS_NODE_IPV4 TOX_SETTINGS_NODE_PREFIX"%d_IPv4"
#define TOX_SETTINGS_NODE_IPV6 TOX_SETTINGS_NODE_PREFIX"%d_IPv6"
#define TOX_SETTINGS_NODE_PORT TOX_SETTINGS_NODE_PREFIX"%d_Port"
#define TOX_SETTINGS_NODE_PKEY TOX_SETTINGS_NODE_PREFIX"%d_PubKey"
#define TOX_SETTINGS_NODE_COUNT TOX_SETTINGS_NODE_PREFIX"Count"

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

#endif //_COMMON_H_