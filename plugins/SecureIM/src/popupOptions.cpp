#include "commonheaders.h"

INT_PTR CALLBACK PopOptionsDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	char getTimeout[5];

	HWND hec = GetDlgItem(hDlg, IDC_EC);
	HWND hdc = GetDlgItem(hDlg, IDC_DC);
	HWND hks = GetDlgItem(hDlg, IDC_KS);
	HWND hkr = GetDlgItem(hDlg, IDC_KR);
	HWND hss = GetDlgItem(hDlg, IDC_SS);
	HWND hsr = GetDlgItem(hDlg, IDC_SR);

	switch (wMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		RefreshPopupOptionsDlg(hec, hdc, hss, hsr, hks, hkr);

		DBVARIANT dbv;
		char *timeout;

		//set timeout value for Key
		if (db_get(0, MODULENAME, "timeoutKey", &dbv) == 0) timeout = dbv.pszVal;
		else timeout = "0";
		SetDlgItemText(hDlg, IDC_TIMEKEY, timeout);
		db_free(&dbv);

		//set timeout value for SEC
		if (db_get(0, MODULENAME, "timeoutSec", &dbv) == 0) timeout = dbv.pszVal;
		else timeout = "0";
		SetDlgItemText(hDlg, IDC_TIMESEC, timeout);
		db_free(&dbv);

		//set timeout value for SR
		if (db_get(0, MODULENAME, "timeoutSR", &dbv) == 0) timeout = dbv.pszVal;
		else timeout = "0";
		SetDlgItemText(hDlg, IDC_TIMESR, timeout);
		db_free(&dbv);

		//key color
		SendDlgItemMessage(hDlg, IDC_BACKKEY, CPM_SETCOLOUR, 0, db_get_dw(0, MODULENAME, "colorKeyb", RGB(230, 230, 255)));
		SendDlgItemMessage(hDlg, IDC_TEXTKEY, CPM_SETCOLOUR, 0, db_get_dw(0, MODULENAME, "colorKeyt", RGB(0, 0, 0)));

		//Session color
		SendDlgItemMessage(hDlg, IDC_BACKSEC, CPM_SETCOLOUR, 0, db_get_dw(0, MODULENAME, "colorSecb", RGB(255, 255, 200)));
		SendDlgItemMessage(hDlg, IDC_TEXTSEC, CPM_SETCOLOUR, 0, db_get_dw(0, MODULENAME, "colorSect", RGB(0, 0, 0)));

		//SR color
		SendDlgItemMessage(hDlg, IDC_BACKSR, CPM_SETCOLOUR, 0, db_get_dw(0, MODULENAME, "colorSRb", RGB(200, 255, 200)));
		SendDlgItemMessage(hDlg, IDC_TEXTSR, CPM_SETCOLOUR, 0, db_get_dw(0, MODULENAME, "colorSRt", RGB(0, 0, 0)));
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == CPN_COLOURCHANGED) {
			//It's a colour picker change. LOWORD(wParam) is the control id.
			DWORD color = SendDlgItemMessage(hDlg, LOWORD(wParam), CPM_GETCOLOUR, 0, 0);

			switch (LOWORD(wParam)) {
			case IDC_BACKKEY:
				db_set_dw(0, MODULENAME, "colorKeyb", color);
				break;
			case IDC_TEXTKEY:
				db_set_dw(0, MODULENAME, "colorKeyt", color);
				break;
			case IDC_BACKSEC:
				db_set_dw(0, MODULENAME, "colorSecb", color);
				break;
			case IDC_TEXTSEC:
				db_set_dw(0, MODULENAME, "colorSect", color);
				break;
			case IDC_BACKSR:
				db_set_dw(0, MODULENAME, "colorSRb", color);
				break;
			case IDC_TEXTSR:
				db_set_dw(0, MODULENAME, "colorSRt", color);
				break;
			}
			return TRUE;
		}

		switch (LOWORD(wParam)) {
		case IDC_PREV:
			//preview popups...
			showPopup(LPGEN("Key Popup"), NULL, g_hPOP[POP_PU_PRC], 0);
			showPopup(LPGEN("Secure Popup"), NULL, g_hPOP[POP_PU_EST], 1);
			showPopup(LPGEN("Message Popup"), NULL, g_hPOP[POP_PU_MSR], 2);
			break;
		case IDC_EC:
			//set ec checkbox value
			db_set_b(0, MODULENAME, "ec", (BYTE)(SendMessage(hec, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_DC:
			//set dc checkbox value
			db_set_b(0, MODULENAME, "dc", (BYTE)(SendMessage(hdc, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_SS:
			//set ss checkbox value
			db_set_b(0, MODULENAME, "ss", (BYTE)(SendMessage(hss, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_SR:
			//set sr checkbox value
			db_set_b(0, MODULENAME, "sr", (BYTE)(SendMessage(hsr, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_KS:
			//set indicator checkbox value
			db_set_b(0, MODULENAME, "ks", (BYTE)(SendMessage(hks, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_KR:
			//set indicator checkbox value
			db_set_b(0, MODULENAME, "kr", (BYTE)(SendMessage(hkr, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_TIMEKEY:
			//set timeout value
			GetDlgItemText(hDlg, IDC_TIMEKEY, getTimeout, sizeof(getTimeout));
			mir_itoa(atoi(getTimeout), getTimeout, 10);
			db_set_s(0, MODULENAME, "timeoutKey", getTimeout);
			break;
		case IDC_TIMESEC:
			//set timeout value
			GetDlgItemText(hDlg, IDC_TIMESEC, getTimeout, sizeof(getTimeout));
			mir_itoa(atoi(getTimeout), getTimeout, 10);
			db_set_s(0, MODULENAME, "timeoutSec", getTimeout);
			break;
		case IDC_TIMESR:
			//set timeout value
			GetDlgItemText(hDlg, IDC_TIMESR, getTimeout, sizeof(getTimeout));
			mir_itoa(atoi(getTimeout), getTimeout, 10);
			db_set_s(0, MODULENAME, "timeoutSR", getTimeout);
			break;
		}

		RefreshPopupOptionsDlg(hec, hdc, hss, hsr, hks, hkr);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

void RefreshPopupOptionsDlg(HWND hec, HWND hdc, HWND hss, HWND hsr, HWND hks, HWND hkr)
{
	// ec checkbox
	SendMessage(hec, BM_SETCHECK, db_get_b(0, MODULENAME, "ec", 1), 0L);

	// dc checkbox
	SendMessage(hdc, BM_SETCHECK, db_get_b(0, MODULENAME, "dc", 1), 0L);

	// ks checkbox
	SendMessage(hks, BM_SETCHECK, db_get_b(0, MODULENAME, "ks", 1), 0L);

	// kr checkbox
	SendMessage(hkr, BM_SETCHECK, db_get_b(0, MODULENAME, "kr", 1), 0L);

	//ss checkbox
	SendMessage(hss, BM_SETCHECK, db_get_b(0, MODULENAME, "ss", 0), 0L);

	//sr checkbox
	SendMessage(hsr, BM_SETCHECK, db_get_b(0, MODULENAME, "sr", 0), 0L);
}

int onRegisterPopOptions(WPARAM wParam, LPARAM)
{
	if (bPopupExists) {
		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.hInstance = g_hInst;
		odp.pszTemplate = MAKEINTRESOURCE(IDD_POPUP);
		odp.pszTitle = (char*)MODULENAME;
		odp.pszGroup = LPGEN("Popups");
		odp.pfnDlgProc = PopOptionsDlgProc;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}
