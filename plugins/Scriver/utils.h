/*
Scriver

Copyright 2000-2009 Miranda ICQ/IM project,

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
	SEARCHENGINE_YAHOO = 3,
	SEARCHENGINE_WIKIPEDIA = 4,
	SEARCHENGINE_GOOGLE_MAPS = 5,
	SEARCHENGINE_GOOGLE_TRANSLATE = 6,
	SEARCHENGINE_FOODNETWORK=7
};

HANDLE HookEvent_Ex(const char *name, MIRANDAHOOK hook);
HANDLE CreateServiceFunction_Ex(const char *name, MIRANDASERVICE service);
extern void UnhookEvents_Ex();
extern void DestroyServices_Ex();
extern int IsUnicodeMIM();
extern int safe_wcslen(wchar_t *msg, int maxLen) ;
extern TCHAR *a2t(const char *text);
extern TCHAR *a2tcp(const char *text, int cp);
extern char* t2a(const TCHAR* src);
extern char* t2acp(const TCHAR* src, int cp);
extern char* u2a( const wchar_t* src, int codepage );
extern wchar_t* a2u( const char* src, int codepage );
extern wchar_t *a2w(const char *src, int len);
extern TCHAR *limitText(TCHAR *text, int limit);
extern void logInfo(const char *fmt, ...);
extern int GetRichTextLength(HWND hwnd, int codepage, BOOL inBytes);
extern TCHAR* GetRichText(HWND hwnd, int codepage);
extern TCHAR *GetRichEditSelection(HWND hwnd);
extern char* GetRichTextRTF(HWND hwnd);
extern char* GetRichTextEncoded(HWND hwnd, int codepage);
extern TCHAR *GetRichTextWord(HWND hwnd, POINTL *pt);
extern int SetRichTextRTF(HWND hwnd, const char *text);
extern int SetRichTextEncoded(HWND hwnd, const char *text, int codepage);
extern void SearchWord(TCHAR * word, int engine);
extern HDWP ResizeToolbar(HWND hwnd, HDWP hdwp, int width, int vPos, int height, int cControls, const ToolbarButton * buttons, int controlVisibility);
extern void ShowToolbarControls(HWND hwndDlg, int cControls, const ToolbarButton * buttons, int controlVisibility, int state);
extern void AppendToBuffer(char **buffer, int *cbBufferEnd, int *cbBufferAlloced, const char *fmt, ...);
extern int MeasureMenuItem(WPARAM wParam, LPARAM lParam);
extern int DrawMenuItem(WPARAM wParam, LPARAM lParam);
extern void SetSearchEngineIcons(HMENU hMenu, HIMAGELIST hImageList);
extern void GetContactUniqueId(struct MessageWindowData *dat, char *buf, int maxlen);
HWND CreateToolTip(HWND hwndParent, LPTSTR ptszText, LPTSTR ptszTitle, RECT *rect);
void SetToolTipText(HWND hwndParent, HWND hwndTT, LPTSTR ptszText, LPTSTR ptszTitle);
void SetToolTipRect(HWND hwndParent, HWND hwndTT, RECT* rect);
int GetToolbarWidth(int cControls, const ToolbarButton * buttons);
BOOL IsToolbarVisible(int cControls, int visibilityFlags);
#endif
