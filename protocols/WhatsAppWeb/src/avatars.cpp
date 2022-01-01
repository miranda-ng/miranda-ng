/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"

void WhatsAppProto::OnGetAvatarInfo(const JSONNode &root, void *pUserInfo)
{
	if (!root) return;

	MCONTACT hContact = (UINT_PTR)pUserInfo;

	PROTO_AVATAR_INFORMATION ai = {};
	ai.hContact = hContact;
	ai.format = PA_FORMAT_JPEG;
	wcsncpy_s(ai.filename, GetAvatarFileName(hContact), _TRUNCATE);

	DWORD dwLastChangeTime = _wtoi(root["tag"].as_mstring());

	CMStringA szUrl(root["eurl"].as_mstring());
	if (szUrl.IsEmpty()) {
		setDword(hContact, DBKEY_AVATAR_TAG, 0); // avatar doesn't exist, don't check it later

LBL_Error:
		ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, HANDLE(&ai));
		return;
	}

	// if avatar was changed or not present at all, download it
	if (dwLastChangeTime > getDword(hContact, DBKEY_AVATAR_TAG)) {
		if (!g_plugin.SaveFile(szUrl, ai))
			goto LBL_Error;

		// set timestamp of avatar being saved
		setDword(hContact, DBKEY_AVATAR_TAG, dwLastChangeTime);
	}
	ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, HANDLE(&ai));
}

INT_PTR WhatsAppProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION*)lParam;

	ptrA id(getStringA(pai->hContact, isChatRoom(pai->hContact) ? "ChatRoomID" : DBKEY_ID));
	if (id == NULL)
		return GAIR_NOAVATAR;

	CMStringW tszFileName(GetAvatarFileName(pai->hContact));
	wcsncpy_s(pai->filename, tszFileName.c_str(), _TRUNCATE);
	pai->format = PA_FORMAT_JPEG;

	DWORD dwTag = getDword(pai->hContact, DBKEY_AVATAR_TAG, -1);
	if (dwTag == -1 || (wParam & GAIF_FORCE) != 0)
		if (pai->hContact != NULL && isOnline()) {
			WSSend(CMStringA(FORMAT, "[\"query\",\"ProfilePicThumb\",\"%s\"]", id.get()), &WhatsAppProto::OnGetAvatarInfo, (void*)pai->hContact);
			return GAIR_WAITFOR;
		}

	debugLogA("No avatar");
	return GAIR_NOAVATAR;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR WhatsAppProto::GetAvatarCaps(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case AF_PROPORTION:
		return PIP_SQUARE;

	case AF_FORMATSUPPORTED: // Jabber supports avatars of virtually all formats
		return PA_FORMAT_JPEG;

	case AF_ENABLED:
		return TRUE;

	case AF_MAXSIZE:
		POINT *size = (POINT*)lParam;
		if (size)
			size->x = size->y = 640;
		return 0;
	}
	return -1;
}

CMStringW WhatsAppProto::GetAvatarFileName(MCONTACT hContact)
{
	CMStringW result = m_tszAvatarFolder + L"\\";

	CMStringA jid;
	if (hContact != NULL) {
		ptrA szId(getStringA(hContact, isChatRoom(hContact) ? "ChatRoomID" : DBKEY_ID));
		if (szId == NULL)
			return L"";

		jid = szId;
	}
	else jid = m_szJid;

	return result + _A2T(jid.c_str()) + L".jpg";
}

INT_PTR WhatsAppProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	std::wstring tszOwnAvatar(m_tszAvatarFolder + L"\\myavatar.jpg");
	wcsncpy_s((wchar_t*)wParam, lParam, tszOwnAvatar.c_str(), _TRUNCATE);
	return 0;
}

INT_PTR WhatsAppProto::SetMyAvatar(WPARAM, LPARAM)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CMPlugin::SaveFile(const char *pszUrl, PROTO_AVATAR_INFORMATION &ai)
{
	NETLIBHTTPREQUEST req = {};
	req.cbSize = sizeof(req);
	req.flags = NLHRF_NODUMP | NLHRF_PERSISTENT | NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT;
	req.requestType = REQUEST_GET;
	req.szUrl = (char*)pszUrl;
	req.nlc = hAvatarConn;

	NETLIBHTTPREQUEST *pReply = Netlib_HttpTransaction(hAvatarUser, &req);
	if (pReply == nullptr) {
		hAvatarConn = nullptr;
		debugLogA("Failed to retrieve avatar from url: %s", pszUrl);
		return false;
	}

	hAvatarConn = pReply->nlc;

	bool bSuccess = false;
	if (pReply->resultCode == 200 && pReply->pData && pReply->dataLength) {
		if (auto *pszHdr = Netlib_GetHeader(pReply, "Content-Type"))
			ai.format = ProtoGetAvatarFormatByMimeType(pszHdr);

		if (ai.format != PA_FORMAT_UNKNOWN) {
			FILE *fout = _wfopen(ai.filename, L"wb");
			if (fout) {
				fwrite(pReply->pData, 1, pReply->dataLength, fout);
				fclose(fout);
				bSuccess = true;
			}
			else debugLogA("Error saving avatar to file %S", ai.filename);
		}
		else debugLogA("unknown avatar mime type");
	}
	else debugLogA("Error %d reading avatar from url: %s", pReply->resultCode, pszUrl);

	Netlib_FreeHttpRequest(pReply);
	return bSuccess;
}
