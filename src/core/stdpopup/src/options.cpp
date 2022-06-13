#include "stdafx.h"

Options options;

HICON hPopupIcon = nullptr;

void LoadModuleDependentOptions()
{
	if (ServiceExists(MS_AV_DRAWAVATAR))
		options.av_layout = (PopupAvLayout)g_plugin.getByte("AVLayout", PAV_RIGHT);
	else
		options.av_layout = PAV_NONE;

	options.time_layout = (PopupTimeLayout)g_plugin.getByte("TimeLayout", (ServiceExists(MS_AV_DRAWAVATAR) ? PT_WITHAV : PT_RIGHT));
	if (options.time_layout == PT_WITHAV && !ServiceExists(MS_AV_DRAWAVATAR))
		options.time_layout = PT_RIGHT;
}

void LoadOptions()
{
	options.default_timeout = g_plugin.getDword("DefaultTimeout", 7);
	options.win_width = g_plugin.getDword("WinWidth", 220);
	options.win_max_height = g_plugin.getDword("WinMaxHeight", 400);
	options.location = (PopupLocation)g_plugin.getByte("Location", (uint8_t)PL_BOTTOMRIGHT);
	options.opacity = g_plugin.getByte("Opacity", 75);
	options.border = g_plugin.getByte("Border", 1) == 1;
	options.round = g_plugin.getByte("RoundCorners", 1) == 1;
	options.av_round = g_plugin.getByte("AvatarRoundCorners", 1) == 1;
	options.animate = g_plugin.getByte("Animate", 0);
	options.trans_bg = g_plugin.getByte("TransparentBg", 0) == 1;
	options.use_mim_monitor = g_plugin.getByte("UseMimMonitor", 1) == 1;
	options.right_icon = g_plugin.getByte("RightIcon", 0) == 1;
	options.av_layout = PAV_NONE; // corrected in LoadModuleDependentOptions function above
	options.av_size = g_plugin.getDword("AVSize", 40); //tweety
	options.text_indent = g_plugin.getDword("TextIndent", 22);
	options.global_hover = g_plugin.getByte("GlobalHover", 1) == 1;
	options.time_layout = (PopupTimeLayout)g_plugin.getByte("TimeLayout", PT_RIGHT);

	char buff[128];
	for (int i = 0; i < _countof(options.disable_status); i++) {
		mir_snprintf(buff, "DisableStatus%d", i - 1); // -1 because i forgot offline status earlier!
		options.disable_status[i] = (g_plugin.getByte(buff, 0) == 1);
	}

	options.disable_full_screen = g_plugin.getByte("DisableFullScreen", 1) == 1;
	options.drop_shadow = g_plugin.getByte("DropShadow", 0) == 1;
	options.sb_width = g_plugin.getDword("SidebarWidth", 22);
	options.padding = g_plugin.getDword("Padding", 4);
	options.av_padding = g_plugin.getDword("AvatarPadding", 4);
}

void SaveOptions()
{
	g_plugin.setDword("DefaultTimeout", options.default_timeout);
	g_plugin.setDword("WinWidth", options.win_width);
	g_plugin.setDword("WinMaxHeight", options.win_max_height);
	g_plugin.setByte("Location", (uint8_t)options.location);
	g_plugin.setByte("Opacity", (uint8_t)options.opacity);
	g_plugin.setByte("Border", (options.border ? 1 : 0));
	g_plugin.setByte("RoundCorners", (options.round ? 1 : 0));
	g_plugin.setByte("AvatarRoundCorners", (options.av_round ? 1 : 0));
	g_plugin.setByte("Animate", options.animate);
	g_plugin.setByte("TransparentBg", (options.trans_bg ? 1 : 0));
	g_plugin.setByte("UseMimMonitor", (options.use_mim_monitor ? 1 : 0));
	g_plugin.setByte("RightIcon", (options.right_icon ? 1 : 0));
	if (ServiceExists(MS_AV_DRAWAVATAR))
		g_plugin.setByte("AVLayout", (uint8_t)options.av_layout);
	g_plugin.setDword("AVSize", options.av_size);
	g_plugin.setDword("TextIndent", options.text_indent);
	g_plugin.setByte("GlobalHover", (options.global_hover ? 1 : 0));
	g_plugin.setByte("TimeLayout", (uint8_t)options.time_layout);

	char buff[128];
	for (int i = 0; i < _countof(options.disable_status); i++) {
		mir_snprintf(buff, "DisableStatus%d", i - 1);
		g_plugin.setByte(buff, options.disable_status[i] ? 1 : 0);
	}
	g_plugin.setByte("DisableFullScreen", (options.disable_full_screen ? 1 : 0));
	g_plugin.setByte("DropShadow", (options.drop_shadow ? 1 : 0));
	g_plugin.setDword("SidebarWidth", options.sb_width);
	g_plugin.setDword("Padding", options.padding);
	g_plugin.setDword("AvatarPadding", options.av_padding);
}

