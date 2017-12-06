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
#include "stdafx.h"


extern HWND hwndHelpDlg;

struct DlgControlData {
	int id;
	int type;
	TCHAR *szTitle;
	char *szText;  // ANSI or UTF-8 depending on _UNICODE defined (for RichEdit)
};

struct DialogData {
	char *szId;
	char *szModule;
	struct DlgControlData *control;
	int controlCount;
	DWORD timeLoaded, timeLastUsed;
	int changes;
	LCID locale;
	UINT defaultCodePage;
	int isLocaleRTL;
};

static struct DialogData *dialogCache = NULL;
static int dialogCacheCount = 0;
static CRITICAL_SECTION csDialogCache;
static HANDLE hServiceFileChange, hFileChange;

#define DIALOGCACHEEXPIRY 10*60*1000 // delete from cache after those milliseconds

static INT_PTR ServiceFileChanged(WPARAM wParam, LPARAM)
{
	EnterCriticalSection(&csDialogCache);
	for (int i = 0; i < dialogCacheCount; i++)
		dialogCache[i].timeLastUsed = 0;
	LeaveCriticalSection(&csDialogCache);

	if ((HANDLE)wParam != NULL)
		FindNextChangeNotification((HANDLE)wParam);

	return 0;
}

void InitDialogCache(void)
{
	TCHAR szFilePath[MAX_PATH], *p;

	InitializeCriticalSection(&csDialogCache);

	hServiceFileChange = CreateServiceFunction("Help/HelpPackChanged", ServiceFileChanged);
	hFileChange = INVALID_HANDLE_VALUE;
	if (GetModuleFileName(NULL, szFilePath, _countof(szFilePath))) {
		p = _tcsrchr(szFilePath, _T('\\'));
		if (p != NULL)
			*(p + 1) = _T('\0');
		hFileChange = FindFirstChangeNotification(szFilePath, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
		if (hFileChange != INVALID_HANDLE_VALUE)
			CallService(MS_SYSTEM_WAITONHANDLE, (WPARAM)hFileChange, (LPARAM)"Help/HelpPackChanged");
	}
}

static void FreeDialogCacheEntry(struct DialogData *entry)
{
	for (int i = 0; i < entry->controlCount; i++) {
		mir_free(entry->control[i].szText); // does NULL check
		mir_free(entry->control[i].szTitle); // does NULL check
	}
	mir_free(entry->control); // does NULL check
	mir_free(entry->szId); // does NULL check
	mir_free(entry->szModule); // does NULL check
}

void FreeDialogCache(void)
{
	if (hFileChange != INVALID_HANDLE_VALUE) {
		CallService(MS_SYSTEM_REMOVEWAIT, (WPARAM)hFileChange, 0);
		FindCloseChangeNotification(hFileChange);
	}
	DestroyServiceFunction(hServiceFileChange);

	DeleteCriticalSection(&csDialogCache);
	for (int i = 0; i < dialogCacheCount; i++)
		FreeDialogCacheEntry(&dialogCache[i]);
	dialogCacheCount = 0;
	mir_free(dialogCache); // does NULL check
	dialogCache = NULL;
}

/**************************** LOAD HELP ***************************/

struct LoaderThreadStartParams {
	HWND hwndCtl;
	char *szDlgId;
	char *szModule;
	int ctrlId;
};

static void LoaderThread(void *arg)
{
	LoaderThreadStartParams *dtsp = (LoaderThreadStartParams *)arg;

	FILE *fp;
	char line[4096];
	char *pszLine, *pszColon, *pszBuf;
	int startOfLine = 0;

	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	TCHAR szDir[MAX_PATH];
	TCHAR szSearch[MAX_PATH];
	TCHAR *p;
	int success = 0;

	struct DialogData dialog;
	struct DlgControlData *control;
	void *buf;
	ZeroMemory(&dialog, sizeof(dialog));

	if (GetModuleFileName(NULL, szDir, _countof(szDir))) {
		p = _tcsrchr(szDir, _T('\\'));
		if (p)
			*p = _T('\0');
		mir_sntprintf(szSearch, L"%s\\helppack_*.txt", szDir);

		hFind = FindFirstFile(szSearch, &wfd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;
				if (lstrlen(wfd.cFileName) < 4 || wfd.cFileName[lstrlen(wfd.cFileName) - 4] != _T('.'))
					continue;
				mir_sntprintf(szSearch, L"%s\\%s", szDir, wfd.cFileName);
				success = 1;
				break;
			} while (FindNextFile(hFind, &wfd));
			FindClose(hFind);
		}
	}
	if (!success) {
		if (!Miranda_Terminated() && IsWindow(hwndHelpDlg))
			PostMessage(hwndHelpDlg, M_HELPLOADFAILED, 0, (LPARAM)dtsp->hwndCtl);
		return;
	}

	fp = _tfopen(szSearch, L"rt");
	if (fp == NULL) {
		if (!Miranda_Terminated() && IsWindow(hwndHelpDlg))
			PostMessage(hwndHelpDlg, M_HELPLOADFAILED, 0, (LPARAM)dtsp->hwndCtl);
		return;
	}
	fgets(line, _countof(line), fp);
	TrimString(line);
	if (lstrcmpA(line, "Miranda Help Pack Version 1")) {
		fclose(fp);
		if (!Miranda_Terminated() && IsWindow(hwndHelpDlg))
			PostMessage(hwndHelpDlg, M_HELPLOADFAILED, 0, (LPARAM)dtsp->hwndCtl);
		return;
	}

	// headers
	dialog.locale = LOCALE_USER_DEFAULT;
	dialog.defaultCodePage = CP_ACP;
	while (!feof(fp)) {
		startOfLine = ftell(fp);
		if (fgets(line, _countof(line), fp) == NULL)
			break;
		TrimString(line);
		if (IsEmpty(line) || line[0] == ';' || line[0] == '\0')
			continue;
		if (line[0] == '[')
			break;
		pszColon = strchr(line, ':');
		if (pszColon == NULL) {
			fclose(fp);
			if (!Miranda_Terminated() && IsWindow(hwndHelpDlg))
				PostMessage(hwndHelpDlg, M_HELPLOADFAILED, 0, (LPARAM)dtsp->hwndCtl);
			return;
		}
		*pszColon = '\0';

		// locale
		if (!lstrcmpA(line, "Locale")) {
			char szCP[6];
			TrimString(pszColon + 1);
			dialog.locale = MAKELCID((USHORT)strtol(pszColon + 1, NULL, 16), SORT_DEFAULT);
			// codepage
			if (GetLocaleInfoA(dialog.locale, LOCALE_IDEFAULTANSICODEPAGE, szCP, sizeof(szCP))) {
				szCP[5] = '\0'; // codepages have 5 digits at max
				dialog.defaultCodePage = atoi(szCP);
			}
		}
	}

	// RTL flag
	// see also: http://blogs.msdn.com/michkap/archive/2006/03/03/542963.aspx
	{
		LOCALESIGNATURE sig;
		dialog.isLocaleRTL = 0;
		if (GetLocaleInfo(dialog.locale, LOCALE_FONTSIGNATURE, (LPTSTR)&sig, sizeof(sig) / sizeof(TCHAR)))
			dialog.isLocaleRTL = (sig.lsUsb[3] & 0x8000000); // Win2000+: testing for 'Layout progress: horizontal from right to left' bit
		switch (PRIMARYLANGID(LANGIDFROMLCID(dialog.locale))) { // prior to Win2000
		case LANG_ARABIC:
		case LANG_HEBREW:
		case LANG_FARSI:
			dialog.isLocaleRTL = 1;
		}
	}

	// body
	fseek(fp, startOfLine, SEEK_SET);
	success = 1;
	control = NULL;
	while (!feof(fp)) {
		if (fgets(line, _countof(line), fp) == NULL)
			break;
		if (IsEmpty(line) || line[0] == ';' || line[0] == '\0')
			continue;
		TrimStringSimple(line);

		if (line[0] == '[' && line[lstrlenA(line) - 1] == ']') {
			pszLine = line + 1;
			line[lstrlenA(line) - 1] = '\0';

			// module
			pszColon = strrchr(pszLine, ':');
			if (pszColon == NULL)
				continue;
			*pszColon = '\0';
			pszColon++;
			TrimString(pszLine);
			if (lstrcmpiA(dtsp->szModule, pszLine))
				continue;
			pszBuf = pszLine;

			// dlgid
			pszLine = pszColon;
			pszColon = strrchr(pszLine, '@');
			if (pszColon == NULL)
				continue;
			*pszColon = '\0';
			pszColon++;
			TrimString(pszColon);
			if (lstrcmpA(dtsp->szDlgId, pszColon))
				continue;

			if (dialog.szModule == NULL && dialog.szId == NULL) {
				dialog.szModule = mir_strdup(pszBuf);
				dialog.szId = mir_strdup(pszColon);
				if (dialog.szId == NULL || dialog.szModule == NULL) {
					success = 0;
					break;
				}
			}
			buf = (struct DlgControlData*)mir_realloc(dialog.control, sizeof(struct DlgControlData)*(dialog.controlCount + 1));
			if (buf == NULL) {
				success = 0;
				break;
			}
			dialog.controlCount++;
			dialog.control = (struct DlgControlData*)buf;
			control = &dialog.control[dialog.controlCount - 1];
			ZeroMemory(control, sizeof(*control));

			// ctlid
			TrimString(pszLine);
			control->id = atoi(pszLine);
		}
		else if (control != NULL) {
			pszLine = line;

			// ctltext
			pszColon = strchr(pszLine, '=');
			if (pszColon == NULL)
				continue;
			*pszColon = '\0';
			pszColon++;
			TrimString(pszColon);
			TrimString(pszLine);
			if (*pszColon == '\0' || *pszLine == '\0')
				continue;
			int size = lstrlenA(pszLine) + 1;
			control->szTitle = (WCHAR*)mir_alloc(size*sizeof(WCHAR));
			if (control->szTitle != NULL) {
				*control->szTitle = _T('\0');
				MultiByteToWideChar(dialog.defaultCodePage, 0, pszLine, -1, control->szTitle, size);
			}

			// text
			control->szText = mir_utf8encodecp(pszColon, dialog.defaultCodePage);
			control = NULL; // control done
		}
	}
	fclose(fp);

	if (success) {
		int i, dialogInserted = 0;

		dialog.timeLoaded = dialog.timeLastUsed = GetTickCount();
		EnterCriticalSection(&csDialogCache);
		for (i = 0; i < dialogCacheCount; i++) {
			if (dialogCache[i].timeLastUsed && dialogCache[i].timeLastUsed<(dialog.timeLoaded - DIALOGCACHEEXPIRY)) {
				FreeDialogCacheEntry(&dialogCache[i]);
				if (dialogInserted || !dialog.controlCount) {
					MoveMemory(dialogCache + i, dialogCache + i + 1, sizeof(struct DialogData)*(dialogCacheCount - i - 1));
					dialogCacheCount--;
					buf = (struct DialogData*)mir_realloc(dialogCache, sizeof(struct DialogData)*dialogCacheCount);
					if (buf != NULL)
						dialogCache = (struct DialogData*)buf;
					else if (!dialogCacheCount)
						dialogCache = NULL;
				}
				else {
					dialogInserted = 1;
					dialogCache[i] = dialog;
				}
			}
		}
		if (dialog.controlCount && !dialogInserted) {
			buf = (struct DialogData*)mir_realloc(dialogCache, sizeof(struct DialogData)*(dialogCacheCount + 1));
			if (buf != NULL) {
				dialogCacheCount++;
				dialogCache = (struct DialogData*)buf;
				dialogCache[dialogCacheCount - 1] = dialog;
				dialogInserted = 1;
			}
		}
		LeaveCriticalSection(&csDialogCache);

		if (!dialogInserted) {
			mir_free(dialog.szId); // does NULL check
			mir_free(dialog.szModule); // does NULL check
			mir_free(dialog.control); // does NULL check
		}
		if (!Miranda_Terminated() && IsWindow(hwndHelpDlg))
			PostMessage(hwndHelpDlg, M_HELPLOADED, 0, (LPARAM)dtsp->hwndCtl);
	}
	if (!success) {
		mir_free(dialog.szId); // does NULL check
		mir_free(dialog.szModule); // does NULL check
		mir_free(dialog.control); // does NULL check
		if (!Miranda_Terminated() && IsWindow(hwndHelpDlg))
			PostMessage(hwndHelpDlg, M_HELPLOADFAILED, 0, (LPARAM)dtsp->hwndCtl);
	}

	mir_free(dtsp->szDlgId);
	mir_free(dtsp->szModule);
	mir_free(dtsp);
}

