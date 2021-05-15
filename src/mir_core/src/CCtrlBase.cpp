/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-21 Miranda NG team

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

#include "stdafx.h"

static mir_cs csCtrl;

static int CompareControls(const CCtrlBase *p1, const CCtrlBase *p2)
{
	return (INT_PTR)p1->GetHwnd() - (INT_PTR)p2->GetHwnd();
}
static LIST<CCtrlBase> arControls(10, CompareControls);

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlBase

CCtrlBase::CCtrlBase(CDlgBase *wnd, int idCtrl) :
	m_parentWnd(wnd),
	m_idCtrl(idCtrl)
{
	if (wnd)
		wnd->AddControl(this);
}

CCtrlBase::~CCtrlBase()
{
}

void CCtrlBase::OnInit()
{
	m_hwnd = (m_idCtrl && m_parentWnd && m_parentWnd->GetHwnd()) ? GetDlgItem(m_parentWnd->GetHwnd(), m_idCtrl) : nullptr;
}

void CCtrlBase::OnDestroy()
{
	PVOID bullshit[2];  // vfptr + hwnd
	bullshit[1] = m_hwnd;
	CCtrlBase *pCtrl = arControls.find((CCtrlBase*)&bullshit);
	if (pCtrl) {
		pCtrl->Unsubclass();
		arControls.remove(pCtrl);
	}

	m_hwnd = nullptr;
}

bool CCtrlBase::OnApply()
{
	m_bChanged = false;
	return true;
}

void CCtrlBase::OnReset()
{}

void CCtrlBase::Show(bool bShow)
{
	::ShowWindow(m_hwnd, bShow ? SW_SHOW : SW_HIDE);
}

void CCtrlBase::Enable(bool bIsEnable)
{
	::EnableWindow(m_hwnd, bIsEnable);
}

bool CCtrlBase::Enabled() const
{
	return (m_hwnd) ? IsWindowEnabled(m_hwnd) != 0 : false;
}

void CCtrlBase::NotifyChange()
{
	if (!m_parentWnd)
		return;

	if (m_parentWnd->IsInitialized()) {
		m_bChanged = true;
		if (!m_bSilent)
			m_parentWnd->NotifyChange();
	}

	OnChange(this);
}

LRESULT CCtrlBase::SendMsg(UINT Msg, WPARAM wParam, LPARAM lParam) const
{
	return ::SendMessage(m_hwnd, Msg, wParam, lParam);
}

void CCtrlBase::SetText(const wchar_t *text)
{
	::SetWindowText(m_hwnd, text);
}

void CCtrlBase::SetTextA(const char *text)
{
	::SetWindowTextA(m_hwnd, text);
}

void CCtrlBase::SetDraw(bool bEnable)
{
	::SendMessage(m_hwnd, WM_SETREDRAW, bEnable, 0);
}

void CCtrlBase::SetInt(int value)
{
	wchar_t buf[32] = { 0 };
	mir_snwprintf(buf, L"%d", value);
	SetWindowText(m_hwnd, buf);
}

wchar_t* CCtrlBase::GetText() const
{
	int length = GetWindowTextLengthW(m_hwnd);
	wchar_t *result = (wchar_t *)mir_alloc((length+1) * sizeof(wchar_t));
	if (length)
		GetWindowTextW(m_hwnd, result, length+1);
	result[length] = 0;
	return result;
}

char* CCtrlBase::GetTextA() const
{
	int length = GetWindowTextLengthA(m_hwnd);
	char *result = (char *)mir_alloc((length+1) * sizeof(char));
	if (length)
		GetWindowTextA(m_hwnd, result, length+1);
	result[length] = 0;
	return result;
}

char* CCtrlBase::GetTextU() const
{
	return mir_utf8encodeW(ptrW(GetText()));
}

wchar_t* CCtrlBase::GetText(wchar_t *buf, size_t size) const
{
	GetWindowTextW(m_hwnd, buf, (int)size);
	buf[size - 1] = 0;
	return buf;
}

char* CCtrlBase::GetTextA(char *buf, size_t size) const
{
	GetWindowTextA(m_hwnd, buf, (int)size);
	buf[size - 1] = 0;
	return buf;
}

char* CCtrlBase::GetTextU(char *buf, size_t size) const
{
	ptrW wszText(GetText());
	strncpy_s(buf, size, T2Utf(wszText), _TRUNCATE);
	return buf;
}

int CCtrlBase::GetInt() const
{
	int length = GetWindowTextLengthW(m_hwnd) + 1;
	wchar_t *result = (wchar_t *)_alloca(length * sizeof(wchar_t));
	GetWindowTextW(m_hwnd, result, length);
	return _wtoi(result);
}

void CCtrlBase::GetCaretPos(CContextMenuPos &pos) const
{
	pos.pCtrl = this;
	pos.iCurr = -1;

	if (pos.pt.x == 0 && pos.pt.y == 0)
		GetCursorPos(&pos.pt);
}

LRESULT CCtrlBase::CustomWndProc(UINT, WPARAM, LPARAM)
{
	return FALSE;
}

LRESULT CALLBACK CCtrlBase::GlobalSubclassWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PVOID bullshit[2];  // vfptr + hwnd
	bullshit[1] = hwnd;
	CCtrlBase *pCtrl = arControls.find((CCtrlBase*)&bullshit);
	if (pCtrl) {
		LRESULT res = pCtrl->CustomWndProc(msg, wParam, lParam);
		if (res != 0)
			return res;
	}

	return mir_callNextSubclass(hwnd, GlobalSubclassWndProc, msg, wParam, lParam);
}

void CCtrlBase::Subclass()
{
	mir_subclassWindow(m_hwnd, GlobalSubclassWndProc);

	mir_cslock lck(csCtrl);
	arControls.insert(this);
}

void CCtrlBase::Unsubclass()
{
	mir_unsubclassWindow(m_hwnd, GlobalSubclassWndProc);
}
