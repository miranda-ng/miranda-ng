#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <time.h>

#include <map>
#include <regex>
#include <vector>
#include <string>
#include <algorithm>

#include <newpluginapi.h>
#include <m_chat_int.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_history.h>
#include <m_message.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_popup.h>
#include <m_json.h>
#include <m_avatars.h>
#include <m_icolib.h>
#include <m_skin.h>
#include <m_clist.h>
#include <m_genmenu.h>
#include <m_imgsrvc.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_idle.h>
#include <m_xstatus.h>
#include <m_extraicons.h>
#include <m_gui.h>
#include <m_http.h>
#include <m_system.h>

extern "C"
{
	#include "../../libs/zlib/src/unzip.h"
}

#include "resource.h"
#include "version.h"

#include "protobuf-c/steammessages_auth.steamclient.pb-c.h"
#include "protobuf-c/steammessages_chat.steamclient.pb-c.h"
#include "protobuf-c/steammessages_clientserver.pb-c.h"
#include "protobuf-c/steammessages_clientserver_2.pb-c.h"
#include "protobuf-c/steammessages_clientserver_appinfo.pb-c.h"
#include "protobuf-c/steammessages_clientserver_login.pb-c.h"
#include "protobuf-c/steammessages_deviceauth.steamclient.pb-c.h"
#include "protobuf-c/steammessages_friendmessages.steamclient.pb-c.h"
#include "protobuf-c/steammessages_notifications.steamclient.pb-c.h"
#include "proto.h"

#define MODULENAME "Steam"

#define STEAM_API_TIMEOUT 20
#define STEAM_API_IDLEOUT_AWAY 600
#define STEAM_API_IDLEOUT_SNOOZE 8000

#define STEAM_API_POLLING_ERRORS_LIMIT 5

class CSteamProto;

extern HANDLE hExtraXStatus;

#define STEAM_API_URL "https://api.steampowered.com"
#define STEAM_WEB_URL "https://steamcommunity.com"

// registered db event types
#define EVENTTYPE_STEAM_CHATSTATES          2000
#define STEAM_DB_GETEVENTTEXT_CHATSTATES    "/GetEventText2000"
#define STEAM_DB_EVENT_CHATSTATES_GONE      1

#include "steam_dialogs.h"

#include "api/enums.h"
#include "steam_proto.h"
#include "steam_utils.h"

#include "api/captcha.h"
#include "api/friend.h"
#include "api/pending.h"
#include "api/search.h"
#include "api/session.h"

#endif //_COMMON_H_