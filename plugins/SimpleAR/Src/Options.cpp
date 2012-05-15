#include "Common.h"

TCHAR* ptszMessage[6]={0};
INT lastIndex=-1;

INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			TranslateDialogDefault(hwndDlg);
			TCHAR* pszStatus;
			char tszStatus[6]={0};

			CheckDlgButton(hwndDlg,IDC_ENABLEREPLIER,DBGetContactSettingByte(NULL,protocolname,KEY_ENABLED,1)==1);
			SetDlgItemInt(hwndDlg,IDC_INTERVAL,DBGetContactSettingWord(NULL,protocolname,KEY_REPEATINTERVAL,300)/60,FALSE);

			if (!DBGetContactSettingTString(NULL,protocolname,KEY_HEADING,&dbv))
			{
				SetDlgItemText(hwndDlg,IDC_HEADING,dbv.ptszVal);
				DBFreeVariant(&dbv);
			}

			for (INT c = ID_STATUS_ONLINE; c < ID_STATUS_IDLE; c++)
			{
				mir_snprintf(tszStatus, SIZEOF(tszStatus), "%d", c);
				pszStatus=(TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)c,GSMDF_TCHAR);
				if (c == 40072 || c == 40077 || c == 40078)
					continue;
				else
				{
					SendDlgItemMessage(hwndDlg,IDC_STATUSMODE,CB_ADDSTRING,0,(LPARAM)pszStatus);

					if (!DBGetContactSettingTString(NULL, protocolname, tszStatus, &dbv))
					{
						if (c < 40077)
							ptszMessage[c-ID_STATUS_ONLINE-1] = _tcsdup(dbv.ptszVal);
						else if (c > 40078)
							ptszMessage[c-ID_STATUS_ONLINE-3] = _tcsdup(dbv.ptszVal);
						DBFreeVariant(&dbv);
					}
				}
			}

			SendDlgItemMessage(hwndDlg,IDC_STATUSMODE,CB_SETCURSEL,0,0);

			lastIndex=0;
			SetDlgItemText(hwndDlg,IDC_MESSAGE,ptszMessage[lastIndex]);

			return TRUE;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_ENABLEREPLIER:
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_STATUSMODE:
					// First, save last, then load current
					if (lastIndex>-1)
					{
						INT size=SendDlgItemMessage(hwndDlg,IDC_MESSAGE,WM_GETTEXTLENGTH,0,0)+1;
						GetDlgItemText(hwndDlg,IDC_MESSAGE,ptszMessage[lastIndex],size);
					}
					lastIndex=SendDlgItemMessage(hwndDlg,IDC_STATUSMODE,CB_GETCURSEL,0,0);
					SetDlgItemText(hwndDlg,IDC_MESSAGE,ptszMessage[lastIndex]);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_DEFAULT:
					SetDlgItemText(hwndDlg,IDC_MESSAGE,TranslateTS(ptszDefaultMsg[lastIndex]));
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
				case IDC_INTERVAL:
				case IDC_HEADING:
				case IDC_MESSAGE:
					if ((HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == EN_CHANGE) && (HWND)lParam == GetFocus())
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					break;
			}
			break;
		
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->code)
			{
				case PSN_APPLY:
				{
					INT size;
					TCHAR ptszText[1024];
					char szStatus[6]={0};
					BOOL translated;

					fEnabled=IsDlgButtonChecked(hwndDlg,IDC_ENABLEREPLIER)==1;
					DBWriteContactSettingByte(NULL,protocolname,KEY_ENABLED,(BYTE)fEnabled);

					GetDlgItemText(hwndDlg,IDC_HEADING,ptszText,SIZEOF(ptszText));
					DBWriteContactSettingTString(NULL,protocolname,KEY_HEADING,ptszText);

					size=GetDlgItemInt(hwndDlg,IDC_INTERVAL,&translated,FALSE);
					if (translated)
						interval=size*60;
					DBWriteContactSettingWord(NULL,protocolname,KEY_REPEATINTERVAL,interval);

					size=SendDlgItemMessage(hwndDlg,IDC_MESSAGE,WM_GETTEXTLENGTH,0,0)+1;
					GetDlgItemText(hwndDlg,IDC_MESSAGE,ptszMessage[lastIndex],size);

					for (int c=ID_STATUS_ONLINE; c<ID_STATUS_IDLE; c++)
					{
						if (c == 40072 || c == 40077 || c == 40078)
							continue;
						else
						{
						mir_snprintf(szStatus,SIZEOF(szStatus),"%d",c);

						if (c<40077 && ptszMessage[c-ID_STATUS_ONLINE-1])
							DBWriteContactSettingTString(NULL,protocolname,szStatus,ptszMessage[c-ID_STATUS_ONLINE-1]);
						else if (c>40078 && ptszMessage[c-ID_STATUS_ONLINE-3])
							DBWriteContactSettingTString(NULL,protocolname,szStatus,ptszMessage[c-ID_STATUS_ONLINE-3]);
						else
							DBDeleteContactSetting(NULL,protocolname,szStatus);
						}
					}
					return TRUE;
				}
			}
			break;
		case WM_DESTROY:
			for (int c=ID_STATUS_ONLINE; c<ID_STATUS_IDLE; c++)
			{
				if (c == 40072 || c == 40077 || c == 40078)
					continue;
				else
				{
					if (c<40077)
						ptszMessage[c-ID_STATUS_ONLINE-1]=NULL;
					else if (c>40078)
						ptszMessage[c-ID_STATUS_ONLINE-3]=NULL;
				}
			}
			break;
	}
	return FALSE;
}

INT OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=-790000000;
	odp.hInstance=hinstance;
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPTION);
	odp.ptszTitle= _T("Simple Auto Replier");
	odp.ptszGroup = _T("Plugins");
	odp.flags=ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.nIDBottomSimpleControl=0;
	odp.pfnDlgProc=DlgProcOpts;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
	return 0;
}