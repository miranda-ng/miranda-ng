/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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
void TfrmAbout::Unload(){
	std::list<TfrmAbout*> lst;
	for(CHandleMapping::iterator iter=_HandleMapping.begin(); iter!=_HandleMapping.end(); ++iter){
		lst.push_back(iter->second);//we can't delete inside loop.. not MT compatible
	}
	while(!lst.empty()){
		DestroyWindow(lst.front()->m_hWnd);//deletes class
		lst.pop_front();
	}
}

//---------------------------------------------------------------------------
TfrmAbout::CHandleMapping TfrmAbout::_HandleMapping;

INT_PTR CALLBACK TfrmAbout::DlgTfrmAbout(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORSTATIC) {
		switch ( GetWindowLongPtr(( HWND )lParam, GWL_ID )) {
			case IDC_CREDIT:
			case IDC_LICENSE:
				SetTextColor((HDC)wParam,GetSysColor(COLOR_WINDOWTEXT));
				break;
			default:
				return FALSE;
		}
		return (LRESULT)GetStockObject(WHITE_BRUSH); 	//GetSysColorBrush(COLOR_WINDOW);
	}

	CHandleMapping::iterator wnd(_HandleMapping.end());
	if (msg == WM_INITDIALOG) {
		wnd = _HandleMapping.insert(CHandleMapping::value_type(hWnd, reinterpret_cast<TfrmAbout*>(lParam))).first;
		reinterpret_cast<TfrmAbout*>(lParam)->m_hWnd = hWnd;
		return wnd->second->wmInitdialog(wParam, lParam);
	}
	wnd=_HandleMapping.find(hWnd);
	if (wnd==_HandleMapping.end()) {	// something screwed up
		return FALSE;					//dialog! do not use ::DefWindowProc(hWnd, msg, wParam, lParam);
	}

	switch (msg)
	{
		// case WM_INITDIALOG:	done on top
		case WM_COMMAND:
			return wnd->second->wmCommand(wParam, lParam);
			break;
		case WM_CLOSE:
			return wnd->second->wmClose(wParam, lParam);
			break;
		case WM_DESTROY:
			delete wnd->second;
			break;
	}
	return FALSE;
}

//---------------------------------------------------------------------------
//WM_INITDIALOG:
LRESULT TfrmAbout::wmInitdialog(WPARAM wParam, LPARAM lParam) {
	char* pszMsg = NULL;
	HRSRC hResInfo;
	DWORD ResSize;
	TCHAR oldTitle[256], newTitle[256];
	TCHAR* temp;
	TCHAR* pszTitle = NULL;
	// Headerbar
	TCHAR* pszPlug = mir_a2t(__PLUGIN_NAME);
	TCHAR* pszVer  = mir_a2t(__VERSION_STRING_DOTS);
	GetDlgItemText( m_hWnd, IDC_HEADERBAR, oldTitle, SIZEOF( oldTitle ));
	mir_sntprintf( newTitle, SIZEOF(newTitle), oldTitle, pszPlug, pszVer );
	mir_free(pszPlug);
	mir_free(pszVer);
	SetDlgItemText( m_hWnd, IDC_HEADERBAR, newTitle );
	SendMessage(GetDlgItem(m_hWnd, IDC_HEADERBAR), WM_SETICON, ICON_BIG, (LPARAM)Skin_GetIcon(ICO_COMMON_SSWINDOW1,1));

	//License
	{	mir_tcsadd(pszTitle ,_T(__COPYRIGHT));
		mir_tcsadd(pszTitle ,_T("\r\n\r\n"));

		hResInfo = FindResource(hInst,MAKEINTRESOURCE(IDR_LICENSE),_T("TEXT"));
		ResSize  = SizeofResource(hInst,hResInfo);
		pszMsg   = (char*)LockResource(LoadResource(hInst,hResInfo));
		temp = mir_a2t(pszMsg);
		temp [ResSize] = 0;			//LockResource is not NULL terminatet !!
		mir_tcsadd(pszTitle ,temp);
		mir_free(temp);
		SetDlgItemText(m_hWnd,IDC_LICENSE, pszTitle);
		mir_freeAndNil(pszTitle);
	}

	//Credit
	{
		hResInfo = FindResource(hInst,MAKEINTRESOURCE(IDR_CREDIT),_T("TEXT"));
		ResSize  = SizeofResource(hInst,hResInfo);
		pszMsg   = (char*)LockResource(LoadResource(hInst,hResInfo));
		temp = mir_a2t(pszMsg);
		temp [ResSize] = 0;			//LockResource is not NULL terminatet !!
		mir_tcsadd(pszTitle ,temp);
		mir_free(temp);
		SetDlgItemText(m_hWnd,IDC_CREDIT, pszTitle);
		mir_freeAndNil(pszTitle);
	}

	SendMessage(m_hWnd, WM_SETICON, ICON_BIG,	(LPARAM)Skin_GetIcon(ICO_COMMON_SSWINDOW1,1));
	SendMessage(m_hWnd, WM_SETICON, ICON_SMALL,	(LPARAM)Skin_GetIcon(ICO_COMMON_SSWINDOW2));

	//init controls
	btnPageClick();
	SendMessage(GetDlgItem(m_hWnd, IDA_CONTRIBLINK), BUTTONSETDEFAULT, 1, NULL);

	TranslateDialogDefault(m_hWnd);
	return FALSE;
}

