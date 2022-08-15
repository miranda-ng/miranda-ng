/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
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

#include "stdafx.h"

bool     g_shutDown = false;

CMPlugin g_plugin;

wchar_t  g_szDataPath[MAX_PATH];		// user datae path (read at startup only)
BOOL     g_AvatarHistoryAvail = FALSE;
HWND     hwndSetMyAvatar = nullptr;

HANDLE   hMyAvatarsFolder;
HANDLE   hGlobalAvatarFolder;
HANDLE   hLoaderEvent, hLoaderThread, hShutdownEvent;
HANDLE   hEventChanged, hEventContactAvatarChanged, hMyAvatarChanged;

char *g_szMetaName = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {E00F1643-263C-4599-B84B-053E5C511D29}
	{ 0xe00f1643, 0x263c, 0x4599, { 0xb8, 0x4b, 0x5, 0x3e, 0x5c, 0x51, 0x1d, 0x29 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("AVS_Settings", pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

static int ComparePicture(const protoPicCacheEntry *p1, const protoPicCacheEntry *p2)
{
	if (p1->cacheType != p2->cacheType)
		return p1->cacheType - p2->cacheType;

	return mir_strcmp(p1->szProtoname, p2->szProtoname);
}

OBJLIST<protoPicCacheEntry> g_ProtoPictures(10, ComparePicture), g_MyAvatars(10, ComparePicture);

// Stores the id of the dialog

static int ProtocolAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack != nullptr && ack->type == ACKTYPE_AVATAR && !db_mc_isMeta(ack->hContact)) {
		if (ack->result == ACKRESULT_SUCCESS) {
			if (ack->hProcess == nullptr)
				ProcessAvatarInfo(ack->hContact, GAIR_NOAVATAR, nullptr, ack->szModule);
			else
				ProcessAvatarInfo(ack->hContact, GAIR_SUCCESS, (PROTO_AVATAR_INFORMATION*)ack->hProcess, ack->szModule);
		}
		else if (ack->result == ACKRESULT_FAILED) {
			ProcessAvatarInfo(ack->hContact, GAIR_FAILED, (PROTO_AVATAR_INFORMATION*)ack->hProcess, ack->szModule);
		}
		else if (ack->result == ACKRESULT_STATUS) {
			char *szProto = Proto_GetBaseAccountName(ack->hContact);
			if (szProto == nullptr || Proto_NeedDelaysForAvatars(szProto)) {
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
	CacheNode *node = FindAvatarInCache(hSubContact, true);
	if (node == nullptr || !node->bLoaded) {
		ace = (AVATARCACHEENTRY*)GetProtoDefaultAvatar(hSubContact);
		QueueAdd(hSubContact);
	}
	else ace = node;

	NotifyEventHooks(hEventChanged, hMeta, (LPARAM)ace);
	return 0;
}

static void LoadDefaultInfo()
{
	protoPicCacheEntry *pce = new protoPicCacheEntry(PCE_TYPE_GLOBAL, AVS_DEFAULT);
	if (CreateAvatarInCache(0, pce, AVS_DEFAULT) != 1)
		db_unset(0, PPICT_MODULE, AVS_DEFAULT);

	g_ProtoPictures.insert(pce);
}

static void LoadProtoInfo(PROTOCOLDESCRIPTOR *proto)
{
	if (proto->type != PROTOTYPE_PROTOWITHACCS)
		return;

	char protoName[MAX_PATH];
	mir_snprintf(protoName, "Global avatar for %s accounts", proto->szName);

	protoPicCacheEntry *pce = new protoPicCacheEntry(PCE_TYPE_PROTO, protoName);
	if (CreateAvatarInCache(0, pce, protoName) != 1)
		db_unset(0, PPICT_MODULE, protoName);

	pce->pd = proto;
	g_ProtoPictures.insert(pce);
}

static void LoadAccountInfo(PROTOACCOUNT *acc)
{
	protoPicCacheEntry *pce = new protoPicCacheEntry(PCE_TYPE_ACCOUNT, acc->szModuleName);
	if (CreateAvatarInCache(0, pce, acc->szModuleName) != 1)
		db_unset(0, PPICT_MODULE, acc->szModuleName);

	pce->pa = acc;
	g_ProtoPictures.insert(pce);

	pce = new protoPicCacheEntry(PCE_TYPE_ACCOUNT, acc->szModuleName);
	CreateAvatarInCache(INVALID_CONTACT_ID, pce, acc->szModuleName);
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
		// little perversion not to call the object's destructor
		protoPicCacheEntry *tmp = (protoPicCacheEntry *)_alloca(sizeof(protoPicCacheEntry));
		tmp->cacheType = PCE_TYPE_ACCOUNT;
		tmp->szProtoname = pa->szModuleName;

		int idx;
		if ((idx = g_ProtoPictures.getIndex(tmp)) != -1)
			g_ProtoPictures.remove(idx);
		if ((idx = g_MyAvatars.getIndex(tmp)) != -1)
			g_MyAvatars.remove(idx);
		break;
	}

	return 0;
}

static int ContactSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
	if (cws == nullptr || g_shutDown)
		return 0;

	if (hContact == 0)
		if (!strcmp(cws->szSetting, "AvatarFile") || !strcmp(cws->szSetting, "PictObject") || !strcmp(cws->szSetting, "AvatarHash"))
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
	return 0;
}

void InternalDrawAvatar(AVATARDRAWREQUEST *r, HBITMAP hbm, LONG bmWidth, LONG bmHeight, uint32_t dwFlags)
{
	int targetWidth = r->rcDraw.right - r->rcDraw.left;
	int targetHeight = r->rcDraw.bottom - r->rcDraw.top;

	HDC hdcAvatar = CreateCompatibleDC(r->hTargetDC);
	HBITMAP hbmMem = (HBITMAP)SelectObject(hdcAvatar, hbm);

	float dScale;
	int newHeight, newWidth;
	if ((r->dwFlags & AVDRQ_DONTRESIZEIFSMALLER) && bmHeight <= targetHeight && bmWidth <= targetWidth) {
		dScale = 0;
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

	uint32_t topoffset = targetHeight > newHeight ? (targetHeight - newHeight) / 2 : 0;
	uint32_t leftoffset = targetWidth > newWidth ? (targetWidth - newWidth) / 2 : 0;

	// create the region for the avatar border - use the same region for clipping, if needed.
	HRGN oldRgn = CreateRectRgn(0, 0, 1, 1);

	if (GetClipRgn(r->hTargetDC, oldRgn) != 1) {
		DeleteObject(oldRgn);
		oldRgn = nullptr;
	}

	HRGN rgn;
	if (r->dwFlags & AVDRQ_ROUNDEDCORNER)
		rgn = CreateRoundRectRgn(r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, r->rcDraw.left + leftoffset + newWidth + 1, r->rcDraw.top + topoffset + newHeight + 1, 2 * r->radius, 2 * r->radius);
	else
		rgn = CreateRectRgn(r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, r->rcDraw.left + leftoffset + newWidth, r->rcDraw.top + topoffset + newHeight);

	ExtSelectClipRgn(r->hTargetDC, rgn, RGN_AND);

	BLENDFUNCTION bf = { 0 };
	bf.SourceConstantAlpha = r->alpha > 0 ? r->alpha : 255;
	bf.AlphaFormat = dwFlags & AVS_PREMULTIPLIED ? AC_SRC_ALPHA : 0;

	if (!(r->dwFlags & AVDRQ_AERO))
		SetStretchBltMode(r->hTargetDC, HALFTONE);
	//else
	//	FillRect(r->hTargetDC, &r->rcDraw, (HBRUSH)GetStockObject(BLACK_BRUSH));

	if (r->dwFlags & AVDRQ_FORCEFASTALPHA && !(r->dwFlags & AVDRQ_AERO))
		GdiAlphaBlend(r->hTargetDC, r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, newWidth, newHeight, hdcAvatar, 0, 0, bmWidth, bmHeight, bf);
	else {
		if (bf.SourceConstantAlpha == 255 && bf.AlphaFormat == 0 && !(r->dwFlags & AVDRQ_FORCEALPHA) && !(r->dwFlags & AVDRQ_AERO))
			StretchBlt(r->hTargetDC, r->rcDraw.left + leftoffset, r->rcDraw.top + topoffset, newWidth, newHeight, hdcAvatar, 0, 0, bmWidth, bmHeight, SRCCOPY);
		else {
			// get around SUCKY AlphaBlend() rescaling quality...
			FIBITMAP *fb = FreeImage_CreateDIBFromHBITMAP(hbm);
			FIBITMAP *fbResized = FreeImage_Rescale(fb, newWidth, newHeight, FILTER_BICUBIC);
			HBITMAP hbmResized = FreeImage_CreateHBITMAPFromDIB(fbResized);
			FreeImage_Unload(fb);
			FreeImage_Unload(fbResized);

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
	wchar_t szEventName[100];
	mir_snwprintf(szEventName, L"avs_loaderthread_%d", GetCurrentThreadId());
	hLoaderEvent = CreateEvent(nullptr, TRUE, FALSE, szEventName);

	SetThreadPriority(mir_forkthread(PicLoader), THREAD_PRIORITY_IDLE);

	// Folders plugin support
	hMyAvatarsFolder = FoldersRegisterCustomPathW(LPGEN("Avatars"), LPGEN("My Avatars"), MIRANDA_USERDATAW L"\\Avatars");
	hGlobalAvatarFolder = FoldersRegisterCustomPathW(LPGEN("Avatars"), LPGEN("My Global Avatar Cache"), MIRANDA_USERDATAW L"\\Avatars");

	g_AvatarHistoryAvail = ServiceExists(MS_AVATARHISTORY_ENABLED);

	LoadDefaultInfo();

	int protoCount;
	PROTOCOLDESCRIPTOR **proto;
	Proto_EnumProtocols(&protoCount, &proto);
	for (int i = 0; i < protoCount; i++)
		LoadProtoInfo(proto[i]);
	
	for (auto &it : Accounts())
		LoadAccountInfo(it);

	// Load global avatar
	protoPicCacheEntry *pce = new protoPicCacheEntry(PCE_TYPE_GLOBAL, "");
	CreateAvatarInCache(INVALID_CONTACT_ID, pce, "");
	g_MyAvatars.insert(pce);

	HookEvent(ME_PROTO_ACCLISTCHANGED, OnAccChanged);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, ShutdownProc);
	HookEvent(ME_USERINFO_INITIALISE, OnDetailsInit);
	return 0;
}

static IconItem iconList[] =
{
	{ LPGEN("Avatar"), "main", IDI_AVATAR },
};

int CMPlugin::Load()
{
	registerIcon("AVS", iconList);

	LoadACC();

	hShutdownEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

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

	wcsncpy_s(g_szDataPath, _countof(g_szDataPath), VARSW(L"%miranda_userdata%\\"), _TRUNCATE);
	wcslwr(g_szDataPath);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	UnregisterClassW(AVATAR_CONTROL_CLASS, 0);

	UninitPolls();
	UnloadCache();

	DestroyHookableEvent(hEventChanged);
	DestroyHookableEvent(hEventContactAvatarChanged);
	DestroyHookableEvent(hMyAvatarChanged);

	if (hLoaderThread)
		WaitForSingleObject(hLoaderThread, INFINITE);

	CloseHandle(hLoaderEvent);
	CloseHandle(hShutdownEvent);
	return 0;
}
