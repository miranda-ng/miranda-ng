#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include <time.h>

#include <newpluginapi.h>
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
#include <m_string.h>
#include <m_freeimage.h>
#include <m_imgsrvc.h>
#include <m_protocols.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protoint.h>
#include <m_idle.h>
#include <m_xstatus.h>
#include <m_extraicons.h>
#include <m_gui.h>

#include <map>
#include <vector>
#include <string>
#include <algorithm>

#include "resource.h"
#include "version.h"

#define MODULE "Steam"

#define STEAM_API_TIMEOUT 30
#define STEAM_API_IDLEOUT_AWAY 600
#define STEAM_API_IDLEOUT_SNOOZE 8000

class CSteamProto;
extern HINSTANCE g_hInstance;

extern HANDLE hExtraXStatus;

#define STEAM_API_URL "https://api.steampowered.com"
#define STEAM_WEB_URL "https://steamcommunity.com"

#include "steam_dialogs.h"
#include "http_request.h"
#include "request_queue.h"
#include "api\authorization.h"
#include "api\authorization.h"
#include "api\avatar.h"
#include "api\captcha.h"
#include "api\friend.h"
#include "api\friend_list.h"
#include "api\login.h"
#include "api\message.h"
#include "api\pending.h"
#include "api\poll.h"
#include "api\rsa_key.h"
#include "api\search.h"
#include "api\session.h"
#include "steam_proto.h"

#endif //_COMMON_H_