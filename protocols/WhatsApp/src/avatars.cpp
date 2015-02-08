#include "common.h"

INT_PTR WhatsAppProto::GetAvatarInfo(WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATIONT* AI = (PROTO_AVATAR_INFORMATIONT*)lParam;

	ptrA id(getStringA(AI->hContact, isChatRoom(AI->hContact) ? "ChatRoomID" : WHATSAPP_KEY_ID));
	if (id == NULL)
		return GAIR_NOAVATAR;

	std::tstring tszFileName = GetAvatarFileName(AI->hContact);
	_tcsncpy_s(AI->filename, tszFileName.c_str(), _TRUNCATE);
	AI->format = PA_FORMAT_JPEG;

	ptrA szAvatarId(getStringA(AI->hContact, WHATSAPP_KEY_AVATAR_ID));
	if (szAvatarId == NULL || (wParam & GAIF_FORCE) != 0)
		if (AI->hContact != NULL && m_pConnection != NULL) {
			m_pConnection->sendGetPicture(id, "preview");
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

std::tstring WhatsAppProto::GetAvatarFileName(MCONTACT hContact)
{
	std::tstring result = m_tszAvatarFolder + _T("\\");

	std::string jid;
	if (hContact != NULL) {
		ptrA szId(getStringA(hContact, isChatRoom(hContact) ? "ChatRoomID" : WHATSAPP_KEY_ID));
		if (szId == NULL)
			return _T("");

		jid = szId;
	}
	else jid = m_szJid;

	return result + std::tstring(_A2T(jid.c_str())) + _T(".jpg");
}

INT_PTR WhatsAppProto::GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	std::tstring tszOwnAvatar(m_tszAvatarFolder + _T("\\myavatar.jpg"));
	_tcsncpy_s((TCHAR*)wParam, lParam, tszOwnAvatar.c_str(), _TRUNCATE);
	return 0;
}

static std::vector<unsigned char>* sttFileToMem(const TCHAR *ptszFileName)
{
	HANDLE hFile = CreateFile(ptszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

	DWORD upperSize, lowerSize = GetFileSize(hFile, &upperSize);
	std::vector<unsigned char> *result = new std::vector<unsigned char>(lowerSize);
	ReadFile(hFile, (void*)result->data(), lowerSize, &upperSize, NULL);
	CloseHandle(hFile);
	return result;
}

int WhatsAppProto::InternalSetAvatar(MCONTACT hContact, const char *szJid, const TCHAR *ptszFileName)
{
	if (!isOnline() || ptszFileName == NULL)
		return 1;

	if (_taccess(ptszFileName, 4) != 0)
		return errno;

	ResizeBitmap resize = { 0 };
	if ((resize.hBmp = (HBITMAP)CallService(MS_UTILS_LOADBITMAPT, 0, (LPARAM)ptszFileName)) == NULL)
		return 2;
	resize.size = sizeof(resize);
	resize.fit = RESIZEBITMAP_KEEP_PROPORTIONS;
	resize.max_height = resize.max_width = 96;

	HBITMAP hbmpPreview = (HBITMAP)CallService(MS_IMG_RESIZE, (LPARAM)&resize, 0);
	if (hbmpPreview == NULL)
		return 3;

	TCHAR tszTempFile[MAX_PATH], tszMyFile[MAX_PATH];
	if (hContact == NULL) {
		mir_sntprintf(tszMyFile, SIZEOF(tszMyFile), _T("%s\\myavatar.jpg"), m_tszAvatarFolder.c_str());
		mir_sntprintf(tszTempFile, SIZEOF(tszTempFile), _T("%s\\myavatar.preview.jpg"), m_tszAvatarFolder.c_str());
	}
	else {
		std::tstring tszContactAva = GetAvatarFileName(hContact);
		_tcsncpy_s(tszMyFile, tszContactAva.c_str(), _TRUNCATE);
		_tcsncpy_s(tszTempFile, (tszContactAva + _T(".preview")).c_str(), _TRUNCATE);
	}

	IMGSRVC_INFO saveInfo = { sizeof(saveInfo), 0 };
	saveInfo.hbm = hbmpPreview;
	saveInfo.tszName = tszTempFile;
	saveInfo.dwMask = IMGI_HBITMAP;
	saveInfo.fif = FIF_JPEG;
	CallService(MS_IMG_SAVE, (WPARAM)&saveInfo, IMGL_TCHAR);

	if (hbmpPreview != resize.hBmp)
		DeleteObject(hbmpPreview);
	DeleteObject(resize.hBmp);

	CopyFile(ptszFileName, tszMyFile, FALSE);

	m_pConnection->sendSetPicture(szJid, sttFileToMem(ptszFileName), sttFileToMem(tszTempFile));
	return 0;
}

INT_PTR WhatsAppProto::SetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	return InternalSetAvatar(NULL, m_szJid.c_str(), (const TCHAR*)lParam);
}
