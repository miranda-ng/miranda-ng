#include "commonheaders.h"
#include <m_utils.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_icolib.h>

#define MS_DB_CHANGEPASSWORD "DB/ChangePassword"

extern LIST<CryptoModule> arCryptors;

CDdxMmapSA* g_Db;
HANDLE hSetPwdMenu;

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL ShowDlgItem(HWND hwndDlg, int iIDCtrl, BOOL bShow)
{
	HWND hwndCtrl = GetDlgItem(hwndDlg, iIDCtrl);
	if (!hwndCtrl) return FALSE;

	// Avoid flickering
	if (bShow && IsWindowVisible(hwndCtrl))
		return TRUE;

	return ShowWindow(hwndCtrl, (bShow ? SW_SHOW : SW_HIDE));
}

BOOL EnableDlgItem(HWND hwndDlg, int iIDCtrl, BOOL bEnable)
{
	HWND hwndCtrl = GetDlgItem(hwndDlg, iIDCtrl);
	if (!hwndCtrl) return FALSE;

	// Avoid flickering
	if (IsWindowEnabled(hwndCtrl) == bEnable)
		return (bEnable == FALSE);

	return EnableWindow(hwndCtrl, bEnable);
}

BOOL IsDlgItemEnabled(HWND hwndDlg, int iIDCtrl)
{
	HWND hwndCtrl = GetDlgItem(hwndDlg, iIDCtrl);
	if (!hwndCtrl) return FALSE;
	return IsWindowEnabled(hwndCtrl);
}

static int OptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.position = -790000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.ptszTitle = LPGENT("Database Features");
	odp.ptszGroup = LPGENT("Services");
	odp.pfnDlgProc = DlgProcOptions;
	Options_AddPage(wParam, &odp);
	return 0;
}

INT_PTR ChangePassword(WPARAM wParam, LPARAM lParam)
{
	if (g_Db->m_bEncoding)
		g_Db->ChangePwd();
	else
		g_Db->EncryptDB();

	return 0;
}

void xModifyMenu(HANDLE hMenu,long flags,const TCHAR* name, HICON hIcon)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | CMIF_TCHAR;
	mi.flags |= name ? CMIM_NAME : 0;
	mi.flags |= hIcon ? CMIM_ICON : 0;
	mi.flags |= flags;
	mi.ptszName = (TCHAR*)name;
	mi.hIcon = hIcon;
	CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)hMenu,(LPARAM)&mi);
}

static IconItem iconList[] = 
{
	{ LPGEN("Database"), "database", IDI_ICON2 },
	{ LPGEN("Change Password"), "password", IDI_ICON3 }
};

int InitMenus(WPARAM, LPARAM)
{
	HookEvent(ME_OPT_INITIALISE, OptionsInit);

	Icon_Register(g_hInst, LPGEN("Database"), iconList, SIZEOF(iconList));

	// main menu item
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ALL | CMIF_TCHAR | CMIF_ICONFROMICOLIB;
	mi.icolibItem = iconList[1].hIcolib;
	mi.ptszName = (g_Db->m_bEncoding) ? LPGENT("Change password") : LPGENT("Set password");
	mi.ptszPopupName = LPGENT("Database");
	mi.pszService = MS_DB_CHANGEPASSWORD;
	mi.position = 500100000;
	hSetPwdMenu = Menu_AddMainMenuItem(&mi);
	return 0;
}

int InitDialogs()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, InitMenus);
	CreateServiceFunction(MS_DB_CHANGEPASSWORD, ChangePassword);
	return 0;
}

