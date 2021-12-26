#include "stdafx.h"

INT_PTR CALLBACK DlgProcContactInfo(HWND hwnd, UINT msg, WPARAM, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			MCONTACT hContact = (MCONTACT)((PROPSHEETPAGE*)lParam)->lParam;
			char name[2048];
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hContact);

			if (db_get_static(hContact, MODNAME, "Name", name, _countof(name)))
				break;
			SetDlgItemTextA(hwnd, IDC_DISPLAY_NAME, name);
			if (db_get_static(hContact, MODNAME, "ToolTip", name, _countof(name)))
				break;
			SetDlgItemTextA(hwnd, IDC_TOOLTIP, name);
		}
		return TRUE;

	case WM_COMMAND:
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		return TRUE;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				if (GetWindowTextLength(GetDlgItem(hwnd, IDC_DISPLAY_NAME))) {
					char text[512];
					GetDlgItemTextA(hwnd, IDC_DISPLAY_NAME, text, _countof(text));
					g_plugin.setString(hContact, "Name", text);
					WriteSetting(hContact, MODNAME, "Name", MODNAME, "Nick");
				}
				else {
					g_plugin.delSetting(hContact, "Name");
					g_plugin.delSetting(hContact, "Nick");
				}

				if (GetWindowTextLength(GetDlgItem(hwnd, IDC_TOOLTIP))) {
					char text[2048];
					GetDlgItemTextA(hwnd, IDC_TOOLTIP, text, _countof(text));
					g_plugin.setString(hContact, "ToolTip", text);
					WriteSetting(hContact, MODNAME, "ToolTip", "UserInfo", "MyNotes");
				}
				else {
					g_plugin.delSetting(hContact, "ToolTip");
					db_unset(hContact, "UserInfo", "MyNotes");
				}
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static WNDPROC g_PrevBtnWndProc = nullptr;

LRESULT CALLBACK ButtWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT res = CallWindowProc(g_PrevBtnWndProc, hWnd, message, wParam, lParam);
	if (WM_PAINT == message) {
		RECT rc;
		HDC dc = GetDC(hWnd);
		BOOL isPressed = BST_CHECKED == SendMessage(hWnd, BM_GETCHECK, 0, 0);

		GetClientRect(hWnd, &rc);
		rc.left += (rc.right - rc.left - 16) / 2;
		rc.top += (rc.bottom - rc.top - 16) / 2;
		if (isPressed)
			OffsetRect(&rc, 1, 1);
		DrawIconEx(dc, rc.left, rc.top, (HICON)GetWindowLongPtr(hWnd, GWLP_USERDATA), 16, 16, 0, nullptr, DI_NORMAL);
		ReleaseDC(hWnd, dc);
	}

	return res;
}

int BrowseForFolder(HWND hwnd, char *szPath)
{
	int result = 0;
	LPMALLOC pMalloc;

	if (SUCCEEDED(CoGetMalloc(1, &pMalloc))) {
		ptrW tszPath(mir_a2u(szPath));
		BROWSEINFO bi = {};
		bi.hwndOwner = hwnd;
		bi.pszDisplayName = tszPath;
		bi.lpszTitle = TranslateT("Select Folder");
		bi.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS;				// Use this combo instead of BIF_USENEWUI
		bi.lParam = (LPARAM)szPath;

		ITEMIDLIST *pidlResult = SHBrowseForFolder(&bi);
		if (pidlResult) {
			SHGetPathFromIDListA(pidlResult, szPath);
			mir_strcat(szPath, "\\");
			result = 1;
		}
		pMalloc->Free(pidlResult);
		pMalloc->Release();
	}

	return result;
}

