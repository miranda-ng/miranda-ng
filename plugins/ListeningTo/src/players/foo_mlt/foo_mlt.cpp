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

#include "foobar2000/SDK/foobar2000.h"
#include "foobar2000/helpers/helpers.h"
#include "m_listeningto.h"
#include <windows.h>
#include <process.h>


using namespace pfc;


// Globals //////////////////////////////////////////////////////////////////////////////


#define MIRANDA_DW_PROTECTION 0x8754

#define DATA_SIZE 1024

UINT timer = 0;
WCHAR lastSongData[DATA_SIZE] = L"";

static bool g_off = true;		//global state for sending listeningto infos


// Functions ////////////////////////////////////////////////////////////////////////////


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	// Find the windows
	TCHAR class_name[256];
	if (GetClassName(hwnd, class_name, 256))
	{
		class_name[255] = _T('\0');

		if (lstrcmpi(MIRANDA_WINDOWCLASS, class_name) == 0)
		{
			COPYDATASTRUCT *cds = (COPYDATASTRUCT *) lParam;
			SendMessage(hwnd, WM_COPYDATA, (WPARAM) NULL, (LPARAM) cds);
		}
	}

	return TRUE;
}

inline void SendData(WCHAR *text)
{
	static WCHAR lastMsg[DATA_SIZE] = L"";

	if (wcscmp(lastMsg, text) == 0)
		return;

	// Prepare the struct
	COPYDATASTRUCT cds;
	cds.dwData = MIRANDA_DW_PROTECTION;
	cds.lpData = text;
	cds.cbData = (wcslen(text) + 1) * sizeof(WCHAR);

	EnumWindows(EnumWindowsProc, (LPARAM) &cds);

	wcsncpy(lastMsg, text, DATA_SIZE);
	lastMsg[DATA_SIZE-1] = L'\0';
}


void Concat(WCHAR *data, size_t &size, const char *str, size_t len = 0)
{
	if (size < 3 * sizeof(WCHAR))
		return;

	if (str != NULL)
	{
		if (len == 0)
			len = strlen(str);

		if (size >= len + 3)
		{
			size -= MultiByteToWideChar(CP_UTF8, 0, str, len  * sizeof(char), &data[DATA_SIZE - size], size * sizeof(WCHAR));
			data[DATA_SIZE - size] = L'\0';
		}
	}

	wcscat(data, L"\\0");
	size -= 2;
}


void Concat(WCHAR *data, size_t &size)
{
	if (size < 3 * sizeof(WCHAR))
		return;

	wcscat(data, L"\\0");
	size -= 2;
}


void Concat(WCHAR *data, size_t &size, const WCHAR *str, size_t len = 0)
{
	if (size < 3 * sizeof(WCHAR))
		return;

	if (str != NULL)
	{
		if (len == 0)
			len = wcslen(str);

		if (size >= len + 3)
		{
			wcscpy(&data[DATA_SIZE - size], str);
			size -= len;
			data[DATA_SIZE - size] = L'\0';
		}
	}

	wcscat(data, L"\\0");
	size -= 2;
}


void GetMetadata(const file_info *info, char *field, WCHAR *data, size_t &size)
{
	const char *val = info->meta_get(field, 0);
	if (val != NULL && val[0] != '\0')
	{
		Concat(data, size, val);
	}
	else
	{
		Concat(data, size);
	}
}


void KillTimer(UINT id = 0)
{
	if (id != 0)
	{
		KillTimer(NULL, id);
	}
	if (timer != 0)
	{
		if (timer != id)
			KillTimer(NULL, timer);
		timer = 0;
	}
}


void CALLBACK SendEmptyData(HWND hWnd = 0, UINT nMsg = 0, UINT nIDEvent = 0, DWORD dwTime = 0)
{
	KillTimer(nIDEvent);

	// L"<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0\\0"
	SendData(L"0\\0foobar2000\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0");
}


void SetTimer()
{
	KillTimer();
	timer = SetTimer(NULL, 1, 1000, SendEmptyData);
}


BOOL IsRadio(metadb_handle_ptr p_track)
{
	const char *filename = p_track->get_path();
	return (filename != NULL && strstr(filename, "://") != 0 && strncmp(filename, "file://", 7) != 0);
}


void SendDataMusic(const char *filename, const file_info *info)
{
	WCHAR data[DATA_SIZE];
	size_t size = DATA_SIZE;
	data[0] = L'\0';

	// L"<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0\\0"
	Concat(data, size, "1");
	Concat(data, size, "foobar2000");
	Concat(data, size, "Music");

	const char *val = info->meta_get("TITLE", 0);
	if (val != NULL && val[0] != '\0')
	{
		Concat(data, size, val);
	}
	else if (filename != NULL && filename[0] != '\0')
	{
		const char *name = strrchr(filename, '\\');
		if (name == NULL)
			strrchr(filename, '/');

		if (name == NULL)
		{
			Concat(data, size);
		}
		else
		{
			const char *dot = strrchr(name, '.');
			Concat(data, size, name + 1, dot == NULL ? 0 : dot - name - 1);
		}
	}
	else
	{
		Concat(data, size);
	}

	GetMetadata(info, "ARTIST", data, size);
	GetMetadata(info, "ALBUM", data, size);
	GetMetadata(info, "TRACKNUMBER", data, size);
	GetMetadata(info, "DATE", data, size);
	GetMetadata(info, "GENRE", data, size);

	int len = (int) info->get_length();
	if (len > 0)
	{
		char tmp[10];
		Concat(data, size, itoa(len, tmp, 10));
	}
	else
	{
		Concat(data, size);
	}

	Concat(data, size);

	SendData(data);
	wcsncpy(lastSongData, data, DATA_SIZE);
	lastSongData[DATA_SIZE-1] = L'\0';
}

