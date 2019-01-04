// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-19 Miranda NG team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

void CIcqProto::InitContactCache()
{
	mir_cslock l(m_csCache);
	for (auto &it : AccContacts())
		m_arCache.insert(new IcqCacheItem(getDword(it, DB_KEY_UIN), it));
}

IcqCacheItem* CIcqProto::FindContactByUIN(DWORD dwUin)
{
	mir_cslock l(m_csCache);
	return m_arCache.find((IcqCacheItem*)&dwUin);
}

MCONTACT CIcqProto::CreateContact(DWORD dwUin, bool bTemporary)
{
	auto *pCache = FindContactByUIN(dwUin);
	if (pCache != nullptr)
		return pCache->m_hContact;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);
	setDword(hContact, DB_KEY_UIN, dwUin);
	pCache = new IcqCacheItem(dwUin, hContact);
	{
		mir_cslock l(m_csCache);
		m_arCache.insert(pCache);
	}
	RetrieveUserInfo(hContact);

	if (bTemporary)
		db_set_b(hContact, "CList", "NotOnList", 1);

	return hContact;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CIcqProto::CalcHash(AsyncHttpRequest *pReq)
{
	CMStringA hashData(FORMAT, "POST&%s&%s", ptrA(mir_urlEncode(pReq->m_szUrl)), ptrA(mir_urlEncode(pReq->m_szParam)));
	unsigned int len;
	BYTE hashOut[MIR_SHA256_HASH_SIZE];
	HMAC(EVP_sha256(), m_szSessionKey, m_szSessionKey.GetLength(), (BYTE*)hashData.c_str(), hashData.GetLength(), hashOut, &len);
	pReq << CHAR_PARAM("sig_sha256", ptrA(mir_base64_encode(hashOut, sizeof(hashOut))));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Avatars

void CIcqProto::GetAvatarFileName(MCONTACT hContact, wchar_t* pszDest, size_t cbLen)
{
	int tPathLen = mir_snwprintf(pszDest, cbLen, L"%s\\%S", VARSW(L"%miranda_avatarcache%"), m_szModuleName);

	DWORD dwAttributes = GetFileAttributes(pszDest);
	if (dwAttributes == 0xffffffff || (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		CreateDirectoryTreeW(pszDest);

	pszDest[tPathLen++] = '\\';

	CMStringW wszFileName(getMStringW(hContact, "IconId"));
	const wchar_t* szFileType = ProtoGetAvatarExtension(getByte(hContact, "AvatarType", PA_FORMAT_PNG));
	mir_snwprintf(pszDest + tPathLen, MAX_PATH - tPathLen, L"%s%s", wszFileName.c_str(), szFileType);
}

INT_PTR __cdecl CIcqProto::GetAvatar(WPARAM wParam, LPARAM lParam)
{
	wchar_t *buf = (wchar_t*)wParam;
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
		((POINT*)lParam)->x = -1;
		((POINT*)lParam)->y = -1;
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
	PROTO_AVATAR_INFORMATION* pai = (PROTO_AVATAR_INFORMATION*)lParam;

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
	wchar_t* pwszFileName = (wchar_t*)lParam;

	wchar_t wszOldName[MAX_PATH];
	GetAvatarFileName(0, wszOldName, _countof(wszOldName));
	_wremove(wszOldName);

	auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_POST, ICQ_API_SERVER "/expressions/upload");
	pReq->m_szUrl.AppendFormat("?f=json&aimsid=%s&r=%s&type=largeBuddyIcon", ptrA(mir_urlEncode(m_aimsid.c_str())), pReq->m_reqId);

	if (pwszFileName == nullptr)
		delSetting("AvatarHash");
	else {
		int fileId = _wopen(pwszFileName, _O_RDONLY | _O_BINARY, _S_IREAD);
		if (fileId < 0) {
			delete pReq;
			return 1;
		}

		unsigned dwSize = (unsigned)_filelengthi64(fileId);
		char* pData = (char*)mir_alloc(dwSize);
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
		
		pReq->AddHeader("Content-Type", _T2A(ProtoGetAvatarMimeType(iAvatarType)));
	}
	Push(pReq);

	return 0;   // TODO
}

/////////////////////////////////////////////////////////////////////////////////////////

int StatusFromString(const CMStringW &wszStatus)
{
	if (wszStatus == "online")
		return ID_STATUS_ONLINE;
	if (wszStatus == "n/a")
		return ID_STATUS_NA;
	if (wszStatus == "away")
		return ID_STATUS_AWAY;
	if (wszStatus == "occupied")
		return ID_STATUS_OCCUPIED;
	if (wszStatus == "dnd")
		return ID_STATUS_DND;

	return ID_STATUS_OFFLINE;
}

/////////////////////////////////////////////////////////////////////////////////////////

__int64 CIcqProto::getId(MCONTACT hContact, const char *szSetting)
{
	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;
	if (db_get(hContact, m_szModuleName, szSetting, &dbv))
		return 0;

	__int64 result = (dbv.cpbVal == sizeof(__int64)) ? *(__int64*)dbv.pbVal : 0;
	db_free(&dbv);
	return result;
}

void CIcqProto::setId(MCONTACT hContact, const char *szSetting, __int64 iValue)
{
	__int64 oldVal = getId(hContact, szSetting);
	if (oldVal != iValue)
		db_set_blob(hContact, m_szModuleName, szSetting, &iValue, sizeof(iValue));
}
