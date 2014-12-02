#include "common.h"

extern BOOL _saveDlgItemInt(HWND hDialog, int controlID, char* option);

void EnablePopupControls(HWND hwnd, BOOL enable)
{
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_NOTIFY_BLOCKED), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_NOTIFY_APPROVED), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_NOTIFY_CHALLENGE), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_BLOCKED_FOREGROUND), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_BLOCKED_BACKGROUND), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_APPROVED_FOREGROUND), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_APPROVED_BACKGROUND), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_CHALLENGE_FOREGROUND), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_CHALLENGE_BACKGROUND), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_BLOCKED_TIMEOUT), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_APPROVED_TIMEOUT), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_CHALLENGE_TIMEOUT), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_DEFAULT_COLORS), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_WINDOWS_COLORS), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_DEFAULT_TIMEOUT), enable);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_PREVIEW), enable);
}

void EnablePopupColors(HWND hwnd, BOOL enableDefault, BOOL enableWindows)
{
	BOOL enable, bEnabled;
	bEnabled = SendDlgItemMessage(hwnd, IDC_OPT_POPUPS_ENABLED, BM_GETCHECK, 0, 0);
	enable = enableDefault || enableWindows;
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_BLOCKED_FOREGROUND), !enable && bEnabled);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_BLOCKED_BACKGROUND), !enable && bEnabled);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_APPROVED_FOREGROUND), !enable && bEnabled);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_APPROVED_BACKGROUND), !enable && bEnabled);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_CHALLENGE_FOREGROUND), !enable && bEnabled);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_CHALLENGE_BACKGROUND), !enable && bEnabled);

	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_WINDOWS_COLORS), !enableDefault && bEnabled);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_DEFAULT_COLORS), !enableWindows && bEnabled);
}

void EnablePopupTimeouts(HWND hwnd, BOOL enable)
{
	BOOL bEnabled = SendDlgItemMessage(hwnd, IDC_OPT_POPUPS_ENABLED, BM_GETCHECK, 0, 0);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_BLOCKED_TIMEOUT), !enable && bEnabled);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_APPROVED_TIMEOUT), !enable && bEnabled);
	EnableWindow(GetDlgItem(hwnd, IDC_OPT_POPUPS_CHALLENGE_TIMEOUT), !enable && bEnabled);
}

