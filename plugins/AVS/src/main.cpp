/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2004 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

HINSTANCE g_hInst = 0;
HICON g_hIcon = 0;
bool g_shutDown = false;

int hLangpack;

static TCHAR g_szDataPath[MAX_PATH];		// user datae path (read at startup only)
static BOOL g_MetaAvail = FALSE;
BOOL g_AvatarHistoryAvail = FALSE;
static long hwndSetMyAvatar = 0;

static HANDLE hMyAvatarsFolder;
static HANDLE hGlobalAvatarFolder;
static HANDLE hLoaderEvent, hShutdownEvent;
static HANDLE hEventContactAvatarChanged, hMyAvatarChanged;
HANDLE hEventChanged;

BOOL (WINAPI *AvsAlphaBlend)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION) = NULL;

static struct CacheNode *g_Cache = 0;
static CRITICAL_SECTION cachecs, alloccs;

static int ComparePicture( const protoPicCacheEntry* p1, const protoPicCacheEntry* p2 )
{
	if ((lstrcmpA(p1->szProtoname, "Global avatar") == 0) || strstr(p1->szProtoname, "Global avatar"))
		return -1;
	if ((lstrcmpA(p2->szProtoname, "Global avatar") == 0) || strstr(p1->szProtoname, "Global avatar"))
		return 1;
	return lstrcmpA( p1->szProtoname, p2->szProtoname );
}

OBJLIST<protoPicCacheEntry>
	g_ProtoPictures( 10, ComparePicture ),
	g_MyAvatars( 10, ComparePicture );

char* g_szMetaName = NULL;

#ifndef SHVIEW_THUMBNAIL
#define SHVIEW_THUMBNAIL 0x702D
#endif

// Stores the id of the dialog

int ChangeAvatar(HANDLE hContact, BOOL fLoad, BOOL fNotifyHist = FALSE, int pa_format = 0);

int OnDetailsInit(WPARAM wParam, LPARAM lParam);
int OptInit(WPARAM wParam, LPARAM lParam);

static int ShutdownProc(WPARAM wParam, LPARAM lParam);
static int OkToExitProc(WPARAM wParam, LPARAM lParam);
static int GetFileHash(TCHAR* filename);
static DWORD GetFileSize(TCHAR *szFilename);

void ProcessAvatarInfo(HANDLE hContact, int type, PROTO_AVATAR_INFORMATIONT *pai, const char *szProto);
int FetchAvatarFor(HANDLE hContact, char *szProto = NULL);
static INT_PTR ReportMyAvatarChanged(WPARAM wParam, LPARAM lParam);

BOOL Proto_IsAvatarsEnabled(const char *proto);
BOOL Proto_IsAvatarFormatSupported(const char *proto, int format);
void Proto_GetAvatarMaxSize(const char *proto, int *width, int *height);
int Proto_AvatarImageProportion(const char *proto);
BOOL Proto_NeedDelaysForAvatars(const char *proto);
int Proto_GetAvatarMaxFileSize(const char *proto);
int Proto_GetDelayAfterFail(const char *proto);

FI_INTERFACE *fei = 0;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {E00F1643-263C-4599-B84B-053E5C511D29}
	{0xe00f1643, 0x263c, 0x4599, {0xb8, 0x4b, 0x5, 0x3e, 0x5c, 0x51, 0x1d, 0x29}}
};

extern INT_PTR CALLBACK DlgProcAvatarOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static int SetProtoMyAvatar(char *protocol, HBITMAP hBmp, TCHAR *originalFilename, int format, BOOL square, BOOL grow);

/*
 * output a notification message.
 * may accept a hContact to include the contacts nickname in the notification message...
 * the actual message is using printf() rules for formatting and passing the arguments...
 *
 * can display the message either as systray notification (baloon popup) or using the
 * popup plugin.
 */

#ifdef _DEBUG

int _DebugTrace(const char *fmt, ...)
{
	char debug[2048];
	int ibsize = 2047;
	va_list va;
	va_start(va, fmt);

	mir_snprintf(debug, SIZEOF(debug) - 10, " ***** AVS [%08d] [ID:%04x]: ", GetTickCount(), GetCurrentThreadId());
	OutputDebugStringA(debug);
	mir_vsnprintf(debug, ibsize, fmt, va);
	OutputDebugStringA(debug);
	OutputDebugStringA(" ***** \n");

	return 0;
}

int _DebugTrace(HANDLE hContact, const char *fmt, ...)
{
	char text[1024];
	size_t len;
	va_list va;

	char *name = NULL;
	char *proto = NULL;
	if (hContact != NULL)
	{
		name = (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
		proto = GetContactProto(hContact);
	}

	mir_snprintf(text, SIZEOF(text) - 10, " ***** AVS [%08d] [ID:%04x]: [%08d - %s - %s] ",
		GetTickCount(), GetCurrentThreadId(), hContact, proto == NULL ? "" : proto, name == NULL ? "" : name);
	len = strlen(text);

	va_start(va, fmt);
	mir_vsnprintf(&text[len], SIZEOF(text) - len, fmt, va);
	va_end(va);

	OutputDebugStringA(text);
	OutputDebugStringA(" ***** \n");

	return 0;
}

#endif

void mir_sleep(int time) 
{
	if (!g_shutDown)
		WaitForSingleObject(hShutdownEvent, time);
}

/*
 * path utilities (make avatar paths relative to *PROFILE* directory, not miranda directory.
 * taken and modified from core services
 */

int AVS_pathIsAbsolute(const TCHAR *path)
{
	if (!path || !(lstrlen(path) > 2))
		return 0;
	if ((path[1]==':'&&path[2]=='\\')||(path[0]=='\\'&&path[1]=='\\')) return 1;
	return 0;
}

size_t AVS_pathToRelative(const TCHAR *pSrc, TCHAR *pOut)
{
	if (!pSrc || !*pSrc || _tcslen(pSrc) > MAX_PATH) return 0;
	if (!AVS_pathIsAbsolute( pSrc ))
		lstrcpyn(pOut, pSrc, MAX_PATH);
	else {
		TCHAR szTmp[MAX_PATH];
		mir_sntprintf(szTmp, SIZEOF(szTmp), _T("%s"), pSrc);
		_tcslwr(szTmp);
		if (_tcsstr(szTmp, g_szDataPath))
			lstrcpyn(pOut, pSrc + _tcslen(g_szDataPath) + 1, MAX_PATH);
		else
			lstrcpyn(pOut, pSrc, MAX_PATH);
	}
	return _tcslen(pOut);
}

size_t AVS_pathToAbsolute(const TCHAR *pSrc, TCHAR *pOut)
{
	if (!pSrc || !lstrlen(pSrc) || lstrlen(pSrc) > MAX_PATH)
		return 0;

	if (AVS_pathIsAbsolute(pSrc) || !_istalnum(pSrc[0]))
		lstrcpyn(pOut, pSrc, MAX_PATH);
	else
		mir_sntprintf(pOut, MAX_PATH, _T("%s\\%s"), g_szDataPath, pSrc, MAX_PATH);
	return lstrlen(pOut);
}

static void NotifyMetaAware(HANDLE hContact, struct CacheNode *node = NULL, AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)-1)
{
	if (g_shutDown)
		return;

	if (ace == (AVATARCACHEENTRY *)-1)
		ace = &node->ace;

	NotifyEventHooks(hEventChanged, (WPARAM)hContact, (LPARAM)ace);

	if (g_MetaAvail && (node->dwFlags & MC_ISSUBCONTACT) && db_get_b(NULL, g_szMetaName, "Enabled", 0)) {
		HANDLE hMasterContact = (HANDLE)db_get_dw(hContact, g_szMetaName, "Handle", 0);
		if (hMasterContact && (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hMasterContact, 0) == hContact &&
			!db_get_b(hMasterContact, "ContactPhoto", "Locked", 0))
			NotifyEventHooks(hEventChanged, (WPARAM)hMasterContact, (LPARAM)ace);
	}

	if (node->dwFlags & AVH_MUSTNOTIFY) {
		// Fire the event for avatar history
		node->dwFlags &= ~AVH_MUSTNOTIFY;
		if (node->ace.szFilename[0] != '\0') {
			CONTACTAVATARCHANGEDNOTIFICATION cacn = {0};
			cacn.cbSize = sizeof(CONTACTAVATARCHANGEDNOTIFICATION);
			cacn.hContact = hContact;
			cacn.format = node->pa_format;
			_tcsncpy(cacn.filename, node->ace.szFilename, MAX_PATH);
			cacn.filename[MAX_PATH - 1] = 0;

			// Get hash
			char *szProto = GetContactProto(hContact);
			if (szProto != NULL) {
				DBVARIANT dbv = {0};
				if ( !db_get_s(hContact, szProto, "AvatarHash", &dbv)) {
					if (dbv.type == DBVT_TCHAR)
						_tcsncpy_s(cacn.hash, SIZEOF(cacn.hash), dbv.ptszVal, _TRUNCATE);
					else if (dbv.type == DBVT_BLOB) {
						// Lets use base64 encode
						char *tab = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
						int i;
						for(i = 0; i < dbv.cpbVal / 3 && i*4+3 < sizeof(cacn.hash)-1; i++) {
							BYTE a = dbv.pbVal[i*3];
							BYTE b = i*3 + 1 < dbv.cpbVal ? dbv.pbVal[i*3 + 1] : 0;
							BYTE c = i*3 + 2 < dbv.cpbVal ? dbv.pbVal[i*3 + 2] : 0;

							cacn.hash[i*4] = tab[(a & 0xFC) >> 2];
							cacn.hash[i*4+1] = tab[((a & 0x3) << 4) + ((b & 0xF0) >> 4)];
							cacn.hash[i*4+2] = tab[((b & 0xF) << 2) + ((c & 0xC0) >> 6)];
							cacn.hash[i*4+3] = tab[c & 0x3F];
						}
						if (dbv.cpbVal % 3 != 0 && i*4+3 < sizeof(cacn.hash)-1) {
							BYTE a = dbv.pbVal[i*3];
							BYTE b = i*3 + 1 < dbv.cpbVal ? dbv.pbVal[i*3 + 1] : 0;

							cacn.hash[i*4] = tab[(a & 0xFC) >> 2];
							cacn.hash[i*4+1] = tab[((a & 0x3) << 4) + ((b & 0xF0) >> 4)];
							if (i + 1 < dbv.cpbVal)
								cacn.hash[i*4+2] = tab[((b & 0xF) << 4)];
							else
								cacn.hash[i*4+2] = '=';
							cacn.hash[i*4+3] = '=';
						}
					}
					db_free(&dbv);
				}
			}

			// Default value
			if (cacn.hash[0] == '\0')
				mir_sntprintf(cacn.hash, SIZEOF(cacn.hash), _T("AVS-HASH-%x"), GetFileHash(cacn.filename));

			NotifyEventHooks(hEventContactAvatarChanged, (WPARAM)cacn.hContact, (LPARAM)&cacn);
		}
		else NotifyEventHooks(hEventContactAvatarChanged, (WPARAM)hContact, NULL);
	}
}

