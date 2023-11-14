/*
Copyright (C) 2012-23 Miranda NG team (https://miranda-ng.org)

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

/////////////////////////////////////////////////////////////////////////////////////////
// Avatars

void CIcqProto::GetAvatarFileName(MCONTACT hContact, wchar_t *pszDest, size_t cbLen)
{
	CMStringW wszPath(GetAvatarPath());
	wszPath += '\\';

	CMStringW wszFileName(getMStringW(hContact, "IconId"));
	const wchar_t *szFileType = ProtoGetAvatarExtension(getByte(hContact, "AvatarType", PA_FORMAT_PNG));
	wszPath.AppendFormat(L"%s%s", wszFileName.c_str(), szFileType);

	wcsncpy_s(pszDest, cbLen, wszPath, _TRUNCATE);
}

INT_PTR __cdecl CIcqProto::GetAvatar(WPARAM wParam, LPARAM lParam)
{
	wchar_t *buf = (wchar_t *)wParam;
	int size = (int)lParam;
	if (buf == nullptr || size <= 0)
		return -1;

	GetAvatarFileName(0, buf, size);
	return 0;
}

INT_PTR __cdecl CIcqProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_MAXSIZE:
		((POINT *)lParam)->x = -1;
		((POINT *)lParam)->y = -1;
		return 0;

	case AF_FORMATSUPPORTED: // nobody
		return 1;

	case AF_DELAYAFTERFAIL:
		return 10 * 60 * 1000;

	case AF_ENABLED:
	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		return 1;
	}
	return 0;
}

INT_PTR __cdecl CIcqProto::GetAvatarInfo(WPARAM, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)lParam;

	ptrW szIconId(getWStringA(pai->hContact, "IconId"));
	if (szIconId == nullptr) {
		debugLogA("No avatar");
		return GAIR_NOAVATAR;
	}

	GetAvatarFileName(pai->hContact, pai->filename, _countof(pai->filename));
	pai->format = getByte(pai->hContact, "AvatarType", 0);

	if (::_waccess(pai->filename, 0) == 0)
		return GAIR_SUCCESS;

	debugLogA("No avatar");
	return GAIR_NOAVATAR;
}

INT_PTR __cdecl CIcqProto::SetAvatar(WPARAM, LPARAM lParam)
{
	wchar_t *pwszFileName = (wchar_t *)lParam;

	wchar_t wszOldName[MAX_PATH];
	GetAvatarFileName(0, wszOldName, _countof(wszOldName));
	_wremove(wszOldName);

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, "/expressions/upload");
	pReq->m_szUrl.AppendFormat("?f=json&aimsid=%s&r=%s&type=largeBuddyIcon", mir_urlEncode(m_aimsid.c_str()).c_str(), pReq->m_reqId);

	if (pwszFileName == nullptr)
		delSetting("AvatarHash");
	else {
		int fileId = _wopen(pwszFileName, _O_RDONLY | _O_BINARY, _S_IREAD);
		if (fileId < 0) {
			delete pReq;
			return 1;
		}

		unsigned dwSize = (unsigned)_filelengthi64(fileId);
		char *pData = (char *)mir_alloc(dwSize);
		if (pData == nullptr) {
			_close(fileId);
			delete pReq;
			return 2;
		}

		_read(fileId, pData, dwSize);
		_close(fileId);

		pReq->pData = pData;
		pReq->dataLength = dwSize;

		int iAvatarType = ProtoGetBufferFormat(pData);
		if (iAvatarType == PA_FORMAT_UNKNOWN) {
			delete pReq;
			delete pData;
			return 3;
		}

		pReq->AddHeader("Content-Type", ProtoGetAvatarMimeType(iAvatarType));
	}
	Push(pReq);

	return 0;   // TODO
}
