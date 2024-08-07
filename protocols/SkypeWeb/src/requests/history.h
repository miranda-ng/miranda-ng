/*
Copyright (c) 2015-24 Miranda NG team (https://miranda-ng.org)

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
	SyncHistoryFirstRequest(int pageSize) :
		AsyncHttpRequest(REQUEST_GET, HOST_DEFAULT, "/users/ME/conversations", &CSkypeProto::OnSyncConversations)
	{
		this << INT_PARAM("startTime", 0) << INT_PARAM("pageSize", pageSize)
			<< CHAR_PARAM("view", "msnp24Equivalent") << CHAR_PARAM("targetType", "Passport|Skype|Lync");
	}
};

struct GetHistoryRequest : public AsyncHttpRequest
{
	CMStringA m_who;

	GetHistoryRequest(MCONTACT _1, const char *who, int pageSize, int64_t timestamp, bool bOperative) :
		AsyncHttpRequest(REQUEST_GET, HOST_DEFAULT, "/users/ME/conversations/" + mir_urlEncode(who) + "/messages", &CSkypeProto::OnGetServerHistory),
		m_who(who)
	{
		hContact = _1;
		if (bOperative)
			pUserInfo = this;

		this << INT64_PARAM("startTime", timestamp) << INT_PARAM("pageSize", pageSize)
			<< CHAR_PARAM("view", "msnp24Equivalent") << CHAR_PARAM("targetType", "Passport|Skype|Lync|Thread");
	}
};

struct EmptyHistoryRequest : public AsyncHttpRequest
{
	EmptyHistoryRequest(const char *who) :
		AsyncHttpRequest(REQUEST_DELETE, HOST_DEFAULT, "/users/ME/conversations/" + mir_urlEncode(who) + "/messages")
	{
	}
};

#endif //_SKYPE_REQUEST_HISTORY_H_