static int g_maxBlock = 0, g_curBlock = 0;
static struct CacheNode **g_cacheBlocks = NULL;

/*
 * allocate a cache block and add it to the list of blocks
 * does not link the new block with the old block(s) - caller needs to do this
 */

static struct CacheNode *AllocCacheBlock()
{
	struct CacheNode *allocedBlock = NULL;

	allocedBlock = (struct CacheNode *)malloc(CACHE_BLOCKSIZE * sizeof(struct CacheNode));
	ZeroMemory((void *)allocedBlock, sizeof(struct CacheNode) * CACHE_BLOCKSIZE);

	for(int i = 0; i < CACHE_BLOCKSIZE - 1; i++)
		allocedBlock[i].pNextNode = &allocedBlock[i + 1];				// pre-link the alloced block

	if (g_Cache == NULL)													// first time only...
		g_Cache = allocedBlock;

	// add it to the list of blocks

	if (g_curBlock == g_maxBlock) {
		g_maxBlock += 10;
		g_cacheBlocks = (struct CacheNode **)realloc(g_cacheBlocks, g_maxBlock * sizeof(struct CacheNode *));
	}
	g_cacheBlocks[g_curBlock++] = allocedBlock;

	return(allocedBlock);
}

int SetAvatarAttribute(HANDLE hContact, DWORD attrib, int mode)
{
	struct CacheNode *сс = g_Cache;

	while(сс) {
		if (сс->ace.hContact == hContact) {
			DWORD dwFlags = сс->ace.dwFlags;

			сс->ace.dwFlags = mode ? (сс->ace.dwFlags | attrib) : (сс->ace.dwFlags & ~attrib);
			if (сс->ace.dwFlags != dwFlags)
				NotifyMetaAware(hContact, сс);
			break;
		}
		сс = сс->pNextNode;
	}
	return 0;
}

/*
 * convert the avatar image path to a relative one...
 * given: contact handle, path to image
 */
void MakePathRelative(HANDLE hContact, TCHAR *path)
{
	TCHAR szFinalPath[MAX_PATH];
	szFinalPath[0] = '\0';

	size_t result = AVS_pathToRelative(path, szFinalPath);
	if (result && lstrlen(szFinalPath) > 0) {
		db_set_ts(hContact, "ContactPhoto", "RFile", szFinalPath);
		if (!db_get_b(hContact, "ContactPhoto", "Locked", 0))
			db_set_ts(hContact, "ContactPhoto", "Backup", szFinalPath);
	}
}

/*
 * convert the avatar image path to a relative one...
 * given: contact handle
 */

static void MakePathRelative(HANDLE hContact)
{
	DBVARIANT dbv;
	if ( !db_get_ts(hContact, "ContactPhoto", "File", &dbv)) {
		MakePathRelative(hContact, dbv.ptszVal);
		db_free(&dbv);
	}
}

static void ResetTranspSettings(HANDLE hContact)
{
	db_unset(hContact, "ContactPhoto", "MakeTransparentBkg");
	db_unset(hContact, "ContactPhoto", "TranspBkgNumPoints");
	db_unset(hContact, "ContactPhoto", "TranspBkgColorDiff");
}

static TCHAR *getJGMailID(char *szProto)
{
	static TCHAR szJID[MAX_PATH+1]; szJID[0] = '\0';

	DBVARIANT dbva, dbvb;
	if ( db_get_ts(NULL, szProto, "LoginName", &dbva))
		return szJID;

	if ( db_get_ts(NULL, szProto, "LoginServer", &dbvb)) {
		db_free(&dbva);
		return szJID;
	}

	mir_sntprintf(szJID, SIZEOF(szJID), _T("%s@%s"), dbva.ptszVal, dbvb.ptszVal);
	db_free(&dbva);
	db_free(&dbvb);
	return szJID;
}

// create the avatar in cache
// returns 0 if not created (no avatar), iIndex otherwise, -2 if has to request avatar, -3 if avatar too big
int CreateAvatarInCache(HANDLE hContact, avatarCacheEntry *ace, char *szProto)
{
	DBVARIANT dbv = {0};
	char *szExt = NULL;
	TCHAR tszFilename[MAX_PATH];
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwFileSizeHigh = 0, dwFileSize = 0, sizeLimit = 0;

	tszFilename[0] = 0;

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

		if (db_get_b(hContact, "ContactPhoto", "Locked", 0)
			&& !db_get_ts(hContact, "ContactPhoto", "Backup", &dbv)) {
				AVS_pathToAbsolute(dbv.ptszVal, tszFilename);
				db_free(&dbv);
		}
		else if ( !db_get_ts(hContact, "ContactPhoto", "RFile", &dbv)) {
			AVS_pathToAbsolute(dbv.ptszVal, tszFilename);
			db_free(&dbv);
		}
		else if ( !db_get_ts(hContact, "ContactPhoto", "File", &dbv)) {
			AVS_pathToAbsolute(dbv.ptszVal, tszFilename);
			db_free(&dbv);
		}
		else return -2;
	}
	else {
		if (hContact == 0) {				// create a protocol picture in the proto picture cache
			if ( !db_get_ts(NULL, PPICT_MODULE, szProto, &dbv)) {
				AVS_pathToAbsolute(dbv.ptszVal, tszFilename);
				db_free(&dbv);
			}
			else {
				if (lstrcmpA(szProto, AVS_DEFAULT)) {
					if ( !db_get_ts(NULL, PPICT_MODULE, AVS_DEFAULT, &dbv)) {
						AVS_pathToAbsolute(dbv.ptszVal, tszFilename);
						db_free(&dbv);
					}

					if (!strstr(szProto, "Global avatar for")) {
						PROTOACCOUNT* pdescr = (PROTOACCOUNT*)CallService(MS_PROTO_GETACCOUNT, 0, (LPARAM)szProto);
						if (pdescr == NULL)
							return -1;
						char key[MAX_PATH];
						mir_snprintf(key, SIZEOF(key), "Global avatar for %s accounts", pdescr->szProtoName);
						if ( !db_get_ts(NULL, PPICT_MODULE, key, &dbv)) {
							AVS_pathToAbsolute(dbv.ptszVal, tszFilename);
							db_free(&dbv);
						}
					}
				}
			}
		}
		else if (hContact == (HANDLE)-1) {	// create own picture - note, own avatars are not on demand, they are loaded once at
			// startup and everytime they are changed.
			if (szProto[0] == '\0') {
				// Global avatar
				if ( db_get_ts(NULL, AVS_MODULE, "GlobalUserAvatarFile", &dbv))
					return -10;

				AVS_pathToAbsolute(dbv.ptszVal, tszFilename);
				db_free(&dbv);
			}
			else if ( ProtoServiceExists(szProto, PS_GETMYAVATART)) {
				if (CallProtoService(szProto, PS_GETMYAVATART, (WPARAM)tszFilename, (LPARAM)MAX_PATH))
					tszFilename[0] = '\0';
			}
			else if ( ProtoServiceExists(szProto, PS_GETMYAVATAR)) {
				char szFileName[ MAX_PATH ];
				if (CallProtoService(szProto, PS_GETMYAVATAR, (WPARAM)szFileName, (LPARAM)MAX_PATH))
					tszFilename[0] = '\0';
				else
					MultiByteToWideChar( CP_ACP, 0, szFileName, -1, tszFilename, SIZEOF(tszFilename));
			}
			else if ( !db_get_ts(NULL, szProto, "AvatarFile", &dbv)) {
				AVS_pathToAbsolute(dbv.ptszVal, tszFilename);
				db_free(&dbv);
			}
			else return -1;
		}
	}

	if ( lstrlen(tszFilename) < 4)
		return -1;

	_tcsncpy_s(tszFilename, SIZEOF(tszFilename), VARST(tszFilename), _TRUNCATE);
	if ((hFile = CreateFile(tszFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		return -2;

	CloseHandle(hFile);
	WPARAM isTransparentImage = 0;

	ace->hbmPic = (HBITMAP) BmpFilterLoadBitmap32((WPARAM)&isTransparentImage, (LPARAM)tszFilename);
	ace->dwFlags = 0;
	ace->bmHeight = 0;
	ace->bmWidth = 0;
	ace->lpDIBSection = NULL;
	_tcsncpy(ace->szFilename, tszFilename, MAX_PATH);
	ace->szFilename[MAX_PATH - 1] = 0;
	if (ace->hbmPic != 0) {
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
		if (hContact != 0 && hContact != (HANDLE)-1) {
			// Have to reset settings? -> do it if image changed
			DWORD imgHash = GetImgHash(ace->hbmPic);
			if (imgHash != db_get_dw(hContact, "ContactPhoto", "ImageHash", 0)) {
				ResetTranspSettings(hContact);
				db_set_dw(hContact, "ContactPhoto", "ImageHash", imgHash);
			}

			// Make transparent?
			if (!noTransparency && !isTransparentImage
				&& db_get_b(hContact, "ContactPhoto", "MakeTransparentBkg",
				db_get_b(0, AVS_MODULE, "MakeTransparentBkg", 0)))
			{
				if (MakeTransparentBkg(hContact, &ace->hbmPic)) {
					ace->dwFlags |= AVS_CUSTOMTRANSPBKG | AVS_HASTRANSPARENCY;
					GetObject(ace->hbmPic, sizeof(bminfo), &bminfo);
					isTransparentImage = TRUE;
				}
			}
		}
		else if (hContact == (HANDLE)-1) { // My avatars
			if (!noTransparency && !isTransparentImage
				&& db_get_b(0, AVS_MODULE, "MakeTransparentBkg", 0)
				&& db_get_b(0, AVS_MODULE, "MakeMyAvatarsTransparent", 0))
			{
				if (MakeTransparentBkg(0, &ace->hbmPic)) {
					ace->dwFlags |= AVS_CUSTOMTRANSPBKG | AVS_HASTRANSPARENCY;
					GetObject(ace->hbmPic, sizeof(bminfo), &bminfo);
					isTransparentImage = TRUE;
				}
			}
		}

		if (db_get_b(0, AVS_MODULE, "MakeGrayscale", 0))
			ace->hbmPic = MakeGrayscale(hContact, ace->hbmPic);

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
			else if (hContact == (HANDLE)-1)
				pAce->dwFlags |= AVS_OWNAVATAR;
		}

		return 1;
	}
	return -1;
}

/*
 * link a new cache block with the already existing chain of blocks
 */

static struct CacheNode *AddToList(struct CacheNode *node)
{
	struct CacheNode *pCurrent = g_Cache;

	while(pCurrent->pNextNode != 0)
		pCurrent = pCurrent->pNextNode;

	pCurrent->pNextNode = node;
	return pCurrent;
}

struct CacheNode *FindAvatarInCache(HANDLE hContact, BOOL add, BOOL findAny = FALSE)
{
	if (g_shutDown)
		return NULL;

	char *szProto = GetContactProto(hContact);
	if (szProto == NULL || !db_get_b(NULL, AVS_MODULE, szProto, 1))
		return NULL;

	mir_cslock lck(cachecs);

	struct CacheNode *сс = g_Cache, *foundNode = NULL;
	while(сс) {
		if (сс->ace.hContact == hContact) {
			сс->ace.t_lastAccess = time(NULL);
			foundNode = сс->loaded || findAny ? сс : NULL;
			return foundNode;
		}

		// found an empty and usable node
		if (foundNode == NULL && сс->ace.hContact == 0)
			foundNode = сс;

		сс = сс->pNextNode;
	}

	// not found
	if (!add)
		return NULL;

	if (foundNode == NULL) {        // no free entry found, create a new and append it to the list
		mir_cslock all(alloccs);     // protect memory block allocation
		struct CacheNode *newNode = AllocCacheBlock();
		AddToList(newNode);
		foundNode = newNode;
	}

	foundNode->ace.hContact = hContact;
	if (g_MetaAvail)
		foundNode->dwFlags |= (db_get_b(hContact, g_szMetaName, "IsSubcontact", 0) ? MC_ISSUBCONTACT : 0);
	foundNode->loaded = FALSE;
	foundNode->mustLoad = 1;        // pic loader will watch this and load images
	SetEvent(hLoaderEvent);         // wake him up
	return NULL;
}

#define POLYNOMIAL (0x488781ED) /* This is the CRC Poly */
#define TOPBIT (1 << (WIDTH - 1)) /* MSB */
#define WIDTH 32

static int GetFileHash(TCHAR* filename)
{
	HANDLE hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	int remainder = 0;
	char data[1024];
	DWORD dwRead;
	do
	{
		// Read file chunk
		dwRead = 0;
		ReadFile(hFile, data, 1024, &dwRead, NULL);

		/* loop through each byte of data */
		for (int byte = 0; byte < (int) dwRead; ++byte) {
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
		while(dwRead == 1024);

	CloseHandle(hFile);

	return remainder;
}

static int ProtocolAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA *) lParam;

	if (ack != NULL && ack->type == ACKTYPE_AVATAR && ack->hContact != 0 && (!g_MetaAvail || strcmp(ack->szModule, g_szMetaName))) {
		if (ack->result == ACKRESULT_SUCCESS) {
			if (ack->hProcess == NULL)
				ProcessAvatarInfo(ack->hContact, GAIR_NOAVATAR, NULL, ack->szModule);
			else
				ProcessAvatarInfo(ack->hContact, GAIR_SUCCESS, (PROTO_AVATAR_INFORMATIONT *) ack->hProcess, ack->szModule);
		}
		else if (ack->result == ACKRESULT_FAILED) {
			ProcessAvatarInfo(ack->hContact, GAIR_FAILED, (PROTO_AVATAR_INFORMATIONT *) ack->hProcess, ack->szModule);
		}
		else if (ack->result == ACKRESULT_STATUS) {
			char *szProto = GetContactProto(ack->hContact);
			if (szProto == NULL || Proto_NeedDelaysForAvatars(szProto)) {
				// Queue
				db_set_b(ack->hContact, "ContactPhoto", "NeedUpdate", 1);
				QueueAdd(ack->hContact);
			}
			else {
				// Fetch it now
				FetchAvatarFor(ack->hContact, szProto);
			}
		}
	}
	return 0;
}

INT_PTR ProtectAvatar(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	BYTE was_locked = db_get_b(hContact, "ContactPhoto", "Locked", 0);

	if (fei == NULL || was_locked == (BYTE)lParam)      // no need for redundant lockings...
		return 0;

	if (hContact) {
		if (!was_locked)
			MakePathRelative(hContact);
		db_set_b(hContact, "ContactPhoto", "Locked", lParam ? 1 : 0);
		if (lParam == 0)
			MakePathRelative(hContact);
		ChangeAvatar(hContact, TRUE);
	}
	return 0;
}

/*
 * for subclassing the open file dialog...
 */
struct OpenFileSubclassData {
	BYTE *locking_request;
	BYTE setView;
};

static BOOL CALLBACK OpenFileSubclass(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OpenFileSubclassData *data= (OpenFileSubclassData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch(msg) {
	case WM_INITDIALOG:
		{
			OPENFILENAME *ofn = (OPENFILENAME *)lParam;

			data = (OpenFileSubclassData *) malloc(sizeof(OpenFileSubclassData));
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
			data->locking_request = (BYTE *)ofn->lCustData;
			data->setView = TRUE;

			TranslateDialogDefault(hwnd);
			CheckDlgButton(hwnd, IDC_PROTECTAVATAR, *(data->locking_request));
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_PROTECTAVATAR)
			*(data->locking_request) = IsDlgButtonChecked(hwnd, IDC_PROTECTAVATAR) ? TRUE : FALSE;
		break;

	case WM_NOTIFY:
		if (data->setView) {
			HWND hwndParent = GetParent(hwnd);
			HWND hwndLv = FindWindowEx(hwndParent, NULL, _T("SHELLDLL_DefView"), NULL) ;
			if (hwndLv != NULL) {
				SendMessage(hwndLv, WM_COMMAND, SHVIEW_THUMBNAIL, 0);
				data->setView = FALSE;
			}
		}
		break;

	case WM_NCDESTROY:
		free((OpenFileSubclassData *)data);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)0);
		break;
	}

	return FALSE;
}

