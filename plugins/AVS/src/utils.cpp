/*
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

#include "commonheaders.h"

void mir_sleep(int time)
{
	if (!g_shutDown)
		WaitForSingleObject(hShutdownEvent, time);
}

/////////////////////////////////////////////////////////////////////////////////////////
// substitutes variables and passes our own data path as base

void MyPathToAbsolute(const TCHAR *ptszPath, TCHAR *ptszDest)
{
	PathToAbsoluteT(VARST(ptszPath), ptszDest, g_szDataPath);
}

/////////////////////////////////////////////////////////////////////////////////////////
// convert the avatar image path to a relative one...
// given: contact handle, path to image

void MakePathRelative(MCONTACT hContact, TCHAR *path)
{
	TCHAR szFinalPath[MAX_PATH];
	szFinalPath[0] = '\0';

	size_t result = PathToRelativeT(path, szFinalPath, g_szDataPath);
	if (result && szFinalPath[0] != '\0') {
		db_set_ts(hContact, "ContactPhoto", "RFile", szFinalPath);
		if (!db_get_b(hContact, "ContactPhoto", "Locked", 0))
			db_set_ts(hContact, "ContactPhoto", "Backup", szFinalPath);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// convert the avatar image path to a relative one...
// given: contact handle

void MakePathRelative(MCONTACT hContact)
{
	ptrT tszPath(db_get_tsa(hContact, "ContactPhoto", "File"));
	if (tszPath)
		MakePathRelative(hContact, tszPath);
}

/////////////////////////////////////////////////////////////////////////////////////////
// create the avatar in cache
// returns 0 if not created (no avatar), iIndex otherwise, -2 if has to request avatar, -3 if avatar too big

int CreateAvatarInCache(MCONTACT hContact, avatarCacheEntry *ace, char *szProto)
{
	ptrT  tszValue;
	TCHAR tszFilename[MAX_PATH]; tszFilename[0] = 0;

	ace->hbmPic = 0;
	ace->dwFlags = 0;
	ace->bmHeight = 0;
	ace->bmWidth = 0;
	ace->lpDIBSection = NULL;
	ace->szFilename[0] = 0;

	if (szProto == NULL) {
		char *proto = GetContactProto(hContact);
		if (proto == NULL || !db_get_b(NULL, AVS_MODULE, proto, 1))
			return -1;

		if (db_get_b(hContact, "ContactPhoto", "Locked", 0) && (tszValue = db_get_tsa(hContact, "ContactPhoto", "Backup")))
			MyPathToAbsolute(tszValue, tszFilename);
		else if (tszValue = db_get_tsa(hContact, "ContactPhoto", "RFile"))
			MyPathToAbsolute(tszValue, tszFilename);
		else if (tszValue = db_get_tsa(hContact, "ContactPhoto", "File"))
			MyPathToAbsolute(tszValue, tszFilename);
		else return -2;
	}
	else {
		if (hContact == 0) {				// create a protocol picture in the proto picture cache
			if (tszValue = db_get_tsa(NULL, PPICT_MODULE, szProto))
				MyPathToAbsolute(tszValue, tszFilename);
			else if (mir_strcmp(szProto, AVS_DEFAULT)) {
				if (tszValue = db_get_tsa(NULL, PPICT_MODULE, AVS_DEFAULT))
					MyPathToAbsolute(tszValue, tszFilename);

				if (!strstr(szProto, "Global avatar for")) {
					PROTOACCOUNT* pdescr = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)szProto);
					if (pdescr == NULL)
						return -1;
					char key[MAX_PATH];
					mir_snprintf(key, SIZEOF(key), "Global avatar for %s accounts", pdescr->szProtoName);
					if (tszValue = db_get_tsa(NULL, PPICT_MODULE, key))
						MyPathToAbsolute(tszValue, tszFilename);
				}
			}
		}
		else if (hContact == INVALID_CONTACT_ID) {
			// create own picture - note, own avatars are not on demand, they are loaded once at
			// startup and everytime they are changed.
			if (szProto[0] == '\0') {
				// Global avatar
				if (tszValue = db_get_tsa(NULL, AVS_MODULE, "GlobalUserAvatarFile"))
					MyPathToAbsolute(tszValue, tszFilename);
				else
					return -10;
			}
			else if (ProtoServiceExists(szProto, PS_GETMYAVATART)) {
				if (CallProtoService(szProto, PS_GETMYAVATART, (WPARAM)tszFilename, (LPARAM)MAX_PATH))
					tszFilename[0] = '\0';
			}
			else if (ProtoServiceExists(szProto, PS_GETMYAVATAR)) {
				char szFileName[MAX_PATH];
				if (CallProtoService(szProto, PS_GETMYAVATAR, (WPARAM)szFileName, (LPARAM)MAX_PATH))
					tszFilename[0] = '\0';
				else
					MultiByteToWideChar(CP_ACP, 0, szFileName, -1, tszFilename, SIZEOF(tszFilename));
			}
			else if (tszValue = db_get_tsa(NULL, szProto, "AvatarFile"))
				MyPathToAbsolute(tszValue, tszFilename);
			else return -1;
		}
	}

	if (mir_tstrlen(tszFilename) < 4)
		return -1;

	_tcsncpy_s(tszFilename, VARST(tszFilename), _TRUNCATE);
	if (_taccess(tszFilename, 4) == -1)
		return -2;

	BOOL isTransparentImage = 0;
	ace->hbmPic = BmpFilterLoadBitmap(&isTransparentImage, tszFilename);
	ace->dwFlags = 0;
	ace->bmHeight = 0;
	ace->bmWidth = 0;
	ace->lpDIBSection = NULL;
	_tcsncpy(ace->szFilename, tszFilename, MAX_PATH);
	ace->szFilename[MAX_PATH - 1] = 0;
	if (ace->hbmPic == 0)
		return -1;

	BITMAP bminfo;
	GetObject(ace->hbmPic, sizeof(bminfo), &bminfo);

	ace->cbSize = sizeof(avatarCacheEntry);
	ace->dwFlags = AVS_BITMAP_VALID;
	if (hContact != NULL && db_get_b(hContact, "ContactPhoto", "Hidden", 0))
		ace->dwFlags |= AVS_HIDEONCLIST;
	ace->hContact = hContact;
	ace->bmHeight = bminfo.bmHeight;
	ace->bmWidth = bminfo.bmWidth;

	BOOL noTransparency = db_get_b(0, AVS_MODULE, "RemoveAllTransparency", 0);

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
		if (!noTransparency && !isTransparentImage && db_get_b(hContact, "ContactPhoto", "MakeTransparentBkg", db_get_b(0, AVS_MODULE, "MakeTransparentBkg", 0))) {
			if (MakeTransparentBkg(hContact, &ace->hbmPic)) {
				ace->dwFlags |= AVS_CUSTOMTRANSPBKG | AVS_HASTRANSPARENCY;
				GetObject(ace->hbmPic, sizeof(bminfo), &bminfo);
				isTransparentImage = TRUE;
			}
		}
	}
	else if (hContact == INVALID_CONTACT_ID) { // My avatars
		if (!noTransparency && !isTransparentImage && db_get_b(0, AVS_MODULE, "MakeTransparentBkg", 0) && db_get_b(0, AVS_MODULE, "MakeMyAvatarsTransparent", 0)) {
			if (MakeTransparentBkg(0, &ace->hbmPic)) {
				ace->dwFlags |= AVS_CUSTOMTRANSPBKG | AVS_HASTRANSPARENCY;
				GetObject(ace->hbmPic, sizeof(bminfo), &bminfo);
				isTransparentImage = TRUE;
			}
		}
	}

	if (db_get_b(0, AVS_MODULE, "MakeGrayscale", 0))
		ace->hbmPic = MakeGrayscale(ace->hbmPic);

	if (noTransparency) {
		fei->FI_CorrectBitmap32Alpha(ace->hbmPic, TRUE);
		isTransparentImage = FALSE;
	}

	if (bminfo.bmBitsPixel == 32 && isTransparentImage) {
		if (fei->FI_Premultiply(ace->hbmPic))
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

int GetFileHash(TCHAR* filename)
{
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	int remainder = 0;
	char data[1024];
	DWORD dwRead;
	do {
		// Read file chunk
		dwRead = 0;
		ReadFile(hFile, data, 1024, &dwRead, NULL);

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
	}
		while (dwRead == 1024);

	CloseHandle(hFile);

	return remainder;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

protoPicCacheEntry::~protoPicCacheEntry()
{
	if (hbmPic != 0)
		DeleteObject(hbmPic);
	mir_free(szProtoname);
	mir_free(tszAccName);
}

void protoPicCacheEntry::clear()
{
	if (hbmPic != 0)
		DeleteObject(hbmPic);

	memset(this, 0, sizeof(avatarCacheEntry));
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
		POINT maxSize;
		CallProtoService(proto, PS_GETAVATARCAPS, AF_MAXSIZE, (LPARAM)&maxSize);
		*width = maxSize.y;
		*height = maxSize.x;
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

protoPicCacheEntry* GetProtoDefaultAvatar(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (szProto) {
		for (int i = 0; i < g_ProtoPictures.getCount(); i++) {
			protoPicCacheEntry& p = g_ProtoPictures[i];
			if (!mir_strcmp(p.szProtoname, szProto) && p.hbmPic != NULL)
				return &g_ProtoPictures[i];
		}
	}
	return NULL;
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
	CacheNode *node = FindAvatarInCache(hContact, g_AvatarHistoryAvail && fNotifyHist, TRUE);
	if (node == NULL)
		return 0;

	if (fNotifyHist)
		node->dwFlags |= AVH_MUSTNOTIFY;

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
	DBVARIANT dbv = { 0 };
	if (db_get_ts(NULL, AVS_MODULE, "GlobalUserAvatarFile", &dbv))
		return;

	TCHAR szFilename[MAX_PATH];
	MyPathToAbsolute(dbv.ptszVal, szFilename);
	db_free(&dbv);

	DeleteFile(szFilename);
	db_unset(NULL, AVS_MODULE, "GlobalUserAvatarFile");
}

void SetIgnoreNotify(char *protocol, BOOL ignore)
{
	for (int i = 0; i < g_MyAvatars.getCount(); i++) {
		if (protocol == NULL || !mir_strcmp(g_MyAvatars[i].szProtoname, protocol)) {
			if (ignore)
				g_MyAvatars[i].dwFlags |= AVS_IGNORENOTIFY;
			else
				g_MyAvatars[i].dwFlags &= ~AVS_IGNORENOTIFY;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD GetFileSize(TCHAR *szFilename)
{
	struct _stat info;
	return (_tstat(szFilename, &info) == -1) ? 0 : info.st_size;
}
