#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>
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
#include <m_string.h>
#include <m_json.h>
#include <m_timezones.h>

struct CSkypeProto;

#include "version.h"
#include "resource.h"
#include "skype_icons.h"
#include "skype_menus.h"
#include "http_request.h"
#include "requests\login.h"
#include "requests\logout.h"
#include "requests\profile.h"
#include "requests\contacts.h"
#include "requests\status.h"
#include "requests\reg_info.h"
#include "requests\endpoint.h"
#include "request_queue.h"
#include "skype_proto.h"

extern HINSTANCE g_hInstance;

#define MODULE "SKYPE"

#define SKYPE_SETTINGS_ID "Skypename"
#define SKYPE_SETTINGS_PASSWORD "Password"
#define SKYPE_SETTINGS_GROUP "DefaultGroup"

#endif //_COMMON_H_