/*
 * set an avatar (service function)
 * if lParam == NULL, a open file dialog will be opened, otherwise, lParam is taken as a FULL
 * image filename (will be checked for existance, though)
 */

INT_PTR avSetAvatar(HANDLE hContact, TCHAR* tszPath)
{
	BYTE is_locked = 0;
	TCHAR FileName[MAX_PATH], szBackupName[MAX_PATH];
	TCHAR *szFinalName = NULL;
	HANDLE hFile = 0;
	BYTE locking_request;

	if (hContact == NULL || fei == NULL)
		return 0;

	is_locked = db_get_b(hContact, "ContactPhoto", "Locked", 0);

	if ( tszPath == NULL ) {
		OPENFILENAME ofn = {0};
		TCHAR filter[256];

		filter[0] = '\0';
		CallService(MS_UTILS_GETBITMAPFILTERSTRINGST, SIZEOF(filter), ( LPARAM )filter);

		if (IsWinVer2000Plus())
			ofn.lStructSize = sizeof(ofn);
		else
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = 0;
		ofn.lpstrFile = FileName;
		ofn.lpstrFilter = filter;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
		ofn.lpstrInitialDir = _T(".");
		*FileName = '\0';
		ofn.lpstrDefExt = _T("");
		ofn.hInstance = g_hInst;
		ofn.lpTemplateName = MAKEINTRESOURCE(IDD_OPENSUBCLASS);
		ofn.lpfnHook = (LPOFNHOOKPROC)OpenFileSubclass;
		locking_request = is_locked;
		ofn.lCustData = (LPARAM)&locking_request;
		if (GetOpenFileName(&ofn)) {
			szFinalName = FileName;
			is_locked = locking_request ? 1 : is_locked;
		}
		else
			return 0;
	}
	else
		szFinalName = tszPath;

	/*
	* filename is now set, check it and perform all needed action
	*/

	if ((hFile = CreateFile(szFinalName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		return 0;

	// file exists...

	CloseHandle(hFile);

	AVS_pathToRelative(szFinalName, szBackupName);
	db_set_ts(hContact, "ContactPhoto", "Backup", szBackupName);

	db_set_b(hContact, "ContactPhoto", "Locked", is_locked);
	db_set_ts(hContact, "ContactPhoto", "File", szFinalName);
	MakePathRelative(hContact, szFinalName);
	// Fix cache
	ChangeAvatar(hContact, TRUE);

	return 0;
}

INT_PTR SetAvatar(WPARAM wParam, LPARAM lParam)
{
	return avSetAvatar(( HANDLE )wParam, _A2T(( const char* )lParam ));
}

INT_PTR SetAvatarW(WPARAM wParam, LPARAM lParam)
{
	return avSetAvatar(( HANDLE )wParam, ( TCHAR* )lParam );
}

/*
 * see if is possible to set the avatar for the expecified protocol
 */
static INT_PTR CanSetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	char *protocol = (char *) wParam;
	if (protocol == NULL || fei == NULL)
		return 0;

	return ProtoServiceExists(protocol, PS_SETMYAVATAR);
}

struct SetMyAvatarHookData {
	char *protocol;
	BOOL square;
	BOOL grow;

	BOOL thumbnail;
};

/*
 * Callback to set thumbnaill view to open dialog
 */
static UINT_PTR CALLBACK SetMyAvatarHookProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			InterlockedExchange(&hwndSetMyAvatar, (LONG) hwnd);

			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lParam);
			OPENFILENAME *ofn = (OPENFILENAME *)lParam;
			SetMyAvatarHookData *data = (SetMyAvatarHookData *) ofn->lCustData;
			data->thumbnail = TRUE;

			SetWindowText(GetDlgItem(hwnd, IDC_MAKE_SQUARE), TranslateT("Make the avatar square"));
			SetWindowText(GetDlgItem(hwnd, IDC_GROW), TranslateT("Grow avatar to fit max allowed protocol size"));

			CheckDlgButton(hwnd, IDC_MAKE_SQUARE, data->square ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd, IDC_GROW, data->grow ? BST_CHECKED : BST_UNCHECKED);

			if (data->protocol != NULL && (Proto_AvatarImageProportion(data->protocol) & PIP_SQUARE))
				EnableWindow(GetDlgItem(hwnd, IDC_MAKE_SQUARE), FALSE);
		}
		break;

	case WM_NOTIFY:
		{
			OPENFILENAME *ofn = (OPENFILENAME *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			SetMyAvatarHookData *data = (SetMyAvatarHookData *) ofn->lCustData;
			if (data->thumbnail)
			{
				HWND hwndParent = GetParent(hwnd);
				HWND hwndLv = FindWindowEx(hwndParent, NULL, _T("SHELLDLL_DefView"), NULL) ;
				if (hwndLv != NULL)
				{
					SendMessage(hwndLv, WM_COMMAND, SHVIEW_THUMBNAIL, 0);
					data->thumbnail = FALSE;
				}
			}
			break;
		}
	case WM_DESTROY:
		{
			OPENFILENAME *ofn = (OPENFILENAME *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			SetMyAvatarHookData *data = (SetMyAvatarHookData *) ofn->lCustData;
			data->square = IsDlgButtonChecked(hwnd, IDC_MAKE_SQUARE);
			data->grow = IsDlgButtonChecked(hwnd, IDC_GROW);

			InterlockedExchange(&hwndSetMyAvatar, 0);
			break;
		}
	}

	return 0;
}

