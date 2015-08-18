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

#include "..\stdafx.h"

static UINT hTimer = NULL;
static HANDLE hLog = NULL;

GenericPlayer *singleton = NULL;

void m_log(const TCHAR *function, const TCHAR *fmt, ...)
{
	if (hLog == NULL) {
		hLog = mir_createLog(MODULE_NAME, _T("ListeningTo log"), _T("c:\\temp\\listeningto.txt"), 0);
		if (hLog == NULL)
			return;
	}

	mir_writeLogT(hLog, _T("%s: "), function);

	va_list args;
	va_start(args, fmt);
	mir_writeLogVT(hLog, fmt, args);
}

static LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COPYDATA:
		if (loaded) {
			COPYDATASTRUCT *pData = (PCOPYDATASTRUCT)lParam;
			if (pData == NULL || pData->dwData != MIRANDA_DW_PROTECTION || pData->cbData == 0 || pData->lpData == NULL)
				return FALSE;

			if (singleton != NULL)
				singleton->NewData((WCHAR *)pData->lpData, pData->cbData / 2);

			return TRUE;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

GenericPlayer::GenericPlayer()
{
	name = _T("GenericPlayer");

	enabled = TRUE;
	received[0] = L'\0';
	singleton = this;

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = ReceiverWndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = MIRANDA_WINDOWCLASS;

	RegisterClass(&wc);

	hWnd = CreateWindow(MIRANDA_WINDOWCLASS, LPGENT("Miranda ListeningTo receiver"),
		0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
}



GenericPlayer::~GenericPlayer()
{
	if (hTimer != NULL) {
		KillTimer(NULL, hTimer);
		hTimer = NULL;
	}

	DestroyWindow(hWnd);
	hWnd = NULL;

	UnregisterClass(MIRANDA_WINDOWCLASS, hInst);
	singleton = NULL;
}



void GenericPlayer::ProcessReceived()
{
	mir_cslockfull lck(cs);

	// Do the processing
	// L"<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0\\0"

	WCHAR *p1 = wcsstr(received, L"\\0");
	if (IsEmpty(received) || p1 == NULL)
		return;

	// Process string
	WCHAR *parts[11] = { 0 };
	int pCount = 0;
	WCHAR *p = received;
	do {
		*p1 = _T('\0');
		parts[pCount] = p;
		pCount++;
		p = p1 + 2;
		p1 = wcsstr(p, _T("\\0"));
	} while (p1 != NULL && pCount < 10);
	if (p1 != NULL)
		*p1 = _T('\0');
	parts[pCount] = p;

	if (pCount < 5)
		return;

	// See if player is enabled
	Player *player = this;
	for (int i = FIRST_PLAYER; i < NUM_PLAYERS; i++) {

		WCHAR *player_name = players[i]->name;

		if (_wcsicmp(parts[1], player_name) == 0) {
			player = players[i];
			break;
		}
	}

	player->FreeData();

	if (wcscmp(L"1", parts[0]) != 0 || IsEmpty(parts[1]) || (IsEmpty(parts[3]) && IsEmpty(parts[4]))) {
		// Stoped playing or not enought info
	}
	else {
		mir_cslock plck(player->GetLock());
		LISTENINGTOINFO *li = player->GetInfo();

		li->cbSize = sizeof(listening_info);
		li->dwFlags = LTI_TCHAR;
		li->ptszType = U2TD(parts[2], _T("Music"));
		li->ptszTitle = U2T(parts[3]);
		li->ptszArtist = U2T(parts[4]);
		li->ptszAlbum = U2T(parts[5]);
		li->ptszTrack = U2T(parts[6]);
		li->ptszYear = U2T(parts[7]);
		li->ptszGenre = U2T(parts[8]);

		if (player == this)
			li->ptszPlayer = mir_u2t(parts[1]);
		else
			li->ptszPlayer = mir_tstrdup(player->name);

		if (parts[9] != NULL) {
			long length = _wtoi(parts[9]);
			if (length > 0) {
				li->ptszLength = (TCHAR*)mir_alloc(10 * sizeof(TCHAR));

				int s = length % 60;
				int m = (length / 60) % 60;
				int h = (length / 60) / 60;

				if (h > 0)
					mir_sntprintf(li->ptszLength, 9, _T("%d:%02d:%02d"), h, m, s);
				else
					mir_sntprintf(li->ptszLength, 9, _T("%d:%02d"), m, s);
			}
		}
	}

	// Put back the '\\'s
	for (int i = 1; i <= pCount; i++)
		*(parts[i] - 2) = L'\\';
	if (p1 != NULL)
		*p1 = L'\\';

	wcscpy(last_received, received);
	lck.unlock();

	NotifyInfoChanged();
}


static VOID CALLBACK SendTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(NULL, hTimer);
	hTimer = NULL;

	if (!loaded)
		return;

	if (singleton != NULL)
		singleton->ProcessReceived();
}


void GenericPlayer::NewData(const WCHAR *data, size_t len)
{
	if (data[0] == _T('\0'))
		return;

	mir_cslock lck(cs);

	len = min(len, 1023);
	if (wcsncmp(received, data, len) != 0) {
		wcsncpy(received, data, len);
		received[len] = L'\0';

		if (hTimer)
			KillTimer(NULL, hTimer);
		hTimer = SetTimer(NULL, NULL, 300, SendTimerProc); // Do the processing after we return true
	}
}