INT_PTR CALLBACK DlgProcOtherStuff(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		{
			MCONTACT hContact = (MCONTACT)((PROPSHEETPAGE*)lParam)->lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hContact);
			if (!hContact)
				break;

			/* link*/
			DBVARIANT dbv;
			if (!g_plugin.getWString(hContact, "ProgramString", &dbv)) {
				SetDlgItemText(hwnd, IDC_LINK, dbv.pwszVal);
				db_free(&dbv);
			}

			if (!g_plugin.getWString(hContact, "ProgramParamsString", &dbv)) {
				SetDlgItemText(hwnd, IDC_PARAMS, dbv.pwszVal);
				db_free(&dbv);
			}

			/* group*/
			wchar_t *szGroup;
			for (int i = 1; (szGroup = Clist_GroupGetName(i, nullptr)) != nullptr; i++)
				SendDlgItemMessage(hwnd, IDC_GROUP, CB_INSERTSTRING, 0, LPARAM(szGroup));

			ptrW wszGroup(Clist_GetGroup(hContact));
			if (wszGroup)
				SetDlgItemTextW(hwnd, IDC_GROUP, wszGroup);

			/* icons */
			CheckRadioButton(hwnd, 40072, 40078, g_plugin.getWord(hContact, "Icon", ID_STATUS_ONLINE));
			SetWindowLongPtr(GetDlgItem(hwnd, CHK_ONLINE), GWLP_USERDATA, (LONG_PTR)Skin_LoadProtoIcon(MODNAME, ID_STATUS_ONLINE));
			g_PrevBtnWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd, CHK_ONLINE), GWLP_WNDPROC, (LONG_PTR)ButtWndProc);
			for (int i = ID_STATUS_ONLINE; i <= ID_STATUS_MAX; i++) {
				SetWindowLongPtr(GetDlgItem(hwnd, i), GWLP_USERDATA, (LONG_PTR)Skin_LoadProtoIcon(MODNAME, i));
				SetWindowLongPtr(GetDlgItem(hwnd, i), GWLP_WNDPROC, (LONG_PTR)ButtWndProc);
			}
			db_free(&dbv);
			/* timer */
			CheckDlgButton(hwnd, CHK_USE_TIMER, g_plugin.getByte(hContact, "UseTimer", 0) ? BST_CHECKED : BST_UNCHECKED);
			if (g_plugin.getWord(hContact, "Timer", 15)) {
				CheckDlgButton(hwnd, CHK_USE_TIMER, BST_CHECKED);
				EnableWindow(GetDlgItem(hwnd, IDC_TIMER), 1);
				wchar_t string[512];
				SetDlgItemText(hwnd, IDC_TIMER, _itow(g_plugin.getWord(hContact, "Timer", 15), string, 10));
				if (!g_plugin.getWord("Timer", 1))
					SetDlgItemText(hwnd, IDC_TIMER_INTERVAL_MSG, TranslateT("Non-IM Contact protocol timer is Disabled"));
				else {
					mir_snwprintf(string, TranslateT("Timer intervals... Non-IM Contact Protocol timer is %d seconds"), g_plugin.getWord("Timer", 1));
					SetDlgItemText(hwnd, IDC_TIMER_INTERVAL_MSG, string);
				}
			}
			/* always visible */
			if (g_plugin.getByte(hContact, "AlwaysVisible", 0)) {
				CheckDlgButton(hwnd, IDC_ALWAYS_VISIBLE, BST_CHECKED);
				EnableWindow(GetDlgItem(hwnd, IDC_VISIBLE_UNLESS_OFFLINE), 1);
				CheckDlgButton(hwnd, IDC_VISIBLE_UNLESS_OFFLINE, g_plugin.getByte(hContact, "VisibleUnlessOffline", 1) ? BST_CHECKED : BST_UNCHECKED);
			}
		}
		return TRUE;

	case WM_COMMAND:
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		switch (LOWORD(wParam)) {
		case IDC_ALWAYS_VISIBLE:
			if (IsDlgButtonChecked(hwnd, IDC_ALWAYS_VISIBLE)) {
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				EnableWindow(GetDlgItem(hwnd, IDC_VISIBLE_UNLESS_OFFLINE), 1);
				CheckDlgButton(hwnd, IDC_VISIBLE_UNLESS_OFFLINE, g_plugin.getByte(hContact, "VisibleUnlessOffline", 1) ? BST_CHECKED : BST_UNCHECKED);
			}
			else EnableWindow(GetDlgItem(hwnd, IDC_VISIBLE_UNLESS_OFFLINE), 0);
			break;

		case CHK_USE_TIMER:
			if (IsDlgButtonChecked(hwnd, CHK_USE_TIMER)) {
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				char string[4];
				EnableWindow(GetDlgItem(hwnd, IDC_TIMER), 1);
				SetDlgItemTextA(hwnd, IDC_TIMER, _itoa(g_plugin.getWord(hContact, "Timer", 15), string, 10));
			}
			else EnableWindow(GetDlgItem(hwnd, IDC_TIMER), 0);
			break;

		case IDC_OPEN_FILE:
			char szFileName[512];
			if (Openfile(szFileName, 1))
				SetDlgItemTextA(hwnd, IDC_LINK, szFileName);
			break;

		case IDC_OPEN_FOLDER:
			if (BrowseForFolder(hwnd, szFileName)) {
				mir_snprintf(szFileName, "%s ,/e", szFileName);
				SetDlgItemTextA(hwnd, IDC_LINK, "explorer.exe");
				SetDlgItemTextA(hwnd, IDC_PARAMS, szFileName);
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				int status = GetLCStatus(0, 0);
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);

				if (GetWindowTextLength(GetDlgItem(hwnd, IDC_LINK))) {
					char text[512];
					GetDlgItemTextA(hwnd, IDC_LINK, text, _countof(text));
					g_plugin.setString(hContact, "ProgramString", text);
					WriteSetting(hContact, MODNAME, "ProgramString", MODNAME, "Program");
				}
				else g_plugin.delSetting(hContact, "ProgramString");

				if (GetWindowTextLength(GetDlgItem(hwnd, IDC_PARAMS))) {
					char text[512];
					GetDlgItemTextA(hwnd, IDC_PARAMS, text, _countof(text));
					g_plugin.setString(hContact, "ProgramParamsString", text);
					WriteSetting(hContact, MODNAME, "ProgramParamsString", MODNAME, "ProgramParams");
				}
				else g_plugin.delSetting(hContact, "ProgramParamsString");

				if (GetWindowTextLength(GetDlgItem(hwnd, IDC_GROUP))) {
					wchar_t text[512];
					GetDlgItemText(hwnd, IDC_GROUP, text, _countof(text));
					Clist_GroupCreate(NULL, text);
					Clist_SetGroup(hContact, text);
				}
				else Clist_SetGroup(hContact, nullptr);

				for (int i = ID_STATUS_ONLINE; i <= ID_STATUS_MAX; i++)
					if (IsDlgButtonChecked(hwnd, i))
						g_plugin.setWord(hContact, "Icon", (uint16_t)i);

				/* set correct status */
				if (status == ID_STATUS_ONLINE || status == ID_STATUS_AWAY || (status == g_plugin.getWord(hContact, "Icon", ID_STATUS_ONLINE)))
					g_plugin.setWord(hContact, "Status", (uint16_t)g_plugin.getWord(hContact, "Icon", ID_STATUS_ONLINE));
				else
					g_plugin.setWord(hContact, "Status", ID_STATUS_OFFLINE);

				if (IsDlgButtonChecked(hwnd, CHK_USE_TIMER)) {
					if (GetWindowTextLength(GetDlgItem(hwnd, IDC_TIMER))) {
						wchar_t text[512];
						GetDlgItemText(hwnd, IDC_TIMER, text, _countof(text));
						g_plugin.setWord(hContact, "Timer", (uint16_t)_wtoi(text));
					}
					else g_plugin.setWord(hContact, "Timer", 15);
				}
				else g_plugin.setWord(hContact, "Timer", 0);

				// always visible
				g_plugin.setByte(hContact, "AlwaysVisible", (uint8_t)IsDlgButtonChecked(hwnd, IDC_ALWAYS_VISIBLE));
				g_plugin.setByte(hContact, "VisibleUnlessOffline", (uint8_t)IsDlgButtonChecked(hwnd, IDC_VISIBLE_UNLESS_OFFLINE));
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

char* copyReplaceString(char* oldStr, char* newStr, char* findStr, char* replaceWithStr)
{
	int i = 0;
	while (oldStr[i] != '\0') {
		//	msg(&oldStr[i],"");
		if (!strncmp(&oldStr[i], findStr, mir_strlen(findStr))) {
			mir_strcat(newStr, replaceWithStr);
			i += (int)mir_strlen(findStr);
		}
		else {
			strncat(newStr, &oldStr[i], 1);
			i++;
		}
	}
	return newStr;
}

#define MAX_REPLACES 15

INT_PTR CALLBACK DlgProcCopy(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, ((PROPSHEETPAGE*)lParam)->lParam);
		return TRUE;

	case WM_COMMAND:
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		switch (LOWORD(wParam)) {
		case IDC_EXPORT:
			ExportContact((MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA));
			break;

		case IDC_DOIT:
			if (GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING_REPLACE))) {
				char newString[MAX_REPLACES][512], oldString[MAX_REPLACES][512];
				char dbVar1[2000], dbVar2[2000];
				int i = 0, j = 0, k = 0;
				char *string = oldString[k];
				MCONTACT hContact1 = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				if (!db_get_static(hContact1, MODNAME, "Name", dbVar1, _countof(dbVar1))) {
					char *replace = (char*)malloc(GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING_REPLACE)) + 1);
					GetDlgItemTextA(hwnd, IDC_STRING_REPLACE, replace, GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING_REPLACE)) + 1);
					// get the list of replace strings
					while (replace[i] != '\0') {
						if (replace[i] == ',') {
							string = newString[k];
							j = 0;
						}
						else if (!strncmp(replace + i, "\r\n", 2)) {
							if (string == newString[k])
								k--;
							if (k == MAX_REPLACES)
								break;
							string = oldString[++k];
							i += 2;
							continue;
						}
						else {
							string[j] = replace[i];
							string[++j] = '\0';
						}
						i++;
					}
					free(replace);
					MCONTACT hContact2 = db_add_contact();
					Proto_AddToContact(hContact2, MODNAME);
					Ignore_Ignore(hContact2, IGNOREEVENT_USERONLINE);
					g_plugin.setString(hContact2, "Nick", Translate("New Non-IM Contact"));
					// blank dbVar2 so the replaceing doesnt crash..
					mir_strcpy(dbVar2, "");
					// copy the name (dbVar1 is the name)
					for (i = 0; i < k; i++)
						copyReplaceString(dbVar1, dbVar2, oldString[i], newString[i]);

					g_plugin.setString(hContact2, "Name", dbVar2);
					// copy the ProgramString
					if (!db_get_static(hContact1, MODNAME, "ProgramString", dbVar1, _countof(dbVar1))) {
						mir_strcpy(dbVar2, "");
						for (i = 0; i <= k; i++)
							copyReplaceString(dbVar1, dbVar2, oldString[i], newString[i]);

						g_plugin.setString(hContact2, "ProgramString", dbVar2);
					}
					// copy the ProgramParamString
					if (!db_get_static(hContact1, MODNAME, "ProgramParamString", dbVar1, _countof(dbVar1))) {
						mir_strcpy(dbVar2, "");
						for (i = 0; i <= k; i++)
							copyReplaceString(dbVar1, dbVar2, oldString[i], newString[i]);

						g_plugin.setString(hContact2, "ProgramParamString", dbVar2);
					}
					// copy the group
					if (!db_get_static(hContact1, "CList", "Group", dbVar1, _countof(dbVar1))) {
						mir_strcpy(dbVar2, "");
						for (i = 0; i <= k; i++)
							copyReplaceString(dbVar1, dbVar2, oldString[i], newString[i]);

						db_set_s(hContact2, "CList", "Group", dbVar2);
					}
					// copy the ToolTip
					if (!db_get_static(hContact1, MODNAME, "ToolTip", dbVar1, _countof(dbVar1))) {
						mir_strcpy(dbVar2, "");
						for (i = 0; i <= k; i++)
							copyReplaceString(dbVar1, dbVar2, oldString[i], newString[i]);

						g_plugin.setString(hContact2, "ToolTip", dbVar2);
					}
					// timer
					g_plugin.setByte(hContact2, "UseTimer", g_plugin.getByte(hContact1, "UseTimer"));
					g_plugin.setByte(hContact2, "Minutes", g_plugin.getByte(hContact1, "Minutes"));
					g_plugin.setWord(hContact2, "Timer", g_plugin.getWord(hContact1, "Timer"));
					//icon
					g_plugin.setWord(hContact2, "Icon", g_plugin.getWord(hContact1, "Icon", 40072));
					replaceAllStrings(hContact2);
				}
			}
			else {
				char dbVar1[2000];
				MCONTACT hContact1 = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				if (!db_get_static(hContact1, MODNAME, "Name", dbVar1, _countof(dbVar1))) {
					MCONTACT hContact2 = db_add_contact();
					if (!hContact2) {
						msg("contact did not get created", "");
						return 0;
					}
					Proto_AddToContact(hContact2, MODNAME);
					Ignore_Ignore(hContact2, IGNOREEVENT_USERONLINE);
					g_plugin.setString(hContact2, "Nick", Translate("New Non-IM Contact"));
					g_plugin.setString(hContact2, "Name", dbVar1);
					if (!db_get_static(hContact1, MODNAME, "ProgramString", dbVar1, _countof(dbVar1)))
						g_plugin.setString(hContact2, "ProgramString", dbVar1);

					// copy the ProgramParamString
					if (!db_get_static(hContact1, MODNAME, "ProgramParamString", dbVar1, _countof(dbVar1)))
						g_plugin.setString(hContact2, "ProgramParamString", dbVar1);

					// copy the group
					if (!db_get_static(hContact1, "CList", "Group", dbVar1, _countof(dbVar1)))
						db_set_s(hContact2, "CList", "Group", dbVar1);

					// copy the ToolTip
					if (!db_get_static(hContact1, MODNAME, "ToolTip", dbVar1, _countof(dbVar1)))
						g_plugin.setString(hContact2, "ToolTip", dbVar1);

					// timer
					g_plugin.setByte(hContact2, "UseTimer", g_plugin.getByte(hContact1, "UseTimer"));
					g_plugin.setByte(hContact2, "Minutes", g_plugin.getByte(hContact1, "Minutes"));
					g_plugin.setWord(hContact2, "Timer", g_plugin.getWord(hContact1, "Timer"));

					//icon
					g_plugin.setWord(hContact2, "Icon", g_plugin.getWord(hContact1, "Icon", 40072));
					replaceAllStrings(hContact2);
				}
			}
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

