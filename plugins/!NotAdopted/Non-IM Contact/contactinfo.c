#include "commonheaders.h"
BOOL CALLBACK DlgProcContactInfo(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
		{
			HANDLE hContact = (HANDLE)((PROPSHEETPAGE*)lParam)->lParam;
			char name[2048];
			TranslateDialogDefault(hwnd);
			SetWindowLong(hwnd, GWL_USERDATA, (LPARAM)(HANDLE)hContact);
		
			if (!DBGetContactSettingString(hContact, modname, "Name", name)) break;
			SetDlgItemText(hwnd, IDC_DISPLAY_NAME, name);
			if (!DBGetContactSettingString(hContact, modname, "ToolTip", name)) break;
			SetDlgItemText(hwnd, IDC_TOOLTIP, name);
		}
		return TRUE;
		case WM_COMMAND:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		return TRUE;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{	
							HANDLE hContact = (HANDLE)GetWindowLong(hwnd, GWL_USERDATA);
							if(GetWindowTextLength(GetDlgItem(hwnd,IDC_DISPLAY_NAME))) 
							{
								char text[512];
								GetDlgItemText(hwnd,IDC_DISPLAY_NAME,text,sizeof(text));
								DBWriteContactSettingString(hContact, modname, "Name", text);
								WriteSetting(hContact, modname, "Name", modname, "Nick");
							}
							else
							{
								DBDeleteContactSetting(hContact, modname, "Name");
								DBDeleteContactSetting(hContact, modname, "Nick");
							}
							if(GetWindowTextLength(GetDlgItem(hwnd,IDC_TOOLTIP))) 
							{
								char text[2048];
								GetDlgItemText(hwnd,IDC_TOOLTIP,text,sizeof(text));
								DBWriteContactSettingString(hContact, modname, "ToolTip", text);
								WriteSetting(hContact, modname, "ToolTip", "UserInfo", "MyNotes");
							}
							else 
							{
								DBDeleteContactSetting(hContact, modname, "ToolTip");
								DBDeleteContactSetting(hContact, "UserInfo", "MyNotes");
							}
						}
						return TRUE;
					}
					break;
			}
			break;
	}
	return FALSE;
}

static WNDPROC g_PrevBtnWndProc = 0;

LRESULT CALLBACK ButtWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT res = CallWindowProc(g_PrevBtnWndProc, hWnd, message, wParam, lParam);
    if(WM_PAINT == message)
    {
        RECT rc;
        HDC dc = GetDC(hWnd);
        BOOL isPressed = BST_CHECKED == SendMessage(hWnd, BM_GETCHECK, 0, 0);

        GetClientRect(hWnd, &rc);
        rc.left += (rc.right - rc.left - 16) / 2;
        rc.top += (rc.bottom - rc.top - 16) / 2;
        if(isPressed)
            OffsetRect(&rc, 1, 1);
        DrawIconEx(dc, rc.left, rc.top, (HICON)GetWindowLong(hWnd, GWL_USERDATA),
            16, 16, 0, 0, DI_NORMAL);
        ReleaseDC(hWnd, dc);
    }

    return res;
}

void checkGroups(char* group)
{
	int i;
    char str[50], name[256];
    DBVARIANT dbv;

	if (lstrlen(group) < 1)
		return;

    for (i = 0;; i++) 
	{
        itoa(i, str, 10);
        if (DBGetContactSetting(NULL, "CListGroups", str, &dbv))
            break;
        if (dbv.type == DBVT_ASCIIZ)
		{
			if (dbv.pszVal[0] != '\0' && !lstrcmpi(dbv.pszVal + 1, group)) 
			{
				DBFreeVariant(&dbv);
				return;
			}
		
	        DBFreeVariant(&dbv);
        }
    }
    name[0] = 1 | GROUPF_EXPANDED;
    strncpy(name + 1, group, sizeof(name) - 1);
    name[strlen(group) + 1] = '\0';
    DBWriteContactSettingString(NULL, "CListGroups", str, name);
    CallService(MS_CLUI_GROUPADDED, i + 1, 0);
}

