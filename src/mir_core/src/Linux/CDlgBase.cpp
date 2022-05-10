/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-22 Miranda NG team

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

static mir_cs csDialogs;

static int CompareDialogs(const CDlgBase *p1, const CDlgBase *p2)
{
	return (INT_PTR)p1->GetHwnd() - (INT_PTR)p2->GetHwnd();
}
static LIST<CDlgBase> arDialogs(10, CompareDialogs);

#pragma comment(lib, "uxtheme")

/////////////////////////////////////////////////////////////////////////////////////////
// CDlgBase

static int CompareControlId(const CCtrlBase *c1, const CCtrlBase *c2)
{
	return c1->GetCtrlId() - c2->GetCtrlId();
}

static int CompareTimerId(const CTimer *t1, const CTimer *t2)
{
	return t1->GetEventId() - t2->GetEventId();
}

CDlgBase::CDlgBase(CMPluginBase &pPlug, int idDialog)
	: m_controls(1, CompareControlId),
	m_timers(1, CompareTimerId),
	m_pPlugin(pPlug)
{
	m_idDialog = idDialog;
	m_autoClose = CLOSE_ON_OK | CLOSE_ON_CANCEL;
}

CDlgBase::~CDlgBase()
{
	m_bInitialized = false; // prevent double call of destructor 
	// if (m_hwnd)
	// 	DestroyWindow(m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// events

bool CDlgBase::OnInitDialog()
{
	return true;
}

bool CDlgBase::OnClose()
{
	return true;
}

bool CDlgBase::OnApply()
{
	return true;
}

void CDlgBase::OnChange()
{}

void CDlgBase::OnDestroy()
{}

void CDlgBase::OnReset()
{}

void CDlgBase::OnTimer(CTimer*)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// methods

void CDlgBase::Close()
{
	// ::SendMessage(m_hwnd, WM_CLOSE, 0, 0);
}

void CDlgBase::Create()
{
	// CreateDialogParam(GetInst(), MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)this);
}

int CDlgBase::DoModal()
{
	m_isModal = true;
	// return DialogBoxParam(GetInst(), MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)this);
	return 0; //!!!!!!!!
}

void CDlgBase::EndModal(INT_PTR nResult)
{
	// ::EndDialog(m_hwnd, nResult);
}

HINSTANCE CDlgBase::GetInst() const
{
	return m_pPlugin.getInst();
}

void CDlgBase::NotifyChange(void)
{
	if (!m_bInitialized)
		return;

	OnChange();

	// if (m_hwndParent)
	// 	SendMessage(m_hwndParent, PSM_CHANGED, (WPARAM)m_hwnd, 0);
}

void CDlgBase::Resize()
{
	// SendMessage(m_hwnd, WM_SIZE, 0, 0);
}

void CDlgBase::SetCaption(const wchar_t *ptszCaption)
{
	// if (m_hwnd && ptszCaption)
	//	SetText(ptszCaption);
}

void CDlgBase::SetDraw(bool bEnable)
{
	// ::SendMessage(m_hwnd, WM_SETREDRAW, bEnable, 0);
}

void CDlgBase::Show(int nCmdShow)
{
	if (m_hwnd == nullptr)
		Create();
	// ShowWindow(m_hwnd, nCmdShow);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDlgBase::CreateLink(CCtrlData& ctrl, const char *szSetting, uint8_t type, uint32_t iValue)
{
	ctrl.CreateDbLink(m_pPlugin.getModule(), szSetting, type, iValue);
}

void CDlgBase::CreateLink(CCtrlData& ctrl, const char *szSetting, wchar_t *szValue)
{
	ctrl.CreateDbLink(m_pPlugin.getModule(), szSetting, szValue);
}

/////////////////////////////////////////////////////////////////////////////////////////
// virtual methods

int CDlgBase::Resizer(UTILRESIZECONTROL*)
{
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

BOOL CALLBACK CDlgBase::GlobalFieldEnum(HWND hwnd, LPARAM lParam)
{
	return TRUE;
}

INT_PTR CDlgBase::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

int CDlgBase::GlobalDlgResizer(HWND hwnd, LPARAM, UTILRESIZECONTROL *urc)
{
	CDlgBase *wnd = CDlgBase::Find(hwnd);
	return (wnd == nullptr) ? 0 : wnd->Resizer(urc);
}

void CDlgBase::ThemeDialogBackground(BOOL tabbed)
{
}

void CDlgBase::AddControl(CCtrlBase *ctrl)
{
	m_controls.insert(ctrl);
}

void CDlgBase::RemoveControl(CCtrlBase *ctrl)
{
	m_controls.remove(ctrl);
}

void CDlgBase::NotifyControls(void (CCtrlBase::*fn)())
{
	for (auto &it : m_controls)
		(it->*fn)();
}

bool CDlgBase::VerifyControls(bool (CCtrlBase::*fn)())
{
	for (auto &it : m_controls)
		if (!(it->*fn)())
			return false;

	return true;
}

CCtrlBase* CDlgBase::FindControl(int idCtrl)
{
	CCtrlBase search(nullptr, idCtrl);
	return m_controls.find(&search);
}

CCtrlBase* CDlgBase::FindControl(HWND hwnd)
{
	for (auto &it : m_controls)
		if (it->GetHwnd() == hwnd)
			return it;

	return nullptr;
}

void CDlgBase::AddTimer(CTimer *timer)
{
	m_timers.insert(timer);
}

void CDlgBase::RemoveTimer(UINT_PTR idEvent)
{
	CTimer search(nullptr, idEvent);
	m_timers.remove(&search);
}

CTimer* CDlgBase::FindTimer(int idEvent)
{
	CTimer search(nullptr, idEvent);
	return m_timers.find(&search);
}

CDlgBase* CDlgBase::Find(HWND hwnd)
{
	void *bullshit[2]; // vfptr + hwnd
	bullshit[1] = hwnd;
	return arDialogs.find((CDlgBase*)&bullshit);
}