void SendDataRadio(const file_info *info, const file_info *info2)
{
	WCHAR data[DATA_SIZE];
	size_t size = DATA_SIZE;
	data[0] = L'\0';

	// L"<Status 0-stoped 1-playing>\\0<Player>\\0<Type>\\0<Title>\\0<Artist>\\0<Album>\\0<Track>\\0<Year>\\0<Genre>\\0<Length (secs)>\\0<Station name>\\0"
	Concat(data, size, "1");
	Concat(data, size, "foobar2000");
	Concat(data, size, "Radio");

	GetMetadata(info, "TITLE", data, size);
	GetMetadata(info, "ARTIST", data, size);
	GetMetadata(info, "ALBUM", data, size);
	GetMetadata(info, "TRACKNUMBER", data, size);
	GetMetadata(info, "DATE", data, size);
	GetMetadata(info2, "GENRE", data, size);

	int len = (int) info->get_length();
	if (len > 0)
	{
		char tmp[10];
		Concat(data, size, itoa(len, tmp, 10));
	}
	else
	{
		Concat(data, size);
	}

	// Station name
	GetMetadata(info2, "TITLE", data, size);

	SendData(data);
	wcsncpy(lastSongData, data, DATA_SIZE);
	lastSongData[DATA_SIZE-1] = L'\0';
}


// Foobar ////////////////////////////////////////////////////////////////////////////

class play_callback_miranda : public play_callback_static
{
	virtual void on_playback_starting(play_control::t_track_command p_command, bool p_paused) {}
	virtual void on_playback_new_track(metadb_handle_ptr p_track)
	{
		if (g_off) return;

		KillTimer();
		if (IsRadio(p_track))
			return;

		in_metadb_sync_fromhandle l_sync(p_track);

		const file_info *info;
		if (p_track->get_info_locked(info))
			SendDataMusic(p_track->get_path(), info);
	}
	virtual void on_playback_stop(play_control::t_stop_reason p_reason)
	{
		if (g_off) return;
		SetTimer();
	}
	virtual void on_playback_seek(double p_time) {}
	virtual void on_playback_pause(bool p_state)
	{
		if (g_off) return;
		if (p_state)
		{
			SetTimer();
		}
		else
		{
			KillTimer();
			if (lastSongData[0] != L'\0')
				SendData(lastSongData);
		}
	}
	virtual void on_playback_edited(metadb_handle_ptr p_track) {}
	virtual void on_playback_dynamic_info(const file_info & info) {}
	virtual void on_playback_dynamic_info_track(const file_info & info)
	{
		if (g_off) return;
		metadb_handle_ptr p_track;
		static_api_ptr_t<play_control>()->get_now_playing(p_track);
		if (p_track.is_valid())
		{
			if (IsRadio(p_track))
			{
				in_metadb_sync_fromhandle l_sync(p_track);

				const file_info *info2;
				if (!p_track->get_info_locked(info2))
					return;

				SendDataRadio(&info, info2);
			}
			p_track.release();
		}
	}
	virtual void on_playback_time(double p_time)  {}
	virtual void on_volume_change(float p_new_val) {};

	virtual unsigned get_flags()
	{
		return flag_on_playback_new_track | flag_on_playback_pause | flag_on_playback_stop | flag_on_playback_dynamic_info_track;
	}
};


static play_callback_static_factory_t<play_callback_miranda> miranda_callback_factory;

class myinitquit : public initquit {
public:
	void on_init()
	{
		//check if foo_comserver2 is present and set g_off to false if foo_mlt go active
		//TODO:detect foo_comserver2 from component list (can also check for other plugins)
		CLSID clsid;
		if(S_OK != CLSIDFromProgID(L"Foobar2000.Application.0.7", &clsid)) {
			g_off = false;
			SetTimer();
		}
	}
	void on_quit()
	{
		if (!g_off && FindWindow(MIRANDA_WINDOWCLASS, NULL) != NULL)
			SendData(L"0\\0foobar2000\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0");
	}
};

static initquit_factory_t<myinitquit> g_myinitquit_factory;

DECLARE_COMPONENT_VERSION("Miranda ListeningTo foobar2000 Plugin",
"1.1.1",
"compiled with foo_SDK-2010-10-02\r\n\
Sending listeningto information to Mitanda IM client\r\n\
if no foo_comserver2 is present.\r\n\
Copyright (C) 2006-2010 Ricardo Pescuma Domenecci\r\n\
http://www.miranda-im.org\r\n\
http://pescuma.org/miranda/listeningto"
)
