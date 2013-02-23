// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  PopUp Plugin stuff
//
// -----------------------------------------------------------------------------
#include "icqoscar.h"

extern BOOL bPopUpService;

static const UINT icqPopupsControls[] = {
	IDC_POPUPS_LOG_ENABLED, IDC_POPUPS_SPAM_ENABLED, IDC_PREVIEW, IDC_USESYSICONS, IDC_POPUP_LOG0_TIMEOUT,
	IDC_POPUP_LOG1_TIMEOUT, IDC_POPUP_LOG2_TIMEOUT, IDC_POPUP_LOG3_TIMEOUT, IDC_POPUP_SPAM_TIMEOUT
};

static const UINT icqPopupColorControls[] = {
	IDC_POPUP_LOG0_TEXTCOLOR, IDC_POPUP_LOG1_TEXTCOLOR, IDC_POPUP_LOG2_TEXTCOLOR, IDC_POPUP_LOG3_TEXTCOLOR, IDC_POPUP_SPAM_TEXTCOLOR,
	IDC_POPUP_LOG0_BACKCOLOR, IDC_POPUP_LOG1_BACKCOLOR, IDC_POPUP_LOG2_BACKCOLOR, IDC_POPUP_LOG3_BACKCOLOR, IDC_POPUP_SPAM_BACKCOLOR
};

