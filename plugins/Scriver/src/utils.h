/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef UTILS_H
#define UTILS_H

enum SEARCHENGINES {
	SEARCHENGINE_GOOGLE = 1,
	SEARCHENGINE_BING = 2,
	SEARCHENGINE_YANDEX = 3,
	SEARCHENGINE_WIKIPEDIA = 4,
	SEARCHENGINE_GOOGLE_MAPS = 5,
	SEARCHENGINE_GOOGLE_TRANSLATE = 6,
	SEARCHENGINE_YAHOO = 7,
	SEARCHENGINE_FOODNETWORK = 8
};

wchar_t *limitText(wchar_t *text, int limit);
void logInfo(const char *fmt, ...);
void SearchWord(wchar_t * word, int engine);

HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle, RECT *rect);
void SetToolTipText(HWND hwndParent, HWND hwndTT, LPTSTR ptszText, LPTSTR ptszTitle);
void SetToolTipRect(HWND hwndParent, HWND hwndTT, RECT* rect);
void SetButtonsPos(HWND hwndDlg, MCONTACT hContact, bool bShow);
#endif