int BrowseForFolder(HWND hwnd,char *szPath)
{
	BROWSEINFO bi={0};
	LPMALLOC pMalloc;
	ITEMIDLIST *pidlResult;
	int result=0;

	if(SUCCEEDED(OleInitialize(NULL))) {
		if(SUCCEEDED(CoGetMalloc(1,&pMalloc))) {
			bi.hwndOwner=hwnd;
			bi.pszDisplayName=szPath;
			bi.lpszTitle=Translate("Select Folder");
			bi.ulFlags=BIF_EDITBOX|BIF_RETURNONLYFSDIRS;				// Use this combo instead of BIF_USENEWUI
		//	bi.lpfn=BrowseCallbackProc;
			bi.lParam=(LPARAM)szPath;

			pidlResult=SHBrowseForFolder(&bi);
			if(pidlResult) {
				SHGetPathFromIDList(pidlResult,szPath);
				lstrcat(szPath,"\\");
				result=1;
			}
			pMalloc->lpVtbl->Free(pMalloc,pidlResult);
			pMalloc->lpVtbl->Release(pMalloc);
		}
		OleUninitialize();
	}
	return result;
}

BOOL CALLBACK DlgProcOtherStuff(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
		{
			int i = 0;
			DBVARIANT dbv;
			char string[512];
			HANDLE hContact = (HANDLE)((PROPSHEETPAGE*)lParam)->lParam;
			TranslateDialogDefault(hwnd);
			SetWindowLong(hwnd, GWL_USERDATA, (LPARAM)(HANDLE)hContact);
			if (!hContact) break;
			else
			{
				/* link*/
				if (DBGetContactSettingString(hContact, modname, "ProgramString", string))
					SetDlgItemText(hwnd, IDC_LINK, string);
				if (DBGetContactSettingString(hContact, modname, "ProgramParamsString", string))
					SetDlgItemText(hwnd, IDC_PARAMS, string);
				/* group*/
				while (i != -1)
				{
					char str[3], name[256];
					wsprintf(str, "%d", i);
					if (!DBGetContactSetting(NULL, "CListGroups", str, &dbv))
					{
						lstrcpyn(name,dbv.pszVal+1,sizeof(name));
						SendMessage(GetDlgItem(hwnd, IDC_GROUP), CB_INSERTSTRING,0, (LPARAM)name);
						i++;
					}
					else i = -1;
				}
				if (!DBGetContactSetting(hContact, "CList", "Group", &dbv))
					SetDlgItemText(hwnd, IDC_GROUP, dbv.pszVal);

			}
			/* icons */
			CheckRadioButton(hwnd, 40072, 40080, DBGetContactSettingWord(hContact, modname, "Icon", ID_STATUS_ONLINE));
			SetWindowLong(GetDlgItem(hwnd, CHK_ONLINE), GWL_USERDATA, (LONG)LoadSkinnedProtoIcon(modname, ID_STATUS_ONLINE));
			g_PrevBtnWndProc = (WNDPROC)SetWindowLong(GetDlgItem(hwnd, CHK_ONLINE), GWL_WNDPROC, (LONG)ButtWndProc);
            for (i = ID_STATUS_ONLINE; i<=ID_STATUS_OUTTOLUNCH; i++)		
			{
				SetWindowLong(GetDlgItem(hwnd, i), GWL_USERDATA, (LONG)LoadSkinnedProtoIcon(modname, i));
				SetWindowLong(GetDlgItem(hwnd, i), GWL_WNDPROC, (LONG)ButtWndProc);
            }
			DBFreeVariant(&dbv);
			/* timer */
			CheckDlgButton(hwnd, CHK_USE_TIMER, DBGetContactSettingByte(hContact, modname ,"UseTimer", 0));
			if (DBGetContactSettingWord(hContact, modname ,"Timer", 15))
			{
				CheckDlgButton(hwnd, CHK_USE_TIMER,1);
				EnableWindow(GetDlgItem(hwnd, IDC_TIMER), 1);
				SetDlgItemText(hwnd, IDC_TIMER, itoa(DBGetContactSettingWord(hContact, modname ,"Timer", 15), string, 10));
				if (!DBGetContactSettingWord(NULL, modname ,"Timer", 1))
					SetDlgItemText(hwnd,IDC_TIMER_INTERVAL_MSG, "Non-IM Contact protocol timer is Disabled");
				else 
				{
					_snprintf(string, sizeof(string), "Timer intervals... Non-IM Contact Protocol timer is %d seconds",DBGetContactSettingWord(NULL, modname ,"Timer", 1));
					SetDlgItemText(hwnd,IDC_TIMER_INTERVAL_MSG, string);
				}
			}
			/* always visible */
			if (DBGetContactSettingByte(hContact, modname ,"AlwaysVisible", 0))
			{
				CheckDlgButton(hwnd, IDC_ALWAYS_VISIBLE, 1);
				EnableWindow(GetDlgItem(hwnd, IDC_VISIBLE_UNLESS_OFFLINE),1);
				CheckDlgButton(hwnd, IDC_VISIBLE_UNLESS_OFFLINE, DBGetContactSettingByte(hContact, modname ,"VisibleUnlessOffline", 1));
			}
			
		}
		return TRUE;
		case WM_COMMAND:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			switch(LOWORD(wParam)) {
			case IDC_ALWAYS_VISIBLE:
				if (IsDlgButtonChecked(hwnd, IDC_ALWAYS_VISIBLE))
				{
					HANDLE hContact = (HANDLE)GetWindowLong(hwnd, GWL_USERDATA);
					EnableWindow(GetDlgItem(hwnd, IDC_VISIBLE_UNLESS_OFFLINE),1);
					CheckDlgButton(hwnd, IDC_VISIBLE_UNLESS_OFFLINE, DBGetContactSettingByte(hContact, modname ,"VisibleUnlessOffline", 1));
				}
				else EnableWindow(GetDlgItem(hwnd, IDC_VISIBLE_UNLESS_OFFLINE),0);
			break;
			case CHK_USE_TIMER:
				if (IsDlgButtonChecked(hwnd, CHK_USE_TIMER))
				{
					HANDLE hContact = (HANDLE)GetWindowLong(hwnd, GWL_USERDATA);
					char string[4];
					EnableWindow(GetDlgItem(hwnd, IDC_TIMER), 1);
					SetDlgItemText(hwnd, IDC_TIMER, itoa(DBGetContactSettingWord(hContact, modname ,"Timer", 15), string, 10));
				}
				else
				{
					EnableWindow(GetDlgItem(hwnd, IDC_TIMER), 0);
				}
			break;
			case IDC_OPEN_FILE:
			{
				char szFileName[512];
				if (Openfile(szFileName,1))
					SetDlgItemText(hwnd, IDC_LINK, szFileName);
			}
			break;
			case IDC_OPEN_FOLDER:
			{
				char szFileName[512];
				if (BrowseForFolder(hwnd, szFileName))
				{
					wsprintf(szFileName, "%s ,/e", szFileName);
					SetDlgItemText(hwnd, IDC_LINK, "explorer.exe");
					SetDlgItemText(hwnd, IDC_PARAMS, szFileName);
				}
			}
			break;	
			
			return TRUE;
			}
		break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{	
							int status = GetLCStatus(0,0);
							HANDLE hContact = (HANDLE)GetWindowLong(hwnd, GWL_USERDATA);
							int i;
							if(GetWindowTextLength(GetDlgItem(hwnd,IDC_LINK))) 
							{
								char text[512];
								GetDlgItemText(hwnd,IDC_LINK,text,sizeof(text));
								DBWriteContactSettingString(hContact, modname, "ProgramString", text);
								WriteSetting(hContact, modname, "ProgramString", modname, "Program");
							}
							else DBDeleteContactSetting(hContact, modname, "ProgramString");
							if(GetWindowTextLength(GetDlgItem(hwnd,IDC_PARAMS))) 
							{
								char text[512];
								GetDlgItemText(hwnd,IDC_PARAMS,text,sizeof(text));
								DBWriteContactSettingString(hContact, modname, "ProgramParamsString", text);
								WriteSetting(hContact, modname, "ProgramParamsString", modname, "ProgramParams");
							}
							else DBDeleteContactSetting(hContact, modname, "ProgramParamsString");
							if(GetWindowTextLength(GetDlgItem(hwnd,IDC_GROUP))) 
							{
								char text[512];
								GetDlgItemText(hwnd,IDC_GROUP,text,sizeof(text));
								checkGroups(text);
								DBWriteContactSettingString(hContact, "CList", "Group", text);
							}
							else DBDeleteContactSetting(hContact, "CList", "Group");
							for (i = ID_STATUS_ONLINE; i<=ID_STATUS_OUTTOLUNCH; i++)
							{
								if (IsDlgButtonChecked(hwnd, i))
									DBWriteContactSettingWord(hContact, modname, "Icon", (WORD)i);
							}
							/* set correct status */
							if ( (status == ID_STATUS_ONLINE) || (status == ID_STATUS_AWAY) || 
								 (status == DBGetContactSettingWord(hContact, modname, "Icon", ID_STATUS_ONLINE) )
							   )
								DBWriteContactSettingWord(hContact, modname, "Status", (WORD)DBGetContactSettingWord(hContact, modname, "Icon", ID_STATUS_ONLINE));
							else
								DBWriteContactSettingWord(hContact, modname, "Status", ID_STATUS_OFFLINE);

							if (IsDlgButtonChecked(hwnd, CHK_USE_TIMER))
							{
								if(GetWindowTextLength(GetDlgItem(hwnd,IDC_TIMER))) 
								{
									char text[512];
									GetDlgItemText(hwnd,IDC_TIMER,text,sizeof(text));
									DBWriteContactSettingWord(hContact, modname, "Timer", (WORD)atoi(text));
								}
								else DBWriteContactSettingWord(hContact, modname, "Timer", 15);
							}
							else DBWriteContactSettingWord(hContact, modname, "Timer", 0);
							// always visible
							DBWriteContactSettingByte(hContact, modname, "AlwaysVisible", (BYTE)IsDlgButtonChecked(hwnd, IDC_ALWAYS_VISIBLE));
							DBWriteContactSettingByte(hContact, modname, "VisibleUnlessOffline", (BYTE)IsDlgButtonChecked(hwnd, IDC_VISIBLE_UNLESS_OFFLINE));
						}
						return TRUE;
					}
					break;
			}
			break;
	}
	return FALSE;
}


