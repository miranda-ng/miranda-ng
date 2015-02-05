#include "commonheaders.h"

INT_PTR CALLBACK DlgProcContactInfo(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			MCONTACT hContact = (MCONTACT)((PROPSHEETPAGE*)lParam)->lParam;
			char name[2048];
			TranslateDialogDefault(hwnd);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hContact);

			if (!db_get_static(hContact, MODNAME, "Name", name)) break;
			SetDlgItemTextA(hwnd, IDC_DISPLAY_NAME, name);
			if (!db_get_static(hContact, MODNAME, "ToolTip", name)) break;
			SetDlgItemTextA(hwnd, IDC_TOOLTIP, name);
		}
		return TRUE;

	case WM_COMMAND:
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		return TRUE;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				if (GetWindowTextLength(GetDlgItem(hwnd,IDC_DISPLAY_NAME))) {
					char text[512];
					GetDlgItemTextA(hwnd,IDC_DISPLAY_NAME,text,SIZEOF(text));
					db_set_s(hContact, MODNAME, "Name", text);
					WriteSetting(hContact, MODNAME, "Name", MODNAME, "Nick");
				}
				else {
					db_unset(hContact, MODNAME, "Name");
					db_unset(hContact, MODNAME, "Nick");
				}

				if (GetWindowTextLength(GetDlgItem(hwnd,IDC_TOOLTIP))) {
					char text[2048];
					GetDlgItemTextA(hwnd,IDC_TOOLTIP,text,SIZEOF(text));
					db_set_s(hContact, MODNAME, "ToolTip", text);
					WriteSetting(hContact, MODNAME, "ToolTip", "UserInfo", "MyNotes");
				}
				else {
					db_unset(hContact, MODNAME, "ToolTip");
					db_unset(hContact, "UserInfo", "MyNotes");
				}
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static WNDPROC g_PrevBtnWndProc = 0;

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
		DrawIconEx(dc, rc.left, rc.top, (HICON)GetWindowLongPtr(hWnd, GWLP_USERDATA), 16, 16, 0, 0, DI_NORMAL);
		ReleaseDC(hWnd, dc);
	}

	return res;
}

int BrowseForFolder(HWND hwnd,char *szPath)
{
	int result=0;
	LPMALLOC pMalloc;

	if (SUCCEEDED(CoGetMalloc(1,&pMalloc))) {
		ptrT tszPath( mir_a2t(szPath));
		BROWSEINFO bi={0};
		bi.hwndOwner = hwnd;
		bi.pszDisplayName = tszPath;
		bi.lpszTitle = TranslateT("Select Folder");
		bi.ulFlags = BIF_EDITBOX | BIF_RETURNONLYFSDIRS;				// Use this combo instead of BIF_USENEWUI
		bi.lParam = (LPARAM)szPath;

		ITEMIDLIST *pidlResult = SHBrowseForFolder(&bi);
		if (pidlResult) {
			SHGetPathFromIDListA(pidlResult, szPath);
			mir_strcat(szPath,"\\");
			result = 1;
		}
		pMalloc->Free(pidlResult);
		pMalloc->Release();
	}

	return result;
}

