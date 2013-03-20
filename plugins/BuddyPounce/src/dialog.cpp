#include "headers.h"

void populateSettingsList(HWND hwnd2List)
{
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)Translate("Send If My Status Is..."));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)Translate("Send If They Change Status to..."));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)"----------------------------");
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)Translate("Reuse Pounce"));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)Translate("Give Up delay"));
	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)Translate("Confirmation Window"));
//	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)"----------------------------"));
//	SendMessage(hwnd2List, LB_ADDSTRING, 0, (LPARAM)Translate("Send a File"));
}

void populateContacts(HANDLE BPhContact,HWND hwnd2CB)
{
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	char name[300], *szProto;
	int index, selectedIndex = 0;
	while (hContact)
	{
		szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if (szProto && (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM))
		{
			_snprintf(name, 300, "%s (%s)", (char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,0), szProto);
			index = SendMessage(hwnd2CB, CB_ADDSTRING, 0, (LPARAM)name);
			SendMessage(hwnd2CB, CB_SETITEMDATA, index, (LPARAM)hContact);
			if (BPhContact == hContact) SendMessage(hwnd2CB, CB_SETCURSEL, index, 0);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}


void saveLastSetting(HANDLE hContact, HWND hwnd)
{
	char number[8], string[1024];
	switch (DBGetContactSettingByte(hContact, modname, "LastSetting", 2)) // nothing to do
	{
		case 0: // Send If My Status Is...
		break;
		case 1: // Send If They Change status to
		break;
		case 2: // ------, fall through
		case 6: // nothing to do...
		break;
		case 3: // Reuse Pounce
			GetDlgItemText(hwnd, IDC_SETTINGNUMBER, number, 8);
			DBWriteContactSettingByte(hContact, modname, "Reuse", (BYTE)atoi(number));
		break;
		case 4: // Give Up delay
			GetDlgItemText(hwnd, IDC_SETTINGNUMBER, number, 8);
			DBWriteContactSettingByte(hContact, modname, "GiveUpDays", (BYTE)atoi(number));
			{
				time_t today = time(NULL);
				DBWriteContactSettingDword(hContact, modname, "GiveUpDate", (DWORD)(atoi(number)*SECONDSINADAY));
			}
		break;
		case 5:	// confirm window
			GetDlgItemText(hwnd, IDC_SETTINGNUMBER, number, 8);
			DBWriteContactSettingByte(hContact, modname, "ConfirmTimeout", (BYTE)atoi(number));
		break;
		
		case 7: // send a file
			GetDlgItemText(hwnd, IDC_SETTINGTEXT, string, 1024);
			DBWriteContactSettingString(hContact, modname, "FileToSend", string);
		break;
	}
}

void hideAll(HWND hwnd)
{
	ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_BUTTON), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_HIDE);
	ShowWindow(GetDlgItem(hwnd, IDC_SETTINGTEXT), SW_HIDE);
}

