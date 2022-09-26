#include "stdafx.h"

static HGENMENU hDisableMenu = nullptr;

static MWindowList hPopupsList = nullptr;

static uint8_t OnePopup;
static uint8_t StartDisabled;
static uint8_t StopDisabled;
static uint8_t ColorMode;
static uint8_t TimeoutMode;
static uint8_t TimeoutMode2;
static int     Timeout;
static int     Timeout2;

static HANDLE hntfStarted = nullptr;
static HANDLE hntfStopped = nullptr;

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

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_COMMAND:
		if (HIWORD(wParam) == STN_CLICKED) {
			CallService(MS_MSG_SENDMESSAGEW, (WPARAM)PUGetContact(hWnd), 0);
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
	if (Contact::IsHidden(hContact) || (db_get_dw(hContact, "Ignore", "Mask1", 0) & 1)) // 9 - online notification
		return;

	if (!Contact::OnList(hContact) && !g_plugin.bTypingUnknown)
		return;

	if (!g_plugin.bPopups)
		return;

	wchar_t *szContactName = Clist_GetContactDisplayName(hContact);

	if (OnePopup) {
		HWND hPopupWnd = WindowList_Find(hPopupsList, hContact);
		while (hPopupWnd) {
			PUDeletePopup(hPopupWnd);
			hPopupWnd = WindowList_Find(hPopupsList, hContact);
		}
	}

	int notyping;
	POPUPDATAW ppd;

	if (iMode == PROTOTYPE_CONTACTTYPING_OFF) {
		if (StopDisabled)
			return;
		wcsncpy_s(ppd.lpwzContactName, szContactName, _TRUNCATE);
		wcsncpy_s(ppd.lpwzText, TranslateT("...has stopped typing."), _TRUNCATE);
		ppd.hNotification = hntfStopped;
		notyping = 1;
	}
	else {
		if (StartDisabled)
			return;
		wcsncpy_s(ppd.lpwzContactName, szContactName, _TRUNCATE);
		wcsncpy_s(ppd.lpwzText, TranslateT("...is typing a message."), _TRUNCATE);
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
	PUAddPopupW(&ppd);
}

class CTypingOpts : public CDlgBase
{
	int newTimeout, newTimeout2;
	uint8_t newTimeoutMode, newTimeoutMode2, newColorMode;

	CCtrlEdit value1, value2;
	CCtrlCheck chkWinColors, chkPopupColors;
	CCtrlCheck chkPopup, chkCustom, chkPermanent, chkProto;
	CCtrlCheck chkPopup2, chkCustom2, chkPermanent2;
	CCtrlButton btnPreview;

public:
	CTypingOpts() :
		CDlgBase(g_plugin, IDD_OPT_TYPINGNOTIFYPOPUP),
		btnPreview(this, IDC_PREVIEW),
		value1(this, IDC_TIMEOUT_VALUE),
		value2(this, IDC_TIMEOUT_VALUE2),
		chkProto(this, IDC_TIMEOUT_PROTO),
		chkPopup(this, IDC_TIMEOUT_POPUP),
		chkPopup2(this, IDC_TIMEOUT_POPUP2),
		chkCustom(this, IDC_TIMEOUT_CUSTOM),
		chkCustom2(this, IDC_TIMEOUT_CUSTOM2),
		chkPermanent(this, IDC_TIMEOUT_PERMANENT),
		chkPermanent2(this, IDC_TIMEOUT_PERMANENT2),
		chkWinColors(this, IDC_USEWINCOLORS),
		chkPopupColors(this, IDC_USEPOPUPCOLORS)
	{
		btnPreview.OnClick = Callback(this, &CTypingOpts::onClick_Preview);

		value1.OnChange = value2.OnChange = Callback(this, &CTypingOpts::onChange_Value);

		chkPopup.OnChange = Callback(this, &CTypingOpts::onChange_Popup);
		chkProto.OnChange = Callback(this, &CTypingOpts::onChange_Proto);
		chkCustom.OnChange = Callback(this, &CTypingOpts::onChange_Custom);
		chkPermanent.OnChange = Callback(this, &CTypingOpts::onChange_Permanent);

		chkPopup2.OnChange = Callback(this, &CTypingOpts::onChange_Popup2);
		chkCustom2.OnChange = Callback(this, &CTypingOpts::onChange_Custom2);
		chkPermanent2.OnChange = Callback(this, &CTypingOpts::onChange_Permanent2);

		chkWinColors.OnChange = Callback(this, &CTypingOpts::onChange_UseWinColors);
		chkPopupColors.OnChange = Callback(this, &CTypingOpts::onChange_UsePopupColors);
	}

	bool OnInitDialog() override
	{
		bool bWindows = (ColorMode == COLOR_WINDOWS);
		chkWinColors.SetState(bWindows);
		chkWinColors.Enable(bWindows);
		chkPopupColors.SetState(!bWindows);
		chkPopupColors.Enable(!bWindows);

		for (auto &it : colorPicker) {
			SendDlgItemMessage(m_hwnd, it.res, CPM_SETCOLOUR, 0, it.color);
			Utils::enableDlgControl(m_hwnd, it.res, (ColorMode == COLOR_OWN));
		}

		chkPopup.SetState(TimeoutMode == TIMEOUT_POPUP);
		chkProto.SetState(TimeoutMode == TIMEOUT_PROTO);
		chkCustom.SetState(TimeoutMode == TIMEOUT_CUSTOM);
		chkPermanent.SetState(TimeoutMode == TIMEOUT_PERMANENT);
		value1.SetInt(Timeout);
		Utils::enableDlgControl(m_hwnd, IDC_TIMEOUT_VALUE, TimeoutMode == TIMEOUT_CUSTOM);

		chkPopup2.SetState(TimeoutMode2 == TIMEOUT_POPUP);
		chkCustom2.SetState(TimeoutMode2 == TIMEOUT_CUSTOM);
		chkPermanent2.SetState(TimeoutMode2 == TIMEOUT_PERMANENT);
		value2.SetInt(Timeout2);
		Utils::enableDlgControl(m_hwnd, IDC_TIMEOUT_VALUE2, TimeoutMode2 == TIMEOUT_CUSTOM);

		CheckDlgButton(m_hwnd, IDC_START, (StartDisabled) ? BST_UNCHECKED : BST_CHECKED);
		CheckDlgButton(m_hwnd, IDC_STOP, (StopDisabled) ? BST_UNCHECKED : BST_CHECKED);

		CheckDlgButton(m_hwnd, IDC_ONEPOPUP, (OnePopup) ? BST_CHECKED : BST_UNCHECKED);

		newTimeout = Timeout;
		newTimeoutMode = TimeoutMode;
		newTimeout2 = Timeout2;
		newTimeoutMode2 = TimeoutMode2;
		newColorMode = ColorMode;
		return true;
	}

	bool OnApply() override
	{
		for (int i = 0; i < sizeof(colorPicker) / sizeof(colorPicker[0]); i++) {
			colorPicker[i].color = SendDlgItemMessage(m_hwnd, colorPicker[i].res, CPM_GETCOLOUR, 0, 0);
			db_set_dw(0, TypingModule, colorPicker[i].desc, colorPicker[i].color);
		}

		Timeout = newTimeout;   TimeoutMode = newTimeoutMode;
		Timeout2 = newTimeout2; TimeoutMode2 = newTimeoutMode2;
		ColorMode = newColorMode;

		StartDisabled = IsDlgButtonChecked(m_hwnd, IDC_START) ? 0 : 2;
		StopDisabled = IsDlgButtonChecked(m_hwnd, IDC_STOP) ? 0 : 4;
		OnePopup = IsDlgButtonChecked(m_hwnd, IDC_ONEPOPUP);

		db_set_b(0, TypingModule, SET_ONEPOPUP, OnePopup);
		db_set_b(0, TypingModule, SET_DISABLED, (uint8_t)(StartDisabled | StopDisabled));
		db_set_b(0, TypingModule, SET_COLOR_MODE, ColorMode);
		db_set_b(0, TypingModule, SET_TIMEOUT_MODE, TimeoutMode);
		db_set_b(0, TypingModule, SET_TIMEOUT, (uint8_t)Timeout);
		db_set_b(0, TypingModule, SET_TIMEOUT_MODE2, TimeoutMode2);
		db_set_b(0, TypingModule, SET_TIMEOUT2, (uint8_t)Timeout2);
		return true;
	}

	void onChange_UseWinColors(CCtrlCheck *pCheck)
	{
		bool bEnableOthers;

		if (pCheck->IsChecked()) {
			newColorMode = COLOR_WINDOWS;
			bEnableOthers = false;
		}
		else {
			newColorMode = COLOR_OWN;
			bEnableOthers = true;
		}

		for (auto &it : colorPicker)
			Utils::enableDlgControl(m_hwnd, it.res, bEnableOthers);

		chkPopupColors.Enable(bEnableOthers);
	}

	void onChange_UsePopupColors(CCtrlCheck *pCheck)
	{
		bool bEnableOthers;

		if (pCheck->IsChecked()) {
			newColorMode = COLOR_POPUP;
			bEnableOthers = false;
		}
		else {
			newColorMode = COLOR_OWN;
			bEnableOthers = true;
		}

		for (int i = 0; i < sizeof(colorPicker) / sizeof(colorPicker[0]); i++)
			Utils::enableDlgControl(m_hwnd, colorPicker[i].res, bEnableOthers);

		chkWinColors.Enable(bEnableOthers);
	}

	void onClick_Preview(CCtrlButton *)
	{
		for (int i = 0; i < 2; i++) {
			POPUPDATAW ppd;
			int notyping;
			if (i == PROTOTYPE_CONTACTTYPING_OFF) {
				wcsncpy_s(ppd.lpwzContactName, TranslateT("Contact"), _TRUNCATE);
				wcsncpy_s(ppd.lpwzText, TranslateT("...has stopped typing."), _TRUNCATE);
				notyping = 1;
			}
			else {
				wcsncpy_s(ppd.lpwzContactName, TranslateT("Contact"), _TRUNCATE);
				wcsncpy_s(ppd.lpwzText, TranslateT("...is typing a message."), _TRUNCATE);
				notyping = 0;
			}

			switch (newColorMode) {
			case COLOR_OWN:
				ppd.colorText = SendDlgItemMessage(m_hwnd, colorPicker[2 * notyping + 1].res, CPM_GETCOLOUR, 0, 0);
				ppd.colorBack = SendDlgItemMessage(m_hwnd, colorPicker[2 * notyping].res, CPM_GETCOLOUR, 0, 0);
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
			ppd.lchContact = 0;
			ppd.PluginWindowProc = nullptr;
			ppd.PluginData = nullptr;
			PUAddPopupW(&ppd);
		}
	}

	void onChange_Popup2(CCtrlCheck *)
	{
		newTimeoutMode2 = TIMEOUT_POPUP;
		Utils::enableDlgControl(m_hwnd, IDC_TIMEOUT_VALUE2, 0);
	}

	void onChange_Custom2(CCtrlCheck *)
	{
		newTimeoutMode2 = TIMEOUT_CUSTOM;
		Utils::enableDlgControl(m_hwnd, IDC_TIMEOUT_VALUE2, 1);
	}

	void onChange_Popup(CCtrlCheck *)
	{
		newTimeoutMode = TIMEOUT_POPUP;
		Utils::enableDlgControl(m_hwnd, IDC_TIMEOUT_VALUE, 0);
	}

	void onChange_Permanent(CCtrlCheck *)
	{
		newTimeoutMode = TIMEOUT_PERMANENT;
		Utils::enableDlgControl(m_hwnd, IDC_TIMEOUT_VALUE, 0);
	}

	void onChange_Permanent2(CCtrlCheck *)
	{
		newTimeoutMode2 = TIMEOUT_PERMANENT;
		Utils::enableDlgControl(m_hwnd, IDC_TIMEOUT_VALUE2, 0);
	}

	void onChange_Custom(CCtrlCheck *)
	{
		newTimeoutMode = TIMEOUT_CUSTOM;
		Utils::enableDlgControl(m_hwnd, IDC_TIMEOUT_VALUE, 1);
	}

	void onChange_Proto(CCtrlCheck *)
	{
		newTimeoutMode = TIMEOUT_PROTO;
		Utils::enableDlgControl(m_hwnd, IDC_TIMEOUT_VALUE, 0);
	}

	void onChange_Value(CCtrlEdit *pEdit)
	{
		int newValue = pEdit->GetInt();

		if (newValue > TIMEOUT_MAXVALUE)
			newValue = TIMEOUT_MAXVALUE;
		else if (newValue < TIMEOUT_MINVALUE)
			newValue = TIMEOUT_MINVALUE;

		if (pEdit->GetCtrlId() == IDC_TIMEOUT_VALUE)
			newTimeout = newValue;
		else
			newTimeout2 = newValue;
	}
};

int TN_OptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = 100000000;
	odp.szTitle.a = LPGEN("Typing notify");
	odp.szGroup.a = LPGEN("Popups");
	odp.pDialog = new CTypingOpts();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

int TN_ModuleInit()
{
	hPopupsList = WindowList_Create();

	OnePopup = db_get_b(0, TypingModule, SET_ONEPOPUP, DEF_ONEPOPUP);

	int i = db_get_b(0, TypingModule, SET_DISABLED, DEF_DISABLED);
	if (i & 1)
		g_plugin.bPopups = false;
	StartDisabled = i & 2;
	StopDisabled = i & 4;

	ColorMode = db_get_b(0, TypingModule, SET_COLOR_MODE, DEF_COLOR_MODE);
	TimeoutMode = db_get_b(0, TypingModule, SET_TIMEOUT_MODE, DEF_TIMEOUT_MODE);
	Timeout = db_get_b(0, TypingModule, SET_TIMEOUT, DEF_TIMEOUT);
	TimeoutMode2 = db_get_b(0, TypingModule, SET_TIMEOUT_MODE2, DEF_TIMEOUT_MODE2);
	Timeout2 = db_get_b(0, TypingModule, SET_TIMEOUT2, DEF_TIMEOUT2);

	if (!(db_get_dw(0, TypingModule, colorPicker[0].desc, 1) && !db_get_dw(0, TypingModule, colorPicker[0].desc, 0)))
		for (auto &it : colorPicker)
			it.color = db_get_dw(0, TypingModule, it.desc, 0);

	g_plugin.addPopupOption(LPGEN("Typing notifications"), g_plugin.bPopups);

	g_plugin.addSound("TNStart", LPGENW("Instant messages"), LPGENW("Contact started typing"));
	g_plugin.addSound("TNStop", LPGENW("Instant messages"), LPGENW("Contact stopped typing"));
	return 0;
}

int TN_ModuleDeInit()
{
	WindowList_Destroy(hPopupsList);
	db_set_b(0, TypingModule, SET_DISABLED, (uint8_t)(StartDisabled | StopDisabled));
	return 0;
}
