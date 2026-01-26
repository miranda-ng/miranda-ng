/*

WhatsApp plugin for Miranda NG
Copyright © 2019-26 George Hazan

*/

#include "stdafx.h"

void WhatsAppProto::OnIqGetAvatar(const WANode &node, void *pUserInfo)
{
	auto *pUser = (WAUser *)pUserInfo;

	PROTO_AVATAR_INFORMATION ai = {};
	ai.hContact = pUser->hContact;
	ai.format = PA_FORMAT_JPEG;
	wcsncpy_s(ai.filename, GetAvatarFileName(pUser->hContact), _TRUNCATE);

	auto *pNode = node.getChild("picture");

	CMStringA szUrl(pNode->getAttr("url"));
	if (szUrl.IsEmpty()) {
		setString(pUser->hContact, DBKEY_AVATAR_HASH, "-"); // avatar doesn't exist, don't check it later

LBL_Error:
		ProtoBroadcastAck(pUser->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, &ai);
		return;
	}

	// if avatar was changed or not present at all, download it
	CMStringA szHash(pNode->getAttr("hash"));
	if (szHash != getMStringA(pUser->hContact, DBKEY_AVATAR_HASH)) {
		if (!g_plugin.SaveFile(szUrl, ai))
			goto LBL_Error;

		// set timestamp of avatar being saved
		setString(pUser->hContact, DBKEY_AVATAR_HASH, szHash);
	}
	ProtoBroadcastAck(pUser->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &ai);
}

INT_PTR WhatsAppProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION*)lParam;

	if (pai->hContact == 0)
		pai->hContact = m_ownContact;

	ptrA jid(getStringA(pai->hContact, DBKEY_ID));
	if (jid == NULL)
		return GAIR_NOAVATAR;

	CMStringW tszFileName(GetAvatarFileName(pai->hContact));
	wcsncpy_s(pai->filename, tszFileName.c_str(), _TRUNCATE);
	pai->format = PA_FORMAT_JPEG;

	CMStringA szHash(getMStringA(pai->hContact, DBKEY_AVATAR_HASH));
	if (szHash.IsEmpty() || (wParam & GAIF_FORCE) != 0)
		if (pai->hContact != NULL && isOnline()) {
			ServerFetchAvatar(jid);
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
	CMStringW result = GetAvatarPath() + L"\\";

	CMStringA jid;
	if (hContact != NULL) {
		ptrA szId(getStringA(hContact, DBKEY_ID));
		if (szId == NULL)
			return L"";

		jid = szId;
	}
	else jid = m_szJid;

	return result + _A2T(jid) + L".jpg";
}

INT_PTR WhatsAppProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	CMStringW wszOwnAvatar(GetAvatarFileName(m_ownContact));
	wcsncpy_s((wchar_t*)wParam, lParam, wszOwnAvatar, _TRUNCATE);
	return 0;
}

INT_PTR WhatsAppProto::SetMyAvatar(WPARAM, LPARAM)
{
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void WhatsAppProto::ServerFetchAvatar(const char *jid)
{
	if (auto *pUser = FindUser(jid)) {
		WANodeIq iq(IQ::GET, "w:profile:picture");
		iq << XATTR("target", jid) << XCHILDP("picture") << CHAR_PARAM("type", "preview") << CHAR_PARAM("query", "url");
		WSSendNode(iq, &WhatsAppProto::OnIqGetAvatar, pUser);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

bool CMPlugin::SaveFile(const char *pszUrl, PROTO_AVATAR_INFORMATION &ai)
{
	MHttpRequest req(REQUEST_GET);
	req.flags = NLHRF_NODUMP | NLHRF_PERSISTENT | NLHRF_SSL | NLHRF_HTTP11 | NLHRF_REDIRECT;
	req.m_szUrl = pszUrl;
	req.nlc = hAvatarConn;

	NLHR_PTR pReply(Netlib_DownloadFile(hAvatarUser, &req, ai.filename));
	if (pReply == nullptr) {
		hAvatarConn = nullptr;
		debugLogA("Failed to retrieve avatar from url: %s", pszUrl);
		return false;
	}

	hAvatarConn = pReply->nlc;

	if (pReply->resultCode != 200) {
		debugLogA("Error %d reading avatar from url: %s", pReply->resultCode, pszUrl);
		return false;
	}

	return true;
}
