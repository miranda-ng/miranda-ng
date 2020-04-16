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

#ifndef _SKYPE_POLL_H_
#define _SKYPE_POLL_H_

class PollRequest : public HttpRequest
{
public:
	PollRequest(CSkypeProto *ppro) :
	  HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints/SELF/subscriptions/0/poll", ppro->m_szServer)
	{
		timeout = 120000;

		if (ppro->m_iPollingId != -1)
			Url << INT_VALUE("ackId", ppro->m_iPollingId);

		Headers
			<< CHAR_VALUE("Referer", "https://web.skype.com/main")
			<< CHAR_VALUE("Content-Type", "application/x-www-form-urlencoded")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", ppro->m_szToken.get())
			<< CHAR_VALUE("ClientInfo", "os=Windows; osVer=8.1; proc=Win32; lcid=en-us; deviceType=1; country=n/a; clientName=swx-skype.com; clientVer=908/1.85.0.29")
			<< CHAR_VALUE("Accept", "application/json; ver=1.0")
			<< CHAR_VALUE("Accept-Language", "en, C");
	}
};
#endif //_SKYPE_POLL_H_