// mir_free() the return value
char *CreateControlIdentifier(const char *pszDlgId, const char *pszModule, int ctrlId, HWND hwndCtl)
{
	int size;
	char *szId;
	TCHAR szDefCtlText[128];
	GetControlTitle(hwndCtl, szDefCtlText, _countof(szDefCtlText));
	size = lstrlenA(pszModule) + lstrlenA(pszDlgId) + _countof(szDefCtlText) + 22;
	szId = (char*)mir_alloc(size);
	mir_snprintf(szId, size, "[%s:%i@%s]\r\n%S%s", pszModule, ctrlId, pszDlgId, szDefCtlText, szDefCtlText[0] ? "=" : "");

	return szId;
}

int GetControlHelp(HWND hwndCtl, const char *pszDlgId, const char *pszModule, int ctrlId, TCHAR **ppszTitle, char **ppszText, int *pType, LCID *pLocaleID, UINT *pCodePage, BOOL *pIsRTL, DWORD flags)
{
	int i, j, useNext = 0;
	struct LoaderThreadStartParams *dtsp;

	EnterCriticalSection(&csDialogCache);
	for (i = 0; i < dialogCacheCount; i++) {
		if (!(flags&GCHF_DONTLOAD)) {
			if (!dialogCache[i].timeLastUsed || dialogCache[i].timeLastUsed<(GetTickCount() - DIALOGCACHEEXPIRY)) {
				struct DialogData *buf;
				FreeDialogCacheEntry(&dialogCache[i]);
				MoveMemory(dialogCache + i, dialogCache + i + 1, sizeof(struct DialogData)*(dialogCacheCount - i - 1));
				dialogCacheCount--;
				buf = (struct DialogData*)mir_realloc(dialogCache, sizeof(struct DialogData)*dialogCacheCount);
				if (buf != NULL)
					dialogCache = (struct DialogData*)buf;
				else if (!dialogCacheCount)
					dialogCache = NULL;
				i--;
				continue;
			}
		}
		if (lstrcmpA(pszDlgId, dialogCache[i].szId))
			break;
		for (j = 0; j < dialogCache[i].controlCount; j++) {
			if (ctrlId == dialogCache[i].control[j].id || useNext || dialogCache[i].control[j].id == 0) {
				if (dialogCache[i].control[j].szTitle == NULL) {
					useNext = 1;
					continue;
				}
				if (ppszTitle)
					*ppszTitle = dialogCache[i].control[j].szTitle;
				if (ppszText)
					*ppszText = dialogCache[i].control[j].szText;
				if (pType)
					*pLocaleID = dialogCache[i].locale;
				if (pCodePage)
					*pCodePage = CP_UTF8;
				if (pIsRTL)
					*pIsRTL = dialogCache[i].isLocaleRTL;
				if (dialogCache[i].control[j].id != ctrlId && !useNext)
					continue;
				dialogCache[i].timeLastUsed = GetTickCount();
				LeaveCriticalSection(&csDialogCache);
				return 0;
			}
		}
		break;
	}

	if (ppszTitle)
		*ppszTitle = NULL;
	if (ppszText)
		*ppszText = NULL;
	if (pType)
		*pType = CTLTYPE_UNKNOWN;
	if (pLocaleID)
		*pLocaleID = LOCALE_USER_DEFAULT;
	if (pCodePage)
		*pCodePage = CP_ACP;

	if (!(flags&GCHF_DONTLOAD)) {
		dtsp = (struct LoaderThreadStartParams*)mir_alloc(sizeof(struct LoaderThreadStartParams));
		if (dtsp == NULL) {
			LeaveCriticalSection(&csDialogCache);
			return 0;
		}
		dtsp->szDlgId = mir_strdup(pszDlgId);
		dtsp->szModule = mir_strdup(pszModule);
		if (dtsp->szDlgId == NULL || dtsp->szModule == NULL) {
			mir_free(dtsp->szDlgId); // does NULL check
			mir_free(dtsp->szModule); // does NULL check
			mir_free(dtsp);
			LeaveCriticalSection(&csDialogCache);
			return 0;
		}
		dtsp->ctrlId = ctrlId;
		dtsp->hwndCtl = hwndCtl;
		mir_forkthread(LoaderThread, dtsp);
	}
	LeaveCriticalSection(&csDialogCache);

	return 1;
}