class CMainOptDlg : public CDlgBase
{
	CCtrlCheck chkTimeout, chkNoTimeout;
	CCtrlButton btnPreview;

public: 
	CMainOptDlg() :
		CDlgBase(g_plugin, IDD_OPT1),
		btnPreview(this, IDC_BTN_PREVIEW),
		chkTimeout(this, IDC_RAD_TIMEOUT),
		chkNoTimeout(this, IDC_RAD_NOTIMEOUT)
	{
		btnPreview.OnClick = Callback(this, &CMainOptDlg::onClick_Preview);

		chkTimeout.OnChange = chkNoTimeout.OnChange = Callback(this, &CMainOptDlg::onChange_Timeout);
	}

	bool OnInitDialog() override
	{
		SendDlgItemMessage(m_hwnd, IDC_CMB_PLACEMENT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom right"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_PLACEMENT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Bottom left"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_PLACEMENT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top right"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_PLACEMENT, CB_ADDSTRING, 0, (LPARAM)TranslateT("Top left"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_PLACEMENT, CB_SETCURSEL, (int)options.location, 0);

		SendDlgItemMessage(m_hwnd, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("Icon on left"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_ICON, CB_ADDSTRING, 0, (LPARAM)TranslateT("Icon on right"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_ICON, CB_SETCURSEL, (options.right_icon ? 1 : 0), 0);

		SendDlgItemMessage(m_hwnd, IDC_CMB_TIME, CB_ADDSTRING, 0, (LPARAM)TranslateT("No time"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_TIME, CB_ADDSTRING, 0, (LPARAM)TranslateT("Time on left"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_TIME, CB_ADDSTRING, 0, (LPARAM)TranslateT("Time on right"));
		if (ServiceExists(MS_AV_DRAWAVATAR))
			SendDlgItemMessage(m_hwnd, IDC_CMB_TIME, CB_ADDSTRING, 0, (LPARAM)TranslateT("Time above avatar"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_TIME, CB_SETCURSEL, (int)options.time_layout, 0);

		SendDlgItemMessage(m_hwnd, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("No avatar"));
		if (ServiceExists(MS_AV_DRAWAVATAR)) {
			SendDlgItemMessage(m_hwnd, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Left avatar"));
			SendDlgItemMessage(m_hwnd, IDC_CMB_AV, CB_ADDSTRING, 0, (LPARAM)TranslateT("Right avatar"));
		}
		else {
			HWND hw = GetDlgItem(m_hwnd, IDC_CMB_AV);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(m_hwnd, IDC_SPIN_AVSIZE);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(m_hwnd, IDC_ED_AVSIZE);
			EnableWindow(hw, FALSE);
		}
		SendDlgItemMessage(m_hwnd, IDC_CMB_AV, CB_SETCURSEL, (int)options.av_layout, 0);

		CheckDlgButton(m_hwnd, IDC_CHK_GLOBALHOVER, options.global_hover ? BST_CHECKED : BST_UNCHECKED);

		{
			// initialise and fill listbox
			HWND hwndList = GetDlgItem(m_hwnd, IDC_LST_STATUS);
			ListView_DeleteAllItems(hwndList);

			SendMessage(hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

			LVCOLUMN lvc = { 0 };
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lvc.fmt = LVCFMT_LEFT;

			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Status");
			lvc.cx = 200;     // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);

			LVITEM lvI = { 0 };

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items. 
			lvI.mask = LVIF_TEXT;

			int i = 0;
			for (; i < _countof(options.disable_status); i++) {
				lvI.pszText = Clist_GetStatusModeDescription(ID_STATUS_OFFLINE + i, 0);
				lvI.iItem = i;
				ListView_InsertItem(hwndList, &lvI);
				ListView_SetCheckState(hwndList, i, options.disable_status[i]);
			}
			lvI.pszText = TranslateT("Full-screen app running");
			lvI.iItem = i;
			ListView_InsertItem(hwndList, &lvI);
			ListView_SetCheckState(hwndList, i, options.disable_full_screen);
		}

		SendDlgItemMessage(m_hwnd, IDC_SPIN_TIMEOUT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(360, 1));
		SendDlgItemMessage(m_hwnd, IDC_SPIN_WIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(m_hwnd, IDC_SPIN_MAXHEIGHT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 16));
		SendDlgItemMessage(m_hwnd, IDC_SPIN_TRANS, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 1));
		SendDlgItemMessage(m_hwnd, IDC_SPIN_AVSIZE, UDM_SETRANGE, 0, (LPARAM)MAKELONG(100, 16));
		SendDlgItemMessage(m_hwnd, IDC_SPIN_INDENT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));
		SendDlgItemMessage(m_hwnd, IDC_SPIN_SBWIDTH, UDM_SETRANGE, 0, (LPARAM)MAKELONG(2048, 0));
		SendDlgItemMessage(m_hwnd, IDC_SPIN_PADDING, UDM_SETRANGE, 0, (LPARAM)MAKELONG(400, 0));

		if (options.default_timeout == -1) {
			CheckDlgButton(m_hwnd, IDC_RAD_NOTIMEOUT, BST_CHECKED);
			HWND hw = GetDlgItem(m_hwnd, IDC_ED_TIMEOUT);
			EnableWindow(hw, FALSE);
			SetDlgItemInt(m_hwnd, IDC_ED_TIMEOUT, 7, FALSE);
		}
		else {
			CheckDlgButton(m_hwnd, IDC_RAD_TIMEOUT, BST_CHECKED);
			SetDlgItemInt(m_hwnd, IDC_ED_TIMEOUT, options.default_timeout, FALSE);
		}

		if (options.right_icon)
			CheckDlgButton(m_hwnd, IDC_RAD_RIGHTICON, BST_CHECKED);
		else
			CheckDlgButton(m_hwnd, IDC_RAD_LEFTICON, BST_CHECKED);

		if (ServiceExists(MS_AV_DRAWAVATAR)) {
			switch (options.av_layout) {
			case PAV_NONE: CheckDlgButton(m_hwnd, IDC_RAD_NOAV, BST_CHECKED); break;
			case PAV_RIGHT: CheckDlgButton(m_hwnd, IDC_RAD_RIGHTAV, BST_CHECKED); break;
			case PAV_LEFT: CheckDlgButton(m_hwnd, IDC_RAD_LEFTAV, BST_CHECKED); break;
			}
		}
		else {
			CheckDlgButton(m_hwnd, IDC_RAD_NOAV, BST_CHECKED);
			HWND hw = GetDlgItem(m_hwnd, IDC_RAD_RIGHTAV);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(m_hwnd, IDC_RAD_LEFTAV);
			EnableWindow(hw, FALSE);
		}

		SetDlgItemInt(m_hwnd, IDC_ED_WIDTH, options.win_width, FALSE);
		SetDlgItemInt(m_hwnd, IDC_ED_MAXHEIGHT, options.win_max_height, FALSE);
		SetDlgItemInt(m_hwnd, IDC_ED_AVSIZE, options.av_size, FALSE);
		SetDlgItemInt(m_hwnd, IDC_ED_INDENT, options.text_indent, FALSE);
		SetDlgItemInt(m_hwnd, IDC_ED_SBWIDTH, options.sb_width, FALSE);
		SetDlgItemInt(m_hwnd, IDC_ED_PADDING, options.padding, FALSE);

		switch (options.location) {
		case PL_BOTTOMRIGHT: CheckDlgButton(m_hwnd, IDC_RAD_BOTTOMRIGHT, BST_CHECKED); break;
		case PL_BOTTOMLEFT: CheckDlgButton(m_hwnd, IDC_RAD_BOTTOMLEFT, BST_CHECKED); break;
		case PL_TOPRIGHT: CheckDlgButton(m_hwnd, IDC_RAD_TOPRIGHT, BST_CHECKED); break;
		case PL_TOPLEFT: CheckDlgButton(m_hwnd, IDC_RAD_TOPLEFT, BST_CHECKED); break;
		}

		SetDlgItemInt(m_hwnd, IDC_ED_TRANS, options.opacity, FALSE);
		CheckDlgButton(m_hwnd, IDC_CHK_BORDER, options.border ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CHK_ROUNDCORNERS, options.round ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(m_hwnd, IDC_CHK_ROUNDCORNERSAV, options.av_round ? BST_CHECKED : BST_UNCHECKED);

		SendDlgItemMessage(m_hwnd, IDC_CMB_ANIMATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("No animate"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_ANIMATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Horizontal animate"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_ANIMATE, CB_ADDSTRING, 0, (LPARAM)TranslateT("Vertical animate"));
		SendDlgItemMessage(m_hwnd, IDC_CMB_ANIMATE, CB_SETCURSEL, options.animate, 0);

		CheckDlgButton(m_hwnd, IDC_CHK_TRANSBG, options.trans_bg ? BST_CHECKED : BST_UNCHECKED);
		return true;
	}

	bool OnApply()
	{
		BOOL trans;
		int new_val;
		if (IsDlgButtonChecked(m_hwnd, IDC_RAD_NOTIMEOUT))
			options.default_timeout = -1;
		else {
			new_val = GetDlgItemInt(m_hwnd, IDC_ED_TIMEOUT, &trans, FALSE);
			if (trans) options.default_timeout = new_val;
		}
		if (options.default_timeout == 0) {
			SetDlgItemInt(m_hwnd, IDC_ED_TIMEOUT, options.default_timeout, FALSE);
			MessageBox(m_hwnd, TranslateT("You cannot set a default timeout of 0.\nValue has been reset."), TranslateT("Error"), MB_OK | MB_ICONWARNING);
			options.default_timeout = 7; // prevent instant timeout
		}

		new_val = GetDlgItemInt(m_hwnd, IDC_ED_WIDTH, &trans, FALSE);
		if (trans) options.win_width = new_val;
		new_val = GetDlgItemInt(m_hwnd, IDC_ED_MAXHEIGHT, &trans, FALSE);
		if (trans) options.win_max_height = new_val;
		new_val = GetDlgItemInt(m_hwnd, IDC_ED_AVSIZE, &trans, FALSE);
		if (trans) options.av_size = new_val;
		new_val = GetDlgItemInt(m_hwnd, IDC_ED_INDENT, &trans, FALSE);
		if (trans) options.text_indent = new_val;
		new_val = GetDlgItemInt(m_hwnd, IDC_ED_SBWIDTH, &trans, FALSE);
		if (trans) options.sb_width = new_val;
		new_val = GetDlgItemInt(m_hwnd, IDC_ED_PADDING, &trans, FALSE);
		if (trans) options.padding = new_val;

		options.location = (PopupLocation)SendDlgItemMessage(m_hwnd, IDC_CMB_PLACEMENT, CB_GETCURSEL, 0, 0);
		options.right_icon = (SendDlgItemMessage(m_hwnd, IDC_CMB_ICON, CB_GETCURSEL, 0, 0) == 1);
		options.av_layout = (PopupAvLayout)SendDlgItemMessage(m_hwnd, IDC_CMB_AV, CB_GETCURSEL, 0, 0);
		options.time_layout = (PopupTimeLayout)SendDlgItemMessage(m_hwnd, IDC_CMB_TIME, CB_GETCURSEL, 0, 0);

		new_val = GetDlgItemInt(m_hwnd, IDC_ED_TRANS, &trans, FALSE);
		if (trans) options.opacity = new_val;
		options.border = IsDlgButtonChecked(m_hwnd, IDC_CHK_BORDER) && IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHK_BORDER)) ? true : false;
		options.round = IsDlgButtonChecked(m_hwnd, IDC_CHK_ROUNDCORNERS) && IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHK_ROUNDCORNERS)) ? true : false;
		options.av_round = IsDlgButtonChecked(m_hwnd, IDC_CHK_ROUNDCORNERSAV) && IsWindowEnabled(GetDlgItem(m_hwnd, IDC_CHK_ROUNDCORNERSAV)) ? true : false;
		options.animate = SendDlgItemMessage(m_hwnd, IDC_CMB_ANIMATE, CB_GETCURSEL, 0, 0);
		options.trans_bg = IsDlgButtonChecked(m_hwnd, IDC_CHK_TRANSBG) ? true : false;
		options.global_hover = IsDlgButtonChecked(m_hwnd, IDC_CHK_GLOBALHOVER) ? true : false;

		int i = 0;
		for (; i < _countof(options.disable_status); i++)
			options.disable_status[i] = (ListView_GetCheckState(GetDlgItem(m_hwnd, IDC_LST_STATUS), i) == 1);
		options.disable_full_screen = (ListView_GetCheckState(GetDlgItem(m_hwnd, IDC_LST_STATUS), i) == 1);

		SaveOptions();
		return true;
	}

	void onClick_Preview(CCtrlButton*)
	{
		PopupData pd = { sizeof(PopupData) };
		pd.hIcon = hPopupIcon;
		pd.flags = PDF_UNICODE;

		pd.pwszTitle = TranslateT("Example");
		pd.pwszText = TranslateT("The quick brown fox jumps over the lazy dog.");
		ShowPopup(pd);

		pd.pwszTitle = TranslateT("Example With a Long Title");
		pd.pwszText = TranslateT("The quick brown fox jumps over the lazy dog.");
		ShowPopup(pd);

		pd.pwszTitle = TranslateT("Example");
		pd.pwszText = TranslateT("Thequickbrownfoxjumpsoverthelazydog.");
		ShowPopup(pd);

		for (auto &hContact : Contacts()) {
			if (options.av_layout != PAV_NONE && ServiceExists(MS_AV_DRAWAVATAR)) {
				AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, hContact, 0);
				if (ace && (ace->dwFlags & AVS_BITMAP_VALID)) {
					pd.hContact = hContact;
					pd.pwszText = TranslateT("An avatar.");
					ShowPopup(pd);
					break;
				}
			}
		}
	}

	void onChange_Timeout(CCtrlCheck *)
	{
		EnableWindow(GetDlgItem(m_hwnd, IDC_ED_TIMEOUT), chkTimeout.GetState());
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Classes' options dialog

class CClassesOptDlg : public CDlgBase
{
	LIST<POPUPCLASS> arNewClasses;

	CCtrlEdit edtTimeout;
	CCtrlCheck chkTimeout;
	CCtrlColor m_backColor, m_textColor;
	CCtrlButton btnPreview;
	CCtrlListBox m_list;

	POPUPCLASS *getCurData()
	{
		int index = m_list.GetCurSel();
		return (index != -1) ? (POPUPCLASS *)m_list.GetItemData(index) : nullptr;
	}

public:
	CClassesOptDlg() :
		CDlgBase(g_plugin, IDD_CLASSES),
		arNewClasses(3),
		m_list(this, IDC_LST_CLASSES),
		m_backColor(this, IDC_COL_BG),
		m_textColor(this, IDC_COL_TEXT),
		btnPreview(this, IDC_BTN_PREVIEW),
		chkTimeout(this, IDC_CHK_TIMEOUT),
		edtTimeout(this, IDC_ED_TIMEOUT)
	{
		arNewClasses = arClasses;

		m_list.OnSelChange = Callback(this, &CClassesOptDlg::onSelChange_List);

		btnPreview.OnClick = Callback(this, &CClassesOptDlg::onClick_Preview);

		chkTimeout.OnChange = Callback(this, &CClassesOptDlg::onCheck_Timeout);
		edtTimeout.OnChange = Callback(this, &CClassesOptDlg::onChange_Timeout);
		m_backColor.OnChange = Callback(this, &CClassesOptDlg::onChange_BackColor);
		m_textColor.OnChange = Callback(this, &CClassesOptDlg::onChange_TextColor);
	}

	~CClassesOptDlg()
	{
		arNewClasses.destroy();
	}

	bool OnInitDialog() override
	{
		for (auto &it: arNewClasses) {
			if (it->flags & PCF_UNICODE)
				m_list.AddString(it->pszDescription.w, (LPARAM)it);
			else
				m_list.AddString(_A2T(it->pszDescription.a), (LPARAM)it);
		}
		return true;
	}

	bool OnApply() override
	{
		arClasses = arNewClasses;
		
		char setting[256];
		for (auto &pc : arClasses) {
			mir_snprintf(setting, "%s/Timeout", pc->pszName);
			g_plugin.setWord(setting, pc->iSeconds);
			mir_snprintf(setting, "%s/TextCol", pc->pszName);
			g_plugin.setDword(setting, (uint32_t)pc->colorText);
			mir_snprintf(setting, "%s/BgCol", pc->pszName);
			g_plugin.setDword(setting, (uint32_t)pc->colorBack);
		}
		return true;
	}

	void onSelChange_List(CCtrlListBox *)
	{
		auto *p = getCurData();
		btnPreview.Enable(p != nullptr);
		chkTimeout.Enable(p != nullptr);
		m_textColor.Enable(p != nullptr);
		m_backColor.Enable(p != nullptr);

		if (p) {
			m_textColor.SetColor(p->colorText);
			m_backColor.SetColor(p->colorBack);
			chkTimeout.SetState(p->iSeconds != -1);
			edtTimeout.SetInt(p->iSeconds);
		}

		edtTimeout.Enable(p != nullptr && chkTimeout.GetState());
	}

	void onChange_Timeout(CCtrlEdit *)
	{
		auto *p = getCurData();
		if (p) {
			p->iSeconds = edtTimeout.GetInt();
			NotifyChange();
		}
	}

	void onCheck_Timeout(CCtrlCheck *)
	{
		auto *p = getCurData();
		if (p) {
			bool bIsChecked = chkTimeout.GetState();
			p->iSeconds = (bIsChecked) ? 0 : -1;

			edtTimeout.Enable(bIsChecked);
			edtTimeout.SetInt(p->iSeconds);
		}
	}

	void onChange_TextColor(CCtrlBase *)
	{
		auto *p = getCurData();
		if (p)
			p->colorText = m_textColor.GetColor();
	}

	void onChange_BackColor(CCtrlBase *)
	{
		auto *p = getCurData();
		if (p)
			p->colorBack = m_backColor.GetColor();
	}

	void onClick_Preview(CCtrlButton*)
	{
		auto *p = getCurData();
		if (p) {
			POPUPCLASS pc = *p;
			pc.PluginWindowProc = nullptr;

			POPUPDATACLASS d = {};
			d.pszClassName = pc.pszName;
			if (p->flags & PCF_UNICODE) {
				d.szTitle.w = TranslateT("Preview");
				d.szText.w = TranslateT("The quick brown fox jumps over the lazy dog.");
				CallService(MS_POPUP_ADDPOPUPCLASS, (WPARAM)&pc, (LPARAM)&d);
			}
			else {
				d.pszClassName = pc.pszName;
				d.szTitle.a = Translate("Preview");
				d.szText.a = Translate("The quick brown fox jumps over the lazy dog.");
				CallService(MS_POPUP_ADDPOPUPCLASS, (WPARAM)&pc, (LPARAM)&d);
			}
		}
	}
};

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = -790000000;
	odp.szTitle.a = LPGEN("Popups");

	odp.szTab.a = LPGEN("Settings");
	odp.pDialog = new CMainOptDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Classes");
	odp.pDialog = new CClassesOptDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

void InitOptions()
{
	HookEvent(ME_OPT_INITIALISE, OptInit);

	// an icon for preview popups
	hPopupIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
	LoadOptions();
}
