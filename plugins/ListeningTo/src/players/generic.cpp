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

#include "..\commons.h"

static LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static UINT hTimer = NULL;

GenericPlayer *singleton = NULL;

int m_log(const TCHAR *function, const TCHAR *fmt, ...)
{
#if 0
    va_list va;
    TCHAR text[1024];
	size_t len;

	len = mir_sntprintf(text, SIZEOF(text), _T("[%08u - %08u] [%s] "), 
				 GetCurrentThreadId(), GetTickCount(), function);

    va_start(va, fmt);
    mir_vsntprintf(&text[len], SIZEOF(text) - len, fmt, va);
    va_end(va);

	BOOL writeBOM = (GetFileAttributes(_T("c:\\miranda_listeningto.log.txt")) == INVALID_FILE_ATTRIBUTES);

	FILE *fp = _tfopen(_T("c:\\miranda_listeningto.log.txt"), _T("ab"));

	if (fp != NULL)
	{

		if (writeBOM)
			fwprintf(fp, L"\xFEFF");


		_ftprintf(fp, _T("%s\r\n"), text);
		fclose(fp);
		return 0;
	}
	else
	{
		return -1;
	}
#else
	return 0;
#endif
}


GenericPlayer::GenericPlayer()
{
	name = _T("GenericPlayer");

	enabled = TRUE;
	received[0] = L'\0';
	singleton = this;

	WNDCLASS wc = {0};
	wc.lpfnWndProc		= ReceiverWndProc;
	wc.hInstance		= hInst;
	wc.lpszClassName	= MIRANDA_WINDOWCLASS;

	RegisterClass(&wc);

	hWnd = CreateWindow(MIRANDA_WINDOWCLASS, LPGENT("Miranda ListeningTo receiver"), 
						0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
}



GenericPlayer::~GenericPlayer()
{
	if (hTimer != NULL)
	{
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
	EnterCriticalSection(&cs);

	// Do the processing
	// L"<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0\\0"

	WCHAR *p1 = wcsstr(received, L"\\0");

	if (IsEmpty(received) || p1 == NULL)
	{
//		if (received[0] == L'\0')
//			m_log(_T("ProcessReceived"), _T("ERROR: Empty text"));
//		else
//			m_log(_T("ProcessReceived"), _T("ERROR: No \\0 found"));

		// Ignore
		LeaveCriticalSection(&cs);
		return;
	}

	// Process string
	WCHAR *parts[11] = {0};
	int pCount = 0;
	WCHAR *p = received;
	do {
		*p1 = _T('\0');
		parts[pCount] = p;
		pCount ++;
		p = p1 + 2;
		p1 = wcsstr(p, _T("\\0"));
	} while( p1 != NULL && pCount < 10 );
	if (p1 != NULL)
		*p1 = _T('\0');
	parts[pCount] = p;

	if (pCount < 5)
	{
//		m_log(_T("ProcessReceived"), _T("ERROR: Too little pieces"));

		// Ignore
		LeaveCriticalSection(&cs);
		return;
	}

	// See if player is enabled
	Player *player = this;
	for (int i = FIRST_PLAYER; i < NUM_PLAYERS; i++)
	{

		WCHAR *player_name = players[i]->name;

		if (_wcsicmp(parts[1], player_name) == 0)
		{
			player = players[i];
			break;
		}
	}


	player->FreeData();


	if (wcscmp(L"1", parts[0]) != 0 || IsEmpty(parts[1]) || (IsEmpty(parts[3]) && IsEmpty(parts[4])))
	{
		// Stoped playing or not enought info

//		if (wcscmp(L"1", parts[0]) != 0)
//			m_log(_T("ProcessReceived"), _T("END: Stoped playing"));
//		else
//			m_log(_T("ProcessReceived"), _T("ERROR: not enought info"));
	}
	else
	{
		LISTENINGTOINFO *li = player->LockListeningInfo();

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

		if (parts[9] != NULL)
		{
			long length = _wtoi(parts[9]);
			if (length > 0)
			{
				li->ptszLength = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));

				int s = length % 60;
				int m = (length / 60) % 60;
				int h = (length / 60) / 60;

				if (h > 0)
					mir_sntprintf(li->ptszLength, 9, _T("%d:%02d:%02d"), h, m, s);
				else
					mir_sntprintf(li->ptszLength, 9, _T("%d:%02d"), m, s);
			}
		}

		player->ReleaseListeningInfo();
	}

	// Put back the '\\'s
	for(int i = 1; i <= pCount; i++)
		*(parts[i] - 2) = L'\\';
	if (p1 != NULL)
		*p1 = L'\\';

	wcscpy(last_received, received);

	LeaveCriticalSection(&cs);

	NotifyInfoChanged();

//	m_log(_T("ProcessReceived"), _T("END: Success"));
}


static VOID CALLBACK SendTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(NULL, hTimer);
	hTimer = NULL;

	if (!loaded)
		return;

//	m_log(_T("SendTimerProc"), _T("It's time to process"));

	if (singleton != NULL)
		singleton->ProcessReceived();
}


void GenericPlayer::NewData(const WCHAR *data, size_t len)
{
//	m_log(_T("NewData"), _T("Processing"));

	if (data[0] == _T('\0'))
	{
//		m_log(_T("NewData"), _T("ERROR: Text is empty"));
		return;
	}

	EnterCriticalSection(&cs);

	len = min(len, 1023);
	if (wcsncmp(received, data, len) != 0)
	{
//		m_log(_T("NewData"), _T("Got new text, scheduling update"));

		wcsncpy(received, data, len);
		received[len] = L'\0';


//		m_log(_T("NewData"), _T("Text: %s"), received);


		if (hTimer)
			KillTimer(NULL, hTimer);
		hTimer = SetTimer(NULL, NULL, 300, SendTimerProc); // Do the processing after we return true
	}
//	else
//	{
//		m_log(_T("NewData"), _T("END: Text is the same as last time"));
//	}

	LeaveCriticalSection(&cs);
}


static LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COPYDATA:
		{
			if (!loaded)
				return FALSE;

//			m_log(_T("ReceiverWndProc"), _T("START: Received message"));

			COPYDATASTRUCT* pData = (PCOPYDATASTRUCT) lParam;
			if (pData == NULL || pData->dwData != MIRANDA_DW_PROTECTION 
					|| pData->cbData == 0 || pData->lpData == NULL)
			{
/*				if (pData == NULL)
					m_log(_T("ReceiverWndProc"), _T("ERROR: COPYDATASTRUCT* is NULL"));
				else if (pData->dwData != MIRANDA_DW_PROTECTION)
					m_log(_T("ReceiverWndProc"), _T("ERROR: pData->dwData is incorrect"));
				else if (pData->cbData == 0)
					m_log(_T("ReceiverWndProc"), _T("ERROR: pData->cbData is 0"));
				else if (pData->lpData == NULL)
					m_log(_T("ReceiverWndProc"), _T("ERROR: pData->lpData is NULL"));
*/
				return FALSE;
			}

//			m_log(_T("ReceiverWndProc"), _T("Going to process"));
			if (singleton != NULL)
				singleton->NewData((WCHAR *) pData->lpData, pData->cbData / 2);

			return TRUE;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default :
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