void getDefaultMessage(HWND hwnd, UINT control, HANDLE hContact)
{
	DBVARIANT dbv;
	if (!DBGetContactSetting(hContact,modname, "PounceMsg", &dbv))
		SetDlgItemText(hwnd, control, dbv.pszVal);
	else if (!DBGetContactSetting(NULL,modname, "PounceMsg", &dbv))
		SetDlgItemText(hwnd, control, dbv.pszVal);
	DBFreeVariant(&dbv);
}
INT_PTR CALLBACK StatusModesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
		case WM_INITDIALOG:
		{
			struct windowInfo *wi = (struct windowInfo *)lParam;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)wi);
			TranslateDialogDefault(hwnd);
			
		}
		return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDOK:
				case IDCANCEL:
				{
					struct windowInfo *wi = (struct windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
					char type[32];
					int flag = 0;
					GetDlgItemText(hwnd, IDC_CHECK1, type, 32);
					flag = (IsDlgButtonChecked(hwnd, IDC_CHECK1))
						|(IsDlgButtonChecked(hwnd, IDC_CHECK2)<<1)
						|(IsDlgButtonChecked(hwnd, IDC_CHECK3)<<2)
						|(IsDlgButtonChecked(hwnd, IDC_CHECK4)<<3)
						|(IsDlgButtonChecked(hwnd, IDC_CHECK5)<<4)
						|(IsDlgButtonChecked(hwnd, IDC_CHECK6)<<5)
						|(IsDlgButtonChecked(hwnd, IDC_CHECK7)<<6)
						|(IsDlgButtonChecked(hwnd, IDC_CHECK8)<<7)
						|(IsDlgButtonChecked(hwnd, IDC_CHECK9)<<8)
						|(IsDlgButtonChecked(hwnd, IDC_CHECK10)<<9);
					if (!strcmp(type, "Any"))
					{
						if (LOWORD(wParam) == IDOK)
							DBWriteContactSettingWord(wi->hContact, modname, "SendIfMyStatusIsFLAG", (WORD)flag);
						wi->SendIfMy = 0;
					}
					else 
					{
						if (LOWORD(wParam) == IDOK)
							DBWriteContactSettingWord(wi->hContact, modname, "SendIfTheirStatusIsFLAG", (WORD)flag);
						wi->SendWhenThey = 0;
					}				
					DestroyWindow(hwnd);
				}
				break;
			}
		break;
	}
	return FALSE;
}
void statusModes(struct windowInfo *wi, int myStatusMode) // myStatusMode=1 sendIfMyStatusFlag
{
	int statusFlag;
	HWND hwnd;
	
	if (myStatusMode) 
	{
		if (wi->SendIfMy) { SetForegroundWindow(wi->SendIfMy); return; }
		else 
		{
			hwnd = wi->SendIfMy = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_STATUSMODES), 0, StatusModesDlgProc, (LPARAM)wi);
			statusFlag = DBGetContactSettingWord(wi->hContact, modname, "SendIfMyStatusIsFLAG", 0);
			SetWindowText(hwnd, Translate("Send If My Status Is"));
			SetDlgItemText(hwnd, IDC_CHECK1, Translate("Any"));
			SetDlgItemText(hwnd, IDC_CHECK2, Translate("Online"));
			SetDlgItemText(hwnd, IDC_CHECK3, Translate("Away"));
			SetDlgItemText(hwnd, IDC_CHECK4, Translate("NA"));
			SetDlgItemText(hwnd, IDC_CHECK5, Translate("Occupied"));
			SetDlgItemText(hwnd, IDC_CHECK6, Translate("DND"));
			SetDlgItemText(hwnd, IDC_CHECK7, Translate("Free For Chat"));
			SetDlgItemText(hwnd, IDC_CHECK8, Translate("Invisible"));
			SetDlgItemText(hwnd, IDC_CHECK9, Translate("On The Phone"));
			SetDlgItemText(hwnd, IDC_CHECK10, Translate("Out To Lunch"));
		}
	}
	else 
	{
		if (wi->SendWhenThey) { SetForegroundWindow(wi->SendWhenThey); return; }
		else 
		{
			hwnd = wi->SendWhenThey = CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_STATUSMODES),0,StatusModesDlgProc, (LPARAM)wi);
			statusFlag = DBGetContactSettingWord(wi->hContact, modname, "SendIfTheirStatusIsFLAG", 0);
			SetWindowText(hwnd, Translate("Send If Their Status changes"));
			SetDlgItemText(hwnd, IDC_CHECK1, Translate("From Offline"));
			SetDlgItemText(hwnd, IDC_CHECK2, Translate("To Online"));
			SetDlgItemText(hwnd, IDC_CHECK3, Translate("To Away"));
			SetDlgItemText(hwnd, IDC_CHECK4, Translate("To NA"));
			SetDlgItemText(hwnd, IDC_CHECK5, Translate("To Occupied"));
			SetDlgItemText(hwnd, IDC_CHECK6, Translate("To DND"));
			SetDlgItemText(hwnd, IDC_CHECK7, Translate("To Free For Chat"));
			SetDlgItemText(hwnd, IDC_CHECK8, Translate("To Invisible"));
			SetDlgItemText(hwnd, IDC_CHECK9, Translate("To On The Phone"));
			SetDlgItemText(hwnd, IDC_CHECK10, Translate("To Out To Lunch"));
		}
	}
	CheckDlgButton(hwnd, IDC_CHECK1, statusFlag&ANY);
	CheckDlgButton(hwnd, IDC_CHECK2, (statusFlag&ONLINE)>>1);
	CheckDlgButton(hwnd, IDC_CHECK3, (statusFlag&AWAY)>>2);
	CheckDlgButton(hwnd, IDC_CHECK4, (statusFlag&NA)>>3);
	CheckDlgButton(hwnd, IDC_CHECK5, (statusFlag&OCCUPIED)>>4);
	CheckDlgButton(hwnd, IDC_CHECK6, (statusFlag&DND)>>5);
	CheckDlgButton(hwnd, IDC_CHECK7, (statusFlag&FFC)>>6);
	CheckDlgButton(hwnd, IDC_CHECK8, (statusFlag&INVISIBLE)>>7);
	CheckDlgButton(hwnd, IDC_CHECK9, (statusFlag&PHONE)>>8);
	CheckDlgButton(hwnd, IDC_CHECK10, (statusFlag&LUNCH)>>9);
}

