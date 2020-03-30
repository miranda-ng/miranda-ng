/*
Copyright (c) 2015-20 Miranda NG team (https://miranda-ng.org)

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

#define SKYPE_ENDPOINTS_HOST "client-s.gateway.messenger.live.com"

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
#include "skype_utils.h"
#include "skype_db.h"
#include "http_request.h"
#include "skype_proto.h"

#include "requests/avatars.h"
#include "requests/capabilities.h"
#include "requests/chatrooms.h"
#include "requests/contacts.h"
#include "requests/endpoint.h"
#include "requests/files.h"
#include "requests/history.h"
#include "requests/login.h"
#include "requests/messages.h"
#include "requests/mslogin.h"
#include "requests/oauth.h"
#include "requests/poll.h"
#include "requests/profile.h"
#include "requests/search.h"
#include "requests/status.h"
#include "requests/subscriptions.h"
#include "requests/trouter.h"
#include "request_queue.h"

void SkypeHttpResponse(const NETLIBHTTPREQUEST *response, void *arg);

class SkypeResponseDelegateBase
{
protected:
	CSkypeProto *proto;
public:
	SkypeResponseDelegateBase(CSkypeProto *ppro) : proto(ppro) {}
	virtual void Invoke(const NETLIBHTTPREQUEST *) = 0;
	virtual ~SkypeResponseDelegateBase(){};
};

class SkypeResponseDelegate : public SkypeResponseDelegateBase
{
	SkypeResponseCallback pfnResponseCallback;
public:
	SkypeResponseDelegate(CSkypeProto *ppro, SkypeResponseCallback callback) : SkypeResponseDelegateBase(ppro), pfnResponseCallback(callback) {}

	virtual void Invoke(const NETLIBHTTPREQUEST *response) override
	{
		(proto->*(pfnResponseCallback))(response);
	}
};

class SkypeResponseDelegateWithArg : public SkypeResponseDelegateBase
{
	SkypeResponseWithArgCallback pfnResponseCallback;
	void *arg;
public:
	SkypeResponseDelegateWithArg(CSkypeProto *ppro, SkypeResponseWithArgCallback callback, void *p) :
		SkypeResponseDelegateBase(ppro),
		pfnResponseCallback(callback),
		arg(p)
	{}

	virtual void Invoke(const NETLIBHTTPREQUEST *response) override
	{
		(proto->*(pfnResponseCallback))(response, arg);
	}
};

template <typename F>
class SkypeResponseDelegateLambda : public SkypeResponseDelegateBase
{
	F lCallback;
public:
	SkypeResponseDelegateLambda(CSkypeProto *ppro, F &callback) : SkypeResponseDelegateBase(ppro), lCallback(callback) {}

	virtual void Invoke(const NETLIBHTTPREQUEST *response) override
	{
		lCallback(response);
	}
};


#define MODULE "Skype"

#define SKYPE_MAX_CONNECT_RETRIES 10

enum SKYPE_LOGIN_ERROR
{
	LOGIN_ERROR_UNKNOWN = 1001,
	LOGIN_ERROR_TOOMANY_REQUESTS
};

#define POLLING_ERRORS_LIMIT 3

#endif //_COMMON_H_