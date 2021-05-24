/*
Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)
Copyright (C) 2006 Ricardo Pescuma Domenecci, Nightwish

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

void mir_sleep(int time)
{
	if (!g_shutDown)
		WaitForSingleObject(hShutdownEvent, time);
}

/////////////////////////////////////////////////////////////////////////////////////////
// substitutes variables and passes our own data path as base

void MyPathToAbsolute(const wchar_t *ptszPath, wchar_t *ptszDest)
{
	PathToAbsoluteW(VARSW(ptszPath), ptszDest, g_szDataPath);
}

/////////////////////////////////////////////////////////////////////////////////////////
// convert the avatar image path to a relative one...
// given: contact handle, path to image

void MakePathRelative(MCONTACT hContact, wchar_t *path)
{
	wchar_t szFinalPath[MAX_PATH];
	szFinalPath[0] = '\0';

	size_t result = PathToRelativeW(path, szFinalPath, g_szDataPath);
	if (result && szFinalPath[0] != '\0') {
		db_set_ws(hContact, "ContactPhoto", "RFile", szFinalPath);
		if (!db_get_b(hContact, "ContactPhoto", "Locked", 0))
			db_set_ws(hContact, "ContactPhoto", "Backup", szFinalPath);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// convert the avatar image path to a relative one...
// given: contact handle

void MakePathRelative(MCONTACT hContact)
{
	ptrW tszPath(db_get_wsa(hContact, "ContactPhoto", "File"));
	if (tszPath)
		MakePathRelative(hContact, tszPath);
}

/////////////////////////////////////////////////////////////////////////////////////////
// create the avatar in cache
// returns 0 if not created (no avatar), iIndex otherwise, -2 if has to request avatar, -3 if avatar too big

int CreateAvatarInCache(MCONTACT hContact, AVATARCACHEENTRY *ace, const char *szProto)
{
	ptrW  tszValue;
	wchar_t tszFilename[MAX_PATH]; tszFilename[0] = 0;

	ace->hbmPic = nullptr;
	ace->dwFlags = 0;
	ace->bmHeight = 0;
	ace->bmWidth = 0;
	ace->lpDIBSection = nullptr;
	ace->szFilename[0] = 0;

	if (szProto == nullptr) {
		char *proto = Proto_GetBaseAccountName(hContact);
		if (proto == nullptr || !g_plugin.getByte(proto, 1))
			return -1;

		if (db_get_b(hContact, "ContactPhoto", "Locked", 0) && (tszValue = db_get_wsa(hContact, "ContactPhoto", "Backup")))
			MyPathToAbsolute(tszValue, tszFilename);
		else if (tszValue = db_get_wsa(hContact, "ContactPhoto", "RFile"))
			MyPathToAbsolute(tszValue, tszFilename);
		else if (tszValue = db_get_wsa(hContact, "ContactPhoto", "File"))
			MyPathToAbsolute(tszValue, tszFilename);
		else return -2;
	}
	else {
		if (hContact == 0) {				// create a protocol picture in the proto picture cache
			if (tszValue = db_get_wsa(0, PPICT_MODULE, szProto))
				MyPathToAbsolute(tszValue, tszFilename);
			else if (mir_strcmp(szProto, AVS_DEFAULT)) {
				if (tszValue = db_get_wsa(0, PPICT_MODULE, AVS_DEFAULT))
					MyPathToAbsolute(tszValue, tszFilename);

				if (!strstr(szProto, "Global avatar for")) {
					PROTOACCOUNT *pdescr = Proto_GetAccount(szProto);
					if (pdescr == nullptr)
						return -1;
					char key[MAX_PATH];
					mir_snprintf(key, "Global avatar for %s accounts", pdescr->szProtoName);
					if (tszValue = db_get_wsa(0, PPICT_MODULE, key))
						MyPathToAbsolute(tszValue, tszFilename);
				}
			}
		}
		else if (hContact == INVALID_CONTACT_ID) {
			// create own picture - note, own avatars are not on demand, they are loaded once at
			// startup and everytime they are changed.
			if (szProto[0] == '\0') {
				// Global avatar
				if (tszValue = g_plugin.getWStringA("GlobalUserAvatarFile"))
					MyPathToAbsolute(tszValue, tszFilename);
				else
					return -10;
			}
			else if (ProtoServiceExists(szProto, PS_GETMYAVATAR)) {
				if (CallProtoService(szProto, PS_GETMYAVATAR, (WPARAM)tszFilename, (LPARAM)MAX_PATH))
					tszFilename[0] = '\0';
			}
			else if (ProtoServiceExists(szProto, PS_GETMYAVATAR)) {
				char szFileName[MAX_PATH];
				if (CallProtoService(szProto, PS_GETMYAVATAR, (WPARAM)szFileName, (LPARAM)MAX_PATH))
					tszFilename[0] = '\0';
				else
					MultiByteToWideChar(CP_ACP, 0, szFileName, -1, tszFilename, _countof(tszFilename));
			}
			else if (tszValue = db_get_wsa(0, szProto, "AvatarFile"))
				MyPathToAbsolute(tszValue, tszFilename);
			else return -1;
		}
	}

	if (mir_wstrlen(tszFilename) < 4)
		return -1;

	wcsncpy_s(tszFilename, VARSW(tszFilename), _TRUNCATE);
	if (_waccess(tszFilename, 4) == -1)
		return -2;

	BOOL isTransparentImage = 0;
	ace->hbmPic = BmpFilterLoadBitmap(&isTransparentImage, tszFilename);
	ace->dwFlags = 0;
	ace->bmHeight = 0;
	ace->bmWidth = 0;
	ace->lpDIBSection = nullptr;
	wcsncpy(ace->szFilename, tszFilename, MAX_PATH);
	ace->szFilename[MAX_PATH - 1] = 0;
	if (ace->hbmPic == nullptr)
		return -1;

	BITMAP bminfo;
	GetObject(ace->hbmPic, sizeof(bminfo), &bminfo);

	ace->dwFlags = AVS_BITMAP_VALID;
	if (hContact != NULL && db_get_b(hContact, "ContactPhoto", "Hidden", 0))
		ace->dwFlags |= AVS_HIDEONCLIST;
	ace->hContact = hContact;
	ace->bmHeight = bminfo.bmHeight;
	ace->bmWidth = bminfo.bmWidth;

	BOOL noTransparency = g_plugin.getByte("RemoveAllTransparency", 0);

	// Calc image hash
	if (hContact != 0 && hContact != INVALID_CONTACT_ID) {
		// Have to reset settings? -> do it if image changed
		DWORD imgHash = GetImgHash(ace->hbmPic);
		if (imgHash != db_get_dw(hContact, "ContactPhoto", "ImageHash", 0)) {
			db_unset(hContact, "ContactPhoto", "MakeTransparentBkg");
			db_unset(hContact, "ContactPhoto", "TranspBkgNumPoints");
			db_unset(hContact, "ContactPhoto", "TranspBkgColorDiff");

			db_set_dw(hContact, "ContactPhoto", "ImageHash", imgHash);
		}

		// Make transparent?
		if (!noTransparency && !isTransparentImage && db_get_b(hContact, "ContactPhoto", "MakeTransparentBkg", g_plugin.getByte("MakeTransparentBkg", 0))) {
			if (MakeTransparentBkg(hContact, &ace->hbmPic)) {
				ace->dwFlags |= AVS_CUSTOMTRANSPBKG | AVS_HASTRANSPARENCY;
				GetObject(ace->hbmPic, sizeof(bminfo), &bminfo);
				isTransparentImage = TRUE;
			}
		}
	}
	else if (hContact == INVALID_CONTACT_ID) { // My avatars
		if (!noTransparency && !isTransparentImage && g_plugin.getByte("MakeTransparentBkg", 0) && g_plugin.getByte("MakeMyAvatarsTransparent", 0)) {
			if (MakeTransparentBkg(0, &ace->hbmPic)) {
				ace->dwFlags |= AVS_CUSTOMTRANSPBKG | AVS_HASTRANSPARENCY;
				GetObject(ace->hbmPic, sizeof(bminfo), &bminfo);
				isTransparentImage = TRUE;
			}
		}
	}

	if (g_plugin.getByte("MakeGrayscale", 0))
		ace->hbmPic = MakeGrayscale(ace->hbmPic);

	if (noTransparency) {
		FreeImage_CorrectBitmap32Alpha(ace->hbmPic, TRUE);
		isTransparentImage = FALSE;
	}

	if (bminfo.bmBitsPixel == 32 && isTransparentImage) {
		if (FreeImage_Premultiply(ace->hbmPic))
			ace->dwFlags |= AVS_HASTRANSPARENCY;

		ace->dwFlags |= AVS_PREMULTIPLIED;
	}

	if (szProto) {
		protoPicCacheEntry *pAce = (protoPicCacheEntry *)ace;
		if (hContact == 0)
			pAce->dwFlags |= AVS_PROTOPIC;
		else if (hContact == INVALID_CONTACT_ID)
			pAce->dwFlags |= AVS_OWNAVATAR;
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

#define POLYNOMIAL (0x488781ED) /* This is the CRC Poly */
#define TOPBIT (1 << (WIDTH - 1)) /* MSB */
#define WIDTH 32

