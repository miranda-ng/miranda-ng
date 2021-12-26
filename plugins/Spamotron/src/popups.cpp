#include "stdafx.h"

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
	bEnabled = IsDlgButtonChecked(hwnd, IDC_OPT_POPUPS_ENABLED);
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
	BOOL bEnabled = IsDlgButtonChecked(hwnd, IDC_OPT_POPUPS_ENABLED);
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
			bEnabled = g_plugin.getByte("NotifyPopup", defaultNotifyPopup);
			CheckDlgButton(optDlg, IDC_OPT_POPUPS_ENABLED, bEnabled ? BST_CHECKED : BST_UNCHECKED);
			EnablePopupControls(optDlg, bEnabled);

			CheckDlgButton(optDlg, IDC_OPT_POPUPS_NOTIFY_BLOCKED, g_plugin.getByte("NotifyPopupBlocked", defaultNotifyPopupBlocked) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_POPUPS_NOTIFY_APPROVED, g_plugin.getByte("NotifyPopupApproved", defaultNotifyPopupApproved) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_POPUPS_NOTIFY_CHALLENGE, g_plugin.getByte("NotifyPopupChallenge", defaultNotifyPopupChallenge) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_POPUPS_DEFAULT_COLORS, g_plugin.getByte("PopupDefaultColors", defaultPopupDefaultColors) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_POPUPS_WINDOWS_COLORS, g_plugin.getByte("PopupWindowsColors", defaultPopupWindowsColors) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(optDlg, IDC_OPT_POPUPS_DEFAULT_TIMEOUT, g_plugin.getByte("PopupDefaultTimeout", defaultPopupDefaultTimeout) ? BST_CHECKED : BST_UNCHECKED);
			SetDlgItemInt(optDlg, IDC_OPT_POPUPS_BLOCKED_TIMEOUT, g_plugin.getDword("PopupBlockedTimeout", defaultPopupBlockedTimeout), FALSE);
			SetDlgItemInt(optDlg, IDC_OPT_POPUPS_APPROVED_TIMEOUT, g_plugin.getDword("PopupApprovedTimeout", defaultPopupApprovedTimeout), FALSE);
			SetDlgItemInt(optDlg, IDC_OPT_POPUPS_CHALLENGE_TIMEOUT, g_plugin.getDword("PopupChallengeTimeout", defaultPopupChallengeTimeout), FALSE);
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_BLOCKED_FOREGROUND, CPM_SETCOLOUR, 0, g_plugin.getDword("PopupBlockedForeground", defaultPopupBlockedForeground));
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_BLOCKED_BACKGROUND, CPM_SETCOLOUR, 0, g_plugin.getDword("PopupBlockedBackground", defaultPopupBlockedBackground));
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_APPROVED_FOREGROUND, CPM_SETCOLOUR, 0, g_plugin.getDword("PopupApprovedForeground", defaultPopupApprovedForeground));
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_APPROVED_BACKGROUND, CPM_SETCOLOUR, 0, g_plugin.getDword("PopupApprovedBackground", defaultPopupApprovedBackground));
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_CHALLENGE_FOREGROUND, CPM_SETCOLOUR, 0, g_plugin.getDword("PopupChallengeForeground", defaultPopupChallengeForeground));
			SendDlgItemMessage(optDlg, IDC_OPT_POPUPS_CHALLENGE_BACKGROUND, CPM_SETCOLOUR, 0, g_plugin.getDword("PopupChallengeBackground", defaultPopupChallengeBackground));
			EnablePopupTimeouts(optDlg, IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_DEFAULT_TIMEOUT));
			EnablePopupColors(optDlg,
				g_plugin.getByte("PopupDefaultColors", defaultPopupDefaultColors),
				g_plugin.getByte("PopupWindowsColors", defaultPopupWindowsColors));

			bInitializing = 0;
			break;
		case WM_COMMAND:
			if (bInitializing)
				return FALSE;
			switch (LOWORD(wParam)) {
				case IDC_OPT_POPUPS_ENABLED:
					bEnabled = IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_ENABLED);
					EnablePopupControls(optDlg, bEnabled);
				case IDC_OPT_POPUPS_NOTIFY_BLOCKED:
				case IDC_OPT_POPUPS_NOTIFY_APPROVED:
				case IDC_OPT_POPUPS_NOTIFY_CHALLENGE:
				case IDC_OPT_POPUPS_DEFAULT_COLORS:
				case IDC_OPT_POPUPS_WINDOWS_COLORS:
				case IDC_OPT_POPUPS_DEFAULT_TIMEOUT:
					EnablePopupColors(optDlg,
						IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_DEFAULT_COLORS),
						IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_WINDOWS_COLORS));
					EnablePopupTimeouts(optDlg, IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_DEFAULT_TIMEOUT));

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
					ShowPopupPreview(optDlg, POPUP_BLOCKED, nullptr, TranslateT("Message blocked due to preview action"));
					ShowPopupPreview(optDlg, POPUP_APPROVED, nullptr, TranslateT("Contact approved due to preview action"));
					ShowPopupPreview(optDlg, POPUP_CHALLENGE, nullptr, TranslateT("Challenge sent to preview contact"));
					return FALSE;
			}
			SendMessage(GetParent(optDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch (((NMHDR*)lParam)->code) {
				case PSN_APPLY:
					g_plugin.setByte("NotifyPopup", IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_ENABLED));
					g_plugin.setByte("NotifyPopupBlocked", IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_NOTIFY_BLOCKED));
					g_plugin.setByte("NotifyPopupApproved", IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_NOTIFY_APPROVED));
					g_plugin.setByte("NotifyPopupChallenge", IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_NOTIFY_CHALLENGE));
					g_plugin.setByte("PopupDefaultColors", IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_DEFAULT_COLORS));
					g_plugin.setByte("PopupWindowsColors", IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_WINDOWS_COLORS));
					g_plugin.setByte("PopupDefaultTimeout", IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_DEFAULT_TIMEOUT));
					_saveDlgItemInt(optDlg, IDC_OPT_POPUPS_BLOCKED_TIMEOUT, "PopupBlockedTimeout");
					_saveDlgItemInt(optDlg, IDC_OPT_POPUPS_APPROVED_TIMEOUT, "PopupApprovedTimeout");
					_saveDlgItemInt(optDlg, IDC_OPT_POPUPS_CHALLENGE_TIMEOUT, "PopupChallengeTimeout");
					g_plugin.setDword("PopupBlockedForeground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_BLOCKED_FOREGROUND,CPM_GETCOLOUR,0,0));
					g_plugin.setDword("PopupBlockedBackground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_BLOCKED_BACKGROUND,CPM_GETCOLOUR,0,0));
					g_plugin.setDword("PopupApprovedForeground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_FOREGROUND,CPM_GETCOLOUR,0,0));
					g_plugin.setDword("PopupApprovedBackground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_BACKGROUND,CPM_GETCOLOUR,0,0));
					g_plugin.setDword("PopupChallengeForeground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_CHALLENGE_FOREGROUND,CPM_GETCOLOUR,0,0));
					g_plugin.setDword("PopupChallengeBackground", SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_CHALLENGE_BACKGROUND,CPM_GETCOLOUR,0,0));
					break;
			}
			break;
		case WM_DESTROY:
			break;
	}
	return FALSE;
}

int ShowPopupPreview(HWND optDlg, uint8_t popupType, wchar_t *line1, wchar_t *line2)
{
	POPUPDATAW ppd;
	switch (popupType)
	{
		case POPUP_DEFAULT:
			ppd.colorText = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_FOREGROUND,CPM_GETCOLOUR,0,0);
			ppd.colorBack = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_BACKGROUND,CPM_GETCOLOUR,0,0);
			break;
		case POPUP_BLOCKED:
			ppd.colorText = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_BLOCKED_FOREGROUND,CPM_GETCOLOUR,0,0);
			ppd.colorBack = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_BLOCKED_BACKGROUND,CPM_GETCOLOUR,0,0);
			ppd.iSeconds = GetDlgItemInt(optDlg, IDC_OPT_POPUPS_BLOCKED_TIMEOUT, nullptr, TRUE);
			ppd.lchIcon = Skin_LoadIcon(SKINICON_OTHER_DELETE);
			break;
		case POPUP_APPROVED:
			ppd.colorText = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_FOREGROUND,CPM_GETCOLOUR,0,0);
			ppd.colorBack = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_APPROVED_BACKGROUND,CPM_GETCOLOUR,0,0);
			ppd.iSeconds = GetDlgItemInt(optDlg, IDC_OPT_POPUPS_APPROVED_TIMEOUT, nullptr, TRUE);
			ppd.lchIcon = Skin_LoadIcon(SKINICON_OTHER_ADDCONTACT);
			break;
		case POPUP_CHALLENGE:
			ppd.colorText = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_CHALLENGE_FOREGROUND,CPM_GETCOLOUR,0,0);
			ppd.colorBack = SendDlgItemMessage(optDlg,IDC_OPT_POPUPS_CHALLENGE_BACKGROUND,CPM_GETCOLOUR,0,0);
			ppd.iSeconds = GetDlgItemInt(optDlg, IDC_OPT_POPUPS_CHALLENGE_TIMEOUT, nullptr, TRUE);
			ppd.lchIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
			break;
	}
	if (IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_WINDOWS_COLORS)) {
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		ppd.colorBack = GetSysColor(COLOR_WINDOW);
	}
	if (IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_DEFAULT_COLORS)) {
		ppd.colorText = NULL;
		ppd.colorBack = NULL;
	}
	if (ppd.iSeconds < 1)
		ppd.iSeconds = -1;
	if (IsDlgButtonChecked(optDlg, IDC_OPT_POPUPS_DEFAULT_TIMEOUT) || popupType == POPUP_DEFAULT)
		ppd.iSeconds = 0;

	ppd.lchContact = NULL;
	wcsncpy_s(ppd.lpwzContactName, (line1 ? line1 : _A2W(MODULENAME)), _TRUNCATE);
	if (line2)
		wcsncpy_s(ppd.lpwzText, line2, _TRUNCATE);
	return (int)PUAddPopupW(&ppd);
}