/**************************** SAVE HELP ***************************/

#ifdef EDITOR
void SetControlHelp(const char *pszDlgId, const char *pszModule, int ctrlId, TCHAR *pszTitle, char *pszText, int type)
{
	int i, j;
	int found = 0;
	void *buf;

	EnterCriticalSection(&csDialogCache);
	j = 0;
	for (i = 0; i < dialogCacheCount; i++) {
		if (lstrcmpA(pszDlgId, dialogCache[i].szId))
			continue;
		for (j = 0; j < dialogCache[i].controlCount; j++) {
			if (ctrlId == dialogCache[i].control[j].id) {
				mir_free(dialogCache[i].control[j].szTitle); // does NULL check
				mir_free(dialogCache[i].control[j].szText); // does NULL check
				dialogCache[i].control[j].szTitle = NULL;
				dialogCache[i].control[j].szText = NULL;
				found = 1;
				break;
			}
		}
		if (!found) {
			buf = (struct DlgControlData*)mir_realloc(dialogCache[i].control, sizeof(struct DlgControlData)*(dialogCache[i].controlCount + 1));
			if (buf == NULL) {
				LeaveCriticalSection(&csDialogCache);
				return;
			}
			dialogCache[i].control = (struct DlgControlData*)buf;
			j = dialogCache[i].controlCount++;
			found = 1;
		}
		break;
	}
	if (!found) {
		buf = (struct DialogData*)mir_realloc(dialogCache, sizeof(struct DialogData)*(dialogCacheCount + 1));
		if (buf == NULL) {
			LeaveCriticalSection(&csDialogCache);
			return;
		}
		dialogCache = (struct DialogData*)buf;
		dialogCache[i].control = (struct DlgControlData*)mir_alloc(sizeof(struct DlgControlData));
		if (dialogCache[i].control == NULL) {
			LeaveCriticalSection(&csDialogCache);
			return;
		}
		dialogCache[i].controlCount = 1;
		i = dialogCacheCount;
		j = 0;

		dialogCache[i].szId = mir_strdup(pszDlgId);
		dialogCache[i].szModule = mir_strdup(pszModule);
		if (dialogCache[i].szId == NULL || dialogCache[i].szModule == NULL) {
			mir_free(dialogCache[i].szId); // does NULL check
			mir_free(dialogCache[i].szModule); // does NULL check
			LeaveCriticalSection(&csDialogCache);
			return;
		}
		dialogCacheCount++;
		dialogCache[i].timeLoaded = 0;
	}
	dialogCache[i].control[j].szTitle = mir_tstrdup(pszTitle); // does NULL arg check
	dialogCache[i].control[j].szText = mir_strdup(pszText); // does NULL arg check
	dialogCache[i].control[j].type = type;
	dialogCache[i].control[j].id = ctrlId;
	dialogCache[i].timeLastUsed = GetTickCount();
	dialogCache[i].changes = 1;
	LeaveCriticalSection(&csDialogCache);
}

