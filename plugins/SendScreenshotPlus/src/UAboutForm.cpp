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
		return (INT_PTR)GetStockObject(WHITE_BRUSH); 	//GetSysColorBrush(COLOR_WINDOW);
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
	// Headerbar
	SendDlgItemMessage(m_hWnd, IDC_HEADERBAR, WM_SETICON, ICON_BIG, (LPARAM)GetIcon(ICO_MAIN));

	//License
	{
		TCHAR* pszText = NULL;
		mir_tstradd(pszText, _T(__COPYRIGHT));
		mir_tstradd(pszText, _T("\r\n\r\n"));

		HRSRC hRes = FindResource(g_hSendSS,MAKEINTRESOURCE(IDR_LICENSE),_T("TEXT"));
		DWORD size = SizeofResource(g_hSendSS,hRes);
		char* data = (char*)mir_alloc(size+1);
		memcpy(data,LockResource(LoadResource(g_hSendSS,hRes)),size);
		data[size] = '\0';
		TCHAR* pszCopyright = mir_a2t(data);
		mir_free(data);
		mir_tstradd(pszText, pszCopyright);
		mir_free(pszCopyright);
		SetDlgItemText(m_hWnd,IDC_LICENSE, pszText);
		mir_free(pszText);
	}

	//Credit
	{
		HRSRC hRes = FindResource(g_hSendSS,MAKEINTRESOURCE(IDR_CREDIT),_T("TEXT"));
		DWORD size = SizeofResource(g_hSendSS,hRes);
		char* data = (char*)mir_alloc(size+1);
		memcpy(data,LockResource(LoadResource(g_hSendSS,hRes)),size);
		data[size] = '\0';
		TCHAR* pszText = mir_a2t(data);
		mir_free(data);
		SetDlgItemText(m_hWnd,IDC_CREDIT, pszText);
		mir_free(pszText);
	}

	SendMessage(m_hWnd, WM_SETICON, ICON_BIG,	(LPARAM)GetIcon(ICO_MAIN));
	SendMessage(m_hWnd, WM_SETICON, ICON_SMALL,	(LPARAM)GetIcon(ICO_MAINXS));

	//init controls
	btnPageClick();

	TranslateDialogDefault(m_hWnd);
	return FALSE;
}

//WM_COMMAND:
LRESULT TfrmAbout::wmCommand(WPARAM wParam, LPARAM lParam) {
	//---------------------------------------------------------------------------
	if (HIWORD(wParam) == BN_CLICKED) {
		switch(LOWORD(wParam)) {
			case IDCANCEL: // ESC pressed
				this->Close();
				break;
			case IDA_btnClose:
				Close();
				break;
			case IDA_CONTRIBLINK:
				m_Page = !m_Page;
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
TfrmAbout::TfrmAbout(HWND Owner) {
	m_hWndOwner = Owner;
	m_Page = 1;
	// create window
	m_hWnd = CreateDialogParam(g_hSendSS, MAKEINTRESOURCE(IDD_UAboutForm),0, DlgTfrmAbout,(LPARAM)this);
	//register object
	_HandleMapping.insert(CHandleMapping::value_type(m_hWnd, this));
}

TfrmAbout::~TfrmAbout() {
	_HandleMapping.erase(m_hWnd);
}

//---------------------------------------------------------------------------
void TfrmAbout::btnPageClick() {
	HWND hCtrl = GetDlgItem(m_hWnd, IDA_CONTRIBLINK);
	const TCHAR* credits=TranslateT("Credits");
	const TCHAR* copyright=TranslateT("Copyright");
	const TCHAR* title;
	const TCHAR* button;
	if(!m_Page) {
		ShowWindow(GetDlgItem(m_hWnd, IDC_CREDIT), SW_HIDE);
		ShowWindow(GetDlgItem(m_hWnd, IDC_LICENSE), SW_SHOW);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIconBtn(ICO_BTN_ARROWR));
		title=copyright;
		button=credits;
	} else {
		ShowWindow(GetDlgItem(m_hWnd, IDC_CREDIT), SW_SHOW);
		ShowWindow(GetDlgItem(m_hWnd, IDC_LICENSE), SW_HIDE);
		SendMessage(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GetIconBtn(ICO_BTN_ARROWL));
		title=credits;
		button=copyright;
	}
	SetWindowText(hCtrl, button);
	TCHAR newTitle[128];
	TCHAR* pszPlug = mir_a2t(__PLUGIN_NAME);
	TCHAR* pszVer  = mir_a2t(__VERSION_STRING_DOTS);
	mir_sntprintf(newTitle,SIZEOF(newTitle), _T("%s - %s\nv%s"), pszPlug, title , pszVer);
	mir_free(pszPlug);
	mir_free(pszVer);
	SetDlgItemText(m_hWnd, IDC_HEADERBAR, newTitle);
	InvalidateRect(GetDlgItem(m_hWnd,IDC_HEADERBAR),NULL,1);
}
