/*
Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)

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

void CVkProto::OnReceiveAvatar(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	if (reply->resultCode != 200 || !pReq->pUserInfo)
		return;

	PROTO_AVATAR_INFORMATION ai = { 0 };
	CVkSendMsgParam *param = (CVkSendMsgParam *)pReq->pUserInfo;
	GetAvatarFileName(param->hContact, ai.filename, _countof(ai.filename));
	ai.format = ProtoGetBufferFormat(reply->pData);

	FILE *out = _wfopen(ai.filename, L"wb");
	if (out == nullptr) {
		ProtoBroadcastAck(param->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &ai);
		delete param;
		pReq->pUserInfo = nullptr;
		return;
	}

	fwrite(reply->pData, 1, reply->dataLength, out);
	fclose(out);
	setByte(param->hContact, "NeedNewAvatar", 0);
	ProtoBroadcastAck(param->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai);
	delete param;
	pReq->pUserInfo = nullptr;
}

INT_PTR CVkProto::SvcGetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		((POINT*)lParam)->x = 100;
		((POINT*)lParam)->y = 100;
		return 0;

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

void CVkProto::ReloadAvatarInfo(MCONTACT hContact)
{
	if (!hContact) {
		ReportSelfAvatarChanged();
		return;
	}

	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = hContact;
	SvcGetAvatarInfo(0, (LPARAM)&ai);
}

INT_PTR CVkProto::SvcGetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)lParam;

	ptrA szUrl(getStringA(pai->hContact, "AvatarUrl"));
	if (szUrl == nullptr)
		return GAIR_NOAVATAR;

	wchar_t wszFileName[MAX_PATH];
	GetAvatarFileName(pai->hContact, wszFileName, _countof(wszFileName));
	wcsncpy(pai->filename, wszFileName, _countof(pai->filename));

	pai->format = ProtoGetAvatarFormat(pai->filename);

	if (::_waccess(pai->filename, 0) == 0 && !getBool(pai->hContact, "NeedNewAvatar"))
		return GAIR_SUCCESS;

	if (IsOnline()) {
		AsyncHttpRequest *pReq = new AsyncHttpRequest();
		pReq->flags = NLHRF_NODUMP | NLHRF_REDIRECT;
		pReq->m_szUrl = szUrl;
		pReq->pUserInfo = new CVkSendMsgParam(pai->hContact);
		pReq->m_pFunc = &CVkProto::OnReceiveAvatar;
		pReq->requestType = REQUEST_GET;
		pReq->m_bApiReq = false;
		Push(pReq);

		debugLogA("Requested to read an avatar from '%s'", szUrl.get());
		return GAIR_WAITFOR;
	}

	debugLogA("No avatar");
	return GAIR_NOAVATAR;
}

INT_PTR CVkProto::SvcGetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	debugLogA("CVkProto::SvcGetMyAvatar");
	PROTO_AVATAR_INFORMATION ai = { 0 };
	if (SvcGetAvatarInfo(0, (LPARAM)&ai) != GAIR_SUCCESS)
		return 1;

	wchar_t *buf = (wchar_t*)wParam;
	int size = (int)lParam;

	wcsncpy(buf, ai.filename, size);
	buf[size - 1] = 0;

	return 0;
}

void CVkProto::GetAvatarFileName(MCONTACT hContact, wchar_t *pwszDest, size_t cbLen)
{
	int tPathLen = mir_snwprintf(pwszDest, cbLen, L"%s\\%S", VARSW(L"%miranda_avatarcache%").get(), m_szModuleName);
	CreateDirectoryTreeW(pwszDest);
	pwszDest[tPathLen++] = '\\';

	const wchar_t *szFileType = L".jpg";
	ptrW wszUrl(getWStringA(hContact, "AvatarUrl"));
	if (wszUrl) {
		wchar_t *p = wcschr(wszUrl, '?');
		if (p != nullptr)
			*p = 0;

		p = wcsrchr(wszUrl, '.');
		if (p != nullptr)
			szFileType = p;

	}

	LONG id = getDword(hContact, "ID", VK_INVALID_USER);
	mir_snwprintf(pwszDest + tPathLen, MAX_PATH - tPathLen, L"%d%s", id, szFileType);
}

void CVkProto::SetAvatarUrl(MCONTACT hContact, CMStringW &wszUrl)
{
	CMStringW oldUrl(ptrW(getWStringA(hContact, "AvatarUrl")));
	if (wszUrl == oldUrl)
		return;

	if (wszUrl.IsEmpty()) {
		delSetting(hContact, "AvatarUrl");
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, nullptr);
	}
	else {
		setWString(hContact, "AvatarUrl", wszUrl);
		setByte(hContact, "NeedNewAvatar", 1);
		PROTO_AVATAR_INFORMATION ai = { 0 };
		ai.hContact = hContact;
		GetAvatarFileName(ai.hContact, ai.filename, _countof(ai.filename));
		ai.format = ProtoGetAvatarFormat(ai.filename);
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai);
	}
}