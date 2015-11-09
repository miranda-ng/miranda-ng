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


#include <windows.h> 
#include <tchar.h> 
#include <stdio.h> 
#include <process.h>
#include "m_listeningto.h"

#include "wa_ipc.h" 
#include "GEN.h" 

// Plugin data //////////////////////////////////////////////////////////////////////////

int init(); 
void quit();
void config();

winampGeneralPurposePlugin plugin = {
	GPPHDR_VER,
	"Miranda ListeningTo Winamp Plugin", // Plug-in description 
	init,
	config, 
	quit,
	0,									// handle to Winamp main window, loaded by winamp when this dll is loaded
	0									// hinstance to this dll, loaded by winamp when this dll is loaded
}; 


// Globals //////////////////////////////////////////////////////////////////////////////


#define MIRANDA_DW_PROTECTION 0x8754

#define MESSAGE_WINDOWCLASS MIRANDA_WINDOWCLASS ".Winamp"

#define DATA_SIZE 1024

#define WA_STATE_CHANGE 0x0000029A

WNDPROC oldWndProc = NULL;
WNDPROC oldMainWndProc = NULL;
HMENU hMenuCreated = NULL;
HWND hMsgWnd = NULL;
HWND hPlWnd = NULL;
HINSTANCE hInst = NULL;

// Message window proc
LRESULT CALLBACK MsgWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

// Playlist window message processor
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);



// Functions ////////////////////////////////////////////////////////////////////////////


void WindowThread(void *param)
{
	// Create window
	WNDCLASS wc = {0};
	wc.lpfnWndProc		= MsgWndProc;
	wc.hInstance		= hInst;
	wc.lpszClassName	= MESSAGE_WINDOWCLASS;

	RegisterClass(&wc);

	hMsgWnd = CreateWindow(MESSAGE_WINDOWCLASS, _T("Miranda ListeningTo Winamp Plugin"), 
							0, 0, 0, 0, 0, NULL, NULL, hInst, NULL);

	if (hMsgWnd != NULL)
		if (FindWindow(MIRANDA_WINDOWCLASS, NULL) != NULL)
			SetTimer(hMsgWnd, 0, 5000, NULL);

	MSG msg;
	BOOL bRet; 
    while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
    { 
        if (bRet == -1)
        {
            // handle the error and possibly exit
			break;
        }
        else
        {
            TranslateMessage(&msg); 
            DispatchMessage(&msg); 
        }
    }

	_endthread();
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpvReserved) 
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		hInst = hInstDll;

		plugin.hwndParent = NULL;

		_beginthread(WindowThread, 0, NULL);
	}

	return TRUE;
}

// Winamp interface function
extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() 
{
	KillTimer(hMsgWnd, 0);

	return &plugin; 
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	// Find the windows
	char class_name[1024];
	if (GetClassName(hwnd, class_name, sizeof(class_name)))
	{
		class_name[sizeof(class_name)-1] = '\0';

		if (_strcmpi(MIRANDA_WINDOWCLASS, class_name) == 0) 
		{
			COPYDATASTRUCT *cds = (COPYDATASTRUCT *)lParam;
			SendMessage(hwnd, WM_COPYDATA, (WPARAM) plugin.hwndParent, (LPARAM) cds);
		}
	}

	return TRUE;
}

inline void SendData(WCHAR *text) 
{
	static WCHAR lastMsg[1024] = L"";

	if (wcscmp(lastMsg, text) == 0)
		return;

	// Prepare the struct
	COPYDATASTRUCT cds;
	cds.dwData = MIRANDA_DW_PROTECTION;
	cds.lpData = text;
	cds.cbData = (wcslen(text) + 1) * sizeof(WCHAR);

	EnumWindows(EnumWindowsProc, (LPARAM) &cds);

	wcsncpy(lastMsg, text, 1024);
	lastMsg[1023] = L'\0';
}


