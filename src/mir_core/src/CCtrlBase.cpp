/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (c) 2012-17 Miranda NG project

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

CCtrlBase::CCtrlBase(CDlgBase *wnd, int idCtrl)
	: m_parentWnd(wnd),
	m_idCtrl(idCtrl),
	m_hwnd(nullptr),
	m_bChanged(false),
	m_bSilent(false),
	m_bUseSystemColors(false)
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

void CCtrlBase::OnApply()
{
	m_bChanged = false;
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
	if (!m_parentWnd || m_parentWnd->IsInitialized())
		m_bChanged = true;

	if (m_parentWnd && !m_bSilent) {
		m_parentWnd->OnChange(this);
		if (m_parentWnd->IsInitialized())
			::SendMessage(::GetParent(m_parentWnd->GetHwnd()), PSM_CHANGED, 0, 0);
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

void CCtrlBase::SetInt(int value)
{
	wchar_t buf[32] = { 0 };
	mir_snwprintf(buf, L"%d", value);
	SetWindowText(m_hwnd, buf);
}

wchar_t* CCtrlBase::GetText()
{
	int length = GetWindowTextLength(m_hwnd) + 1;
	wchar_t *result = (wchar_t *)mir_alloc(length * sizeof(wchar_t));
	GetWindowText(m_hwnd, result, length);
	return result;
}

char* CCtrlBase::GetTextA()
{
	int length = GetWindowTextLength(m_hwnd) + 1;
	char *result = (char *)mir_alloc(length * sizeof(char));
	GetWindowTextA(m_hwnd, result, length);
	return result;
}

wchar_t* CCtrlBase::GetText(wchar_t *buf, int size)
{
	GetWindowText(m_hwnd, buf, size);
	buf[size - 1] = 0;
	return buf;
}

char* CCtrlBase::GetTextA(char *buf, int size)
{
	GetWindowTextA(m_hwnd, buf, size);
	buf[size - 1] = 0;
	return buf;
}

int CCtrlBase::GetInt()
{
	int length = GetWindowTextLength(m_hwnd) + 1;
	wchar_t *result = (wchar_t *)_alloca(length * sizeof(wchar_t));
	GetWindowText(m_hwnd, result, length);
	return _wtoi(result);
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
