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

static WATrack *instance = NULL;

int NewStatusCallback(WPARAM wParam, LPARAM lParam) 
{
	if (!loaded)
		return 0;
	if (instance != NULL)
		instance->NewStatus(wParam, lParam);
	return 0;
}


WATrack::WATrack()
{
	name = _T("WATrack");
	instance = this;
	hNewStatusHook = NULL;
}



WATrack::~WATrack()
{
	if (hNewStatusHook != NULL) 
	{
		UnhookEvent(hNewStatusHook);
		hNewStatusHook = NULL;
	}
	instance = NULL;
}


void WATrack::EnableDisable()
{
	if (!ServiceExists(MS_WAT_GETMUSICINFO))
	{
		enabled = FALSE;
		return;
	}

	if (hNewStatusHook == NULL)
		hNewStatusHook = HookEvent(ME_WAT_NEWSTATUS, NewStatusCallback);
}


void WATrack::NewStatus(int event, int value)
{
	EnterCriticalSection(&cs);

	if (event == WAT_EVENT_PLUGINSTATUS && value != 0)
	{
		FreeData();
	}
	else
	{
		GetData();
	}

	LeaveCriticalSection(&cs);

	NotifyInfoChanged();
}


void WATrack::GetData()
{


	SONGINFO *si = NULL;

	int playing = CallService(MS_WAT_GETMUSICINFO, WAT_INF_UNICODE, (LPARAM) &si);



	FreeData();

	// See if something is playing
	if (playing ==  WAT_RES_NOTFOUND
		|| si == NULL
		|| si->status != 1
		|| ( IsEmpty(si->artist) && IsEmpty(si->title)) )
	{
		return;
	}

	// Copy new data

	listening_info.ptszAlbum = DUP(si->album);
	listening_info.ptszArtist = DUP(si->artist);
	listening_info.ptszTitle = DUP(si->title);
	listening_info.ptszYear = DUP(si->year);

	if (si->track > 0)
	{
		listening_info.ptszTrack = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));
		_itot(si->track, listening_info.ptszTrack, 10);
	}

	listening_info.ptszGenre = DUP(si->genre);

	if (si->total > 0)
	{
		listening_info.ptszLength = (TCHAR*) mir_alloc(10 * sizeof(TCHAR));

		int s = si->total % 60;
		int m = (si->total / 60) % 60;
		int h = (si->total / 60) / 60;

		if (h > 0)
			mir_sntprintf(listening_info.ptszLength, 9, _T("%d:%02d:%02d"), h, m, s);
		else
			mir_sntprintf(listening_info.ptszLength, 9, _T("%d:%02d"), m, s);
	}

	if (si->width > 0)
		listening_info.ptszType = mir_tstrdup(_T("Video"));
	else
		listening_info.ptszType = mir_tstrdup(_T("Music"));

	listening_info.ptszPlayer = DUPD(si->player, name);

	listening_info.cbSize = sizeof(listening_info);
	listening_info.dwFlags = LTI_TCHAR;
}
