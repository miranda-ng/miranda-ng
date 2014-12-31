/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org)
Copyright (c) 2000-04 Miranda ICQ/IM project,
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

CLIST_INTERFACE *pcli;

HINSTANCE g_hInst = 0;
HICON g_hIcon = 0;
bool g_shutDown = false;

int hLangpack;

TCHAR  g_szDataPath[MAX_PATH];		// user datae path (read at startup only)
BOOL   g_AvatarHistoryAvail = FALSE;
HWND   hwndSetMyAvatar = 0;

HANDLE hMyAvatarsFolder;
HANDLE hGlobalAvatarFolder;
HANDLE hLoaderEvent, hShutdownEvent;
HANDLE hEventChanged, hEventContactAvatarChanged, hMyAvatarChanged;

void   InitServices();

static int ComparePicture(const protoPicCacheEntry* p1, const protoPicCacheEntry* p2)
{
	if ((mir_strcmp(p1->szProtoname, "Global avatar") == 0) || strstr(p1->szProtoname, "Global avatar"))
		return -1;
	if ((mir_strcmp(p2->szProtoname, "Global avatar") == 0) || strstr(p1->szProtoname, "Global avatar"))
		return 1;
	return mir_strcmp(p1->szProtoname, p2->szProtoname);
}

OBJLIST<protoPicCacheEntry>
g_ProtoPictures(10, ComparePicture),
g_MyAvatars(10, ComparePicture);

char* g_szMetaName = NULL;

// Stores the id of the dialog

int OnDetailsInit(WPARAM wParam, LPARAM lParam);
int OptInit(WPARAM wParam, LPARAM lParam);

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
	{ 0xe00f1643, 0x263c, 0x4599, { 0xb8, 0x4b, 0x5, 0x3e, 0x5c, 0x51, 0x1d, 0x29 } }
};

static int ProtocolAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack != NULL && ack->type == ACKTYPE_AVATAR && !db_mc_isMeta(ack->hContact)) {
		if (ack->result == ACKRESULT_SUCCESS) {
			if (ack->hProcess == NULL)
				ProcessAvatarInfo(ack->hContact, GAIR_NOAVATAR, NULL, ack->szModule);
			else
				ProcessAvatarInfo(ack->hContact, GAIR_SUCCESS, (PROTO_AVATAR_INFORMATIONT *)ack->hProcess, ack->szModule);
		}
		else if (ack->result == ACKRESULT_FAILED) {
			ProcessAvatarInfo(ack->hContact, GAIR_FAILED, (PROTO_AVATAR_INFORMATIONT *)ack->hProcess, ack->szModule);
		}
		else if (ack->result == ACKRESULT_STATUS) {
			char *szProto = GetContactProto(ack->hContact);
			if (szProto == NULL || Proto_NeedDelaysForAvatars(szProto)) {
				// Queue
				db_set_b(ack->hContact, "ContactPhoto", "NeedUpdate", 1);
				QueueAdd(ack->hContact);
			}
			else // Fetch it now
				FetchAvatarFor(ack->hContact, szProto);
		}
	}
	return 0;
}

static int MetaChanged(WPARAM hMeta, LPARAM hSubContact)
{
	if (g_shutDown)
		return 0;

	AVATARCACHEENTRY *ace;

	// Get the node
	CacheNode *node = FindAvatarInCache(hSubContact, TRUE);
	if (node == NULL || !node->loaded) {
		ace = (AVATARCACHEENTRY*)GetProtoDefaultAvatar(hSubContact);
		QueueAdd(hSubContact);
	}
	else ace = node;

	NotifyEventHooks(hEventChanged, hMeta, (LPARAM)ace);
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

static void LoadProtoInfo(PROTOCOLDESCRIPTOR *proto)
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

static void LoadAccountInfo(PROTOACCOUNT *acc)
{
	protoPicCacheEntry *pce = new protoPicCacheEntry;
	if (CreateAvatarInCache(0, pce, acc->szModuleName) != 1)
		db_unset(0, PPICT_MODULE, acc->szModuleName);

	pce->szProtoname = mir_strdup(acc->szModuleName);
	pce->tszAccName = mir_tstrdup(acc->tszAccountName);
	g_ProtoPictures.insert(pce);

	pce = new protoPicCacheEntry;
	CreateAvatarInCache(INVALID_CONTACT_ID, pce, acc->szModuleName);
	pce->szProtoname = mir_strdup(acc->szModuleName);
	pce->tszAccName = mir_tstrdup(acc->tszAccountName);
	g_MyAvatars.insert(pce);
}

static int OnAccChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *pa = (PROTOACCOUNT*)lParam;

	switch (wParam) {
	case PRAC_ADDED:
		LoadAccountInfo(pa);
		break;

	case PRAC_REMOVED:
	{
		int idx;
		protoPicCacheEntry tmp;
		tmp.szProtoname = mir_strdup(pa->szModuleName);
		if ((idx = g_ProtoPictures.getIndex(&tmp)) != -1)
			g_ProtoPictures.remove(idx);
		if ((idx = g_MyAvatars.getIndex(&tmp)) != -1)
			g_MyAvatars.remove(idx);
	}
	break;
	}

	return 0;
}

static int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	if (cws == NULL || g_shutDown)
		return 0;

	if (hContact == 0)
		if (!strcmp(cws->szSetting, "AvatarFile") || !strcmp(cws->szSetting, "PictObject") || !strcmp(cws->szSetting, "AvatarHash") || !strcmp(cws->szSetting, "AvatarSaved"))
			ReportMyAvatarChanged((WPARAM)cws->szModule, 0);

	return 0;
}

