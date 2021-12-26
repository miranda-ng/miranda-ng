/*
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

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

#include "../stdafx.h"

#define WMP_WINDOWCLASS L"MsnMsgrUIManager"

static LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static UINT hTimer = NULL;

WindowsMediaPlayer *singleton = nullptr;

WindowsMediaPlayer::WindowsMediaPlayer()
{
	name = L"WindowsMediaPlayer";
	received[0] = '\0';
	singleton = this;

	WNDCLASS wc = {};
	wc.lpfnWndProc = ReceiverWndProc;
	wc.hInstance = g_plugin.getInst();
	wc.lpszClassName = WMP_WINDOWCLASS;
	RegisterClass(&wc);

	hWnd = CreateWindow(WMP_WINDOWCLASS, LPGENW("Miranda ListeningTo WMP receiver"), 0, 0, 0, 0, 0, nullptr, nullptr, g_plugin.getInst(), nullptr);
}

WindowsMediaPlayer::~WindowsMediaPlayer()
{
	if (hTimer != NULL) {
		KillTimer(nullptr, hTimer);
		hTimer = NULL;
	}

	DestroyWindow(hWnd);
	hWnd = nullptr;

	UnregisterClass(WMP_WINDOWCLASS, g_plugin.getInst());
	singleton = nullptr;
}

void WindowsMediaPlayer::ProcessReceived()
{
	{
		mir_cslock lck(cs);
		FreeData();

		// Do the processing
		// MSNMusicString = L"\\0Music\\0%d\\0%s\\0%s\\0%s\\0%s\\0%s\\0\\0"
		// MSNMusicString, msn->msncommand, strMSNFormat, msn->title, msn->artist, msn->album, msn->wmcontentid);

		wchar_t *p1 = wcsstr(received, L"\\0");

		if (received[0] == L'\0' || p1 == nullptr) {
			NotifyInfoChanged();
			return;
		}

		// Process string
		wchar_t *parts[8] = {};
		int pCount = 0;
		wchar_t *p = received;
		do {
			*p1 = L'\0';
			parts[pCount] = p;
			pCount++;
			p = p1 + 2;
			p1 = wcsstr(p, L"\\0");
		} while (p1 != nullptr && pCount < 7);
		if (p1 != nullptr)
			*p1 = L'\0';
		parts[pCount] = p;

		// Fill cache
		if (pCount > 4 && !IsEmpty(parts[1]) && (!IsEmpty(parts[4]) || !IsEmpty(parts[5]))) {
			listening_info.cbSize = sizeof(listening_info);
			listening_info.dwFlags = LTI_TCHAR;

			listening_info.ptszType = U2T(parts[1]);
			listening_info.ptszTitle = U2T(parts[4]);
			listening_info.ptszArtist = U2T(parts[5]);
			listening_info.ptszAlbum = U2T(parts[6]);

			listening_info.ptszPlayer = mir_wstrdup(name);
		}

		// Put back the '\\'s
		for (int i = 1; i <= pCount; i++)
			*(parts[i] - 2) = L'\\';
		if (p1 != nullptr)
			*p1 = L'\\';
	}

	NotifyInfoChanged();
}

static VOID CALLBACK SendTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(nullptr, hTimer);
	hTimer = NULL;

	if (!loaded)
		return;

	if (singleton != nullptr)
		singleton->ProcessReceived();
}

void WindowsMediaPlayer::NewData(const wchar_t *data, size_t len)
{
	mir_cslock lck(cs);

	len = min(len, 1023);
	if (wcsncmp(received, data, len) != 0) {
		wcsncpy(received, data, len);
		received[len] = '\0';

		if (hTimer)
			KillTimer(nullptr, hTimer);
		hTimer = SetTimer(nullptr, NULL, 300, SendTimerProc); // Do the processing after we return true
	}
}

static LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COPYDATA) {
		if (!loaded)
			return FALSE;

		if (singleton == nullptr || !singleton->enabled)
			return FALSE;

		COPYDATASTRUCT* pData = (PCOPYDATASTRUCT)lParam;
		if (pData->dwData != 0x547 || pData->cbData == 0 || pData->lpData == nullptr)
			return FALSE;

		if (singleton != nullptr)
			singleton->NewData((wchar_t *)pData->lpData, pData->cbData / 2);

		return TRUE;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
