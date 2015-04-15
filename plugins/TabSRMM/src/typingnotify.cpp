#include "commonheaders.h"

HANDLE hTypingNotify;

static HGENMENU hDisableMenu = NULL;

static HANDLE hPopupsList = NULL;

static BYTE   OnePopup;
static BYTE   ShowMenu;
static BYTE   StartDisabled;
static BYTE   StopDisabled;
static BYTE   Disabled;
static BYTE   ColorMode;
static BYTE   TimeoutMode;
static BYTE   TimeoutMode2;
static int    Timeout;
static int    Timeout2;
static int    newTimeout;
static int    newTimeout2;
static BYTE   newTimeoutMode;
static BYTE   newTimeoutMode2;
static BYTE   newColorMode;
static TCHAR  szStart[128];
static TCHAR  szStop[128];

static HANDLE hntfStarted = 0;
static HANDLE hntfStopped = 0;

struct
{
	int res;
	char desc[10];
	COLORREF color;
}
static colorPicker[4] =
{
	{ IDC_TYPEON_BG, "ON_BG", RGB(255, 255, 255) },
	{ IDC_TYPEON_TX, "ON_TX", RGB(0, 0, 0) },
	{ IDC_TYPEOFF_BG, "OFF_BG", RGB(255, 255, 255) },
	{ IDC_TYPEOFF_TX, "OFF_TX", RGB(0, 0, 0) }
};

static INT_PTR EnableDisableMenuCommand(WPARAM, LPARAM)
{
	Disabled = !Disabled;

	if (PluginConfig.g_bPopupAvail) {
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_ICON | CMIM_NAME;

		if (!Disabled) {
			mi.pszName = LPGEN("Disable &typing notification");
			mi.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ENABLED));
		}
		else {
			mi.pszName = LPGEN("Enable &typing notification");
			mi.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DISABLED));
		}

		Menu_ModifyItem(hDisableMenu, &mi);
	}

	return 0;
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			CallService(MS_MSG_SENDMESSAGET, (WPARAM)PUGetContact(hWnd), 0);
			PUDeletePopup(hWnd);
			return 1;
		}
		break;

	case WM_CONTEXTMENU:
		PUDeletePopup(hWnd);
		return 1;

	case UM_INITPOPUP:
		WindowList_Add(hPopupsList, hWnd, PUGetContact(hWnd));
		return 1;

	case UM_FREEPLUGINDATA:
		WindowList_Remove(hPopupsList, hWnd);
		return 1;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void TN_TypingMessage(MCONTACT hContact, int iMode)
{
	// hidden & ignored contacts check
	if (db_get_b(hContact, "CList", "Hidden", 0) || (db_get_dw(hContact, "Ignore", "Mask1", 0) & 1)) // 9 - online notification
		return;

	if (!PluginConfig.g_bPopupAvail || Disabled)
		return;

	TCHAR *szContactName = pcli->pfnGetContactDisplayName(hContact, 0);

	if (OnePopup) {
		HWND hPopupWnd = WindowList_Find(hPopupsList, hContact);
		while (hPopupWnd) {
			PUDeletePopup(hPopupWnd);
			hPopupWnd = WindowList_Find(hPopupsList, hContact);
		}
	}

	int notyping;
	POPUPDATAT_V2 ppd = { 0 };
	ppd.cbSize = sizeof(ppd);

	if (iMode == PROTOTYPE_CONTACTTYPING_OFF) {
		if (StopDisabled)
			return;
		_tcsncpy_s(ppd.lptzContactName, szContactName, _TRUNCATE);
		_tcsncpy_s(ppd.lptzText, szStop, _TRUNCATE);
		ppd.hNotification = hntfStopped;
		notyping = 1;
	}
	else {
		if (StartDisabled)
			return;
		_tcsncpy_s(ppd.lptzContactName, szContactName, _TRUNCATE);
		_tcsncpy_s(ppd.lptzText, szStart, _TRUNCATE);
		ppd.hNotification = hntfStarted;
		notyping = 0;
	}

	switch (ColorMode) {
	case COLOR_OWN:
		ppd.colorBack = colorPicker[2 * notyping].color;
		ppd.colorText = colorPicker[2 * notyping + 1].color;
		break;
	case COLOR_WINDOWS:
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);
		ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
		break;
	case COLOR_POPUP:
	default:
		ppd.colorBack = ppd.colorText = 0;
		break;
	}

	if (notyping)
		switch (TimeoutMode2) {
		case TIMEOUT_CUSTOM:
			ppd.iSeconds = Timeout2;
			break;
		case TIMEOUT_PERMANENT:
			ppd.iSeconds = -1;
			break;
		case TIMEOUT_POPUP:
		default:
			ppd.iSeconds = 0;
			break;
	}
	else
		switch (TimeoutMode) {
		case TIMEOUT_CUSTOM:
			ppd.iSeconds = Timeout;
			break;
		case TIMEOUT_PROTO:
			ppd.iSeconds = iMode;
			break;
		case TIMEOUT_PERMANENT:
			ppd.iSeconds = -1;
			break;
		case TIMEOUT_POPUP:
		default:
			ppd.iSeconds = 0;
			break;
	}

	ppd.lchIcon = PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING];
	ppd.lchContact = hContact;
	ppd.PluginWindowProc = PopupDlgProc;
	CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, APF_NEWDATA);
}

static INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		if (ColorMode == COLOR_WINDOWS) {
			CheckDlgButton(hwndDlg, IDC_USEWINCOLORS, BST_CHECKED);
			Utils::enableDlgControl(hwndDlg, IDC_USEPOPUPCOLORS, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_USEWINCOLORS, TRUE);
			CheckDlgButton(hwndDlg, IDC_USEPOPUPCOLORS, BST_UNCHECKED);
		}
		else if (ColorMode == COLOR_POPUP) {
			CheckDlgButton(hwndDlg, IDC_USEWINCOLORS, BST_UNCHECKED);
			Utils::enableDlgControl(hwndDlg, IDC_USEWINCOLORS, FALSE);
			Utils::enableDlgControl(hwndDlg, IDC_USEPOPUPCOLORS, TRUE);
			CheckDlgButton(hwndDlg, IDC_USEPOPUPCOLORS, BST_CHECKED);
		}

		for (i = 0; i < SIZEOF(colorPicker); i++) {
			SendDlgItemMessage(hwndDlg, colorPicker[i].res, CPM_SETCOLOUR, 0, colorPicker[i].color);
			Utils::enableDlgControl(hwndDlg, colorPicker[i].res, (ColorMode == COLOR_OWN));
		}

		CheckDlgButton(hwndDlg, IDC_TIMEOUT_PERMANENT, (TimeoutMode == TIMEOUT_PERMANENT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TIMEOUT_POPUP, (TimeoutMode == TIMEOUT_POPUP) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TIMEOUT_PROTO, (TimeoutMode == TIMEOUT_PROTO) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TIMEOUT_CUSTOM, (TimeoutMode == TIMEOUT_CUSTOM) ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg, IDC_TIMEOUT_VALUE, Timeout, 0);
		Utils::enableDlgControl(hwndDlg, IDC_TIMEOUT_VALUE, TimeoutMode == TIMEOUT_CUSTOM);

		CheckDlgButton(hwndDlg, IDC_TIMEOUT_PERMANENT2, (TimeoutMode2 == TIMEOUT_PERMANENT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TIMEOUT_POPUP2, (TimeoutMode2 == TIMEOUT_POPUP) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_TIMEOUT_CUSTOM2, (TimeoutMode2 == TIMEOUT_CUSTOM) ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemInt(hwndDlg, IDC_TIMEOUT_VALUE2, Timeout2, 0);
		Utils::enableDlgControl(hwndDlg, IDC_TIMEOUT_VALUE2, TimeoutMode2 == TIMEOUT_CUSTOM);

		CheckDlgButton(hwndDlg, IDC_START, (StartDisabled) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_STOP, (StopDisabled) ? BST_UNCHECKED : BST_CHECKED);

		CheckDlgButton(hwndDlg, IDC_ONEPOPUP, (OnePopup) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SHOWMENU, (ShowMenu) ? BST_CHECKED : BST_UNCHECKED);

		Utils::enableDlgControl(hwndDlg, IDC_ONEPOPUP, PluginConfig.g_bPopupAvail);
		Utils::enableDlgControl(hwndDlg, IDC_SHOWMENU, PluginConfig.g_bPopupAvail);
		Utils::enableDlgControl(hwndDlg, IDC_PREVIEW, PluginConfig.g_bPopupAvail);

		newTimeout = Timeout;
		newTimeoutMode = TimeoutMode;
		newTimeout2 = Timeout2;
		newTimeoutMode2 = TimeoutMode2;
		newColorMode = ColorMode;
		break;

	case WM_COMMAND:
	{
		WORD idCtrl = LOWORD(wParam), wNotifyCode = HIWORD(wParam);

		if (wNotifyCode == CPN_COLOURCHANGED) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}

		switch (idCtrl) {
		case IDC_USEWINCOLORS:
			if (wNotifyCode == BN_CLICKED) {
				bool bEnableOthers;

				if (IsDlgButtonChecked(hwndDlg, IDC_USEWINCOLORS)) {
					newColorMode = COLOR_WINDOWS;
					bEnableOthers = false;
				}
				else {
					newColorMode = COLOR_OWN;
					bEnableOthers = true;
				}

				for (i = 0; i < SIZEOF(colorPicker); i++)
					Utils::enableDlgControl(hwndDlg, colorPicker[i].res, bEnableOthers);

				Utils::enableDlgControl(hwndDlg, IDC_USEPOPUPCOLORS, bEnableOthers);

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_USEPOPUPCOLORS:
			if (wNotifyCode == BN_CLICKED) {
				bool bEnableOthers;

				if (IsDlgButtonChecked(hwndDlg, IDC_USEPOPUPCOLORS)) {
					newColorMode = COLOR_POPUP;
					bEnableOthers = false;
				}
				else {
					newColorMode = COLOR_OWN;
					bEnableOthers = true;
				}

				for (i = 0; i < sizeof(colorPicker) / sizeof(colorPicker[0]); i++)
					Utils::enableDlgControl(hwndDlg, colorPicker[i].res, bEnableOthers);

				Utils::enableDlgControl(hwndDlg, IDC_USEWINCOLORS, bEnableOthers);

				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}
			break;

		case IDC_ONEPOPUP:
		case IDC_CLIST:
		case IDC_DISABLED:
		case IDC_SHOWMENU:
		case IDC_START:
		case IDC_STOP:
		case IDC_WOCL:
			if (wNotifyCode == BN_CLICKED)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_PREVIEW:
			if (PluginConfig.g_bPopupAvail) {
				POPUPDATAT ppd = { 0 };
				for (i = 0; i < 2; i++) {
					int notyping;
					if (i == PROTOTYPE_CONTACTTYPING_OFF) {
						_tcsncpy_s(ppd.lptzContactName, TranslateT("Contact"), _TRUNCATE);
						_tcsncpy_s(ppd.lptzText, szStop, _TRUNCATE);
						notyping = 1;
					}
					else {
						_tcsncpy_s(ppd.lptzContactName, TranslateT("Contact"), _TRUNCATE);
						_tcsncpy_s(ppd.lptzText, szStart, _TRUNCATE);
						notyping = 0;
					}

					switch (newColorMode) {
					case COLOR_OWN:
						ppd.colorText = SendDlgItemMessage(hwndDlg, colorPicker[2 * notyping + 1].res, CPM_GETCOLOUR, 0, 0);
						ppd.colorBack = SendDlgItemMessage(hwndDlg, colorPicker[2 * notyping].res, CPM_GETCOLOUR, 0, 0);
						break;
					case COLOR_WINDOWS:
						ppd.colorBack = GetSysColor(COLOR_BTNFACE);
						ppd.colorText = GetSysColor(COLOR_WINDOWTEXT);
						break;
					case COLOR_POPUP:
					default:
						ppd.colorBack = ppd.colorText = 0;
						break;
					}

					if (notyping)
						switch (newTimeoutMode2) {
						case TIMEOUT_CUSTOM:
							ppd.iSeconds = newTimeout2;
							break;
						case TIMEOUT_PERMANENT:
							ppd.iSeconds = -1;
							break;
						case TIMEOUT_POPUP:
						default:
							ppd.iSeconds = 0;
							break;
					}
					else
						switch (newTimeoutMode) {
						case TIMEOUT_CUSTOM:
							ppd.iSeconds = newTimeout;
							break;
						case TIMEOUT_PROTO:
							ppd.iSeconds = 10;
							break;
						case TIMEOUT_PERMANENT:
							ppd.iSeconds = -1;
							break;
						case TIMEOUT_POPUP:
						default:
							ppd.iSeconds = 0;
							break;
					}

					ppd.lchIcon = PluginConfig.g_buttonBarIcons[ICON_DEFAULT_TYPING];
					ppd.lchContact = wParam;
					ppd.PluginWindowProc = NULL;
					ppd.PluginData = NULL;
					PUAddPopupT(&ppd);
				}
			}
			break;

		case IDC_TIMEOUT_POPUP2:
			if (wNotifyCode != BN_CLICKED)
				break;
			newTimeoutMode2 = TIMEOUT_POPUP;
			Utils::enableDlgControl(hwndDlg, IDC_TIMEOUT_VALUE2, 0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_TIMEOUT_CUSTOM2:
			if (wNotifyCode != BN_CLICKED)
				break;
			newTimeoutMode2 = TIMEOUT_CUSTOM;
			Utils::enableDlgControl(hwndDlg, IDC_TIMEOUT_VALUE2, 1);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_TIMEOUT_POPUP:
			if (wNotifyCode != BN_CLICKED)
				break;
			newTimeoutMode = TIMEOUT_POPUP;
			Utils::enableDlgControl(hwndDlg, IDC_TIMEOUT_VALUE, 0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_TIMEOUT_PERMANENT:
			if (wNotifyCode != BN_CLICKED)
				break;
			newTimeoutMode = TIMEOUT_PERMANENT;
			Utils::enableDlgControl(hwndDlg, IDC_TIMEOUT_VALUE, 0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_TIMEOUT_PERMANENT2:
			if (wNotifyCode != BN_CLICKED)
				break;
			newTimeoutMode2 = TIMEOUT_PERMANENT;
			Utils::enableDlgControl(hwndDlg, IDC_TIMEOUT_VALUE2, 0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_TIMEOUT_CUSTOM:
			if (wNotifyCode != BN_CLICKED)
				break;
			newTimeoutMode = TIMEOUT_CUSTOM;
			Utils::enableDlgControl(hwndDlg, IDC_TIMEOUT_VALUE, 1);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_TIMEOUT_PROTO:
			if (wNotifyCode != BN_CLICKED)
				break;
			newTimeoutMode = TIMEOUT_PROTO;
			Utils::enableDlgControl(hwndDlg, IDC_TIMEOUT_VALUE, 0);
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case IDC_TIMEOUT_VALUE:
		case IDC_TIMEOUT_VALUE2:
			int newValue = GetDlgItemInt(hwndDlg, idCtrl, NULL, 0);

			if (wNotifyCode == EN_KILLFOCUS) {
				int oldValue;

				if (idCtrl == IDC_TIMEOUT_VALUE)
					oldValue = newTimeout;
				else
					oldValue = newTimeout2;

				if (newValue != oldValue)
					SetDlgItemInt(hwndDlg, idCtrl, oldValue, 0);
				return TRUE;
			}
			if (wNotifyCode != EN_CHANGE || (HWND)lParam != GetFocus())
				return TRUE;

			if (newValue > TIMEOUT_MAXVALUE)
				newValue = TIMEOUT_MAXVALUE;
			else if (newValue < TIMEOUT_MINVALUE)
				newValue = TIMEOUT_MINVALUE;

			if (idCtrl == IDC_TIMEOUT_VALUE)
				newTimeout = newValue;
			else
				newTimeout2 = newValue;

			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
	}
	break;

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				for (i = 0; i < sizeof(colorPicker) / sizeof(colorPicker[0]); i++) {
					colorPicker[i].color = SendDlgItemMessage(hwndDlg, colorPicker[i].res, CPM_GETCOLOUR, 0, 0);
					db_set_dw(0, Module, colorPicker[i].desc, colorPicker[i].color);
				}

				Timeout = newTimeout;   TimeoutMode = newTimeoutMode;
				Timeout2 = newTimeout2; TimeoutMode2 = newTimeoutMode2;
				ColorMode = newColorMode;

				if (Disabled != IsDlgButtonChecked(hwndDlg, IDC_DISABLED))
					EnableDisableMenuCommand(0, 0);

				StartDisabled = IsDlgButtonChecked(hwndDlg, IDC_START) ? 0 : 2;
				StopDisabled = IsDlgButtonChecked(hwndDlg, IDC_STOP) ? 0 : 4;
				OnePopup = IsDlgButtonChecked(hwndDlg, IDC_ONEPOPUP);
				ShowMenu = IsDlgButtonChecked(hwndDlg, IDC_SHOWMENU);

				db_set_b(0, Module, SET_ONEPOPUP, OnePopup);
				db_set_b(0, Module, SET_SHOWDISABLEMENU, ShowMenu);
				db_set_b(0, Module, SET_DISABLED, (BYTE)(StartDisabled | StopDisabled));
				db_set_b(0, Module, SET_COLOR_MODE, ColorMode);
				db_set_b(0, Module, SET_TIMEOUT_MODE, TimeoutMode);
				db_set_b(0, Module, SET_TIMEOUT, (BYTE)Timeout);
				db_set_b(0, Module, SET_TIMEOUT_MODE2, TimeoutMode2);
				db_set_b(0, Module, SET_TIMEOUT2, (BYTE)Timeout2);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

int TN_OptionsInitialize(WPARAM wParam, LPARAM)
{
	if (ServiceExists(MS_POPUP_ADDPOPUPT)) {
		OPTIONSDIALOGPAGE odp = { 0 };
		odp.position = 100000000;
		odp.hInstance = g_hInst;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TYPINGNOTIFYPOPUP);
		odp.pszTitle = LPGEN("Typing notify");
		odp.pszGroup = LPGEN("Popups");
		odp.groupPosition = 910000000;
		odp.flags = ODPF_BOLDGROUPS;
		odp.pfnDlgProc = DlgProcOpts;
		Options_AddPage(wParam, &odp);
	}
	return 0;
}

int TN_ModuleInit()
{
	hPopupsList = WindowList_Create();

	OnePopup = M.GetByte(Module, SET_ONEPOPUP, DEF_ONEPOPUP);
	ShowMenu = M.GetByte(Module, SET_SHOWDISABLEMENU, DEF_SHOWDISABLEMENU);

	int i = M.GetByte(Module, SET_DISABLED, DEF_DISABLED);
	Disabled = i & 1;
	StartDisabled = i & 2;
	StopDisabled = i & 4;

	ColorMode = M.GetByte(Module, SET_COLOR_MODE, DEF_COLOR_MODE);
	TimeoutMode = M.GetByte(Module, SET_TIMEOUT_MODE, DEF_TIMEOUT_MODE);
	Timeout = M.GetByte(Module, SET_TIMEOUT, DEF_TIMEOUT);
	TimeoutMode2 = M.GetByte(Module, SET_TIMEOUT_MODE2, DEF_TIMEOUT_MODE2);
	Timeout2 = M.GetByte(Module, SET_TIMEOUT2, DEF_TIMEOUT2);

	if (!(M.GetDword(Module, colorPicker[0].desc, 1) && !M.GetDword(Module, colorPicker[0].desc, 0)))
		for (i = 0; i < SIZEOF(colorPicker); i++)
			colorPicker[i].color = M.GetDword(Module, colorPicker[i].desc, 0);

	mir_sntprintf(szStart, SIZEOF(szStart), TranslateT("...is typing a message."));
	mir_sntprintf(szStop, SIZEOF(szStop), TranslateT("...has stopped typing."));

	if (PluginConfig.g_bPopupAvail && ShowMenu) {
		hTypingNotify = CreateServiceFunction("TypingNotify/EnableDisableMenuCommand", EnableDisableMenuCommand);

		CLISTMENUITEM mi = { sizeof(mi) };
		if (!Disabled) {
			mi.pszName = LPGEN("Disable &typing notification");
			mi.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ENABLED));
		}
		else {
			mi.pszName = LPGEN("Enable &typing notification");
			mi.hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_DISABLED));
		}
		mi.pszService = "TypingNotify/EnableDisableMenuCommand";
		mi.pszPopupName = LPGEN("Popups");
		hDisableMenu = Menu_AddMainMenuItem(&mi);
	}

	SkinAddNewSoundEx("TNStart", LPGEN("Instant messages"), LPGEN("Contact started typing"));
	SkinAddNewSoundEx("TNStop", LPGEN("Instant messages"), LPGEN("Contact stopped typing"));
	return 0;
}

int TN_ModuleDeInit()
{
	WindowList_Destroy(hPopupsList);
	db_set_b(0, Module, SET_DISABLED, (BYTE)(Disabled | StartDisabled | StopDisabled));
	return 0;
}