//WM_COMMAND:
LRESULT TfrmAbout::wmCommand(WPARAM wParam, LPARAM lParam) {
	//---------------------------------------------------------------------------
	if (HIWORD(wParam) == BN_CLICKED) {
		switch(LOWORD(wParam)) {
			case IDCANCEL:
			case IDCLOSE:
				break;
			case IDA_btnClose:
				Close();
				break;
			case IDA_CONTRIBLINK:
				m_Page = m_Page ? 0 : 1;
				btnPageClick();
				break;
			default:
				break;
		}
	}
	return FALSE;
}

//WM_CLOSE:
LRESULT TfrmAbout::wmClose(WPARAM wParam, LPARAM lParam) {
	SendMessage(m_hWndOwner,UM_CLOSING, (WPARAM)m_hWnd, (LPARAM)IDD_UAboutForm);
	DestroyWindow(m_hWnd);
	return FALSE;
}

//---------------------------------------------------------------------------
// Standard konstruktor/destruktor
TfrmAbout::TfrmAbout(HWND Owner) {
	m_hWndOwner = Owner;
	// create window
	m_hWnd = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_UAboutForm),0, DlgTfrmAbout,(LPARAM)this);
	//register object
	_HandleMapping.insert(CHandleMapping::value_type(m_hWnd, this));
	//init page
	m_Page	= 1;
}

TfrmAbout::~TfrmAbout() {
	_HandleMapping.erase(m_hWnd);
}

//---------------------------------------------------------------------------
void TfrmAbout::btnPageClick() {
	HWND hCtrl = GetDlgItem(m_hWnd, IDA_CONTRIBLINK);
	if(!m_Page) {
		ShowWindow(GetDlgItem(m_hWnd, IDC_CREDIT), SW_HIDE);
		ShowWindow(GetDlgItem(m_hWnd, IDC_LICENSE), SW_SHOW);
		SendDlgItemMessage(m_hWnd, IDA_CONTRIBLINK, BUTTONADDTOOLTIP, (WPARAM)TranslateT("Credits >"), MBBF_TCHAR);
		HICON hIcon = Skin_GetIcon(ICO_COMMON_ARROWR);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? TranslateT("Credits") : TranslateT("Credits >"));
	}
	else {
		ShowWindow(GetDlgItem(m_hWnd, IDC_CREDIT), SW_SHOW);
		ShowWindow(GetDlgItem(m_hWnd, IDC_LICENSE), SW_HIDE);
		SendDlgItemMessage(m_hWnd, IDA_CONTRIBLINK, BUTTONADDTOOLTIP, (WPARAM)TranslateT("< Copyright"), MBBF_TCHAR);
		HICON hIcon = Skin_GetIcon(ICO_COMMON_ARROWL);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		SetWindowText(hCtrl, hIcon ? TranslateT("Copyright") : TranslateT("< Copyright"));
	}
}
