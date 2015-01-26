/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (с) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#include "global.h"

#include <list>
void TfrmMain::Unload(){
	std::list<TfrmMain*> lst;
	for(CHandleMapping::iterator iter=_HandleMapping.begin(); iter!=_HandleMapping.end(); ++iter){
		lst.push_back(iter->second);//we can't delete inside loop.. not MT compatible
	}
	while(!lst.empty()){
		DestroyWindow(lst.front()->m_hWnd);//deletes class
		lst.pop_front();
	}
}

//---------------------------------------------------------------------------
INT_PTR CALLBACK TfrmMain::DlgProc_CaptureTabPage(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
// main message handling is done inside TfrmMain::DlgTfrmMain
	switch (uMsg) {
	case WM_INITDIALOG:
		switch(lParam){
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
		SetTextColor((HDC)wParam,GetSysColor(COLOR_WINDOWTEXT));
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	case WM_COMMAND:
		if(HIWORD(wParam)==BN_CLICKED && LOWORD(wParam)==ID_btnExplore){ /// local file tab
			OPENFILENAME ofn={sizeof(OPENFILENAME)};
			TCHAR filename[MAX_PATH];
			GetDlgItemText(hDlg,ID_edtSize,filename,SIZEOF(filename));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFilter = _T("Images\0*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tif;*.tiff\0");
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = filename;
			ofn.nMaxFile = MAX_PATH;
//			ofn.lpstrInitialDir = m_FDestFolder;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_READONLY;
			if(GetOpenFileName(&ofn)){
				SetDlgItemText(hDlg,ID_edtSize,filename);
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

//---------------------------------------------------------------------------

TfrmMain::CHandleMapping TfrmMain::_HandleMapping;

INT_PTR CALLBACK TfrmMain::DlgTfrmMain(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORSTATIC) {
		switch ( GetWindowLongPtr(( HWND )lParam, GWL_ID )) {
			case IDC_HEADERBAR:
				SetTextColor((HDC)wParam,GetSysColor(COLOR_WINDOWTEXT));
				break;
			default:
				return 0;
		}
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		return (INT_PTR)GetStockObject(WHITE_BRUSH); 	//GetSysColorBrush(COLOR_WINDOW);
	}

	CHandleMapping::iterator wnd;
	if(msg==WM_INITDIALOG) {
		wnd = _HandleMapping.insert(CHandleMapping::value_type(hWnd, reinterpret_cast<TfrmMain*>(lParam))).first;
		wnd->second->m_hWnd = hWnd;
		wnd->second->wmInitdialog(wParam, lParam);
		return 0;
	}
	wnd=_HandleMapping.find(hWnd);
	if(wnd==_HandleMapping.end()) {	//something screwed up dialog!
		return 0;					//do not use ::DefWindowProc(hWnd, msg, wParam, lParam);
	}

	switch (msg){
		case WM_DROPFILES:{ /// Drag&Drop of local files
			TCHAR filename[MAX_PATH];
			if(!DragQueryFile((HDROP)wParam,0,filename,MAX_PATH)) *filename='\0';
			DragFinish((HDROP)wParam);
			if(wnd->second->m_hwndTabPage)
				ShowWindow(wnd->second->m_hwndTabPage,SW_HIDE);
			TAB_INFO itab={TCIF_PARAM};
			wnd->second->m_opt_tabCapture=2; // activate file tab
			TabCtrl_SetCurSel(wnd->second->m_hwndTab,wnd->second->m_opt_tabCapture);
			TabCtrl_GetItem(wnd->second->m_hwndTab,wnd->second->m_opt_tabCapture,&itab);
			wnd->second->m_hwndTabPage=itab.hwndTabPage;
			ShowWindow(wnd->second->m_hwndTabPage,SW_SHOW);
			SetDlgItemText(wnd->second->m_hwndTabPage,ID_edtSize,filename);
			break;}
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
		case UM_CLOSING:
			wnd->second->UMClosing(wParam, lParam);
			break;
		case UM_EVENT:
			wnd->second->UMevent(wParam, lParam);
			break;
	}
	return 0;
}

//---------------------------------------------------------------------------
//WM_INITDIALOG:
void TfrmMain::wmInitdialog(WPARAM wParam, LPARAM lParam) {
	HWND hCtrl;
	/// Taskbar and Window icon
	SendMessage(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM)GetIcon(ICO_MAIN));
	SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)GetIcon(ICO_MAINXS));
	TCHAR* pt = mir_a2t(__PLUGIN_NAME);
	SetWindowText(m_hWnd, pt);
	mir_free(pt);

	/// Headerbar
	pt = mir_tstrdup((TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)m_hContact, (LPARAM)GCDNF_TCHAR));
	if (pt && (m_hContact != 0)) {
		TCHAR* lptString = NULL;
		mir_tstradd(lptString , TranslateT("Send screenshot to\n"));
		mir_tstradd(lptString , pt);
		SetDlgItemText(m_hWnd, IDC_HEADERBAR, lptString);
		mir_free(lptString);
	}
	mir_free(pt);

	SendDlgItemMessage(m_hWnd, IDC_HEADERBAR, WM_SETICON, ICON_BIG, (LPARAM)GetIcon(ICO_MAIN));

	/// Timed controls
	CheckDlgButton(m_hWnd,ID_chkTimed,				m_opt_chkTimed ? BST_CHECKED : BST_UNCHECKED);
	SetDlgItemInt (m_hWnd,ID_edtTimed,				(UINT)m_opt_edtTimed, FALSE);
	SendDlgItemMessage(m_hWnd, ID_upTimed,			UDM_SETRANGE, 0, (LPARAM)MAKELONG(250, 1));
	chkTimedClick();		//enable disable Timed controls

	/// create Image list for tab control
	if(!m_himlTab){
		//m_himlTab = ImageList_Create(16, 16, PluginConfig.m_bIsXP ? ILC_COLOR32 | ILC_MASK : ILC_COLOR8 | ILC_MASK, 2, 0);
		m_himlTab = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 1);
		ImageList_AddIcon(m_himlTab, GetIcon(ICO_TARGET));
		ImageList_AddIcon(m_himlTab, GetIcon(ICO_MONITOR));
		ImageList_AddIcon(m_himlTab, GetIconBtn(ICO_BTN_FOLDER));
	}

	/// create the tab control.
	{
	m_hwndTab = GetDlgItem(m_hWnd, IDC_CAPTURETAB);
	TabCtrl_SetImageList(m_hwndTab, m_himlTab);
	TabCtrl_SetItemExtra(m_hwndTab, sizeof(TAB_INFO) - sizeof(TCITEMHEADER));
	RECT rcTab;
	TAB_INFO itab;
	itab.hwndMain	= m_hWnd;
	itab.hwndTab	= m_hwndTab;
	itab.tcih.mask		= TCIF_PARAM|TCIF_TEXT|TCIF_IMAGE;

	/// Add a tab for each of the three child dialog boxes.
	itab.tcih.pszText	= TranslateT("Window");
	itab.tcih.iImage	= 0;
	itab.hwndTabPage	= CreateDialogParam(g_hSendSS,MAKEINTRESOURCE(IDD_UMain_CaptureWindow),m_hWnd,DlgProc_CaptureTabPage,IDD_UMain_CaptureWindow);
	TabCtrl_InsertItem(m_hwndTab,0,&itab);
	/// get tab boundaries (required after 1st tab)
	GetClientRect(m_hwndTab,&rcTab);
	MapWindowPoints(m_hwndTab,m_hWnd,(POINT*)&rcTab,2);
	TabCtrl_AdjustRect(m_hwndTab,0,&rcTab);
	rcTab.bottom-=rcTab.top; rcTab.right-=rcTab.left;
	///
	SetWindowPos(itab.hwndTabPage,HWND_TOP,rcTab.left,rcTab.top,rcTab.right,rcTab.bottom,0);
	CheckDlgButton(itab.hwndTabPage, ID_chkIndirectCapture, m_opt_chkIndirectCapture ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(itab.hwndTabPage, ID_chkClientArea, m_opt_chkClientArea ? BST_CHECKED : BST_UNCHECKED);

	itab.tcih.pszText	= TranslateT("Desktop");
	itab.tcih.iImage	= 1;
	itab.hwndTabPage	= CreateDialogParam(g_hSendSS,MAKEINTRESOURCE(IDD_UMain_CaptureDesktop),m_hWnd,DlgProc_CaptureTabPage,IDD_UMain_CaptureDesktop);
	TabCtrl_InsertItem(m_hwndTab,1,&itab);
	SetWindowPos(itab.hwndTabPage,HWND_TOP,rcTab.left,rcTab.top,rcTab.right,rcTab.bottom,0);

	hCtrl = GetDlgItem(itab.hwndTabPage, ID_edtCaption);
	ComboBox_ResetContent(hCtrl);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("<Entire Desktop>"))  ,0);
	ComboBox_SetCurSel (hCtrl,0);
	if(m_MonitorCount >1) {
		TCHAR tszTemp[120];
		for(size_t mon=0; mon<m_MonitorCount; ++mon) { /// @todo : fix format for non MSVC compilers
			mir_sntprintf(tszTemp, SIZEOF(tszTemp),_T("%Iu. %s%s"),
				mon+1, TranslateT("Monitor"),
				(m_Monitors[mon].dwFlags & MONITORINFOF_PRIMARY) ? TranslateT(" (primary)") : _T("")
				);
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, tszTemp)  , mon+1);
		}
		ComboBox_SelectItemData (hCtrl, -1, m_opt_cboxDesktop);	//use Workaround for MS bug ComboBox_SelectItemData
	}
	PostMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(ID_edtCaption, CBN_SELCHANGE),(LPARAM)hCtrl);

	itab.tcih.pszText	= TranslateT("File");
	itab.tcih.iImage	= 2;
	itab.hwndTabPage	= CreateDialogParam(g_hSendSS,MAKEINTRESOURCE(IDD_UMain_CaptureFile),m_hWnd,DlgProc_CaptureTabPage,IDD_UMain_CaptureFile);
	TabCtrl_InsertItem(m_hwndTab,2,&itab);
	SetWindowPos(itab.hwndTabPage,HWND_TOP,rcTab.left,rcTab.top,rcTab.right,rcTab.bottom,0);

	/// select tab and set m_hwndTabPage
	TabCtrl_SetCurSel(m_hwndTab, m_opt_tabCapture);
	itab.tcih.mask = TCIF_PARAM;
	TabCtrl_GetItem(m_hwndTab,m_opt_tabCapture,&itab);
	m_hwndTabPage = itab.hwndTabPage;
	ShowWindow(m_hwndTabPage,SW_SHOW);
	
	/// enable Drag&Drop for local file pane
	typedef BOOL (WINAPI *ChangeWindowMessageFilterEx_t)(HWND hwnd,UINT message,DWORD action,PCHANGEFILTERSTRUCT pChangeFilterStruct);
	ChangeWindowMessageFilterEx_t pChangeWindowMessageFilterEx;
	pChangeWindowMessageFilterEx=(ChangeWindowMessageFilterEx_t)GetProcAddress(GetModuleHandleA("user32"),"ChangeWindowMessageFilterEx");
	if(pChangeWindowMessageFilterEx){ /// Win7+, UAC fix
		pChangeWindowMessageFilterEx(m_hWnd,WM_DROPFILES,MSGFLT_ALLOW,NULL);
		pChangeWindowMessageFilterEx(m_hWnd,WM_COPYDATA,MSGFLT_ALLOW,NULL);
		pChangeWindowMessageFilterEx(m_hWnd,0x0049/*WM_COPYGLOBALDATA*/,MSGFLT_ALLOW,NULL);
	}
	DragAcceptFiles(m_hWnd,1);
	}
	/// init Format combo box
	{
	hCtrl = GetDlgItem(m_hWnd, ID_cboxFormat);
	ComboBox_ResetContent(hCtrl);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, _T("PNG")),0);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, _T("JPG")),1);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, _T("BMP")),2);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, _T("TIF")),3);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, _T("GIF")),4);
	ComboBox_SelectItemData (hCtrl, -1, m_opt_cboxFormat);	//use Workaround for MS bug ComboBox_SelectItemData
	}
	/// init SendBy combo box
	{
	hCtrl = GetDlgItem(m_hWnd, ID_cboxSendBy);
	ComboBox_ResetContent(hCtrl);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("<Only save>"))  ,SS_JUSTSAVE);
	if(m_hContact){
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("File Transfer")),SS_FILESEND);
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("E-mail"))       ,SS_EMAIL);
		if (g_myGlobals.PluginHTTPExist) {
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, _T("HTTP Server"))  ,SS_HTTPSERVER);
		}else if(m_opt_cboxSendBy == SS_HTTPSERVER) {
			m_opt_cboxSendBy = SS_IMAGESHACK;
		}
		if (g_myGlobals.PluginFTPExist) {
			ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("FTP File"))     ,SS_FTPFILE);
		}else if(m_opt_cboxSendBy == SS_FTPFILE) {
			m_opt_cboxSendBy = SS_IMAGESHACK;
		}
	}else if(m_opt_cboxSendBy == SS_FILESEND || m_opt_cboxSendBy == SS_EMAIL || m_opt_cboxSendBy == SS_HTTPSERVER || m_opt_cboxSendBy == SS_FTPFILE) {
		m_opt_cboxSendBy = SS_IMAGESHACK;
	}
	if (g_myGlobals.PluginDropboxExist) {
		ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, _T("Dropbox")), SS_DROPBOX);
	}else if(m_opt_cboxSendBy == SS_DROPBOX) {
		m_opt_cboxSendBy = SS_IMAGESHACK;
	}
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, _T("ImageShack")), SS_IMAGESHACK);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Upload Pie (30m)")), SS_UPLOADPIE_30M);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Upload Pie (1d)")), SS_UPLOADPIE_1D);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, TranslateT("Upload Pie (1w)")), SS_UPLOADPIE_1W);
	ComboBox_SetItemData(hCtrl, ComboBox_AddString(hCtrl, _T("imgur")), SS_IMGUR);
	ComboBox_SelectItemData (hCtrl, -1, m_opt_cboxSendBy);	//use Workaround for MS bug ComboBox_SelectItemData
	}
	/// init footer options
	CheckDlgButton(m_hWnd,ID_chkOpenAgain, m_opt_chkOpenAgain ? BST_CHECKED : BST_UNCHECKED);

	if (hCtrl = GetDlgItem(m_hWnd, ID_btnAbout)) {
		SendDlgItemMessage(m_hWnd, ID_btnAbout, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Information"), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(ICO_BTN_HELP);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? _T("") : _T("?"));
	}

	if (hCtrl = GetDlgItem(m_hWnd, ID_btnExplore)) {
		SendDlgItemMessage(m_hWnd, ID_btnExplore, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Open Folder"), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(ICO_BTN_FOLDER);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? _T("") : _T("..."));
	}

	if (hCtrl = GetDlgItem(m_hWnd, ID_chkDesc)) {
		SendDlgItemMessage(m_hWnd, ID_chkDesc, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Fill description textbox."), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(m_opt_btnDesc ? ICO_BTN_DESCON : ICO_BTN_DESC);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? _T("") : _T("D"));
		SendMessage(hCtrl, BM_SETCHECK, m_opt_btnDesc ? BST_CHECKED : BST_UNCHECKED, NULL);
	}

	if (hCtrl = GetDlgItem(m_hWnd, ID_chkDeleteAfterSend)) {
		SendDlgItemMessage(m_hWnd, ID_chkDeleteAfterSend, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Delete after send"), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(m_opt_btnDeleteAfterSend ? ICO_BTN_DELON : ICO_BTN_DEL);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? _T("") : _T("X"));
		SendMessage(hCtrl, BM_SETCHECK, m_opt_btnDeleteAfterSend ? BST_CHECKED : BST_UNCHECKED, NULL);
	}

	if (hCtrl = GetDlgItem(m_hWnd, ID_chkEditor)) {
		SendDlgItemMessage(m_hWnd, ID_chkEditor, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Open editor before sending"), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(m_opt_chkEditor ? ICO_BTN_EDITON : ICO_BTN_EDIT);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? _T("") : _T("E"));
		SendMessage(hCtrl, BM_SETCHECK, m_opt_chkEditor ? BST_CHECKED : BST_UNCHECKED, NULL);
	}

	if (hCtrl = GetDlgItem(m_hWnd, ID_btnCapture)) {
		SendMessage(hCtrl, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Capture"), MBBF_TCHAR);
		HICON hIcon = GetIconBtn(ICO_BTN_OK);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, TranslateT("&Capture"));
		SendMessage(hCtrl, BUTTONSETDEFAULT, 1, NULL);
	}
	cboxSendByChange();		//enable disable controls

	TranslateDialogDefault(m_hWnd);
}

//WM_COMMAND:
void TfrmMain::wmCommand(WPARAM wParam, LPARAM lParam) {
	//---------------------------------------------------------------------------
	int IDControl = LOWORD(wParam);
	switch (HIWORD(wParam)) {
		case BN_CLICKED:		//Button controls
			switch(IDControl) {
				case IDCANCEL: // ESC pressed
					this->Close();
					break;
				case ID_chkTimed:
					m_opt_chkTimed = (BYTE)Button_GetCheck((HWND)lParam);
					TfrmMain::chkTimedClick();
					break;
				case ID_chkIndirectCapture:
					m_opt_chkIndirectCapture = (BYTE)Button_GetCheck((HWND)lParam);
					break;
				case ID_chkClientArea:
					m_opt_chkClientArea = (BYTE)Button_GetCheck((HWND)lParam);
					if(m_hTargetWindow)
						edtSizeUpdate(m_hTargetWindow, m_opt_chkClientArea, GetParent((HWND)lParam), ID_edtSize);
					break;
				case ID_imgTarget:
					if(m_opt_tabCapture!=0) break;
					m_hLastWin=NULL;
					SetTimer(m_hWnd,ID_imgTarget,BUTTON_POLLDELAY,NULL);
					break;
				case ID_btnAbout:
					TfrmMain::btnAboutClick();
					break;
				case ID_btnExplore:
					TfrmMain::btnExploreClick();
					break;
				case ID_chkDesc:{
					m_opt_btnDesc=!m_opt_btnDesc;
					HICON hIcon=GetIconBtn(m_opt_btnDesc?ICO_BTN_DESCON:ICO_BTN_DESC);
					SendMessage((HWND)lParam,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcon);
					break;}
				case ID_chkDeleteAfterSend:{
					m_opt_btnDeleteAfterSend=!m_opt_btnDeleteAfterSend;
					HICON hIcon=GetIconBtn(m_opt_btnDeleteAfterSend?ICO_BTN_DELON:ICO_BTN_DEL);
					SendMessage((HWND)lParam,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcon);
					if(m_cSend) m_cSend->m_bDeleteAfterSend=m_opt_btnDeleteAfterSend;
					break;}
				case ID_chkEditor:{
					m_opt_chkEditor=!m_opt_chkEditor;
					HICON hIcon=GetIconBtn(m_opt_chkEditor?ICO_BTN_EDITON:ICO_BTN_EDIT);
					SendMessage((HWND)lParam,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hIcon);
					break;}
				case ID_chkOpenAgain:
					m_opt_chkOpenAgain=Button_GetCheck((HWND)lParam);
					break;
				case ID_btnCapture:
					TfrmMain::btnCaptureClick();
					break;
				default:
					break;
			}
			break;
		case CBN_SELCHANGE:		//ComboBox controls
			switch(IDControl) {
				//lParam = Handle to the control
				case ID_cboxFormat:			//not finish
					m_opt_cboxFormat = (BYTE)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
					break;
				case ID_cboxSendBy:
					m_opt_cboxSendBy = (BYTE)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
					cboxSendByChange();
					break;
				case ID_edtCaption:			//cboxDesktopChange
					m_opt_cboxDesktop = (BYTE)ComboBox_GetItemData((HWND)lParam, ComboBox_GetCurSel((HWND)lParam));
					m_hTargetWindow = NULL;
					if (m_opt_cboxDesktop > 0) {
						edtSizeUpdate(m_Monitors[m_opt_cboxDesktop-1].rcMonitor, GetParent((HWND)lParam), ID_edtSize);
					}
					else {
						edtSizeUpdate(m_VirtualScreen, GetParent((HWND)lParam), ID_edtSize);
					}
					break;
				default:
					break;
			}
			break;
		case EN_CHANGE:			//Edit controls
			switch(IDControl) {
				//lParam = Handle to the control
				case ID_edtQuality:
					m_opt_edtQuality = (BYTE)GetDlgItemInt(m_hWnd, ID_edtQuality, NULL, FALSE);
					break;
				case ID_edtTimed:
					m_opt_edtTimed = (BYTE)GetDlgItemInt(m_hWnd, ID_edtTimed, NULL, FALSE);
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

//WM_CLOSE:
void TfrmMain::wmClose(WPARAM wParam, LPARAM lParam) {
	DestroyWindow(m_hWnd);
	return;
}

//WM_TIMER:
const int g_iTargetBorder=7;
void TfrmMain::SetTargetWindow(HWND hwnd){
	if(!hwnd){
		POINT point; GetCursorPos(&point);
		hwnd=WindowFromPoint(point);
//		if(!((GetAsyncKeyState(VK_SHIFT)|GetAsyncKeyState(VK_MENU))&0x8000))
		for(HWND hTMP; (hTMP=GetParent(hwnd)); hwnd=hTMP);
	}
	m_hTargetWindow=hwnd;
	int len=GetWindowTextLength(m_hTargetWindow)+1;
	TCHAR* lpTitle;
	if(len>1){
		lpTitle=(TCHAR*)mir_alloc(len*sizeof(TCHAR));
		GetWindowText(m_hTargetWindow,lpTitle,len);
	}else{//no WindowText present, use WindowClass
		lpTitle=(TCHAR*)mir_alloc(64*sizeof(TCHAR));
		RealGetWindowClass(m_hTargetWindow,lpTitle,64);
	}
	SetDlgItemText(m_hwndTabPage,ID_edtCaption,lpTitle);
	mir_free(lpTitle);
	edtSizeUpdate(m_hTargetWindow,m_opt_chkClientArea,m_hwndTabPage,ID_edtSize);
}
void TfrmMain::wmTimer(WPARAM wParam, LPARAM lParam){
	if(wParam==ID_imgTarget){// Timer for Target selector
		static int primarymouse;
		if(!m_hTargetHighlighter){
			primarymouse=GetSystemMetrics(SM_SWAPBUTTON)?VK_RBUTTON:VK_LBUTTON;
			m_hTargetHighlighter=CreateWindowEx(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_TOOLWINDOW,(TCHAR*)g_clsTargetHighlighter,NULL,WS_POPUP,0,0,0,0,NULL,NULL,g_hSendSS,NULL);
			if(!m_hTargetHighlighter) return;
			SetLayeredWindowAttributes(m_hTargetHighlighter,0,123,LWA_ALPHA);
			SetSystemCursor(CopyCursor(GetIcon(ICO_TARGET)),OCR_IBEAM);//text cursor
			SetSystemCursor(CopyCursor(GetIcon(ICO_TARGET)),OCR_NORMAL);
			SetActiveWindow(m_hTargetHighlighter); // activate highlighter to fix focus problems with UAC (unelevated GetAsyncKeyState() fails if an elevated app got focus)
			Hide();
		}
		if(!(GetAsyncKeyState(primarymouse)&0x8000)){
			KillTimer(m_hWnd,ID_imgTarget);
			SystemParametersInfo(SPI_SETCURSORS,0,NULL,0);
			DestroyWindow(m_hTargetHighlighter),m_hTargetHighlighter=NULL;
			SetTargetWindow(m_hLastWin);
			Show();
			return;
		}
		POINT point; GetCursorPos(&point);
		HWND hwnd=WindowFromPoint(point);
		if(!((GetAsyncKeyState(VK_SHIFT)|GetAsyncKeyState(VK_MENU))&0x8000))
			for(HWND hTMP; (hTMP=GetAncestor(hwnd,GA_PARENT)) && IsChild(hTMP,hwnd); hwnd=hTMP);
		else{
			ScreenToClient(hwnd,&point);
			HWND hTMP; if((hTMP=RealChildWindowFromPoint(hwnd,point)))
				hwnd=hTMP;
		}
		if(hwnd!=m_hLastWin){
			m_hLastWin=hwnd;
			RECT rect;
			if(m_opt_chkClientArea){
				GetClientRect(hwnd,&rect);
				ClientToScreen(hwnd,(POINT*)&rect);
				rect.right=rect.left+rect.right;
				rect.bottom=rect.top+rect.bottom;
			}else
				GetWindowRect(hwnd,&rect);
			int width=rect.right-rect.left;
			int height=rect.bottom-rect.top;
			if(g_iTargetBorder){
				SetWindowPos(m_hTargetHighlighter,NULL,0,0,0,0,SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE);
				if(width>g_iTargetBorder*2 && height>g_iTargetBorder*2) {
					HRGN hRegnNew=CreateRectRgn(0,0,width,height);
					HRGN hRgnHole=CreateRectRgn(g_iTargetBorder,g_iTargetBorder,width-g_iTargetBorder,height-g_iTargetBorder);
					CombineRgn(hRegnNew,hRegnNew,hRgnHole,RGN_XOR);
					DeleteObject(hRgnHole);
					SetWindowRgn(m_hTargetHighlighter,hRegnNew,FALSE);//cleans up hRegnNew
				}else SetWindowRgn(m_hTargetHighlighter,NULL,FALSE);
			}
			SetWindowPos(m_hTargetHighlighter,HWND_TOPMOST,rect.left,rect.top,width,height,SWP_SHOWWINDOW|SWP_NOACTIVATE);
		}
		return;
	}
	if(wParam==ID_chkTimed){// Timer for Screenshot
		#ifdef _DEBUG
			OutputDebugStringA("SS Bitmap Timer Start\r\n" );
		#endif
		if(!m_bCapture) {		//only start once
			if (m_Screenshot) {
				FIP->FI_Unload(m_Screenshot);
				m_Screenshot = NULL;
			}
			m_bCapture = true;
			switch (m_opt_tabCapture) {
				case 0:
					m_Screenshot = CaptureWindow(m_hTargetWindow, m_opt_chkClientArea, m_opt_chkIndirectCapture);
					break;
				case 1:
					m_Screenshot = CaptureMonitor((m_opt_cboxDesktop > 0) ? m_Monitors[m_opt_cboxDesktop-1].szDevice : NULL);
					break;
				case 2: /// edge case, existing local file
					break;
				#ifdef _DEBUG
				default:
					OutputDebugStringA("SS Bitmap Timer Stop (no tabCapture)\r\n" );
				#endif
			}
			m_bCapture = false;
			if (m_Screenshot || m_opt_tabCapture==2) { /// @note : test without "if"
				KillTimer(m_hWnd,ID_chkTimed);
				#ifdef _DEBUG
					OutputDebugStringA("SS Bitmap Timer Stop (CaptureDone)\r\n" );
				#endif
				SendMessage(m_hWnd,UM_EVENT, 0, (LPARAM)EVT_CaptureDone);
			}
		}
	}
}

//WM_NOTIFY:
void TfrmMain::wmNotify(WPARAM wParam, LPARAM lParam) {
	switch(((LPNMHDR)lParam)->idFrom) {
		case IDC_CAPTURETAB:				//TabControl IDC_CAPTURETAB
			switch (((LPNMHDR)lParam)->code) {
				//    HWND hwndFrom;	= member is handle to the tab control
				//    UINT_PTR idFrom;	= member is the child window identifier of the tab control.
				//    UINT code;		= member is TCN_SELCHANGE
				case TCN_SELCHANGING:{
					if(!m_hwndTabPage) break;
					ShowWindow(m_hwndTabPage,SW_HIDE);
					m_hwndTabPage=NULL;
					break;}
				case TCN_SELCHANGE:{
					TAB_INFO itab={TCIF_PARAM};
					m_opt_tabCapture=TabCtrl_GetCurSel(m_hwndTab);
					TabCtrl_GetItem(m_hwndTab, m_opt_tabCapture, &itab);
					m_hwndTabPage=itab.hwndTabPage;
					ShowWindow(m_hwndTabPage, SW_SHOW);
					break;}
				default:
					break;
			}
			break;
		default:
			break;
	}
}

//UM_CLOSING:
void TfrmMain::UMClosing(WPARAM wParam, LPARAM lParam) {
	HWND hWnd = (HWND)wParam;
	switch (lParam) {
		case IDD_UAboutForm:
			btnAboutOnCloseWindow(hWnd);
			break;
		case IDD_UEditForm:
			;
			break;
		default:
			break;
	}
}

//UM_EVENT:
void TfrmMain::UMevent(WPARAM wParam, LPARAM lParam) {
	//HWND hWnd = (HWND)wParam;
	switch (lParam) {
		case EVT_CaptureDone:
			if (!m_Screenshot && m_opt_tabCapture!=2) {
				TCHAR *err = TranslateT("Couldn't take a screenshot");
				MessageBox(NULL,err,ERROR_TITLE,MB_OK|MB_ICONWARNING);
				Show();
				return;
			}
			FormClose();
			break;
		case EVT_SendFileDone:
			break;
		case EVT_CheckOpenAgain:
			if(m_opt_chkOpenAgain){
				if(m_Screenshot){
					FIP->FI_Unload(m_Screenshot);
					m_Screenshot = NULL;
				}
				Show();
			}else{// Saving Options and close
				SaveOptions();
				Close();
			}
			break;
		default:
			break;
	}
}

//---------------------------------------------------------------------------
// Standard konstruktor/destruktor
TfrmMain::TfrmMain() {
	/* m_opt_XXX */
	m_bOnExitSave	= TRUE;

	m_hWnd			= NULL;
	m_hContact		= NULL;
	m_bFormAbout=false;
	m_hTargetWindow	=m_hLastWin=NULL;
	m_hTargetHighlighter=NULL;
	m_FDestFolder	=m_pszFile=m_pszFileDesc=NULL;
	m_Screenshot	= NULL;
	/* m_AlphaColor */
	m_cSend			= NULL;

	m_Monitors		= NULL;
	m_MonitorCount	= MonitorInfoEnum(m_Monitors, m_VirtualScreen);
	/* m_opt_XXX */ LoadOptions();
	m_bCapture		= false;
	/* m_hwndTab,m_hwndTabPage */
	m_himlTab		= NULL;
}

TfrmMain::~TfrmMain() {
	_HandleMapping.erase(m_hWnd);
	mir_free(m_pszFile);
	mir_free(m_FDestFolder);
	mir_free(m_pszFileDesc);
	mir_free(m_Monitors);
	if (m_Screenshot) FIP->FI_Unload(m_Screenshot);
	if (m_cSend) delete m_cSend;
	if(m_hTargetHighlighter){
		DestroyWindow(m_hTargetHighlighter),m_hTargetHighlighter=NULL;
		SystemParametersInfo(SPI_SETCURSORS,0,NULL,0);
	}
}

//---------------------------------------------------------------------------
// Load / Saving options from miranda's database
void TfrmMain::LoadOptions(void) {
	DWORD rgb					= db_get_dw(NULL, SZ_SENDSS, "AlphaColor", 16777215);
	m_AlphaColor.rgbRed			= GetRValue(rgb);
	m_AlphaColor.rgbGreen		= GetGValue(rgb);
	m_AlphaColor.rgbBlue		= GetBValue(rgb);
	m_AlphaColor.rgbReserved	= 0;

//	m_opt_chkEmulateClick		= db_get_b(NULL, SZ_SENDSS, "AutoSend", 1);
	m_opt_edtQuality			= db_get_b(NULL, SZ_SENDSS, "JpegQuality", 75);

	m_opt_tabCapture			= db_get_b(NULL, SZ_SENDSS, "Capture", 0);
	m_opt_chkIndirectCapture	= db_get_b(NULL, SZ_SENDSS, "IndirectCapture", 0);
	m_opt_chkClientArea			= db_get_b(NULL, SZ_SENDSS, "ClientArea", 0);
	m_opt_cboxDesktop			= db_get_b(NULL, SZ_SENDSS, "Desktop", 0);

	m_opt_chkTimed				= db_get_b(NULL, SZ_SENDSS, "TimedCap", 0);
	m_opt_edtTimed				= db_get_b(NULL, SZ_SENDSS, "CapTime", 3);
	m_opt_cboxFormat			= db_get_b(NULL, SZ_SENDSS, "OutputFormat", 0);
	m_opt_cboxSendBy			= db_get_b(NULL, SZ_SENDSS, "SendBy", 0);
	
	m_opt_btnDesc				= db_get_b(NULL, SZ_SENDSS, "AutoDescription", 1);
	m_opt_btnDeleteAfterSend	= db_get_b(NULL, SZ_SENDSS, "DelAfterSend", 1)!=0;
	m_opt_chkEditor				= db_get_b(NULL, SZ_SENDSS, "Preview", 0);
	m_opt_chkOpenAgain			= db_get_b(NULL, SZ_SENDSS, "OpenAgain", 0);
}

void TfrmMain::SaveOptions(void) {
	if(m_bOnExitSave) {
		db_set_dw(NULL, SZ_SENDSS, "AlphaColor",
			(DWORD)RGB(m_AlphaColor.rgbRed, m_AlphaColor.rgbGreen, m_AlphaColor.rgbBlue));

//		db_set_b(NULL, SZ_SENDSS, "AutoSend", m_opt_chkEmulateClick);
		db_set_b(NULL, SZ_SENDSS, "JpegQuality", m_opt_edtQuality);

		db_set_b(NULL, SZ_SENDSS, "Capture", m_opt_tabCapture);
		db_set_b(NULL, SZ_SENDSS, "IndirectCapture", m_opt_chkIndirectCapture);
		db_set_b(NULL, SZ_SENDSS, "ClientArea", m_opt_chkClientArea);
		db_set_b(NULL, SZ_SENDSS, "Desktop", m_opt_cboxDesktop);

		db_set_b(NULL, SZ_SENDSS, "TimedCap", m_opt_chkTimed);
		db_set_b(NULL, SZ_SENDSS, "CapTime", m_opt_edtTimed);
		db_set_b(NULL, SZ_SENDSS, "OutputFormat", m_opt_cboxFormat);
		db_set_b(NULL, SZ_SENDSS, "SendBy", m_opt_cboxSendBy);

		db_set_b(NULL, SZ_SENDSS, "AutoDescription", m_opt_btnDesc);
		db_set_b(NULL, SZ_SENDSS, "DelAfterSend", m_opt_btnDeleteAfterSend);
		db_set_b(NULL, SZ_SENDSS, "Preview", m_opt_chkEditor);
		db_set_b(NULL, SZ_SENDSS, "OpenAgain", m_opt_chkOpenAgain);
	}
}

//---------------------------------------------------------------------------
void TfrmMain::Init(TCHAR* DestFolder, MCONTACT Contact) {
	m_FDestFolder = mir_tstrdup(DestFolder);
	m_hContact = Contact;

	// create window
	m_hWnd = CreateDialogParam(g_hSendSS, MAKEINTRESOURCE(IDD_UMainForm),0,DlgTfrmMain,(LPARAM)this);
	//register object
	_HandleMapping.insert(CHandleMapping::value_type(m_hWnd, this));

	//check Contact
	if(m_cSend)
		m_cSend->SetContact(Contact);
}

//---------------------------------------------------------------------------
void TfrmMain::btnCaptureClick() {
	if(m_opt_tabCapture==1) m_hTargetWindow=GetDesktopWindow();
	else if(m_opt_tabCapture==2){
		TCHAR filename[MAX_PATH];
		GetDlgItemText(m_hwndTabPage, ID_edtSize, filename, SIZEOF(filename));
		FILE* fp=_wfopen(filename,_T("rb"));
		if(!fp){
			TCHAR *err = TranslateT("Select a file");
			MessageBox(m_hWnd,err,ERROR_TITLE,MB_OK|MB_ICONWARNING);
			return;
		}
		fclose(fp);
		mir_free(m_pszFile); m_pszFile=mir_tstrdup(filename);
	}else if(!m_hTargetWindow) {
		TCHAR *err = TranslateT("Select a target window.");
		MessageBox(m_hWnd,err,ERROR_TITLE,MB_OK|MB_ICONWARNING);
		return;
	}
	TfrmMain::Hide();

	if(m_opt_chkTimed){
		SetTimer(m_hWnd, ID_chkTimed, m_opt_edtTimed ? m_opt_edtTimed*1000 : 500, NULL); /// calls EVT_CaptureDone
		return;
	}
	if(m_opt_tabCapture==1){ /// desktop needs always time to update from TfrmMain::Hide()
		SetTimer(m_hWnd, ID_chkTimed, 500, NULL); /// calls EVT_CaptureDone
		return;
	}
	if(m_opt_tabCapture!=2){
		m_Screenshot = CaptureWindow(m_hTargetWindow, m_opt_chkClientArea, m_opt_chkIndirectCapture);
	}
	SendMessage(m_hWnd,UM_EVENT, 0, (LPARAM)EVT_CaptureDone);
}

//---------------------------------------------------------------------------
void TfrmMain::chkTimedClick() {
	Button_Enable(GetDlgItem(m_hWnd, ID_edtTimedLabel), (BOOL)m_opt_chkTimed);
	Button_Enable(GetDlgItem(m_hWnd, ID_edtTimed), (BOOL)m_opt_chkTimed);
	Button_Enable(GetDlgItem(m_hWnd, ID_upTimed), (BOOL)m_opt_chkTimed);
}

//---------------------------------------------------------------------------
void TfrmMain::cboxSendByChange() {
	BOOL bState;
	HICON hIcon;
	BYTE itemFlag = SS_DLG_DESCRIPTION;		//SS_DLG_AUTOSEND | SS_DLG_DELETEAFTERSSEND |
	if(m_cSend)
		delete m_cSend;
	switch(m_opt_cboxSendBy) {
		case SS_FILESEND:		//"File Transfer"
			m_cSend = new CSendFile(m_hWnd, m_hContact, true);
			break;
		case SS_EMAIL:			//"E-mail"
			m_cSend = new CSendEmail(m_hWnd, m_hContact, true);
			break;
		case SS_HTTPSERVER:		//"HTTP Server"
			m_cSend = new CSendHTTPServer(m_hWnd, m_hContact, true);
			break;
		case SS_FTPFILE:		//"FTP File"
			m_cSend = new CSendFTPFile(m_hWnd, m_hContact, true);
			break;
		case SS_DROPBOX:		//"Dropbox"
			m_cSend = new CSendDropbox(m_hWnd, m_hContact, false);
			break;
		case SS_IMAGESHACK:		//"ImageShack"
			m_cSend = new CSendHost_ImageShack(m_hWnd, m_hContact, true);
			break;
		case SS_UPLOADPIE_30M:		//"Upload Pie (30 minutes)"
			m_cSend = new CSendHost_UploadPie(m_hWnd, m_hContact, true,1);
			break;
		case SS_UPLOADPIE_1D:		//"Upload Pie (1 day)"
			m_cSend = new CSendHost_UploadPie(m_hWnd, m_hContact, true,4);
			break;
		case SS_UPLOADPIE_1W:		//"Upload Pie (1 week)"
			m_cSend = new CSendHost_UploadPie(m_hWnd, m_hContact, true,5);
			break;
		case SS_IMGUR:
			m_cSend = new CSendHost_Imgur(m_hWnd, m_hContact, true);
			break;
		default:				//SS_JUSTSAVE - "Just save it "
			m_cSend = NULL;
			break;
	}
	if(m_cSend){
		itemFlag = m_cSend->GetEnableItem();
		m_cSend->m_bDeleteAfterSend = m_opt_btnDeleteAfterSend;
	}
	bState = (itemFlag & SS_DLG_DELETEAFTERSSEND);
	hIcon = GetIconBtn(m_opt_btnDeleteAfterSend ? ICO_BTN_DELON : ICO_BTN_DEL);
	SendDlgItemMessage(m_hWnd, ID_chkDeleteAfterSend, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(bState ? hIcon : 0));
	Button_Enable(GetDlgItem(m_hWnd, ID_chkDeleteAfterSend), bState);

	bState = (itemFlag & SS_DLG_DESCRIPTION);
	hIcon = GetIconBtn(m_opt_btnDesc ? ICO_BTN_DESCON : ICO_BTN_DESC);
	SendDlgItemMessage(m_hWnd, ID_chkDesc, BM_SETIMAGE, IMAGE_ICON, (LPARAM)(bState ? hIcon : 0));
	Button_Enable(GetDlgItem(m_hWnd, ID_chkDesc), bState);
}

//---------------------------------------------------------------------------
void TfrmMain::btnAboutClick() {
	if (m_bFormAbout) return;

	TfrmAbout *frmAbout=new TfrmAbout(m_hWnd);
	frmAbout->Show();
	m_bFormAbout = true;
}

// Edit window call this event before it closes
void TfrmMain::btnAboutOnCloseWindow(HWND hWnd) {
	m_bFormAbout = false;
}

//---------------------------------------------------------------------------
void TfrmMain::btnExploreClick() {
	if (m_FDestFolder)
		ShellExecute(NULL, _T("explore"), m_FDestFolder, NULL, NULL, SW_SHOW);
}

//---------------------------------------------------------------------------
void TfrmMain::edtSizeUpdate(HWND hWnd, BOOL ClientArea, HWND hTarget, UINT Ctrl) {
	// get window dimensions
	RECT rect = {0};
	RECT cliRect = {0};
	TCHAR B[33], H[16];
	GetWindowRect(hWnd, &rect);
	if (ClientArea) {
		POINT pt = {0};
		GetClientRect(hWnd, &cliRect);
		pt.x = cliRect.left;
		pt.y = cliRect.top;
		ClientToScreen(hWnd, &pt);
		pt.x = pt.x - rect.left;			//offset x for client area
		pt.y = pt.y - rect.top;				//offset y for client area
		rect = cliRect;
	}
//	_itot_s(rect.right - rect.left, B, 33, 10);
	_itot(rect.right - rect.left, B, 10);
//	_itot_s(rect.bottom - rect.top, H, 16, 10);
	_itot(rect.bottom - rect.top, H, 10);
	mir_tstrncat(B, _T("x"), 33);
	mir_tstrncat(B, H, 33);
	SetDlgItemText(hTarget, Ctrl, B);
}

void TfrmMain::edtSizeUpdate(RECT rect, HWND hTarget, UINT Ctrl) {
	TCHAR B[33], H[16];
//	_itot_s(ABS(rect.right - rect.left), B, 33, 10);
	_itot(ABS(rect.right - rect.left), B, 10);
//	_itot_s(ABS(rect.bottom - rect.top), H, 16, 10);
	_itot(ABS(rect.bottom - rect.top), H, 10);
	mir_tstrncat(B, _T("x"), 33);
	mir_tstrncat(B, H, 33);
	SetDlgItemText(hTarget, Ctrl, B);
}

//---------------------------------------------------------------------------
INT_PTR TfrmMain::SaveScreenshot(FIBITMAP* dib) {
	//generate File name
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	TCHAR* ret;
	TCHAR* path = NULL;
	TCHAR* pszFilename = NULL;
	TCHAR* pszFileDesc = NULL;
	if (!dib) return 1;		//error
	unsigned FileNumber=db_get_dw(NULL,SZ_SENDSS,"FileNumber",0)+1;
	if(FileNumber>99999) FileNumber=1;
	//Generate FileName
	mir_tstradd(path, m_FDestFolder);
	if (path[mir_tstrlen(path)-1] != _T('\\')) mir_tstradd(path, _T("\\"));
	mir_tstradd(path, _T("shot%.5u"));//on format change, adapt "len" below
	size_t len=mir_tstrlen(path)+2;
	pszFilename = (TCHAR*)mir_alloc(sizeof(TCHAR)*(len));
	mir_sntprintf(pszFilename,len,path,FileNumber);
	mir_free(path);

	//Generate a description according to the screenshot
	TCHAR winText[1024];
	mir_tstradd(pszFileDesc, TranslateT("Screenshot "));
	if (m_opt_tabCapture == 0 && m_opt_chkClientArea) {
		mir_tstradd(pszFileDesc, TranslateT("for Client area "));
	}
	mir_tstradd(pszFileDesc, TranslateT("of \""));
	GetDlgItemText(m_hwndTabPage, ID_edtCaption, winText, SIZEOF(winText));
	mir_tstradd(pszFileDesc, winText);
	if(m_opt_tabCapture==1)
		mir_tstradd(pszFileDesc, _T("\""));
	else
		mir_tstradd(pszFileDesc, TranslateT("\" Window"));

	// convert to 32Bits (make shure it is 32bit)
	FIBITMAP *dib_new = FIP->FI_ConvertTo32Bits(dib);
	//RGBQUAD appColor = { 245, 0, 254, 0 };	//bgr0	schwarz
	//FIP->FI_SetBackgroundColor(dib_new, &appColor);
	FIP->FI_SetTransparent(dib_new,TRUE);

	// Investigates the color type of the bitmap (test for RGB or CMYK colour space)
	switch (FREE_IMAGE_COLOR_TYPE ColTye=FIP->FI_GetColorType(dib_new)) {
		case FIC_MINISBLACK:
			//Monochrome bitmap (1-bit) : first palette entry is black.
			//Palletised bitmap (4 or 8-bit) and single channel non standard bitmap: the bitmap has a greyscale palette
		case FIC_MINISWHITE:
			//Monochrome bitmap (1-bit) : first palette entry is white.
			//Palletised bitmap (4 or 8-bit) : the bitmap has an inverted greyscale palette
		case FIC_PALETTE:
			//Palettized bitmap (1, 4 or 8 bit)
		case FIC_RGB:
			//High-color bitmap (16, 24 or 32 bit), RGB16 or RGBF
		case FIC_RGBALPHA:
			//High-color bitmap with an alpha channel (32 bit bitmap, RGBA16 or RGBAF)
		case FIC_CMYK:
			//CMYK bitmap (32 bit only)
		default:
			break;
	}

//	bool bDummy = !(FIP->FI_GetICCProfile(dib_new)->flags & FIICC_COLOR_IS_CMYK);

	FIBITMAP *dib32,*dib24;
	HWND hwndCombo = GetDlgItem(m_hWnd, ID_cboxFormat);
	switch (ComboBox_GetItemData(hwndCombo, ComboBox_GetCurSel(hwndCombo))) {
		case 0: //PNG
			ret = SaveImage(fif,dib_new, pszFilename, _T("png"));
			break;

		case 1: //JPG
			/*
			#define JPEG_QUALITYSUPERB  0x80	// save with superb quality (100:1)
			#define JPEG_QUALITYGOOD    0x0100	// save with good quality (75:1)
			#define JPEG_QUALITYNORMAL  0x0200	// save with normal quality (50:1)
			#define JPEG_QUALITYAVERAGE 0x0400	// save with average quality (25:1)
			#define JPEG_QUALITYBAD     0x0800	// save with bad quality (10:1)
			#define JPEG_PROGRESSIVE	0x2000	// save as a progressive-JPEG (use | to combine with other save flags)
			*/
			dib32 = FIP->FI_Composite(dib_new,FALSE,&m_AlphaColor,NULL);
			dib24 = FIP->FI_ConvertTo24Bits(dib32);
			FIP->FI_Unload(dib32);
			ret = SaveImage(fif,dib24, pszFilename, _T("jpg"));
			FIP->FI_Unload(dib24);
			break;

		case 2: //BMP
		//	ret = SaveImage(FIF_BMP,dib_new, pszFilename, _T("bmp")); //32bit alpha BMP
			dib32 = FIP->FI_Composite(dib_new,FALSE,&m_AlphaColor,NULL);
			dib24 = FIP->FI_ConvertTo24Bits(dib32);
			FIP->FI_Unload(dib32);
			ret = SaveImage(FIF_BMP,dib24, pszFilename, _T("bmp"));
			FIP->FI_Unload(dib24);
			break;

		case 3: //TIFF (miranda freeimage interface do not support save tiff, we udse GDI+)
			{
			TCHAR* pszFile = NULL;
			mir_tstradd(pszFile, pszFilename);
			mir_tstradd(pszFile, _T(".tif"));

			dib32 = FIP->FI_Composite(dib_new,FALSE,&m_AlphaColor,NULL);
			dib24 = FIP->FI_ConvertTo24Bits(dib32);
			FIP->FI_Unload(dib32);

			HBITMAP hBmp = FIP->FI_CreateHBITMAPFromDIB(dib24);
			FIP->FI_Unload(dib24);
			SaveTIF(hBmp, pszFile);
			ret=pszFile;
			DeleteObject(hBmp);
			}
			break;

		case 4: //GIF
			{
			//dib24 = FIP->FI_ConvertTo8Bits(dib_new);
			//ret = SaveImage(FIF_GIF,dib24, pszFilename, _T("gif"));
			//FIP->FI_Unload(dib24);
			TCHAR* pszFile = NULL;
			mir_tstradd(pszFile, pszFilename);
			mir_tstradd(pszFile, _T(".gif"));
			HBITMAP hBmp = FIP->FI_CreateHBITMAPFromDIB(dib_new);
			SaveGIF(hBmp, pszFile);
			ret=pszFile;
			DeleteObject(hBmp);
			}
			break;

		default:
			ret=NULL;
	}
/*	//load PNG and save file in user format (if differ)
	//this get better result for transparent aereas
	//TCHAR* pszFormat = (TCHAR*)ComboBox_GetItemData(hwndCombo, ComboBox_GetCurSel(hwndCombo));
	TCHAR pszFormat[6];
	ComboBox_GetText(hwndCombo, pszFormat, 6);
	if(ret && (_tcsicmp (pszFormat,_T("png")) != 0)) {

			fif = FIP->FI_GetFIFFromFilenameU(ret);
			dib_new = FIP->FI_LoadU(fif, ret,0);


		if(dib_new) {
			DeleteFile(ret);
			mir_free(ret);
			FIBITMAP *dib_save = FIP->FI_ConvertTo24Bits(dib_new);
			ret = SaveImage(FIF_UNKNOWN,dib_save, pszFilename, pszFormat);
			FIP->FI_Unload(dib_new); dib_new = NULL;
			FIP->FI_Unload(dib_save); dib_save = NULL;
		}
	}*/
	FIP->FI_Unload(dib_new);
	mir_free(pszFilename);

	if(ret){
		db_set_dw(NULL,SZ_SENDSS,"FileNumber",FileNumber);
		mir_free(m_pszFile); m_pszFile=ret;
		mir_free(m_pszFileDesc);
		if(IsWindowEnabled(GetDlgItem(m_hWnd,ID_chkDesc)) && m_opt_btnDesc){
			m_pszFileDesc=pszFileDesc;
		}else{
			mir_free(pszFileDesc);
			m_pszFileDesc=mir_tstrdup(_T(""));
		}

		if(m_cSend) {
			m_cSend->SetFile(m_pszFile);
			m_cSend->SetDescription(m_pszFileDesc);
		}
		return 0;//OK
	}
	mir_free(pszFileDesc);
	return 1;//error
}

//---------------------------------------------------------------------------
void TfrmMain::FormClose() {
	bool bCanDelete=m_opt_btnDeleteAfterSend;
	if(m_opt_tabCapture==2){ /// existing file
		TCHAR description[1024];
		GetDlgItemText(m_hwndTabPage, ID_edtCaption, description, SIZEOF(description));
		if(!IsWindowEnabled(GetDlgItem(m_hWnd,ID_chkDesc)) || !m_opt_btnDesc)
			*description='\0';
		if(m_cSend) {
			m_cSend->m_bDeleteAfterSend = false; /// well... guess it's better to not delete existing files for now...
			m_cSend->SetFile(m_pszFile);
			m_cSend->SetDescription(description);
		}
		bCanDelete=false;
	}else if(SaveScreenshot(m_Screenshot)){ /// Saving the screenshot
		Show();		// Error from SaveScreenshot
		return;
	}
	
	bool send=true;
	if(m_opt_chkEditor){
		SHELLEXECUTEINFO shex={sizeof(SHELLEXECUTEINFO)};
		shex.fMask=SEE_MASK_NOCLOSEPROCESS;
		shex.lpVerb=_T("edit");
		shex.lpFile=m_pszFile;
		shex.nShow=SW_SHOWNORMAL;
		ShellExecuteEx(&shex);
		if(shex.hProcess){
			DWORD res;
			MSG msg;
			do{
				// wait for editor exit or messages/input
				res=MsgWaitForMultipleObjects(1,&shex.hProcess,0,INFINITE,QS_ALLINPUT);
				while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)){
					if(msg.message==WM_QUIT){
						res=WAIT_OBJECT_0;
						PostMessage(NULL,WM_QUIT,0,0); // forward for outer message loops
						break;
					}
					// process dialog messages (of unknown dialogs)
//					HWND hwndDlgModeless=msg.hwnd;
//					for(HWND hTMP; (hTMP=GetAncestor(hwndDlgModeless,GA_PARENT)) && IsChild(hTMP,hwndDlgModeless); hwndDlgModeless=hTMP);
//					if(IsDialogMessage(hwndDlgModeless,&msg))
					if(IsDialogMessage(GetActiveWindow(),&msg))
						continue;
					// process messages
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}while(res==WAIT_OBJECT_0+1);
			CloseHandle(shex.hProcess);
		}
		if(MessageBox(m_hWnd,TranslateT("Send screenshot?"),_T("SendSS"),MB_YESNO|MB_ICONQUESTION|MB_SYSTEMMODAL)!=IDYES)
			send=false;
	}

	if(send && m_cSend && m_pszFile){
		if(!m_cSend->Send()) m_cSend=NULL; // not sent now, class deletes itself later
		cboxSendByChange();
	}else if(!send && bCanDelete){
		DeleteFile(m_pszFile);
	}
	SendMessage(m_hWnd,UM_EVENT, 0, (LPARAM)EVT_CheckOpenAgain);
}

//---------------------------------------------------------------------------
/*/ Edit window call this event before it closes
void TfrmMain::OnCloseEditWindow(TObject *Sender, TCloseAction &Action) {
	TfrmEdit *form=dynamic_cast<TfrmEdit*>(Sender);
	form->Hide();

	// delete the form automatically,after this event returns
	Action = caFree;

	// This will saves settings, free resources, ...
	form->CallBeforeClose(Action);

	// User selected "Capture" on action menu of edit window
	if (form->ModalResult == mrCancel) {
		this->Show();
	} else {
		Screenshot = form->Screen;
		bFormEdit = form->DontSend;
		this->Close();
	}
}*/

//---------------------------------------------------------------------------
