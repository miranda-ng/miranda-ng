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
#ifndef _SKYPE_REQUEST_HISTORY_H_
#define _SKYPE_REQUEST_HISTORY_H_

struct SyncHistoryFirstRequest : public AsyncHttpRequest
{
	SyncHistoryFirstRequest(int pageSize, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_GET, "/users/ME/conversations", &CSkypeProto::OnSyncHistory)
	{
		this << INT_PARAM("startTime", 0) << INT_PARAM("pageSize", pageSize)
			<< CHAR_PARAM("view", "msnp24Equivalent") << CHAR_PARAM("targetType", "Passport|Skype|Lync");

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset = UTF-8");
		AddRegistrationToken(ppro);
	}

	SyncHistoryFirstRequest(const char *url, CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_GET, url, &CSkypeProto::OnSyncHistory)
	{
		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset = UTF-8");
		AddRegistrationToken(ppro);
	}
};

struct GetHistoryRequest : public AsyncHttpRequest
{
	GetHistoryRequest(const char *username, int pageSize, bool isChat, LONGLONG timestamp, CSkypeProto *ppro) :
	  AsyncHttpRequest(REQUEST_GET, 0, &CSkypeProto::OnGetServerHistory)
	{
		m_szUrl.Format("/users/ME/conversations/%d:%s/messages", isChat ? 19 : 8, mir_urlEncode(username).c_str());

		this << INT_PARAM("startTime", timestamp) << INT_PARAM("pageSize", pageSize)
			<< CHAR_PARAM("view", "msnp24Equivalent") << CHAR_PARAM("targetType", "Passport|Skype|Lync|Thread");

		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset = UTF-8");
		AddRegistrationToken(ppro);
	}
};

struct GetHistoryOnUrlRequest : public AsyncHttpRequest
{
	GetHistoryOnUrlRequest(const char *url, CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_GET, url, &CSkypeProto::OnGetServerHistory)
	{
		AddHeader("Accept", "application/json, text/javascript");
		AddHeader("Content-Type", "application/json; charset = UTF-8");
		AddRegistrationToken(ppro);
	}
};

#endif //_SKYPE_REQUEST_HISTORY_H_