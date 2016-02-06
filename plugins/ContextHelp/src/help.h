/*
Miranda IM Help Plugin
Copyright (C) 2002 Richard Hughes, 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Help-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef __CONTEXTHELP_HELP_H__
#define __CONTEXTHELP_HELP_H__

// dlgboxsubclass.c
int InstallDialogBoxHook(void);
int RemoveDialogBoxHook(void);

// utils.c
#define CTLTYPE_UNKNOWN    0
#define CTLTYPE_DIALOG     1
#define CTLTYPE_BUTTON     2
#define CTLTYPE_CHECKBOX   3
#define CTLTYPE_RADIO      4
#define CTLTYPE_TEXT       5
#define CTLTYPE_IMAGE      6
#define CTLTYPE_EDIT       7
#define CTLTYPE_GROUP      8
#define CTLTYPE_COMBO      9
#define CTLTYPE_LIST       10
#define CTLTYPE_SPINEDIT   11
#define CTLTYPE_PROGRESS   12
#define CTLTYPE_SLIDER     13
#define CTLTYPE_LISTVIEW   14
#define CTLTYPE_TREEVIEW   15
#define CTLTYPE_DATETIME   16
#define CTLTYPE_IP         17
#define CTLTYPE_STATUSBAR  18
#define CTLTYPE_HYPERLINK  19
#define CTLTYPE_CLC        20
#define CTLTYPE_SCROLL     21
#define CTLTYPE_ANIMATION  22
#define CTLTYPE_HOTKEY     23
#define CTLTYPE_TABS       24
#define CTLTYPE_COLOUR     25
#define CTLTYPE_TOOLBAR    26
#define CTLTYPE_SIZEGRIP   27

extern const TCHAR *szControlTypeNames[];

int GetControlType(HWND hwndCtl);
HWND GetControlDialog(HWND hwndCtl);
int GetControlTitle(HWND hwndCtl, TCHAR *pszTitle, int cchTitle);
char *GetControlModuleName(HWND hwndCtl);
int GetControlID(HWND hwndCtl);
char *CreateDialogIdString(HWND hwndDlg);

struct ResizableCharBuffer {
	char *sz;
	int iEnd, cbAlloced;
};

void AppendCharToCharBuffer(struct ResizableCharBuffer *rcb, char c);
void AppendToCharBuffer(struct ResizableCharBuffer *rcb, const char *fmt, ...);

// helpdlg.c
#define M_CHANGEHELPCONTROL (WM_APP+0x100)
#define M_HELPLOADED        (WM_APP+0x101)
#ifdef EDITOR
#define M_SAVECOMPLETE      (WM_APP+0x102)
#endif
#define M_LOADHELP          (WM_APP+0x103)
#define M_HELPLOADFAILED    (WM_APP+0x104)
#define M_CLIPBOARDCOPY     (WM_APP+0x105)
INT_PTR CALLBACK HelpDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ShadowDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// streaminout.c
void StreamInHtml(HWND hwndEdit, const char *szHtml, UINT codepage, COLORREF clrBkgrnd);
#ifdef EDITOR
char *StreamOutHtml(HWND hwndEdit);
#endif
#define TEXTSIZE_BIG      18  // in half points
#define TEXTSIZE_NORMAL   16
#define TEXTSIZE_SMALL    13
#ifndef EDITOR
void FreeHyperlinkData(void);
int IsHyperlink(LONG cpPos, LONG *pcpMin, LONG *pcpMax, char **ppszLink);
#endif

// datastore.c
void InitDialogCache(void);
void FreeDialogCache(void);
#define GCHF_DONTLOAD     1
int GetControlHelp(HWND hwndCtl, const char *pszDlgId, const char *pszModule, int ctrlId, TCHAR **ppszTitle, char **ppszText, int *pType, LCID *pLocaleID, UINT *pCodePage, BOOL *pIsRTL, DWORD flags);
#ifdef EDITOR
void SetControlHelp(const char *pszDlgId, const char *pszModule, int ctrlId, TCHAR *pszTitle, char *pszText, int type);
void SaveDialogCache(void);
#else
char *CreateControlIdentifier(const char *pszDlgId, const char *pszModule, int ctrlId, HWND hwndCtl);
#endif
void RegisterFileChange(void);
void CloseFileChange(void);

// options.c
void ReloadLangOptList(void);
void InitOptions(void);
void UninitOptions(void);

// helppack.c
void TrimStringSimple(char *str);
void TrimString(char *str);
BOOL IsEmpty(const char *str);

typedef struct {
	TCHAR szLanguage[64];
	LCID Locale;
	WORD codepage;
	char szAuthors[1024];
	char szAuthorEmail[128];
	char szLastModifiedUsing[64];
	char szPluginsIncluded[4080];
	char szVersion[21];
	char szFLName[128];
	FILETIME ftFileDate;
	TCHAR szFileName[MAX_PATH]; /* just the file name itself */
	BYTE flags; /* see HPIF_* flags */
} HELPPACK_INFO;

#define HPF_ENABLED   0x01  // pack is enabled
#define HPF_NOLOCALE  0x02  // pack has no valid locale
#define HPF_DEFAULT   0x04  // pack is english default

BOOL GetPackPath(TCHAR *pszPath, int nSize, BOOL fEnabledPacks, const TCHAR *pszFile);
typedef INT_PTR(CALLBACK *ENUM_PACKS_CALLBACK)(HELPPACK_INFO *pack, WPARAM wParam, LPARAM lParam);
BOOL EnumPacks(ENUM_PACKS_CALLBACK callback, const TCHAR *pszFilePattern, const char *pszFileVersionHeader, WPARAM wParam, LPARAM lParam);
BOOL IsPluginIncluded(const HELPPACK_INFO *pack, char *pszFileBaseName);
BOOL EnablePack(const HELPPACK_INFO *pack, const TCHAR *pszFilePattern);
void CorrectPacks(const TCHAR *pszFilePattern, const TCHAR *pszDefaultFile, BOOL fDisableAll);


#endif  // __CONTEXTHELP_HELP_H__