const TCHAR *GetFormatExtension(int format)
{
	if (format == PA_FORMAT_PNG)
		return _T(".png");
	if (format == PA_FORMAT_JPEG)
		return _T(".jpg");
	if (format == PA_FORMAT_ICON)
		return _T(".ico");
	if (format == PA_FORMAT_BMP)
		return _T(".bmp");
	if (format == PA_FORMAT_GIF)
		return _T(".gif");
	if (format == PA_FORMAT_SWF)
		return _T(".swf");
	if (format == PA_FORMAT_XML)
		return _T(".xml");

	return NULL;
}

int GetImageFormat(TCHAR *filename)
{
	size_t len = lstrlen(filename);

	if (len < 5)
		return PA_FORMAT_UNKNOWN;

	if (_tcsicmp(_T(".png"), &filename[len-4]) == 0)
		return PA_FORMAT_PNG;

	if (_tcsicmp(_T(".jpg"), &filename[len-4]) == 0 || _tcsicmp(_T(".jpeg"), &filename[len-4]) == 0)
		return PA_FORMAT_JPEG;

	if (_tcsicmp(_T(".ico"), &filename[len-4]) == 0)
		return PA_FORMAT_ICON;

	if (_tcsicmp(_T(".bmp"), &filename[len-4]) == 0 || _tcsicmp(_T(".rle"), &filename[len-4]) == 0)
		return PA_FORMAT_BMP;

	if (_tcsicmp(_T(".gif"), &filename[len-4]) == 0)
		return PA_FORMAT_GIF;

	if (_tcsicmp(_T(".swf"), &filename[len-4]) == 0)
		return PA_FORMAT_SWF;

	if (_tcsicmp(_T(".xml"), &filename[len-4]) == 0)
		return PA_FORMAT_XML;

	return PA_FORMAT_UNKNOWN;
}

static void FilterGetStrings(TCHAR *filter, int bytesLeft, BOOL xml, BOOL swf)
{
	TCHAR *pfilter;
	int wParam = bytesLeft;

	lstrcpyn(filter, TranslateT("All Files"), bytesLeft); bytesLeft-=lstrlen(filter);
	_tcsncat(filter, _T(" (*.bmp;*.jpg;*.gif;*.png"), bytesLeft);
	if (swf) _tcscat(filter, _T(";*.swf"));
	if (xml) _tcscat(filter, _T(";*.xml"));
	_tcscat(filter, _T(")"));
	pfilter=filter+lstrlen(filter)+1; bytesLeft=wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*.BMP;*.RLE;*.JPG;*.JPEG;*.GIF;*.PNG"), bytesLeft);
	if (swf) _tcscat(pfilter, _T(";*.SWF"));
	if (xml) _tcscat(pfilter, _T(";*.XML"));
	pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);

	lstrcpyn(pfilter, TranslateT("Windows Bitmaps"), bytesLeft); bytesLeft-=lstrlen(pfilter);
	_tcsncat(pfilter, _T(" (*.bmp;*.rle)"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*.BMP;*.RLE"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);

	lstrcpyn(pfilter,TranslateT("JPEG Bitmaps"),bytesLeft); bytesLeft-=lstrlen(pfilter);
	_tcsncat(pfilter, _T(" (*.jpg;*.jpeg)"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*.JPG;*.JPEG"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);

	lstrcpyn(pfilter,TranslateT("GIF Bitmaps"),bytesLeft); bytesLeft-=lstrlen(pfilter);
	_tcsncat(pfilter, _T(" (*.gif)"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*.GIF"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);

	lstrcpyn(pfilter,TranslateT("PNG Bitmaps"), bytesLeft); bytesLeft-=lstrlen(pfilter);
	_tcsncat(pfilter, _T(" (*.png)"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);
	lstrcpyn(pfilter, _T("*.PNG"), bytesLeft);
	pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);

	if (swf)
	{
		lstrcpyn(pfilter,TranslateT("Flash Animations"), bytesLeft); bytesLeft-=lstrlen(pfilter);
		_tcsncat(pfilter, _T(" (*.swf)"), bytesLeft);
		pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);
		lstrcpyn(pfilter, _T("*.SWF"), bytesLeft);
		pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);
	}

	if (xml)
	{
		lstrcpyn(pfilter, TranslateT("XML Files"), bytesLeft); bytesLeft-=lstrlen(pfilter);
		_tcsncat(pfilter, _T(" (*.xml)"), bytesLeft);
		pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);
		lstrcpyn(pfilter, _T("*.XML"), bytesLeft);
		pfilter+=lstrlen(pfilter)+1; bytesLeft=wParam-(pfilter-filter);
	}

	if (bytesLeft) *pfilter='\0';
}

static void DeleteGlobalUserAvatar()
{
	DBVARIANT dbv = {0};
	if (db_get_ts(NULL, AVS_MODULE, "GlobalUserAvatarFile", &dbv))
		return;

	TCHAR szFilename[MAX_PATH];
	AVS_pathToAbsolute(dbv.ptszVal, szFilename);
	db_free(&dbv);

	DeleteFile(szFilename);
	db_unset(NULL, AVS_MODULE, "GlobalUserAvatarFile");
}

static void SetIgnoreNotify(char *protocol, BOOL ignore)
{
	for(int i = 0; i < g_MyAvatars.getCount(); i++) {
		if (protocol == NULL || !lstrcmpA(g_MyAvatars[i].szProtoname, protocol)) {
			if (ignore)
				g_MyAvatars[i].dwFlags |= AVS_IGNORENOTIFY;
			else
				g_MyAvatars[i].dwFlags &= ~AVS_IGNORENOTIFY;
		}
	}
}

static int InternalRemoveMyAvatar(char *protocol)
{
	SetIgnoreNotify(protocol, TRUE);

	// Remove avatar
	int ret = 0;
	if (protocol != NULL)
	{
		if ( ProtoServiceExists(protocol, PS_SETMYAVATAR))
			ret = SaveAvatar(protocol, NULL);
		else
			ret = -3;

		if (ret == 0)
		{
			// Has global avatar?
			DBVARIANT dbv = {0};
			if ( !db_get_ts(NULL, AVS_MODULE, "GlobalUserAvatarFile", &dbv)) {
				db_free(&dbv);
				db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1);
				DeleteGlobalUserAvatar();
			}
		}
	}
	else
	{
		PROTOACCOUNT **accs;
		int i,count;

		ProtoEnumAccounts( &count, &accs );
		for (i = 0; i < count; i++)
		{
			if ( !ProtoServiceExists( accs[i]->szModuleName, PS_SETMYAVATAR))
				continue;

			if (!Proto_IsAvatarsEnabled( accs[i]->szModuleName ))
				continue;

			// Found a protocol
			int retTmp = SaveAvatar( accs[i]->szModuleName, NULL);
			if (retTmp != 0)
				ret = retTmp;
		}

		DeleteGlobalUserAvatar();

		if (ret)
			db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1);
		else
			db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 0);
	}

	SetIgnoreNotify(protocol, FALSE);

	ReportMyAvatarChanged((WPARAM)(( protocol == NULL ) ? "" : protocol ), 0);
	return ret;
}

static int InternalSetMyAvatar(char *protocol, TCHAR *szFinalName, SetMyAvatarHookData &data, BOOL allAcceptXML, BOOL allAcceptSWF)
{
	HANDLE hFile = 0;

	int format = GetImageFormat(szFinalName);
	if (format == PA_FORMAT_UNKNOWN || (hFile = CreateFile(szFinalName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
		return -3;

	CloseHandle(hFile);

	// file exists...

	HBITMAP hBmp = NULL;

	if (format == PA_FORMAT_SWF)
	{
		if (!allAcceptSWF)
			return -4;
	}
	else if (format == PA_FORMAT_XML)
	{
		if (!allAcceptXML)
			return -4;
	}
	else
	{
		// Try to open if is not a flash or XML
		hBmp = (HBITMAP) CallService(MS_IMG_LOAD, (WPARAM) szFinalName, IMGL_TCHAR);
		if (hBmp == NULL)
			return -4;
	}

	SetIgnoreNotify(protocol, TRUE);

	int ret = 0;
	if (protocol != NULL)
	{
		ret = SetProtoMyAvatar(protocol, hBmp, szFinalName, format, data.square, data.grow);

		if (ret == 0)
		{
			DeleteGlobalUserAvatar();
			db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1);
		}
	}
	else
	{
		PROTOACCOUNT **accs;
		int i,count;

		ProtoEnumAccounts( &count, &accs );
		for (i = 0; i < count; i++)
		{
			if ( !ProtoServiceExists( accs[i]->szModuleName, PS_SETMYAVATAR))
				continue;

			if ( !Proto_IsAvatarsEnabled( accs[i]->szModuleName ))
				continue;

			int retTmp = SetProtoMyAvatar( accs[i]->szModuleName, hBmp, szFinalName, format, data.square, data.grow);
			if (retTmp != 0)
				ret = retTmp;
		}

		DeleteGlobalUserAvatar();

		if (ret)
		{
			db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1);
		}
		else
		{
			// Copy avatar file to store as global one
			TCHAR globalFile[1024];
			BOOL saved = TRUE;
			if (FoldersGetCustomPathT(hGlobalAvatarFolder, globalFile, SIZEOF(globalFile), _T("")))
			{
				mir_sntprintf(globalFile, SIZEOF(globalFile), _T("%s\\%s"), g_szDataPath, _T("GlobalAvatar"));
				CreateDirectory(globalFile, NULL);
			}

			TCHAR *ext = _tcsrchr(szFinalName, _T('.')); // Can't be NULL here
			if (format == PA_FORMAT_XML || format == PA_FORMAT_SWF)
			{
				mir_sntprintf(globalFile, SIZEOF(globalFile), _T("%s\\my_global_avatar%s"), globalFile, ext);
				CopyFile(szFinalName, globalFile, FALSE);
			}
			else
			{
				// Resize (to avoid too big avatars)
				ResizeBitmap rb = {0};
				rb.size = sizeof(ResizeBitmap);
				rb.hBmp = hBmp;
				rb.max_height = 300;
				rb.max_width = 300;
				rb.fit = (data.grow ? 0 : RESIZEBITMAP_FLAG_DONT_GROW)
						| (data.square ? RESIZEBITMAP_MAKE_SQUARE : RESIZEBITMAP_KEEP_PROPORTIONS);

				HBITMAP hBmpTmp = (HBITMAP) BmpFilterResizeBitmap((WPARAM)&rb, 0);

				// Check if need to resize
				if (hBmpTmp == hBmp || hBmpTmp == NULL)
				{
					// Use original image
					mir_sntprintf(globalFile, SIZEOF(globalFile), _T("%s\\my_global_avatar%s"), globalFile, ext);
					CopyFile(szFinalName, globalFile, FALSE);
				}
				else
				{
					// Save as PNG
					mir_sntprintf(globalFile, SIZEOF(globalFile), _T("%s\\my_global_avatar.png"), globalFile);
					if (BmpFilterSaveBitmap((WPARAM) hBmpTmp, (LPARAM) globalFile))
						saved = FALSE;

					DeleteObject(hBmpTmp);
				}
			}

			if (saved)
			{
				TCHAR relFile[1024];
				if (AVS_pathToRelative(globalFile, relFile))
					db_set_ts(NULL, AVS_MODULE, "GlobalUserAvatarFile", relFile);
				else
					db_set_ts(NULL, AVS_MODULE, "GlobalUserAvatarFile", globalFile);

				db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 0);
			}
			else
			{
				db_set_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1);
			}
		}
	}

	DeleteObject(hBmp);

	SetIgnoreNotify(protocol, FALSE);

	ReportMyAvatarChanged((WPARAM)(( protocol == NULL ) ? "" : protocol ), 0);
	return ret;
}

