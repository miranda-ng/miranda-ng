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

#include "common.h"

using namespace std;

BYTE iNumber;

HANDLE hOnOptInitialized;
HANDLE hOnButtonPressed;
HANDLE hQuickRepliesService;

INT_PTR QuickRepliesService(WPARAM, LPARAM)
{
	return TRUE;
}

static IconItem icon = { LPGEN("Button"), "qr_button", IDI_QICON };

int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(hOnModulesLoaded);

	if ( !ServiceExists(MS_QUICKREPLIES_SERVICE)) {
		iNumber = 0;
		hQuickRepliesService = CreateServiceFunction(MS_QUICKREPLIES_SERVICE, QuickRepliesService);
	}
	else iNumber = db_get_b(NULL, MODULE, "InstancesCount", 0);
	db_set_b(NULL, MODULE, "InstancesCount", iNumber + 1);

	hOnOptInitialized = HookEvent(ME_OPT_INITIALISE, OnOptInitialized);
	hOnButtonPressed = HookEvent(ME_MSG_BUTTONPRESSED, OnButtonPressed);

	if ( ServiceExists(MS_BB_ADDBUTTON)) {
		Icon_Register(hInstance, "TabSRMM/Quick Replies", &icon, 1);

		char buttonNameTranslated[32], buttonName[32];
		mir_snprintf(buttonNameTranslated, SIZEOF(buttonNameTranslated), "%s %x", Translate("Button"), iNumber + 1);
		mir_snprintf(buttonName, SIZEOF(buttonName), MODULE" %x", iNumber + 1);

		BBButton bbd = {0};
		bbd.cbSize = sizeof(BBButton);
		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON;
		bbd.pszModuleName = buttonName;
		bbd.ptszTooltip = LPGENT("Quick Replies\r\nLeft button - open menu\r\nRight button - options page");
		bbd.hIcon = icon.hIcolib;
		bbd.dwButtonID = iNumber;
		bbd.dwDefPos = 220;
		
		CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);
	}

	return 0;
}

int OnButtonPressed(WPARAM wParam, LPARAM lParam)
{
	char key[64];
	int index = 0;
	int count = 0;
	HMENU hMenu = NULL;
	char buttonName[32];
	CMString replies;
	LIST<wchar_t> replyList(1);
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;

	mir_snprintf(buttonName, SIZEOF(buttonName), MODULE" %x", iNumber + 1);
	if (mir_strcmp(cbcd->pszModule, buttonName))
		return 0;

	if (cbcd->dwButtonId != iNumber)
		return 1;

	mir_snprintf(key, SIZEOF(key), "RepliesCount_%x", iNumber);
	count = db_get_w(NULL, MODULE, key, 0);

	{
		if (count == 0 || cbcd->flags & BBCF_RIGHTBUTTON)
		{
			mir_snprintf(buttonName, SIZEOF(buttonName), "%s %x", Translate("Button"), iNumber + 1);

			OPENOPTIONSDIALOG ood = {0};
			ood.cbSize = sizeof(ood);
			ood.pszGroup = "Message Sessions";
			ood.pszPage = "Quick Replies";
			ood.pszTab = buttonName;
			Options_Open(&ood);
			return 0;
		}

		hMenu = CreatePopupMenu();

		for (int i = 0; i < count; i++)
		{
			mir_snprintf(key, SIZEOF(key), "Reply_%x_%x", iNumber, i);
			wchar_t *value = db_get_wsa(NULL, MODULE, key);

			if (!value)
				replyList.insert(mir_wstrdup(_T("")));
			else
				replyList.insert(variables_parsedup(value, 0, wParam));

			if (!mir_tstrcmp(value, _T("---")))
				AppendMenu((HMENU)hMenu, MF_SEPARATOR, i + 1, NULL);
			else
				AppendMenu((HMENU)hMenu, MF_STRING, i + 1, replyList[i]);

			mir_free(value);
		}
	}

	{
		int index = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbcd->pt.x, cbcd->pt.y, 0, cbcd->hwndFrom, NULL);
		if (index > 0)
		{
			if (mir_tstrcmp(replyList[index - 1], _T("")))
			{
				HWND hEdit = GetDlgItem(cbcd->hwndFrom, IDC_MESSAGE);
				if (!hEdit) hEdit = GetDlgItem(cbcd->hwndFrom, IDC_CHATMESSAGE);

				SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)replyList[index - 1]);

				mir_snprintf(key, SIZEOF(key), "ImmediatelySend_%x", iNumber);
				if ((BYTE)db_get_b(NULL, MODULE, key, 1) || cbcd->flags & BBCF_CONTROLPRESSED)
					SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
			}
		}
	}
	
	for (int i = 0; i < replyList.getCount(); i++)
		mir_free(replyList[i]);
	replyList.destroy();

	return 1;
}

int OnPreShutdown(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_BB_REMOVEBUTTON))
	{
		char buttonName[32];
		mir_snprintf(buttonName, SIZEOF(buttonName), MODULE" %x", iNumber + 1);

		BBButton bbd = {0};
		bbd.cbSize = sizeof(BBButton);
		bbd.pszModuleName = buttonName;
		bbd.dwButtonID = iNumber;

		CallService(MS_BB_REMOVEBUTTON, 0, (LPARAM)&bbd);
	}
	UnhookEvent(hOnButtonPressed);
	UnhookEvent(hOnOptInitialized);
	UnhookEvent(hOnPreShutdown);
	return 0;
}