char* copyReplaceString(char* oldStr, char* newStr, char* findStr, char* replaceWithStr)
{
	int i=0;
	while (oldStr[i] != '\0')
	{
	//	msg(&oldStr[i],"");
		if (!strncmp(&oldStr[i],findStr, strlen(findStr)))
		{
			strcat(newStr,replaceWithStr);
			i += strlen(findStr);
		}
		else 
		{
			strncat(newStr,&oldStr[i],1);
			i++;
		}
	}
	return newStr;
}
#define MAX_REPLACES 15
BOOL CALLBACK DlgProcCopy(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
		{
			HANDLE hContact = (HANDLE)((PROPSHEETPAGE*)lParam)->lParam;
			TranslateDialogDefault(hwnd);
			SetWindowLong(hwnd, GWL_USERDATA, (LPARAM)(HANDLE)hContact);
		}
		return TRUE;
		case WM_COMMAND:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			switch(LOWORD(wParam)) {
			case IDC_EXPORT:
				ExportContact((HANDLE)GetWindowLong(hwnd, GWL_USERDATA));
			break;
			case IDC_DOIT:
				if (GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING_REPLACE)))
				{
					char *replace = (char*)malloc(GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING_REPLACE)) +1);
					char newString[MAX_REPLACES][512], oldString[MAX_REPLACES][512];
					char dbVar1[2000], dbVar2[2000];
					int i=0,j=0, k=0;
					char *string = oldString[k];
					HANDLE hContact1 = (HANDLE)GetWindowLong(hwnd, GWL_USERDATA), hContact2;
					GetDlgItemText(hwnd, IDC_STRING_REPLACE, replace, GetWindowTextLength(GetDlgItem(hwnd, IDC_STRING_REPLACE)) +1);
					if (DBGetContactSettingString(hContact1, modname, "Name", dbVar1))
					{
						// get the list of replace strings
						while (replace[i] != '\0')
						{
							if (replace[i] == ',')
							{
								string = newString[k];
								j=0;
							}
							else if (!strncmp(&replace[i], "\r\n",2))
							{
								if (string == newString[k])
									k--;
								if (k = MAX_REPLACES) break;
								string = oldString[++k];
								i+=2;
								continue;
							}
							else
							{
								string[j] = replace[i];
								string[++j] = '\0';
							}
							i++;
						}	
						hContact2 =(HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0);
						CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hContact2,(LPARAM)modname);
						CallService(MS_IGNORE_IGNORE, (WPARAM)hContact2, IGNOREEVENT_USERONLINE);
						DBWriteContactSettingString(hContact2, modname, "Nick", Translate("New Non-IM Contact"));
						// blank dbVar2 so the replaceing doesnt crash..
						strcpy(dbVar2, "");
						// copy the name (dbVar1 is the name)
						for (i=0;i<=k;i++)
						{
							copyReplaceString(dbVar1, dbVar2, oldString[i],newString[i] );
						}
						DBWriteContactSettingString(hContact2, modname, "Name", dbVar2);
						// copy the ProgramString
						if (DBGetContactSettingString(hContact1, modname, "ProgramString",dbVar1))
						{
							strcpy(dbVar2, "");
							for (i=0;i<=k;i++)
							{
								copyReplaceString(dbVar1, dbVar2, oldString[i],newString[i] );
							}
							DBWriteContactSettingString(hContact2, modname, "ProgramString", dbVar2);
						}
						// copy the ProgramParamString
						if (DBGetContactSettingString(hContact1, modname, "ProgramParamString",dbVar1))
						{
							strcpy(dbVar2, "");
							for (i=0;i<=k;i++)
							{
								copyReplaceString(dbVar1, dbVar2, oldString[i],newString[i] );
							}
							DBWriteContactSettingString(hContact2, modname, "ProgramParamString", dbVar2);
						}
						// copy the group
						if (DBGetContactSettingString(hContact1, "CList", "Group",dbVar1))
						{
							strcpy(dbVar2, "");
							for (i=0;i<=k;i++)
							{
								copyReplaceString(dbVar1, dbVar2, oldString[i],newString[i] );
							}
							DBWriteContactSettingString(hContact2, "CList", "Group", dbVar2);
						}
						// copy the ToolTip
						if (DBGetContactSettingString(hContact1, modname, "ToolTip",dbVar1))
						{
							strcpy(dbVar2, "");
							for (i=0;i<=k;i++)
							{
								copyReplaceString(dbVar1, dbVar2, oldString[i],newString[i] );
							}
							DBWriteContactSettingString(hContact2, modname, "ToolTip", dbVar2);
						}
						// timer
						DBWriteContactSettingByte(hContact2, modname, "UseTimer", (BYTE)DBGetContactSettingByte(hContact1, modname, "UseTimer", 0));
						DBWriteContactSettingByte(hContact2, modname, "Minutes", (BYTE)DBGetContactSettingByte(hContact1, modname, "Minutes", 0));
						DBWriteContactSettingWord(hContact2, modname, "Timer", (WORD)DBGetContactSettingWord(hContact1, modname, "Timer", 0));
						//icon
						DBWriteContactSettingWord(hContact2, modname, "Icon", (WORD)DBGetContactSettingWord(hContact1, modname, "Icon", 40072));
						replaceAllStrings(hContact2);
					}	
				}
				else 
				{
					char dbVar1[2000];
					HANDLE hContact1 = (HANDLE)GetWindowLong(hwnd, GWL_USERDATA), hContact2;
					if (DBGetContactSettingString(hContact1, modname, "Name", dbVar1))
					{
						if (!(hContact2 =(HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0))) 
						{
							msg("contact did not get created","");
							return 0;
						}
						CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hContact2,(LPARAM)modname);
						CallService(MS_IGNORE_IGNORE, (WPARAM)hContact2, IGNOREEVENT_USERONLINE);
						DBWriteContactSettingString(hContact2, modname, "Nick", Translate("New Non-IM Contact"));
						DBWriteContactSettingString(hContact2, modname, "Name", dbVar1);
						if (DBGetContactSettingString(hContact1, modname, "ProgramString",dbVar1))
						{
							DBWriteContactSettingString(hContact2, modname, "ProgramString", dbVar1);
						}
						// copy the ProgramParamString
						if (DBGetContactSettingString(hContact1, modname, "ProgramParamString",dbVar1))
						{
							DBWriteContactSettingString(hContact2, modname, "ProgramParamString", dbVar1);
						}
						// copy the group
						if (DBGetContactSettingString(hContact1, "CList", "Group",dbVar1))
						{
							DBWriteContactSettingString(hContact2, "CList", "Group", dbVar1);
						}
						// copy the ToolTip
						if (DBGetContactSettingString(hContact1, modname, "ToolTip",dbVar1))
						{
							DBWriteContactSettingString(hContact2, modname, "ToolTip", dbVar1);
						}
						// timer
						DBWriteContactSettingByte(hContact2, modname, "UseTimer", (BYTE)DBGetContactSettingByte(hContact1, modname, "UseTimer", 0));
						DBWriteContactSettingByte(hContact2, modname, "Minutes", (BYTE)DBGetContactSettingByte(hContact1, modname, "Minutes", 0));
						DBWriteContactSettingWord(hContact2, modname, "Timer", (WORD)DBGetContactSettingWord(hContact1, modname, "Timer", 0));
						//icon
						DBWriteContactSettingWord(hContact2, modname, "Icon", (WORD)DBGetContactSettingWord(hContact1, modname, "Icon", 40072));
						replaceAllStrings(hContact2);
					}
				}
				

			break;

			return TRUE;
			}
		break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{	
						}
						return TRUE;
					}
					break;
			}
			break;
	}
	return FALSE;
}