/*
 * set an avatar for a protocol (service function)
 * if lParam == NULL, a open file dialog will be opened, otherwise, lParam is taken as a FULL
 * image filename (will be checked for existance, though)
 */

INT_PTR avSetMyAvatar( char* protocol, TCHAR* tszPath )
{
	TCHAR FileName[MAX_PATH];
	TCHAR *szFinalName = NULL;
	BOOL allAcceptXML;
	BOOL allAcceptSWF;

	// Protocol allow seting of avatar?
	if (protocol != NULL && !CanSetMyAvatar((WPARAM) protocol, 0))
		return -1;

	if (tszPath == NULL && hwndSetMyAvatar != 0)
	{
		SetForegroundWindow((HWND) hwndSetMyAvatar);
		SetFocus((HWND) hwndSetMyAvatar);
		ShowWindow((HWND) hwndSetMyAvatar, SW_SHOW);
		return -2;
	}

	SetMyAvatarHookData data = { 0 };

	// Check for XML and SWF
	if (protocol == NULL)
	{
		allAcceptXML = TRUE;
		allAcceptSWF = TRUE;

		PROTOACCOUNT **accs;
		int i,count;

		ProtoEnumAccounts( &count, &accs );
		for (i = 0; i < count; i++)
		{
			if ( !ProtoServiceExists( accs[i]->szModuleName, PS_SETMYAVATAR))
				continue;

			if ( !Proto_IsAvatarsEnabled( accs[i]->szModuleName ))
				continue;

			allAcceptXML = allAcceptXML && Proto_IsAvatarFormatSupported( accs[i]->szModuleName, PA_FORMAT_XML);
			allAcceptSWF = allAcceptSWF && Proto_IsAvatarFormatSupported( accs[i]->szModuleName, PA_FORMAT_SWF);
		}

		data.square = db_get_b(0, AVS_MODULE, "SetAllwaysMakeSquare", 0);
	}
	else
	{
		allAcceptXML = Proto_IsAvatarFormatSupported(protocol, PA_FORMAT_XML);
		allAcceptSWF = Proto_IsAvatarFormatSupported(protocol, PA_FORMAT_SWF);

		data.protocol = protocol;
		data.square = (Proto_AvatarImageProportion(protocol) & PIP_SQUARE)
						|| db_get_b(0, AVS_MODULE, "SetAllwaysMakeSquare", 0);
	}

	if (tszPath == NULL) {
		OPENFILENAME ofn = {0};
		TCHAR filter[512];
		TCHAR inipath[1024];

		data.protocol = protocol;

		filter[0] = '\0';
		FilterGetStrings(filter, SIZEOF(filter), allAcceptXML, allAcceptSWF);

		FoldersGetCustomPathT(hMyAvatarsFolder, inipath, SIZEOF(inipath), _T("."));

		if (IsWinVer2000Plus())
			ofn.lStructSize = sizeof(ofn);
		else
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
		ofn.hwndOwner = 0;
		ofn.lpstrFile = FileName;
		ofn.lpstrFilter = filter;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_ENABLETEMPLATE | OFN_EXPLORER | OFN_ENABLESIZING | OFN_ENABLEHOOK;
		ofn.lpstrInitialDir = inipath;
		ofn.lpTemplateName = MAKEINTRESOURCE(IDD_SET_OWN_SUBCLASS);
		ofn.lpfnHook = SetMyAvatarHookProc;
		ofn.lCustData = (LPARAM) &data;

		*FileName = '\0';
		ofn.lpstrDefExt = _T("");
		ofn.hInstance = g_hInst;

		TCHAR title[256];
		if (protocol == NULL)
			mir_sntprintf(title, SIZEOF(title), TranslateT("Set My Avatar"));
		else
		{
			TCHAR* prototmp = mir_a2t(protocol);
			mir_sntprintf(title, SIZEOF(title), TranslateT("Set My Avatar for %s"), prototmp);
			mir_free(prototmp);
		}
		ofn.lpstrTitle = title;

		if (GetOpenFileName(&ofn))
			szFinalName = FileName;
		else
			return 1;
	}
	else
		szFinalName = (TCHAR *)tszPath;

	/*
	* filename is now set, check it and perform all needed action
	*/

	if (szFinalName[0] == '\0')
		return InternalRemoveMyAvatar(protocol);

	return InternalSetMyAvatar(protocol, szFinalName, data, allAcceptXML, allAcceptSWF);
}

static INT_PTR SetMyAvatar( WPARAM wParam, LPARAM lParam )
{
	return avSetMyAvatar(( char* )wParam, _A2T(( const char* )lParam ));
}

static INT_PTR SetMyAvatarW( WPARAM wParam, LPARAM lParam )
{
	return avSetMyAvatar(( char* )wParam, ( TCHAR* )lParam );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

static DWORD GetFileSize(TCHAR *szFilename)
{
	HANDLE hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	DWORD low = GetFileSize(hFile, NULL);

	CloseHandle(hFile);

	if (low == INVALID_FILE_SIZE)
		return 0;

	return low;
}

struct SaveProtocolData {
	DWORD max_size;
	TCHAR image_file_name[MAX_PATH];
	BOOL saved;
	BOOL need_smaller_size;
	int width;
	int height;
	TCHAR temp_file[MAX_PATH];
	HBITMAP hBmpProto;
};

static void SaveImage(SaveProtocolData &d, char *protocol, int format)
{
	if (Proto_IsAvatarFormatSupported(protocol, format))
	{
		mir_sntprintf(d.image_file_name, SIZEOF(d.image_file_name), _T("%s%s"), d.temp_file, GetFormatExtension(format));
		if (!BmpFilterSaveBitmapT(d.hBmpProto, d.image_file_name, format == PA_FORMAT_JPEG ? JPEG_QUALITYSUPERB : 0))
		{
			if (d.max_size != 0 && GetFileSize(d.image_file_name) > d.max_size)
			{
				DeleteFile(d.image_file_name);

				if (format == PA_FORMAT_JPEG)
				{
					// Try with lower quality
					if (!BmpFilterSaveBitmapT(d.hBmpProto, d.image_file_name, JPEG_QUALITYGOOD))
					{
						if (GetFileSize(d.image_file_name) > d.max_size)
						{
							DeleteFile(d.image_file_name);
							d.need_smaller_size = TRUE;
						}
						else
							d.saved = TRUE;
					}
				}
				else
					d.need_smaller_size = TRUE;
			}
			else
				d.saved = TRUE;
		}
	}
}

static int SetProtoMyAvatar(char *protocol, HBITMAP hBmp, TCHAR *originalFilename, int originalFormat, BOOL square, BOOL grow)
{
	if (!ProtoServiceExists(protocol, PS_SETMYAVATAR))
		return -1;

	// If is swf or xml, just set it

	if (originalFormat == PA_FORMAT_SWF)
	{
		if (!Proto_IsAvatarFormatSupported(protocol, PA_FORMAT_SWF))
			return -1;

		return SaveAvatar(protocol, originalFilename);
	}

	if (originalFormat == PA_FORMAT_XML)
	{
		if (!Proto_IsAvatarFormatSupported(protocol, PA_FORMAT_XML))
			return -1;

		return SaveAvatar(protocol, originalFilename);
	}

	// Get protocol info
	SaveProtocolData d = {0};

	d.max_size = (DWORD) Proto_GetAvatarMaxFileSize(protocol);

	Proto_GetAvatarMaxSize(protocol, &d.width, &d.height);
	int orig_width = d.width;
	int orig_height = d.height;

	if (Proto_AvatarImageProportion(protocol) & PIP_SQUARE)
		square = TRUE;

	// Try to save until a valid image is found or we give up
	int num_tries = 0;
	do {
		// Lets do it
		ResizeBitmap rb;
		rb.size = sizeof(ResizeBitmap);
		rb.hBmp = hBmp;
		rb.max_height = d.height;
		rb.max_width = d.width;
		rb.fit = (grow ? 0 : RESIZEBITMAP_FLAG_DONT_GROW)
				| (square ? RESIZEBITMAP_MAKE_SQUARE : RESIZEBITMAP_KEEP_PROPORTIONS);

		d.hBmpProto = (HBITMAP) BmpFilterResizeBitmap((WPARAM)&rb, 0);

		if (d.hBmpProto == NULL)
		{
			if (d.temp_file[0] != '\0')
				DeleteFile(d.temp_file);
			return -1;
		}

		// Check if can use original image
		if (d.hBmpProto == hBmp
			&& Proto_IsAvatarFormatSupported(protocol, originalFormat)
			&& (d.max_size == 0 || GetFileSize(originalFilename) < d.max_size))
		{
			if (d.temp_file[0] != '\0')
				DeleteFile(d.temp_file);

			// Use original image
			return SaveAvatar(protocol, originalFilename);
		}

		// Create a temporary file (if was not created already)
		if (d.temp_file[0] == '\0')
		{
			d.temp_file[0] = '\0';
			if (GetTempPath(MAX_PATH, d.temp_file) == 0
				|| GetTempFileName(d.temp_file, _T("mir_av_"), 0, d.temp_file) == 0)
			{
				DeleteObject(d.hBmpProto);
				return -1;
			}
		}

		// Which format?

		// First try to use original format
		if (originalFormat != PA_FORMAT_BMP)
			SaveImage(d, protocol, originalFormat);

		if (!d.saved && originalFormat != PA_FORMAT_PNG)
			SaveImage(d, protocol, PA_FORMAT_PNG);

		if (!d.saved && originalFormat != PA_FORMAT_JPEG)
			SaveImage(d, protocol, PA_FORMAT_JPEG);

		if (!d.saved && originalFormat != PA_FORMAT_GIF)
			SaveImage(d, protocol, PA_FORMAT_GIF);

		if (!d.saved)
			SaveImage(d, protocol, PA_FORMAT_BMP);

		num_tries++;
		if (!d.saved && d.need_smaller_size && num_tries < 4)
		{
			// Cleanup
			if (d.hBmpProto != hBmp)
				DeleteObject(d.hBmpProto);

			// use a smaller size
			d.width = orig_width * (4 - num_tries) / 4;
			d.height = orig_height * (4 - num_tries) / 4;
		}

	} while(!d.saved && d.need_smaller_size && num_tries < 4);

	int ret;

	if (d.saved)
	{
		// Call proto service
		ret = SaveAvatar(protocol, d.image_file_name);
		DeleteFile(d.image_file_name);
	}
	else
	{
		ret = -1;
	}

	if (d.temp_file[0] != '\0')
		DeleteFile(d.temp_file);

	if (d.hBmpProto != hBmp)
		DeleteObject(d.hBmpProto);

	return ret;
}

static INT_PTR ContactOptions(WPARAM wParam, LPARAM lParam)
{
	if (wParam)
		CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_AVATAROPTIONS), 0, DlgProcAvatarOptions, (LPARAM)wParam);
	return 0;
}

