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

#ifndef _SKYPE_REQUEST_AVATAR_H_
#define _SKYPE_REQUEST_AVATAR_H_

struct GetAvatarRequest : public AsyncHttpRequest
{
	GetAvatarRequest(const char *url, MCONTACT hContact) :
		AsyncHttpRequest(REQUEST_GET, HOST_OTHER, url, &CSkypeProto::OnReceiveAvatar)
	{
		flags |= NLHRF_REDIRECT;
		pUserInfo = (void *)hContact;
	}
};

struct SetAvatarRequest : public AsyncHttpRequest
{
	SetAvatarRequest(const uint8_t *data, size_t dataSize, const char *szMime, CSkypeProto *ppro) :
		AsyncHttpRequest(REQUEST_PUT, HOST_API, 0, &CSkypeProto::OnSentAvatar)
	{
		m_szUrl.AppendFormat("/users/%s/profile/avatar", ppro->m_szSkypename.MakeLower().c_str());

		AddHeader("Content-Type", szMime);

		pData = (char *)mir_alloc(dataSize);
		memcpy(pData, data, dataSize);
		dataLength = (int)dataSize;
	}

	~SetAvatarRequest()
	{
		mir_free(pData);
	}
};

#endif //_SKYPE_REQUEST_AVATAR_H_