void ExportContact(HANDLE hContact)
{
	FILE* file;
	char szFileName[MAX_PATH];
	char DBVar[1024];
	int tmp;

	if (Openfile(szFileName, 0))
	{
	//	if (tmp = MessageBox(0, "Do you want to overwrite the contents of the file?\r\n\r\nPressing No will append this contact to the end of the file.",modFullname, MB_YESNO) == IDYES)
	//		file = fopen(szFileName, "w");
	//	else 
		file = fopen(szFileName, "a");
		if (file)
		{
			if (DBGetContactSettingString(hContact, modname, "Name", DBVar))
			{
				fprintf(file, "\r\n[Non-IM Contact]\r\nName=%s\r\n", DBVar);
				if (DBGetContactSettingString(hContact, modname, "ProgramString", DBVar))
					fprintf(file, "ProgramString=%s\r\n", DBVar);
				if (DBGetContactSettingString(hContact, modname, "ProgramParamString", DBVar))
					fprintf(file, "ProgramParamString=%s\r\n", DBVar);
				if (DBGetContactSettingString(hContact, modname, "ToolTip", DBVar))
					fprintf(file, "ToolTip=%s</tooltip>\r\n", DBVar);
				if (DBGetContactSettingString(hContact, "CList", "Group", DBVar))
					fprintf(file, "Group=%s\r\n", DBVar);
				if (tmp = DBGetContactSettingWord(hContact, modname, "Icon", 40072))
					fprintf(file, "Icon=%d\r\n", tmp);
				if (tmp = DBGetContactSettingByte(hContact, modname, "UseTimer", 0))
					fprintf(file, "UseTimer=%d\r\n", tmp);
				if (tmp = DBGetContactSettingByte(hContact, modname, "Minutes", 1))
					fprintf(file, "Minutes=%d\r\n", tmp);
				if (tmp = DBGetContactSettingWord(hContact, modname, "Timer", 0))
					fprintf(file, "Timer=%d\r\n", tmp);
				fprintf(file, "[/Non-IM Contact]\r\n");
			}
			else ("Contact is invalid",modFullname);
			fclose(file);
		}
	}
}

