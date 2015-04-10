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

#ifndef _SKYPE_REQUEST_LOGOUT_H_
#define _SKYPE_REQUEST_LOGOUT_H_

class LogoutRequest : public HttpRequest
{
public:
	LogoutRequest() : HttpRequest(REQUEST_GET, "login.skype.com/logout")
	{
		flags |= NLHRF_REDIRECT;
		Url
			<< INT_VALUE("client_id", 578134)
			<< CHAR_VALUE("redirect_uri", "https%3A%2F%2Fweb.skype.com&intsrc=client-_-webapp-_-production-_-go-signin");
		Headers
			<< CHAR_VALUE("Referer", "https://web.skype.com/");
	}
};

#endif //_SKYPE_REQUEST_LOGOUT_H_
