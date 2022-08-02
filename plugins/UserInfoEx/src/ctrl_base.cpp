/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "stdafx.h"

/***********************************************************************************************************
 * Old methods for setting text color of dialog controls
 ***********************************************************************************************************/

COLORREF clrBoth = -1;
COLORREF clrChanged = -1;
COLORREF clrCustom = -1;
COLORREF clrNormal = -1;
COLORREF clrMeta = -1;

void Ctrl_InitTextColours()
{
	clrBoth = g_plugin.clrBoth;
	clrChanged = g_plugin.clrChanged;
	clrCustom = g_plugin.clrCustom;
	clrNormal = g_plugin.clrNormal;
	clrMeta = g_plugin.clrMeta;
}

INT_PTR CALLBACK Ctrl_SetTextColour(HDC hdc, uint16_t wFlags)
{
	// OLD stuff
	SetTextColor(hdc, 
		(wFlags & CTRLF_CHANGED) 
			? clrChanged : ((wFlags & CTRLF_HASCUSTOM) && (wFlags & (CTRLF_HASPROTO|CTRLF_HASMETA)))
				? clrBoth : (wFlags & CTRLF_HASMETA)
					? clrMeta : (wFlags & CTRLF_HASCUSTOM)
						? clrCustom	: clrNormal
 );
	return (INT_PTR)GetStockObject(WHITE_BRUSH);
}

INT_PTR CALLBACK Ctrl_SetTextColour(HWND hCtrl, HDC hdc)
{
	if (hCtrl && g_plugin.bShowColours) 
	{
		LPCTRL pCtrl = (LPCTRL)GetUserData(hCtrl);
		if (PtrIsValid(pCtrl))
			return Ctrl_SetTextColour(hdc, pCtrl->wFlags);
	}
	return FALSE;
}

/***********************************************************************************************************
 * Implementation of CBaseCtrl
 ***********************************************************************************************************/

/**
 *
 *
 **/
CBaseCtrl::CBaseCtrl()
{
	memset(this, 0, sizeof(*this));
	_cbSize = sizeof(CBaseCtrl);
}

/**
 *
 *
 **/
CBaseCtrl::CBaseCtrl(HWND hDlg, uint16_t idCtrl, LPCSTR pszSetting)
{
	memset(this, 0, sizeof(*this));
	_cbSize = sizeof(CBaseCtrl);
	_hwnd = GetDlgItem(hDlg, idCtrl);
	if (!IsWindow(_hwnd)) throw;
	_idCtrl = idCtrl;
	_pszModule = USERINFO;
	_pszSetting = pszSetting;
	SetUserData(_hwnd, this);
}

/**
 *
 *
 **/
CBaseCtrl::CBaseCtrl(HWND hDlg, uint16_t idCtrl, LPCSTR pszModule, LPCSTR pszSetting)
{
	memset(this, 0, sizeof(*this));
	_cbSize		= sizeof(CBaseCtrl);
	_hwnd		= GetDlgItem(hDlg, idCtrl);
	if (!IsWindow(_hwnd)) throw;
	_idCtrl		= idCtrl;
	_pszModule	= pszModule;
	_pszSetting	= pszSetting;
	SetUserData(_hwnd, this);
}


/**
 *
 *
 **/
CBaseCtrl::~CBaseCtrl()
{
	SetUserData(_hwnd, NULL);
	MIR_FREE(_pszValue);
}

/**
 *
 *
 **/
INT_PTR CBaseCtrl::OnSetTextColour(HDC hdc)
{
	SetTextColor(hdc, 
		(_Flags.B.hasChanged) 
		? clrChanged : ((_Flags.B.hasCustom) && (_Flags.B.hasProto || _Flags.B.hasMeta))
				? clrBoth : _Flags.B.hasMeta
					? clrMeta : _Flags.B.hasCustom
						? clrCustom	: clrNormal
 );
	return (INT_PTR)GetStockObject(WHITE_BRUSH);
}

/***********************************************************************************************************
 * Implementation of CCtrlList
 ***********************************************************************************************************/

/**
 *
 *
 **/
CCtrlList* CCtrlList::CreateObj(HWND hOwnerDlg)
{
	Ctrl_InitTextColours();
	return new CCtrlList(hOwnerDlg);
}

/**
 *
 *
 **/
int CCtrlList::sortFunc(const CBaseCtrl *p1, const CBaseCtrl *p2)
{
	return p1->_idCtrl - p2->_idCtrl;
}

/**
 *
 *
 **/
CCtrlList::CCtrlList(HWND hOwnerDlg) :
	LIST<CBaseCtrl>(10, &CCtrlList::sortFunc)
{
	_hOwnerDlg = hOwnerDlg; 
	SetUserData(_hOwnerDlg, this);
}

/**
 *
 *
 **/
CCtrlList::~CCtrlList()
{
	SetUserData(_hOwnerDlg, NULL);

	// delete data
	for (int i = 0 ; i < count; i++)
		delete (*this)[i];

	// delete the list
	LIST<CBaseCtrl>::destroy();
}

/**
 *
 *
 **/
void CCtrlList::Release()
{ 
	delete this; 
}

/**
 *
 *
 **/
void CCtrlList::OnReset()
{
	for (int i = 0; i < count; i++)
		if (items[i]) 
			items[i]->OnReset();
}

/**
 *
 *
 **/
BOOL CCtrlList::OnInfoChanged(MCONTACT hContact, LPCSTR pszProto)
{
	BOOL bChanged = 0;

	for (int i = 0; i < count; i++)
		if (PtrIsValid(items[i]))
			bChanged |= items[i]->OnInfoChanged(hContact, pszProto);

	return bChanged;
}

/**
 *
 *
 **/
void CCtrlList::OnApply(MCONTACT hContact, LPCSTR pszProto)
{
	for (int i = 0; i < count; i++)
		if (PtrIsValid(items[i]))
			items[i]->OnApply(hContact, pszProto);
}

/**
 *
 *
 **/
void CCtrlList::OnChangedByUser(uint16_t idCtrl, uint16_t wChangedMsg)
{
	// prefilter messages to avoid unessesary search operations
	switch (wChangedMsg) {
	case EN_UPDATE:
	case EN_CHANGE:
	case CBN_SELCHANGE:
		CBaseCtrl *pResult = CBaseCtrl::GetObj(_hOwnerDlg, idCtrl);
		if (PtrIsValid(pResult) && (pResult->_cbSize == sizeof(CBaseCtrl)))
			pResult->OnChangedByUser(wChangedMsg);
	}
}

/**
 *
 *
 **/
INT_PTR CCtrlList::OnSetTextColour(HWND hCtrl, HDC hdc)
{
	if (IsWindow(hCtrl) && myGlobals.ShowPropsheetColours) {
		CBaseCtrl* pCtrl = CBaseCtrl::GetObj(hCtrl);
		if (PtrIsValid(pCtrl) && (pCtrl->_cbSize == sizeof(CBaseCtrl)))
			return pCtrl->OnSetTextColour(hdc);
	}
	return FALSE;
}
