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

#ifndef _SKYPE_REQUEST_SEARCH_H_
#define _SKYPE_REQUEST_SEARCH_H_

class GetSearchRequest : public HttpRequest
{
public:
	GetSearchRequest(const char *token, const char *string) :
		HttpRequest(REQUEST_GET, "api.skype.com/search/users/any")
	{
		Url
			<< CHAR_VALUE("keyWord", string)
			<< CHAR_VALUE("contactTypes[]", "skype");
		Headers
			<< CHAR_VALUE("Accept", "application/json")
			<< CHAR_VALUE("Connection", "keep-alive")
			<< CHAR_VALUE("X-Skypetoken", token);
	}
};

#endif //_SKYPE_REQUEST_SEARCH_H_
