#include "headers.h"

int addSettingToWatchList(MCONTACT hContact, char* module, char* setting)
{
	if (WatchListArray.count == WatchListArray.size)
	{
		WatchListArray.size += 32;
		WatchListArray.item = (struct DBsetting*)mir_realloc(WatchListArray.item, sizeof(struct DBsetting)*WatchListArray.size);
	}
	if (!WatchListArray.item) return 0;
	if (setting && db_get(hContact,module, setting, &(WatchListArray.item[WatchListArray.count].dbv))) return 0;
	WatchListArray.item[WatchListArray.count].hContact = hContact;
	WatchListArray.item[WatchListArray.count].module = mir_tstrdup(module);
	if (setting) WatchListArray.item[WatchListArray.count].setting = mir_tstrdup(setting);
	else WatchListArray.item[WatchListArray.count].setting = 0;

	WatchListArray.item[WatchListArray.count].WatchModule = setting?WATCH_SETTING:WATCH_MODULE;
	WatchListArray.count++;
	return 1;
}

void freeWatchListItem(int item)
{
	if (WatchListArray.item[item].module) mir_free(WatchListArray.item[item].module);
	WatchListArray.item[item].module = 0;
	if (WatchListArray.item[item].setting) mir_free(WatchListArray.item[item].setting);
	WatchListArray.item[item].setting = 0;
	db_free(&(WatchListArray.item[item].dbv));
	WatchListArray.item[item].hContact = 0;
}


void addwatchtolist(HWND hwnd2list, struct DBsetting *lParam)
{
	LVITEM lvItem;
	int index;
	char data[32], tmp[32];
	DBVARIANT *dbv = &(lParam->dbv);
	MCONTACT hContact = lParam->hContact;
	char *module = lParam->module;
	char *setting = lParam->setting;
	if (!module) return;
	lvItem.lParam = (LPARAM)lParam->hContact;
	lvItem.mask = LVIF_TEXT|LVIF_PARAM;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;

	if (!setting || (int)(lParam->setting) == WATCH_MODULE) // add every item in the module and return
	{
		ModuleSettingLL settinglist;
		struct DBsetting dummy;
		ModSetLinkLinkItem *setting;
		if (!EnumSettings(hContact,module,&settinglist)) return;
		dummy.hContact = hContact;
		dummy.module = mir_tstrdup(module);
		setting = settinglist.first;
		while (setting)
		{
			dummy.setting = setting->name;
			addwatchtolist(hwnd2list, &dummy);
			setting = (ModSetLinkLinkItem *)setting->next;
		}
		mir_free(dummy.module);
		FreeModuleSettingLL(&settinglist);
		return;
	}
	db_free(&(lParam->dbv));
	if (GetSetting(hContact, module, setting, &(lParam->dbv))) return;

	if (!hContact)
		lvItem.pszText = "NULL";
	else
		lvItem.pszText = (char*)GetContactName(hContact, NULL, 1);

	index = ListView_InsertItem(hwnd2list,&lvItem);

	WCHAR* ptszText = mir_a2u(lvItem.pszText);
	ListView_SetItemTextW(hwnd2list, index, 0, ptszText);
	mir_free(ptszText);

	ListView_SetItemText(hwnd2list,index,1,module);
	ListView_SetItemText(hwnd2list,index,2,setting);

	switch (dbv->type) {
	case DBVT_BLOB:
		{
			int j;
			char *data = NULL;
			if (!(data = (char*)mir_realloc(data, 3*(dbv->cpbVal+1))    ))
				return;
			data[0] = '\0';
			for (j=0; j<dbv->cpbVal; j++)
			{
				char tmp[16];
				mir_snprintf(tmp, SIZEOF(tmp), "%02X ", (BYTE)dbv->pbVal[j]);
				strcat(data, tmp);
			}
			ListView_SetItemText(hwnd2list,index,4,data);
			ListView_SetItemText(hwnd2list,index,3,"BLOB");
			mir_free(data);
		}
		break;

	case DBVT_BYTE:
		mir_snprintf(data, SIZEOF(data), "0x%02X (%s)", dbv->bVal, itoa(dbv->bVal,tmp,10));
		ListView_SetItemText(hwnd2list,index,4,data);
		ListView_SetItemText(hwnd2list,index,3,"BYTE");
		break;

	case DBVT_WORD:
		mir_snprintf(data, SIZEOF(data), "0x%04X (%s)", dbv->wVal, itoa(dbv->wVal,tmp,10));
		ListView_SetItemText(hwnd2list,index,4,data);
		ListView_SetItemText(hwnd2list,index,3,"WORD");
		break;

	case DBVT_DWORD:
		mir_snprintf(data, SIZEOF(data), "0x%08X (%s)", dbv->dVal, itoa(dbv->dVal,tmp,10));
		ListView_SetItemText(hwnd2list,index,4,data);
		ListView_SetItemText(hwnd2list,index,3,"DWORD");
		break;

	case DBVT_ASCIIZ:
		ListView_SetItemText(hwnd2list,index,4,dbv->pszVal);
		ListView_SetItemText(hwnd2list,index,3,"STRING");
		break;

	case DBVT_UTF8:
		int length = (int)strlen(dbv->pszVal) + 1;
		WCHAR *wc = (WCHAR*)_alloca(length*sizeof(WCHAR));
		MultiByteToWideChar(CP_UTF8, 0, dbv->pszVal, -1, wc, length);
		ListView_SetItemTextW(hwnd2list,index,4,wc);
		ListView_SetItemText(hwnd2list,index,3,"UNICODE");
		break;
	}
}

