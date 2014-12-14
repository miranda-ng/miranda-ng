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

			CheckDlgButton(hwndDlg,IDC_ENABLEREPLIER,db_get_b(NULL,protocolname,KEY_ENABLED,1)==1 ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemInt(hwndDlg,IDC_INTERVAL,db_get_w(NULL,protocolname,KEY_REPEATINTERVAL,300)/60,FALSE);

			if (!db_get_ts(NULL,protocolname,KEY_HEADING,&dbv))
			{
				SetDlgItemText(hwndDlg,IDC_HEADING,dbv.ptszVal);
				db_free(&dbv);
			}

			for (INT c = ID_STATUS_ONLINE; c < ID_STATUS_IDLE; c++)
			{
				mir_snprintf(tszStatus, SIZEOF(tszStatus), "%d", c);
				pszStatus=(TCHAR*)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,(WPARAM)c,GSMDF_TCHAR);
				if (c == ID_STATUS_ONLINE || c == ID_STATUS_FREECHAT || c == ID_STATUS_INVISIBLE)
					continue;
				else
				{
					SendDlgItemMessage(hwndDlg,IDC_STATUSMODE,CB_ADDSTRING,0,(LPARAM)pszStatus);

					if (!db_get_ts(NULL, protocolname, tszStatus, &dbv))
					{
						if (c < ID_STATUS_FREECHAT)
							ptszMessage[c-ID_STATUS_ONLINE-1] = _tcsdup(dbv.ptszVal);
						else if (c > ID_STATUS_INVISIBLE)
							ptszMessage[c-ID_STATUS_ONLINE-3] = _tcsdup(dbv.ptszVal);
						db_free(&dbv);
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
						int size=SendDlgItemMessage(hwndDlg,IDC_MESSAGE,WM_GETTEXTLENGTH,0,0)+1;
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
					TCHAR ptszText[1024];
					BOOL translated;

					BOOL fEnabled = IsDlgButtonChecked(hwndDlg, IDC_ENABLEREPLIER) == 1;
					db_set_b(NULL, protocolname, KEY_ENABLED, (BYTE)fEnabled);
					CLISTMENUITEM mi = { sizeof(mi) };
					mi.flags = CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
					mi.ptszName = fEnabled ? LPGENT("Disable Auto&reply") : LPGENT("Enable Auto&reply");
					mi.icolibItem = fEnabled ? iconList[0].hIcolib : iconList[1].hIcolib;
					Menu_ModifyItem(hEnableMenu, &mi);

					GetDlgItemText(hwndDlg, IDC_HEADING, ptszText, SIZEOF(ptszText));
					db_set_ts(NULL, protocolname, KEY_HEADING, ptszText);

					INT size = GetDlgItemInt(hwndDlg, IDC_INTERVAL, &translated, FALSE);
					if (translated)
						interval=size*60;
					db_set_w(NULL,protocolname,KEY_REPEATINTERVAL,interval);

					size=SendDlgItemMessage(hwndDlg,IDC_MESSAGE,WM_GETTEXTLENGTH,0,0)+1;
					GetDlgItemText(hwndDlg,IDC_MESSAGE,ptszMessage[lastIndex],size);

					for (int c=ID_STATUS_ONLINE; c<ID_STATUS_IDLE; c++)
					{
						if (c == ID_STATUS_ONLINE || c == ID_STATUS_FREECHAT || c == ID_STATUS_INVISIBLE)
							continue;
						else
						{
							char szStatus[6] = {0};
							mir_snprintf(szStatus, SIZEOF(szStatus), "%d", c);

							if (c<ID_STATUS_FREECHAT && ptszMessage[c-ID_STATUS_ONLINE-1])
								db_set_ts(NULL,protocolname,szStatus,ptszMessage[c-ID_STATUS_ONLINE-1]);
							else if (c>ID_STATUS_INVISIBLE && ptszMessage[c-ID_STATUS_ONLINE-3])
								db_set_ts(NULL,protocolname,szStatus,ptszMessage[c-ID_STATUS_ONLINE-3]);
							else
								db_unset(NULL,protocolname,szStatus);
						}
					}
					return TRUE;
				}
			}
			break;
		case WM_DESTROY:
			for (int c=ID_STATUS_ONLINE; c<ID_STATUS_IDLE; c++)
			{
				if (c == ID_STATUS_ONLINE || c == ID_STATUS_FREECHAT || c == ID_STATUS_INVISIBLE)
					continue;
				else
				{
					if (c<ID_STATUS_FREECHAT)
						ptszMessage[c-ID_STATUS_ONLINE-1]=NULL;
					else if (c>ID_STATUS_INVISIBLE)
						ptszMessage[c-ID_STATUS_ONLINE-3]=NULL;
				}
			}
			break;
	}
	return FALSE;
}

INT OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = -790000000;
	odp.hInstance = hinstance;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTION);
	odp.pszTitle = LPGEN("Simple Auto Replier");
	odp.pszGroup = LPGEN("Message Sessions");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = DlgProcOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}