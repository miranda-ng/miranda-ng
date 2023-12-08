/*
Copyright (C) 2010 Unsane

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

using namespace std;

static IconItem iconList[] = 
{
	{ LPGEN("Button"), "qr_button", IDI_QICON }
};

int iNumber = 0;

int OnModulesLoaded(WPARAM, LPARAM)
{
	HookEvent(ME_MSG_BUTTONPRESSED, OnButtonPressed);

	g_plugin.registerIcon("TabSRMM/Quick Replies", iconList);

	char buttonNameTranslated[32], buttonName[32];
	mir_snprintf(buttonNameTranslated, "%s %x", Translate("Button"), iNumber + 1);
	mir_snprintf(buttonName, MODULENAME" %x", iNumber + 1);

	BBButton bbd = {};
	bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_NOREADONLY;
	bbd.pszModuleName = buttonName;
	bbd.pwszTooltip = LPGENW("Quick Replies");
	bbd.hIcon = iconList[0].hIcolib;
	bbd.dwButtonID = iNumber;
	bbd.dwDefPos = 220;
	g_plugin.addButton(&bbd);
	return 0;
}

int OnButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;

	char buttonName[32];
	mir_snprintf(buttonName, MODULENAME" %x", iNumber + 1);
	if (mir_strcmp(cbcd->pszModule, buttonName))
		return 0;

	if (cbcd->dwButtonId != iNumber)
		return 1;

	char key[64];
	mir_snprintf(key, "RepliesCount_%x", iNumber);
	int count = g_plugin.getWord(key, 0);

	if (count == 0 || cbcd->flags & BBCF_RIGHTBUTTON) {
		mir_snprintf(buttonName, "%s %x", Translate("Button"), iNumber + 1);
		g_plugin.openOptions(L"Message sessions", L"Quick Replies", _A2T(buttonName));
		return 0;
	}

	HMENU hMenu = CreatePopupMenu();

	LIST<wchar_t> replyList(1);
	for (int i = 0; i < count; i++) {
		mir_snprintf(key, "Reply_%x_%x", iNumber, i);
		ptrW value(g_plugin.getWStringA(key));
		if (value == nullptr)
			replyList.insert(mir_wstrdup(L""));
		else
			replyList.insert(variables_parsedup(value, nullptr, wParam));

		if (!mir_wstrcmp(value, L"---"))
			AppendMenu((HMENU)hMenu, MF_SEPARATOR, i + 1, nullptr);
		else
			AppendMenu((HMENU)hMenu, MF_STRING, i + 1, replyList[i]);
	}

	int index = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbcd->pt.x, cbcd->pt.y, 0, cbcd->hwndFrom, nullptr);
	if (index > 0) {
		if (mir_wstrcmp(replyList[index - 1], L"")) {
			CallService(MS_MSG_SENDMESSAGEW, cbcd->hContact, (LPARAM)replyList[index - 1]);

			mir_snprintf(key, "ImmediatelySend_%x", iNumber);
			if (g_plugin.getByte(key, 1) || cbcd->flags & BBCF_CONTROLPRESSED)
				SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
		}
	}

	for (auto &it : replyList)
		mir_free(it);
	replyList.destroy();

	return 1;
}