void deletePounce(HANDLE hContact)
{
	DBDeleteContactSetting(hContact,modname, "PounceMsg");
	DBDeleteContactSetting(hContact,modname, "SendIfMyStatusIsFLAG");
	DBDeleteContactSetting(hContact,modname, "SendIfTheirStatusIsFLAG");
	DBDeleteContactSetting(hContact,modname, "Reuse");
	DBDeleteContactSetting(hContact, modname, "GiveUpDays");
	DBDeleteContactSetting(hContact, modname, "GiveUpDate");
	DBDeleteContactSetting(hContact, modname, "ConfirmTimeout");
	DBDeleteContactSetting(hContact, modname, "FileToSend");
}

INT_PTR CALLBACK BuddyPounceSimpleDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
		case WM_INITDIALOG:
		{
			HANDLE hContact = (HANDLE)lParam;
			char msg[1024];
			TranslateDialogDefault(hwnd);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)lParam);
			
			getDefaultMessage(hwnd, IDC_MESSAGE, hContact);
			_snprintf(msg, 1024, Translate("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
			SetDlgItemText(hwnd, GRP_MSG, msg);	

		}
		return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDC_MESSAGE:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						int length;
						char msg[1024];
						length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE));
						_snprintf(msg, 1024, Translate("The Message    (%d Characters)"), length);
						SetDlgItemText(hwnd, GRP_MSG, msg);
					}
				}
				break;
				case IDC_ADVANCED:
				// fall through
				case IDOK:
				{
					char *text;
					HANDLE hContact = (HANDLE)GetWindowLongPtr(hwnd, GWLP_USERDATA);
					int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE))+1;
					if (length>1)
					{
						text = (char*)malloc(length);
						if (!text) { msg("Couldnt Allocate enough memory",""); break; }
						GetDlgItemText(hwnd, IDC_MESSAGE, text, length);
						
					}
					else DBDeleteContactSetting(hContact, modname, "PounceMsg");
				} // fall through
				case IDCANCEL:
					if (LOWORD(wParam) == IDC_ADVANCED)
						CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_POUNCE), 0, BuddyPounceDlgProc, GetWindowLongPtr(hwnd, GWLP_USERDATA));
					DestroyWindow(hwnd);
				break;
				return FALSE;
			}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK BuddyPounceDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
		case WM_INITDIALOG:
		{
			struct windowInfo *wi = (struct windowInfo *)malloc(sizeof(struct windowInfo));
			char msg[1024];
			if (!wi) { msg("error......","Buddy Pounce"); DestroyWindow(hwnd); }
			TranslateDialogDefault(hwnd);
			wi->hContact = (HANDLE)lParam;
			wi->SendIfMy = 0;
			wi->SendWhenThey = 0;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)wi);
			getDefaultMessage(hwnd, IDC_MESSAGE, wi->hContact);
			_snprintf(msg, 1024, Translate("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
			SetDlgItemText(hwnd, GRP_MSG, msg);	
			populateSettingsList(GetDlgItem(hwnd, IDC_SETTINGS));
			populateContacts(wi->hContact, GetDlgItem(hwnd,IDC_CONTACTS));
			SendMessage(GetDlgItem(hwnd,IDC_SPIN), UDM_SETRANGE, 0, (LPARAM) MAKELONG ((short) 1024, (short) 0));
			DBWriteContactSettingByte(wi->hContact, modname, "LastSetting", 0);
		}
		return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDC_MESSAGE:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						int length;
						char msg[1024];
						length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE));
						_snprintf(msg, 1024, Translate("The Message    (%d Characters)"), length);
						SetDlgItemText(hwnd, GRP_MSG, msg);
					}
				}
				break;
				case IDC_SIMPLE:
				case IDOK:
				{
					char *text;
					HANDLE hContact = (HANDLE)SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_GETITEMDATA, SendMessage(GetDlgItem(hwnd, IDC_CONTACTS), CB_GETCURSEL,0,0),0);
					int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE))+1;
					if (length>1)
					{
						text = (char*)malloc(length);
						if (!text) { msg("Couldnt Allocate enough memory",""); break; }
						GetDlgItemText(hwnd, IDC_MESSAGE, text, length);
						DBWriteContactSettingString(hContact, modname, "PounceMsg", text);
						free(text);
					}
					else DBDeleteContactSetting(hContact, modname, "PounceMsg");
					saveLastSetting(hContact, hwnd);
				} // fall through
					if (LOWORD(wParam) == IDC_SIMPLE)
						CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_POUNCE_SIMPLE), 0, BuddyPounceSimpleDlgProc, (LPARAM)((struct windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->hContact);
					// fall through
				case IDCANCEL:
				{
					struct windowInfo *wi = (struct windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
					if (wi->SendIfMy) DestroyWindow(wi->SendIfMy);
					if (wi->SendWhenThey) DestroyWindow(wi->SendWhenThey);
					free(wi);
					DestroyWindow(hwnd);
				}
				break;
				case IDC_DELETE:
				{
					HANDLE hContact = ((struct windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->hContact;
					deletePounce(hContact);
					SetDlgItemText(hwnd, IDC_MESSAGE, "");
					SetDlgItemText(hwnd, GRP_MSG, "The Message    (0 Characters)");
				}
				break;
				case IDC_DEFAULT:
				{
					HANDLE hContact = ((struct windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->hContact;
					char msg[1024];
					getDefaultMessage(hwnd, IDC_MESSAGE, hContact);
					_snprintf(msg, 1024, Translate("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
					SetDlgItemText(hwnd, GRP_MSG, msg);	
					DBWriteContactSettingWord(hContact,modname, "SendIfMyStatusIsFLAG", (WORD)DBGetContactSettingWord(NULL, modname, "SendIfMyStatusIsFLAG",0));
					DBWriteContactSettingWord(hContact,modname, "SendIfTheirStatusIsFLAG", (WORD)DBGetContactSettingWord(NULL, modname, "SendIfTheirStatusIsFLAG",0));
					DBWriteContactSettingByte(hContact,modname, "Reuse",(BYTE)DBGetContactSettingByte(NULL, modname, "Reuse",0));
					DBWriteContactSettingByte(hContact, modname, "GiveUpDays", (BYTE)DBGetContactSettingByte(NULL, modname, "GiveUpDays",0));
					DBWriteContactSettingDword(hContact, modname, "GiveUpDate", (DWORD)DBGetContactSettingDword(NULL, modname, "GiveUpDate",0));
					DBWriteContactSettingByte(hContact, modname, "ConfirmTimeout", (BYTE)DBGetContactSettingByte(NULL, modname, "ConfirmTimeout",0));
				}
				break;
				case IDC_SETTINGS:
					if (HIWORD(wParam) == LBN_SELCHANGE)
					{	
						struct windowInfo *wi = (struct windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
						int item = SendMessage(GetDlgItem(hwnd, IDC_SETTINGS), LB_GETCURSEL, 0, 0);
						char temp[5];
						saveLastSetting(wi->hContact, hwnd);
						hideAll(hwnd);
						switch (item)
						{
							case 0: // Send If My Status Is...
								statusModes(wi, 1);
							break;
							case 1: // Send If They Change status to
								statusModes(wi, 0);
							break;
							case 3: // Reuse Pounce
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG, "Reuse this message? (0 to use it once)");
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG2, "Times");
								SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(DBGetContactSettingByte(wi->hContact, modname, "Reuse", 0), temp, 10));
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
								ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
							break;
							case 4: // Give Up delay
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG, "Giveup after.. (0 to not giveup)");
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG2, "Days");
								SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(DBGetContactSettingByte(wi->hContact, modname, "GiveUpDays", 0), temp, 10));
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
								ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
							break;
							case 5:	// confirm window
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG, "Show confirmation window? (0 to not Show)");
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG2, "Seconds to wait before sending");
								SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(DBGetContactSettingByte(wi->hContact, modname, "ConfirmTimeout", 0), temp, 10));
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
								ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
							break;
							
							case 7: // send a file
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG, "Send a file");
								ShowWindow(GetDlgItem(hwnd, IDC_BUTTON), SW_SHOW);
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGTEXT), SW_SHOW);
							break;
						}
						DBWriteContactSettingByte(wi->hContact, modname, "LastSetting", (BYTE)item);
					}
				break;
			}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK BuddyPounceOptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) 
	{
		case WM_INITDIALOG:
		{
			struct windowInfo *wi = (struct windowInfo *)malloc(sizeof(struct windowInfo));
			char msg[1024];
			TranslateDialogDefault(hwnd);
			wi->hContact = 0;
			wi->SendIfMy = 0;
			wi->SendWhenThey = 0;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)wi);
			
			getDefaultMessage(hwnd, IDC_MESSAGE, wi->hContact);
			_snprintf(msg, 1024, Translate("The Message    (%d Characters)"), GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE)));
			SetDlgItemText(hwnd, GRP_MSG, msg);	
			populateSettingsList(GetDlgItem(hwnd, IDC_SETTINGS));
			SendMessage(GetDlgItem(hwnd,IDC_SPIN), UDM_SETRANGE, 0, (LPARAM) MAKELONG ((short) 1024, (short) 0));
			CheckDlgButton(hwnd, IDC_USEADVANCED, DBGetContactSettingByte(NULL, modname, "UseAdvanced", 0));
			CheckDlgButton(hwnd, IDC_SHOWDELIVERYMSGS, DBGetContactSettingByte(NULL, modname, "ShowDeliveryMessages", 1));
			DBWriteContactSettingByte(wi->hContact, modname, "LastSetting", 0);
		}
		return FALSE;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
							{
								char *text;
								struct windowInfo *wi = (struct windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
								HANDLE hContact = ((struct windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->hContact;
								int length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE))+1;
								if (length>1)
								{
									text = (char*)malloc(length);
									if (!text) { msg("Couldnt Allocate enough memory",""); break; }
									GetDlgItemText(hwnd, IDC_MESSAGE, text, length);
									DBWriteContactSettingString(hContact, modname, "PounceMsg", text);
									free(text);
								}
								else DBDeleteContactSetting(hContact, modname, "PounceMsg");
								DBWriteContactSettingByte(NULL, modname, "UseAdvanced", (BYTE)IsDlgButtonChecked(hwnd, IDC_USEADVANCED));
								DBWriteContactSettingByte(NULL, modname, "ShowDeliveryMessages", (BYTE)IsDlgButtonChecked(hwnd, IDC_SHOWDELIVERYMSGS));
								
								if (wi->SendIfMy) DestroyWindow(wi->SendIfMy);
								if (wi->SendWhenThey) DestroyWindow(wi->SendWhenThey);
							}
						break;
					}
				break;
			}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case IDC_MESSAGE:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						int length;
						char msg[1024];
						length = GetWindowTextLength(GetDlgItem(hwnd, IDC_MESSAGE));
						_snprintf(msg, 1024, Translate("The Message    (%d Characters)"), length);
						SetDlgItemText(hwnd, GRP_MSG, msg);
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
				}
				break;
				case IDC_SHOWDELIVERYMSGS:
				case IDC_USEADVANCED:
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				break;
				case IDC_SETTINGS:
					if (HIWORD(wParam) == LBN_SELCHANGE)
					{	
						struct windowInfo *wi = (struct windowInfo *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
						int item = SendMessage(GetDlgItem(hwnd, IDC_SETTINGS), LB_GETCURSEL, 0, 0);
						char temp[5];
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
						saveLastSetting(wi->hContact, hwnd);
						hideAll(hwnd);
						switch (item)
						{
							case 0: // Send If My Status Is...
								statusModes(wi, 1);
							break;
							case 1: // Send If They Change status to
								statusModes(wi, 0);
							break;
							case 3: // Reuse Pounce
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG, "Reuse this message? (0 to use it once)");
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG2, "Times");
								SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(DBGetContactSettingByte(wi->hContact, modname, "Reuse", 0), temp, 10));
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
								ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
							break;
							case 4: // Give Up delay
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG, "Giveup after.. (0 to not giveup)");
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG2, "Days");
								SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(DBGetContactSettingByte(wi->hContact, modname, "GiveUpDays", 0), temp, 10));
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
								ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
							break;
							case 5:	// confirm window
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG, "Show confirmation window? (0 to not Show)");
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG2), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG2, "Seconds to wait before sending");
								SetDlgItemText(hwnd, IDC_SETTINGNUMBER, _itot(DBGetContactSettingByte(wi->hContact, modname, "ConfirmTimeout", 0), temp, 10));
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGNUMBER), SW_SHOW);
								ShowWindow(GetDlgItem(hwnd, IDC_SPIN), SW_SHOW);
							break;
							
							case 7: // send a file
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGMSG), SW_SHOW);
								SetDlgItemText(hwnd, IDC_SETTINGMSG, "Send a file");
								ShowWindow(GetDlgItem(hwnd, IDC_BUTTON), SW_SHOW);
								ShowWindow(GetDlgItem(hwnd, IDC_SETTINGTEXT), SW_SHOW);
							break;
						}
						DBWriteContactSettingByte(wi->hContact, modname, "LastSetting", (BYTE)item);
					}
				break;
			}
		break;
	}
	return FALSE;
}



