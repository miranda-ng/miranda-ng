//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#include "stdafx.h"
#include "resources.h"
#include "options.h"

#define AVA_FILE_NAME_FORMAT  L"%s\\%s\\AvatarCache\\Jabber\\" _T(SHORT_PLUGIN_NAME) L".pseudoava.png"
#define AVA_RES_TYPE  L"PNG"
#define SRMM_MODULE_NAME  "SRMM"
#define SRMM_AVATAR_SETTING_NAME  "Avatar"

#define SET_AVATAR_INTERVAL 2000

LPTSTR CreateAvaFile(HANDLE *hFile)
{
	wchar_t name[MAX_PATH + 2];
	if (CallService(MS_DB_GETPROFILENAMEW, (WPARAM)_countof(name), (LPARAM)&name))
		return NULL;

	wchar_t *p = wcsrchr(name, '.');
	if (p)
		*p = 0;

	wchar_t path[MAX_PATH + 2];
	if (CallService(MS_DB_GETPROFILEPATHW, (WPARAM)_countof(path), (LPARAM)&path))
		return NULL;

	wchar_t full[MAX_PATH + 2];
	mir_snwprintf(full, AVA_FILE_NAME_FORMAT, path, name);
	CreateDirectoryTreeW(full);

	HANDLE h = CreateFile(full, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
		return NULL;

	if (hFile)
		*hFile = h;
	else
		CloseHandle(h);

	return mir_wstrdup(full);
}

BOOL SaveAvatar(HANDLE hFile)
{
	HRSRC hres = FindResource(g_hInst, MAKEINTRESOURCE(IDI_PSEUDOAVA), AVA_RES_TYPE);
	if (!hres) return FALSE;

	HGLOBAL hglob = LoadResource(g_hInst, hres);
	if (!hglob) return FALSE;

	PVOID p = LockResource(hglob);
	if (!p) return FALSE;

	DWORD l = SizeofResource(g_hInst, hres);
	if (!l) return FALSE;

	DWORD written;
	if (!WriteFile(hFile, p, l, &written, NULL)) return FALSE;
	return written == l;
}

struct AVACHANGED {
	HANDLE hTimer;
	MCONTACT hContact;
};

VOID CALLBACK CallSetAvatar(PVOID lpParameter, BOOLEAN)
{
	Thread_Push(0);
	__try {
		AVACHANGED *ach = (AVACHANGED*)lpParameter;
		__try {
			SetAvatar(ach->hContact);
			DeleteTimerQueueTimer(NULL, ach->hTimer, NULL);
		}
		__finally {
			free(ach);
		}
	}
	__finally {
		Thread_Pop();
	}
}

int AvaChanged(WPARAM hContact, LPARAM lParam)
{
	if (!lParam && db_get_b(hContact, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0)) {
		BOOL enqueued = FALSE;
		AVACHANGED *ach = (AVACHANGED*)malloc(sizeof(AVACHANGED));
		__try {
			ach->hContact = hContact;
			enqueued = CreateTimerQueueTimer(&ach->hTimer, NULL, CallSetAvatar, ach, SET_AVATAR_INTERVAL, 0, WT_EXECUTEONLYONCE);
		}
		__finally {
			if (!enqueued) free(ach);
		}
	}
	return 0;
}

mir_cs g_csSetAvatar;
HANDLE hAvaChanged = 0;

BOOL InitAvaUnit(BOOL init)
{
	if (init) {
		hAvaChanged = HookEvent(ME_AV_AVATARCHANGED, AvaChanged);
		return hAvaChanged != 0;
	}

	if (hAvaChanged) {
		UnhookEvent(hAvaChanged);
		hAvaChanged = 0;
	}
	return TRUE;
}

void SetAvatar(MCONTACT hContact)
{
	mir_cslock lck(g_csSetAvatar);

	avatarCacheEntry *ava = (avatarCacheEntry*)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
	if (ava && GetFileAttributes(&ava->szFilename[0]) != INVALID_FILE_ATTRIBUTES)
		return;

	HANDLE hFile;
	ptrW avaFile(CreateAvaFile(&hFile));
	if (avaFile == NULL)
		return;

	BOOL saved = SaveAvatar(hFile);
	CloseHandle(hFile); hFile = 0;
	if (!saved)
		return;

	if (ava)
		CallService(MS_AV_SETAVATART, hContact, (LPARAM)L"");
	CallService(MS_AV_SETAVATART, hContact, (LPARAM)avaFile);
	db_set_ws(hContact, SRMM_MODULE_NAME, SRMM_AVATAR_SETTING_NAME, avaFile);
}