INT_PTR GetMyAvatar(WPARAM wParam, LPARAM lParam)
{
	if (wParam || g_shutDown || fei == NULL)
		return 0;

	char *szProto = (char *)lParam;
	if (lParam == 0 || IsBadReadPtr(szProto, 4))
		return 0;

	for(int i = 0; i < g_MyAvatars.getCount(); i++)
		if (!lstrcmpA(szProto, g_MyAvatars[i].szProtoname) && g_MyAvatars[i].hbmPic != 0)
			return (INT_PTR)&g_MyAvatars[i];

	return 0;
}

static protoPicCacheEntry *GetProtoDefaultAvatar(HANDLE hContact)
{
	char *szProto = GetContactProto(hContact);
	if (szProto) {
		for(int i = 0; i < g_ProtoPictures.getCount(); i++) {
			protoPicCacheEntry& p = g_ProtoPictures[i];
			if ( !lstrcmpA(p.szProtoname, szProto) && p.hbmPic != NULL)
				return &g_ProtoPictures[i];
		}
	}
	return NULL;
}

HANDLE GetContactThatHaveTheAvatar(HANDLE hContact, int locked = -1)
{
	if (g_MetaAvail && db_get_b(NULL, g_szMetaName, "Enabled", 0)) {
		if (db_get_dw(hContact, g_szMetaName, "NumContacts", 0) >= 1) {
			if (locked == -1)
				locked = db_get_b(hContact, "ContactPhoto", "Locked", 0);

			if (!locked)
				hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		}
	}
	return hContact;
}

INT_PTR GetAvatarBitmap(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0 || g_shutDown || fei == NULL)
		return 0;

	HANDLE hContact = (HANDLE) wParam;
	hContact = GetContactThatHaveTheAvatar(hContact);

	// Get the node
	struct CacheNode *node = FindAvatarInCache(hContact, TRUE);
	if (node == NULL || !node->loaded)
		return (INT_PTR) GetProtoDefaultAvatar(hContact);
	else
		return (INT_PTR) &node->ace;
}

// Just delete an avatar from cache
// An cache entry is never deleted. What is deleted is the image handle inside it
// This is done this way to keep track of which avatars avs have to keep track
void DeleteAvatarFromCache(HANDLE hContact, BOOL forever)
{
	hContact = GetContactThatHaveTheAvatar(hContact);

	struct CacheNode *node = FindAvatarInCache(hContact, FALSE);
	if (node == NULL) {
		struct CacheNode temp_node = {0};
		if (g_MetaAvail)
			temp_node.dwFlags |= (db_get_b(hContact, g_szMetaName, "IsSubcontact", 0) ? MC_ISSUBCONTACT : 0);
		NotifyMetaAware(hContact, &temp_node, (AVATARCACHEENTRY *)GetProtoDefaultAvatar(hContact));
		return;
	}
	node->mustLoad = -1;                        // mark for deletion
	if (forever)
		node->dwFlags |= AVS_DELETENODEFOREVER;
	SetEvent(hLoaderEvent);
}

int ChangeAvatar(HANDLE hContact, BOOL fLoad, BOOL fNotifyHist, int pa_format)
{
	if (g_shutDown)
		return 0;

	hContact = GetContactThatHaveTheAvatar(hContact);

	// Get the node
	struct CacheNode *node = FindAvatarInCache(hContact, g_AvatarHistoryAvail && fNotifyHist, TRUE);
	if (node == NULL)
		return 0;

	if (fNotifyHist)
		node->dwFlags |= AVH_MUSTNOTIFY;

	node->mustLoad = fLoad ? 1 : -1;
	node->pa_format = pa_format;
	SetEvent(hLoaderEvent);
	return 0;
}

/*
 * this thread scans the cache and handles nodes which have mustLoad set to > 0 (must be loaded/reloaded) or
 * nodes where mustLoad is < 0 (must be deleted).
 * its waken up by the event and tries to lock the cache only when absolutely necessary.
 */

static void PicLoader(LPVOID param)
{
	DWORD dwDelay = db_get_dw(NULL, AVS_MODULE, "picloader_sleeptime", 80);

	if (dwDelay < 30)
		dwDelay = 30;
	else if (dwDelay > 100)
		dwDelay = 100;

	while(!g_shutDown) {
		struct CacheNode *node = g_Cache;

		while(!g_shutDown && node) {
			if (node->mustLoad > 0 && node->ace.hContact) {
				node->mustLoad = 0;
				AVATARCACHEENTRY ace_temp;

				if (db_get_b(node->ace.hContact, "ContactPhoto", "NeedUpdate", 0))
					QueueAdd(node->ace.hContact);

				CopyMemory(&ace_temp, &node->ace, sizeof(AVATARCACHEENTRY));
				ace_temp.hbmPic = 0;

				int result = CreateAvatarInCache(node->ace.hContact, &ace_temp, NULL);

				if (result == -2) {
					char *szProto = GetContactProto(node->ace.hContact);
					if (szProto == NULL || Proto_NeedDelaysForAvatars(szProto))
						QueueAdd(node->ace.hContact);
					else if (FetchAvatarFor(node->ace.hContact, szProto) == GAIR_SUCCESS) // Try yo create again
						result = CreateAvatarInCache(node->ace.hContact, &ace_temp, NULL);
				}

				if (result == 1 && ace_temp.hbmPic != 0) { // Loaded
					HBITMAP oldPic = node->ace.hbmPic;

					EnterCriticalSection(&cachecs);
					CopyMemory(&node->ace, &ace_temp, sizeof(AVATARCACHEENTRY));
					node->loaded = TRUE;
					LeaveCriticalSection(&cachecs);
					if (oldPic)
						DeleteObject(oldPic);
					NotifyMetaAware(node->ace.hContact, node);
				}
				else if (result == 0 || result == -3) { // Has no avatar
					HBITMAP oldPic = node->ace.hbmPic;

					EnterCriticalSection(&cachecs);
					CopyMemory(&node->ace, &ace_temp, sizeof(AVATARCACHEENTRY));
					node->loaded = FALSE;
					node->mustLoad = 0;
					LeaveCriticalSection(&cachecs);
					if (oldPic)
						DeleteObject(oldPic);
					NotifyMetaAware(node->ace.hContact, node);
				}

				mir_sleep(dwDelay);
			}
			else if (node->mustLoad < 0 && node->ace.hContact) {         // delete this picture
				HANDLE hContact = node->ace.hContact;
				EnterCriticalSection(&cachecs);
				node->mustLoad = 0;
				node->loaded = 0;
				if (node->ace.hbmPic)
					DeleteObject(node->ace.hbmPic);
				ZeroMemory(&node->ace, sizeof(AVATARCACHEENTRY));
				if (node->dwFlags & AVS_DELETENODEFOREVER)
					node->dwFlags &= ~AVS_DELETENODEFOREVER;
				else
					node->ace.hContact = hContact;

				LeaveCriticalSection(&cachecs);
				NotifyMetaAware(hContact, node, (AVATARCACHEENTRY *)GetProtoDefaultAvatar(hContact));
			}
			// protect this by changes from the cache block allocator as it can cause inconsistencies while working
			// on allocating a new block.
			EnterCriticalSection(&alloccs);
			node = node->pNextNode;
			LeaveCriticalSection(&alloccs);
		}
		WaitForSingleObject(hLoaderEvent, INFINITE);
		//_DebugTrace(0, "pic loader awake...");
		ResetEvent(hLoaderEvent);
	}
}

static int MetaChanged(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0 || g_shutDown)
		return 0;

	AVATARCACHEENTRY *ace;

	HANDLE hContact = (HANDLE) wParam;
	HANDLE hSubContact = GetContactThatHaveTheAvatar(hContact);

	// Get the node
	struct CacheNode *node = FindAvatarInCache(hSubContact, TRUE);
	if (node == NULL || !node->loaded) {
		ace = (AVATARCACHEENTRY *)GetProtoDefaultAvatar(hSubContact);
		QueueAdd(hSubContact);
	}
	else ace = &node->ace;

	NotifyEventHooks(hEventChanged, (WPARAM)hContact, (LPARAM)ace);
	return 0;
}

static void LoadDefaultInfo()
{
	protoPicCacheEntry *pce = new protoPicCacheEntry;
	if (CreateAvatarInCache(0, pce, AVS_DEFAULT) != 1)
		db_unset(0, PPICT_MODULE, AVS_DEFAULT);

	pce->szProtoname = mir_strdup(AVS_DEFAULT);
	pce->tszAccName = mir_tstrdup(TranslateT("Global avatar"));
	g_ProtoPictures.insert(pce);
}

