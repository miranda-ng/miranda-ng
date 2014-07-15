/*
 * astyle --force-indent=tab=4 --brackets=linux --indent-switches
 *		  --pad=oper --one-line=keep-blocks  --unpad=paren
 *
 * Miranda NG: the free IM client for Microsoft* Windows*
 *
 * Copyright (c) 2000-09 Miranda ICQ/IM project,
 * all portions of this codebase are copyrighted to the people
 * listed in contributors.txt.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * you should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * part of tabSRMM messaging plugin for Miranda.
 *
 * (C) 2005-2009 by silvercircle _at_ gmail _dot_ com and contributors
 *
 * implements features of the tabSRMM "MADMOD" patch, developed by
 * Mad Cluster in May 2008
 *
 * the "mad mod" patch added the following features:
 *
 * ) typing sounds
 * ) support for animated avatars through ACC (avs)
 * ) a fully customizable tool bar providing services useable by external plugins
 *   to add and change buttons
 * ) toolbar on the bottom
 * ) image tag button
 * ) client icon in status bar
 * ) close tab/window on send and the "hide container feature"
 * ) bug fixes
 *
 */

#include "commonheaders.h"

static TCHAR* getMenuEntry(int i)
{
	char MEntry[256];
	mir_snprintf(MEntry, 255, "MenuEntry_%u", i);
	return db_get_tsa(NULL, "tabmodplus", MEntry);
}

static int RegisterCustomButton(WPARAM wParam, LPARAM lParam)
{
	if (!ServiceExists(MS_BB_ADDBUTTON))
		return 1;

	BBButton bbd = { 0 };
	bbd.cbSize = sizeof(BBButton);
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISLSIDEBUTTON | BBBF_ISPUSHBUTTON;
	bbd.dwButtonID = 1;
	bbd.dwDefPos = 200;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[3];
	bbd.pszModuleName = (char*)"Tabmodplus";
	bbd.ptszTooltip = TranslateT("Insert [img] tag / surround selected text with [img][/img]");
	return CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);
}

static int CustomButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	if (strcmp(cbcd->pszModule, "Tabmodplus") || cbcd->dwButtonId != 1)
		return 0;

	BBButton bbd = { sizeof(bbd) };
	bbd.dwButtonID = 1;
	bbd.pszModuleName = (char *)"Tabmodplus";
	CallService(MS_BB_GETBUTTONSTATE, wParam, (LPARAM)&bbd);

	TCHAR *pszText = _T("");
	CHARRANGE cr;
	cr.cpMin = cr.cpMax = 0;
	SendDlgItemMessage(cbcd->hwndFrom, IDC_MESSAGE, EM_EXGETSEL, 0, (LPARAM)&cr);
	UINT textlenght = cr.cpMax - cr.cpMin;
	if (textlenght) {
		pszText = (TCHAR*)mir_alloc((textlenght + 1)*sizeof(TCHAR));
		ZeroMemory(pszText, (textlenght + 1)*sizeof(TCHAR));
		SendDlgItemMessage(cbcd->hwndFrom, IDC_MESSAGE, EM_GETSELTEXT, 0, (LPARAM)pszText);
	}

	int state = 0;
	if (cbcd->flags & BBCF_RIGHTBUTTON)
		state = 1;
	else if (textlenght)
		state = 2;
	else if (bbd.bbbFlags & BBSF_PUSHED)
		state = 3;
	else
		state = 4;

	TCHAR *pszFormatedText = NULL, *pszMenu[256] = { 0 };

	size_t bufSize;

	switch (state) {
	case 1:
		{
			int menulimit = M.GetByte("tabmodplus", "MenuCount", 0);
			if (menulimit == 0)
				break;

			HMENU hMenu = CreatePopupMenu();

			for (int menunum = 0; menunum < menulimit; menunum++) {
				pszMenu[menunum] = getMenuEntry(menunum);
				AppendMenu(hMenu, MF_STRING, menunum + 1, pszMenu[menunum]);
			}

			int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbcd->pt.x, cbcd->pt.y, 0, cbcd->hwndFrom, NULL);
			if (res == 0)
				break;

			bufSize = textlenght + lstrlen(pszMenu[res - 1]) + 2;
			pszFormatedText = (TCHAR*)_alloca(bufSize*sizeof(TCHAR));
			mir_sntprintf(pszFormatedText, bufSize, pszMenu[res-1], pszText);
		}
		break;

	case 2:
		SendDlgItemMessage(cbcd->hwndFrom, IDC_MESSAGE, EM_GETSELTEXT, 0, (LPARAM)pszText);

		bufSize = textlenght + 12;
		pszFormatedText = (TCHAR*)_alloca(bufSize*sizeof(TCHAR));
		mir_sntprintf(pszFormatedText, bufSize*sizeof(TCHAR), _T("[img]%s[/img]"), pszText);

		bbd.ptszTooltip = 0;
		bbd.hIcon = 0;
		bbd.bbbFlags = BBSF_RELEASED;
		CallService(MS_BB_SETBUTTONSTATE, wParam, (LPARAM)&bbd);
		break;

	case 3:
		pszFormatedText = _T("[img]");

		bbd.ptszTooltip = TranslateT("Insert [img] tag / surround selected text with [img][/img]");
		CallService(MS_BB_SETBUTTONSTATE, wParam, (LPARAM)&bbd);
		break;

	case 4:
		pszFormatedText = _T("[/img]");

		bbd.ptszTooltip = TranslateT("Insert [img] tag / surround selected text with [img][/img]");
		CallService(MS_BB_SETBUTTONSTATE, wParam, (LPARAM)&bbd);
		break;
	}

	for (int i = 0; pszMenu[i]; i++)
		mir_free(pszMenu[i]);

	if (pszFormatedText)
		SendDlgItemMessage(cbcd->hwndFrom, IDC_MESSAGE, EM_REPLACESEL, TRUE, (LPARAM)pszFormatedText);

	if (textlenght)
		mir_free(pszText);
	return 1;
}

int ModPlus_Init(WPARAM wparam, LPARAM lparam)
{
	HookEvent(ME_MSG_BUTTONPRESSED, CustomButtonPressed);
	HookEvent(ME_MSG_TOOLBARLOADED, RegisterCustomButton);
	return 0;
}
