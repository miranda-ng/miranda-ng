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
#include "diatheme.h"

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

CDlgBase::CDlgBase(CMPluginBase &pPlug, int idDialog) :
	m_controls(1, CompareControlId),
	m_timers(1, CompareTimerId),
	m_pPlugin(pPlug),
	m_bFixedSize(!g_bEnableDpiAware)
{
	m_idDialog = idDialog;
	m_autoClose = CLOSE_ON_OK | CLOSE_ON_CANCEL;
}

CDlgBase::~CDlgBase()
{
	m_bInitialized = false; // prevent double call of destructor 
	if (m_hwnd)
		DestroyWindow(m_hwnd);
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
	::SendMessage(m_hwnd, WM_CLOSE, 0, 0);
}

void CDlgBase::Create()
{
	if (!m_bFixedSize) {
		mir_ptr<DLGTEMPLATE> pDlgTemplate(LoadThemedDialogTemplate(MAKEINTRESOURCE(m_idDialog), GetInst()));
		CreateDialogIndirectParam(GetInst(), pDlgTemplate, m_hwndParent, GlobalDlgProc, (LPARAM)this);
	}
	else CreateDialogParam(GetInst(), MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)this);
}

int CDlgBase::DoModal()
{
	m_isModal = true;
	
	if (m_bFixedSize)
		return DialogBoxParam(GetInst(), MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)this);
		
	mir_ptr<DLGTEMPLATE> pDlgTemplate(LoadThemedDialogTemplate(MAKEINTRESOURCE(m_idDialog), GetInst()));
	return DialogBoxIndirectParam(GetInst(), pDlgTemplate, m_hwndParent, GlobalDlgProc, (LPARAM)this);
}