static void LoadProtoInfo(PROTOCOLDESCRIPTOR* proto)
{
	if (proto->type != PROTOTYPE_PROTOCOL || proto->cbSize != sizeof(*proto))
		return;

	char protoName[MAX_PATH];
	mir_snprintf(protoName, SIZEOF(protoName), "Global avatar for %s accounts", proto->szName);

	TCHAR protoNameTmp[MAX_PATH];
	mir_sntprintf(protoNameTmp, SIZEOF(protoNameTmp), TranslateT("Global avatar for %s accounts"), _A2T(proto->szName));
	protoPicCacheEntry *pce = new protoPicCacheEntry;
	if (CreateAvatarInCache(0, pce, protoName) != 1)
		db_unset(0, PPICT_MODULE, protoName);

	pce->szProtoname = mir_strdup(protoName);
	pce->tszAccName = mir_tstrdup(protoNameTmp);
	g_ProtoPictures.insert(pce);
}

static void LoadAccountInfo( PROTOACCOUNT* acc )
{
	protoPicCacheEntry *pce = new protoPicCacheEntry;
	if ( CreateAvatarInCache(0, pce, acc->szModuleName) != 1)
		db_unset(0, PPICT_MODULE, acc->szModuleName);

	pce->szProtoname = mir_strdup( acc->szModuleName);
	pce->tszAccName = mir_tstrdup( acc->tszAccountName);
	g_ProtoPictures.insert(pce);

	pce = new protoPicCacheEntry;
	CreateAvatarInCache((HANDLE)-1, pce, acc->szModuleName);
	pce->szProtoname = mir_strdup(acc->szModuleName);
	pce->tszAccName = mir_tstrdup(acc->tszAccountName);
	g_MyAvatars.insert(pce);
}

static int OnAccChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;

	switch( wParam ) {
	case PRAC_ADDED:
		LoadAccountInfo(pa);
		break;

	case PRAC_REMOVED:
		{
			int idx;
			protoPicCacheEntry tmp;
			tmp.szProtoname = mir_strdup(pa->szModuleName);
			if ((idx = g_ProtoPictures.getIndex( &tmp )) != -1)
				g_ProtoPictures.remove( idx );
			if ((idx = g_MyAvatars.getIndex( &tmp )) != -1)
				g_MyAvatars.remove( idx );
		}
		break;
	}

	return 0;
}

static int ModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	int i;
	DBVARIANT dbv = {0};
	TCHAR szEventName[100];
	int result = 0;

	mir_sntprintf(szEventName, 100, _T("avs_loaderthread_%d"), GetCurrentThreadId());
	hLoaderEvent = CreateEvent(NULL, TRUE, FALSE, szEventName);
	SetThreadPriority( mir_forkthread(PicLoader, 0), THREAD_PRIORITY_IDLE);

	// Folders plugin support
	hMyAvatarsFolder = FoldersRegisterCustomPathT(LPGEN("Avatars"), LPGEN("My Avatars"), MIRANDA_USERDATAT _T("\\Avatars"));
	hGlobalAvatarFolder = FoldersRegisterCustomPathT(LPGEN("Avatars"), LPGEN("My Global Avatar Cache"), MIRANDA_USERDATAT _T("\\Avatars"));

	g_AvatarHistoryAvail = ServiceExists(MS_AVATARHISTORY_ENABLED);

	g_MetaAvail = ServiceExists(MS_MC_GETPROTOCOLNAME) ? TRUE : FALSE;
	if (g_MetaAvail) {
		g_szMetaName = (char *)CallService(MS_MC_GETPROTOCOLNAME, 0, 0);
		if (g_szMetaName == NULL)
			g_MetaAvail = FALSE;
	}

	PROTOACCOUNT **accs = NULL;
	int accCount;
	ProtoEnumAccounts(&accCount, &accs);

	if (fei != NULL) {
		LoadDefaultInfo();
		PROTOCOLDESCRIPTOR** proto;
		int protoCount;
		CallService(MS_PROTO_ENUMPROTOS, (WPARAM)&protoCount, (LPARAM)&proto);
		for (i=0; i < protoCount; i++ )
			LoadProtoInfo( proto[i] );
		for (i=0; i < accCount; i++)
			LoadAccountInfo( accs[i] );
	}

	// Load global avatar
	protoPicCacheEntry *pce = new protoPicCacheEntry;
	CreateAvatarInCache((HANDLE)-1, pce, "");
	pce->szProtoname = mir_strdup("");
	g_MyAvatars.insert( pce );

	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccChanged);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownProc);
	HookEvent(ME_USERINFO_INITIALISE, OnDetailsInit);
	return 0;
}

static void ReloadMyAvatar(LPVOID lpParam)
{
	char *szProto = (char *)lpParam;

	mir_sleep(500);
	for(int i = 0; !g_shutDown && i < g_MyAvatars.getCount(); i++) {
		char *myAvatarProto = g_MyAvatars[i].szProtoname;

		if (szProto[0] == 0) {
			// Notify to all possibles
			if (lstrcmpA(myAvatarProto, szProto)) {
				if (!ProtoServiceExists( myAvatarProto, PS_SETMYAVATAR))
					continue;
				if (!Proto_IsAvatarsEnabled( myAvatarProto ))
					continue;
			}

		}
		else if (lstrcmpA(myAvatarProto, szProto))
			continue;

		if (g_MyAvatars[i].hbmPic)
			DeleteObject(g_MyAvatars[i].hbmPic);

		if (CreateAvatarInCache((HANDLE)-1, &g_MyAvatars[i], myAvatarProto) != -1)
			NotifyEventHooks(hMyAvatarChanged, (WPARAM)myAvatarProto, (LPARAM)&g_MyAvatars[i]);
		else
			NotifyEventHooks(hMyAvatarChanged, (WPARAM)myAvatarProto, 0);
	}

	free(lpParam);
}

static INT_PTR ReportMyAvatarChanged(WPARAM wParam, LPARAM lParam)
{
	if (wParam == NULL)
		return -1;

	char *proto = (char *) wParam;

	for(int i = 0; i < g_MyAvatars.getCount(); i++) {
		if (g_MyAvatars[i].dwFlags & AVS_IGNORENOTIFY)
			continue;

		if ( !lstrcmpA(g_MyAvatars[i].szProtoname, proto)) {
			LPVOID lpParam = (void *)malloc(lstrlenA(g_MyAvatars[i].szProtoname) + 2);
			strcpy((char *)lpParam, g_MyAvatars[i].szProtoname);
			mir_forkthread(ReloadMyAvatar, lpParam);
			return 0;
		}
	}

	return -2;
}

static int ContactSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	if (cws == NULL || g_shutDown)
		return 0;

	if (wParam == 0) {
		if (!strcmp(cws->szSetting, "AvatarFile") || !strcmp(cws->szSetting, "PictObject") || !strcmp(cws->szSetting, "AvatarHash") || !strcmp(cws->szSetting, "AvatarSaved"))
			ReportMyAvatarChanged((WPARAM) cws->szModule, 0);
		return 0;
	}

	if (g_MetaAvail && !strcmp(cws->szModule, g_szMetaName)) {
		if (lstrlenA(cws->szSetting) > 6 && !strncmp(cws->szSetting, "Status", 5))
			MetaChanged(wParam, 0);
	}
	return 0;
}

static int ContactDeleted(WPARAM wParam, LPARAM lParam)
{
	DeleteAvatarFromCache((HANDLE)wParam, TRUE);
	return 0;
}

static int ShutdownProc(WPARAM wParam, LPARAM lParam)
{
	g_shutDown = true;
	SetEvent(hLoaderEvent);
	SetEvent(hShutdownEvent);
	CloseHandle(hShutdownEvent); hShutdownEvent = NULL;
	return 0;
}

