#include "commonheaders.h"
#include <m_utils.h>
#include <m_clist.h>
#include <m_options.h>
#include <m_icolib.h>

#define MS_DB_CHANGEPASSWORD "DB/ChangePassword"

extern LIST<CryptoModule> arCryptors;

HGENMENU hSetPwdMenu;

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

static int OptionsInit(void *obj, WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = -790000000;
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.ptszTitle = LPGENT("Database Features");
	odp.ptszGroup = LPGENT("Database");
	odp.pfnDlgProc = DlgProcOptions;
	odp.dwInitParam = (LPARAM)obj;
	Options_AddPage(wParam, &odp);
	return 0;
}

INT_PTR ChangePassword(void* obj, LPARAM wParam, LPARAM lParam)
{
	CDbxMmapSA *p = (CDbxMmapSA*)obj;

	if (p->m_bEncoding)
		p->ChangePwd();
	else
		p->EncryptDB();

	return 0;
}

void xModifyMenu(HGENMENU hMenu, long flags, const TCHAR *name, HICON hIcon)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | CMIF_TCHAR;
	mi.flags |= name ? CMIM_NAME : 0;
	mi.flags |= hIcon ? CMIM_ICON : 0;
	mi.flags |= flags;
	mi.ptszName = (TCHAR*)name;
	mi.hIcon = hIcon;
	Menu_ModifyItem(hMenu, &mi);
}

static IconItem iconList[] =
{
	{ LPGEN("Database"), "database", IDI_ICON2 },
	{ LPGEN("Change Password"), "password", IDI_ICON3 }
};

int InitMenus(void *obj, WPARAM, LPARAM)
{
	CDbxMmapSA *p = (CDbxMmapSA*)obj;

	HookEventObj(ME_OPT_INITIALISE, OptionsInit, obj);

	Icon_Register(g_hInst, LPGEN("Database"), iconList, SIZEOF(iconList));

	// main menu item
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_ALL | CMIF_TCHAR;
	mi.icolibItem = iconList[1].hIcolib;
	mi.ptszName = (p->m_bEncoding) ? LPGENT("Change password") : LPGENT("Set password");
	mi.ptszPopupName = LPGENT("Database");
	mi.pszService = MS_DB_CHANGEPASSWORD;
	mi.position = 500000000;
	hSetPwdMenu = Menu_AddMainMenuItem(&mi);
	return 0;
}

void CDbxMmapSA::InitDialogs()
{
	HookEventObj(ME_SYSTEM_MODULESLOADED, InitMenus, this);
	CreateServiceFunctionObj(MS_DB_CHANGEPASSWORD, ChangePassword, this);
}

