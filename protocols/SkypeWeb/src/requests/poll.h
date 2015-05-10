/*
Copyright (c) 2015 Miranda NG project (http://miranda-ng.org)

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
	PollRequest(const char *regToken, const char *server = SKYPE_ENDPOINTS_HOST) :
		HttpRequest(REQUEST_POST, FORMAT, "%s/v1/users/ME/endpoints/SELF/subscriptions/0/poll", server)
	{
		timeout = INFINITE;
		flags |= NLHRF_PERSISTENT;
		Headers
			<< CHAR_VALUE("Connection", "keep-alive")
			<< CHAR_VALUE("Accept", "application/json, text/javascript")
			<< FORMAT_VALUE("RegistrationToken", "registrationToken=%s", regToken);
	}
};
#endif //_SKYPE_POLL_H_