/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-21 George Hazan

*/

#include "stdafx.h"

INT_PTR WhatsAppProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION*)lParam;

	ptrA id(getStringA(pai->hContact, isChatRoom(pai->hContact) ? "ChatRoomID" : DBKEY_ID));
	if (id == NULL)
		return GAIR_NOAVATAR;

	CMStringW tszFileName(GetAvatarFileName(pai->hContact));
	wcsncpy_s(pai->filename, tszFileName.c_str(), _TRUNCATE);
	pai->format = PA_FORMAT_JPEG;

	ptrA szAvatarId(getStringA(pai->hContact, DBKEY_AVATAR_ID));
	if (szAvatarId == NULL || (wParam & GAIF_FORCE) != 0)
		if (pai->hContact != NULL && isOnline()) {
			// m_pConnection->sendGetPicture(id, "image");
			return GAIR_WAITFOR;
		}

	debugLogA("No avatar");
	return GAIR_NOAVATAR;
}

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
