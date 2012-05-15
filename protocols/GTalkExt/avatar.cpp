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
#include "avatar.h"
#include "resources.h"
#include "options.h"

static const LPSTR AVA_FILE_NAME_FORMAT = "%s\\%s\\AvatarCache\\Jabber\\" SHORT_PLUGIN_NAME ".pseudoava.png";
static const LPTSTR AVA_RES_TYPE = _T("PNG");
static const LPSTR SRMM_MODULE_NAME = "SRMM";
static const LPSTR SRMM_AVATAR_SETTING_NAME = "Avatar";

static const int SET_AVATAR_INTERVAL = 2000;

void ForceDir(LPSTR dir, int len)
{
	if (GetFileAttributesA(dir) != INVALID_FILE_ATTRIBUTES) return;
	for (int i = len - 1; i >= 0; i--)
		if ('\\' == dir[i]) {
			dir[i] = 0;
			__try {
				ForceDir(dir, i);
			}
			__finally {
				dir[i] = '\\';
			}
			CreateDirectoryA(dir, NULL);
		}
}

void ForceFileDir(LPSTR file)
{
	for (int i = lstrlenA(file) - 1; i >= 0; i--)
		if ('\\' == file[i]) {
			file[i] = 0;
			__try {
				ForceDir(file, i);
			}
			__finally {
				file[i] = '\\';
			}
			break;
		}
}

LPSTR CreateAvaFile(HANDLE *hFile)
{
	char name[MAX_PATH + 2];
	char path[MAX_PATH + 2];
	char full[MAX_PATH + 2];

	if (CallService(MS_DB_GETPROFILENAME, (WPARAM)sizeof(name), (LPARAM)&name))
		return NULL;
	for (int i = lstrlenA(name); i >= 0; i--)
		if ('.' == name[i]) {
			name[i] = 0;
			break;
		}

	if (CallService(MS_DB_GETPROFILEPATH, (WPARAM)sizeof(path), (LPARAM)&path))
		return NULL;
	sprintf(&full[0], AVA_FILE_NAME_FORMAT, path, name);

	ForceFileDir(&full[0]);

	HANDLE h = 0;
	__try {
		h = CreateFileA(&full[0], GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (INVALID_HANDLE_VALUE == h) return NULL;

		if (hFile) *hFile = h; else CloseHandle(h);
		h = 0;
		return _strdup(&full[0]);
	}
	__finally {
		CloseHandle(h);
	}
}

extern HINSTANCE hInst;

BOOL SaveAvatar(HANDLE hFile)
{
	HRSRC hres = FindResource(hInst, MAKEINTRESOURCE(IDI_PSEUDOAVA), AVA_RES_TYPE);
	if (!hres) return FALSE;

	HGLOBAL hglob = LoadResource(hInst, hres);
	if (!hglob) return FALSE;

	PVOID p = LockResource(hglob);
	if (!p) return FALSE;

	DWORD l = SizeofResource(hInst, hres);
	if (!l) return FALSE;

	DWORD written;
	if (!WriteFile(hFile, p, l, &written, NULL)) return FALSE;
	return written == l;
}

struct AVACHANGED {
	HANDLE hTimer;
	HANDLE hContact;
};

VOID CALLBACK CallSetAvatar(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
	CallService(MS_SYSTEM_THREAD_PUSH, 0, 0);
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
		CallService(MS_SYSTEM_THREAD_POP, 0, 0);
	}
}

int AvaChanged(WPARAM wParam, LPARAM lParam)
{
	if (!lParam && DBGetContactSettingByte((HANDLE)wParam, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0)) {
		BOOL enqueued = FALSE;
		AVACHANGED *ach = (AVACHANGED*)malloc(sizeof(AVACHANGED));
		__try {
			ach->hContact = (HANDLE)wParam;
			enqueued = CreateTimerQueueTimer(&ach->hTimer, NULL, CallSetAvatar, ach, SET_AVATAR_INTERVAL, 0, WT_EXECUTEONLYONCE);
		}
		__finally {
			if (!enqueued) free(ach);
		}
	}
	return 0;
}

CRITICAL_SECTION g_csSetAvatar;
HANDLE hAvaChanged = 0;
BOOL initialized = FALSE;

BOOL InitAvaUnit(BOOL init)
{
	if (init) {
		hAvaChanged = HookEvent(ME_AV_AVATARCHANGED, AvaChanged);
		InitializeCriticalSection(&g_csSetAvatar);
		initialized = TRUE;
		return hAvaChanged != 0;
	}
	else {
		if (initialized) {
			initialized = FALSE;
			DeleteCriticalSection(&g_csSetAvatar);
		}
		if (hAvaChanged) {
			UnhookEvent(hAvaChanged);
			hAvaChanged = 0;
		}
		return TRUE;
	}
}

void SetAvatar(HANDLE hContact)
{
	EnterCriticalSection(&g_csSetAvatar);
	__try {
		avatarCacheEntry *ava = (avatarCacheEntry*)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)hContact, 0);
		if (ava && GetFileAttributesA(&ava->szFilename[0]) != INVALID_FILE_ATTRIBUTES)
			return;

		HANDLE hFile;
		LPSTR avaFile = CreateAvaFile(&hFile);
		if (avaFile)
			__try {
				BOOL saved = SaveAvatar(hFile);
				CloseHandle(hFile); hFile = 0;
				if (saved){
					if (ava) CallService(MS_AV_SETAVATAR, (WPARAM)hContact, (LPARAM)"");
					CallService(MS_AV_SETAVATAR, (WPARAM)hContact, (LPARAM)avaFile);
					DBWriteContactSettingString(hContact, SRMM_MODULE_NAME, SRMM_AVATAR_SETTING_NAME, avaFile);
				}
			}
			__finally {
				free(avaFile);
				CloseHandle(hFile);
			}
	}
	__finally {
		LeaveCriticalSection(&g_csSetAvatar);
	}
}