/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
(c) 2004-2006 Sérgio Vieira Rolanski (portet from Borland C++)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

#include <list>

void TfrmMain::Unload()
{
	std::list<TfrmMain*> lst;
	for (auto &it : _HandleMapping)
		lst.push_back(it.second); // we can't delete inside loop.. not MT compatible

	while (!lst.empty()) {
		DestroyWindow(lst.front()->m_hWnd); // deletes class
		lst.pop_front();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK TfrmMain::DlgProc_CaptureTabPage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// main message handling is done inside TfrmMain::DlgTfrmMain
	switch (uMsg) {
	case WM_INITDIALOG:
		switch (lParam) {
		case IDD_UMain_CaptureWindow:
			Static_SetIcon(GetDlgItem(hDlg, ID_imgTarget), GetIcon(ICO_TARGET));
			SetDlgItemText(hDlg, ID_edtCaption, TranslateT("Drag&Drop the target on the desired window."));
			break;
		case IDD_UMain_CaptureDesktop:
			Static_SetIcon(GetDlgItem(hDlg, ID_imgTarget), GetIcon(ICO_MONITOR));
			break;
		case IDD_UMain_CaptureFile:
			Static_SetIcon(GetDlgItem(hDlg, ID_imgTarget), GetIcon(ICO_MAIN));
			break;
		}
		SetFocus(GetDlgItem(hDlg, ID_imgTarget));
		return FALSE;

	case WM_CTLCOLORDLG:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
		return (INT_PTR)GetStockObject(WHITE_BRUSH);

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == ID_btnExplore) { // local file tab
			OPENFILENAME ofn = { sizeof(OPENFILENAME) };
			wchar_t filename[MAX_PATH];
			GetDlgItemText(hDlg, ID_edtSize, filename, _countof(filename));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFilter = L"Images\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tif;*.tiff\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_READONLY;
			if (GetOpenFileName(&ofn)) {
				SetDlgItemText(hDlg, ID_edtSize, filename);
			}
			break;
		}
		SendMessage(GetParent(hDlg), uMsg, wParam, lParam);
		break;
	case WM_NOTIFY:
		SendMessage(GetParent(hDlg), uMsg, wParam, lParam);
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////

TfrmMain::CHandleMapping TfrmMain::_HandleMapping;

INT_PTR CALLBACK TfrmMain::DlgTfrmMain(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORSTATIC) {
		switch (GetWindowLongPtr((HWND)lParam, GWL_ID)) {
		case IDC_HEADERBAR:
			SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
			break;
		default:
			return 0;
		}
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	}

	CHandleMapping::iterator wnd;
	if (msg == WM_INITDIALOG) {
		wnd = _HandleMapping.insert(CHandleMapping::value_type(hWnd, reinterpret_cast<TfrmMain*>(lParam))).first;
		wnd->second->m_hWnd = hWnd;
		wnd->second->wmInitdialog(wParam, lParam);
		return 0;
	}
	wnd = _HandleMapping.find(hWnd);
	if (wnd == _HandleMapping.end())
		return 0;

	switch (msg) {
	case WM_DROPFILES:
		// Drag&Drop of local files
		{
			wchar_t filename[MAX_PATH];
			if (!DragQueryFile((HDROP)wParam, 0, filename, MAX_PATH))
				*filename = '\0';
			DragFinish((HDROP)wParam);
			if (wnd->second->m_hwndTabPage)
				ShowWindow(wnd->second->m_hwndTabPage, SW_HIDE);

			wnd->second->m_opt_tabCapture = 2; // activate file tab
			TabCtrl_SetCurSel(wnd->second->m_hwndTab, wnd->second->m_opt_tabCapture);

			TAB_INFO itab = { TCIF_PARAM };
			TabCtrl_GetItem(wnd->second->m_hwndTab, wnd->second->m_opt_tabCapture, &itab);
			wnd->second->m_hwndTabPage = itab.hwndTabPage;

			ShowWindow(wnd->second->m_hwndTabPage, SW_SHOW);
			SetDlgItemText(wnd->second->m_hwndTabPage, ID_edtSize, filename);
		}
		break;
	case WM_COMMAND:
		wnd->second->wmCommand(wParam, lParam);
		break;
	case WM_CLOSE:
		wnd->second->wmClose(wParam, lParam);
		break;
	case WM_DESTROY:
		delete wnd->second;
		break;
	case WM_NOTIFY:
		wnd->second->wmNotify(wParam, lParam);
		break;
	case WM_TIMER:
		wnd->second->wmTimer(wParam, lParam);
		break;
	case UM_EVENT:
		wnd->second->UMevent(wParam, lParam);
		break;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// WM_INITDIALOG:

int EnumCloudFileServices(const CFSERVICEINFO *serviceInfo, void *param)
{
	HWND hCtrl = (HWND)param;
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, serviceInfo->userName), new UPLOAD_INFO(SS_CLOUDFILE, (void*)serviceInfo->accountName));
	return 0;
}

void TfrmMain::wmInitdialog(WPARAM, LPARAM)
{
	HWND hCtrl;
	// Taskbar and Window icon
	Window_SetIcon_IcoLib(m_hWnd, GetIconHandle(ICO_MAIN));

	wchar_t *pt = mir_wstrdup(Clist_GetContactDisplayName(m_hContact));
	if (pt && (m_hContact != 0)) {
		CMStringW string;
		string.AppendFormat(TranslateT("Send screenshot to %s"), pt);
		SetWindowText(m_hWnd, string);
	}
	mir_free(pt);

	// Headerbar
	SendDlgItemMessage(m_hWnd, IDC_HEADERBAR, WM_SETICON, ICON_BIG, (LPARAM)GetIcon(ICO_MAIN));

	// Timed controls
	CheckDlgButton(m_hWnd, ID_chkTimed, m_opt_chkTimed ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemInt(m_hWnd, ID_edtTimed, (UINT)m_opt_edtTimed, FALSE);
	SendDlgItemMessage(m_hWnd, ID_upTimed, UDM_SETRANGE, 0, (LPARAM)MAKELONG(250, 1));
	chkTimedClick();		// enable disable Timed controls

	// create Image list for tab control
	if (!m_himlTab) {
		m_himlTab = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 1);
		ImageList_AddIcon(m_himlTab, GetIcon(ICO_TARGET));
		ImageList_AddIcon(m_himlTab, GetIcon(ICO_MONITOR));
		ImageList_AddIcon(m_himlTab, GetIconBtn(ICO_BTN_FOLDER));
	}

	// create the tab control.
	{
		m_hwndTab = GetDlgItem(m_hWnd, IDC_CAPTURETAB);
		TabCtrl_SetImageList(m_hwndTab, m_himlTab);
		TabCtrl_SetItemExtra(m_hwndTab, sizeof(TAB_INFO) - sizeof(TCITEMHEADER));
		RECT rcTab;
		TAB_INFO itab;
		itab.hwndMain = m_hWnd;
		itab.hwndTab = m_hwndTab;
		itab.tcih.mask = TCIF_PARAM | TCIF_TEXT | TCIF_IMAGE;

		// Add a tab for each of the three child dialog boxes.
		itab.tcih.pszText = TranslateT("Window");
		itab.tcih.iImage = 0;
		itab.hwndTabPage = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_UMain_CaptureWindow), m_hWnd, DlgProc_CaptureTabPage, IDD_UMain_CaptureWindow);
		TabCtrl_InsertItem(m_hwndTab, 0, &itab);

		// get tab boundaries (required after 1st tab)
		GetClientRect(m_hwndTab, &rcTab);
		MapWindowPoints(m_hwndTab, m_hWnd, (POINT*)&rcTab, 2);
		TabCtrl_AdjustRect(m_hwndTab, 0, &rcTab);
		rcTab.bottom -= rcTab.top; rcTab.right -= rcTab.left;

		SetWindowPos(itab.hwndTabPage, HWND_TOP, rcTab.left, rcTab.top, rcTab.right, rcTab.bottom, 0);
		CheckDlgButton(itab.hwndTabPage, ID_chkIndirectCapture, m_opt_chkIndirectCapture ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(itab.hwndTabPage, ID_chkClientArea, m_opt_chkClientArea ? BST_CHECKED : BST_UNCHECKED);

		itab.tcih.pszText = TranslateT("Desktop");
		itab.tcih.iImage = 1;
		itab.hwndTabPage = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_UMain_CaptureDesktop), m_hWnd, DlgProc_CaptureTabPage, IDD_UMain_CaptureDesktop);
		TabCtrl_InsertItem(m_hwndTab, 1, &itab);
		SetWindowPos(itab.hwndTabPage, HWND_TOP, rcTab.left, rcTab.top, rcTab.right, rcTab.bottom, 0);

		hCtrl = GetDlgItem(itab.hwndTabPage, ID_edtCaption);
		ComboBox_ResetContent(hCtrl);
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("<Entire Desktop>")), 0);
		ComboBox_SetCurSel(hCtrl, 0);
		if (m_MonitorCount > 1) {
			wchar_t tszTemp[120];
			for (size_t mon = 0; mon < m_MonitorCount; ++mon) { // @todo : fix format for non MSVC compilers
				mir_snwprintf(tszTemp, L"%Iu. %s%s",
					mon + 1, TranslateT("Monitor"),
					(m_Monitors[mon].dwFlags & MONITORINFOF_PRIMARY) ? TranslateT(" (primary)") : L""
				);
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, tszTemp), mon + 1);
			}
			ComboBox_SelectItem(hCtrl, m_opt_cboxDesktop);
		}
		PostMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(ID_edtCaption, CBN_SELCHANGE), (LPARAM)hCtrl);

		itab.tcih.pszText = TranslateT("File");
		itab.tcih.iImage = 2;
		itab.hwndTabPage = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_UMain_CaptureFile), m_hWnd, DlgProc_CaptureTabPage, IDD_UMain_CaptureFile);
		TabCtrl_InsertItem(m_hwndTab, 2, &itab);
		SetWindowPos(itab.hwndTabPage, HWND_TOP, rcTab.left, rcTab.top, rcTab.right, rcTab.bottom, 0);

		// select tab and set m_hwndTabPage
		TabCtrl_SetCurSel(m_hwndTab, m_opt_tabCapture);
		itab.tcih.mask = TCIF_PARAM;
		TabCtrl_GetItem(m_hwndTab, m_opt_tabCapture, &itab);
		m_hwndTabPage = itab.hwndTabPage;
		ShowWindow(m_hwndTabPage, SW_SHOW);

		// enable Drag&Drop for local file pane
		typedef BOOL(WINAPI *ChangeWindowMessageFilterEx_t)(HWND hwnd, UINT message, uint32_t action, PCHANGEFILTERSTRUCT pChangeFilterStruct);
		ChangeWindowMessageFilterEx_t pChangeWindowMessageFilterEx;
		pChangeWindowMessageFilterEx = (ChangeWindowMessageFilterEx_t)GetProcAddress(GetModuleHandleA("user32"), "ChangeWindowMessageFilterEx");
		if (pChangeWindowMessageFilterEx) { // Win7+, UAC fix
			pChangeWindowMessageFilterEx(m_hWnd, WM_DROPFILES, MSGFLT_ALLOW, nullptr);
			pChangeWindowMessageFilterEx(m_hWnd, WM_COPYDATA, MSGFLT_ALLOW, nullptr);
			pChangeWindowMessageFilterEx(m_hWnd, 0x0049/*WM_COPYGLOBALDATA*/, MSGFLT_ALLOW, nullptr);
		}
		DragAcceptFiles(m_hWnd, 1);
	}

	// init Format combo box
	{
		hCtrl = GetDlgItem(m_hWnd, ID_cboxFormat);
		ComboBox_ResetContent(hCtrl);
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, L"PNG"), 0);
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, L"JPG"), 1);
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, L"BMP"), 2);
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, L"TIF"), 3);
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, L"GIF"), 4);
		ComboBox_SelectItem(hCtrl, m_opt_cboxFormat);
	}

	// init SendBy combo box
	UPLOAD_INFO *pDefault = nullptr;
	{
		hCtrl = GetDlgItem(m_hWnd, ID_cboxSendBy);
		ComboBox_ResetContent(hCtrl);
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("<Only save>")), new UPLOAD_INFO(SS_JUSTSAVE));
		if (m_hContact) {
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("File Transfer")), new UPLOAD_INFO(SS_FILESEND));
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("E-mail")), new UPLOAD_INFO(SS_EMAIL));
			if (g_myGlobals.PluginHTTPExist) {
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, L"HTTP Server"), new UPLOAD_INFO(SS_HTTPSERVER));
			}
			else if (m_opt_cboxSendBy == SS_HTTPSERVER) {
				m_opt_cboxSendBy = SS_IMAGESHACK;
			}
			if (g_myGlobals.PluginFTPExist) {
				ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("FTP File")), new UPLOAD_INFO(SS_FTPFILE));
			}
			else if (m_opt_cboxSendBy == SS_FTPFILE) {
				m_opt_cboxSendBy = SS_IMAGESHACK;
			}
		}
		else if (m_opt_cboxSendBy == SS_FILESEND || m_opt_cboxSendBy == SS_EMAIL || m_opt_cboxSendBy == SS_HTTPSERVER || m_opt_cboxSendBy == SS_FTPFILE) {
			m_opt_cboxSendBy = SS_IMAGESHACK;
		}
		if (g_myGlobals.PluginCloudFileExist) {
			CallService(MS_CLOUDFILE_ENUMSERVICES, (WPARAM)EnumCloudFileServices, (LPARAM)hCtrl);
		}
		else if (m_opt_cboxSendBy == SS_CLOUDFILE) {
			m_opt_cboxSendBy = SS_IMAGESHACK;
		}
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, L"ImageShack"), new UPLOAD_INFO(SS_IMAGESHACK));
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Upload Pie (30m)")), new UPLOAD_INFO(SS_UPLOADPIE, (void*)1));
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Upload Pie (1d)")), new UPLOAD_INFO(SS_UPLOADPIE, (void*)4));
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Upload Pie (1w)")), new UPLOAD_INFO(SS_UPLOADPIE, (void*)5));
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, L"Imgur"), new UPLOAD_INFO(SS_IMGUR));

		for (int i = 0; i < ComboBox_GetCount(hCtrl); i++) {
			UPLOAD_INFO *p = (UPLOAD_INFO*)ComboBox_GetItemData(hCtrl, i);
			if (p && p->sendBy == m_opt_cboxSendBy) {
				pDefault = p;
				ComboBox_SetCurSel(hCtrl, i);
				break;
			}
		}
	}

	// init footer options
	CheckDlgButton(m_hWnd, ID_chkOpenAgain, m_opt_chkOpenAgain ? BST_CHECKED : BST_UNCHECKED);

	if (hCtrl = GetDlgItem(m_hWnd, ID_btnExplore)) {
		SendDlgItemMessage(m_hWnd, ID_btnExplore, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Open Folder"), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(ICO_BTN_FOLDER);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? L"" : L"...");
	}

	if (hCtrl = GetDlgItem(m_hWnd, ID_chkDesc)) {
		SendDlgItemMessage(m_hWnd, ID_chkDesc, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Fill description textbox."), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(m_opt_btnDesc ? ICO_BTN_DESCON : ICO_BTN_DESC);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? L"" : L"D");
		SendMessage(hCtrl, BM_SETCHECK, m_opt_btnDesc ? BST_CHECKED : BST_UNCHECKED, NULL);
	}

	if (hCtrl = GetDlgItem(m_hWnd, ID_chkDeleteAfterSend)) {
		SendDlgItemMessage(m_hWnd, ID_chkDeleteAfterSend, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Delete after send"), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(m_opt_btnDeleteAfterSend ? ICO_BTN_DELON : ICO_BTN_DEL);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? L"" : L"X");
		SendMessage(hCtrl, BM_SETCHECK, m_opt_btnDeleteAfterSend ? BST_CHECKED : BST_UNCHECKED, NULL);
	}

	if (hCtrl = GetDlgItem(m_hWnd, ID_chkEditor)) {
		SendDlgItemMessage(m_hWnd, ID_chkEditor, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Open editor before sending"), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(m_opt_chkEditor ? ICO_BTN_EDITON : ICO_BTN_EDIT);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? L"" : L"E");
		SendMessage(hCtrl, BM_SETCHECK, m_opt_chkEditor ? BST_CHECKED : BST_UNCHECKED, NULL);
	}

	if (hCtrl = GetDlgItem(m_hWnd, ID_btnCapture)) {
		SendMessage(hCtrl, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Capture"), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(ICO_BTN_OK);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, TranslateT("&Capture"));
		SendMessage(hCtrl, BUTTONSETDEFAULT, 1, NULL);
	}
	cboxSendByChange((pDefault) ? pDefault->param : nullptr); // enable disable controls

	TranslateDialogDefault(m_hWnd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// WM_COMMAND:

void TfrmMain::wmCommand(WPARAM wParam, LPARAM lParam)
{
	HICON hIcon;

	int IDControl = LOWORD(wParam);
	switch (HIWORD(wParam)) {
	case BN_CLICKED: // Button controls
		switch (IDControl) {
		case IDCANCEL: // ESC pressed
			this->Close();
			break;
		case ID_chkTimed:
			m_opt_chkTimed = (uint8_t)Button_GetCheck((HWND)lParam);
			TfrmMain::chkTimedClick();
			break;
		case ID_chkIndirectCapture:
			m_opt_chkIndirectCapture = (uint8_t)Button_GetCheck((HWND)lParam);
			break;
		case ID_chkClientArea:
			m_opt_chkClientArea = (uint8_t)Button_GetCheck((HWND)lParam);
			if (m_hTargetWindow)
				edtSizeUpdate(m_hTargetWindow, m_opt_chkClientArea, GetParent((HWND)lParam), ID_edtSize);
			break;
		case ID_imgTarget:
			if (m_opt_tabCapture != 0) break;
			m_hLastWin = nullptr;
			SetTimer(m_hWnd, ID_imgTarget, BUTTON_POLLDELAY, nullptr);
			break;
		case ID_btnExplore:
			TfrmMain::btnExploreClick();
			break;
		case ID_chkDesc:
			m_opt_btnDesc = !m_opt_btnDesc;
			hIcon = GetIconBtn(m_opt_btnDesc ? ICO_BTN_DESCON : ICO_BTN_DESC);
			SendMessage((HWND)lParam, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
			break;
		case ID_chkDeleteAfterSend:
			m_opt_btnDeleteAfterSend = !m_opt_btnDeleteAfterSend;
			hIcon = GetIconBtn(m_opt_btnDeleteAfterSend ? ICO_BTN_DELON : ICO_BTN_DEL);
			SendMessage((HWND)lParam, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
			if (m_cSend) m_cSend->m_bDeleteAfterSend = m_opt_btnDeleteAfterSend;
			break;
		case ID_chkEditor:
			m_opt_chkEditor = !m_opt_chkEditor;
			hIcon = GetIconBtn(m_opt_chkEditor ? ICO_BTN_EDITON : ICO_BTN_EDIT);
			SendMessage((HWND)lParam, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
			break;
		case ID_chkOpenAgain:
			m_opt_chkOpenAgain = Button_GetCheck((HWND)lParam);
			break;
		case ID_btnCapture:
			TfrmMain::btnCaptureClick();
			break;
		}
		break;

	case CBN_SELCHANGE: // ComboBox controls
		switch (IDControl) { // lParam = Handle to the control
		case ID_cboxFormat:  // not finish
			m_opt_cboxFormat = (uint8_t)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
			break;
		case ID_cboxSendBy:
			{
				UPLOAD_INFO *upload = (UPLOAD_INFO*)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
				m_opt_cboxSendBy = upload->sendBy;
				cboxSendByChange(upload->param);
			}
			break;

		case ID_edtCaption: // cboxDesktopChange
			m_opt_cboxDesktop = (uint8_t)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
			m_hTargetWindow = nullptr;
			if (m_opt_cboxDesktop > 0) {
				edtSizeUpdate(m_Monitors[m_opt_cboxDesktop - 1].rcMonitor, GetParent((HWND)lParam), ID_edtSize);
			}
			else {
				edtSizeUpdate(m_VirtualScreen, GetParent((HWND)lParam), ID_edtSize);
			}
			break;
		}
		break;

	case EN_CHANGE: // Edit controls
		switch (IDControl) { // lParam = Handle to the control
		case ID_edtQuality:
			m_opt_edtQuality = (uint8_t)GetDlgItemInt(m_hWnd, ID_edtQuality, nullptr, FALSE);
			break;
		case ID_edtTimed:
			m_opt_edtTimed = (uint8_t)GetDlgItemInt(m_hWnd, ID_edtTimed, nullptr, FALSE);
			break;
		}
		break;
	}
}

// WM_CLOSE:
void TfrmMain::wmClose(WPARAM, LPARAM)
{
	HWND hCtrl = GetDlgItem(m_hWnd, ID_cboxSendBy);
	size_t count = ComboBox_GetCount(hCtrl);
	for (size_t i = 0; i < count; i++) {
		UPLOAD_INFO *ui = (UPLOAD_INFO*)ComboBox_GetItemData(hCtrl, i);
		delete ui;
	}
	DestroyWindow(m_hWnd);
	return;
}

// WM_TIMER:
const int g_iTargetBorder = 7;
void TfrmMain::SetTargetWindow(HWND hwnd)
{
	if (!hwnd) {
		POINT point; GetCursorPos(&point);
		hwnd = WindowFromPoint(point);
		for (HWND hTMP; (hTMP = GetParent(hwnd)); hwnd = hTMP)
			;
	}
	m_hTargetWindow = hwnd;
	int len = GetWindowTextLength(m_hTargetWindow) + 1;
	wchar_t *lpTitle;
	if (len > 1) {
		lpTitle = (wchar_t*)mir_alloc(len*sizeof(wchar_t));
		GetWindowText(m_hTargetWindow, lpTitle, len);
	}
	else { // no WindowText present, use WindowClass
		lpTitle = (wchar_t*)mir_alloc(64 * sizeof(wchar_t));
		RealGetWindowClass(m_hTargetWindow, lpTitle, 64);
	}
	SetDlgItemText(m_hwndTabPage, ID_edtCaption, lpTitle);
	mir_free(lpTitle);
	edtSizeUpdate(m_hTargetWindow, m_opt_chkClientArea, m_hwndTabPage, ID_edtSize);
}

void TfrmMain::wmTimer(WPARAM wParam, LPARAM)
{
	if (wParam == ID_imgTarget) { // Timer for Target selector
		static int primarymouse;
		if (!m_hTargetHighlighter) {
			primarymouse = GetSystemMetrics(SM_SWAPBUTTON) ? VK_RBUTTON : VK_LBUTTON;
			m_hTargetHighlighter = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW, (wchar_t*)g_clsTargetHighlighter, nullptr, WS_POPUP, 0, 0, 0, 0, nullptr, nullptr, g_plugin.getInst(), nullptr);
			if (!m_hTargetHighlighter) return;
			SetLayeredWindowAttributes(m_hTargetHighlighter, 0, 123, LWA_ALPHA);
			SetSystemCursor(CopyCursor(GetIcon(ICO_TARGET)), OCR_IBEAM); // text cursor
			SetSystemCursor(CopyCursor(GetIcon(ICO_TARGET)), OCR_NORMAL);
			SetActiveWindow(m_hTargetHighlighter); // activate highlighter to fix focus problems with UAC (unelevated GetAsyncKeyState() fails if an elevated app got focus)
			Hide();
		}
		if (!(GetAsyncKeyState(primarymouse) & 0x8000)) {
			KillTimer(m_hWnd, ID_imgTarget);
			SystemParametersInfo(SPI_SETCURSORS, 0, nullptr, 0);
			DestroyWindow(m_hTargetHighlighter), m_hTargetHighlighter = nullptr;
			SetTargetWindow(m_hLastWin);
			Show();
			return;
		}
		POINT point; GetCursorPos(&point);
		HWND hwnd = WindowFromPoint(point);
		if (!((GetAsyncKeyState(VK_SHIFT) | GetAsyncKeyState(VK_MENU)) & 0x8000))
			for (HWND hTMP; (hTMP = GetAncestor(hwnd, GA_PARENT)) && IsChild(hTMP, hwnd); hwnd = hTMP);
		else {
			ScreenToClient(hwnd, &point);
			HWND hTMP; if ((hTMP = RealChildWindowFromPoint(hwnd, point)))
				hwnd = hTMP;
		}
		if (hwnd != m_hLastWin) {
			m_hLastWin = hwnd;
			RECT rect;
			if (m_opt_chkClientArea) {
				GetClientRect(hwnd, &rect);
				ClientToScreen(hwnd, (POINT*)&rect);
				rect.right = rect.left + rect.right;
				rect.bottom = rect.top + rect.bottom;
			}
			else
				GetWindowRect(hwnd, &rect);
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;
			if (g_iTargetBorder) {
				SetWindowPos(m_hTargetHighlighter, nullptr, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);
				if (width > g_iTargetBorder * 2 && height > g_iTargetBorder * 2) {
					HRGN hRegnNew = CreateRectRgn(0, 0, width, height);
					HRGN hRgnHole = CreateRectRgn(g_iTargetBorder, g_iTargetBorder, width - g_iTargetBorder, height - g_iTargetBorder);
					CombineRgn(hRegnNew, hRegnNew, hRgnHole, RGN_XOR);
					DeleteObject(hRgnHole);
					SetWindowRgn(m_hTargetHighlighter, hRegnNew, FALSE); // cleans up hRegnNew
				}
				else SetWindowRgn(m_hTargetHighlighter, nullptr, FALSE);
			}
			SetWindowPos(m_hTargetHighlighter, HWND_TOPMOST, rect.left, rect.top, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE);
		}
		return;
	}
	if (wParam == ID_chkTimed) { // Timer for Screenshot
#ifdef _DEBUG
		OutputDebugStringA("SS Bitmap Timer Start\r\n");
#endif
		if (!m_bCapture) { // only start once
			if (m_Screenshot) {
				FreeImage_Unload(m_Screenshot);
				m_Screenshot = nullptr;
			}
			m_bCapture = true;
			switch (m_opt_tabCapture) {
			case 0:
				m_Screenshot = CaptureWindow(m_hTargetWindow, m_opt_chkClientArea, m_opt_chkIndirectCapture);
				break;
			case 1:
				m_Screenshot = CaptureMonitor((m_opt_cboxDesktop > 0) ? m_Monitors[m_opt_cboxDesktop - 1].szDevice : nullptr);
				break;
			case 2: // edge case, existing local file
				break;
#ifdef _DEBUG
			default:
				OutputDebugStringA("SS Bitmap Timer Stop (no tabCapture)\r\n");
#endif
			}
			m_bCapture = false;
			if (m_Screenshot || m_opt_tabCapture == 2) { // @note : test without "if"
				KillTimer(m_hWnd, ID_chkTimed);
#ifdef _DEBUG
				OutputDebugStringA("SS Bitmap Timer Stop (CaptureDone)\r\n");
#endif
				SendMessage(m_hWnd, UM_EVENT, 0, (LPARAM)EVT_CaptureDone);
			}
		}
	}
}

// WM_NOTIFY:
void TfrmMain::wmNotify(WPARAM, LPARAM lParam)
{
	switch (((LPNMHDR)lParam)->idFrom) {
	case IDC_CAPTURETAB:
		// HWND hwndFrom; = member is handle to the tab control
		// UINT_PTR idFrom; = member is the child window identifier of the tab control.
		// UINT code; = member is TCN_SELCHANGE
		switch (((LPNMHDR)lParam)->code) {
		case TCN_SELCHANGING:
			if (m_hwndTabPage) {
				ShowWindow(m_hwndTabPage, SW_HIDE);
				m_hwndTabPage = nullptr;
			}
			break;

		case TCN_SELCHANGE:
			{
				TAB_INFO itab = { TCIF_PARAM };
				m_opt_tabCapture = TabCtrl_GetCurSel(m_hwndTab);
				TabCtrl_GetItem(m_hwndTab, m_opt_tabCapture, &itab);
				m_hwndTabPage = itab.hwndTabPage;
			}
			ShowWindow(m_hwndTabPage, SW_SHOW);
			break;
		}
		break;
	}
}

// UM_EVENT:
void TfrmMain::UMevent(WPARAM, LPARAM lParam)
{
	// HWND hWnd = (HWND)wParam;
	switch (lParam) {
	case EVT_CaptureDone:
		if (!m_Screenshot && m_opt_tabCapture != 2) {
			wchar_t *err = TranslateT("Couldn't take a screenshot");
			MessageBox(nullptr, err, ERROR_TITLE, MB_OK | MB_ICONWARNING);
			Show();
			return;
		}
		FormClose();
		break;

	case EVT_SendFileDone:
		break;

	case EVT_CheckOpenAgain:
		if (m_opt_chkOpenAgain) {
			if (m_Screenshot) {
				FreeImage_Unload(m_Screenshot);
				m_Screenshot = nullptr;
			}
			Show();
		}
		else {// Saving Options and close
			SaveOptions();
			Close();
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Standard konstruktor/destruktor

TfrmMain::TfrmMain()
{
	/* m_opt_XXX */
	m_bOnExitSave = TRUE;

	m_hWnd = nullptr;
	m_hContact = NULL;
	m_hTargetWindow = m_hLastWin = nullptr;
	m_hTargetHighlighter = nullptr;
	m_FDestFolder = m_pszFile = nullptr;
	m_Screenshot = nullptr;
	/* m_AlphaColor */
	m_cSend = nullptr;

	m_Monitors = nullptr;
	m_MonitorCount = MonitorInfoEnum(m_Monitors, m_VirtualScreen);
	/* m_opt_XXX */ LoadOptions();
	m_bCapture = false;
	/* m_hwndTab,m_hwndTabPage */
	m_himlTab = nullptr;
}

TfrmMain::~TfrmMain()
{
	_HandleMapping.erase(m_hWnd);
	mir_free(m_pszFile);
	mir_free(m_FDestFolder);
	mir_free(m_Monitors);
	if (m_Screenshot) FreeImage_Unload(m_Screenshot);
	if (m_cSend) delete m_cSend;
	if (m_hTargetHighlighter) {
		DestroyWindow(m_hTargetHighlighter), m_hTargetHighlighter = nullptr;
		SystemParametersInfo(SPI_SETCURSORS, 0, nullptr, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load / Saving options from miranda's database

void TfrmMain::LoadOptions(void)
{
	uint32_t rgb = g_plugin.getDword("AlphaColor", 16777215);
	m_AlphaColor.rgbRed = GetRValue(rgb);
	m_AlphaColor.rgbGreen = GetGValue(rgb);
	m_AlphaColor.rgbBlue = GetBValue(rgb);
	m_AlphaColor.rgbReserved = 0;

	m_opt_edtQuality = g_plugin.getByte("JpegQuality", 75);

	m_opt_tabCapture = g_plugin.getByte("Capture", 0);
	m_opt_chkIndirectCapture = g_plugin.getByte("IndirectCapture", 0);
	m_opt_chkClientArea = g_plugin.getByte("ClientArea", 0);
	m_opt_cboxDesktop = g_plugin.getByte("Desktop", 0);

	m_opt_chkTimed = g_plugin.getByte("TimedCap", 0);
	m_opt_edtTimed = g_plugin.getByte("CapTime", 3);
	m_opt_cboxFormat = g_plugin.getByte("OutputFormat", 0);
	m_opt_cboxSendBy = g_plugin.getByte("SendBy", 0);

	m_opt_btnDesc = g_plugin.getByte("AutoDescription", 1);
	m_opt_btnDeleteAfterSend = g_plugin.getByte("DelAfterSend", 1) != 0;
	m_opt_chkEditor = g_plugin.getByte("Preview", 0);
	m_opt_chkOpenAgain = g_plugin.getByte("OpenAgain", 0);
}

void TfrmMain::SaveOptions(void)
{
	if (m_bOnExitSave) {
		g_plugin.setDword("AlphaColor",
			(uint32_t)RGB(m_AlphaColor.rgbRed, m_AlphaColor.rgbGreen, m_AlphaColor.rgbBlue));

		g_plugin.setByte("JpegQuality", m_opt_edtQuality);

		g_plugin.setByte("Capture", m_opt_tabCapture);
		g_plugin.setByte("IndirectCapture", m_opt_chkIndirectCapture);
		g_plugin.setByte("ClientArea", m_opt_chkClientArea);
		g_plugin.setByte("Desktop", m_opt_cboxDesktop);

		g_plugin.setByte("TimedCap", m_opt_chkTimed);
		g_plugin.setByte("CapTime", m_opt_edtTimed);
		g_plugin.setByte("OutputFormat", m_opt_cboxFormat);
		g_plugin.setByte("SendBy", m_opt_cboxSendBy);

		g_plugin.setByte("AutoDescription", m_opt_btnDesc);
		g_plugin.setByte("DelAfterSend", m_opt_btnDeleteAfterSend);
		g_plugin.setByte("Preview", m_opt_chkEditor);
		g_plugin.setByte("OpenAgain", m_opt_chkOpenAgain);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

void TfrmMain::Init(wchar_t *DestFolder, MCONTACT Contact)
{
	m_FDestFolder = mir_wstrdup(DestFolder);
	m_hContact = Contact;

	// create window
	m_hWnd = CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_UMainForm), nullptr, DlgTfrmMain, (LPARAM)this);

	// register object
	_HandleMapping.insert(CHandleMapping::value_type(m_hWnd, this));

	// check Contact
	if (m_cSend)
		m_cSend->SetContact(Contact);
}

/////////////////////////////////////////////////////////////////////////////////////////

void TfrmMain::btnCaptureClick()
{
	if (m_opt_tabCapture == 1) m_hTargetWindow = GetDesktopWindow();
	else if (m_opt_tabCapture == 2) {
		wchar_t filename[MAX_PATH];
		GetDlgItemText(m_hwndTabPage, ID_edtSize, filename, _countof(filename));
		FILE *fp = _wfopen(filename, L"rb");
		if (!fp) {
			wchar_t *err = TranslateT("Select a file");
			MessageBox(m_hWnd, err, ERROR_TITLE, MB_OK | MB_ICONWARNING);
			return;
		}
		fclose(fp);
		mir_free(m_pszFile); m_pszFile = mir_wstrdup(filename);
	}
	else if (!m_hTargetWindow) {
		wchar_t *err = TranslateT("Select a target window.");
		MessageBox(m_hWnd, err, ERROR_TITLE, MB_OK | MB_ICONWARNING);
		return;
	}
	TfrmMain::Hide();

	if (m_opt_chkTimed) {
		SetTimer(m_hWnd, ID_chkTimed, m_opt_edtTimed ? m_opt_edtTimed * 1000 : 500, nullptr); // calls EVT_CaptureDone
		return;
	}
	if (m_opt_tabCapture == 1) { // desktop needs always time to update from TfrmMain::Hide()
		SetTimer(m_hWnd, ID_chkTimed, 500, nullptr); // calls EVT_CaptureDone
		return;
	}
	if (m_opt_tabCapture != 2) {
		m_Screenshot = CaptureWindow(m_hTargetWindow, m_opt_chkClientArea, m_opt_chkIndirectCapture);
	}
	SendMessage(m_hWnd, UM_EVENT, 0, (LPARAM)EVT_CaptureDone);
}

/////////////////////////////////////////////////////////////////////////////////////////

void TfrmMain::chkTimedClick()
{
	Button_Enable(GetDlgItem(m_hWnd, ID_edtTimedLabel), (BOOL)m_opt_chkTimed);
	Button_Enable(GetDlgItem(m_hWnd, ID_edtTimed), (BOOL)m_opt_chkTimed);
	Button_Enable(GetDlgItem(m_hWnd, ID_upTimed), (BOOL)m_opt_chkTimed);
}

/////////////////////////////////////////////////////////////////////////////////////////

void TfrmMain::cboxSendByChange(void *param)
{
	BOOL bState;
	HICON hIcon;
	uint8_t itemFlag = SS_DLG_DESCRIPTION;
	if (m_cSend)
		delete m_cSend;
	switch (m_opt_cboxSendBy) {
	case SS_FILESEND: // "File Transfer"
		m_cSend = new CSendFile(m_hWnd, m_hContact, true);
		break;
	case SS_EMAIL: // "E-mail"
		m_cSend = new CSendEmail(m_hWnd, m_hContact, true);
		break;
	case SS_HTTPSERVER: // "HTTP Server"
		m_cSend = new CSendHTTPServer(m_hWnd, m_hContact, true);
		break;
	case SS_FTPFILE: // "FTP File"
		m_cSend = new CSendFTPFile(m_hWnd, m_hContact, true);
		break;
	case SS_CLOUDFILE: // "CloudFile"
		m_cSend = new CSendCloudFile(m_hWnd, m_hContact, false, (char*)param);
		break;
	case SS_IMAGESHACK: // "ImageShack"
		m_cSend = new CSendHost_ImageShack(m_hWnd, m_hContact, true);
		break;
	case SS_UPLOADPIE: // "Upload Pie"
		m_cSend = new CSendHost_UploadPie(m_hWnd, m_hContact, true, (INT_PTR)param);
		break;
	case SS_IMGUR:
		m_cSend = new CSendHost_Imgur(m_hWnd, m_hContact, true);
		break;
	default:
		m_cSend = nullptr;
		break;
	}
	if (m_cSend) {
		itemFlag = m_cSend->GetEnableItem();
		m_cSend->m_bDeleteAfterSend = m_opt_btnDeleteAfterSend;
	}
	bState = (itemFlag & SS_DLG_DELETEAFTERSSEND);
	hIcon = GetIconBtn(m_opt_btnDeleteAfterSend ? ICO_BTN_DELON : ICO_BTN_DEL);
	SendDlgItemMessage(m_hWnd, ID_chkDeleteAfterSend, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(bState ? hIcon : nullptr));
	Button_Enable(GetDlgItem(m_hWnd, ID_chkDeleteAfterSend), bState);

	bState = (itemFlag & SS_DLG_DESCRIPTION);
	hIcon = GetIconBtn(m_opt_btnDesc ? ICO_BTN_DESCON : ICO_BTN_DESC);
	SendDlgItemMessage(m_hWnd, ID_chkDesc, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(bState ? hIcon : nullptr));
	Button_Enable(GetDlgItem(m_hWnd, ID_chkDesc), bState);
}

/////////////////////////////////////////////////////////////////////////////////////////

void TfrmMain::btnExploreClick()
{
	if (m_FDestFolder)
		ShellExecute(nullptr, L"explore", m_FDestFolder, nullptr, nullptr, SW_SHOW);
}

/////////////////////////////////////////////////////////////////////////////////////////

void TfrmMain::edtSizeUpdate(HWND hWnd, BOOL ClientArea, HWND hTarget, UINT Ctrl)
{
	// get window dimensions
	RECT rect = { 0 };
	RECT cliRect = { 0 };
	wchar_t B[33], H[16];
	GetWindowRect(hWnd, &rect);
	if (ClientArea) {
		POINT pt = { 0 };
		GetClientRect(hWnd, &cliRect);
		pt.x = cliRect.left;
		pt.y = cliRect.top;
		ClientToScreen(hWnd, &pt);
		pt.x = pt.x - rect.left; // offset x for client area
		pt.y = pt.y - rect.top;  // offset y for client area
		rect = cliRect;
	}

	_itow(rect.right - rect.left, B, 10);
	_itow(rect.bottom - rect.top, H, 10);
	mir_wstrncat(B, L"x", _countof(B) - mir_wstrlen(B));
	mir_wstrncat(B, H, _countof(B) - mir_wstrlen(B));
	SetDlgItemText(hTarget, Ctrl, B);
}

void TfrmMain::edtSizeUpdate(RECT rect, HWND hTarget, UINT Ctrl)
{
	wchar_t B[33], H[16];
	_itow(ABS(rect.right - rect.left), B, 10);
	_itow(ABS(rect.bottom - rect.top), H, 10);
	mir_wstrncat(B, L"x", _countof(B) - mir_wstrlen(B));
	mir_wstrncat(B, H, _countof(B) - mir_wstrlen(B));
	SetDlgItemText(hTarget, Ctrl, B);
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR TfrmMain::SaveScreenshot(FIBITMAP *dib)
{
	if (!dib)
		return 1;

	// generate file name
	unsigned FileNumber = g_plugin.getDword("FileNumber", 0) + 1;
	if (FileNumber > 99999)
		FileNumber = 1;

	CMStringW wszFileName(m_FDestFolder);
	if (wszFileName.Right(1) != L"\\")
		wszFileName.Append(L"\\");
	wszFileName.AppendFormat(L"shot%.5u", FileNumber);

	// generate a description according to the screenshot
	wchar_t winText[1024];
	GetDlgItemText(m_hwndTabPage, ID_edtCaption, winText, _countof(winText));

	CMStringW wszFileDesc;
	if (m_opt_tabCapture)
		wszFileDesc.Format(TranslateT("Screenshot of \"%s\""), winText);
	else {
		if (m_opt_chkClientArea)
			wszFileDesc.Format(TranslateT("Screenshot for client area of \"%s\" window"), winText);
		else
			wszFileDesc.Format(TranslateT("Screenshot of \"%s\" window"), winText);
	}

	// convert to 32Bits (make sure it is 32bit)
	FIBITMAP *dib_new = FreeImage_ConvertTo32Bits(dib);
	FreeImage_SetTransparent(dib_new, TRUE);

	BOOL ret = FALSE;
	HWND hwndCombo = GetDlgItem(m_hWnd, ID_cboxFormat);
	switch (ComboBox_GetItemData(hwndCombo, ComboBox_GetCurSel(hwndCombo))) {
	case 0: // PNG
		wszFileName.Append(L".png");
		ret = FreeImage_SaveU(FIF_PNG, dib_new, wszFileName, 0);
		break;

	case 1: // JPG
		wszFileName.Append(L".jpg");
		{
			FIBITMAP *dib32 = FreeImage_Composite(dib_new, FALSE, &m_AlphaColor, nullptr);
			FIBITMAP *dib24 = FreeImage_ConvertTo24Bits(dib32);
			FreeImage_Unload(dib32);
			ret = FreeImage_SaveU(FIF_JPEG, dib24, wszFileName, 0);
			FreeImage_Unload(dib24);
		}
		break;

	case 2: // BMP
		wszFileName.Append(L".bmp");
		{
			FIBITMAP *dib32 = FreeImage_Composite(dib_new, FALSE, &m_AlphaColor, nullptr);
			FIBITMAP *dib24 = FreeImage_ConvertTo24Bits(dib32);
			FreeImage_Unload(dib32);
			ret = FreeImage_SaveU(FIF_BMP, dib24, wszFileName, 0);
			FreeImage_Unload(dib24);
		}
		break;

	case 3: // TIFF (miranda freeimage interface do not support save tiff, we udse GDI+)
		wszFileName.Append(L".tif");
		{
			FIBITMAP *dib32 = FreeImage_Composite(dib_new, FALSE, &m_AlphaColor, nullptr);
			FIBITMAP *dib24 = FreeImage_ConvertTo24Bits(dib32);
			FreeImage_Unload(dib32);

			HBITMAP hBmp = FreeImage_CreateHBITMAPFromDIB(dib24);
			FreeImage_Unload(dib24);
			SaveTIF(hBmp, wszFileName);
			DeleteObject(hBmp);
		}
		ret = TRUE;
		break;

	case 4: // GIF
		wszFileName.Append(L".gif");
		{
			HBITMAP hBmp = FreeImage_CreateHBITMAPFromDIB(dib_new);
			SaveGIF(hBmp, wszFileName);
			DeleteObject(hBmp);
		}
		ret = TRUE;
		break;
	}

	FreeImage_Unload(dib_new);

	if (!ret)
		return 1;

	g_plugin.setDword("FileNumber", FileNumber);
	replaceStrW(m_pszFile, wszFileName);

	if (!IsWindowEnabled(GetDlgItem(m_hWnd, ID_chkDesc)) || !m_opt_btnDesc)
		wszFileDesc.Empty();

	if (m_cSend) {
		m_cSend->SetFile(m_pszFile);
		m_cSend->SetDescription(wszFileDesc);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void TfrmMain::FormClose()
{
	bool bCanDelete = m_opt_btnDeleteAfterSend;
	if (m_opt_tabCapture == 2) { // existing file
		wchar_t description[1024];
		GetDlgItemText(m_hwndTabPage, ID_edtCaption, description, _countof(description));
		if (!IsWindowEnabled(GetDlgItem(m_hWnd, ID_chkDesc)) || !m_opt_btnDesc)
			*description = '\0';
		if (m_cSend) {
			m_cSend->m_bDeleteAfterSend = false; // well... guess it's better to not delete existing files for now...
			m_cSend->SetFile(m_pszFile);
			m_cSend->SetDescription(description);
		}
		bCanDelete = false;
	}
	else if (SaveScreenshot(m_Screenshot)) { // Saving the screenshot
		Show(); // Error from SaveScreenshot
		return;
	}

	bool send = true;
	if (m_opt_chkEditor) {
		SHELLEXECUTEINFO shex = { sizeof(SHELLEXECUTEINFO) };
		shex.fMask = SEE_MASK_NOCLOSEPROCESS;
		shex.lpVerb = L"edit";
		shex.lpFile = m_pszFile;
		shex.nShow = SW_SHOWNORMAL;
		ShellExecuteEx(&shex);
		if (shex.hProcess) {
			uint32_t res;
			MSG msg;
			do {
				// wait for editor exit or messages/input
				res = MsgWaitForMultipleObjects(1, &shex.hProcess, 0, INFINITE, QS_ALLINPUT);
				while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
					if (msg.message == WM_QUIT) {
						res = WAIT_OBJECT_0;
						PostMessage(nullptr, WM_QUIT, 0, 0); // forward for outer message loops
						break;
					}

					// process dialog messages (of unknown dialogs)
					HWND hwndDlgModeless = GetActiveWindow();
					if (hwndDlgModeless != nullptr && IsDialogMessage(hwndDlgModeless, &msg)) /* Wine fix. */
						continue;

					// process messages
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			} while (res == WAIT_OBJECT_0 + 1);
			CloseHandle(shex.hProcess);
		}
		if (MessageBox(m_hWnd, TranslateT("Send screenshot?"), L"SendSS", MB_YESNO | MB_ICONQUESTION | MB_SYSTEMMODAL) != IDYES)
			send = false;
	}

	if (send && m_cSend && m_pszFile) {
		if (!m_cSend->Send()) // not sent now, class deletes itself later
			m_cSend = nullptr;
		cboxSendByChange(nullptr);
	}
	else if (!send && bCanDelete)
		DeleteFile(m_pszFile);

	SendMessage(m_hWnd, UM_EVENT, 0, (LPARAM)EVT_CheckOpenAgain);
}
