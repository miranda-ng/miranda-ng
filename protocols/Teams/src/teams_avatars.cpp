/*
Copyright (c) 2015-25 Miranda NG team (https://miranda-ng.org)

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

#include "stdafx.h"

void CTeamsProto::GetAvatarFileName(MCONTACT hContact, wchar_t *pszDest, size_t cbLen)
{
	CMStringW wszPath(GetAvatarPath());
	wszPath += '\\';

	const wchar_t *szFileType = ProtoGetAvatarExtension(getByte(hContact, "AvatarType", PA_FORMAT_JPEG));
	CMStringA username(getId(hContact));
	username.Replace("live:", "__live_");
	username.Replace("facebook:", "__facebook_");
	wszPath.AppendFormat(L"%S%s", username.c_str(), szFileType);

	wcsncpy_s(pszDest, cbLen, wszPath, _TRUNCATE);
}

void CTeamsProto::ReloadAvatarInfo(MCONTACT hContact)
{
	if (hContact == NULL) {
		ReportSelfAvatarChanged();
		return;
	}

	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = hContact;
	SvcGetAvatarInfo(0, (LPARAM)&ai);
}

void CTeamsProto::SetAvatarUrl(MCONTACT hContact, const CMStringW &tszUrl)
{
	ptrW oldUrl(getWStringA(hContact, "AvatarUrl"));
	if (oldUrl != NULL)
		if (tszUrl == oldUrl)
			return;

	if (tszUrl.IsEmpty()) {
		delSetting(hContact, "AvatarUrl");
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, nullptr);
	}
	else {
		setWString(hContact, "AvatarUrl", tszUrl);
		setByte(hContact, "NeedNewAvatar", 1);

		PROTO_AVATAR_INFORMATION ai = {};
		ai.hContact = hContact;
		GetAvatarFileName(ai.hContact, ai.filename, _countof(ai.filename));
		ai.format = ProtoGetAvatarFormat(ai.filename);
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Avatar services for Miranda

INT_PTR CTeamsProto::SvcGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		((POINT*)lParam)->x = 98;
		((POINT*)lParam)->y = 98;
		break;

	case AF_MAXFILESIZE:
		return 32000;

	case AF_PROPORTION:
		return PIP_SQUARE;

	case AF_FORMATSUPPORTED:
	case AF_ENABLED:
	case AF_DONTNEEDDELAYS:
	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		return 1;
	}
	return 0;
}

INT_PTR CTeamsProto::SvcGetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)lParam;

	pai->format = getByte(pai->hContact, "AvatarType", PA_FORMAT_JPEG);

	wchar_t tszFileName[MAX_PATH];
	GetAvatarFileName(pai->hContact, tszFileName, _countof(tszFileName));
	wcsncpy(pai->filename, tszFileName, _countof(pai->filename));

	if (::_waccess(pai->filename, 0) == 0 && !getBool(pai->hContact, "NeedNewAvatar", 0))
		return GAIR_SUCCESS;

	if (IsOnline())
		if (ReceiveAvatar(pai->hContact))
			return GAIR_WAITFOR;

	debugLogA("No avatar");
	return GAIR_NOAVATAR;
}

INT_PTR CTeamsProto::SvcGetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	wchar_t path[MAX_PATH];
	GetAvatarFileName(NULL, path, _countof(path));
	wcsncpy((wchar_t *)wParam, path, (int)lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Avatars' receiving

struct GetAvatarRequest : public AsyncHttpRequest
{
	GetAvatarRequest(const char *url, MCONTACT hContact) :
		AsyncHttpRequest(REQUEST_GET, HOST_OTHER, url, &CTeamsProto::OnReceiveAvatar)
	{
		flags |= NLHRF_REDIRECT;
		pUserInfo = (void *)hContact;
	}
};

void CTeamsProto::OnReceiveAvatar(MHttpResponse *response, AsyncHttpRequest *pRequest)
{
	if (response == nullptr || response->body.IsEmpty())
		return;
	
	MCONTACT hContact = (DWORD_PTR)pRequest->pUserInfo;
	if (response->resultCode != 200)
		return;

	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.format = ProtoGetBufferFormat(response->body);
	setByte(hContact, "AvatarType", ai.format);
	GetAvatarFileName(hContact, ai.filename, _countof(ai.filename));

	FILE *out = _wfopen(ai.filename, L"wb");
	if (out == nullptr) {
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &ai, 0);
		return;
	}

	fwrite(response->body, 1, response->body.GetLength(), out);
	fclose(out);
	setByte(hContact, "NeedNewAvatar", 0);
	ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai, 0);
}

bool CTeamsProto::ReceiveAvatar(MCONTACT hContact)
{
	ptrA szUrl(getStringA(hContact, "AvatarUrl"));
	if (!mir_strlen(szUrl))
		return false;

	PushRequest(new GetAvatarRequest(szUrl, hContact));
	debugLogA("Requested to read an avatar from '%s'", szUrl.get());
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Setting my own avatar

struct SetAvatarRequest : public AsyncHttpRequest
{
	SetAvatarRequest(const uint8_t *data, int dataSize, const char *szMime, CTeamsProto *ppro) :
		AsyncHttpRequest(REQUEST_PUT, HOST_API, 0, &CTeamsProto::OnSentAvatar)
	{
		m_szUrl.AppendFormat("/users/%s/profile/avatar", ppro->m_szSkypename.MakeLower().c_str());

		AddHeader("Content-Type", szMime);

		m_szParam.Truncate(dataSize);
		memcpy(m_szParam.GetBuffer(), data, dataSize);
	}
};

void CTeamsProto::OnSentAvatar(MHttpResponse *response, AsyncHttpRequest*)
{
	SkypeReply root(response);
	if (root.error())
		return;
}

INT_PTR CTeamsProto::SvcSetMyAvatar(WPARAM, LPARAM lParam)
{
	wchar_t *path = (wchar_t*)lParam;
	wchar_t avatarPath[MAX_PATH];
	GetAvatarFileName(NULL, avatarPath, _countof(avatarPath));
	if (path != nullptr) {
		if (CopyFile(path, avatarPath, FALSE)) {
			FILE *hFile = _wfopen(path, L"rb");
			if (hFile) {
				fseek(hFile, 0, SEEK_END);
				size_t length = ftell(hFile);
				if (length != -1) {
					rewind(hFile);

					mir_ptr<uint8_t> data((uint8_t*)mir_alloc(length));

					if (data != NULL && fread(data, sizeof(uint8_t), length, hFile) == length) {
						const char *szMime = FreeImage_GetFIFMimeType(FreeImage_GetFIFFromFilenameU(path));

						PushRequest(new SetAvatarRequest(data, (int)length, szMime, this));
						fclose(hFile);
						return 0;
					}
				}
				fclose(hFile);
			}
		}
		return -1;
	}
	else if (IsFileExists(avatarPath))
		DeleteFile(avatarPath);

	return 0;
}