int ImageList_AddIcon_IconLibLoaded(HIMAGELIST hIml, char* name)
{
	HICON hIcon = Skin_GetIcon(name);
	int res = ImageList_AddIcon(hIml, hIcon);
	return res;
}

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_MODULES);
	LVCOLUMN col;
	LVITEM item;
	int i, iRow, iIndex;
	NMLISTVIEW * hdr = (NMLISTVIEW *) lParam;
	WORD uid;
	HIMAGELIST hIml;

	switch ( msg ) {
	case WM_INITDIALOG:
		hIml = ImageList_Create(16, 16, ILC_MASK | (IsWinVerXPPlus()? ILC_COLOR32 : ILC_COLOR16), 2, 0);
		TranslateDialogDefault( hwndDlg );

		ImageList_AddIcon_IconLibLoaded( hIml, "core_main_29" );
		ImageList_AddIcon_IconLibLoaded( hIml, "core_main_30" );
		ListView_SetImageList( hwndList, hIml, LVSIL_SMALL );

		col.pszText = NULL;
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 50;
		ListView_InsertColumn(hwndList, 1, &col);

		col.pszText = TranslateT("Dll");
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.fmt = LVCFMT_LEFT;
		col.cx = 1000;
		ListView_InsertColumn(hwndList, 2, &col);

		col.pszText = TranslateT("Name");
		col.cx = 1000;
		ListView_InsertColumn(hwndList, 3, &col);

		col.pszText = TranslateT("Version");
		col.cx = 1000;
		ListView_InsertColumn(hwndList, 4, &col);

		ListView_SetExtendedListViewStyleEx(hwndList, 0, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);

		uid = DBGetContactSettingWord(NULL, "SecureMMAP", "CryptoModule", 0);

		for (i = 0; i < arCryptors.getCount(); i++) {
			TCHAR buf[100];

			item.mask = LVIF_TEXT;
			item.iItem = i;
			item.iSubItem = 0;
			item.pszText = NULL;
			iRow = ListView_InsertItem(hwndList, &item);

			ListView_SetItemText(hwndList, iRow, 1, arCryptors[i]->dllname);
			_tcsncpy(buf, _A2T(arCryptors[i]->cryptor->Name), SIZEOF(buf));
			ListView_SetItemText(hwndList, iRow, 2, buf);
			mir_sntprintf(buf,SIZEOF(buf),_T("%d.%d.%d.%d"), HIBYTE(HIWORD(arCryptors[i]->cryptor->Version)), LOBYTE(HIWORD(arCryptors[i]->cryptor->Version)), HIBYTE(LOWORD(arCryptors[i]->cryptor->Version)), LOBYTE(LOWORD(arCryptors[i]->cryptor->Version)));
			ListView_SetItemText(hwndList, iRow, 3, buf);

			if (uid == arCryptors[i]->cryptor->uid && g_Db->m_bEncoding)
				ListView_SetCheckState(hwndList, i, 1);

			item.mask = LVIF_IMAGE;
			item.iItem = iRow;
			item.iSubItem = 0;
			item.iImage = ( CryptoEngine == arCryptors[i]->cryptor && g_Db->m_bEncoding ) ? 0 : 1;
			ListView_SetItem( hwndList, &item );
		}

		ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, 2, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, 3, LVSCW_AUTOSIZE);
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD(wParam) == STN_CLICKED ) {
			switch (LOWORD(wParam)) {
				case IDC_EMAIL:
				case IDC_SITE:
				{
					char buf[512];
					char * p = &buf[7];
					lstrcpyA(buf,"mailto:");
					if ( GetWindowTextA(GetDlgItem(hwndDlg, LOWORD(wParam)), p, SIZEOF(buf) - 7)) {
						CallService(MS_UTILS_OPENURL,0,(LPARAM) (LOWORD(wParam) == IDC_EMAIL ? buf : p));
					}
					break;
		}	}	}
		break;

	case WM_NOTIFY:
		if ( hdr && hdr->hdr.code == LVN_ITEMCHANGED && IsWindowVisible(hdr->hdr.hwndFrom) && hdr->iItem != (-1)) {
			iIndex = hdr->iItem;
			if (hdr->uNewState & 0x2000){
				for (i = 0; i < arCryptors.getCount(); i++) {
					if (i != iIndex) ListView_SetCheckState(hwndList, i, 0);
				}
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			if (hdr->uNewState & 0x1000){
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			if (hdr->uNewState & LVIS_SELECTED){
				SetDlgItemTextA(hwndDlg, IDC_AUTHOR, arCryptors[iIndex]->cryptor->Author);
				{
					TCHAR* info_t = mir_a2t((char*)(arCryptors[iIndex]->cryptor->Info));
					SetDlgItemText(hwndDlg, IDC_INFO, TranslateTS(info_t));
					mir_free(info_t);
				}
				SetDlgItemTextA(hwndDlg, IDC_SITE, arCryptors[iIndex]->cryptor->Site);
				SetDlgItemTextA(hwndDlg, IDC_EMAIL, arCryptors[iIndex]->cryptor->Email);
				SetDlgItemTextA(hwndDlg, IDC_ENC, arCryptors[iIndex]->cryptor->Name);
				SetDlgItemInt(hwndDlg, IDC_UID, arCryptors[iIndex]->cryptor->uid, 0);
			} else {
				SetDlgItemTextA(hwndDlg, IDC_AUTHOR, "");
				SetDlgItemTextA(hwndDlg, IDC_INFO, "");
				SetDlgItemTextA(hwndDlg, IDC_SITE, "");
				SetDlgItemTextA(hwndDlg, IDC_EMAIL, "");
				SetDlgItemTextA(hwndDlg, IDC_ENC, "");
				SetDlgItemTextA(hwndDlg, IDC_UID, "");
			}

			break;
		}
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			int alg = -1;
			for (i = 0; i < arCryptors.getCount(); i++) {
				if (ListView_GetCheckState(hwndList, i)) {
					alg = i;
					break;
				}
			}

			if (alg > -1){
				if (!g_Db->m_bEncoding){
					DBWriteContactSettingWord(NULL, "SecureMMAP", "CryptoModule", arCryptors[alg]->cryptor->uid);
					g_Db->EncryptDB();
				}
				else {
					if (arCryptors[alg]->cryptor->uid != DBGetContactSettingWord(NULL, "SecureMMAP", "CryptoModule", -1)) {
						DBWriteContactSettingWord(NULL, "SecureMMAP", "CryptoModule", arCryptors[alg]->cryptor->uid);
						g_Db->RecryptDB();
					}
				}
			}
			else if (g_Db->m_bEncoding)
				g_Db->DecryptDB();

			uid = DBGetContactSettingWord(NULL, "SecureMMAP", "CryptoModule", 0);

			for (i = 0; i < arCryptors.getCount(); i++) {
				if (uid == arCryptors[i]->cryptor->uid && g_Db->m_bEncoding)
					ListView_SetCheckState(hwndList, i, 1);

				item.mask = LVIF_IMAGE;
				item.iItem = i;
				item.iSubItem = 0;
				item.iImage = ( CryptoEngine == arCryptors[i]->cryptor && g_Db->m_bEncoding ) ? 0 : 1;

				ListView_SetItem( hwndList, &item );
			}

			return TRUE;

		}
		break;
	}

	return FALSE;
}

