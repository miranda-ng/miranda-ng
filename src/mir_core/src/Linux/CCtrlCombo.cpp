/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-24 Miranda NG team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "../stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCombo class

CCtrlCombo::CCtrlCombo(CDlgBase *dlg, int ctrlId)
	: CCtrlData(dlg, ctrlId)
{}

BOOL CCtrlCombo::OnCommand(MWindow, uint16_t, uint16_t idCode)
{
	switch (idCode) {
	// case CBN_CLOSEUP:  OnCloseup(this); break;
	// case CBN_DROPDOWN: OnDropdown(this); break;
	// case CBN_SELCHANGE: OnSelChanged(this); break;
	// case CBN_KILLFOCUS: OnKillFocus(this); break;

	// case CBN_EDITCHANGE:
	// case CBN_EDITUPDATE:
	// case CBN_SELENDOK:
	// 		NotifyChange();
		break;
	}
	return TRUE;
}

void CCtrlCombo::OnInit()
{
	CSuper::OnInit();
	OnReset();
}

bool CCtrlCombo::OnApply()
{
	CSuper::OnApply();

	if (GetDataType() == DBVT_WCHAR) {
	// 	int len = GetWindowTextLength(m_hwnd) + 1;
	// 	wchar_t *buf = (wchar_t *)_alloca(sizeof(wchar_t) * len);
	// 	GetWindowText(m_hwnd, buf, len);
	// 	SaveText(buf);
	}
	else if (GetDataType() != DBVT_DELETED) {
	 	SaveInt(GetInt());
	}
	return true;
}

void CCtrlCombo::OnReset()
{
	if (GetDataType() == DBVT_WCHAR)
		SetText(LoadText());
	else if (GetDataType() != DBVT_DELETED)
		SetInt(LoadInt());
}

LPARAM CCtrlCombo::GetCurData() const
{
	return GetItemData(GetCurSel());
}

// selects line with userdata passed
int CCtrlCombo::SelectData(LPARAM data)
{
	int ret = -1, nCount = GetCount();

	for (int i = 0; i < nCount; i++)
		if (GetItemData(i) == data) {
			ret = i;
			break;
		}

	return SetCurSel(ret);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Windows API

int CCtrlCombo::AddString(const wchar_t *text, LPARAM data)
{
	return -1;
}

int CCtrlCombo::AddStringA(const char *text, LPARAM data)
{
	return -1;
}

void CCtrlCombo::DeleteString(int index)
{
}

int CCtrlCombo::FindString(const wchar_t *str, int index, bool exact)
{	return 0; 
}

int CCtrlCombo::FindStringA(const char *str, int index, bool exact)
{	return 0; 
}

int CCtrlCombo::GetCount() const
{	return 0; 
}

int CCtrlCombo::GetCurSel() const
{	return 0; 
}

bool CCtrlCombo::GetDroppedState() const
{	return 0; 
}

LPARAM CCtrlCombo::GetItemData(int index) const
{	return 0; 
}

wchar_t* CCtrlCombo::GetItemText(int index) const
{	return 0; 
}

wchar_t* CCtrlCombo::GetItemText(int index, wchar_t *buf, int size) const
{	return 0; 
}

int CCtrlCombo::InsertString(const wchar_t *text, int pos, LPARAM data)
{	return 0; 
}

void CCtrlCombo::ResetContent()
{
}

int CCtrlCombo::SelectString(const wchar_t *str)
{	return 0;
}

int CCtrlCombo::SetCurSel(int index)
{	return 0;
}

void CCtrlCombo::SetItemData(int index, LPARAM data)
{
}

void CCtrlCombo::ShowDropdown(bool show)
{
}