INT_PTR CALLBACK DlgProcOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndList = GetDlgItem(hwndDlg, IDC_MODULES);
	CDbxMmapSA *p_Db = (CDbxMmapSA*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	LVITEM item;
	int i, iRow, iIndex;
	NMLISTVIEW * hdr = (NMLISTVIEW *)lParam;
	WORD uid;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		p_Db = (CDbxMmapSA*)lParam;
		{
			HIMAGELIST hIml = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR32, 2, 0);
			ImageList_AddIcon(hIml, LoadSkinnedIcon(SKINICON_OTHER_LOADED));
			ImageList_AddIcon(hIml, LoadSkinnedIcon(SKINICON_OTHER_NOTLOADED));
			ListView_SetImageList(hwndList, hIml, LVSIL_SMALL);

			LVCOLUMN col;
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
		}

		ListView_SetExtendedListViewStyleEx(hwndList, 0, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);

		uid = db_get_w(NULL, "SecureMMAP", "CryptoModule", 0);

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
			mir_sntprintf(buf, SIZEOF(buf), _T("%d.%d.%d.%d"), HIBYTE(HIWORD(arCryptors[i]->cryptor->Version)), LOBYTE(HIWORD(arCryptors[i]->cryptor->Version)), HIBYTE(LOWORD(arCryptors[i]->cryptor->Version)), LOBYTE(LOWORD(arCryptors[i]->cryptor->Version)));
			ListView_SetItemText(hwndList, iRow, 3, buf);

			if (uid == arCryptors[i]->cryptor->uid && p_Db->m_bEncoding)
				ListView_SetCheckState(hwndList, i, 1);

			item.mask = LVIF_IMAGE;
			item.iItem = iRow;
			item.iSubItem = 0;
			item.iImage = (CryptoEngine == arCryptors[i]->cryptor && p_Db->m_bEncoding) ? 0 : 1;
			ListView_SetItem(hwndList, &item);
		}

		ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, 2, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(hwndList, 3, LVSCW_AUTOSIZE);
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			switch (LOWORD(wParam)) {
			case IDC_EMAIL:
			case IDC_SITE:
				char buf[512];
				char *p = &buf[7];
				lstrcpyA(buf, "mailto:");
				if (GetWindowTextA(GetDlgItem(hwndDlg, LOWORD(wParam)), p, SIZEOF(buf) - 7))
					CallService(MS_UTILS_OPENURL, 0, (LPARAM)(LOWORD(wParam) == IDC_EMAIL ? buf : p));
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (hdr && hdr->hdr.code == LVN_ITEMCHANGED && IsWindowVisible(hdr->hdr.hwndFrom) && hdr->iItem != (-1)) {
			iIndex = hdr->iItem;
			if (hdr->uNewState & 0x2000) {
				for (i = 0; i < arCryptors.getCount(); i++)
					if (i != iIndex)
						ListView_SetCheckState(hwndList, i, 0);

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			if (hdr->uNewState & 0x1000) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			if (hdr->uNewState & LVIS_SELECTED) {
				SetDlgItemTextA(hwndDlg, IDC_AUTHOR, arCryptors[iIndex]->cryptor->Author);
				{
					TCHAR *info_t = mir_a2t((char*)(arCryptors[iIndex]->cryptor->Info));
					SetDlgItemText(hwndDlg, IDC_INFO, TranslateTS(info_t));
					mir_free(info_t);
				}
				SetDlgItemTextA(hwndDlg, IDC_SITE, arCryptors[iIndex]->cryptor->Site);
				SetDlgItemTextA(hwndDlg, IDC_EMAIL, arCryptors[iIndex]->cryptor->Email);
				SetDlgItemTextA(hwndDlg, IDC_ENC, arCryptors[iIndex]->cryptor->Name);
				SetDlgItemInt(hwndDlg, IDC_UID, arCryptors[iIndex]->cryptor->uid, 0);
			}
			else {
				SetDlgItemTextA(hwndDlg, IDC_AUTHOR, "");
				SetDlgItemTextA(hwndDlg, IDC_INFO, "");
				SetDlgItemTextA(hwndDlg, IDC_SITE, "");
				SetDlgItemTextA(hwndDlg, IDC_EMAIL, "");
				SetDlgItemTextA(hwndDlg, IDC_ENC, "");
				SetDlgItemTextA(hwndDlg, IDC_UID, "");
			}

			break;
		}
		if (((LPNMHDR)lParam)->code == PSN_APPLY) {
			int alg = -1;
			for (i = 0; i < arCryptors.getCount(); i++) {
				if (ListView_GetCheckState(hwndList, i)) {
					alg = i;
					break;
				}
			}

			if (alg > -1) {
				if (!p_Db->m_bEncoding) {
					db_set_w(NULL, "SecureMMAP", "CryptoModule", arCryptors[alg]->cryptor->uid);
					p_Db->EncryptDB();
				}
				else {
					if (arCryptors[alg]->cryptor->uid != db_get_w(NULL, "SecureMMAP", "CryptoModule", -1)) {
						db_set_w(NULL, "SecureMMAP", "CryptoModule", arCryptors[alg]->cryptor->uid);
						p_Db->RecryptDB();
					}
				}
			}
			else if (p_Db->m_bEncoding)
				p_Db->DecryptDB();

			uid = db_get_w(NULL, "SecureMMAP", "CryptoModule", 0);

			for (i = 0; i < arCryptors.getCount(); i++) {
				if (uid == arCryptors[i]->cryptor->uid && p_Db->m_bEncoding)
					ListView_SetCheckState(hwndList, i, 1);

				item.mask = LVIF_IMAGE;
				item.iItem = i;
				item.iSubItem = 0;
				item.iImage = (CryptoEngine == arCryptors[i]->cryptor && p_Db->m_bEncoding) ? 0 : 1;

				ListView_SetItem(hwndList, &item);
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
	char Lang[3] = { 0 };
	if (LangID != oldLangID) {
		oldLangID = LangID;
		GetLocaleInfoA(MAKELCID((LangID & 0xffffffff), SORT_DEFAULT), LOCALE_SABBREVLANGNAME, Lang, 2);
		Lang[0] = toupper(Lang[0]);
		Lang[1] = tolower(Lang[1]);
		SetDlgItemTextA(hDlg, IDC_LANG, Lang);
	}
}

extern BOOL wrongPass;

INT_PTR CALLBACK DlgStdInProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR tszHeaderTxt[256];

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		{
			HICON hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2));
			SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

			DlgStdInProcParam *param = (DlgStdInProcParam*)lParam;
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LPARAM)param->p_Db);

			if (!wrongPass) {
				mir_sntprintf(tszHeaderTxt, SIZEOF(tszHeaderTxt), _T("%s\n%s"), TranslateT("Please type in your password for"), param->pStr);
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
		}
		oldLangID = 0;
		SetTimer(hDlg, 1, 200, NULL);
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
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			break;

		case IDOK:
			CDbxMmapSA *p_Db = (CDbxMmapSA*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			p_Db->encryptKeyLength = GetDlgItemTextA(hDlg, IDC_USERPASS, p_Db->encryptKey, 254);
			EndDialog(hDlg, IDOK);
		}
		break;

	case WM_TIMER:
		LanguageChanged(hDlg);
		return FALSE;

	case WM_DESTROY:
		KillTimer(hDlg, 1);
		DestroyIcon((HICON)SendMessage(hDlg, WM_GETICON, ICON_SMALL, 0));
	}

	return FALSE;
}

