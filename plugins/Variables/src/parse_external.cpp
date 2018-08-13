/*
	Variables Plugin for Miranda-IM (www.miranda-im.org)
	Copyright 2003-2006 P. Boon

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

static wchar_t* getFullWinampTitleText()
{
	HWND hwndWinamp = FindWindow(L"STUDIO", nullptr);
	if (hwndWinamp == nullptr)
		hwndWinamp = FindWindow(L"Winamp v1.x", nullptr);

	if (hwndWinamp == nullptr)
		return nullptr;

	SIZE_T dwWinTextLength = (GetWindowTextLength(hwndWinamp) + 1);
	wchar_t *szWinText = (wchar_t*)mir_alloc(dwWinTextLength * sizeof(wchar_t));
	if (szWinText == nullptr)
		return nullptr;

	if (GetWindowText(hwndWinamp, szWinText, dwWinTextLength) == 0) {
		mir_free(szWinText);
		return nullptr;
	}
	wchar_t *szTitle = (wchar_t*)mir_alloc((2 * mir_wstrlen(szWinText) + 1) * sizeof(wchar_t));
	if (szTitle == nullptr) {
		mir_free(szWinText);
		return nullptr;
	}
	mir_wstrcpy(szTitle, szWinText);
	mir_wstrcpy(szTitle + mir_wstrlen(szTitle), szWinText);
	mir_free(szWinText);

	return szTitle;
}

static wchar_t* parseWinampSong(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return nullptr;

	wchar_t *res = nullptr;
	wchar_t *szTitle = getFullWinampTitleText();
	if (szTitle == nullptr)
		return nullptr;

	wchar_t *scur = wcschr(szTitle, '.');
	wchar_t *cur;
	if ((scur == nullptr) || ((cur = wcsstr(scur, L" - Winamp")) == nullptr) || (scur >= cur) || (scur > (szTitle + mir_wstrlen(szTitle) - 2)) || (cur > (szTitle + mir_wstrlen(szTitle)))) {
		mir_free(szTitle);
		return nullptr;
	}
	scur++;
	scur++;
	*cur = '\0';
	res = mir_wstrdup(scur);
	mir_free(szTitle);
	ai->flags |= AIF_DONTPARSE;

	return res;
}

static wchar_t* parseWinampState(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return nullptr;

	wchar_t *res = nullptr;
	wchar_t *szTitle = getFullWinampTitleText();
	if (szTitle == nullptr)
		return nullptr;

	wchar_t *scur = wcschr(szTitle, '.');
	wchar_t *cur;
	if (scur == nullptr || (cur = wcsstr(scur, L" - Winamp")) == nullptr)
		res = mir_wstrdup(TranslateT("Stopped"));
	else if ((!wcsncmp(cur + 10, L"[Stopped]", 9)))
		res = mir_wstrdup(TranslateT("Stopped"));
	else if ((!wcsncmp(cur + 10, L"[Paused]", 8)))
		res = mir_wstrdup(TranslateT("Paused"));
	else
		res = mir_wstrdup(L"Playing");
	mir_free(szTitle);
	return res;
}

void registerExternalTokens()
{
	registerIntToken(WINAMPSONG, parseWinampSong, TRF_FIELD, LPGEN("External Applications") "\t" LPGEN("retrieves song name of the song currently playing in Winamp"));
	registerIntToken(WINAMPSTATE, parseWinampState, TRF_FIELD, LPGEN("External Applications") "\t" LPGEN("retrieves current Winamp state (Playing/Paused/Stopped)"));
}
