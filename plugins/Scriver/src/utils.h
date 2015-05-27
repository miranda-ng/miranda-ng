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

int IsUnicodeMIM();
TCHAR *limitText(TCHAR *text, int limit);
void logInfo(const char *fmt, ...);
int GetRichTextLength(HWND hwnd, int codepage, BOOL inBytes);
TCHAR *GetRichEditSelection(HWND hwnd);
char* GetRichTextRTF(HWND hwnd);
char* GetRichTextUtf(HWND hwnd);
TCHAR *GetRichTextWord(HWND hwnd, POINTL *pt);
int SetRichText(HWND hwnd, const TCHAR *text);
void SearchWord(TCHAR * word, int engine);
HDWP ResizeToolbar(HWND hwnd, HDWP hdwp, int width, int vPos, int height, int cControls, const ToolbarButton * buttons, int controlVisibility);
void ShowToolbarControls(HWND hwndDlg, int cControls, const ToolbarButton * buttons, int controlVisibility, int state);
void AppendToBuffer(char *&buffer, size_t &cbBufferEnd, size_t &cbBufferAlloced, const char *fmt, ...);
int MeasureMenuItem(WPARAM wParam, LPARAM lParam);
int DrawMenuItem(WPARAM wParam, LPARAM lParam);
void SetSearchEngineIcons(HMENU hMenu, HIMAGELIST hImageList);
void GetContactUniqueId(SrmmWindowData *dat, char *buf, int maxlen);
HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle, RECT *rect);
void SetToolTipText(HWND hwndParent, HWND hwndTT, LPTSTR ptszText, LPTSTR ptszTitle);
void SetToolTipRect(HWND hwndParent, HWND hwndTT, RECT* rect);
int GetToolbarWidth(int cControls, const ToolbarButton * buttons);
BOOL IsToolbarVisible(int cControls, int visibilityFlags);
#endif