void Concat(WCHAR *data, size_t &size, char *str, size_t len = 0)
{
	if (size < 3 * sizeof(WCHAR))
		return;

	if (str != NULL)
	{
		if (len == 0)
			len = strlen(str);

		if (size >= len + 3)
		{
			MultiByteToWideChar(CP_ACP, 0, str, len  * sizeof(char), &data[DATA_SIZE - size], size * sizeof(WCHAR));
			size -= len;
			data[DATA_SIZE - size] = L'\0';
		}
	}

	wcscat(data, L"\\0");
	size -= 2;
}


void GetMetadata(extendedFileInfoStruct *efi, char *field, WCHAR *data, size_t &size)
{
	efi->ret[0] = '\0';
	efi->metadata = field;
	if (SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM) efi, IPC_GET_EXTENDED_FILE_INFO_HOOKABLE) && efi->ret[0] != '\0')
	{
		Concat(data, size, efi->ret);
	}
	else
	{
		Concat(data, size, NULL);
	}
}


void SendDataToMiranda(char *filename, char *title)
{
	extendedFileInfoStruct efi;
	char tmp[256];

	efi.ret = tmp;
	efi.retlen = sizeof(tmp);
	efi.filename = filename;

	WCHAR data[DATA_SIZE];
	size_t size = DATA_SIZE;
	data[0] = L'\0';

	// L"<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0\\0"
	Concat(data, size, "1");
	Concat(data, size, "Winamp");

	int version = SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETVERSION);
	BOOL is_radio = (strstr(filename, "://") != 0) && (strncmp(filename, "cda://", 6) != 0);

	if (is_radio)
		Concat(data, size, "Radio");
	else if (WINAMP_VERSION_MAJOR(version) >= 5 && SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_IS_PLAYING_VIDEO))
		Concat(data, size, "Video");
	else
		Concat(data, size, "Music");

	efi.ret[0] = '\0';
	efi.metadata = "TITLE";
	if (SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM) &efi, IPC_GET_EXTENDED_FILE_INFO_HOOKABLE) && efi.ret[0] != '\0')
	{
		Concat(data, size, efi.ret);
	}
	else if (title != NULL && title[0] != '\0' && strcmpi(title, filename) != 0)
	{
		Concat(data, size, title);
	}
	else
	{
		char *name = strrchr(filename, '\\');
		if (name == NULL)
			strrchr(filename, '/');

		if (name == NULL)
		{
			Concat(data, size, NULL);
		}
		else 
		{
			char *dot = strrchr(name, '.');
			Concat(data, size, name + 1, dot == NULL ? 0 : dot - name - 1);
		}
	}

	GetMetadata(&efi, "ARTIST", data, size);
	GetMetadata(&efi, "ALBUM", data, size);
	GetMetadata(&efi, "TRACK", data, size);
	GetMetadata(&efi, "YEAR", data, size);
	GetMetadata(&efi, "GENRE", data, size);

	efi.ret[0] = '\0';
	efi.metadata = "LENGTH";
	if (SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM) &efi, IPC_GET_EXTENDED_FILE_INFO_HOOKABLE)
		&& efi.ret[0] != '\0' && efi.ret[1] != '\0')
	{
		char tmp[10];
		itoa(atoi(efi.ret) / 1000, tmp, 10);
		Concat(data, size, tmp);
	}
	else
	{
		Concat(data, size, NULL);
	}
	Concat(data, size, NULL);

	SendData(data);
}