INT_PTR CALLBACK DlgProcOtherStuff(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			MCONTACT hContact = (MCONTACT)((PROPSHEETPAGE*)lParam)->lParam;
			TranslateDialogDefault(hwnd);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hContact);
			if (!hContact)
				break;

			/* link*/
			DBVARIANT dbv;
			if (!db_get_ts(hContact, MODNAME, "ProgramString", &dbv))
			{
				SetDlgItemText(hwnd, IDC_LINK, dbv.ptszVal);
				db_free(&dbv);
			}

			if (!db_get_ts(hContact, MODNAME, "ProgramParamsString", &dbv))
			{
				SetDlgItemText(hwnd, IDC_PARAMS, dbv.ptszVal);
				db_free(&dbv);
			}

			/* group*/
			TCHAR *szGroup;
			for (int i=1; (szGroup = pcli->pfnGetGroupName(i, NULL)) != NULL; i++)
				SendDlgItemMessage(hwnd, IDC_GROUP, CB_INSERTSTRING,0, LPARAM(szGroup));

			if (!db_get_ts(hContact, "CList", "Group", &dbv))
			{
				SetDlgItemText(hwnd, IDC_GROUP, dbv.ptszVal);
				db_free(&dbv);
			}

			/* icons */
			CheckRadioButton(hwnd, 40072, 40080, db_get_w(hContact, MODNAME, "Icon", ID_STATUS_ONLINE));
			SetWindowLongPtr(GetDlgItem(hwnd, CHK_ONLINE), GWLP_USERDATA, (LONG_PTR)LoadSkinnedProtoIcon(MODNAME, ID_STATUS_ONLINE));
			g_PrevBtnWndProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwnd, CHK_ONLINE), GWLP_WNDPROC, (LONG_PTR)ButtWndProc);
			for (int i = ID_STATUS_ONLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
				SetWindowLongPtr(GetDlgItem(hwnd, i), GWLP_USERDATA, (LONG_PTR)LoadSkinnedProtoIcon(MODNAME, i));
				SetWindowLongPtr(GetDlgItem(hwnd, i), GWLP_WNDPROC, (LONG_PTR)ButtWndProc);
			}
			db_free(&dbv);
			/* timer */
			CheckDlgButton(hwnd, CHK_USE_TIMER, db_get_b(hContact, MODNAME ,"UseTimer", 0) ? BST_CHECKED : BST_UNCHECKED);
			if (db_get_w(hContact, MODNAME ,"Timer", 15)) {
				CheckDlgButton(hwnd, CHK_USE_TIMER, BST_CHECKED);
				EnableWindow(GetDlgItem(hwnd, IDC_TIMER), 1);
				TCHAR string[512];
				SetDlgItemText(hwnd, IDC_TIMER, _itot(db_get_w(hContact, MODNAME ,"Timer", 15), string, 10));
				if (!db_get_w(NULL, MODNAME ,"Timer", 1))
					SetDlgItemText(hwnd,IDC_TIMER_INTERVAL_MSG, TranslateT("Non-IM Contact protocol timer is Disabled"));
				else {
					mir_sntprintf(string, SIZEOF(string), TranslateT("Timer intervals... Non-IM Contact Protocol timer is %d seconds"),db_get_w(NULL, MODNAME ,"Timer", 1));
					SetDlgItemText(hwnd,IDC_TIMER_INTERVAL_MSG, string);
				}
			}
			/* always visible */
			if (db_get_b(hContact, MODNAME ,"AlwaysVisible", 0)) {
				CheckDlgButton(hwnd, IDC_ALWAYS_VISIBLE, BST_CHECKED);
				EnableWindow(GetDlgItem(hwnd, IDC_VISIBLE_UNLESS_OFFLINE),1);
				CheckDlgButton(hwnd, IDC_VISIBLE_UNLESS_OFFLINE, db_get_b(hContact, MODNAME ,"VisibleUnlessOffline", 1) ? BST_CHECKED : BST_UNCHECKED);
			}
		}
		return TRUE;

	case WM_COMMAND:
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		switch(LOWORD(wParam)) {
		case IDC_ALWAYS_VISIBLE:
			if (IsDlgButtonChecked(hwnd, IDC_ALWAYS_VISIBLE)) {
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				EnableWindow(GetDlgItem(hwnd, IDC_VISIBLE_UNLESS_OFFLINE),1);
				CheckDlgButton(hwnd, IDC_VISIBLE_UNLESS_OFFLINE, db_get_b(hContact, MODNAME ,"VisibleUnlessOffline", 1) ? BST_CHECKED : BST_UNCHECKED);
			}
			else EnableWindow(GetDlgItem(hwnd, IDC_VISIBLE_UNLESS_OFFLINE),0);
			break;

		case CHK_USE_TIMER:
			if (IsDlgButtonChecked(hwnd, CHK_USE_TIMER)) {
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				char string[4];
				EnableWindow(GetDlgItem(hwnd, IDC_TIMER), 1);
				SetDlgItemTextA(hwnd, IDC_TIMER, _itoa(db_get_w(hContact, MODNAME ,"Timer", 15), string, 10));
			}
			else EnableWindow(GetDlgItem(hwnd, IDC_TIMER), 0);
			break;

		case IDC_OPEN_FILE:
			{
				char szFileName[512];
				if ( Openfile(szFileName,1))
					SetDlgItemTextA(hwnd, IDC_LINK, szFileName);
			}
			break;

		case IDC_OPEN_FOLDER:
			{
				char szFileName[512];
				if (BrowseForFolder(hwnd, szFileName)) {
					mir_snprintf(szFileName, SIZEOF(szFileName), "%s ,/e", szFileName);
					SetDlgItemTextA(hwnd, IDC_LINK, "explorer.exe");
					SetDlgItemTextA(hwnd, IDC_PARAMS, szFileName);
				}
			}
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				int status = GetLCStatus(0,0);
				MCONTACT hContact = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);

				if (GetWindowTextLength(GetDlgItem(hwnd,IDC_LINK))) {
					char text[512];
					GetDlgItemTextA(hwnd,IDC_LINK,text,SIZEOF(text));
					db_set_s(hContact, MODNAME, "ProgramString", text);
					WriteSetting(hContact, MODNAME, "ProgramString", MODNAME, "Program");
				}
				else db_unset(hContact, MODNAME, "ProgramString");

				if (GetWindowTextLength(GetDlgItem(hwnd,IDC_PARAMS))) {
					char text[512];
					GetDlgItemTextA(hwnd,IDC_PARAMS,text,SIZEOF(text));
					db_set_s(hContact, MODNAME, "ProgramParamsString", text);
					WriteSetting(hContact, MODNAME, "ProgramParamsString", MODNAME, "ProgramParams");
				}
				else db_unset(hContact, MODNAME, "ProgramParamsString");

				if (GetWindowTextLength(GetDlgItem(hwnd,IDC_GROUP))) {
					TCHAR text[512];
					GetDlgItemText(hwnd, IDC_GROUP, text, SIZEOF(text));
					Clist_CreateGroup(NULL, text);
					db_set_ts(hContact, "CList", "Group", text);
				}
				else db_unset(hContact, "CList", "Group");

				for (int i = ID_STATUS_ONLINE; i<=ID_STATUS_OUTTOLUNCH; i++)
					if (IsDlgButtonChecked(hwnd, i))
						db_set_w(hContact, MODNAME, "Icon", (WORD)i);

				/* set correct status */
				if (status == ID_STATUS_ONLINE || status == ID_STATUS_AWAY || (status == db_get_w(hContact, MODNAME, "Icon", ID_STATUS_ONLINE)))
					db_set_w(hContact, MODNAME, "Status", (WORD)db_get_w(hContact, MODNAME, "Icon", ID_STATUS_ONLINE));
				else
					db_set_w(hContact, MODNAME, "Status", ID_STATUS_OFFLINE);

				if (IsDlgButtonChecked(hwnd, CHK_USE_TIMER)) {
					if (GetWindowTextLength(GetDlgItem(hwnd,IDC_TIMER))) {
						TCHAR text[512];
						GetDlgItemText(hwnd,IDC_TIMER,text,SIZEOF(text));
						db_set_w(hContact, MODNAME, "Timer", (WORD)_ttoi(text));
					}
					else db_set_w(hContact, MODNAME, "Timer", 15);
				}
				else db_set_w(hContact, MODNAME, "Timer", 0);

				// always visible
				db_set_b(hContact, MODNAME, "AlwaysVisible", (BYTE)IsDlgButtonChecked(hwnd, IDC_ALWAYS_VISIBLE));
				db_set_b(hContact, MODNAME, "VisibleUnlessOffline", (BYTE)IsDlgButtonChecked(hwnd, IDC_VISIBLE_UNLESS_OFFLINE));
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

