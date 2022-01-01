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

#include "stdafx.h"

INT_PTR CSkypeProto::SvcGetAvatarCaps(WPARAM wParam, LPARAM lParam)
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

void CSkypeProto::ReloadAvatarInfo(MCONTACT hContact)
{
	if (hContact == NULL) {
		ReportSelfAvatarChanged();
		return;
	}
	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = hContact;
	SvcGetAvatarInfo(0, (LPARAM)&ai);
}

void CSkypeProto::OnReceiveAvatar(NETLIBHTTPREQUEST *response, AsyncHttpRequest *pRequest)
{
	if (response == nullptr || response->pData == nullptr)
		return;
	
	MCONTACT hContact = (DWORD_PTR)pRequest->pUserInfo;
	if (response->resultCode != 200)
		return;

	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.format = ProtoGetBufferFormat(response->pData);
	setByte(hContact, "AvatarType", ai.format);
	GetAvatarFileName(hContact, ai.filename, _countof(ai.filename));

	FILE *out = _wfopen(ai.filename, L"wb");
	if (out == nullptr) {
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &ai, 0);
		return;
	}

	fwrite(response->pData, 1, response->dataLength, out);
	fclose(out);
	setByte(hContact, "NeedNewAvatar", 0);
	ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai, 0);
}

void CSkypeProto::OnSentAvatar(NETLIBHTTPREQUEST *response, AsyncHttpRequest*)
{
	JsonReply root(response);
	if (root.error())
		return;
}

INT_PTR CSkypeProto::SvcGetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION*)lParam;

	ptrA szUrl(getStringA(pai->hContact, "AvatarUrl"));
	if (szUrl == NULL)
		return GAIR_NOAVATAR;

	pai->format = getByte(pai->hContact, "AvatarType", PA_FORMAT_JPEG);

	wchar_t tszFileName[MAX_PATH];
	GetAvatarFileName(pai->hContact, tszFileName, _countof(tszFileName));
	wcsncpy(pai->filename, tszFileName, _countof(pai->filename));

	if (::_waccess(pai->filename, 0) == 0 && !getBool(pai->hContact, "NeedNewAvatar", 0))
		return GAIR_SUCCESS;

	if (IsOnline()) {
		PushRequest(new GetAvatarRequest(szUrl, pai->hContact));
		debugLogA("Requested to read an avatar from '%s'", szUrl.get());
		return GAIR_WAITFOR;
	}

	debugLogA("No avatar");
	return GAIR_NOAVATAR;
}

INT_PTR CSkypeProto::SvcGetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	wchar_t path[MAX_PATH];
	GetAvatarFileName(NULL, path, _countof(path));
	wcsncpy((wchar_t*)wParam, path, (int)lParam);
	return 0;
}

void CSkypeProto::GetAvatarFileName(MCONTACT hContact, wchar_t* pszDest, size_t cbLen)
{
	int tPathLen = mir_snwprintf(pszDest, cbLen, L"%s\\%s", VARSW(L"%miranda_avatarcache%").get(), m_tszUserName);
	CreateDirectoryTreeW(pszDest);
	pszDest[tPathLen++] = '\\';

	const wchar_t* szFileType = ProtoGetAvatarExtension(getByte(hContact, "AvatarType", PA_FORMAT_JPEG));
	CMStringA username(getId(hContact));
	username.Replace("live:", "__live_");
	username.Replace("facebook:", "__facebook_");
	mir_snwprintf(pszDest + tPathLen, MAX_PATH - tPathLen, L"%S%s", username.c_str(), szFileType);
}

void CSkypeProto::SetAvatarUrl(MCONTACT hContact, CMStringW &tszUrl)
{
	ptrW oldUrl(getWStringA(hContact, "AvatarUrl"));
	if (oldUrl != NULL)
		if (tszUrl == oldUrl)
			return;

	if (tszUrl.IsEmpty()) {
		delSetting(hContact, "AvatarUrl");
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, NULL, 0);
	}
	else {
		setWString(hContact, "AvatarUrl", tszUrl.GetBuffer());
		setByte(hContact, "NeedNewAvatar", 1);
		PROTO_AVATAR_INFORMATION ai = { 0 };
		ai.hContact = hContact;
		GetAvatarFileName(ai.hContact, ai.filename, _countof(ai.filename));
		ai.format = ProtoGetAvatarFormat(ai.filename);
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai, 0);
	}
}

INT_PTR CSkypeProto::SvcSetMyAvatar(WPARAM, LPARAM lParam)
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

						PushRequest(new SetAvatarRequest(data, length, szMime, this));
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
