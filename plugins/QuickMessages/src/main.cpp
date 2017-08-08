/*
Quick Messages plugin for Miranda IM

Copyright (C) 2008 Danil Mozhar

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

CLIST_INTERFACE *pcli;
HINSTANCE hinstance;
int hLangpack;

WNDPROC mainProc;

int g_iButtonsCount=0;
int g_bShutDown=0;
int g_bStartup=0;
BOOL g_bRClickAuto=0;
BOOL g_bLClickAuto=0;
BOOL g_bQuickMenu=0;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {37ED754B-6CF9-40ED-9EB6-0FEF8E822475}
	{ 0x37ed754b, 0x6cf9, 0x40ed, { 0x9e, 0xb6, 0xf, 0xef, 0x8e, 0x82, 0x24, 0x75 } }
};

int PreShutdown(WPARAM, LPARAM)
{
	g_bShutDown = 1;
	DestructButtonsList();
	return 0;
}

static int InputMenuPopup(WPARAM, LPARAM lParam)
{
	HMENU hSubMenu = NULL;
	int i = 0;
	MessageWindowPopupData *mwpd = (MessageWindowPopupData *)lParam;
	if (mwpd->uFlags == MSG_WINDOWPOPUP_LOG || !g_bQuickMenu || !QuickList->realCount)
		return 0;

	if (mwpd->uType == MSG_WINDOWPOPUP_SHOWING) {
		hSubMenu = CreatePopupMenu();

		InsertMenu((HMENU)mwpd->hMenu, 6, MF_STRING | MF_POPUP | MF_BYPOSITION, (UINT_PTR)hSubMenu, TranslateT("Quick Messages"));
		InsertMenu((HMENU)mwpd->hMenu, 7, MF_SEPARATOR | MF_BYPOSITION, 0, 0);
		qsort(QuickList->items, QuickList->realCount, sizeof(QuickData *), sstQuickSortButtons);
		for (i = 0; i < QuickList->realCount; i++) {
			QuickData *qd = (QuickData*)QuickList->items[i];
			if (qd->fEntryType & QMF_EX_SEPARATOR)
				AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
			else
				AppendMenu(hSubMenu, MF_STRING, qd->dwPos + 254, qd->ptszValueName);
		}
	}
	else if (mwpd->uType == MSG_WINDOWPOPUP_SELECTED && mwpd->selection >= 254) {
		for (i = 0; i < QuickList->realCount; i++) {
			QuickData *qd = (QuickData *)QuickList->items[i];
			if ((qd->dwPos + 254) == mwpd->selection) {
				CHARRANGE cr;
				UINT textlenght = 0;
				ptrW pszText, pszCBText;
				BOOL bIsService = 0;
				wchar_t *ptszQValue = nullptr;

				if (IsClipboardFormatAvailable(CF_TEXT)) {
					if (OpenClipboard(mwpd->hwnd)) {
						HANDLE hData = NULL;
						wchar_t* chBuffer = NULL;
						int textLength = 0;

						hData = GetClipboardData(CF_UNICODETEXT);

						chBuffer = (wchar_t*)GlobalLock(hData);
						textLength = (int)mir_wstrlen(chBuffer);
						pszCBText = mir_wstrdup(chBuffer);
						GlobalUnlock(hData);
						CloseClipboard();
					}
				}

				SendMessage(mwpd->hwnd, EM_EXGETSEL, 0, (LPARAM)&cr);
				textlenght = cr.cpMax - cr.cpMin;

				if (textlenght) {
					pszText = (wchar_t *)mir_alloc((textlenght + 10)*sizeof(wchar_t));
					memset(pszText, 0, ((textlenght + 10) * sizeof(wchar_t)));
					SendMessage(mwpd->hwnd, EM_GETSELTEXT, 0, (LPARAM)pszText);
				}
				if (qd->ptszValue) {
					ptszQValue = ParseString(mwpd->hContact, qd->ptszValue, pszText ? pszText : L"", pszCBText ? pszCBText : L"", (int)mir_wstrlen(qd->ptszValue), textlenght, pszCBText ? (int)mir_wstrlen(pszCBText) : 0);
					if ((bIsService = qd->bIsService) && ptszQValue)
						CallService(mir_u2a(ptszQValue), (WPARAM)mwpd->hContact, 0);
				}

				if (ptszQValue) {
					SendMessage(mwpd->hwnd, EM_REPLACESEL, TRUE, (LPARAM)ptszQValue);
					free(ptszQValue);
				}
				break;
			}
		}
		return 1;
	}
	return 0;
}

static int CustomButtonPressed(WPARAM, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	if (mir_strcmp(cbcd->pszModule, PLGNAME))
		return 0;

	if (!ButtonsList[cbcd->dwButtonId])
		return 1;

	SortedList *sl = ButtonsList[cbcd->dwButtonId]->sl;
	if (!sl)
		return 1;

	BOOL bCTRL = 0;
	BOOL bIsService = 0;
	ptrW pszText, pszCBText;
	wchar_t *ptszQValue = nullptr;

	if (IsClipboardFormatAvailable(CF_TEXT)) {
		if (OpenClipboard(cbcd->hwndFrom)) {
			HANDLE hData = GetClipboardData(CF_UNICODETEXT);
			pszCBText = mir_wstrdup((wchar_t*)GlobalLock(hData));
			GlobalUnlock(hData);
			CloseClipboard();
		}
	}

	qsort(sl->items, sl->realCount, sizeof(ButtonData *), sstSortButtons);

	HWND hEdit = GetDlgItem(cbcd->hwndFrom, IDC_SRMM_MESSAGE);

	CHARRANGE cr;
	cr.cpMin = cr.cpMax = 0;
	SendMessage(hEdit, EM_EXGETSEL, 0, (LPARAM)&cr);

	UINT textlenght = cr.cpMax - cr.cpMin;
	if (textlenght) {
		pszText = (wchar_t *)mir_alloc((textlenght + 10)*sizeof(wchar_t));
		memset(pszText, 0, ((textlenght + 10) * sizeof(wchar_t)));
		SendMessage(hEdit, EM_GETSELTEXT, 0, (LPARAM)pszText);
	}

	int state;
	if (cbcd->flags & BBCF_RIGHTBUTTON)
		state = 1;
	else if (sl->realCount == 1)
		state = 2;
	else
		state = 3;

	switch (state) {
	case 1:
		if (ButtonsList[cbcd->dwButtonId]->ptszQValue)
			ptszQValue = ParseString(cbcd->hContact, ButtonsList[cbcd->dwButtonId]->ptszQValue, pszText ? pszText : L"", pszCBText ? pszCBText : L"", (int)mir_wstrlen(ButtonsList[cbcd->dwButtonId]->ptszQValue), textlenght, pszCBText ? (int)mir_wstrlen(pszCBText) : 0);
		if ((bIsService = ButtonsList[cbcd->dwButtonId]->bIsServName) && ptszQValue)
			CallService(mir_u2a(ptszQValue), (WPARAM)cbcd->hContact, 0);
		break;

	case 2:
		{
			ButtonData *bd = (ButtonData *)sl->items[0];
			if (bd && bd->pszValue) {
				ptszQValue = ParseString(cbcd->hContact, bd->pszValue, pszText ? pszText : L"", pszCBText ? pszCBText : L"", (int)mir_wstrlen(bd->pszValue), textlenght, pszCBText ? (int)mir_wstrlen(pszCBText) : 0);
				if ((bIsService = bd->bIsServName) && ptszQValue)
					CallService(mir_u2a(ptszQValue), (WPARAM)cbcd->hContact, 0);
			}
		}
		break;

	case 3:
		if (!g_iButtonsCount)
			break;
			
		HMENU hMenu = CreatePopupMenu(), hSubMenu = NULL;

		for (int menunum = 0; menunum < sl->realCount; menunum++) {
			ButtonData *bd = (ButtonData *)sl->items[menunum];
			if (bd->dwOPFlags & QMF_NEW)
				continue;

			BOOL bSetPopupMark = FALSE;
			if (bd->pszValue == 0 && bd->fEntryType == 0) {
				hSubMenu = CreatePopupMenu();
				bSetPopupMark = TRUE;
			}

			if (bd->pszValue && bd->fEntryType == 0)
				hSubMenu = NULL;

			if (bd->fEntryType & QMF_EX_SEPARATOR)
				AppendMenu((HMENU)((hSubMenu && !bSetPopupMark) ? hSubMenu : hMenu), MF_SEPARATOR, 0, NULL);
			else
				AppendMenu((HMENU)((hSubMenu && !bSetPopupMark) ? hSubMenu : hMenu),
				MF_STRING | (bSetPopupMark ? MF_POPUP : 0),
				(bSetPopupMark ? (UINT_PTR)hSubMenu : (menunum + 1)), bd->pszName);
		}

		int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbcd->pt.x, cbcd->pt.y, 0, cbcd->hwndFrom, NULL);
		if (res == 0)
			break;

		ButtonData *bd = (ButtonData *)sl->items[res - 1];
		bCTRL = (GetKeyState(VK_CONTROL) & 0x8000) ? 1 : 0;
		if (bd->pszValue) {
			ptszQValue = ParseString(cbcd->hContact, bd->pszValue, pszText ? pszText : L"", pszCBText ? pszCBText : L"", (int)mir_wstrlen(bd->pszValue), textlenght, pszCBText ? (int)mir_wstrlen(pszCBText) : 0);
			if ((bIsService = bd->bIsServName) && ptszQValue)
				CallService(mir_u2a(ptszQValue), (WPARAM)cbcd->hContact, 0);
		}
		break;
	}

	if (ptszQValue) {
		if (!bIsService) {
			SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)ptszQValue);

			if ((g_bLClickAuto && state != 1) || (g_bRClickAuto && state == 1) || cbcd->flags & BBCF_CONTROLPRESSED || bCTRL)
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
		}
		free(ptszQValue);
	}

	return 1;
}

static int PluginInit(WPARAM, LPARAM)
{
	g_bStartup = 1;
	HookEvent(ME_OPT_INITIALISE, OptionsInit);
	HookEvent(ME_MSG_BUTTONPRESSED, CustomButtonPressed);
	HookEvent(ME_MSG_WINDOWPOPUP, InputMenuPopup);

	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, RegisterCustomButton);

	g_bRClickAuto = db_get_b(NULL, PLGNAME, "RClickAuto", 0);
	g_bLClickAuto = db_get_b(NULL, PLGNAME, "LClickAuto", 0);
	g_iButtonsCount = db_get_b(NULL, PLGNAME, "ButtonsCount", 0);
	g_bQuickMenu = db_get_b(NULL, PLGNAME, "QuickMenu", 1);

	InitButtonsList();

	g_bStartup = 0;
	return 0;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD, LPVOID)
{
	hinstance = hinst;
	return 1;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	pcli = Clist_GetInterface();

	HookEvent(ME_SYSTEM_MODULESLOADED, PluginInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	return 0;
}