UINT oldLangID = 0;
void LanguageChanged(HWND hDlg)
{
	UINT LangID = (UINT)GetKeyboardLayout(0);
	char Lang[3] = {0};
	if (LangID != oldLangID)
	{
		oldLangID = LangID;
		GetLocaleInfoA(MAKELCID((LangID & 0xffffffff),  SORT_DEFAULT),  LOCALE_SABBREVLANGNAME, Lang, 2);
		Lang[0] = toupper(Lang[0]);
		Lang[1] = tolower(Lang[1]);
		SetDlgItemTextA(hDlg, IDC_LANG, Lang);
	}
}

extern BOOL wrongPass;

INT_PTR CALLBACK DlgStdInProc(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HICON hIcon = 0;
	TCHAR tszHeaderTxt[256];

	switch(uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);

		hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2));
		SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_SETICON, 0, (LPARAM)hIcon);

		if (!wrongPass) {
			mir_sntprintf(tszHeaderTxt, SIZEOF(tszHeaderTxt), _T("%s\n%s"), TranslateT("Please type in your password for"), (TCHAR*)lParam);
			SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), tszHeaderTxt);
		}
		else {
			if (wrongPass > 2) {
				HWND hwndCtrl = GetDlgItem(hDlg, IDC_USERPASS);
				EnableWindow(hwndCtrl, FALSE);
				hwndCtrl = GetDlgItem(hDlg, IDOK);
				EnableWindow(hwndCtrl, FALSE);
				SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Too many errors!"));
			}
			else SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Password is not correct!"));
		}
		oldLangID = 0;
		SetTimer(hDlg,1,200,NULL);
		LanguageChanged(hDlg);
		return TRUE;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LANG)) {
			SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (BOOL)GetSysColorBrush(COLOR_HIGHLIGHT);
		}
		return FALSE;

	case WM_COMMAND:
		switch( LOWORD(wParam)) {
		case IDOK:
			if (!GetWindowLongPtr(hDlg,GWLP_USERDATA)) {
				g_Db->encryptKeyLength = GetDlgItemTextA(hDlg, IDC_USERPASS, g_Db->encryptKey, 254);
				EndDialog(hDlg,IDOK);
			}
			break;

		case IDCANCEL:
			EndDialog(hDlg,IDCANCEL);
		}
		break;

	case WM_TIMER:
		LanguageChanged(hDlg);
		return FALSE;

	case WM_DESTROY:
		KillTimer(hDlg, 1);
		DestroyIcon(hIcon);
	}

	return FALSE;
}

