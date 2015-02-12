#include "Mra.h"
#include "MraPopup.h"

#define POPUPS_TYPES_COUNT	7
static LPCWSTR lpcwszPopupsTypes[] =
{
	LPGENT("None"),
	LPGENT("Debug"),
	LPGENT("Information"),
	LPGENT("Question"),
	LPGENT("Warning"),
	LPGENT("Error"),
	LPGENT("NewMail"),
	NULL
};

INT_PTR CALLBACK MraPopupDlgProcOpts(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CMraProto *ppro = (CMraProto*)GetWindowLongPtr(hWndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hWndDlg);
		SetWindowLongPtr(hWndDlg, GWLP_USERDATA, lParam);
		ppro = (CMraProto*)lParam;
		{
			HWND hWndCombo = GetDlgItem(hWndDlg, IDC_COMBO_POPUP_TYPE);
			SendMessage(hWndCombo, CB_RESETCONTENT, 0, 0);

			for (size_t i = 0; i < POPUPS_TYPES_COUNT; i++) {
				DWORD dwItem = SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)TranslateTS(lpcwszPopupsTypes[i]));
				SendMessage(hWndCombo, CB_SETITEMDATA, dwItem, i);
			}
			SendMessage(hWndCombo, CB_SETCURSEL, 0, 0);
			SendMessage(hWndDlg, WM_COMMAND, MAKELONG(IDC_COMBO_POPUP_TYPE, CBN_SELCHANGE), 0);
		}
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_COMBO_POPUP_TYPE:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				BOOL bEnabled, bUseWinColors;
				char szBuff[MAX_PATH];
				DWORD dwType = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_COMBO_POPUP_TYPE);

				bEnabled = GetBit(ppro->getDword("PopupsEventFilter", MRA_DEFAULT_POPUPS_EVENT_FILTER), dwType);
				CheckDlgButton(hWndDlg, IDC_CHK_ENABLE, bEnabled ? BST_CHECKED : BST_UNCHECKED);

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SUseWinColors", lpcwszPopupsTypes[dwType]);
				bUseWinColors = ppro->getByte(szBuff, MRA_DEFAULT_POPUP_USE_WIN_COLORS);
				CheckDlgButton(hWndDlg, IDC_CHK_USE_WIN_COLORS, bUseWinColors ? BST_CHECKED : BST_UNCHECKED);
				EnableWindow(GetDlgItem(hWndDlg, IDC_CHK_USE_WIN_COLORS), bEnabled);

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorBack", lpcwszPopupsTypes[dwType]);
				SendDlgItemMessage(hWndDlg, IDC_POPUP_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getDword(szBuff, MRA_DEFAULT_POPUP_COLOR_BACK));
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_BACKCOLOR), (bEnabled && bUseWinColors == FALSE));

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorText", lpcwszPopupsTypes[dwType]);
				SendDlgItemMessage(hWndDlg, IDC_POPUP_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getDword(szBuff, MRA_DEFAULT_POPUP_COLOR_TEXT));
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_TEXTCOLOR), (bEnabled && bUseWinColors == FALSE));

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%STimeout", lpcwszPopupsTypes[dwType]);
				SetDlgItemInt(hWndDlg, IDC_POPUP_TIMEOUT, ppro->getDword(szBuff, MRA_DEFAULT_POPUP_TIMEOUT), FALSE);
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_TIMEOUT), bEnabled);
			}
			break;

		case IDC_PREVIEW:
			{
				for (size_t i = 0; i < POPUPS_TYPES_COUNT; i++) {
					ppro->MraPopupShowFromAgentW(i, 0, TranslateTS(lpcwszPopupsTypes[i]));
				}
			}
			break;

		case IDC_CHK_ENABLE:
		case IDC_CHK_USE_WIN_COLORS:
			{
				BOOL bEnabled = IsDlgButtonChecked(hWndDlg, IDC_CHK_ENABLE);
				BOOL bUseWinColors = IsDlgButtonChecked(hWndDlg, IDC_CHK_USE_WIN_COLORS);

				EnableWindow(GetDlgItem(hWndDlg, IDC_CHK_USE_WIN_COLORS), bEnabled);
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_BACKCOLOR), (bEnabled && bUseWinColors == FALSE));
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_TEXTCOLOR), (bEnabled && bUseWinColors == FALSE));
				EnableWindow(GetDlgItem(hWndDlg, IDC_POPUP_TIMEOUT), bEnabled);
			}
		default:
			if ((LOWORD(wParam) == IDC_POPUP_TIMEOUT) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
				return FALSE;
			SendMessage(GetParent(hWndDlg), PSM_CHANGED, 0, 0);
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			{
				char szBuff[MAX_PATH];
				DWORD dwType = GET_CURRENT_COMBO_DATA(hWndDlg, IDC_COMBO_POPUP_TYPE);
				DWORD dwPopupsEventFilter = ppro->getDword("PopupsEventFilter", MRA_DEFAULT_POPUPS_EVENT_FILTER);
				if (IsDlgButtonChecked(hWndDlg, IDC_CHK_ENABLE))
					dwPopupsEventFilter |= (1<<dwType);
				else
					dwPopupsEventFilter &= ~(1<<dwType);

				ppro->setDword("PopupsEventFilter", dwPopupsEventFilter);

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SUseWinColors", lpcwszPopupsTypes[dwType]);
				ppro->setByte(szBuff, IsDlgButtonChecked(hWndDlg, IDC_CHK_USE_WIN_COLORS));

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorBack", lpcwszPopupsTypes[dwType]);
				ppro->setDword(szBuff, SendDlgItemMessage(hWndDlg, IDC_POPUP_BACKCOLOR, CPM_GETCOLOUR, 0, 0));

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorText", lpcwszPopupsTypes[dwType]);
				ppro->setDword(szBuff, SendDlgItemMessage(hWndDlg, IDC_POPUP_TEXTCOLOR, CPM_GETCOLOUR, 0, 0));

				mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%STimeout", lpcwszPopupsTypes[dwType]);
				ppro->setDword(szBuff, GetDlgItemInt(hWndDlg, IDC_POPUP_TIMEOUT, NULL, FALSE));
			}
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int CMraProto::OnPopupOptInit(WPARAM wParam, LPARAM)
{
	if ( ServiceExists(MS_POPUP_ADDPOPUPT)) {
		OPTIONSDIALOGPAGE odp = { sizeof(odp) };
		odp.dwInitParam = (LPARAM)this;
		odp.position = 100000000;
		odp.hInstance = g_hInstance;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUPS);
		odp.ptszTitle = m_tszUserName;
		odp.ptszGroup = LPGENT("Popups");
		odp.groupPosition = 900000000;
		odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
		odp.pfnDlgProc = MraPopupDlgProcOpts;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// ShowPopup - popup plugin support

struct MraPopupData
{
	CMraProto *ppro;
	int iPopupType;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Popup plugin window proc

LRESULT CALLBACK MraPopupDlgProc(HWND hWndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MraPopupData* dat = (MraPopupData*)PUGetPluginData(hWndDlg);

	switch (msg) {
	case UM_INITPOPUP:
		if (dat->iPopupType == MRA_POPUP_TYPE_EMAIL_STATUS) {
			// update/load avatar
			dat->ppro->MraAvatarsQueueGetAvatarSimple(dat->ppro->hAvatarsQueueHandle, GAIF_FORCE, PUGetContact(hWndDlg));

			// delete old email popup
			if (dat->ppro->hWndEMailPopupStatus)
				PUDeletePopup(dat->ppro->hWndEMailPopupStatus);

			dat->ppro->hWndEMailPopupStatus = hWndDlg;
		}
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) { //It was a click on the Popup.
			if (dat->iPopupType == MRA_POPUP_TYPE_EMAIL_STATUS) {
				CallProtoService(dat->ppro->m_szModuleName, MRA_GOTO_INBOX, 0, 0);
				dat->ppro->hWndEMailPopupStatus = NULL;
			}
		}
		PUDeletePopup(hWndDlg);
		break;

	case WM_CONTEXTMENU:
		if (dat->iPopupType == MRA_POPUP_TYPE_EMAIL_STATUS)
			dat->ppro->hWndEMailPopupStatus = NULL;
		PUDeletePopup(hWndDlg);
		break;

	case UM_FREEPLUGINDATA:
		if (dat->iPopupType == MRA_POPUP_TYPE_EMAIL_STATUS)
			dat->ppro->hWndEMailPopupStatus = NULL;
		mir_free(dat);
		break;
	}
	return DefWindowProc(hWndDlg, msg, wParam, lParam);
}

void CMraProto::MraPopupShowFromContactW(MCONTACT hContact, DWORD dwType, DWORD dwFlags, LPCWSTR lpszMessage)
{
	WCHAR szTitle[MAX_CONTACTNAME];
	CMStringW szNick, szEmail;
	mraGetStringW(hContact, "Nick", szNick);
	mraGetStringW(hContact, "e-mail", szEmail);
	if (hContact)
		mir_snwprintf(szTitle, SIZEOF(szTitle), L"%s <%s>", szNick, szEmail);
	else
		mir_snwprintf(szTitle, SIZEOF(szTitle), L"%s:  %s <%s>", m_tszUserName, szNick, szEmail);

	MraPopupShowW(hContact, dwType, dwFlags, szTitle, lpszMessage);
}

void CMraProto::MraPopupShowW(MCONTACT hContact, DWORD dwType, DWORD dwFlags, LPWSTR lpszTitle, LPCWSTR lpszMessage)
{
	if (getByte("PopupsEnabled", MRA_DEFAULT_POPUPS_ENABLED))
	if (GetBit(getDword("PopupsEventFilter", MRA_DEFAULT_POPUPS_EVENT_FILTER), dwType))
	if ( ServiceExists(MS_POPUP_ADDPOPUPW)) {
		POPUPDATAT ppd = { 0 };

		// delete old email popup
		if (dwType == MRA_POPUP_TYPE_EMAIL_STATUS && hWndEMailPopupStatus) {
			PUDeletePopup(hWndEMailPopupStatus);
			hWndEMailPopupStatus = NULL;
		}

		// load icon
		switch (dwType) {
		case MRA_POPUP_TYPE_NONE:// proto icon
			ppd.lchIcon = g_hMainIcon;
			break;
		case MRA_POPUP_TYPE_DEBUG:// IDI_APPLICATION
			ppd.lchIcon = (HICON)LoadImage(NULL, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_INFORMATION:// IDI_INFORMATION
			ppd.lchIcon = (HICON)LoadImage(NULL, IDI_INFORMATION, IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_QUESTION:// IDI_QUESTION
			ppd.lchIcon = (HICON)LoadImage(NULL, IDI_QUESTION, IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_WARNING:// IDI_WARNING
			ppd.lchIcon = (HICON)LoadImage(NULL, IDI_WARNING, IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_ERROR:// IDI_ERROR
			ppd.lchIcon = (HICON)LoadImage(NULL, IDI_ERROR, IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		case MRA_POPUP_TYPE_EMAIL_STATUS:
			ppd.lchIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_MAIL_NOTIFY), IMAGE_ICON, 0, 0, LR_SHARED);
			break;
		}

		MraPopupData *dat = (MraPopupData*)mir_calloc( sizeof(MraPopupData));
		dat->iPopupType = dwType;
		dat->ppro = this;

		ppd.lchContact = hContact;
		if (lpszTitle)
			mir_tstrncpy(ppd.lptzContactName, lpszTitle, MAX_CONTACTNAME);
		if (lpszMessage)
			mir_tstrncpy(ppd.lptzText, lpszMessage, MAX_SECONDLINE);
		ppd.PluginWindowProc = MraPopupDlgProc;
		ppd.PluginData = dat;

		char szBuff[MAX_PATH];
		mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SUseWinColors", lpcwszPopupsTypes[dwType]);
		BOOL bUseWinColors = getByte(szBuff, MRA_DEFAULT_POPUP_USE_WIN_COLORS);
		if (bUseWinColors) {
			ppd.colorBack = GetSysColor(COLOR_BTNFACE);
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		}
		else {
			mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorBack", lpcwszPopupsTypes[dwType]);
			ppd.colorBack = getDword(szBuff, MRA_DEFAULT_POPUP_COLOR_BACK);

			mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%SColorText", lpcwszPopupsTypes[dwType]);
			ppd.colorText = getDword(szBuff, MRA_DEFAULT_POPUP_COLOR_TEXT);
		}

		mir_snprintf(szBuff, SIZEOF(szBuff), "PopupType%STimeout", lpcwszPopupsTypes[dwType]);
		ppd.iSeconds = getDword(szBuff, MRA_DEFAULT_POPUP_TIMEOUT);

		if (dat->iPopupType == MRA_POPUP_TYPE_EMAIL_STATUS && dat->ppro->hWndEMailPopupStatus)
			PUChangeTextW(dat->ppro->hWndEMailPopupStatus, ppd.lpwzText);
		else
			PUAddPopupW(&ppd);
	}
	else if (dwFlags & MRA_POPUP_ALLOW_MSGBOX)
		MessageBox(NULL, lpszMessage, lpszTitle, MB_OK+(dwType == MRA_POPUP_TYPE_WARNING)?MB_ICONERROR:MB_ICONINFORMATION);
}