void ExportContact(MCONTACT hContact)
{
	char szFileName[MAX_PATH];
	char DBVar[1024];
	int tmp;

	if (Openfile(szFileName, 0)) {
		//	if (tmp = MessageBox(0, "Do you want to overwrite the contents of the file?\r\n\r\nPressing No will append this contact to the end of the file.",modFullname, MB_YESNO) == IDYES)
		//		file = fopen(szFileName, "w");
		//	else 
		FILE *file = fopen(szFileName, "a");
		if (file) {
			if (!db_get_static(hContact, MODNAME, "Name", DBVar, _countof(DBVar))) {
				fprintf(file, "\r\n[Non-IM Contact]\r\nName=%s\r\n", DBVar);
				if (!db_get_static(hContact, MODNAME, "ProgramString", DBVar, _countof(DBVar)))
					fprintf(file, "ProgramString=%s\r\n", DBVar);
				if (!db_get_static(hContact, MODNAME, "ProgramParamString", DBVar, _countof(DBVar)))
					fprintf(file, "ProgramParamString=%s\r\n", DBVar);
				if (!db_get_static(hContact, MODNAME, "ToolTip", DBVar, _countof(DBVar)))
					fprintf(file, "ToolTip=%s</tooltip>\r\n", DBVar);
				if (!db_get_static(hContact, "CList", "Group", DBVar, _countof(DBVar)))
					fprintf(file, "Group=%s\r\n", DBVar);
				if (tmp = g_plugin.getWord(hContact, "Icon", 40072))
					fprintf(file, "Icon=%d\r\n", tmp);
				if (tmp = g_plugin.getByte(hContact, "UseTimer", 0))
					fprintf(file, "UseTimer=%d\r\n", tmp);
				if (tmp = g_plugin.getByte(hContact, "Minutes", 1))
					fprintf(file, "Minutes=%d\r\n", tmp);
				if (tmp = g_plugin.getWord(hContact, "Timer", 0))
					fprintf(file, "Timer=%d\r\n", tmp);
				fprintf(file, "[/Non-IM Contact]\r\n");
			}
			else ("Contact is invalid", modFullname);
			fclose(file);
		}
	}
}

