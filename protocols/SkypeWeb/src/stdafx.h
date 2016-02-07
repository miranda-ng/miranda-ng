/*
Copyright (c) 2015-16 Miranda NG project (http://miranda-ng.org)

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
#include <time.h>
#include <string>
#include <vector>
#include <regex>
#include <map>
#include <memory>

#include <newpluginapi.h>

#include <m_protoint.h>
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
#include <m_string.h>
#include <m_json.h>
#include <m_gui.h>
#include <m_imgsrvc.h>
#include <m_xml.h>
#include <m_assocmgr.h>
#include <m_file.h>

struct CSkypeProto;

extern HINSTANCE g_hInstance;
extern char g_szMirVer[];
extern HANDLE g_hCallEvent;
extern FI_INTERFACE *fii;

#define SKYPE_ENDPOINTS_HOST "client-s.gateway.messenger.live.com"

struct LoginInfo
{
	struct
	{
		ptrA szToken;
	} api;

	struct
	{
		ptrA szToken;
		ptrA szId;
		ptrA szServer;
	} endpoint;

	ptrA szSkypename;
};


struct TRInfo
{
	std::string socketIo,
	connId,
	st,
	se,
	instance,
	ccid,
	sessId,
	sig,
	url;
	time_t lastRegistrationTime;
};

struct MessageId
{
	ULONGLONG id;
	HANDLE handle;
};

#include "version.h"
#include "resource.h"
#include "skype_menus.h"
#include "skype_dialogs.h"
#include "skype_options.h"
#include "skype_trouter.h"
#include "skype_db.h"
#include "skype_utils.h"
#include "http_request.h"
#include "requests\login.h"
#include "requests\profile.h"
#include "requests\contacts.h"
#include "requests\status.h"
#include "requests\endpoint.h"
#include "requests\capabilities.h"
#include "requests\subscriptions.h"
#include "requests\messages.h"
#include "requests\history.h"
#include "requests\poll.h"
#include "requests\avatars.h"
#include "requests\search.h"
#include "requests\chatrooms.h"
#include "requests\trouter.h"
#include "requests\mslogin.h"
#include "requests\asm\files.h"
#include "request_queue.h"
#include "skype_proto.h"

#define MODULE "Skype"

#define SKYPE_MAX_CONNECT_RETRIES 10

enum SKYPE_LOGIN_ERROR
{
	LOGIN_ERROR_UNKNOWN = 1001,
	LOGIN_ERROR_TOOMANY_REQUESTS
};

#define POLLING_ERRORS_LIMIT 3

#endif //_COMMON_H_