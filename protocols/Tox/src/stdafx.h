#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <commctrl.h>

#include <io.h>
#include <malloc.h>
#include <time.h>
#include <msapi/comptr.h>

#include <vector>
#include <regex>
#include <map>

#include <newpluginapi.h>

#include <m_protoint.h>
#include <m_protosvc.h>

#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_netlib.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <m_userinfo.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_avatars.h>
#include <m_skin.h>
#include <m_chat_int.h>
#include <m_genmenu.h>
#include <m_clc.h>
#include <m_clistint.h>
#include <m_gui.h>
#include <m_folders.h>
#include <m_assocmgr.h>
#include <m_json.h>
#include <m_http.h>

#include <tox.h>
#include <toxencryptsave.h>

class CToxProto;

#define now() time(0)

#define MODULE "Tox"

#define TOX_ERROR -1

#define TOX_DEFAULT_INTERVAL 50
#define TOX_CHECKING_INTERVAL 1000

#define TOX_INI_PATH "%miranda_path%\\Plugins\\tox.ini"
#define TOX_JSON_PATH L"%miranda_userdata%\\tox.json"

#define TOX_SETTINGS_ID "ToxID"
#define TOX_SETTINGS_DNS "DnsID"
#define TOX_SETTINGS_CHAT_ID "ChatID"
#define TOX_SETTINGS_PASSWORD "Password"
#define TOX_SETTINGS_GROUP "DefaultGroup"
#define TOX_SETTINGS_AVATAR_HASH "AvatarHash"

#define TOX_SETTINGS_NODE_PREFIX "Node_"
#define TOX_SETTINGS_NODE_IPV4 TOX_SETTINGS_NODE_PREFIX"%d_IPv4"
#define TOX_SETTINGS_NODE_IPV6 TOX_SETTINGS_NODE_PREFIX"%d_IPv6"
#define TOX_SETTINGS_NODE_PORT TOX_SETTINGS_NODE_PREFIX"%d_Port"
#define TOX_SETTINGS_NODE_PKEY TOX_SETTINGS_NODE_PREFIX"%d_PubKey"
#define TOX_SETTINGS_NODE_COUNT TOX_SETTINGS_NODE_PREFIX"Count"

#define DB_EVENT_ACTION 10000 + TOX_MESSAGE_TYPE_ACTION
#define DB_EVENT_CORRECTION 10000 + TOX_MESSAGE_TYPE_CORRECTION

#define TOX_MAX_AVATAR_SIZE 1 << 16 // 2 ^ 16 bytes

#include "version.h"
#include "resource.h"
#include "tox_menus.h"
#include "tox_address.h"
#include "tox_dialogs.h"
#include "tox_options.h"
#include "tox_transfer.h"
#include "tox_proto.h"

#include "http_request.h"

extern HANDLE hProfileFolderPath;

#endif //_COMMON_H_