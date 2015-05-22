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

#include "variables.h"

static TCHAR *getFullWinampTitleText()
{
	HWND hwndWinamp = FindWindow(_T("STUDIO"), NULL);
	if (hwndWinamp == NULL)
		hwndWinamp = FindWindow(_T("Winamp v1.x"), NULL);

	if (hwndWinamp == NULL)
		return NULL;

	SIZE_T dwWinTextLength = (GetWindowTextLength(hwndWinamp) + 1);
	TCHAR *szWinText = (TCHAR*)mir_alloc(dwWinTextLength * sizeof(TCHAR));
	if (szWinText == NULL)
		return NULL;

	if (GetWindowText(hwndWinamp, szWinText, dwWinTextLength) == 0) {
		mir_free(szWinText);
		return NULL;
	}
	TCHAR *szTitle = (TCHAR*)mir_alloc((2 * mir_tstrlen(szWinText) + 1)*sizeof(TCHAR));
	if (szTitle == NULL) {
		mir_free(szWinText);
		return NULL;
	}
	_tcscpy(szTitle, szWinText);
	_tcscpy(szTitle + mir_tstrlen(szTitle), szWinText);
	mir_free(szWinText);

	return szTitle;
}

static TCHAR *parseWinampSong(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return NULL;

	TCHAR *res = NULL;
	TCHAR *szTitle = getFullWinampTitleText();
	if (szTitle == NULL)
		return NULL;

	TCHAR *scur = _tcschr(szTitle, '.');
	TCHAR *cur;
	if ((scur == NULL) || ((cur = _tcsstr(scur, _T(" - Winamp"))) == NULL) || (scur >= cur) || (scur > (szTitle + mir_tstrlen(szTitle) - 2)) || (cur > (szTitle + mir_tstrlen(szTitle)))) {
		mir_free(szTitle);
		return NULL;
	}
	scur++;
	scur++;
	*cur = '\0';
	res = mir_tstrdup(scur);
	mir_free(szTitle);
	ai->flags |= AIF_DONTPARSE;

	return res;
}

static TCHAR *parseWinampState(ARGUMENTSINFO *ai)
{
	if (ai->argc != 1)
		return NULL;

	TCHAR *res = NULL;
	TCHAR *szTitle = getFullWinampTitleText();
	if (szTitle == NULL)
		return NULL;

	TCHAR *scur = _tcschr(szTitle, '.');
	TCHAR *cur;
	if (scur == NULL || (cur = _tcsstr(scur, _T(" - Winamp"))) == NULL)
		res = mir_tstrdup(TranslateT("Stopped"));
	else if ((!_tcsncmp(cur + 10, _T("[Stopped]"), 9)))
		res = mir_tstrdup(TranslateT("Stopped"));
	else if ((!_tcsncmp(cur + 10, _T("[Paused]"), 8)))
		res = mir_tstrdup(TranslateT("Paused"));
	else
		res = mir_tstrdup(_T("Playing"));
	mir_free(szTitle);
	return res;
}

void registerExternalTokens()
{
	registerIntToken(WINAMPSONG, parseWinampSong, TRF_FIELD, LPGEN("External Applications")"\t"LPGEN("retrieves song name of the song currently playing in Winamp"));
	registerIntToken(WINAMPSTATE, parseWinampState, TRF_FIELD, LPGEN("External Applications")"\t"LPGEN("retrieves current Winamp state (Playing/Paused/Stopped)"));
}