INT_PTR CALLBACK DlgStdNewPass(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HICON hIcon = 0;

	switch(uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);

		hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2));
		SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_SETICON, 0, (LPARAM)hIcon);

		SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Please enter your new password"));

		oldLangID = 0;
		SetTimer(hDlg,1,200,NULL);
		LanguageChanged(hDlg);
		return TRUE;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LANG)) {
			SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (BOOL)GetSysColorBrush(COLOR_HIGHLIGHT);
		}
		return FALSE;

	case WM_COMMAND:
		{
			UINT uid = LOWORD(wParam);
			if (uid == IDOK) {
				if (!GetWindowLongPtr(hDlg,GWLP_USERDATA)) {
					char pass1[255], pass2[255];
					if (GetDlgItemTextA(hDlg, IDC_USERPASS1, pass1, 254) < 3){
						SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Password is too short!"));
						SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
						SetDlgItemTextA(hDlg,IDC_USERPASS1,"");
						SetDlgItemTextA(hDlg,IDC_USERPASS2,"");
					}
					else {
						GetDlgItemTextA(hDlg, IDC_USERPASS2, pass2, 254);
						if (!strcmp(pass1, pass2)) {
							g_Db->encryptKeyLength = strlen(pass1);
							strcpy(g_Db->encryptKey, pass1);
							EndDialog(hDlg,IDOK);
						}
						else {
							SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Passwords do not match!"));
							SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
							SetDlgItemTextA(hDlg,IDC_USERPASS1,"");
							SetDlgItemTextA(hDlg,IDC_USERPASS2,"");
						}
					}
				}
			}
			else if (uid == IDCANCEL)
				EndDialog(hDlg,IDCANCEL);
		}
		break;

	case WM_TIMER:
		LanguageChanged(hDlg);
		return FALSE;

	case WM_DESTROY:
		KillTimer(hDlg, 1);
		DestroyIcon(hIcon);
		return FALSE;
	}
	return FALSE;
}

char* newPass;

INT_PTR CALLBACK DlgChangePass(HWND hDlg, UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	HICON hIcon = 0;

	switch(uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);

		hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2));
		SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_SETICON, 0, (LPARAM)hIcon);
		SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Change password"));

		newPass = (char*)lParam;
		oldLangID = 0;
		SetTimer(hDlg,1,200,NULL);
		LanguageChanged(hDlg);

		return TRUE;

	case WM_CTLCOLORSTATIC:
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_LANG)) {
			SetTextColor((HDC)wParam, GetSysColor(COLOR_HIGHLIGHTTEXT));
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (BOOL)GetSysColorBrush(COLOR_HIGHLIGHT);
		}

		return FALSE;

	case WM_COMMAND:
		{
			UINT uid = LOWORD(wParam);
			if (uid == IDOK) {
				char pass1[255], pass2[255], oldpass[255];
				GetDlgItemTextA(hDlg, IDC_OLDPASS, oldpass, 254);
				if (strcmp(oldpass, g_Db->encryptKey)) {
					SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Wrong password!"));
					SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
					break;
				}
				if (GetDlgItemTextA(hDlg, IDC_NEWPASS1, pass1, 254) < 3){
					SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Password is too short!"));
					SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);

				}
				else {
					GetDlgItemTextA(hDlg, IDC_NEWPASS2, pass2, 254);
					if (!strcmp(pass1, pass2)) {
						strcpy(newPass, pass1);
						EndDialog(hDlg,IDOK);
					}
					else {
						SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Passwords do not match!"));
						SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
					}
				}
			}
			else if (uid == IDCANCEL)
				EndDialog(hDlg,IDCANCEL);
			else if (uid == IDREMOVE) {
				char oldpass[255];
				GetDlgItemTextA(hDlg, IDC_OLDPASS, oldpass, 254);
				if (strcmp(oldpass, g_Db->encryptKey)) {
					SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Wrong password!"));
					SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
					break;
				}
				EndDialog(hDlg, IDREMOVE);
			}
		}
		break;

	case WM_TIMER:
		LanguageChanged(hDlg);
		return FALSE;

	case WM_DESTROY:
		KillTimer(hDlg, 1);
		return FALSE;
	}
	return FALSE;
}
