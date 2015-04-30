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

#ifndef _SKYPE_REQUEST_AVATAR_H_
#define _SKYPE_REQUEST_AVATAR_H_

class GetAvatarRequest : public HttpRequest
{
public:
	GetAvatarRequest(const char *url) : HttpRequest(REQUEST_GET, url)
	{
		flags |= NLHRF_REDIRECT;
	}
};

class SetAvatarRequest : public HttpRequest
{
public:
	SetAvatarRequest(const char *token, const char *skypename, const char *data, size_t dataSize) :
		HttpRequest(REQUEST_PUT, FORMAT, "api.skype.com/users/%s/profile/avatar", skypename)
	{
		Headers
			<< CHAR_VALUE("X-Skypetoken", token)
			<< CHAR_VALUE("Content-Type", "image/jpeg");

		pData = (char*)mir_alloc(dataSize);
		memcpy(pData, data, dataSize);
		dataLength = (int)dataSize;
	}

	~SetAvatarRequest()
	{
		HttpRequest::~HttpRequest();
		mir_free(pData);
	}
};


#endif //_SKYPE_REQUEST_AVATAR_H_
