#include "commonheaders.h"

static void RefreshPopupOptionsDlg(HWND hec, HWND hdc, HWND hss, HWND hsr, HWND hks, HWND hkr)
{
	// ec checkbox
	SendMessage(hec, BM_SETCHECK, g_plugin.getByte("ec", 1), 0L);

	// dc checkbox
	SendMessage(hdc, BM_SETCHECK, g_plugin.getByte("dc", 1), 0L);

	// ks checkbox
	SendMessage(hks, BM_SETCHECK, g_plugin.getByte("ks", 1), 0L);

	// kr checkbox
	SendMessage(hkr, BM_SETCHECK, g_plugin.getByte("kr", 1), 0L);

	//ss checkbox
	SendMessage(hss, BM_SETCHECK, g_plugin.getByte("ss", 0), 0L);

	//sr checkbox
	SendMessage(hsr, BM_SETCHECK, g_plugin.getByte("sr", 0), 0L);
}

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
		if (g_plugin.getString("timeoutKey", &dbv) == 0) timeout = dbv.pszVal;
		else timeout = "0";
		SetDlgItemText(hDlg, IDC_TIMEKEY, timeout);
		db_free(&dbv);

		//set timeout value for SEC
		if (g_plugin.getString("timeoutSec", &dbv) == 0) timeout = dbv.pszVal;
		else timeout = "0";
		SetDlgItemText(hDlg, IDC_TIMESEC, timeout);
		db_free(&dbv);

		//set timeout value for SR
		if (g_plugin.getString("timeoutSR", &dbv) == 0) timeout = dbv.pszVal;
		else timeout = "0";
		SetDlgItemText(hDlg, IDC_TIMESR, timeout);
		db_free(&dbv);

		//key color
		SendDlgItemMessage(hDlg, IDC_BACKKEY, CPM_SETCOLOUR, 0, g_plugin.getDword("colorKeyb", RGB(230, 230, 255)));
		SendDlgItemMessage(hDlg, IDC_TEXTKEY, CPM_SETCOLOUR, 0, g_plugin.getDword("colorKeyt", RGB(0, 0, 0)));

		//Session color
		SendDlgItemMessage(hDlg, IDC_BACKSEC, CPM_SETCOLOUR, 0, g_plugin.getDword("colorSecb", RGB(255, 255, 200)));
		SendDlgItemMessage(hDlg, IDC_TEXTSEC, CPM_SETCOLOUR, 0, g_plugin.getDword("colorSect", RGB(0, 0, 0)));

		//SR color
		SendDlgItemMessage(hDlg, IDC_BACKSR, CPM_SETCOLOUR, 0, g_plugin.getDword("colorSRb", RGB(200, 255, 200)));
		SendDlgItemMessage(hDlg, IDC_TEXTSR, CPM_SETCOLOUR, 0, g_plugin.getDword("colorSRt", RGB(0, 0, 0)));
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == CPN_COLOURCHANGED) {
			//It's a colour picker change. LOWORD(wParam) is the control id.
			uint32_t color = SendDlgItemMessage(hDlg, LOWORD(wParam), CPM_GETCOLOUR, 0, 0);

			switch (LOWORD(wParam)) {
			case IDC_BACKKEY:
				g_plugin.setDword("colorKeyb", color);
				break;
			case IDC_TEXTKEY:
				g_plugin.setDword("colorKeyt", color);
				break;
			case IDC_BACKSEC:
				g_plugin.setDword("colorSecb", color);
				break;
			case IDC_TEXTSEC:
				g_plugin.setDword("colorSect", color);
				break;
			case IDC_BACKSR:
				g_plugin.setDword("colorSRb", color);
				break;
			case IDC_TEXTSR:
				g_plugin.setDword("colorSRt", color);
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
			g_plugin.setByte("ec", (uint8_t)(SendMessage(hec, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_DC:
			//set dc checkbox value
			g_plugin.setByte("dc", (uint8_t)(SendMessage(hdc, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_SS:
			//set ss checkbox value
			g_plugin.setByte("ss", (uint8_t)(SendMessage(hss, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_SR:
			//set sr checkbox value
			g_plugin.setByte("sr", (uint8_t)(SendMessage(hsr, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_KS:
			//set indicator checkbox value
			g_plugin.setByte("ks", (uint8_t)(SendMessage(hks, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_KR:
			//set indicator checkbox value
			g_plugin.setByte("kr", (uint8_t)(SendMessage(hkr, BM_GETCHECK, 0L, 0L) == BST_CHECKED));
			break;
		case IDC_TIMEKEY:
			//set timeout value
			GetDlgItemText(hDlg, IDC_TIMEKEY, getTimeout, _countof(getTimeout));
			mir_itoa(atoi(getTimeout), getTimeout, 10);
			g_plugin.setString("timeoutKey", getTimeout);
			break;
		case IDC_TIMESEC:
			//set timeout value
			GetDlgItemText(hDlg, IDC_TIMESEC, getTimeout, _countof(getTimeout));
			mir_itoa(atoi(getTimeout), getTimeout, 10);
			g_plugin.setString("timeoutSec", getTimeout);
			break;
		case IDC_TIMESR:
			//set timeout value
			GetDlgItemText(hDlg, IDC_TIMESR, getTimeout, _countof(getTimeout));
			mir_itoa(atoi(getTimeout), getTimeout, 10);
			g_plugin.setString("timeoutSR", getTimeout);
			break;
		}

		RefreshPopupOptionsDlg(hec, hdc, hss, hsr, hks, hkr);
		break;

	default:
		return FALSE;
	}
	return TRUE;
}