// Message window proc
LRESULT CALLBACK MsgWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL last_was_stop = TRUE;
	static DWORD last_notification = 0;
	static char last_filename[1024] = {0};

	switch(message)
	{
		case WM_TIMER:
		{
			KillTimer(hwnd, wParam);

			if (wParam == 0)
			{
				// Startup
				if (plugin.hwndParent == NULL)
				{
					plugin.hwndParent = FindWindow("Winamp v1.x", NULL);
					if (plugin.hwndParent != NULL) 
					{
						init();

						// If playing, show current song
						if (SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING) == 1)
							if (FindWindow(MIRANDA_WINDOWCLASS, NULL) != NULL)
								SetTimer(hMsgWnd, 1, 500, NULL);
					}
				}
			}
			else if (wParam == 1)
			{
				// Song change
				if (SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING) == 1)
				{
					int track = SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
					char *filename = (char *) SendMessage(plugin.hwndParent, WM_WA_IPC, track, IPC_GETPLAYLISTFILE);

					if (filename == NULL || filename[0] == '\0')
					{
						if (!last_was_stop)
						{
							last_was_stop = TRUE;

							if (FindWindow(MIRANDA_WINDOWCLASS, NULL) != NULL)
								SendData(L"0\\0Winamp\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0");
						}
					}
					else
					{
						BOOL is_radio = (strstr(filename, "://") != 0);
						if (last_was_stop || is_radio || strcmpi(last_filename, filename) != 0)
						{
							last_was_stop = FALSE;
							strncpy(last_filename, filename, sizeof(last_filename));
							last_filename[sizeof(last_filename) - 1] = '\0';

							// Miranda is running?
							if (FindWindow(MIRANDA_WINDOWCLASS, NULL) != NULL)
								SendDataToMiranda(last_filename, (char *) SendMessage(plugin.hwndParent, WM_WA_IPC, track, IPC_GETPLAYLISTTITLE));

							if (is_radio)
								// To try to get info from radio stations
								SetTimer(hMsgWnd, 1, 3000, NULL);
						}
					}
				}
				else
				{
					if (!last_was_stop)
					{
						last_was_stop = TRUE;

						if (FindWindow(MIRANDA_WINDOWCLASS, NULL) != NULL)
							SendData(L"0\\0Winamp\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0");
					}
				}
			}

			break;
		}
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}

// Playlist window message processor
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_USER:
		{
			if(wParam == WA_STATE_CHANGE)
			{
				int type = HIWORD(lParam);
				if(type == 0x4000 || type == 0)
				{
					KillTimer(hMsgWnd, 1);
					SetTimer(hMsgWnd, 1, 1000, NULL);
				}
			}
			break;
		}
	}

	return CallWindowProc(oldWndProc, hwnd, message, wParam, lParam);
}


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_COMMAND:
		{
			switch(wParam)
			{
				case 40045: // Play
				case 40046: // Pause
				{
					KillTimer(hMsgWnd, 1);
					SetTimer(hMsgWnd, 1, 500, NULL);
					break;
				}
			}
			break;
		}
		case WM_WA_IPC:
		{
			switch(lParam)
			{
				case IPC_PLAYING_FILE:
				{
					KillTimer(hMsgWnd, 1);
					SetTimer(hMsgWnd, 1, 500, NULL);
					break;
				}
			}
			break;
		}
		case WM_CLOSE:
		{
			PostMessage(hMsgWnd, WM_TIMER, 1, 0);
			PostMessage(hMsgWnd, WM_CLOSE, 0, 0);
			break;
		}
	}

	return CallWindowProc(oldMainWndProc, hwnd, message, wParam, lParam);
}


void quit() 
{
	if (oldMainWndProc != NULL)
		SetWindowLongPtr(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR) oldMainWndProc);

	if (oldWndProc != NULL)
		SetWindowLongPtr(hPlWnd, GWLP_WNDPROC, (LONG_PTR) oldWndProc);

	if (FindWindow(MIRANDA_WINDOWCLASS, NULL) != NULL)
		SendData(L"0\\0Winamp\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0");

} 


int init() 
{
	KillTimer(hMsgWnd, 0);

	oldMainWndProc = (WNDPROC)SetWindowLongPtr(plugin.hwndParent, GWLP_WNDPROC, (LONG_PTR) MainWndProc);

	hPlWnd = (HWND) SendMessage(plugin.hwndParent, WM_WA_IPC, IPC_GETWND_PE, IPC_GETWND);
	oldWndProc = (WNDPROC) SetWindowLongPtr(hPlWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);

	return 0; 
} 

void config() {
	MessageBox(NULL, _T("Copyright (C) 2006 Ricardo Pescuma Domenecci"), _T("Miranda ListeningTo Winamp Plugin"), 0);
}
