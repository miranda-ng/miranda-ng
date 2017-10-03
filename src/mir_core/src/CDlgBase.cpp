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

CDlgBase::CDlgBase(HINSTANCE hInst, int idDialog)
	: m_controls(1, CompareControlId),
	m_timers(1, CompareTimerId)
{
	m_hInst = hInst;
	m_idDialog = idDialog;
	m_hwnd = m_hwndParent = nullptr;
	m_isModal = m_initialized = m_bExiting = false;
	m_autoClose = CLOSE_ON_OK | CLOSE_ON_CANCEL;
	m_forceResizable = false;
}

CDlgBase::~CDlgBase()
{
	m_initialized = false; // prevent double call of destructor 
	if (m_hwnd)
		DestroyWindow(m_hwnd);
}

/////////////////////////////////////////////////////////////////////////////////////////
// methods

void CDlgBase::Close()
{
	::SendMessage(m_hwnd, WM_CLOSE, 0, 0);
}

void CDlgBase::Create()
{
	CreateDialogParam(m_hInst, MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)this);
}

int CDlgBase::DoModal()
{
	m_isModal = true;
	return DialogBoxParam(m_hInst, MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)this);
}

void CDlgBase::EndModal(INT_PTR nResult)
{
	::EndDialog(m_hwnd, nResult);
}

void CDlgBase::NotifyChange(void)
{
	if (m_hwndParent)
		SendMessage(m_hwndParent, PSM_CHANGED, (WPARAM)m_hwnd, 0);
}

void CDlgBase::Resize()
{
	SendMessage(m_hwnd, WM_SIZE, 0, 0);
}

void CDlgBase::SetCaption(const wchar_t *ptszCaption)
{
	if (m_hwnd && ptszCaption)
		SetWindowText(m_hwnd, ptszCaption);
}

void CDlgBase::Show(int nCmdShow)
{
	if (m_hwnd == nullptr)
		Create();
	ShowWindow(m_hwnd, nCmdShow);
}

/////////////////////////////////////////////////////////////////////////////////////////
// virtual methods

