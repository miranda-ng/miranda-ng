#include "headers.h"

INT_PTR CALLBACK DlgProcOpts(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{
			DBVARIANT dbv;
			CheckDlgButton(hwnd,IDC_EXPANDSETTINGS,DBGetContactSettingByte(NULL,modname,"ExpandSettingsOnOpen",0));
			CheckDlgButton(hwnd,IDC_RESTORESETTINGS,DBGetContactSettingByte(NULL,modname,"RestoreOnOpen",1));
			CheckDlgButton(hwnd,IDC_USEKNOWNMODS,DBGetContactSettingByte(NULL,modname,"UseKnownModList",0));
			CheckDlgButton(hwnd,IDC_WARNONDEL,DBGetContactSettingByte(NULL,modname,"WarnOnDelete",1));
			CheckDlgButton(hwnd,IDC_MENU,DBGetContactSettingByte(NULL,modname,"UserMenuItem",0));
			CheckDlgButton(hwnd,IDC_POPUPS,usePopUps);
			if (!DBGetContactSetting(NULL,modname,"CoreModules",&dbv) && dbv.type == DBVT_ASCIIZ)
				SetDlgItemText(hwnd,IDC_MODULES,dbv.pszVal);
			DBFreeVariant(&dbv);
			SetDlgItemInt(hwnd,IDC_POPUPTIMEOUT,DBGetContactSettingWord(NULL,modname,"PopupDelay",4),0);
			SendDlgItemMessage(hwnd, IDC_COLOUR, CPM_SETCOLOUR, 0, (LPARAM)DBGetContactSettingDword(NULL,modname,"PopupColour",RGB(255,0,0)));
			TranslateDialogDefault(hwnd);
		}
		return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_RESTORESETTINGS:
				case IDC_EXPANDSETTINGS:
				case IDC_USEKNOWNMODS:
				case IDC_MODULES:
				case IDC_MENU:
				case IDC_POPUPS:
				case IDC_WARNONDEL:
				case IDC_COLOUR:
				case IDC_POPUPTIMEOUT:
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
								CLISTMENUITEM mi = {0};
								char mods[4096];
								DBWriteContactSettingByte(NULL,modname,"ExpandSettingsOnOpen",(BYTE)IsDlgButtonChecked(hwnd,IDC_EXPANDSETTINGS));
								DBWriteContactSettingByte(NULL,modname,"RestoreOnOpen",(BYTE)IsDlgButtonChecked(hwnd,IDC_RESTORESETTINGS));
								DBWriteContactSettingByte(NULL,modname,"WarnOnDelete",(BYTE)IsDlgButtonChecked(hwnd,IDC_WARNONDEL));
								DBWriteContactSettingByte(NULL,modname,"UserMenuItem",(BYTE)IsDlgButtonChecked(hwnd,IDC_MENU));
								DBWriteContactSettingByte(NULL,modname,"UseKnownModList",(BYTE)IsDlgButtonChecked(hwnd,IDC_USEKNOWNMODS));
								usePopUps = IsDlgButtonChecked(hwnd,IDC_POPUPS);
								DBWriteContactSettingByte(NULL,modname,"UsePopUps",(BYTE)usePopUps);
								if (GetDlgItemText(hwnd,IDC_MODULES,mods,4096))
									DBWriteContactSettingString(NULL,modname,"CoreModules",mods);
								DBWriteContactSettingWord(NULL,modname,"PopupDelay",(WORD)GetDlgItemInt(hwnd,IDC_POPUPTIMEOUT,NULL,0));
								DBWriteContactSettingDword(NULL,modname,"PopupColour",(DWORD)SendDlgItemMessage(hwnd, IDC_COLOUR, CPM_GETCOLOUR, 0, 0));

								mi.cbSize = sizeof(mi);

								if (!IsDlgButtonChecked(hwnd,IDC_MENU))
									mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
								else
									mi.flags = CMIM_FLAGS;

							    CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM) hUserMenu, (LPARAM) & mi);

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
	OPTIONSDIALOGPAGE odp;

	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=0;
	odp.hInstance=hInst;
	odp.pszTemplate=MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszGroup= "Services";
	odp.pszTitle= modFullname;
	odp.pfnDlgProc = DlgProcOpts;
	odp.flags = ODPF_BOLDGROUPS;
	odp.expertOnlyControls=NULL;
	CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);

	return 0;
}