INT_PTR CALLBACK DlgStdNewPass(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		SetWindowLongPtr(hDlg, GWLP_USERDATA, (LPARAM)lParam);

		SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2)));

		SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Please enter your new password"));

		oldLangID = 0;
		SetTimer(hDlg, 1, 200, NULL);
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
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			break;

		case IDOK:
			CDbxMmapSA *p_Db = (CDbxMmapSA*)GetWindowLongPtr(hDlg, GWLP_USERDATA);
			char pass1[255], pass2[255];
			if (GetDlgItemTextA(hDlg, IDC_USERPASS1, pass1, 254) < 3) {
				SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Password is too short!"));
				SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
				SetDlgItemTextA(hDlg, IDC_USERPASS1, "");
				SetDlgItemTextA(hDlg, IDC_USERPASS2, "");
			}
			else {
				GetDlgItemTextA(hDlg, IDC_USERPASS2, pass2, 254);
				if (!strcmp(pass1, pass2)) {
					p_Db->encryptKeyLength = strlen(pass1);
					strcpy(p_Db->encryptKey, pass1);
					EndDialog(hDlg, IDOK);
				}
				else {
					SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Passwords do not match!"));
					SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
					SetDlgItemTextA(hDlg, IDC_USERPASS1, "");
					SetDlgItemTextA(hDlg, IDC_USERPASS2, "");
				}
			}
		}
		break;

	case WM_TIMER:
		LanguageChanged(hDlg);
		return FALSE;

	case WM_DESTROY:
		KillTimer(hDlg, 1);
		DestroyIcon((HICON)SendMessage(hDlg, WM_GETICON, ICON_SMALL, 0));
		return FALSE;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgChangePass(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static char* newPass;
	CDbxMmapSA *p_Db = (CDbxMmapSA*)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);

		SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_SETICON, 0, (LPARAM)LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2)));
		SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Change password"));
		{
			DlgChangePassParam *param = (DlgChangePassParam*)lParam;
			newPass = param->pszNewPass;
			SetWindowLongPtr(hDlg, GWLP_USERDATA, (LPARAM)param->p_Db);
		}
		oldLangID = 0;
		SetTimer(hDlg, 1, 200, NULL);
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
		switch (LOWORD(wParam)) {
			char pass1[255], pass2[255], oldpass[255];
		case IDOK:
			GetDlgItemTextA(hDlg, IDC_OLDPASS, oldpass, 254);
			if (strcmp(oldpass, p_Db->encryptKey)) {
				SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Wrong password!"));
				SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
				break;
			}

			if (GetDlgItemTextA(hDlg, IDC_NEWPASS1, pass1, 254) < 3) {
				SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Password is too short!"));
				SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
			}
			else {
				GetDlgItemTextA(hDlg, IDC_NEWPASS2, pass2, 254);
				if (!strcmp(pass1, pass2)) {
					strcpy(newPass, pass1);
					EndDialog(hDlg, IDOK);
				}
				else {
					SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Passwords do not match!"));
					SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
				}
			}
			break;

		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			break;

		case IDREMOVE:
			GetDlgItemTextA(hDlg, IDC_OLDPASS, oldpass, 254);
			if (strcmp(oldpass, p_Db->encryptKey)) {
				SetWindowText(GetDlgItem(hDlg, IDC_HEADERBAR), TranslateT("Wrong password!"));
				SendMessage(GetDlgItem(hDlg, IDC_HEADERBAR), WM_NCPAINT, 0, 0);
				break;
			}
			EndDialog(hDlg, IDREMOVE);
		}
		break;

	case WM_TIMER:
		LanguageChanged(hDlg);
		return FALSE;

	case WM_DESTROY:
		KillTimer(hDlg, 1);
		DestroyIcon((HICON)SendMessage(hDlg, WM_GETICON, ICON_SMALL, 0));
		return FALSE;
	}
	return FALSE;
}
