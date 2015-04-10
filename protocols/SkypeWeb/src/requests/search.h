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
	  HttpRequest(REQUEST_GET, FORMAT, "api.skype.com/search/users/any?keyWord=%s&contactTypes[]=skype", mir_urlEncode(string))
	{
		Headers
			<< CHAR_VALUE("Accept", "application/json")
			<< CHAR_VALUE("Content-Type", "application/json; charset=UTF-8")
			<< CHAR_VALUE("Connection", "keep-alive")
			<< CHAR_VALUE("X-Skypetoken", token);
	}
};

#endif //_SKYPE_REQUEST_SEARCH_H_
