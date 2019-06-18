/*

Facebook plugin for Miranda NG
Copyright © 2019 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#define FB_HOST_BAPI "https://b-api.facebook.com"

#define FACEBOOK_MESSAGE_LIMIT 100000

#include "../../../miranda-private-keys/Facebook/app_secret.h"

class FacebookProto;

struct AsyncHttpRequest : public MTHttpRequest<FacebookProto>
{
	struct Param
	{
		Param(const char *p1, const char *p2) :
			key(p1), val(p2)
		{}
		
		CMStringA key, val;
	};
	OBJLIST<Param> params;

	AsyncHttpRequest();

	void CalcSig();
};

AsyncHttpRequest *operator<<(AsyncHttpRequest *, const CHAR_PARAM &);
AsyncHttpRequest *operator<<(AsyncHttpRequest *, const INT_PARAM &);

class JsonReply
{
	JSONNode *m_root = nullptr;
	int m_errorCode = 0;
	JSONNode *m_data = nullptr;

public:
	JsonReply(NETLIBHTTPREQUEST *);
	~JsonReply();

	__forceinline JSONNode &data() const { return *m_data; }
	__forceinline int error() const { return m_errorCode; }
};

class FacebookProto : public PROTO<FacebookProto>
{
	AsyncHttpRequest* CreateRequest(const char *szName, const char *szMethod);
	NETLIBHTTPREQUEST* ExecuteRequest(AsyncHttpRequest *pReq);

	// MQTT functions
	void MqttOpen();

	// internal data
	CMStringA m_szDeviceID; // stored, GUID that identifies this miranda's account
	CMStringA m_szClientID; // stored, random alphanumeric string of 20 chars
	__int64   m_uid;        // stored, Facebook user id
	__int64   m_iMqttId;
	
	bool      m_invisible;
	bool      m_bOnline;

	CMStringA m_szAuthToken; // calculated 

	void OnLoggedOut();

	void __cdecl ServerThread(void *);

public:
	FacebookProto(const char *proto_name, const wchar_t *username);
	~FacebookProto();

	////////////////////////////////////////////////////////////////////////////////////////
	// PROTO_INTERFACE

	void OnModulesLoaded() override;

	INT_PTR  GetCaps(int type, MCONTACT hContact) override;

	int      SetStatus(int iNewStatus) override;
};

struct CMPlugin : public ACCPROTOPLUGIN<FacebookProto>
{
	CMPlugin();

	int Load() override;
};
