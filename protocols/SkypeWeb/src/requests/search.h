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

#ifndef _SKYPE_REQUEST_SEARCH_H_
#define _SKYPE_REQUEST_SEARCH_H_

struct GetSearchRequest : public AsyncHttpRequest
{
	GetSearchRequest(const char *string) :
		AsyncHttpRequest(REQUEST_GET, HOST_GRAPH, "/search/v1.1/namesearch/swx/", &CSkypeProto::OnSearch)
	{
		this << CHAR_PARAM("requestid", "skype.com-1.48.78-00000000-0000-0000-0000-000000000000")
			<< CHAR_PARAM("locale", "en-US") << CHAR_PARAM("searchstring", string);

		AddHeader("Accept", "application/json");
	}
};

#endif //_SKYPE_REQUEST_SEARCH_H_