int ShowPopup(MCONTACT hContact, uint8_t popupType, wchar_t *line1, wchar_t *line2)
{
	POPUPDATAW ppd;
	switch (popupType)
	{
		case POPUP_DEFAULT:
			ppd.colorText = g_plugin.getDword("PopupApprovedForeground", defaultPopupApprovedForeground);
			ppd.colorBack = g_plugin.getDword("PopupApprovedBackground", defaultPopupApprovedBackground);
			break;
		case POPUP_BLOCKED:
			ppd.colorText = g_plugin.getDword("PopupBlockedForeground", defaultPopupBlockedForeground);
			ppd.colorBack = g_plugin.getDword("PopupBlockedBackground", defaultPopupBlockedBackground);
			ppd.iSeconds = g_plugin.getDword("PopupBlockedTimeout", defaultPopupBlockedTimeout);
			ppd.lchIcon = Skin_LoadIcon(SKINICON_OTHER_DELETE);
			break;
		case POPUP_APPROVED:
			ppd.colorText = g_plugin.getDword("PopupApprovedForeground", defaultPopupApprovedForeground);
			ppd.colorBack = g_plugin.getDword("PopupApprovedBackground", defaultPopupApprovedBackground);
			ppd.iSeconds = g_plugin.getDword("PopupApprovedTimeout", defaultPopupApprovedTimeout);
			ppd.lchIcon = Skin_LoadIcon(SKINICON_OTHER_ADDCONTACT);
			break;
		case POPUP_CHALLENGE:
			ppd.colorText = g_plugin.getDword("PopupChallengeForeground", defaultPopupChallengeForeground);
			ppd.colorBack = g_plugin.getDword("PopupChallengeBackground", defaultPopupChallengeBackground);
			ppd.iSeconds = g_plugin.getDword("PopupChallengeTimeout", defaultPopupChallengeTimeout);
			ppd.lchIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
			break;
	}
	if (g_plugin.getByte("PopupWindowsColors", defaultPopupWindowsColors)) {
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		ppd.colorBack = GetSysColor(COLOR_WINDOW);
	}
	if (g_plugin.getByte("PopupDefaultColors", defaultPopupDefaultColors)) {
		ppd.colorText = NULL;
		ppd.colorBack = NULL;
	}
	if (ppd.iSeconds < 1)
		ppd.iSeconds = -1;
	if (g_plugin.getByte("PopupDefaultTimeout", defaultPopupDefaultTimeout) || popupType == POPUP_DEFAULT)
		ppd.iSeconds = 0;

	ppd.lchContact = hContact;
	wcsncpy_s(ppd.lpwzContactName, (line1 ? line1 : _A2W(MODULENAME)), _TRUNCATE);
	if (line2)
		wcsncpy_s(ppd.lpwzText, line2, _TRUNCATE);
	return (int)PUAddPopupW(&ppd);
}