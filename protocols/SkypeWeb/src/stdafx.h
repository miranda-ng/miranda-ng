/*
Copyright (c) 2015-22 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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

struct CSkypeProto;

extern char g_szMirVer[];
extern HANDLE g_hCallEvent;

struct MessageId
{
	ULONGLONG id;
	HANDLE handle;
};

struct CMPlugin : public ACCPROTOPLUGIN<CSkypeProto>
{
	CMPlugin();

	CMStringA szDefaultServer;

	int Load() override;
	int Unload() override;
};

#include "version.h"
#include "resource.h"
#include "skype_menus.h"
#include "skype_utils.h"
#include "skype_db.h"
#include "skype_proto.h"

/////////////////////////////////////////////////////////////////////////////////////////

#define SKYPEWEB_CLIENTINFO_NAME "swx-skype.com"
#define SKYPEWEB_CLIENTINFO_VERSION "908/1.85.0.29"

enum SkypeHost
{
	HOST_API,
	HOST_CONTACTS,
	HOST_DEFAULT,
	HOST_GRAPH,
	HOST_LOGIN,
	HOST_OTHER
};

struct AsyncHttpRequest : public MTHttpRequest<CSkypeProto>
{
	SkypeHost m_host;
	MCONTACT hContact = 0;

	AsyncHttpRequest(int type, SkypeHost host, LPCSTR url = nullptr, MTHttpRequestHandler pFunc = nullptr);
};

#include "requests/avatars.h"
#include "requests/capabilities.h"
#include "requests/chatrooms.h"
#include "requests/contacts.h"
#include "requests/endpoint.h"
#include "requests/files.h"
#include "requests/history.h"
#include "requests/login.h"
#include "requests/messages.h"
#include "requests/oauth.h"
#include "requests/poll.h"
#include "requests/profile.h"
#include "requests/search.h"
#include "requests/status.h"
#include "requests/subscriptions.h"

#define MODULE "Skype"

enum SKYPE_LOGIN_ERROR
{
	LOGIN_ERROR_UNKNOWN = 1001,
	LOGIN_ERROR_TOOMANY_REQUESTS
};

#define POLLING_ERRORS_LIMIT 3

#endif //_COMMON_H_