int GetFileHash(wchar_t *filename)
{
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	int remainder = 0;
	char data[1024];
	DWORD dwRead;
	do {
		// Read file chunk
		dwRead = 0;
		ReadFile(hFile, data, 1024, &dwRead, nullptr);

		/* loop through each byte of data */
		for (int byte = 0; byte < (int)dwRead; ++byte) {
			/* store the next byte into the remainder */
			remainder ^= (data[byte] << (WIDTH - 8));
			/* calculate for all 8 bits in the byte */
			for (int bit = 8; bit > 0; --bit) {
				/* check if MSB of remainder is a one */
				if (remainder & TOPBIT)
					remainder = (remainder << 1) ^ POLYNOMIAL;
				else
					remainder = (remainder << 1);
			}
		}
	} while (dwRead == 1024);

	CloseHandle(hFile);

	return remainder;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

protoPicCacheEntry::protoPicCacheEntry(int _type, const char *_szName) :
	cacheType(_type),
	szProtoname(mir_strdup(_szName))
{
}

protoPicCacheEntry::~protoPicCacheEntry()
{
	mir_free(szProtoname);

	if (hbmPic != nullptr)
		DeleteObject(hbmPic);
}

void protoPicCacheEntry::clear()
{
	if (hbmPic != nullptr)
		DeleteObject(hbmPic);

	memset(this, 0, sizeof(AVATARCACHEENTRY));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

BOOL Proto_IsAvatarsEnabled(const char *proto)
{
	if (ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_ENABLED, 0);

	return TRUE;
}

BOOL Proto_IsAvatarFormatSupported(const char *proto, int format)
{
	if (ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_FORMATSUPPORTED, format);

	if (format >= PA_FORMAT_SWF)
		return FALSE;

	return TRUE;
}

int Proto_AvatarImageProportion(const char *proto)
{
	if (ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_PROPORTION, 0);

	return 0;
}

void Proto_GetAvatarMaxSize(const char *proto, int *width, int *height)
{
	if (ProtoServiceExists(proto, PS_GETAVATARCAPS)) {
		POINT maxSize = { 300, 300 };
		CallProtoService(proto, PS_GETAVATARCAPS, AF_MAXSIZE, (LPARAM)&maxSize);
		*width = maxSize.x;
		*height = maxSize.y;
	}
	else {
		*width = 300;
		*height = 300;
	}

	if (*width < 0)
		*width = 0;
	else if (*width > 300)
		*width = 300;

	if (*height < 0)
		*height = 0;
	else if (*height > 300)
		*height = 300;
}

BOOL Proto_NeedDelaysForAvatars(const char *proto)
{
	if (ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_DONTNEEDDELAYS, 0) <= 0;

	return TRUE;
}

int Proto_GetAvatarMaxFileSize(const char *proto)
{
	if (ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_MAXFILESIZE, 0);

	return 0;
}

int Proto_GetDelayAfterFail(const char *proto)
{
	if (ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_DELAYAFTERFAIL, 0);

	return 0;
}

BOOL Proto_IsFetchingWhenProtoNotVisibleAllowed(const char *proto)
{
	if (ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_FETCHIFPROTONOTVISIBLE, 0);

	return FALSE;
}

BOOL Proto_IsFetchingWhenContactOfflineAllowed(const char *proto)
{
	if (ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_FETCHIFCONTACTOFFLINE, 0);

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

protoPicCacheEntry *GetProtoDefaultAvatar(MCONTACT hContact)
{
	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto)
		for (auto &p : g_ProtoPictures)
			if (!mir_strcmp(p->szProtoname, szProto) && p->hbmPic != nullptr)
				return p;

	return nullptr;
}

MCONTACT GetContactThatHaveTheAvatar(MCONTACT hContact, int locked)
{
	if (db_mc_isMeta(hContact)) {
		if (locked == -1)
			locked = db_get_b(hContact, "ContactPhoto", "Locked", 0);

		if (!locked)
			hContact = db_mc_getMostOnline(hContact);
	}
	return hContact;
}

int ChangeAvatar(MCONTACT hContact, bool fLoad, bool fNotifyHist, int pa_format)
{
	if (g_shutDown)
		return 0;

	hContact = GetContactThatHaveTheAvatar(hContact);

	// Get the node
	CacheNode *node = FindAvatarInCache(hContact, g_AvatarHistoryAvail && fNotifyHist, true);
	if (node == nullptr)
		return 0;

	if (fNotifyHist)
		node->bNotify = true;

	node->pa_format = pa_format;
	if (fLoad) {
		PushAvatarRequest(node);
		SetEvent(hLoaderEvent);
	}
	else node->wipeInfo();

	return 0;
}

void DeleteGlobalUserAvatar()
{
	ptrW wszPath(g_plugin.getWStringA("GlobalUserAvatarFile"));
	if (!wszPath)
		return;

	wchar_t szFilename[MAX_PATH];
	MyPathToAbsolute(wszPath, szFilename);

	DeleteFile(szFilename);
	g_plugin.delSetting("GlobalUserAvatarFile");
}

void SetIgnoreNotify(char *protocol, BOOL ignore)
{
	for (auto &it : g_MyAvatars) {
		if (protocol == nullptr || !mir_strcmp(it->szProtoname, protocol)) {
			if (ignore)
				it->dwFlags |= AVS_IGNORENOTIFY;
			else
				it->dwFlags &= ~AVS_IGNORENOTIFY;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD GetFileSize(wchar_t *szFilename)
{
	struct _stat info;
	return (_wstat(szFilename, &info) == -1) ? 0 : info.st_size;
}
