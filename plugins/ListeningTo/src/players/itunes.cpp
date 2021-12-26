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
extern "C"
{
#include "iTunesCOMInterface_i.c"
}

ITunes::ITunes()
{
	name = L"iTunes";
	needPoll = TRUE;

	filename[0] = L'\0';

	hwnd = nullptr;
	iTunesApp = nullptr;
	track = nullptr;
	file = nullptr;
	ret = nullptr;
}

void ITunes::FindWindow()
{
	hwnd = ::FindWindow(L"iTunes", L"iTunes");
}

void ITunes::FreeTempData()
{
#define RELEASE(_x_) if (_x_ != NULL) { _x_->Release(); _x_ = NULL; }

	RELEASE(file);
	RELEASE(track);
	RELEASE(iTunesApp);

	if (ret != nullptr) {
		SysFreeString(ret);
		ret = nullptr;
	}
}

#define CALL(_F_) hr = _F_; if (FAILED(hr)) return FALSE

// Init data and put filename playing in ret and ->fi.filename
BOOL ITunes::InitAndGetFilename()
{
	HRESULT hr;

	// Find window
	FindWindow();
	if (hwnd == nullptr)
		return FALSE;

	CALL(CoCreateInstance(CLSID_iTunesApp, nullptr, CLSCTX_LOCAL_SERVER, __uuidof(iTunesApp), (void **)&iTunesApp));

	ITPlayerState state;
	CALL(iTunesApp->get_PlayerState(&state));
	if (state == ITPlayerStateStopped)
		return FALSE;

	CALL(iTunesApp->get_CurrentTrack(&track));
	if (track == nullptr)
		return FALSE;

	CALL(track->QueryInterface(__uuidof(file), (void **)&file));

	CALL(file->get_Location(&ret));

	return !IsEmpty(ret);
}

BOOL ITunes::FillCache()
{
	HRESULT hr;
	long lret;

	CALL(track->get_Album(&ret));
	listening_info.ptszAlbum = U2T(ret);

	CALL(track->get_Artist(&ret));
	listening_info.ptszArtist = U2T(ret);

	CALL(track->get_Name(&ret));
	listening_info.ptszTitle = U2T(ret);

	CALL(track->get_Year(&lret));
	if (lret > 0) {
		listening_info.ptszYear = (wchar_t*)mir_alloc(10 * sizeof(wchar_t));
		_itow(lret, listening_info.ptszYear, 10);
	}

	CALL(track->get_TrackNumber(&lret));
	if (lret > 0) {
		listening_info.ptszTrack = (wchar_t*)mir_alloc(10 * sizeof(wchar_t));
		_itow(lret, listening_info.ptszTrack, 10);
	}

	CALL(track->get_Genre(&ret));
	listening_info.ptszGenre = U2T(ret);

	CALL(track->get_Duration(&lret));
	if (lret > 0) {
		listening_info.ptszLength = (wchar_t*)mir_alloc(10 * sizeof(wchar_t));

		int s = lret % 60;
		int m = (lret / 60) % 60;
		int h = (lret / 60) / 60;

		if (h > 0)
			mir_snwprintf(listening_info.ptszLength, 9, L"%d:%02d:%02d", h, m, s);
		else
			mir_snwprintf(listening_info.ptszLength, 9, L"%d:%02d", m, s);
	}

	listening_info.ptszType = mir_wstrdup(L"Music");

	if (listening_info.ptszTitle == nullptr) {
		// Get from filename
		wchar_t *p = wcsrchr(filename, '\\');
		if (p != nullptr)
			p++;
		else
			p = filename;

		listening_info.ptszTitle = mir_wstrdup(p);

		wchar_t *pt = wcsrchr(listening_info.ptszTitle, '.');
		if (pt != nullptr)
			*p = '\0';
	}

	listening_info.ptszPlayer = mir_wstrdup(name);

	listening_info.cbSize = sizeof(listening_info);
	listening_info.dwFlags = LTI_TCHAR;

	return TRUE;
}

BOOL ITunes::GetListeningInfo(LISTENINGTOINFO *lti)
{
	FreeData();

	if (InitAndGetFilename() && lstrcmp(filename, ret) != 0) {
		// Fill the data cache
		wcscpy(filename, ret);

		if (!FillCache())
			FreeData();
	}

	FreeTempData();

	return Player::GetListeningInfo(lti);
}