int CDlgBase::Resizer(UTILRESIZECONTROL*)
{
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

INT_PTR CDlgBase::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		m_initialized = false;
		TranslateDialog_LP(m_hwnd, GetPluginLangByInstance(m_hInst));

		NotifyControls(&CCtrlBase::OnInit);
		OnInitDialog();

		m_initialized = true;
		return TRUE;

	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORSTATIC:
		if (CCtrlBase *ctrl = FindControl(HWND(lParam))) {
			if (ctrl->m_bUseSystemColors) {
				SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
				return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
			}
		}
		break;

	case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *param = (MEASUREITEMSTRUCT *)lParam;
			if (param && param->CtlID)
				if (CCtrlBase *ctrl = FindControl(param->CtlID))
					return ctrl->OnMeasureItem(param);
		}
		return FALSE;

	case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *param = (DRAWITEMSTRUCT *)lParam;
			if (param && param->CtlID)
				if (CCtrlBase *ctrl = FindControl(param->CtlID))
					return ctrl->OnDrawItem(param);
		}
		return FALSE;

	case WM_DELETEITEM:
		{
			DELETEITEMSTRUCT *param = (DELETEITEMSTRUCT *)lParam;
			if (param && param->CtlID)
				if (CCtrlBase *ctrl = FindControl(param->CtlID))
					return ctrl->OnDeleteItem(param);
		}
		return FALSE;

	case WM_COMMAND:
		{
			HWND hwndCtrl = (HWND)lParam;
			WORD idCtrl = LOWORD(wParam);
			WORD idCode = HIWORD(wParam);
			if (CCtrlBase *ctrl = FindControl(idCtrl)) {
				BOOL result = ctrl->OnCommand(hwndCtrl, idCtrl, idCode);
				if (result != FALSE)
					return result;
			}

			if (idCode == BN_CLICKED) {
				// close dialog automatically if 'Cancel' button is pressed
				if (idCtrl == IDCANCEL && (m_autoClose & CLOSE_ON_CANCEL)) {
					m_bExiting = true;
					PostMessage(m_hwnd, WM_CLOSE, 0, 0);
				}

				// close dialog automatically if 'OK' button is pressed
				if (idCtrl == IDOK && (m_autoClose & CLOSE_ON_OK)) {
					// validate dialog data first
					m_bExiting = true;
					m_lresult = TRUE;
					NotifyControls(&CCtrlBase::OnApply);
					OnApply();

					// everything ok? good, let's close it
					if (m_lresult == TRUE)
						PostMessage(m_hwnd, WM_CLOSE, 0, 0);
					else
						m_bExiting = false;
				}
			}
		}
		return FALSE;

	case WM_NOTIFY:
		{
			int idCtrl = wParam;
			NMHDR *pnmh = (NMHDR *)lParam;
			if (pnmh->idFrom == 0) {
				if (pnmh->code == PSN_APPLY) {
					if (LPPSHNOTIFY(lParam)->lParam != 3) // IDC_APPLY
						m_bExiting = true;

					m_lresult = true;
					NotifyControls(&CCtrlBase::OnApply);
					if (m_lresult)
						OnApply();
				}
				else if (pnmh->code == PSN_RESET) {
					NotifyControls(&CCtrlBase::OnReset);
					OnReset();
				}
			}

			if (CCtrlBase *ctrl = FindControl(pnmh->idFrom))
				return ctrl->OnNotify(idCtrl, pnmh);
		}
		return FALSE;

	case WM_CONTEXTMENU:
		if (CCtrlBase *ctrl = FindControl(HWND(wParam)))
			ctrl->OnBuildMenu(ctrl);
		break;

	case WM_SIZE:
		if (m_forceResizable || (GetWindowLongPtr(m_hwnd, GWL_STYLE) & WS_THICKFRAME))
			Utils_ResizeDialog(m_hwnd, m_hInst, MAKEINTRESOURCEA(m_idDialog), GlobalDlgResizer);
		return TRUE;

	case WM_TIMER:
		if (CTimer *timer = FindTimer(wParam))
			return timer->OnTimer();
		return FALSE;

	case WM_CLOSE:
		m_bExiting = true;
		m_lresult = FALSE;
		OnClose();
		if (!m_lresult) {
			if (m_isModal)
				EndModal(0);
			else
				DestroyWindow(m_hwnd);
		}
		return TRUE;

	case WM_DESTROY:
		m_bExiting = true;
		OnDestroy();
		NotifyControls(&CCtrlBase::OnDestroy);
		{
			mir_cslock lck(csDialogs);
			int idx = arDialogs.getIndex(this);
			if (idx != -1)
				arDialogs.remove(idx);
		}
		m_hwnd = nullptr;
		if (m_initialized) {
			if (m_isModal)
				m_isModal = false;
			else // modeless dialogs MUST be allocated with 'new'
				delete this;
		}

		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK CDlgBase::GlobalDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CDlgBase *wnd;
	if (msg == WM_INITDIALOG) {
		wnd = (CDlgBase*)lParam;
		wnd->m_hwnd = hwnd;

		mir_cslock lck(csDialogs);
		arDialogs.insert(wnd);
	}
	else wnd = CDlgBase::Find(hwnd);

	return (wnd == nullptr) ? FALSE : wnd->DlgProc(msg, wParam, lParam);
}

int CDlgBase::GlobalDlgResizer(HWND hwnd, LPARAM, UTILRESIZECONTROL *urc)
{
	CDlgBase *wnd = CDlgBase::Find(hwnd);
	return (wnd == nullptr) ? 0 : wnd->Resizer(urc);
}

void CDlgBase::ThemeDialogBackground(BOOL tabbed)
{
	EnableThemeDialogTexture(m_hwnd, (tabbed ? ETDT_ENABLE : ETDT_DISABLE) | ETDT_USETABTEXTURE);
}

void CDlgBase::AddControl(CCtrlBase *ctrl)
{
	m_controls.insert(ctrl);
}

void CDlgBase::NotifyControls(void (CCtrlBase::*fn)())
{
	for (int i = 0; i < m_controls.getCount(); i++)
		(m_controls[i]->*fn)();
}

CCtrlBase* CDlgBase::FindControl(int idCtrl)
{
	CCtrlBase search(nullptr, idCtrl);
	return m_controls.find(&search);
}

CCtrlBase* CDlgBase::FindControl(HWND hwnd)
{
	for (int i = 0; i < m_controls.getCount(); i++)
		if (m_controls[i]->GetHwnd() == hwnd)
			return m_controls[i];

	return nullptr;
}

void CDlgBase::AddTimer(CTimer *timer)
{
	m_timers.insert(timer);
}

CTimer* CDlgBase::FindTimer(int idEvent)
{
	CTimer search(nullptr, idEvent);
	return m_timers.find(&search);
}

CDlgBase* CDlgBase::Find(HWND hwnd)
{
	PVOID bullshit[2]; // vfptr + hwnd
	bullshit[1] = hwnd;
	return arDialogs.find((CDlgBase*)&bullshit);
}
