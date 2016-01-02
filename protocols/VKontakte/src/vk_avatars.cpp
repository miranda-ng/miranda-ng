/*
Copyright (c) 2013-16 Miranda NG project (http://miranda-ng.org)

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

void CVkProto::OnReceiveAvatar(NETLIBHTTPREQUEST *reply, AsyncHttpRequest* pReq)
{
	if (reply->resultCode != 200 || !pReq->pUserInfo)
		return;

	PROTO_AVATAR_INFORMATION ai = { 0 };
	CVkSendMsgParam * param = (CVkSendMsgParam *)pReq->pUserInfo;
	GetAvatarFileName(param->hContact, ai.filename, _countof(ai.filename));
	ai.format = ProtoGetBufferFormat(reply->pData);

	FILE *out = _tfopen(ai.filename, _T("wb"));
	if (out == NULL) {
		ProtoBroadcastAck(param->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &ai);
		delete param;
		pReq->pUserInfo = NULL;
		return;
	}

	fwrite(reply->pData, 1, reply->dataLength, out);
	fclose(out);
	setByte(param->hContact, "NeedNewAvatar", 0);
	ProtoBroadcastAck(param->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai);
	delete param;
	pReq->pUserInfo = NULL;
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
		CallService(MS_AV_REPORTMYAVATARCHANGED, (WPARAM)m_szModuleName);
		return;
	}

	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.hContact = hContact;
	SvcGetAvatarInfo(0, (LPARAM)&ai);
}

INT_PTR CVkProto::SvcGetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION* pai = (PROTO_AVATAR_INFORMATION*)lParam;

	ptrA szUrl(getStringA(pai->hContact, "AvatarUrl"));
	if (szUrl == NULL)
		return GAIR_NOAVATAR;

	TCHAR tszFileName[MAX_PATH];
	GetAvatarFileName(pai->hContact, tszFileName, _countof(tszFileName));
	_tcsncpy(pai->filename, tszFileName, _countof(pai->filename));

	pai->format = ProtoGetAvatarFormat(pai->filename);

	if (::_taccess(pai->filename, 0) == 0 && !getBool(pai->hContact, "NeedNewAvatar"))
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

		debugLogA("Requested to read an avatar from '%s'", szUrl);
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

	TCHAR* buf = (TCHAR*)wParam;
	int size = (int)lParam;

	_tcsncpy(buf, ai.filename, size);
	buf[size - 1] = 0;

	return 0;
}

void CVkProto::GetAvatarFileName(MCONTACT hContact, TCHAR* pszDest, size_t cbLen)
{
	int tPathLen = mir_sntprintf(pszDest, cbLen, _T("%s\\%S"), VARST(_T("%miranda_avatarcache%")), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(pszDest);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeT(pszDest);

	pszDest[tPathLen++] = '\\';

	const TCHAR* szFileType = _T(".jpg");
	ptrT szUrl(getTStringA(hContact, "AvatarUrl"));
	if (szUrl) {
		TCHAR *p = _tcsrchr(szUrl, '.');
		if (p != NULL)
			szFileType = p;
	}

	LONG id = getDword(hContact, "ID", -1);
	mir_sntprintf(pszDest + tPathLen, MAX_PATH - tPathLen, _T("%d%s"), id, szFileType);
}

void CVkProto::SetAvatarUrl(MCONTACT hContact, CMString &tszUrl)
{
	CMString oldUrl(getTStringA(hContact, "AvatarUrl"));

	if (tszUrl == oldUrl)
		return;

	if (tszUrl.IsEmpty()) {
		delSetting(hContact, "AvatarUrl");
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, NULL);
	}
	else {
		setTString(hContact, "AvatarUrl", tszUrl);
		setByte(hContact,"NeedNewAvatar", 1);
		PROTO_AVATAR_INFORMATION ai = { 0 };
		ai.hContact = hContact;
		GetAvatarFileName(ai.hContact, ai.filename, _countof(ai.filename));
		ai.format = ProtoGetAvatarFormat(ai.filename);
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai);
	}
}