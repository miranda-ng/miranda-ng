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

#define WMP_WINDOWCLASS _T("MsnMsgrUIManager")

static LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


static UINT hTimer = NULL;


WindowsMediaPlayer *singleton = NULL;



WindowsMediaPlayer::WindowsMediaPlayer()
{
	name = _T("WindowsMediaPlayer");
	received[0] = _T('\0');
	singleton = this;

	WNDCLASS wc = {0};
	wc.lpfnWndProc		= ReceiverWndProc;
	wc.hInstance		= hInst;
	wc.lpszClassName	= WMP_WINDOWCLASS;

	RegisterClass(&wc);

	hWnd = CreateWindow(WMP_WINDOWCLASS, LPGENT("Miranda ListeningTo WMP receiver"), 
							0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);
}



WindowsMediaPlayer::~WindowsMediaPlayer()
{
	if (hTimer != NULL)
	{
		KillTimer(NULL, hTimer);
		hTimer = NULL;
	}

	DestroyWindow(hWnd);
	hWnd = NULL;

	UnregisterClass(WMP_WINDOWCLASS, hInst);
	singleton = NULL;
}



void WindowsMediaPlayer::ProcessReceived()
{
	EnterCriticalSection(&cs);

	FreeData();

	// Do the processing
	// MSNMusicString = L"\\0Music\\0%d\\0%s\\0%s\\0%s\\0%s\\0%s\\0\\0"
	// MSNMusicString, msn->msncommand, strMSNFormat, msn->title, msn->artist, msn->album, msn->wmcontentid);

	WCHAR *p1 = wcsstr(received, L"\\0");

	if (received[0] == L'\0' || p1 == NULL)
	{
		LeaveCriticalSection(&cs);
		NotifyInfoChanged();
		return;
	}

	// Process string
	WCHAR *parts[8] = {0};
	int pCount = 0;
	WCHAR *p = received;
	do {
		*p1 = L'\0';
		parts[pCount] = p;
		pCount ++;
		p = p1 + 2;
		p1 = wcsstr(p, L"\\0");
	} while( p1 != NULL && pCount < 7 );
	if (p1 != NULL)
		*p1 = L'\0';
	parts[pCount] = p;

	// Fill cache
	if (pCount > 4 && !IsEmpty(parts[1]) && (!IsEmpty(parts[4]) || !IsEmpty(parts[5])))
	{
		listening_info.cbSize = sizeof(listening_info);
		listening_info.dwFlags = LTI_TCHAR;

		listening_info.ptszType = U2T(parts[1]);
		listening_info.ptszTitle = U2T(parts[4]);
		listening_info.ptszArtist = U2T(parts[5]);
		listening_info.ptszAlbum = U2T(parts[6]);

		listening_info.ptszPlayer = mir_tstrdup(name);
	}

	// Put back the '\\'s
	for(int i = 1; i <= pCount; i++)
		*(parts[i] - 2) = L'\\';
	if (p1 != NULL)
		*p1 = L'\\';

	LeaveCriticalSection(&cs);

	NotifyInfoChanged();
}



static VOID CALLBACK SendTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(NULL, hTimer);
	hTimer = NULL;

	if (!loaded)
		return;

	if (singleton != NULL)
		singleton->ProcessReceived();
}


void WindowsMediaPlayer::NewData(const WCHAR *data, size_t len)
{
	EnterCriticalSection(&cs);

	len = min(len, 1023);
	if (wcsncmp(received, data, len) != 0)
	{
		wcsncpy(received, data, len);
		received[len] = _T('\0');

/*
		m_log(_T("ReceiverWndProc"), _T("WMP : New data: [%d] %s"), len, received);

*/
		if (hTimer)
			KillTimer(NULL, hTimer);
		hTimer = SetTimer(NULL, NULL, 300, SendTimerProc); // Do the processing after we return true
	}
/*	else
	{
		m_log(_T("NewData"), _T("END: Text is the same as last time"));
	}
*/
	LeaveCriticalSection(&cs);
}



static LRESULT CALLBACK ReceiverWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_COPYDATA :
		{
			if (!loaded)
				return FALSE;

			if (singleton == NULL || !singleton->enabled)
				return FALSE;

			COPYDATASTRUCT* pData = (PCOPYDATASTRUCT) lParam;
			if (pData->dwData != 0x547 || pData->cbData == 0 || pData->lpData == NULL)
				return FALSE;

			if (singleton != NULL)
				singleton->NewData((WCHAR *) pData->lpData, pData->cbData / 2);

			return TRUE;
		}
		case WM_DESTROY :
			PostQuitMessage(0);
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}