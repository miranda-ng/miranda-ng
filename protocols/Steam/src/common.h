#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
#include <commctrl.h>
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
#include <win2k.h>

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

#include "Steam\steam.h"

#include "steam_proto.h"

#endif //_COMMON_H_