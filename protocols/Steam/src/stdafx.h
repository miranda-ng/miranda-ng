#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <time.h>

#include <map>
#include <vector>
#include <string>
#include <algorithm>

#include <newpluginapi.h>
#include <m_contacts.h>
#include <m_database.h>
#include <m_langpack.h>
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

#include "resource.h"
#include "version.h"

#define MODULE "Steam"
#define DB_KEY_LASTMSGTS "LastMessageTS"

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

#define now() time(0)

#include "steam_dialogs.h"
#include "http_request.h"

#include "api/enums.h"
#include "steam_proto.h"

#include "api/app_info.h"
#include "api/authorization.h"
#include "api/authorization.h"
#include "api/avatar.h"
#include "api/captcha.h"
#include "api/friend.h"
#include "api/friend_list.h"
#include "api/history.h"
#include "api/login.h"
#include "api/message.h"
#include "api/pending.h"
#include "api/poll.h"
#include "api/rsa_key.h"
#include "api/search.h"
#include "api/session.h"

#endif //_COMMON_H_