INT_PTR CALLBACK DlgProcOptionsPopups(HWND optDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL bEnabled;
	static int bInitializing = 0;
	switch (msg) {
		case WM_INITDIALOG:
			bInitializing = 1;
			TranslateDialogDefault(optDlg);
			bEnabled = _getOptB("NotifyPopup", defaultNotifyPopup);
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_ENABLED, BM_SETCHECK, bEnabled, 0);
			EnablePopupControls(optDlg, bEnabled);

			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_NOTIFY_BLOCKED, BM_SETCHECK, _getOptB("NotifyPopupBlocked", defaultNotifyPopupBlocked), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_NOTIFY_APPROVED, BM_SETCHECK, _getOptB("NotifyPopupApproved", defaultNotifyPopupApproved), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_NOTIFY_CHALLENGE, BM_SETCHECK, _getOptB("NotifyPopupChallenge", defaultNotifyPopupChallenge), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_DEFAULT_COLORS, BM_SETCHECK, _getOptB("PopupDefaultColors", defaultPopupDefaultColors), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_WINDOWS_COLORS, BM_SETCHECK, _getOptB("PopupWindowsColors", defaultPopupWindowsColors), 0);
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_DEFAULT_TIMEOUT, BM_SETCHECK, _getOptB("PopupDefaultTimeout", defaultPopupDefaultTimeout), 0);
			SetDlgItemInt(optDlg, IDC_OPT_POPUPS_BLOCKED_TIMEOUT, _getOptD("PopupBlockedTimeout", defaultPopupBlockedTimeout), FALSE);
			SetDlgItemInt(optDlg, IDC_OPT_POPUPS_APPROVED_TIMEOUT, _getOptD("PopupApprovedTimeout", defaultPopupApprovedTimeout), FALSE);
			SetDlgItemInt(optDlg, IDC_OPT_POPUPS_CHALLENGE_TIMEOUT, _getOptD("PopupChallengeTimeout", defaultPopupChallengeTimeout), FALSE);
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_BLOCKED_FOREGROUND, CPM_SETCOLOUR, 0, _getOptD("PopupBlockedForeground", defaultPopupBlockedForeground));
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_BLOCKED_BACKGROUND, CPM_SETCOLOUR, 0, _getOptD("PopupBlockedBackground", defaultPopupBlockedBackground));
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_APPROVED_FOREGROUND, CPM_SETCOLOUR, 0, _getOptD("PopupApprovedForeground", defaultPopupApprovedForeground));
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_APPROVED_BACKGROUND, CPM_SETCOLOUR, 0, _getOptD("PopupApprovedBackground", defaultPopupApprovedBackground));
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_CHALLENGE_FOREGROUND, CPM_SETCOLOUR, 0, _getOptD("PopupChallengeForeground", defaultPopupChallengeForeground));
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_CHALLENGE_BACKGROUND, CPM_SETCOLOUR, 0, _getOptD("PopupChallengeBackground", defaultPopupChallengeBackground));
			EnablePopupTimeouts(optDlg, SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_DEFAULT_TIMEOUT, BM_GETCHECK, 0, 0));
			EnablePopupColors(optDlg,
				_getOptB("PopupDefaultColors", defaultPopupDefaultColors),
				_getOptB("PopupWindowsColors", defaultPopupWindowsColors));

			bInitializing = 0;
			break;
		case WM_COMMAND:
			if (bInitializing)
				return FALSE;
			switch (LOWORD(wParam)) {
				case IDC_OPT_POPUPS_ENABLED:
					bEnabled = SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_ENABLED, BM_GETCHECK, 0, 0);
					EnablePopupControls(optDlg, bEnabled);
				case IDC_OPT_POPUPS_NOTIFY_BLOCKED:
				case IDC_OPT_POPUPS_NOTIFY_APPROVED:
				case IDC_OPT_POPUPS_NOTIFY_CHALLENGE:
				case IDC_OPT_POPUPS_DEFAULT_COLORS:
				case IDC_OPT_POPUPS_WINDOWS_COLORS:
				case IDC_OPT_POPUPS_DEFAULT_TIMEOUT:
					EnablePopupColors(optDlg,
						SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_DEFAULT_COLORS, BM_GETCHECK, 0, 0),
						SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_WINDOWS_COLORS, BM_GETCHECK, 0, 0));
					EnablePopupTimeouts(optDlg, SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_DEFAULT_TIMEOUT, BM_GETCHECK, 0, 0));

					if (HIWORD(wParam) != BN_CLICKED)
						return FALSE;
					break;
				case IDC_OPT_POPUPS_BLOCKED_TIMEOUT:
				case IDC_OPT_POPUPS_APPROVED_TIMEOUT:
				case IDC_OPT_POPUPS_CHALLENGE_TIMEOUT:
					if (HIWORD(wParam) != EN_CHANGE)
						return FALSE;
					break;
				case IDC_OPT_POPUPS_PREVIEW:
					ShowPopupPreview(optDlg, POPUP_BLOCKED, NULL, TranslateT("Message blocked due to preview action"));
					ShowPopupPreview(optDlg, POPUP_APPROVED, NULL, TranslateT("Contact approved due to preview action"));
					ShowPopupPreview(optDlg, POPUP_CHALLENGE, NULL, TranslateT("Challenge sent to preview contact"));
					return FALSE;
			}
			SendMessage(GetParent(optDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch (((NMHDR*)lParam)->code) {
				case PSN_APPLY:
					_setOptB("NotifyPopup", SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_ENABLED, BM_GETCHECK, 0, 0));
					_setOptB("NotifyPopupBlocked", SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_NOTIFY_BLOCKED, BM_GETCHECK, 0, 0));
					_setOptB("NotifyPopupApproved", SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_NOTIFY_APPROVED, BM_GETCHECK, 0, 0));
					_setOptB("NotifyPopupChallenge", SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_NOTIFY_CHALLENGE, BM_GETCHECK, 0, 0));
					_setOptB("PopupDefaultColors", SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_DEFAULT_COLORS, BM_GETCHECK, 0, 0));
					_setOptB("PopupWindowsColors", SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_WINDOWS_COLORS, BM_GETCHECK, 0, 0));
					_setOptB("PopupDefaultTimeout", SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_DEFAULT_TIMEOUT, BM_GETCHECK, 0, 0));
					_saveDlgItemInt(optDlg, IDC_OPT_POPUPS_BLOCKED_TIMEOUT, "PopupBlockedTimeout");
					_saveDlgItemInt(optDlg, IDC_OPT_POPUPS_APPROVED_TIMEOUT, "PopupApprovedTimeout");
					_saveDlgItemInt(optDlg, IDC_OPT_POPUPS_CHALLENGE_TIMEOUT, "PopupChallengeTimeout");
					_setOptD("PopupBlockedForeground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_BLOCKED_FOREGROUND,CPM_GETCOLOUR,0,0));
					_setOptD("PopupBlockedBackground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_BLOCKED_BACKGROUND,CPM_GETCOLOUR,0,0));
					_setOptD("PopupApprovedForeground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_FOREGROUND,CPM_GETCOLOUR,0,0));
					_setOptD("PopupApprovedBackground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_BACKGROUND,CPM_GETCOLOUR,0,0));
					_setOptD("PopupChallengeForeground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_CHALLENGE_FOREGROUND,CPM_GETCOLOUR,0,0));
					_setOptD("PopupChallengeBackground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_CHALLENGE_BACKGROUND,CPM_GETCOLOUR,0,0));
					break;
			}
			break;
		case WM_DESTROY:
			break;
	}
	return FALSE;
}

int ShowPopupPreview(HWND optDlg, BYTE popupType, TCHAR *line1, TCHAR *line2)
{
	POPUPDATAT ppdp = {0};
	switch (popupType)
	{
		case POPUP_DEFAULT:
			ppdp.colorText = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_FOREGROUND,CPM_GETCOLOUR,0,0);
			ppdp.colorBack = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_BACKGROUND,CPM_GETCOLOUR,0,0);
			break;
		case POPUP_BLOCKED:
			ppdp.colorText = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_BLOCKED_FOREGROUND,CPM_GETCOLOUR,0,0);
			ppdp.colorBack = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_BLOCKED_BACKGROUND,CPM_GETCOLOUR,0,0);
			ppdp.iSeconds = GetDlgItemInt(optDlg, IDC_OPT_POPUPS_BLOCKED_TIMEOUT, NULL, TRUE);
			ppdp.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_DELETE);
			break;
		case POPUP_APPROVED:
			ppdp.colorText = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_FOREGROUND,CPM_GETCOLOUR,0,0);
			ppdp.colorBack = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_BACKGROUND,CPM_GETCOLOUR,0,0);
			ppdp.iSeconds = GetDlgItemInt(optDlg, IDC_OPT_POPUPS_APPROVED_TIMEOUT, NULL, TRUE);
			ppdp.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_ADDCONTACT);
			break;
		case POPUP_CHALLENGE:
			ppdp.colorText = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_CHALLENGE_FOREGROUND,CPM_GETCOLOUR,0,0);
			ppdp.colorBack = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_CHALLENGE_BACKGROUND,CPM_GETCOLOUR,0,0);
			ppdp.iSeconds = GetDlgItemInt(optDlg, IDC_OPT_POPUPS_CHALLENGE_TIMEOUT, NULL, TRUE);
			ppdp.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
			break;
	}
	if (SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_WINDOWS_COLORS, BM_GETCHECK, 0, 0)) {
		ppdp.colorText = GetSysColor(COLOR_WINDOWTEXT);
		ppdp.colorBack = GetSysColor(COLOR_WINDOW);
	}
	if (SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_DEFAULT_COLORS, BM_GETCHECK, 0, 0)) {
		ppdp.colorText = NULL;
		ppdp.colorBack = NULL;
	}
	if (ppdp.iSeconds < 1)
		ppdp.iSeconds = -1;
	if (SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_DEFAULT_TIMEOUT, BM_GETCHECK, 0, 0) || popupType == POPUP_DEFAULT)
		ppdp.iSeconds = 0;

	ppdp.lchContact = NULL;
	_tcsncpy_s(ppdp.lptzContactName, (line1 ? line1 : _T(PLUGIN_NAME)), _TRUNCATE);
	if (line2)
		_tcsncpy_s(ppdp.lptzText, line2, _TRUNCATE);
	return PUAddPopupT(&ppdp);

}