void PopulateWatchedWindow(HWND hwnd)
{
	int i;
	ListView_DeleteAllItems(hwnd);
	for (i=0;i<WatchListArray.count;i++)
	{
		addwatchtolist(hwnd, &(WatchListArray.item[i]));
	}
}

void freeAllWatches()
{
	int i;
	for (i=0;i<WatchListArray.count;i++)
	{
		freeWatchListItem(i);
	}
	mir_free(WatchListArray.item);
	WatchListArray.item = 0;
	WatchListArray.count = 0;
}

int WatchDialogResize(HWND hwnd,LPARAM lParam,UTILRESIZECONTROL *urc)
{
   switch(urc->wId)
   {
       case IDC_VARS:
           urc->rcItem.top = 0;
           urc->rcItem.bottom = urc->dlgNewSize.cy;
           urc->rcItem.left = 0;
           urc->rcItem.right = urc->dlgNewSize.cx;
           return RD_ANCHORY_CUSTOM|RD_ANCHORX_CUSTOM;
   }
   return RD_ANCHORX_LEFT|RD_ANCHORY_TOP;
}

INT_PTR CALLBACK WatchDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			// setup the list...
			LVCOLUMN sLC;

			sLC.fmt = LVCFMT_LEFT;
			ListView_SetExtendedListViewStyle(GetDlgItem(hwnd, IDC_VARS), 32|LVS_EX_SUBITEMIMAGES); //LVS_EX_FULLROWSELECT
			sLC.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;

			sLC.pszText = Translate("Contact"); sLC.cx = 80;
			ListView_InsertColumn(GetDlgItem(hwnd, IDC_VARS),0,&sLC);
			sLC.pszText = Translate("Module"); sLC.cx = 80;
			ListView_InsertColumn(GetDlgItem(hwnd, IDC_VARS),1,&sLC);
			sLC.pszText = Translate("Setting"); sLC.cx = 80;
			ListView_InsertColumn(GetDlgItem(hwnd, IDC_VARS),2,&sLC);
			sLC.pszText = Translate("Type"); sLC.cx = 60;
			ListView_InsertColumn(GetDlgItem(hwnd, IDC_VARS),3,&sLC);
			sLC.pszText = Translate("Data"); sLC.cx = 300;
			ListView_InsertColumn(GetDlgItem(hwnd, IDC_VARS),4,&sLC);

			PopulateWatchedWindow(GetDlgItem(hwnd, IDC_VARS));

			TranslateMenu(GetMenu(hwnd));
			TranslateMenu(GetSubMenu(GetMenu(hwnd),0));
			TranslateDialogDefault(hwnd);
			// do the icon
			SendMessage(hwnd,WM_SETICON,ICON_BIG,(LPARAM)LoadIcon(hInst,MAKEINTRESOURCE(ICO_REGEDIT)));
		}
		return TRUE;
		// for the resize
       case WM_GETMINMAXINFO:
       {
               MINMAXINFO *mmi=(MINMAXINFO*)lParam;
               mmi->ptMinTrackSize.x=200;
               mmi->ptMinTrackSize.y=90;
               return 0;
       }
       case WM_SIZE:
       {
               UTILRESIZEDIALOG urd;
               memset(&urd, 0, sizeof(urd));
               urd.cbSize=sizeof(urd);
               urd.hInstance=hInst;
               urd.hwndDlg=hwnd;
               urd.lParam=0;
               urd.lpTemplate=MAKEINTRESOURCE(IDD_WATCH_DIAG);
               urd.pfnResizer=WatchDialogResize;
               CallService(MS_UTILS_RESIZEDIALOG,0,(LPARAM)&urd);
               break;

       }
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case MENU_REMALL_WATCHES:
					freeAllWatches();
					ListView_DeleteAllItems(GetDlgItem(hwnd, IDC_VARS));
					break;
				case MENU_EXIT:
				case IDCANCEL:
					hwnd2watchedVarsWindow = 0;
					DestroyWindow(hwnd);
					break;
				case MENU_REFRESH:
					PopulateWatchedWindow(GetDlgItem(hwnd, IDC_VARS));
			}
			break;

		case WM_NOTIFY:
			switch(LOWORD(wParam))
			{
				case IDC_VARS:
					switch(((NMHDR*)lParam)->code)
					{
						case NM_DBLCLK:
						{
							LVHITTESTINFO hti;
							LVITEM lvi;
							hti.pt=((NMLISTVIEW*)lParam)->ptAction;
							if (ListView_SubItemHitTest(GetDlgItem(hwnd,IDC_VARS),&hti) >-1)
							{
								if (hti.flags&LVHT_ONITEM)
								{
									lvi.mask = LVIF_PARAM;
									lvi.iItem = hti.iItem;
									lvi.iSubItem = 0;
									if (ListView_GetItem(GetDlgItem(hwnd,IDC_VARS),&lvi))
									{
										ItemInfo ii;
										ii.hContact = (MCONTACT)lvi.lParam;
										ListView_GetItemText(GetDlgItem(hwnd, IDC_VARS), hti.iItem, 1, ii.module, SIZEOF(ii.module));
										ListView_GetItemText(GetDlgItem(hwnd, IDC_VARS), hti.iItem, 2, ii.setting, SIZEOF(ii.setting));
										ii.type = FW_SETTINGNAME;
										SendMessage(hwnd2mainWindow,WM_FINDITEM,(WPARAM)&ii,0);
									}
								}
							}
						}
						break;
					}
				break;
			}
		break;
	}
	return 0;
}


