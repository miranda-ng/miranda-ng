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

#ifndef _SKYPE_REQUEST_HISTORY_H_
#define _SKYPE_REQUEST_HISTORY_H_

struct SyncHistoryFirstRequest : public AsyncHttpRequest
{
	SyncHistoryFirstRequest(int pageSize) :
		AsyncHttpRequest(REQUEST_GET, HOST_DEFAULT, "/users/ME/conversations", &CSkypeProto::OnSyncHistory)
	{
		this << INT_PARAM("startTime", 0) << INT_PARAM("pageSize", pageSize)
			<< CHAR_PARAM("view", "msnp24Equivalent") << CHAR_PARAM("targetType", "Passport|Skype|Lync");
	}

	SyncHistoryFirstRequest(const char *url) :
		AsyncHttpRequest(REQUEST_GET, HOST_DEFAULT, url, &CSkypeProto::OnSyncHistory)
	{
	}
};

struct GetHistoryRequest : public AsyncHttpRequest
{
	GetHistoryRequest(const char *username, int pageSize, uint32_t timestamp, bool bOperative) :
		AsyncHttpRequest(REQUEST_GET, HOST_DEFAULT, 0, &CSkypeProto::OnGetServerHistory)
	{
		m_szUrl.AppendFormat("/users/ME/conversations/%s/messages", mir_urlEncode(username).c_str());

		if (bOperative)
			pUserInfo = this;

		this << INT_PARAM("startTime", timestamp) << INT_PARAM("pageSize", pageSize)
			<< CHAR_PARAM("view", "msnp24Equivalent") << CHAR_PARAM("targetType", "Passport|Skype|Lync|Thread");
	}

	GetHistoryRequest(const char *url, void *pInfo) :
		AsyncHttpRequest(REQUEST_GET, HOST_DEFAULT, url, &CSkypeProto::OnGetServerHistory)
	{
		pUserInfo = pInfo;
	}
};

#endif //_SKYPE_REQUEST_HISTORY_H_
