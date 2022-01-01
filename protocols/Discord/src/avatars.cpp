/*
Copyright © 2016-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CMStringW CDiscordProto::GetAvatarFilename(MCONTACT hContact)
{
	CMStringW wszResult(FORMAT, L"%s\\%S", VARSW(L"%miranda_avatarcache%"), m_szModuleName);
	CreateDirectoryTreeW(wszResult);

	wszResult.AppendChar('\\');

	const wchar_t* szFileType = ProtoGetAvatarExtension(getByte(hContact, "AvatarType", PA_FORMAT_PNG));
	wszResult.AppendFormat(L"%lld%s", getId(hContact, DB_KEY_ID), szFileType);
	return wszResult;
}

INT_PTR CDiscordProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	int res = 0;

	switch (wParam) {
	case AF_MAXSIZE:
		((POINT*)lParam)->x = ((POINT*)lParam)->y = 128;
		break;

	case AF_FORMATSUPPORTED:
		res = lParam == PA_FORMAT_PNG || lParam == PA_FORMAT_GIF || lParam == PA_FORMAT_JPEG;
		break;

	case AF_ENABLED:
	case AF_DONTNEEDDELAYS:
	case AF_FETCHIFPROTONOTVISIBLE:
	case AF_FETCHIFCONTACTOFFLINE:
		return 1;
	}

	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnReceiveAvatar(NETLIBHTTPREQUEST *reply, AsyncHttpRequest *pReq)
{
	PROTO_AVATAR_INFORMATION ai = { 0 };
	ai.format = PA_FORMAT_UNKNOWN;
	ai.hContact = (UINT_PTR)pReq->pUserInfo;

	if (reply->resultCode != 200) {
LBL_Error:
		ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, (HANDLE)&ai);
		return;
	}

	if (auto *pszHdr = Netlib_GetHeader(reply, "Content-Type"))
		ai.format = ProtoGetAvatarFormatByMimeType(pszHdr);

	if (ai.format == PA_FORMAT_UNKNOWN) {
		debugLogA("unknown avatar mime type");
		goto LBL_Error;
	}

	setByte(ai.hContact, "AvatarType", ai.format);
	mir_wstrncpy(ai.filename, GetAvatarFilename(ai.hContact), _countof(ai.filename));

	FILE *out = _wfopen(ai.filename, L"wb");
	if (out == nullptr) {
		debugLogA("cannot open avatar file %S for writing", ai.filename);
		goto LBL_Error;
	}

	fwrite(reply->pData, 1, reply->dataLength, out);
	fclose(out);

	if (ai.hContact)
		ProtoBroadcastAck(ai.hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, (HANDLE)&ai);
	else
		ReportSelfAvatarChanged();
}

bool CDiscordProto::RetrieveAvatar(MCONTACT hContact)
{
	ptrA szAvatarHash(getStringA(hContact, DB_KEY_AVHASH));
	SnowFlake id = getId(hContact, DB_KEY_ID);
	if (id == 0 || szAvatarHash == nullptr)
		return false;

	CMStringA szUrl(FORMAT, "https://cdn.discordapp.com/avatars/%lld/%s.jpg", id, szAvatarHash.get());
	AsyncHttpRequest *pReq = new AsyncHttpRequest(this, REQUEST_GET, szUrl, &CDiscordProto::OnReceiveAvatar);
	pReq->pUserInfo = (void*)hContact;
	Push(pReq);
	return true;
}

INT_PTR CDiscordProto::GetAvatarInfo(WPARAM flags, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)lParam;

	CMStringW wszFileName(GetAvatarFilename(pai->hContact));
	if (!wszFileName.IsEmpty()) {
		mir_wstrncpy(pai->filename, wszFileName, _countof(pai->filename));

		bool bFileExist = _waccess(wszFileName, 0) == 0;

		// if we still need to load an avatar
		if ((flags & GAIF_FORCE) || !bFileExist) {
			if (RetrieveAvatar(pai->hContact))
				return GAIR_WAITFOR;
		}
		else if (bFileExist)
			return GAIR_SUCCESS;
	}

	return GAIR_NOAVATAR;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (!wParam || !lParam)
		return -3;

	wchar_t* buf = (wchar_t*)wParam;
	int  size = (int)lParam;

	PROTO_AVATAR_INFORMATION ai = {};
	switch (GetAvatarInfo(0, (LPARAM)&ai)) {
	case GAIR_SUCCESS:
		wcsncpy_s(buf, size, ai.filename, _TRUNCATE);
		return 0;

	case GAIR_WAITFOR:
		return -1;
	}

	return -2;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CDiscordProto::SetMyAvatar(WPARAM, LPARAM lParam)
{
	CMStringW wszFileName(GetAvatarFilename(0));

	const wchar_t *pwszFilename = (const wchar_t*)lParam;
	if (pwszFilename == nullptr) { // remove my avatar file
		delSetting(DB_KEY_AVHASH);
		DeleteFile(wszFileName);
	}

	CMStringA szPayload("data:");

	const char *szMimeType = ProtoGetAvatarMimeType(ProtoGetAvatarFileFormat(pwszFilename));
	if (szMimeType == nullptr) {
		debugLogA("invalid file format for avatar %S", pwszFilename);
		return 1;
	}
	szPayload.AppendFormat("%s;base64,", szMimeType);
	FILE *in = _wfopen(pwszFilename, L"rb");
	if (in == nullptr) {
		debugLogA("cannot open avatar file %S for reading", pwszFilename);
		return 2;
	}

	int iFileLength = _filelength(_fileno(in));
	ptrA szFileContents((char*)mir_alloc(iFileLength));
	fread(szFileContents, 1, iFileLength, in);
	fclose(in);
	szPayload.Append(ptrA(mir_base64_encode(szFileContents.get(), iFileLength)));

	JSONNode root; root << CHAR_PARAM("avatar", szPayload);
	Push(new AsyncHttpRequest(this, REQUEST_PATCH, "/users/@me", nullptr, &root));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::CheckAvatarChange(MCONTACT hContact, const CMStringW &wszNewHash)
{
	if (wszNewHash.IsEmpty())
		return;

	ptrW wszOldAvatar(getWStringA(hContact, DB_KEY_AVHASH));

	// if avatar's hash changed, we need to request a new one
	if (mir_wstrcmp(wszNewHash, wszOldAvatar)) {
		setWString(hContact, DB_KEY_AVHASH, wszNewHash);
		RetrieveAvatar(hContact);
	}
}
