#pragma once

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <time.h>

#include <map>
#include <memory>
#include <string>
#include <algorithm>

#include <newpluginapi.h>
#include <m_chat_int.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_gui.h>
#include <m_history.h>
#include <m_imgsrvc.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_newstory.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_avatars.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_skin.h>
#include <m_srmm_int.h>
#include <m_system.h>
#include <m_timezones.h>
#include <m_userinfo.h>
#include <m_utils.h>
#include <m_icolib.h>
#include <m_clist.h>
#include <m_http.h>
#include "../../libs/zlib/src/zlib.h"
#include "../../libs/libqrencode/src/qrencode.h"

#include "resource.h"
#include "version.h"

#include "max_proto.h"

#define MODULENAME "Max"

#define DB_KEY_DEVICEID "DeviceId"
#define DB_KEY_LOGIN_TOKEN "LoginToken"
#define DB_KEY_MAX_UID     "MaxUid"
#define DB_KEY_MAX_CHATID  "MaxChatId"
/// 0=unknown (legacy), 1=seen in server contacts[] merge, 2=created from dialog/live msg only (not address book).
#define DB_KEY_MAX_PEER_ORIGIN "MaxPeerOrigin"
#define MAX_PEER_ORIGIN_UNKNOWN  0
#define MAX_PEER_ORIGIN_CONTACTS 1
#define MAX_PEER_ORIGIN_CHATONLY 2
#define DB_KEY_MY_MAX_ID   "MyMaxId"
/// 1 = Max peer is a bot (`options` contains "BOT" from server): Status mirrors protocol online/offline.
#define DB_KEY_MAX_IS_BOT "MaxIsBot"
#define DB_KEY_DEFAULT_GROUP "DefaultGroup"
#define DB_KEY_AVATAR_URL    "AvatarUrl"

// Browser-like HTTP fingerprint (WebSocket + avatar downloads)
#define MAX_HTTP_ORIGIN_HEADER "https://web.max.ru"
/// Chromium-based Edge (matches web.max.ru / QR flow; used for WS, HTTP, and handshake `headerUserAgent`).
#define MAX_HTTP_USER_AGENT    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/147.0.0.0 Safari/537.36 Edg/147.0.0.0"

bool MaxJsonKeyIsSensitiveForLog(const char *name);
void MaxRedactJsonForLog(JSONNode &n);
CMStringA MaxRedactUrlForLog(const char *szUrl);
CMStringA MaxRedactTextForLog(const char *szText, size_t cbLimit = 0);