INT_PTR CALLBACK SendPounceDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
		case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			struct SendPounceDlgProcStruct *spdps = (struct SendPounceDlgProcStruct *)lParam;
			if (!spdps) DestroyWindow(hwnd);
			TranslateDialogDefault(hwnd);
			spdps->timer = DBGetContactSettingByte(spdps->hContact, modname, "ConfirmTimeout", 0);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (WPARAM)spdps);
			if (DBGetContactSetting(spdps->hContact,modname,"PounceMsg",&dbv)) DestroyWindow(hwnd);
			SetDlgItemText(hwnd,IDC_MESSAGE,dbv.pszVal);
			DBFreeVariant(&dbv);
			SetTimer(hwnd,1,1000,NULL);
			SendMessage(hwnd,WM_TIMER,0,0);
		}
		break;
		case WM_TIMER:
		{	
			struct SendPounceDlgProcStruct *spdps = (struct SendPounceDlgProcStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			char message[1024];
			_snprintf(message,sizeof(message),Translate("Pounce being sent to %s in %d seconds"),CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)spdps->hContact, 0),spdps->timer);
			SetDlgItemText(hwnd,LBL_CONTACT	,message);
			spdps->timer--;

			if (spdps->timer == -1)
			{
				KillTimer(hwnd,1);
				SendPounce(spdps->message, ((struct SendPounceDlgProcStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->hContact);
				free((char*)((struct SendPounceDlgProcStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->message);
				free((struct SendPounceDlgProcStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA));
				DestroyWindow(hwnd);
			}
			break;
		}
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
					KillTimer(hwnd,1);
					SendPounce(((struct SendPounceDlgProcStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->message, ((struct SendPounceDlgProcStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->hContact);
					// fall through
				case IDCANCEL:
					KillTimer(hwnd,1);
					free(((struct SendPounceDlgProcStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->message);
					free((struct SendPounceDlgProcStruct *)GetWindowLongPtr(hwnd, GWLP_USERDATA));
					DestroyWindow(hwnd);
				break;					
			}
		break;
	}
	return 0;
}

INT_PTR CALLBACK PounceSentDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	switch(msg) {
		case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (WPARAM)lParam);
			TranslateDialogDefault(hwnd);
			hContact = (HANDLE)lParam;
			if (DBGetContactSetting(hContact,modname,"PounceMsg",&dbv)) DestroyWindow(hwnd);
			SetDlgItemText(hwnd,IDC_MESSAGE,dbv.pszVal);
			DBFreeVariant(&dbv);
		}
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
				{
					char text[2048];
					GetWindowText(GetDlgItem(hwnd,IDOK),text,16);
					if (!strcmp(text,Translate("Retry")))
					{
						GetWindowText(GetDlgItem(hwnd,IDC_MESSAGE),text,2048);
						SendPounce(text,hContact);
					}
				}
					// fall through
				case IDCANCEL:
					DestroyWindow(hwnd);
				break;					
			}
		break;
	}
	return 0;
}
void CreateMessageAcknowlegedWindow(HANDLE hContact, int SentSuccess)
{
	HWND hwnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CONFIRMSEND), 0, PounceSentDlgProc, (LPARAM)hContact);
	char msg[256];
	if (SentSuccess)
	{
		_snprintf(msg,256,Translate("Message successfully sent to %s"),CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0)	);
		SetWindowText(GetDlgItem(hwnd,IDOK),"OK");
		ShowWindow(GetDlgItem(hwnd,IDCANCEL),0);
	}
	else 
	{
		_snprintf(msg,256,Translate("Message failed to send to %s"),CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0));
		SetWindowText(GetDlgItem(hwnd,IDOK),"Retry");
	}
	SetWindowText(GetDlgItem(hwnd,LBL_CONTACT),msg);
	SetWindowText(hwnd,modFullname);
}