void popupWatchedVar(MCONTACT hContact,const char* module,const char* setting)
{
	HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(ICO_REGEDIT));
	char lpzContactName[MAX_CONTACTNAME];
	char lpzText[MAX_SECONDLINE];
	COLORREF colorBack = db_get_dw(NULL,modname,"PopupColour",RGB(255,0,0));
	COLORREF colorText = RGB(0,0,0);
	int timeout = db_get_b(NULL,modname,"PopupDelay",3);

	if (hContact) {
		// contacts nick
		char szProto[256];
		if (GetValue(hContact,"Protocol","p",szProto,SIZEOF(szProto)))
			mir_snprintf(lpzContactName, SIZEOF(lpzContactName), "%s (%s)", (char*)GetContactName(hContact, szProto, 0), szProto);
		else
			mir_snprintf(lpzContactName, SIZEOF(lpzContactName), nick_unknown);
	}
	else strcpy(lpzContactName,Translate("Settings"));

	// 2nd line
	DBVARIANT dbv;
	if ( GetSetting(hContact, module, setting, &dbv))
		return;

	switch (dbv.type) {
	case DBVT_BYTE:
		mir_snprintf(lpzText, SIZEOF(lpzText), Translate("Database Setting Changed: \nModule: \"%s\" , Setting: \"%s\"\nNew Value: (BYTE) %d"), module, setting, dbv.bVal);
		break;
	case DBVT_WORD:
		mir_snprintf(lpzText, SIZEOF(lpzText), Translate("Database Setting Changed: \nModule: \"%s\" , Setting: \"%s\"\nNew Value: (WORD) %d"), module, setting, dbv.wVal);
		break;
	case DBVT_DWORD:
		mir_snprintf(lpzText, SIZEOF(lpzText), Translate("Database Setting Changed: \nModule: \"%s\" , Setting: \"%s\"\nNew Value: (DWORD) 0x%X"), module, setting, dbv.dVal);
		break;
	case DBVT_ASCIIZ:
		mir_snprintf(lpzText, SIZEOF(lpzText), Translate("Database Setting Changed: \nModule: \"%s\" , Setting: \"%s\"\nNew Value: \"%s\""), module, setting, dbv.pszVal);
		break;
	case DBVT_UTF8:
		mir_snprintf(lpzText, SIZEOF(lpzText), Translate("Database Setting Changed: \nModule: \"%s\" , Setting: \"%s\"\nNew Value (UTF8): \"%s\""), module, setting, dbv.pszVal);
		break;
	default:
		return;
	}

	db_free(&dbv);

	POPUPDATA ppd = { 0 };
	ppd.lchContact = (MCONTACT)hContact;
	ppd.lchIcon = hIcon;
	mir_tstrncpy(ppd.lpzContactName, lpzContactName,MAX_CONTACTNAME);
	mir_tstrncpy(ppd.lpzText, lpzText,MAX_SECONDLINE);
	ppd.colorBack = colorBack;
	ppd.colorText = colorText;
	ppd.iSeconds = timeout ? timeout : -1;
	PUAddPopup(&ppd);
}