static int ContactDeleted(WPARAM wParam, LPARAM)
{
	DeleteAvatarFromCache(wParam, TRUE);
	return 0;
}

static int ShutdownProc(WPARAM, LPARAM)
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
	BLENDFUNCTION bf = { 0 };

	hdcAvatar = CreateCompatibleDC(r->hTargetDC);
	hbmMem = (HBITMAP)SelectObject(hdcAvatar, hbm);

	if ((r->dwFlags & AVDRQ_DONTRESIZEIFSMALLER) && bmHeight <= targetHeight && bmWidth <= targetWidth) {
		newHeight = bmHeight;
		newWidth = bmWidth;
	}
	else if (bmHeight >= bmWidth) {
		dScale = targetHeight / (float)bmHeight;
		newHeight = targetHeight;
		newWidth = (int)(bmWidth * dScale);
	}
	else {
		dScale = targetWidth / (float)bmWidth;
		newWidth = targetWidth;
		newHeight = (int)(bmHeight * dScale);
	}

	topoffset = targetHeight > newHeight ? (targetHeight - newHeight) / 2 : 0;
	leftoffset = targetWidth > newWidth ? (targetWidth - newWidth) / 2 : 0;

	// create the region for the avatar border - use the same region for clipping, if needed.

	oldRgn = CreateRectRgn(0, 0, 1, 1);

	if (GetClipRgn(r->hTargetDC, oldRgn) != 1) {
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

	if (r->dwFlags & AVDRQ_FORCEFASTALPHA && !(r->dwFlags & AVDRQ_AERO)) {
		GdiAlphaBlend(
			r->hTargetDC, r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, newWidth, newHeight,
			hdcAvatar, 0, 0, bmWidth, bmHeight, bf);
	}
	else {
		if (bf.SourceConstantAlpha == 255 && bf.AlphaFormat == 0 && !(r->dwFlags & AVDRQ_FORCEALPHA) && !(r->dwFlags & AVDRQ_AERO)) {
			StretchBlt(r->hTargetDC, r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, newWidth, newHeight, hdcAvatar, 0, 0, bmWidth, bmHeight, SRCCOPY);
		}
		else {
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

			GdiAlphaBlend(
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

static int ModulesLoaded(WPARAM, LPARAM)
{
	int i;
	TCHAR szEventName[100];

	mir_sntprintf(szEventName, SIZEOF(szEventName), _T("avs_loaderthread_%d"), GetCurrentThreadId());
	hLoaderEvent = CreateEvent(NULL, TRUE, FALSE, szEventName);
	SetThreadPriority(mir_forkthread(PicLoader, 0), THREAD_PRIORITY_IDLE);

	// Folders plugin support
	hMyAvatarsFolder = FoldersRegisterCustomPathT(LPGEN("Avatars"), LPGEN("My Avatars"), MIRANDA_USERDATAT _T("\\Avatars"));
	hGlobalAvatarFolder = FoldersRegisterCustomPathT(LPGEN("Avatars"), LPGEN("My Global Avatar Cache"), MIRANDA_USERDATAT _T("\\Avatars"));

	g_AvatarHistoryAvail = ServiceExists(MS_AVATARHISTORY_ENABLED);

	PROTOACCOUNT **accs = NULL;
	int accCount;
	ProtoEnumAccounts(&accCount, &accs);

	if (fei != NULL) {
		LoadDefaultInfo();
		PROTOCOLDESCRIPTOR** proto;
		int protoCount;
		CallService(MS_PROTO_ENUMPROTOS, (WPARAM)&protoCount, (LPARAM)&proto);
		for (i = 0; i < protoCount; i++)
			LoadProtoInfo(proto[i]);
		for (i = 0; i < accCount; i++)
			LoadAccountInfo(accs[i]);
	}

	// Load global avatar
	protoPicCacheEntry *pce = new protoPicCacheEntry;
	CreateAvatarInCache(INVALID_CONTACT_ID, pce, "");
	pce->szProtoname = mir_strdup("");
	g_MyAvatars.insert(pce);

	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccChanged);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownProc);
	HookEvent(ME_USERINFO_INITIALISE, OnDetailsInit);
	return 0;
}

static int LoadAvatarModule()
{
	hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HookEvent(ME_OPT_INITIALISE, OptInit);
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, ContactSettingChanged);
	HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	HookEvent(ME_PROTO_ACK, ProtocolAck);
	HookEvent(ME_MC_DEFAULTTCHANGED, MetaChanged);
	HookEvent(ME_MC_SUBCONTACTSCHANGED, MetaChanged);

	hEventChanged = CreateHookableEvent(ME_AV_AVATARCHANGED);
	hEventContactAvatarChanged = CreateHookableEvent(ME_AV_CONTACTAVATARCHANGED);
	hMyAvatarChanged = CreateHookableEvent(ME_AV_MYAVATARCHANGED);

	InitServices();
	InitPolls();

	_tcsncpy_s(g_szDataPath, SIZEOF(g_szDataPath), VARST(_T("%miranda_userdata%\\")), _TRUNCATE);
	_tcslwr(g_szDataPath);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD, LPVOID)
{
	g_hInst = hInstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD)
{
	return &pluginInfoEx;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfoEx);
	mir_getCLI();

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
	UnloadCache();

	DestroyHookableEvent(hEventChanged);
	DestroyHookableEvent(hEventContactAvatarChanged);
	DestroyHookableEvent(hMyAvatarChanged);

	CloseHandle(hLoaderEvent);
	return 0;
}