int ImportContacts(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact;
	char name[256] = "", program[256] = "", programparam[256] = "", group[256] = "", tooltip[3000] = "", line[2001] = "";
	int icon = 40072, usetimer = 0, minutes = 1, timer = 0;
	FILE* file;
	char fn[MAX_PATH];
	int i,j, contactDone = 0;
	if (Openfile(fn, 1))
	{
		file = fopen(fn, "r");
		if (file)
		{
			while (fgets(line,2000,file) )
			{
				if (!strcmp(line, "\r\n\0")) continue;
				else if (!strcmp(line,"[Non-IM Contact]\r\n"))
				{
					contactDone = 0;
				}
				else if (!strncmp(line, "Name=" ,strlen("Name=")))
				{
					i=strlen("Name=");j=0;
					while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0')
					{
						name[j] = line[i++];
						name[++j] = '\0';
					}
					contactDone =1;
				}
				else if (!strncmp(line, "ProgramString=" ,strlen("ProgramString=")))
				{
					i=strlen("ProgramString=");j=0;
					while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0')
					{
						program[j] = line[i++];
						program[++j] = '\0';
					}
				}
				else if (!strncmp(line, "ProgramParamString=" ,strlen("ProgramParamString=")))
				{
					i=strlen("ProgramParamString=");j=0;
					while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0')
					{
						programparam[j] = line[i++];
						programparam[++j] = '\0';
					}
				}
				else if (!strncmp(line, "Group=" ,strlen("Group=")))
				{
					i=strlen("Group=");j=0;
					while (line[i] != '\r' && line[i] != '\n' && line[i] != '\0')
					{
						group[j] = line[i++];
						group[++j] = '\0';
					}
				}
				else if (!strncmp(line, "ToolTip=" ,strlen("ToolTip=")))
				{
					i=strlen("ToolTip=");
					strcpy(tooltip, &line[i]);
					fgets(line,2000,file);
					while (!strstr(line,"</tooltip>\r\n"))
					{
						strcat(tooltip,line);
						fgets(line,2000,file);
					}
					// the line that has the </tooltip>
					strncat(tooltip,line, strlen(line) - strlen("</tooltip>\r\n"));
				}
				else if (!strncmp(line, "Icon=", strlen("Icon=")))
				{
					i=strlen("Icon=");
					sscanf(&line[i], "%d", &icon);
				}
				else if (!strncmp(line, "UseTimer=", strlen("UseTimer=")))
				{
					i=strlen("UseTimer=");
					sscanf(&line[i], "%d", &usetimer);
				}
				else if (!strncmp(line, "Timer=" ,strlen("Timer=")))
				{
					i=strlen("Timer=");
					sscanf(&line[i], "%d", &timer);
				}
				else if (!strncmp(line, "Minutes=", strlen("Minutes=")))
				{
					i=strlen("Minutes=");
					sscanf(&line[i], "%d", &minutes);
				}
				else if (contactDone && !strcmp(line,"[/Non-IM Contact]\r\n"))
				{
					char *msg;
					if (!name) continue;
					msg = (char*)malloc(strlen(name) + strlen("Do you want to import this Non-IM Contact?\r\n\r\nName: \r\n") + 1);
					wsprintf(msg, "Do you want to import this Non-IM Contact?\r\n\r\nName: %s\r\n", name);
					if (program) 
					{
						msg = (char*)realloc(msg, strlen(msg) + strlen(program) +strlen("Program: \r\n") +1);
						strcat(msg, "Program: ");
						strcat(msg,program);
						strcat(msg,"\r\n");
					}
					if (programparam) 
					{
						msg = (char*)realloc(msg, strlen(msg) + strlen(programparam) +strlen("Program Parameters: \r\n") +1);
						strcat(msg, "Program Parameters: ");
						strcat(msg,programparam);
						strcat(msg,"\r\n");
					}
					if (tooltip) 
					{
						msg = (char*)realloc(msg, strlen(msg) + strlen(tooltip) +strlen("ToolTip: \r\n") +1);
						strcat(msg, "ToolTip: ");
						strcat(msg,tooltip);
						strcat(msg,"\r\n");
					}
					if (group) 
					{
						msg = (char*)realloc(msg, strlen(msg) + strlen(group) +strlen("Group: \r\n") +1);
						strcat(msg, "Group: ");
						strcat(msg,group);
						strcat(msg,"\r\n");
					}
					if (icon)
					{
						char tmp[64];
						if (icon == ID_STATUS_ONLINE)
							wsprintf(tmp, "Icon: Online\r\n");
						else if (icon == ID_STATUS_AWAY)
							wsprintf(tmp, "Icon: Away\r\n");
						else if (icon == ID_STATUS_NA)
							wsprintf(tmp, "Icon: NA\r\n");
						else if (icon == ID_STATUS_DND)
							wsprintf(tmp, "Icon: DND\r\n");
						else if (icon == ID_STATUS_OCCUPIED)
							wsprintf(tmp, "Icon: Occupied\r\n");
						else if (icon == ID_STATUS_FREECHAT)
							wsprintf(tmp, "Icon: Free For Chat\r\n");
						else if (icon == ID_STATUS_INVISIBLE)
							wsprintf(tmp, "Icon: Invisible\r\n");
						else if (icon == ID_STATUS_ONTHEPHONE)
							wsprintf(tmp, "Icon: On The Phone\r\n");
						else if (icon == ID_STATUS_OUTTOLUNCH)
							wsprintf(tmp, "Icon: Out To Lunch\r\n");
						msg = (char*)realloc(msg, strlen(msg) + strlen(tmp) +1);
						strcat(msg,tmp);
					}
					if (usetimer && timer)
					{
						char tmp[64],tmp2[8];
						if (minutes)
							strcpy(tmp2,"Minutes");
						else strcpy(tmp2,"Seconds");
						wsprintf(tmp, "UseTimer: Yes\r\nTimer: %d %s",timer, tmp2);
						msg = (char*)realloc(msg, strlen(msg) + strlen(tmp) +1);
						strcat(msg,tmp);
					}
				
					if (MessageBox(0,msg,modFullname,MB_YESNO) == IDYES)
					{
						if (!(hContact =(HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0))) 
						{
							msg("contact did get created","");
							continue;
						}
						CallService(MS_PROTO_ADDTOCONTACT,(WPARAM)hContact,(LPARAM)modname);
						CallService(MS_IGNORE_IGNORE, (WPARAM)hContact, IGNOREEVENT_USERONLINE);
						DBWriteContactSettingString(hContact, modname, "Nick", Translate("New Non-IM Contact"));
						DBWriteContactSettingString(hContact, modname, "Name", name);
						DBWriteContactSettingString(hContact, modname, "ProgramString", program);
						// copy the ProgramParamString
						DBWriteContactSettingString(hContact, modname, "ProgramParamString", programparam);
						// copy the group
						DBWriteContactSettingString(hContact, "CList", "Group", group);
						// copy the ToolTip
						DBWriteContactSettingString(hContact, modname, "ToolTip", tooltip);
						// timer
						DBWriteContactSettingByte(hContact, modname, "UseTimer", (BYTE)usetimer);
						DBWriteContactSettingByte(hContact, modname, "Minutes", (BYTE)minutes);
						DBWriteContactSettingWord(hContact, modname, "Timer", (WORD)timer);
						//icon
						DBWriteContactSettingWord(hContact, modname, "Icon", (WORD)icon);
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
		// ask if they want to import this contact
		}
	}
	return 1;
}
