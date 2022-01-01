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

#ifndef _SKYPE_POLL_H_
#define _SKYPE_POLL_H_

struct PollRequest : public AsyncHttpRequest
{
	PollRequest(CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_POST, HOST_DEFAULT, "/users/ME/endpoints/" + mir_urlEncode(ppro->m_szId) + "/subscriptions/0/poll")
	{
		timeout = 120000;

		if (ppro->m_iPollingId != -1)
			m_szUrl.AppendFormat("?ackId=%d", ppro->m_iPollingId);

		AddHeader("Referer", "https://web.skype.com/main");
		AddHeader("ClientInfo", "os=Windows; osVer=8.1; proc=Win32; lcid=en-us; deviceType=1; country=n/a; clientName=swx-skype.com; clientVer=908/1.85.0.29");
		AddHeader("Accept", "application/json; ver=1.0");
		AddHeader("Accept-Language", "en, C");
	}
};
#endif //_SKYPE_POLL_H_