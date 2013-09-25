#include "headers.h"

INT_PTR CALLBACK DlgProcOpts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInitDone = true;
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			bInitDone = false;
			DBVARIANT dbv;
			CheckDlgButton(hwnd,IDC_EXPANDSETTINGS,db_get_b(NULL,modname,"ExpandSettingsOnOpen",0));
			CheckDlgButton(hwnd,IDC_RESTORESETTINGS,db_get_b(NULL,modname,"RestoreOnOpen",1));
			CheckDlgButton(hwnd,IDC_USEKNOWNMODS,db_get_b(NULL,modname,"UseKnownModList",0));
			CheckDlgButton(hwnd,IDC_WARNONDEL,db_get_b(NULL,modname,"WarnOnDelete",1));
			CheckDlgButton(hwnd,IDC_MENU,db_get_b(NULL,modname,"UserMenuItem",0));
			CheckDlgButton(hwnd,IDC_POPUPS,usePopups);
			if (!db_get(NULL,modname,"CoreModules",&dbv) && dbv.type == DBVT_ASCIIZ)
				SetDlgItemText(hwnd,IDC_MODULES,dbv.pszVal);
			db_free(&dbv);
			SetDlgItemInt(hwnd,IDC_POPUPTIMEOUT,db_get_w(NULL,modname,"PopupDelay",4),0);
			SendDlgItemMessage(hwnd, IDC_COLOUR, CPM_SETCOLOUR, 0, (LPARAM)db_get_dw(NULL,modname,"PopupColour",RGB(255,0,0)));
			TranslateDialogDefault(hwnd);
			bInitDone = true;
		}
		return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_RESTORESETTINGS:
				case IDC_EXPANDSETTINGS:
				case IDC_USEKNOWNMODS:
				case IDC_MENU:
				case IDC_POPUPS:
				case IDC_WARNONDEL:
				case IDC_COLOUR:
					SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					break;
				case IDC_POPUPTIMEOUT:
				case IDC_MODULES:
					if(bInitDone && (HIWORD(wParam) == EN_CHANGE))
						SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					break;
			}
		break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
							{
								char mods[4096];
								db_set_b(NULL,modname,"ExpandSettingsOnOpen",(BYTE)IsDlgButtonChecked(hwnd,IDC_EXPANDSETTINGS));
								db_set_b(NULL,modname,"RestoreOnOpen",(BYTE)IsDlgButtonChecked(hwnd,IDC_RESTORESETTINGS));
								db_set_b(NULL,modname,"WarnOnDelete",(BYTE)IsDlgButtonChecked(hwnd,IDC_WARNONDEL));
								db_set_b(NULL,modname,"UserMenuItem",(BYTE)IsDlgButtonChecked(hwnd,IDC_MENU));
								db_set_b(NULL,modname,"UseKnownModList",(BYTE)IsDlgButtonChecked(hwnd,IDC_USEKNOWNMODS));
								usePopups = IsDlgButtonChecked(hwnd,IDC_POPUPS);
								db_set_b(NULL,modname,"UsePopUps",(BYTE)usePopups);
								if (GetDlgItemText(hwnd,IDC_MODULES,mods,4096))
									db_set_s(NULL,modname,"CoreModules",mods);
								db_set_w(NULL,modname,"PopupDelay",(WORD)GetDlgItemInt(hwnd,IDC_POPUPTIMEOUT,NULL,0));
								db_set_dw(NULL,modname,"PopupColour",(DWORD)SendDlgItemMessage(hwnd, IDC_COLOUR, CPM_GETCOLOUR, 0, 0));

								Menu_ShowItem(hUserMenu, IsDlgButtonChecked(hwnd,IDC_MENU));
							}
							return TRUE;
					}
					break;
			}
			break;
	}
	return FALSE;
}

INT OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszGroup = LPGEN("Database");
	odp.pszTitle = modFullname;
	odp.pfnDlgProc = DlgProcOpts;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);
	return 0;
}