INT_PTR CALLBACK DlgProcIcqPopupOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool bInitDone = true;
	BYTE bEnabled;
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

	switch (msg) {
	case WM_INITDIALOG:
		bInitDone = false;
		TranslateDialogDefault(hwndDlg);

		ppro = (CIcqProto*)lParam;
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

		CheckDlgButton(hwndDlg, IDC_POPUPS_LOG_ENABLED, ppro->getSettingByte(NULL,"PopupsLogEnabled",DEFAULT_LOG_POPUPS_ENABLED));
		CheckDlgButton(hwndDlg, IDC_POPUPS_SPAM_ENABLED, ppro->getSettingByte(NULL,"PopupsSpamEnabled",DEFAULT_SPAM_POPUPS_ENABLED));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG0_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups0TextColor",DEFAULT_LOG0_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG0_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups0BackColor",DEFAULT_LOG0_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG0_TIMEOUT, ppro->getSettingDword(NULL,"Popups0Timeout",DEFAULT_LOG0_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG1_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups1TextColor",DEFAULT_LOG1_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG1_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups1BackColor",DEFAULT_LOG1_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG1_TIMEOUT, ppro->getSettingDword(NULL,"Popups1Timeout",DEFAULT_LOG1_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG2_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups2TextColor",DEFAULT_LOG2_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG2_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups2BackColor",DEFAULT_LOG2_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG2_TIMEOUT, ppro->getSettingDword(NULL,"Popups2Timeout",DEFAULT_LOG2_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG3_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups3TextColor",DEFAULT_LOG3_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_LOG3_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"Popups3BackColor",DEFAULT_LOG3_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_LOG3_TIMEOUT, ppro->getSettingDword(NULL,"Popups3Timeout",DEFAULT_LOG3_TIMEOUT),FALSE);
		SendDlgItemMessage(hwndDlg, IDC_POPUP_SPAM_TEXTCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsSpamTextColor",DEFAULT_SPAM_TEXT_COLORS));
		SendDlgItemMessage(hwndDlg, IDC_POPUP_SPAM_BACKCOLOR, CPM_SETCOLOUR, 0, ppro->getSettingDword(NULL,"PopupsSpamBackColor",DEFAULT_SPAM_BACK_COLORS));
		SetDlgItemInt(hwndDlg, IDC_POPUP_SPAM_TIMEOUT, ppro->getSettingDword(NULL,"PopupsSpamTimeout",DEFAULT_SPAM_TIMEOUT),FALSE);
		bEnabled = ppro->getSettingByte(NULL,"PopupsWinColors",DEFAULT_POPUPS_WIN_COLORS);
		CheckDlgButton(hwndDlg, IDC_USEWINCOLORS, bEnabled);
		bEnabled |= ppro->getSettingByte(NULL,"PopupsDefColors",DEFAULT_POPUPS_DEF_COLORS);
		CheckDlgButton(hwndDlg, IDC_USEDEFCOLORS, bEnabled);
		icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled);
		CheckDlgButton(hwndDlg, IDC_USESYSICONS, ppro->getSettingByte(NULL,"PopupsSysIcons",DEFAULT_POPUPS_SYS_ICONS));
		bEnabled = ppro->getSettingByte(NULL,"PopupsEnabled",DEFAULT_POPUPS_ENABLED);
		CheckDlgButton(hwndDlg, IDC_POPUPS_ENABLED, bEnabled);
		icq_EnableMultipleControls(hwndDlg, icqPopupsControls, SIZEOF(icqPopupsControls), bEnabled);
		if (bEnabled)
		{
			if (IsDlgButtonChecked(hwndDlg, IDC_USEDEFCOLORS))
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEWINCOLORS), !WM_ENABLE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEDEFCOLORS), WM_ENABLE);
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEWINCOLORS), WM_ENABLE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEDEFCOLORS), !WM_ENABLE);
			}
		}
		icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled & (!IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS) && !IsDlgButtonChecked(hwndDlg,IDC_USEDEFCOLORS)));
		bInitDone = true;
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_PREVIEW:
			{
				ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Note"),    LOG_NOTE);
				ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Warning"), LOG_WARNING);
				ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Error"),   LOG_ERROR);
				ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Fatal"),   LOG_FATAL);
				ppro->ShowPopUpMsg(NULL, LPGEN("Popup Title"), LPGEN("Sample Spambot"), POPTYPE_SPAM);
			}
			return FALSE;

		case IDC_POPUPS_ENABLED:
			bEnabled = IsDlgButtonChecked(hwndDlg,IDC_POPUPS_ENABLED);
			if (bEnabled)
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_USEDEFCOLORS))
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_USEWINCOLORS), !WM_ENABLE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_USEDEFCOLORS), WM_ENABLE);
				}
				else
				{
					EnableWindow(GetDlgItem(hwndDlg, IDC_USEWINCOLORS), WM_ENABLE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_USEDEFCOLORS), !WM_ENABLE);
				}
			}
			else
			{
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEWINCOLORS), !WM_ENABLE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEDEFCOLORS), !WM_ENABLE);
			}
			icq_EnableMultipleControls(hwndDlg, icqPopupsControls, SIZEOF(icqPopupsControls), bEnabled);

		case IDC_USEWINCOLORS:
			bEnabled = IsDlgButtonChecked(hwndDlg,IDC_POPUPS_ENABLED);
			if (bEnabled)
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_USEWINCOLORS))
					EnableWindow(GetDlgItem(hwndDlg, IDC_USEDEFCOLORS), !WM_ENABLE);
				else
					EnableWindow(GetDlgItem(hwndDlg, IDC_USEDEFCOLORS), WM_ENABLE);
			}
			icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled & !IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS));

		case IDC_USEDEFCOLORS:
			bEnabled = IsDlgButtonChecked(hwndDlg,IDC_POPUPS_ENABLED);
			if (bEnabled)
			{
				if (IsDlgButtonChecked(hwndDlg, IDC_USEDEFCOLORS))
					EnableWindow(GetDlgItem(hwndDlg, IDC_USEWINCOLORS), !WM_ENABLE);
				else
					EnableWindow(GetDlgItem(hwndDlg, IDC_USEWINCOLORS), WM_ENABLE);
			}
			icq_EnableMultipleControls(hwndDlg, icqPopupColorControls, SIZEOF(icqPopupColorControls), bEnabled & !IsDlgButtonChecked(hwndDlg,IDC_USEDEFCOLORS));
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case IDC_POPUP_LOG0_TIMEOUT:
		case IDC_POPUP_LOG1_TIMEOUT:
		case IDC_POPUP_LOG2_TIMEOUT:
		case IDC_POPUP_LOG3_TIMEOUT:
		case IDC_POPUP_SPAM_TIMEOUT:
			if ((HIWORD(wParam) == EN_CHANGE) && bInitDone)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		default:
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
		case PSN_APPLY:
			ppro->setSettingByte(NULL,"PopupsEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_ENABLED));
			ppro->setSettingByte(NULL,"PopupsLogEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_LOG_ENABLED));
			ppro->setSettingByte(NULL,"PopupsSpamEnabled",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_POPUPS_SPAM_ENABLED));
			ppro->setSettingDword(NULL,"Popups0TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG0_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups0BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG0_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups0Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG0_TIMEOUT, NULL, FALSE));
			ppro->setSettingDword(NULL,"Popups1TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG1_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups1BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG1_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups1Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG1_TIMEOUT, NULL, FALSE));
			ppro->setSettingDword(NULL,"Popups2TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG2_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups2BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG2_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups2Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG2_TIMEOUT, NULL, FALSE));
			ppro->setSettingDword(NULL,"Popups3TextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG3_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups3BackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_LOG3_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"Popups3Timeout",GetDlgItemInt(hwndDlg, IDC_POPUP_LOG3_TIMEOUT, NULL, FALSE));
			ppro->setSettingDword(NULL,"PopupsSpamTextColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_SPAM_TEXTCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsSpamBackColor",SendDlgItemMessage(hwndDlg,IDC_POPUP_SPAM_BACKCOLOR,CPM_GETCOLOUR,0,0));
			ppro->setSettingDword(NULL,"PopupsSpamTimeout",GetDlgItemInt(hwndDlg, IDC_POPUP_SPAM_TIMEOUT, NULL, FALSE));
			ppro->setSettingByte(NULL,"PopupsWinColors",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_USEWINCOLORS));
			ppro->setSettingByte(NULL,"PopupsDefColors",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_USEDEFCOLORS));
			ppro->setSettingByte(NULL,"PopupsSysIcons",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_USESYSICONS));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int CIcqProto::ShowPopUpMsg(HANDLE hContact, const char *szTitle, const char *szMsg, BYTE bType)
{
	if (bPopUpService && getSettingByte(NULL, "PopupsEnabled", DEFAULT_POPUPS_ENABLED))
	{
		POPUPDATAEX ppd = {0};
		POPUPDATAW ppdw = {0};
		LPCTSTR rsIcon;
		char szPrefix[32], szSetting[32];

		strcpy(szPrefix, "Popups");
		ppd.iSeconds = 0;

		switch(bType) {
		case LOG_NOTE:
			rsIcon = MAKEINTRESOURCE(IDI_INFORMATION);
			ppd.colorBack = DEFAULT_LOG0_BACK_COLORS;
			ppd.colorText = DEFAULT_LOG0_TEXT_COLORS;
			strcat(szPrefix, "0");
			break;

		case LOG_WARNING:
			rsIcon = MAKEINTRESOURCE(IDI_WARNING);
			ppd.colorBack = DEFAULT_LOG1_BACK_COLORS;
			ppd.colorText = DEFAULT_LOG1_TEXT_COLORS;
			strcat(szPrefix, "1");
			break;

		case LOG_ERROR:
			rsIcon = MAKEINTRESOURCE(IDI_ERROR);
			ppd.colorBack = DEFAULT_LOG2_BACK_COLORS;
			ppd.colorText = DEFAULT_LOG2_TEXT_COLORS;
			strcat(szPrefix, "2");
			break;

		case LOG_FATAL:
			rsIcon = MAKEINTRESOURCE(IDI_ERROR);
			ppd.colorBack = DEFAULT_LOG3_BACK_COLORS;
			ppd.colorText = DEFAULT_LOG3_TEXT_COLORS;
			strcat(szPrefix, "3");
			break;

		case POPTYPE_SPAM:
			rsIcon = MAKEINTRESOURCE(IDI_WARNING);
			ppd.colorBack = DEFAULT_SPAM_BACK_COLORS;
			ppd.colorText = DEFAULT_SPAM_TEXT_COLORS;
			strcat(szPrefix, "Spam");
			break;
		default:
			return -1;
		}
		if (!getSettingByte(NULL, "PopupsSysIcons", DEFAULT_POPUPS_SYS_ICONS))
			ppd.lchIcon = Skin_GetIconByHandle(m_hProtoIcon);
		else
			ppd.lchIcon = (HICON)LoadImage( NULL, rsIcon, IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
		if (getSettingByte(NULL, "PopupsWinColors", DEFAULT_POPUPS_WIN_COLORS))
		{
			ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
			ppd.colorBack = GetSysColor(COLOR_WINDOW);
		}
		else
		{
			if (getSettingByte(NULL, "PopupsDefColors", DEFAULT_POPUPS_DEF_COLORS))
			{
				ppd.colorText = NULL;
				ppd.colorBack = NULL;
			}
			else
			{
				strcpy(szSetting, szPrefix);
				strcat(szSetting, "TextColor");
				ppd.colorText = getSettingDword(NULL, szSetting, ppd.colorText);
				strcpy(szSetting, szPrefix);
				strcat(szSetting, "BackColor");
				ppd.colorBack = getSettingDword(NULL, szSetting, ppd.colorBack);
			}
		}
		strcpy(szSetting, szPrefix);
		strcat(szSetting, "Timeout");
		ppd.iSeconds = getSettingDword(NULL, szSetting, ppd.iSeconds);

		// call unicode popup module - only on unicode OS otherwise it will not work properly :(
		// due to Popup Plug bug in ADDPOPUPW implementation
		if ( ServiceExists( MS_POPUP_ADDPOPUPW ))
		{
			char str[4096];

			make_unicode_string_static(ICQTranslateUtfStatic(szTitle, str, sizeof(str)), ppdw.lpwzContactName, MAX_CONTACTNAME);
			make_unicode_string_static(ICQTranslateUtfStatic(szMsg, str, sizeof(str)), ppdw.lpwzText, MAX_SECONDLINE);
			ppdw.lchContact = hContact;
			ppdw.lchIcon = ppd.lchIcon;
			ppdw.colorBack = ppd.colorBack;
			ppdw.colorText = ppd.colorText;
			ppdw.PluginWindowProc = NULL;
			ppdw.PluginData = NULL;
			ppdw.iSeconds = ppd.iSeconds;
			return CallService(MS_POPUP_ADDPOPUPW, (WPARAM)&ppdw, 0);
		}
		else

		{
			char str[MAX_PATH];

			utf8_decode_static(ICQTranslateUtfStatic(szTitle, str, MAX_PATH), ppd.lpzContactName, MAX_CONTACTNAME);
			utf8_decode_static(ICQTranslateUtfStatic(szMsg, str, MAX_PATH), ppd.lpzText, MAX_SECONDLINE);
			ppd.lchContact = hContact;
			ppd.PluginWindowProc = NULL;
			ppd.PluginData = NULL;

			return CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&ppd, 0);
		}
	}
	return -1; // Failure
}