void InternalDrawAvatar(AVATARDRAWREQUEST *r, HBITMAP hbm, LONG bmWidth, LONG bmHeight, DWORD dwFlags)
{
	float dScale = 0;
	int newHeight, newWidth;
	HDC hdcAvatar;
	HBITMAP hbmMem;
	DWORD topoffset = 0, leftoffset = 0;
	HRGN rgn = 0, oldRgn = 0;
	int targetWidth = r->rcDraw.right - r->rcDraw.left;
	int targetHeight = r->rcDraw.bottom - r->rcDraw.top;
	BLENDFUNCTION bf = {0};

	hdcAvatar = CreateCompatibleDC(r->hTargetDC);
	hbmMem = (HBITMAP)SelectObject(hdcAvatar, hbm);

	if ((r->dwFlags & AVDRQ_DONTRESIZEIFSMALLER) && bmHeight <= targetHeight && bmWidth <= targetWidth) {
		newHeight = bmHeight;
		newWidth = bmWidth;
	}
	else if (bmHeight >= bmWidth) {
		dScale = targetHeight / (float)bmHeight;
		newHeight = targetHeight;
		newWidth = (int) (bmWidth * dScale);
	}
	else {
		dScale = targetWidth / (float)bmWidth;
		newWidth = targetWidth;
		newHeight = (int) (bmHeight * dScale);
	}

	topoffset = targetHeight > newHeight ? (targetHeight - newHeight) / 2 : 0;
	leftoffset = targetWidth > newWidth ? (targetWidth - newWidth) / 2 : 0;

	// create the region for the avatar border - use the same region for clipping, if needed.

	oldRgn = CreateRectRgn(0,0,1,1);

	if (GetClipRgn(r->hTargetDC, oldRgn) != 1)
	{
		DeleteObject(oldRgn);
		oldRgn = NULL;
	}

	if (r->dwFlags & AVDRQ_ROUNDEDCORNER)
		rgn = CreateRoundRectRgn(r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, r->rcDraw.left + leftoffset + newWidth + 1, r->rcDraw.top + topoffset + newHeight + 1, 2 * r->radius, 2 * r->radius);
	else
		rgn = CreateRectRgn(r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, r->rcDraw.left + leftoffset + newWidth, r->rcDraw.top + topoffset + newHeight);

	ExtSelectClipRgn(r->hTargetDC, rgn, RGN_AND);

	bf.SourceConstantAlpha = r->alpha > 0 ? r->alpha : 255;
	bf.AlphaFormat = dwFlags & AVS_PREMULTIPLIED ? AC_SRC_ALPHA : 0;

	if (!(r->dwFlags & AVDRQ_AERO))
		SetStretchBltMode(r->hTargetDC, HALFTONE);
	//else
	//	FillRect(r->hTargetDC, &r->rcDraw, (HBRUSH)GetStockObject(BLACK_BRUSH));

	if (r->dwFlags & AVDRQ_FORCEFASTALPHA && !(r->dwFlags & AVDRQ_AERO) && AvsAlphaBlend) {
		AvsAlphaBlend(
			r->hTargetDC, r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, newWidth, newHeight,
			hdcAvatar, 0, 0, bmWidth, bmHeight, bf);
	} else {
		if ((bf.SourceConstantAlpha == 255 && bf.AlphaFormat == 0 && !(r->dwFlags & AVDRQ_FORCEALPHA) && !(r->dwFlags & AVDRQ_AERO)) || !AvsAlphaBlend) {
			StretchBlt(r->hTargetDC, r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, newWidth, newHeight, hdcAvatar, 0, 0, bmWidth, bmHeight, SRCCOPY);
		} else {
			/*
			* get around SUCKY AlphaBlend() rescaling quality...
			*/
			FIBITMAP *fb = fei->FI_CreateDIBFromHBITMAP(hbm);
			FIBITMAP *fbResized = fei->FI_Rescale(fb, newWidth, newHeight, FILTER_BICUBIC);
			HBITMAP hbmResized = fei->FI_CreateHBITMAPFromDIB(fbResized);
			fei->FI_Unload(fb);
			fei->FI_Unload(fbResized);

			HBITMAP hbmTempOld;
			HDC hdcTemp = CreateCompatibleDC(r->hTargetDC);
			hbmTempOld = (HBITMAP)SelectObject(hdcTemp, hbmResized);

			AvsAlphaBlend(
				r->hTargetDC, r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, newWidth, newHeight,
				hdcTemp, 0, 0, newWidth, newHeight, bf);

			SelectObject(hdcTemp, hbmTempOld);
			DeleteObject(hbmResized);
			DeleteDC(hdcTemp);
		}

		if ((r->dwFlags & AVDRQ_DRAWBORDER) && !((r->dwFlags & AVDRQ_HIDEBORDERONTRANSPARENCY) && (dwFlags & AVS_HASTRANSPARENCY))) {
			HBRUSH br = CreateSolidBrush(r->clrBorder);
			HBRUSH brOld = (HBRUSH)SelectObject(r->hTargetDC, br);
			FrameRgn(r->hTargetDC, rgn, br, 1, 1);
			SelectObject(r->hTargetDC, brOld);
			DeleteObject(br);
		}

		SelectClipRgn(r->hTargetDC, oldRgn);
		DeleteObject(rgn);
		if (oldRgn) DeleteObject(oldRgn);

		SelectObject(hdcAvatar, hbmMem);
		DeleteDC(hdcAvatar);
	}
}

INT_PTR DrawAvatarPicture(WPARAM wParam, LPARAM lParam)
{
	AVATARDRAWREQUEST *r = (AVATARDRAWREQUEST *)lParam;
	AVATARCACHEENTRY *ace = NULL;

	if (fei == NULL || r == NULL || IsBadReadPtr((void *)r, sizeof(AVATARDRAWREQUEST)))
		return 0;

	if (r->cbSize != sizeof(AVATARDRAWREQUEST))
		return 0;

	if (r->dwFlags & AVDRQ_PROTOPICT) {
		if (r->szProto == NULL)
			return 0;

		for(int i = 0; i < g_ProtoPictures.getCount(); i++) {
			protoPicCacheEntry& p = g_ProtoPictures[i];
			if ( !lstrcmpA(p.szProtoname, r->szProto) && lstrlenA(r->szProto) == lstrlenA(p.szProtoname) && p.hbmPic != 0) {
				ace = (AVATARCACHEENTRY *)&g_ProtoPictures[i];
				break;
			}
		}
	}
	else if (r->dwFlags & AVDRQ_OWNPIC) {
		if (r->szProto == NULL)
			return 0;

		if (r->szProto[0] == '\0' && db_get_b(NULL, AVS_MODULE, "GlobalUserAvatarNotConsistent", 1))
			return -1;

		ace = (AVATARCACHEENTRY *)GetMyAvatar(0, (LPARAM)r->szProto);
	}
	else
		ace = (AVATARCACHEENTRY *)GetAvatarBitmap((WPARAM)r->hContact, 0);

	if (ace && (!(r->dwFlags & AVDRQ_RESPECTHIDDEN) || !(ace->dwFlags & AVS_HIDEONCLIST))) {
		ace->t_lastAccess = time(NULL);

		if (ace->bmHeight == 0 || ace->bmWidth == 0 || ace->hbmPic == 0)
			return 0;

		InternalDrawAvatar(r, ace->hbmPic, ace->bmWidth, ace->bmHeight, ace->dwFlags);
		return 1;
	}

	return 0;
}

static int LoadAvatarModule()
{
	InitializeCriticalSection(&cachecs);
	InitializeCriticalSection(&alloccs);

	hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	HookEvent(ME_PROTO_ACK, ProtocolAck);

	CreateServiceFunction(MS_AV_GETAVATARBITMAP, GetAvatarBitmap);
	CreateServiceFunction(MS_AV_PROTECTAVATAR, ProtectAvatar);
	CreateServiceFunction(MS_AV_SETAVATAR, SetAvatar);
	CreateServiceFunction(MS_AV_SETMYAVATAR, SetMyAvatar);
	CreateServiceFunction(MS_AV_CANSETMYAVATAR, CanSetMyAvatar);
	CreateServiceFunction(MS_AV_CONTACTOPTIONS, ContactOptions);
	CreateServiceFunction(MS_AV_DRAWAVATAR, DrawAvatarPicture);
	CreateServiceFunction(MS_AV_GETMYAVATAR, GetMyAvatar);
	CreateServiceFunction(MS_AV_REPORTMYAVATARCHANGED, ReportMyAvatarChanged);
	CreateServiceFunction(MS_AV_LOADBITMAP32, BmpFilterLoadBitmap32);
	CreateServiceFunction(MS_AV_SAVEBITMAP, BmpFilterSaveBitmap);
	CreateServiceFunction(MS_AV_CANSAVEBITMAP, BmpFilterCanSaveBitmap);
	CreateServiceFunction(MS_AV_RESIZEBITMAP, BmpFilterResizeBitmap);
	CreateServiceFunction(MS_AV_SETAVATARW, SetAvatarW);
	CreateServiceFunction(MS_AV_SETMYAVATARW, SetMyAvatarW);

	hEventChanged = CreateHookableEvent(ME_AV_AVATARCHANGED);
	hEventContactAvatarChanged = CreateHookableEvent(ME_AV_CONTACTAVATARCHANGED);
	hMyAvatarChanged = CreateHookableEvent(ME_AV_MYAVATARCHANGED);

	AllocCacheBlock();
	InitPolls();

	HMODULE hDll;
	if (hDll = GetModuleHandle(_T("gdi32")))
		AvsAlphaBlend = (BOOL (WINAPI *)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION)) GetProcAddress(hDll, "GdiAlphaBlend");
	if (AvsAlphaBlend == NULL && (hDll = LoadLibrary(_T("msimg32.dll"))))
		AvsAlphaBlend = (BOOL (WINAPI *)(HDC, int, int, int, int, HDC, int, int, int, int, BLENDFUNCTION)) GetProcAddress(hDll, "AlphaBlend");

	lstrcpyn(g_szDataPath, VARST(_T("%miranda_userdata%")), SIZEOF(g_szDataPath)-1);
	g_szDataPath[MAX_PATH - 1] = 0;
	_tcslwr(g_szDataPath);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID reserved)
{
	g_hInst = hInstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);

	INT_PTR result = CALLSERVICE_NOTFOUND;
	if (ServiceExists(MS_IMG_GETINTERFACE))
		result = CallService(MS_IMG_GETINTERFACE, FI_IF_VERSION, (LPARAM)&fei);

	if (fei == NULL || result != S_OK) {
		MessageBox(0, TranslateT("Fatal error, image services not found. Avatar services will be disabled."), TranslateT("Avatar Service"), MB_OK);
		return 1;
	}
	LoadACC();
	return LoadAvatarModule();
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UninitPolls();

	struct CacheNode *pNode = g_Cache;
	while(pNode) {
		if (pNode->ace.hbmPic != 0)
			DeleteObject(pNode->ace.hbmPic);
		pNode = pNode->pNextNode;
	}

	for(int i = 0; i < g_curBlock; i++)
		free(g_cacheBlocks[i]);
	free(g_cacheBlocks);

	g_ProtoPictures.destroy();
	g_MyAvatars.destroy();

	DestroyHookableEvent(hEventChanged);
	DestroyHookableEvent(hEventContactAvatarChanged);
	DestroyHookableEvent(hMyAvatarChanged);

	CloseHandle(hLoaderEvent);
	DeleteCriticalSection(&alloccs);
	DeleteCriticalSection(&cachecs);
	return 0;
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

BOOL Proto_IsAvatarsEnabled(const char *proto)
{
	if ( ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_ENABLED, 0);

	return TRUE;
}

BOOL Proto_IsAvatarFormatSupported(const char *proto, int format)
{
	if ( ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_FORMATSUPPORTED, format);

	if (format >= PA_FORMAT_SWF)
		return FALSE;

	return TRUE;
}

int Proto_AvatarImageProportion(const char *proto)
{
	if ( ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_PROPORTION, 0);

	return 0;
}

void Proto_GetAvatarMaxSize(const char *proto, int *width, int *height)
{
	if ( ProtoServiceExists(proto, PS_GETAVATARCAPS)) {
		POINT maxSize;
		CallProtoService(proto, PS_GETAVATARCAPS, AF_MAXSIZE, (LPARAM) &maxSize);
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
	if ( ProtoServiceExists(proto, PS_GETAVATARCAPS)) {
		int ret = CallProtoService(proto, PS_GETAVATARCAPS, AF_DONTNEEDDELAYS, 0);
		if (ret > 0)
			return FALSE;
		else
			return TRUE;
	}

	return TRUE;
}

int Proto_GetAvatarMaxFileSize(const char *proto)
{
	if ( ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_MAXFILESIZE, 0);

	return 0;
}

int Proto_GetDelayAfterFail(const char *proto)
{
	if ( ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_DELAYAFTERFAIL, 0);

	return 0;
}

BOOL Proto_IsFetchingAlwaysAllowed(const char *proto)
{
	if ( ProtoServiceExists(proto, PS_GETAVATARCAPS))
		return CallProtoService(proto, PS_GETAVATARCAPS, AF_FETCHALWAYS, 0);

	return FALSE;
}
