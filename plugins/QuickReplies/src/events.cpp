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
	else iNumber = db_get_b(NULL, MODULE_NAME, "InstancesCount", 0);
	db_set_b(NULL, MODULE_NAME, "InstancesCount", iNumber + 1);

	hOnOptInitialized = HookEvent(ME_OPT_INITIALISE, OnOptInitialized); 
	hOnButtonPressed = HookEvent(ME_MSG_BUTTONPRESSED, OnButtonPressed); 

	if ( ServiceExists(MS_BB_ADDBUTTON)) {
		Icon_Register(hInstance, "TabSRMM/Quick Replies", &icon, 1);

		char buttonNameTranslated[32], buttonName[32];
		mir_snprintf(buttonNameTranslated, SIZEOF(buttonNameTranslated), "%s %x",Translate("Button"), iNumber + 1);
		mir_snprintf(buttonName, SIZEOF(buttonName), MODULE_NAME" %x", iNumber + 1);

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
	tString replies = _T("");
	vector<tString> replyList;
	CustomButtonClickData *cbcd = (CustomButtonClickData *)lParam;

	mir_snprintf(buttonName, SIZEOF(buttonName), MODULE_NAME" %x", iNumber + 1);
	if (strcmp(cbcd->pszModule, buttonName))
		return 0;

	if (cbcd->dwButtonId != iNumber) 
		return 1;

	mir_snprintf(key, 64, "RepliesCount_%x", iNumber);
	count = db_get_w(NULL, MODULE_NAME, key, 0);
			
	{		
		if (count == 0 || cbcd->flags & BBCF_RIGHTBUTTON)
		{
			mir_snprintf(buttonName, SIZEOF(buttonName), "Button %x", iNumber + 1);

			OPENOPTIONSDIALOG ood = {0};
			ood.cbSize = sizeof(ood);
			ood.pszGroup = "Message Sessions";
			ood.pszPage = "Quick Replies";
			ood.pszTab = buttonName;
			Options_Open(&ood);
			return 0;
		}

		hMenu = CreatePopupMenu();

		DBVARIANT dbv = {0};

		for (int i = 0; i < count; i++)
		{
			mir_snprintf(key, 64, "Reply_%x_%x", iNumber, i);
			db_get_ts(NULL, MODULE_NAME, key, &dbv);

			if (dbv.ptszVal == NULL)
				replyList.push_back(_T(""));
			else
				replyList.push_back((TCHAR*)variables_parsedup(dbv.ptszVal, 0, (MCONTACT)wParam));

			if (_tcscmp(dbv.ptszVal, _T("---")))
				AppendMenu((HMENU)hMenu, MF_STRING,	i + 1, replyList[i].c_str());
			else
				AppendMenu((HMENU)hMenu, MF_SEPARATOR,	i + 1, NULL);
		}

		db_free(&dbv);
	}

	{
		int index = TrackPopupMenu(hMenu, TPM_RETURNCMD, cbcd->pt.x, cbcd->pt.y, 0, cbcd->hwndFrom, NULL);
		if (index > 0)
		{
			if (_tcscmp(replyList.at(index - 1).c_str(), _T("")))
			{
				HWND hEdit = GetDlgItem(cbcd->hwndFrom, IDC_MESSAGE);
				if (!hEdit) hEdit = GetDlgItem(cbcd->hwndFrom, IDC_CHATMESSAGE);

				SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)replyList.at(index - 1).c_str());

				mir_snprintf(key, 64, "ImmediatelySend_%x", iNumber);
				if ((BYTE)db_get_b(NULL, MODULE_NAME, key, 1) || cbcd->flags & BBCF_CONTROLPRESSED)
					SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
			}
		}
	}
	
	for (std::vector<tString>::reverse_iterator item = replyList.rbegin(); item != replyList.rend(); ++item)
		((tString)*item).clear();
	replyList.clear();

	return 1;
}

int OnPreShutdown(WPARAM wParam, LPARAM lParam)
{
	if (ServiceExists(MS_BB_REMOVEBUTTON))
	{
		char buttonName[32];
		mir_snprintf(buttonName, SIZEOF(buttonName), MODULE_NAME" %x", iNumber + 1);

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