INT_PTR ImportContacts(WPARAM, LPARAM)
{
	MCONTACT hContact;
	char name[256] = "", program[256] = "", programparam[256] = "", group[256] = "", line[2001] = "";
	int icon = 40072, usetimer = 0, minutes = 1, timer = 0;
	char fn[MAX_PATH];
	int i, j, contactDone = 0;
	if (!Openfile(fn, 1))
		return 1;

	FILE *file = fopen(fn, "r");
	if (!file)
		return 1;

	CMStringA tooltip;

	while (fgets(line, 2000, file)) {
		if (!mir_strcmp(line, "\r\n\0"))
			continue;
		if (!mir_strcmp(line, "[Non-IM Contact]\r\n"))
			contactDone = 0;
		else if (!strncmp(line, "Name=", mir_strlen("Name="))) {
			i = (int)mir_strlen("Name="); j = 0;
			while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0') {
				name[j] = line[i++];
				name[++j] = '\0';
			}
			contactDone = 1;
		}
		else if (!strncmp(line, "ProgramString=", mir_strlen("ProgramString="))) {
			i = (int)mir_strlen("ProgramString="); j = 0;
			while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0') {
				program[j] = line[i++];
				program[++j] = '\0';
			}
		}
		else if (!strncmp(line, "ProgramParamString=", mir_strlen("ProgramParamString="))) {
			i = (int)mir_strlen("ProgramParamString="); j = 0;
			while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0') {
				programparam[j] = line[i++];
				programparam[++j] = '\0';
			}
		}
		else if (!strncmp(line, "Group=", mir_strlen("Group="))) {
			i = (int)mir_strlen("Group="); j = 0;
			while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0') {
				group[j] = line[i++];
				group[++j] = '\0';
			}
		}
		else if (!strncmp(line, "ToolTip=", mir_strlen("ToolTip="))) {
			i = (int)mir_strlen("ToolTip=");
			tooltip = &line[i];
			fgets(line, 2000, file);
			while (!strstr(line, "</tooltip>\r\n")) {
				tooltip.Append(line);
				fgets(line, 2000, file);
			}
			// the line that has the </tooltip>
			tooltip.Append(line);
		}
		else if (!strncmp(line, "Icon=", mir_strlen("Icon="))) {
			i = (int)mir_strlen("Icon=");
			sscanf(&line[i], "%d", &icon);
		}
		else if (!strncmp(line, "UseTimer=", mir_strlen("UseTimer="))) {
			i = (int)mir_strlen("UseTimer=");
			sscanf(&line[i], "%d", &usetimer);
		}
		else if (!strncmp(line, "Timer=", mir_strlen("Timer="))) {
			i = (int)mir_strlen("Timer=");
			sscanf(&line[i], "%d", &timer);
		}
		else if (!strncmp(line, "Minutes=", mir_strlen("Minutes="))) {
			i = (int)mir_strlen("Minutes=");
			sscanf(&line[i], "%d", &minutes);
		}
		else if (contactDone && !mir_strcmp(line, "[/Non-IM Contact]\r\n")) {
			if (!name) continue;
			size_t size = mir_strlen(name) + mir_strlen("Do you want to import this Non-IM Contact?\r\n\r\nName: \r\n") + 1;
			char *msg = (char*)malloc(size);
			mir_snprintf(msg, size, "Do you want to import this Non-IM Contact?\r\n\r\nName: %s\r\n", name);
			if (program[0] != '\0') {
				msg = (char*)realloc(msg, mir_strlen(msg) + mir_strlen(program) + mir_strlen("Program: \r\n") + 1);
				mir_strcat(msg, "Program: ");
				mir_strcat(msg, program);
				mir_strcat(msg, "\r\n");
			}
			if (programparam[0] != '\0') {
				msg = (char*)realloc(msg, mir_strlen(msg) + mir_strlen(programparam) + mir_strlen("Program Parameters: \r\n") + 1);
				mir_strcat(msg, "Program Parameters: ");
				mir_strcat(msg, programparam);
				mir_strcat(msg, "\r\n");
			}
			if (tooltip) {
				msg = (char*)realloc(msg, mir_strlen(msg) + mir_strlen(tooltip) + mir_strlen("ToolTip: \r\n") + 1);
				mir_strcat(msg, "ToolTip: ");
				mir_strcat(msg, tooltip);
				mir_strcat(msg, "\r\n");
			}
			if (group[0] != '\0') {
				msg = (char*)realloc(msg, mir_strlen(msg) + mir_strlen(group) + mir_strlen("Group: \r\n") + 1);
				mir_strcat(msg, "Group: ");
				mir_strcat(msg, group);
				mir_strcat(msg, "\r\n");
			}
			if (icon) {
				char tmp[64];
				if (icon == ID_STATUS_ONLINE)
					mir_snprintf(tmp, "Icon: Online\r\n");
				else if (icon == ID_STATUS_AWAY)
					mir_snprintf(tmp, "Icon: Away\r\n");
				else if (icon == ID_STATUS_NA)
					mir_snprintf(tmp, "Icon: N/A\r\n");
				else if (icon == ID_STATUS_DND)
					mir_snprintf(tmp, "Icon: DND\r\n");
				else if (icon == ID_STATUS_OCCUPIED)
					mir_snprintf(tmp, "Icon: Occupied\r\n");
				else if (icon == ID_STATUS_FREECHAT)
					mir_snprintf(tmp, "Icon: Free for chat\r\n");
				else if (icon == ID_STATUS_INVISIBLE)
					mir_snprintf(tmp, "Icon: Invisible\r\n");
				else {
					free(msg);
					continue;
				}
				char *msgtemp = (char*)realloc(msg, mir_strlen(msg) + mir_strlen(tmp) + 1);
				if (msgtemp) {
					msg = msgtemp;
					mir_strcat(msg, tmp);
				}
			}
			if (usetimer && timer) {
				char tmp[64], tmp2[8];
				if (minutes)
					mir_strcpy(tmp2, "Minutes");
				else mir_strcpy(tmp2, "Seconds");
				mir_snprintf(tmp, "UseTimer: Yes\r\nTimer: %d %s", timer, tmp2);
				char *msgtemp = (char*)realloc(msg, mir_strlen(msg) + mir_strlen(tmp) + 1);
				if (msgtemp) {
					msg = msgtemp;
					mir_strcat(msg, tmp);
				}
			}

			if (MessageBoxA(nullptr, msg, modFullname, MB_YESNO) == IDYES) {
				if (!(hContact = db_add_contact())) {
					msg("contact did get created", "");
					continue;
				}
				Proto_AddToContact(hContact, MODNAME);
				Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);
				g_plugin.setString(hContact, "Nick", Translate("New Non-IM Contact"));
				g_plugin.setString(hContact, "Name", name);
				g_plugin.setString(hContact, "ProgramString", program);
				// copy the ProgramParamString
				g_plugin.setString(hContact, "ProgramParamString", programparam);
				// copy the group
				db_set_s(hContact, "CList", "Group", group);
				// copy the ToolTip
				g_plugin.setString(hContact, "ToolTip", tooltip);
				// timer
				g_plugin.setByte(hContact, "UseTimer", (uint8_t)usetimer);
				g_plugin.setByte(hContact, "Minutes", (uint8_t)minutes);
				g_plugin.setWord(hContact, "Timer", (uint16_t)timer);
				//icon
				g_plugin.setWord(hContact, "Icon", (uint16_t)icon);
				replaceAllStrings(hContact);
			}
			free(msg);
			contactDone = 0;
			name[0] = '\0';
			program[0] = '\0';
			programparam[0] = '\0';
			group[0] = '\0';
			line[0] = '\0';
			tooltip.Empty();
			icon = 40072;
			usetimer = 0;
			minutes = 1;
			timer = 0;
		}
	}
	fclose(file);

	return 1;
}
