/*
Copyright (c) 2015-17 Miranda NG project (https://miranda-ng.org)

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
	GetSearchRequest(const char *string, LoginInfo &li) :
		HttpRequest(REQUEST_GET, "skypegraph.skype.com/search/v1.1/namesearch/swx/")
	{

		wchar_t locale[LOCALE_NAME_MAX_LENGTH] = L"en-US";
		//LCIDToLocaleName(Langpack_GetDefaultLocale(), locale, _countof(locale), 0); //FIXME: xp support

		Url
			<< CHAR_VALUE("requestid", "skype.com-1.48.78-00000000-0000-0000-0000-000000000000")
			<< CHAR_VALUE("locale", T2Utf(locale))
			<< CHAR_VALUE("searchstring", string);
		Headers
			<< CHAR_VALUE("Accept", "application/json")
			<< CHAR_VALUE("X-Skypetoken", li.api.szToken);
	}
};

#endif //_SKYPE_REQUEST_SEARCH_H_
