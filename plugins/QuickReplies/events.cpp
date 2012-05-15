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

INT_PTR OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	UnhookEvent(hOnModulesLoaded);
	HICON hIcon = NULL;
	char buttonName[32];

	if (!ServiceExists(MS_QUICKREPLIES_SERVICE))
	{
		iNumber = 0;
		hQuickRepliesService = CreateServiceFunction(MS_QUICKREPLIES_SERVICE, QuickRepliesService);
	}
	else
		iNumber = DBGetContactSettingByte(NULL, MODULE_NAME, "InstancesCount", 0);
	DBWriteContactSettingByte(NULL, MODULE_NAME, "InstancesCount", iNumber + 1);

	hOnOptInitialized = HookEvent(ME_OPT_INITIALISE, OnOptInitialized); 
	hOnButtonPressed = HookEvent(ME_MSG_BUTTONPRESSED, OnButtonPressed); 

	if (ServiceExists(MS_SKIN2_ADDICON))
	{
		char buttonNameTranslated[32];
		mir_snprintf(buttonName, SIZEOF(buttonName), "Button %x", iNumber + 1);
		mir_snprintf(buttonNameTranslated, SIZEOF(buttonNameTranslated), "%s %x",Translate("Button"), iNumber + 1);

		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszSection = "TabSRMM/Quick Replies";
		sid.cx = sid.cy = 16;
		sid.pszDescription = buttonNameTranslated;
		sid.pszName = buttonName;
		sid.hDefaultIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_QICON));

		hIcon = (HICON)CallService(MS_SKIN2_ADDICON, 0, (LPARAM) &sid);
	}
	else
		hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_QICON));

	if (ServiceExists(MS_BB_ADDBUTTON))
	{
		mir_snprintf(buttonName, SIZEOF(buttonName), MODULE_NAME" %x", iNumber + 1);

		BBButton bbd = {0};
		bbd.cbSize = sizeof(BBButton);
		bbd.bbbFlags = BBBF_ISIMBUTTON | BBBF_ISCHATBUTTON | BBBF_ISLSIDEBUTTON;
		bbd.pszModuleName = buttonName;
		bbd.ptszTooltip = _T("Quick Replies\r\nLeft button - open menu\r\nRight button - options page");
		bbd.hIcon = (HANDLE)hIcon;
		bbd.dwButtonID = iNumber;
		bbd.dwDefPos = 220;
		
		CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);
	}

	DestroyIcon(hIcon);

	return 0;
}

INT_PTR OnButtonPressed(WPARAM wParam, LPARAM lParam)
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
	count = DBGetContactSettingWord(NULL, MODULE_NAME, key, 0);
			
	{		
		if (count == 0 || cbcd->flags & BBCF_RIGHTBUTTON)
		{
			mir_snprintf(buttonName, SIZEOF(buttonName), "Button %x", iNumber + 1);

			OPENOPTIONSDIALOG ood = {0};
			ood.cbSize = sizeof(ood);
			ood.pszGroup = "Message Sessions";
			ood.pszPage = "Quick Replies";
			ood.pszTab = buttonName;

			CallService(MS_OPT_OPENOPTIONS, NULL, (LPARAM)&ood);

			return 0;
		}

		hMenu = CreatePopupMenu();

		DBVARIANT dbv = {0};

		for (int i = 0; i < count; i++)
		{
			mir_snprintf(key, 64, "Reply_%x_%x", iNumber, i);
			DBGetContactSettingTString(NULL, MODULE_NAME, key, &dbv);

			if (dbv.ptszVal == NULL)
				replyList.push_back(_T(""));
			else
				replyList.push_back((TCHAR*)variables_parsedup(dbv.ptszVal, 0, (HANDLE)wParam));

			if (_tcscmp(dbv.ptszVal, _T("---")))
				AppendMenu((HMENU)hMenu, MF_STRING,	i + 1, replyList[i].c_str());
			else
				AppendMenu((HMENU)hMenu, MF_SEPARATOR,	i + 1, NULL);
		}

		DBFreeVariant(&dbv);
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
				if ((BYTE)DBGetContactSettingByte(NULL, MODULE_NAME, key, 1) || cbcd->flags & BBCF_CONTROLPRESSED)
					SendMessage(cbcd->hwndFrom, WM_COMMAND, IDOK, 0);
			}
		}
	}
	
	for (std::vector<tString>::reverse_iterator item = replyList.rbegin(); item != replyList.rend(); ++item)
		((tString)*item).clear();
	replyList.clear();

	return 1;
}

INT_PTR OnPreShutdown(WPARAM wParam, LPARAM lParam)
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