char* copyReplaceString(char* oldStr, char* newStr, char* findStr, char* replaceWithStr)
{
	int i=0;
	while (oldStr[i] != '\0') {
		//	msg(&oldStr[i],"");
		if (!strncmp(&oldStr[i],findStr, strlen(findStr))) {
			strcat(newStr,replaceWithStr);
			i += (int)strlen(findStr);
		}
		else {
			strncat(newStr,&oldStr[i],1);
			i++;
		}
	}
	return newStr;
}

#define MAX_REPLACES 15

INT_PTR CALLBACK DlgProcCopy(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			MCONTACT hContact = (MCONTACT)((PROPSHEETPAGE*)lParam)->lParam;
			TranslateDialogDefault(hwnd);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)hContact);
		}
		return TRUE;

	case WM_COMMAND:
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		switch(LOWORD(wParam)) {
		case IDC_EXPORT:
			ExportContact((MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA));
			break;

		case IDC_DOIT:
			if (GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING_REPLACE))) {
				char newString[MAX_REPLACES][512], oldString[MAX_REPLACES][512];
				char dbVar1[2000], dbVar2[2000];
				int i=0,j=0, k=0;
				char *string = oldString[k];
				MCONTACT hContact1 = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA), hContact2;
				if (db_get_static(hContact1, MODNAME, "Name", dbVar1)) {
					char *replace = (char*)malloc(GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING_REPLACE)) +1);
					GetDlgItemTextA(hwnd, IDC_STRING_REPLACE, replace, GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING_REPLACE)) +1);
					// get the list of replace strings
					while (replace[i] != '\0') {
						if (replace[i] == ',') {
							string = newString[k];
							j=0;
						}
						else if (!strncmp(replace + i, "\r\n",2)) {
							if (string == newString[k])
								k--;
							if (k == MAX_REPLACES) break;
							string = oldString[++k];
							i+=2;
							continue;
						}
						else {
							string[j] = replace[i];
							string[++j] = '\0';
						}
						i++;
					}
					free(replace);
					hContact2 =(MCONTACT) CallService(MS_DB_CONTACT_ADD, 0, 0);
					CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hContact2,(LPARAM)MODNAME);
					CallService(MS_IGNORE_IGNORE, (WPARAM)hContact2, IGNOREEVENT_USERONLINE);
					db_set_s(hContact2, MODNAME, "Nick", Translate("New Non-IM Contact"));
					// blank dbVar2 so the replaceing doesnt crash..
					strcpy(dbVar2, "");
					// copy the name (dbVar1 is the name)
					for (i=0;i<=k;i++)
						copyReplaceString(dbVar1, dbVar2, oldString[i],newString[i] );

					db_set_s(hContact2, MODNAME, "Name", dbVar2);
					// copy the ProgramString
					if (db_get_static(hContact1, MODNAME, "ProgramString",dbVar1)) {
						strcpy(dbVar2, "");
						for (i=0;i<=k;i++)
							copyReplaceString(dbVar1, dbVar2, oldString[i],newString[i] );

						db_set_s(hContact2, MODNAME, "ProgramString", dbVar2);
					}
					// copy the ProgramParamString
					if (db_get_static(hContact1, MODNAME, "ProgramParamString",dbVar1)) {
						strcpy(dbVar2, "");
						for (i=0;i<=k;i++)
							copyReplaceString(dbVar1, dbVar2, oldString[i],newString[i] );

						db_set_s(hContact2, MODNAME, "ProgramParamString", dbVar2);
					}
					// copy the group
					if (db_get_static(hContact1, "CList", "Group",dbVar1)) {
						strcpy(dbVar2, "");
						for (i=0;i<=k;i++)
							copyReplaceString(dbVar1, dbVar2, oldString[i],newString[i] );

						db_set_s(hContact2, "CList", "Group", dbVar2);
					}
					// copy the ToolTip
					if (db_get_static(hContact1, MODNAME, "ToolTip",dbVar1)) {
						strcpy(dbVar2, "");
						for (i=0;i<=k;i++)
							copyReplaceString(dbVar1, dbVar2, oldString[i],newString[i] );

						db_set_s(hContact2, MODNAME, "ToolTip", dbVar2);
					}
					// timer
					db_set_b(hContact2, MODNAME, "UseTimer", (BYTE)db_get_b(hContact1, MODNAME, "UseTimer", 0));
					db_set_b(hContact2, MODNAME, "Minutes", (BYTE)db_get_b(hContact1, MODNAME, "Minutes", 0));
					db_set_w(hContact2, MODNAME, "Timer", (WORD)db_get_w(hContact1, MODNAME, "Timer", 0));
					//icon
					db_set_w(hContact2, MODNAME, "Icon", (WORD)db_get_w(hContact1, MODNAME, "Icon", 40072));
					replaceAllStrings(hContact2);
				}	
			}
			else {
				char dbVar1[2000];
				MCONTACT hContact1 = (MCONTACT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
				if (db_get_static(hContact1, MODNAME, "Name", dbVar1)) {
					MCONTACT hContact2 = (MCONTACT) CallService(MS_DB_CONTACT_ADD, 0, 0);
					if (!hContact2) {
						msg("contact did not get created","");
						return 0;
					}
					CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hContact2,(LPARAM)MODNAME);
					CallService(MS_IGNORE_IGNORE, (WPARAM)hContact2, IGNOREEVENT_USERONLINE);
					db_set_s(hContact2, MODNAME, "Nick", Translate("New Non-IM Contact"));
					db_set_s(hContact2, MODNAME, "Name", dbVar1);
					if (db_get_static(hContact1, MODNAME, "ProgramString",dbVar1))
						db_set_s(hContact2, MODNAME, "ProgramString", dbVar1);

					// copy the ProgramParamString
					if (db_get_static(hContact1, MODNAME, "ProgramParamString",dbVar1))
						db_set_s(hContact2, MODNAME, "ProgramParamString", dbVar1);

					// copy the group
					if (db_get_static(hContact1, "CList", "Group",dbVar1))
						db_set_s(hContact2, "CList", "Group", dbVar1);

					// copy the ToolTip
					if (db_get_static(hContact1, MODNAME, "ToolTip",dbVar1))
						db_set_s(hContact2, MODNAME, "ToolTip", dbVar1);

					// timer
					db_set_b(hContact2, MODNAME, "UseTimer", (BYTE)db_get_b(hContact1, MODNAME, "UseTimer", 0));
					db_set_b(hContact2, MODNAME, "Minutes", (BYTE)db_get_b(hContact1, MODNAME, "Minutes", 0));
					db_set_w(hContact2, MODNAME, "Timer", (WORD)db_get_w(hContact1, MODNAME, "Timer", 0));

					//icon
					db_set_w(hContact2, MODNAME, "Icon", (WORD)db_get_w(hContact1, MODNAME, "Icon", 40072));
					replaceAllStrings(hContact2);
				}
			}
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
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

	if (Openfile(szFileName, 0))
	{
		//	if (tmp = MessageBox(0, "Do you want to overwrite the contents of the file?\r\n\r\nPressing No will append this contact to the end of the file.",modFullname, MB_YESNO) == IDYES)
		//		file = fopen(szFileName, "w");
		//	else 
		FILE *file = fopen(szFileName, "a");
		if (file)
		{
			if (db_get_static(hContact, MODNAME, "Name", DBVar))
			{
				fprintf(file, "\r\n[Non-IM Contact]\r\nName=%s\r\n", DBVar);
				if (db_get_static(hContact, MODNAME, "ProgramString", DBVar))
					fprintf(file, "ProgramString=%s\r\n", DBVar);
				if (db_get_static(hContact, MODNAME, "ProgramParamString", DBVar))
					fprintf(file, "ProgramParamString=%s\r\n", DBVar);
				if (db_get_static(hContact, MODNAME, "ToolTip", DBVar))
					fprintf(file, "ToolTip=%s</tooltip>\r\n", DBVar);
				if (db_get_static(hContact, "CList", "Group", DBVar))
					fprintf(file, "Group=%s\r\n", DBVar);
				if (tmp = db_get_w(hContact, MODNAME, "Icon", 40072))
					fprintf(file, "Icon=%d\r\n", tmp);
				if (tmp = db_get_b(hContact, MODNAME, "UseTimer", 0))
					fprintf(file, "UseTimer=%d\r\n", tmp);
				if (tmp = db_get_b(hContact, MODNAME, "Minutes", 1))
					fprintf(file, "Minutes=%d\r\n", tmp);
				if (tmp = db_get_w(hContact, MODNAME, "Timer", 0))
					fprintf(file, "Timer=%d\r\n", tmp);
				fprintf(file, "[/Non-IM Contact]\r\n");
			}
			else ("Contact is invalid",modFullname);
			fclose(file);
		}
	}
}