static void DialogCacheSaveThread(void *unused)
{
	int success = 0;

	// TODO: port the following code to write to the helppack file instead
	// (netlib code already removed)
	/*
	{	WCHAR *szDecoded;
	char *szEncoded=mir_strdup(dialogCache[i].control[j].szText);
	if (mir_utf8decode(szEncoded, &szDecoded)) {
		[...]
		mir_free(szDecoded)
	}
	mir_free(szEncoded); // does NULL check
	}


	int i, j;
	struct ResizableCharBuffer data={0}, dlgId={0};

	for (i = 0; i < dialogCacheCount; i++) {
		if(!dialogCache[i].changes) continue;

		AppendToCharBuffer(&data, "t <dialog id=\"%s\" module=\"%s\">\r\n", dialogCache[i].szId, dialogCache[i].szModule);
		for (j = 0; j < dialogCache[i].controlCount; j++) {
			AppendToCharBuffer(&data, "<control id=%d type=%d>\r\n", dialogCache[i].control[j].id, dialogCache[i].control[j].type);
			if (dialogCache[i].control[j].szTitle)
				AppendToCharBuffer(&data, "<title>%s</title>\r\n", dialogCache[i].control[j].szTitle);
			if (dialogCache[i].control[j].szText)
				AppendToCharBuffer(&data, "<text>%s</text>\r\n", dialogCache[i].control[j].szText);
			AppendToCharBuffer(&data, "</control>\r\n");
		}
		AppendToCharBuffer(&data, "</dialog>");
	}

	if(success) {
		dialogCache[i].changes = 0;
		dialogCache[i].timeLoaded = GetTickCount();
	}
	*/
	MessageBoxEx(NULL, success ? TranslateT("Help saving complete.") : TranslateT("Help saving failed!"), TranslateT("Help editor"), (success ? MB_ICONINFORMATION : MB_ICONERROR) | MB_OK | MB_SETFOREGROUND | MB_TOPMOST | MB_TASKMODAL, LANGIDFROMLCID(Langpack_GetDefaultLocale()));
}

void SaveDialogCache(void)
{
	mir_forkthread(DialogCacheSaveThread, 0);
}
#endif // defined EDITOR