void CDlgBase::EndModal(INT_PTR nResult)
{
	::EndDialog(m_hwnd, nResult);
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

void CDlgBase::SetDraw(bool bEnable)
{
	::SendMessage(m_hwnd, WM_SETREDRAW, bEnable, 0);
}

void CDlgBase::Show(int nCmdShow)
{
	if (m_hwnd == nullptr)
		Create();
	ShowWindow(m_hwnd, nCmdShow);
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

int CDlgBase::GlobalDlgResizer(HWND hwnd, LPARAM, UTILRESIZECONTROL *urc)
{
	CDlgBase *wnd = CDlgBase::Find(hwnd);
	return (wnd == nullptr) ? 0 : wnd->Resizer(urc);
}

void CDlgBase::OnResize()
{
	if (m_forceResizable || (GetWindowLongPtr(m_hwnd, GWL_STYLE) & WS_THICKFRAME))
		Utils_ResizeDialog(m_hwnd, m_pPlugin.getInst(), MAKEINTRESOURCEA(m_idDialog), GlobalDlgResizer);
}

int CDlgBase::Resizer(UTILRESIZECONTROL*)
{
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

BOOL CALLBACK CDlgBase::GlobalFieldEnum(HWND hwnd, LPARAM lParam)
{
	CDlgBase *pDlg = (CDlgBase*)lParam;
	int id = GetWindowLongPtrW(hwnd, GWLP_ID);
	if (id <= 0)
		return TRUE;

	// already declared inside the class? skipping
	CCtrlBase *ctrl = pDlg->FindControl(id);
	if (ctrl != nullptr)
		return TRUE;

	wchar_t wszClass[100];
	GetClassNameW(hwnd, wszClass, _countof(wszClass));
	if (!wcsicmp(wszClass, L"Static"))
		new CCtrlLabel(pDlg, id);
	if (!wcsicmp(wszClass, L"Edit"))
		new CCtrlEdit(pDlg, id);
	else if (!wcsicmp(wszClass, L"ComboBox"))
		new CCtrlCombo(pDlg, id);
	else if (!wcsicmp(wszClass, L"Button")) {
		switch (GetWindowLongW(hwnd, GWL_STYLE) & (BS_CHECKBOX | BS_RADIOBUTTON | BS_AUTOCHECKBOX | BS_AUTORADIOBUTTON)) {
		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
			new CCtrlCheck(pDlg, id);
			break;

		default:
			new CCtrlButton(pDlg, id);
		}
	}
	else if (!wcsicmp(wszClass, L"RichEdit50W"))
		new CCtrlRichEdit(pDlg, id);
	else if (!wcsicmp(wszClass, L"msctls_updown32"))
		new CCtrlSpin(pDlg, id);

	return TRUE;
}

INT_PTR CDlgBase::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		m_bInitialized = m_bSucceeded = false;
		TranslateDialog_LP(m_hwnd, &m_pPlugin);

		::EnumChildWindows(m_hwnd, &GlobalFieldEnum, LPARAM(this));

		NotifyControls(&CCtrlBase::OnInit);
		if (!OnInitDialog())
			return FALSE;

		for (auto &it : m_controls)
			if (it->m_bNotifiable)
				it->OnChange(it);

		m_bInitialized = true;
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

	case WM_GETMINMAXINFO:
		if (m_iMinHeight != -1 && m_iMinWidth != -1) {
			MINMAXINFO *lpmmi = (MINMAXINFO*)lParam;
			lpmmi->ptMinTrackSize.y = m_iMinHeight;
			lpmmi->ptMinTrackSize.x = m_iMinWidth;
			return 0;
		}
		break;

	case WM_MEASUREITEM:
		if (!Menu_MeasureItem(lParam)) {
			MEASUREITEMSTRUCT *param = (MEASUREITEMSTRUCT *)lParam;
			if (param && param->CtlID)
				if (CCtrlBase *ctrl = FindControl(param->CtlID))
					return ctrl->OnMeasureItem(param);
		}
		return FALSE;

	case WM_DRAWITEM:
		if (!Menu_DrawItem(lParam)) {
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
			uint16_t idCtrl = LOWORD(wParam);
			uint16_t idCode = HIWORD(wParam);
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
					if (VerifyControls(&CCtrlBase::OnApply)) {
						m_bExiting = true;

						// everything ok? good, let's close it
						if (OnApply()) {
							m_bSucceeded = true;
							PostMessage(m_hwnd, WM_CLOSE, 0, 0);
						}
						else m_bExiting = false;
					}
				}
			}
		}
		return FALSE;

	case WM_NOTIFY:
		{
			int idCtrl = wParam;
			NMHDR *pnmh = (NMHDR *)lParam;
			if (pnmh->idFrom == 0) {
				switch (pnmh->code) {
				case PSN_APPLY:
					if (LPPSHNOTIFY(lParam)->lParam != 3) // IDC_APPLY
						m_bExiting = true;

					if (!VerifyControls(&CCtrlBase::OnApply))
						m_bExiting = false;
					else if (!OnApply())
						m_bExiting = false;
					break;

				case PSN_RESET:
					NotifyControls(&CCtrlBase::OnReset);
					OnReset();
					break;

				case PSN_WIZFINISH:
					m_OnFinishWizard(this);
					break;
				}
			}

			if (CCtrlBase *ctrl = FindControl(pnmh->idFrom))
				return ctrl->OnNotify(idCtrl, pnmh);
		}
		return FALSE;

	case WM_HSCROLL:
		if (auto *pCtrl = FindControl(HWND(lParam)))
			pCtrl->OnCommand(HWND(lParam), pCtrl->m_idCtrl, WM_HSCROLL);
		break;

	case PSM_CHANGED:
		if (m_bInitialized)
			OnChange();
		break;

	case WM_CONTEXTMENU:
		if (CCtrlBase *ctrl = FindControl(HWND(wParam))) {
			CContextMenuPos pos = {};
			if (lParam != -1) {
				pos.pt.x = GET_X_LPARAM(lParam);
				pos.pt.y = GET_Y_LPARAM(lParam);
			}
			ctrl->GetCaretPos(pos);
			ctrl->OnBuildMenu(&pos);
		}
		break;

	case WM_SIZE:
		OnResize();
		return TRUE;

	case WM_TIMER:
		if (CTimer *timer = FindTimer(wParam))
			return timer->OnTimer();
		return FALSE;

	case WM_CLOSE:
		if (OnClose()) {
			m_bExiting = true;
			if (m_isModal)
				EndModal(m_bSucceeded ? IDOK : FALSE);
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
		if (m_bInitialized) {
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

void CDlgBase::ThemeDialogBackground(BOOL tabbed)
{
	EnableThemeDialogTexture(m_hwnd, (tabbed ? ETDT_ENABLE : ETDT_DISABLE) | ETDT_USETABTEXTURE);
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
	PVOID bullshit[2]; // vfptr + hwnd
	bullshit[1] = hwnd;
	return arDialogs.find((CDlgBase*)&bullshit);
}