INT_PTR ImportContacts(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact;
	char name[256] = "", program[256] = "", programparam[256] = "", group[256] = "", tooltip[3000] = "", line[2001] = "";
	int icon = 40072, usetimer = 0, minutes = 1, timer = 0;
	char fn[MAX_PATH];
	int i,j, contactDone = 0;
	if ( !Openfile(fn, 1))
		return 1;

	FILE *file = fopen(fn, "r");
	if (!file)
		return 1;

	while (fgets(line,2000,file)) {
		if (!strcmp(line, "\r\n\0"))
			continue;
		if (!strcmp(line,"[Non-IM Contact]\r\n"))
			contactDone = 0;
		else if (!strncmp(line, "Name=" ,strlen("Name="))) {
			i = (int)strlen("Name=");j=0;
			while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0') {
				name[j] = line[i++];
				name[++j] = '\0';
			}
			contactDone =1;
		}
		else if (!strncmp(line, "ProgramString=" ,strlen("ProgramString="))) {
			i = (int)strlen("ProgramString=");j=0;
			while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0') {
				program[j] = line[i++];
				program[++j] = '\0';
			}
		}
		else if (!strncmp(line, "ProgramParamString=" ,strlen("ProgramParamString="))) {
			i = (int)strlen("ProgramParamString=");j=0;
			while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0') {
				programparam[j] = line[i++];
				programparam[++j] = '\0';
			}
		}
		else if (!strncmp(line, "Group=" ,strlen("Group="))) {
			i = (int)strlen("Group=");j=0;
			while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0') {
				group[j] = line[i++];
				group[++j] = '\0';
			}
		}
		else if (!strncmp(line, "ToolTip=" ,strlen("ToolTip="))) {
			i = (int)strlen("ToolTip=");
			strcpy(tooltip, &line[i]);
			fgets(line,2000,file);
			while (!strstr(line,"</tooltip>\r\n")) {
				strcat(tooltip,line);
				fgets(line,2000,file);
			}
			// the line that has the </tooltip>
			strncat(tooltip,line, strlen(line) - strlen("</tooltip>\r\n"));
		}
		else if (!strncmp(line, "Icon=", strlen("Icon="))) {
			i = (int)strlen("Icon=");
			sscanf(&line[i], "%d", &icon);
		}
		else if (!strncmp(line, "UseTimer=", strlen("UseTimer="))) {
			i = (int)strlen("UseTimer=");
			sscanf(&line[i], "%d", &usetimer);
		}
		else if (!strncmp(line, "Timer=" ,strlen("Timer="))) {
			i = (int)strlen("Timer=");
			sscanf(&line[i], "%d", &timer);
		}
		else if (!strncmp(line, "Minutes=", strlen("Minutes="))) {
			i = (int)strlen("Minutes=");
			sscanf(&line[i], "%d", &minutes);
		}
		else if (contactDone && !strcmp(line,"[/Non-IM Contact]\r\n")) {
			if (!name) continue;
			size_t size = strlen(name) + strlen("Do you want to import this Non-IM Contact?\r\n\r\nName: \r\n") + 1;
			char *msg = (char*)malloc(size);
			mir_snprintf(msg, size, "Do you want to import this Non-IM Contact?\r\n\r\nName: %s\r\n", name);
			if (program) {
				msg = (char*)realloc(msg, strlen(msg) + strlen(program) +strlen("Program: \r\n") +1);
				strcat(msg, "Program: ");
				strcat(msg,program);
				strcat(msg,"\r\n");
			}
			if (programparam) {
				msg = (char*)realloc(msg, strlen(msg) + strlen(programparam) +strlen("Program Parameters: \r\n") +1);
				strcat(msg, "Program Parameters: ");
				strcat(msg,programparam);
				strcat(msg,"\r\n");
			}
			if (tooltip) {
				msg = (char*)realloc(msg, strlen(msg) + strlen(tooltip) +strlen("ToolTip: \r\n") +1);
				strcat(msg, "ToolTip: ");
				strcat(msg,tooltip);
				strcat(msg,"\r\n");
			}
			if (group) {
				msg = (char*)realloc(msg, strlen(msg) + strlen(group) +strlen("Group: \r\n") +1);
				strcat(msg, "Group: ");
				strcat(msg,group);
				strcat(msg,"\r\n");
			}
			if (icon) {
				char tmp[64];
				if (icon == ID_STATUS_ONLINE)
					mir_snprintf(tmp, SIZEOF(tmp), "Icon: Online\r\n");
				else if (icon == ID_STATUS_AWAY)
					mir_snprintf(tmp, SIZEOF(tmp), "Icon: Away\r\n");
				else if (icon == ID_STATUS_NA)
					mir_snprintf(tmp, SIZEOF(tmp), "Icon: NA\r\n");
				else if (icon == ID_STATUS_DND)
					mir_snprintf(tmp, SIZEOF(tmp), "Icon: DND\r\n");
				else if (icon == ID_STATUS_OCCUPIED)
					mir_snprintf(tmp, SIZEOF(tmp), "Icon: Occupied\r\n");
				else if (icon == ID_STATUS_FREECHAT)
					mir_snprintf(tmp, SIZEOF(tmp), "Icon: Free For Chat\r\n");
				else if (icon == ID_STATUS_INVISIBLE)
					mir_snprintf(tmp, SIZEOF(tmp), "Icon: Invisible\r\n");
				else if (icon == ID_STATUS_ONTHEPHONE)
					mir_snprintf(tmp, SIZEOF(tmp), "Icon: On The Phone\r\n");
				else if (icon == ID_STATUS_OUTTOLUNCH)
					mir_snprintf(tmp, SIZEOF(tmp), "Icon: Out To Lunch\r\n");
				else {
					free(msg);
					continue;
				}
				char *msgtemp = (char*)realloc(msg, strlen(msg) + strlen(tmp) +1);
				if (msgtemp) {
					msg = msgtemp;
					strcat(msg,tmp);
				}
			}
			if (usetimer && timer) {
				char tmp[64],tmp2[8];
				if (minutes)
					strcpy(tmp2,"Minutes");
				else strcpy(tmp2,"Seconds");
				mir_snprintf(tmp, SIZEOF(tmp), "UseTimer: Yes\r\nTimer: %d %s",timer, tmp2);
				char *msgtemp = (char*)realloc(msg, strlen(msg) + strlen(tmp) +1);
				if (msgtemp) {
					msg = msgtemp;
					strcat(msg,tmp);
				}
			}

			if (MessageBoxA(0,msg,modFullname,MB_YESNO) == IDYES) {
				if (!(hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0))) {
					msg("contact did get created","");
					continue;
				}
				CallService(MS_PROTO_ADDTOCONTACT,hContact,(LPARAM)MODNAME);
				CallService(MS_IGNORE_IGNORE, hContact, IGNOREEVENT_USERONLINE);
				db_set_s(hContact, MODNAME, "Nick", Translate("New Non-IM Contact"));
				db_set_s(hContact, MODNAME, "Name", name);
				db_set_s(hContact, MODNAME, "ProgramString", program);
				// copy the ProgramParamString
				db_set_s(hContact, MODNAME, "ProgramParamString", programparam);
				// copy the group
				db_set_s(hContact, "CList", "Group", group);
				// copy the ToolTip
				db_set_s(hContact, MODNAME, "ToolTip", tooltip);
				// timer
				db_set_b(hContact, MODNAME, "UseTimer", (BYTE)usetimer);
				db_set_b(hContact, MODNAME, "Minutes", (BYTE)minutes);
				db_set_w(hContact, MODNAME, "Timer", (WORD)timer);
				//icon
				db_set_w(hContact, MODNAME, "Icon", (WORD)icon);
				replaceAllStrings(hContact);
			}
			free(msg);
			contactDone = 0;
			name[0] = '\0';
			program[0] = '\0';
			programparam[0] = '\0';
			group[0] = '\0';
			tooltip[0] = '\0';
			line[0] = '\0';
			icon = 40072;
			usetimer = 0;
			minutes = 1;
			timer = 0;
		}
	}
	fclose(file);

	return 1;
}