int ShowPopup(MCONTACT hContact, BYTE popupType, TCHAR *line1, TCHAR *line2)
{
	POPUPDATAT ppdp = {0};
	switch (popupType)
	{
		case POPUP_DEFAULT:
			ppdp.colorText = _getOptD("PopupApprovedForeground", defaultPopupApprovedForeground);
			ppdp.colorBack = _getOptD("PopupApprovedBackground", defaultPopupApprovedBackground);
			break;
		case POPUP_BLOCKED:
			ppdp.colorText = _getOptD("PopupBlockedForeground", defaultPopupBlockedForeground);
			ppdp.colorBack = _getOptD("PopupBlockedBackground", defaultPopupBlockedBackground);
			ppdp.iSeconds = _getOptD("PopupBlockedTimeout", defaultPopupBlockedTimeout);
			ppdp.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_DELETE);
			break;
		case POPUP_APPROVED:
			ppdp.colorText = _getOptD("PopupApprovedForeground", defaultPopupApprovedForeground);
			ppdp.colorBack = _getOptD("PopupApprovedBackground", defaultPopupApprovedBackground);
			ppdp.iSeconds = _getOptD("PopupApprovedTimeout", defaultPopupApprovedTimeout);
			ppdp.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_ADDCONTACT);
			break;
		case POPUP_CHALLENGE:
			ppdp.colorText = _getOptD("PopupChallengeForeground", defaultPopupChallengeForeground);
			ppdp.colorBack = _getOptD("PopupChallengeBackground", defaultPopupChallengeBackground);
			ppdp.iSeconds = _getOptD("PopupChallengeTimeout", defaultPopupChallengeTimeout);
			ppdp.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
			break;
	}
	if (_getOptB("PopupWindowsColors", defaultPopupWindowsColors)) {
		ppdp.colorText = GetSysColor(COLOR_WINDOWTEXT);
		ppdp.colorBack = GetSysColor(COLOR_WINDOW);
	}
	if (_getOptB("PopupDefaultColors", defaultPopupDefaultColors)) {
		ppdp.colorText = NULL;
		ppdp.colorBack = NULL;
	}
	if (ppdp.iSeconds < 1)
		ppdp.iSeconds = -1;
	if (_getOptB("PopupDefaultTimeout", defaultPopupDefaultTimeout) || popupType == POPUP_DEFAULT)
		ppdp.iSeconds = 0;

	ppdp.lchContact = hContact;
	_tcsncpy_s(ppdp.lptzContactName, (line1 ? line1 : _T(PLUGIN_NAME)), _TRUNCATE);
	if (line2)
		_tcsncpy_s(ppdp.lptzText, line2, _TRUNCATE);
	return PUAddPopupT(&ppdp);
}