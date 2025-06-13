#ifndef _COMMON_H_
#define _COMMON_H_

#include <windows.h>

#include <malloc.h>
#include <time.h>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <memory>
#include <functional>

#include <newpluginapi.h>

#include <m_protoint.h>
#include <m_protosvc.h>

#include <m_database.h>
#include <m_langpack.h>
#include <m_clistint.h>
#include <m_options.h>
#include <m_netlib.h>
#include <m_popup.h>
#include <m_icolib.h>
#include <m_userinfo.h>
#include <m_timezones.h>
#include <m_contacts.h>
#include <m_message.h>
#include <m_avatars.h>
#include <m_skin.h>
#include <m_chat_int.h>
#include <m_genmenu.h>
#include <m_clc.h>
#include <m_json.h>
#include <m_gui.h>
#include <m_imgsrvc.h>
#include <m_xml.h>
#include <m_assocmgr.h>
#include <m_file.h>

extern char g_szMirVer[];
extern HANDLE g_hCallEvent;

struct MessageId
{
	ULONGLONG id;
	HANDLE handle;
};

#include "resource.h"
#include "version.h"
#include "teams_menus.h"
#include "teams_utils.h"

#define MODULENAME "Teams"

class CTeamsProto;

/////////////////////////////////////////////////////////////////////////////////////////

#define SKYPEWEB_CLIENTINFO_NAME "swx-skype.com"
#define SKYPEWEB_CLIENTINFO_VERSION "908/1.85.0.29"

enum SkypeHost
{
	HOST_API,
	HOST_CONTACTS,
	HOST_DEFAULT,
	HOST_DEFAULT_V2,
	HOST_LOGIN,
	HOST_TEAMS,
	HOST_TEAMS_API,
	HOST_CHATS,
	HOST_PRESENCE,
	HOST_OTHER
};

struct AsyncHttpRequest : public MTHttpRequest<CTeamsProto>
{
	SkypeHost m_host;
	MCONTACT hContact = 0;

	AsyncHttpRequest(int type, SkypeHost host, LPCSTR url = nullptr, MTHttpRequestHandler pFunc = nullptr);

	void AddAuthentication(CTeamsProto *ppro);
};

#include "teams_proto.h"

#include "requests/chatrooms.h"

#endif //_COMMON_H_
