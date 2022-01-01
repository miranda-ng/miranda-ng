/////////////////////////////////////////////////////////////////////////////////////////
// Miranda NG: the free IM client for Microsoft* Windows*
//
// Copyright (C) 2012-22 Miranda NG team,
// Copyright (c) 2000-09 Miranda ICQ/IM project,
// all portions of this codebase are copyrighted to the people
// listed in contributors.txt.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// you should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// part of tabSRMM messaging plugin for Miranda.
//
// (C) 2005-2010 by silvercircle _at_ gmail _dot_ com and contributors
//
// implements features of the tabSRMM "MADMOD" patch, developed by
// Mad Cluster in May 2008
//
// the "mad mod" patch added the following features:
//
// ) typing sounds
// ) support for animated avatars through ACC (avs)
// ) a fully customizable tool bar providing services useable by external plugins
//   to add and change buttons
// ) toolbar on the bottom
// ) image tag button
// ) client icon in status bar
// ) close tab/window on send and the "hide container feature"

#include "stdafx.h"

static wchar_t* getMenuEntry(int i)
{
	char MEntry[256];
	mir_snprintf(MEntry, "MenuEntry_%u", i);
	return db_get_wsa(0, "tabmodplus", MEntry);
}

static int RegisterCustomButton(WPARAM, LPARAM)
{
	BBButton bbd = {};
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISPUSHBUTTON;
	bbd.dwButtonID = 1;
	bbd.dwDefPos = 200;
	bbd.hIcon = PluginConfig.g_buttonBarIconHandles[3];
	bbd.pszModuleName = "Tabmodplus";
	bbd.pwszTooltip = LPGENW("Insert [img] tag / surround selected text with [img][/img]");
	Srmm_AddButton(&bbd, &g_plugin);
	return 0;
}

static int CustomButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;
	if (mir_strcmp(cbcd->pszModule, "Tabmodplus") || cbcd->dwButtonId != 1)
		return 0;

	BBButton bbd = {};
	bbd.dwButtonID = 1;
	bbd.pszModuleName = "Tabmodplus";
	Srmm_GetButtonState(cbcd->hwndFrom, &bbd);

	ptrW pszText;
	CHARRANGE cr;
	cr.cpMin = cr.cpMax = 0;
	SendDlgItemMessage(cbcd->hwndFrom, IDC_SRMM_MESSAGE, EM_EXGETSEL, 0, (LPARAM)&cr);
	UINT textlenght = cr.cpMax - cr.cpMin;
	if (textlenght) {
		pszText = (wchar_t*)mir_alloc((textlenght + 1)*sizeof(wchar_t));
		memset(pszText, 0, ((textlenght + 1) * sizeof(wchar_t)));
		SendDlgItemMessage(cbcd->hwndFrom, IDC_SRMM_MESSAGE, EM_GETSELTEXT, 0, (LPARAM)pszText);
	}

	size_t bufSize;
	CMStringW pwszFormatedText;

	if (cbcd->flags & BBCF_RIGHTBUTTON) {
		int menulimit = db_get_b(0, "tabmodplus", "MenuCount", 0);
		if (menulimit != 0) {
			HMENU hMenu = CreatePopupMenu();
			LIST<wchar_t> arMenuLines(1);

			for (int menunum = 0; menunum < menulimit; menunum++) {
				wchar_t *pwszText = getMenuEntry(menunum);
				arMenuLines.insert(pwszText);
				AppendMenu(hMenu, MF_STRING, menunum + 1, pwszText);
			}

			int res = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbcd->pt.x, cbcd->pt.y, 0, cbcd->hwndFrom, nullptr);
			if (res != 0) {
				bufSize = textlenght + mir_wstrlen(arMenuLines[res-1]) + 2;
				pwszFormatedText.Format(arMenuLines[res-1], pszText.get());
			}

			for (auto &it : arMenuLines)
				mir_free(it);
		}
	}
	else if (textlenght) {
		SendDlgItemMessage(cbcd->hwndFrom, IDC_SRMM_MESSAGE, EM_GETSELTEXT, 0, (LPARAM)pszText);

		pwszFormatedText.Format(L"[img]%s[/img]", pszText.get());

		bbd.pwszTooltip = nullptr;
		bbd.hIcon = nullptr;
		bbd.bbbFlags = BBSF_RELEASED;
		Srmm_SetButtonState(wParam, &bbd);
	}
	else if (bbd.bbbFlags & BBSF_PUSHED) {
		pwszFormatedText = L"[img]";

		bbd.pwszTooltip = LPGENW("Insert [img] tag / surround selected text with [img][/img]");
		Srmm_SetButtonState(wParam, &bbd);
	}
	else {
		pwszFormatedText = L"[/img]";

		bbd.pwszTooltip = LPGENW("Insert [img] tag / surround selected text with [img][/img]");
		Srmm_SetButtonState(wParam, &bbd);
	}

	if (!pwszFormatedText.IsEmpty())
		SendDlgItemMessage(cbcd->hwndFrom, IDC_SRMM_MESSAGE, EM_REPLACESEL, TRUE, (LPARAM)pwszFormatedText.c_str());
	return 1;
}

int ModPlus_Init()
{
	HookEvent(ME_MSG_BUTTONPRESSED, CustomButtonPressed);
	
	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, RegisterCustomButton);
	return 0;
}
