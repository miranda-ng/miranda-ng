/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (ñ) 2012-15 Miranda NG project

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

#include "commonheaders.h"

#include <m_button.h>
#include <m_gui.h>
#include <m_icolib.h>
#include <m_skin.h>

static mir_cs csDialogs, csCtrl;

static int CompareDialogs(const CDlgBase *p1, const CDlgBase *p2)
{	return (INT_PTR)p1->GetHwnd() - (INT_PTR)p2->GetHwnd();
}
static LIST<CDlgBase> arDialogs(10, CompareDialogs);

static int CompareControls(const CCtrlBase *p1, const CCtrlBase *p2)
{	return (INT_PTR)p1->GetHwnd() - (INT_PTR)p2->GetHwnd();
}
static LIST<CCtrlBase> arControls(10, CompareControls);

#pragma comment(lib, "uxtheme")

CDlgBase::CDlgBase(HINSTANCE hInst, int idDialog)
	: m_controls(1, CCtrlBase::cmp)
{
	m_hInst = hInst;
	m_idDialog = idDialog;
	m_hwndParent = NULL;
	m_hwnd = NULL;
	m_first = NULL;
	m_isModal = false;
	m_initialized = false;
	m_autoClose = CLOSE_ON_OK | CLOSE_ON_CANCEL;
	m_forceResizable = false;
}

CDlgBase::~CDlgBase()
{
	if (m_hwnd)
		DestroyWindow(m_hwnd);
}

void CDlgBase::Create()
{
	ShowWindow(CreateDialogParam(m_hInst, MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)this), SW_HIDE);
}

void CDlgBase::Show(int nCmdShow)
{
	ShowWindow(CreateDialogParam(m_hInst, MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)this), nCmdShow);
}

int CDlgBase::DoModal()
{
	m_isModal = true;
	return DialogBoxParam(m_hInst, MAKEINTRESOURCE(m_idDialog), m_hwndParent, GlobalDlgProc, (LPARAM)this);
}

void CDlgBase::NotifyChange(void)
{
	if (m_hwndParent)
		SendMessage(m_hwndParent, PSM_CHANGED, (WPARAM)m_hwnd, 0);
}

void CDlgBase::SetCaption(const TCHAR *ptszCaption)
{
	if (m_hwnd && ptszCaption)
		SetWindowText(m_hwnd, ptszCaption);
}

int CDlgBase::Resizer(UTILRESIZECONTROL*)
{
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

INT_PTR CDlgBase::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		m_initialized = false;
		TranslateDialogDefault(m_hwnd);
		{
			for (CCtrlBase* p = m_first; p != NULL; p = p->m_next)
				AddControl(p);
		}
		NotifyControls(&CCtrlBase::OnInit);
		OnInitDialog();

		m_initialized = true;
		return TRUE;

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

			if (idCode == BN_CLICKED &&
				((idCtrl == IDOK) && (m_autoClose & CLOSE_ON_OK) ||
				(idCtrl == IDCANCEL) && (m_autoClose & CLOSE_ON_CANCEL))) {
				PostMessage(m_hwnd, WM_CLOSE, 0, 0);
			}
		}
		return FALSE;

	case WM_NOTIFY:
		{
			int idCtrl = wParam;
			NMHDR *pnmh = (NMHDR *)lParam;
			if (pnmh->idFrom == 0) {
				if (pnmh->code == PSN_APPLY) {
					NotifyControls(&CCtrlBase::OnApply);
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

	case WM_SIZE:
		if (m_forceResizable || (GetWindowLongPtr(m_hwnd, GWL_STYLE) & WS_SIZEBOX)) {
			UTILRESIZEDIALOG urd;
			urd.cbSize = sizeof(urd);
			urd.hwndDlg = m_hwnd;
			urd.hInstance = m_hInst;
			urd.lpTemplate = MAKEINTRESOURCEA(m_idDialog);
			urd.lParam = 0;
			urd.pfnResizer = GlobalDlgResizer;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
		}
		return TRUE;

	case WM_CLOSE:
		m_lresult = FALSE;
		OnClose();
		if (!m_lresult) {
			if (m_isModal)
				EndDialog(m_hwnd, 0);
			else
				DestroyWindow(m_hwnd);
		}
		return TRUE;

	case WM_DESTROY:
		OnDestroy();
		NotifyControls(&CCtrlBase::OnDestroy);
		{
			mir_cslock lck(csDialogs);
			int idx = arDialogs.getIndex(this);
			if (idx != -1)
				arDialogs.remove(idx);
		}
		m_hwnd = NULL;
		if (m_isModal)
			m_isModal = false;
		else // modeless dialogs MUST be allocated with 'new'
			delete this;

		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK CDlgBase::GlobalDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CDlgBase *wnd = NULL;
	if (msg == WM_INITDIALOG) {
		wnd = (CDlgBase*)lParam;
		wnd->m_hwnd = hwnd;

		mir_cslock lck(csDialogs);
		arDialogs.insert(wnd);
	}
	else wnd = CDlgBase::Find(hwnd);

	return (wnd == NULL) ? FALSE : wnd->DlgProc(msg, wParam, lParam);
}

int CDlgBase::GlobalDlgResizer(HWND hwnd, LPARAM, UTILRESIZECONTROL *urc)
{
	CDlgBase *wnd = wnd = CDlgBase::Find(hwnd);
	return (wnd == NULL) ? 0 : wnd->Resizer(urc);
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
	CCtrlBase search(NULL, idCtrl);
	return m_controls.find(&search);
}

CDlgBase* CDlgBase::Find(HWND hwnd)
{
	PVOID bullshit[2]; // vfptr + hwnd
	bullshit[1] = hwnd;
	return arDialogs.find((CDlgBase*)&bullshit);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCombo class

CCtrlCombo::CCtrlCombo(CDlgBase* dlg, int ctrlId)
	: CCtrlData(dlg, ctrlId)
{}

BOOL CCtrlCombo::OnCommand(HWND, WORD, WORD idCode)
{
	switch (idCode) {
	case CBN_CLOSEUP:  OnCloseup(this);  break;
	case CBN_DROPDOWN: OnDropdown(this); break;

	case CBN_EDITCHANGE:
	case CBN_EDITUPDATE:
	case CBN_SELCHANGE:
	case CBN_SELENDOK:
		NotifyChange();
		break;
	}
	return TRUE;
}

void CCtrlCombo::OnInit()
{
	CSuper::OnInit();
	OnReset();
}

void CCtrlCombo::OnApply()
{
	CSuper::OnApply();

	if (GetDataType() == DBVT_TCHAR) {
		int len = GetWindowTextLength(m_hwnd) + 1;
		TCHAR *buf = (TCHAR *)_alloca(sizeof(TCHAR) * len);
		GetWindowText(m_hwnd, buf, len);
		SaveText(buf);
	}
	else if (GetDataType() != DBVT_DELETED) {
		SaveInt(GetInt());
	}
}

void CCtrlCombo::OnReset()
{
	if (GetDataType() == DBVT_TCHAR)
		SetText(LoadText());
	else if (GetDataType() != DBVT_DELETED)
		SetInt(LoadInt());
}

int CCtrlCombo::AddString(const TCHAR *text, LPARAM data)
{
	int iItem = SendMessage(m_hwnd, CB_ADDSTRING, 0, (LPARAM)text);
	if (data)
		SendMessage(m_hwnd, CB_SETITEMDATA, iItem, data);
	return iItem;
}

int CCtrlCombo::AddStringA(const char *text, LPARAM data)
{
	int iItem = SendMessageA(m_hwnd, CB_ADDSTRING, 0, (LPARAM)text);
	if (data)
		SendMessage(m_hwnd, CB_SETITEMDATA, iItem, data);
	return iItem;
}

void CCtrlCombo::DeleteString(int index)
{	SendMessage(m_hwnd, CB_DELETESTRING, index, 0);
}

int CCtrlCombo::FindString(const TCHAR *str, int index, bool exact)
{	return SendMessage(m_hwnd, exact?CB_FINDSTRINGEXACT:CB_FINDSTRING, index, (LPARAM)str);
}

int CCtrlCombo::FindStringA(const char *str, int index, bool exact)
{	return SendMessageA(m_hwnd, exact?CB_FINDSTRINGEXACT:CB_FINDSTRING, index, (LPARAM)str);
}

int CCtrlCombo::GetCount()
{	return SendMessage(m_hwnd, CB_GETCOUNT, 0, 0);
}

int CCtrlCombo::GetCurSel()
{	return SendMessage(m_hwnd, CB_GETCURSEL, 0, 0);
}

bool CCtrlCombo::GetDroppedState()
{	return SendMessage(m_hwnd, CB_GETDROPPEDSTATE, 0, 0) ? true : false;
}

LPARAM CCtrlCombo::GetItemData(int index)
{	return SendMessage(m_hwnd, CB_GETITEMDATA, index, 0);
}

TCHAR* CCtrlCombo::GetItemText(int index)
{
	TCHAR *result = (TCHAR *)mir_alloc(sizeof(TCHAR) * (SendMessage(m_hwnd, CB_GETLBTEXTLEN, index, 0) + 1));
	SendMessage(m_hwnd, CB_GETLBTEXT, index, (LPARAM)result);
	return result;
}

TCHAR* CCtrlCombo::GetItemText(int index, TCHAR *buf, int size)
{
	TCHAR *result = (TCHAR *)_alloca(sizeof(TCHAR) * (SendMessage(m_hwnd, CB_GETLBTEXTLEN, index, 0) + 1));
	SendMessage(m_hwnd, CB_GETLBTEXT, index, (LPARAM)result);
	mir_tstrncpy(buf, result, size);
	return buf;
}

int CCtrlCombo::InsertString(TCHAR *text, int pos, LPARAM data)
{
	int iItem = SendMessage(m_hwnd, CB_INSERTSTRING, pos, (LPARAM)text);
	SendMessage(m_hwnd, CB_SETITEMDATA, iItem, data);
	return iItem;
}

void CCtrlCombo::ResetContent()
{	SendMessage(m_hwnd, CB_RESETCONTENT, 0, 0);
}

int CCtrlCombo::SelectString(TCHAR *str)
{	return SendMessage(m_hwnd, CB_SELECTSTRING, 0, (LPARAM)str);
}

int CCtrlCombo::SetCurSel(int index)
{	return SendMessage(m_hwnd, CB_SETCURSEL, index, 0);
}

void CCtrlCombo::SetItemData(int index, LPARAM data)
{	SendMessage(m_hwnd, CB_SETITEMDATA, index, data);
}

void CCtrlCombo::ShowDropdown(bool show)
{	SendMessage(m_hwnd, CB_SHOWDROPDOWN, show ? TRUE : FALSE, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlListBox class

CCtrlListBox::CCtrlListBox(CDlgBase* dlg, int ctrlId) :
CCtrlBase(dlg, ctrlId)
{}

BOOL CCtrlListBox::OnCommand(HWND, WORD, WORD idCode)
{
	switch (idCode) {
		case LBN_DBLCLK:    OnDblClick(this); break;
		case LBN_SELCANCEL: OnSelCancel(this); break;
		case LBN_SELCHANGE: OnSelChange(this); break;
	}
	return TRUE;
}

int CCtrlListBox::AddString(TCHAR *text, LPARAM data)
{
	int iItem = SendMessage(m_hwnd, LB_ADDSTRING, 0, (LPARAM)text);
	SendMessage(m_hwnd, LB_SETITEMDATA, iItem, data);
	return iItem;
}

void CCtrlListBox::DeleteString(int index)
{	SendMessage(m_hwnd, LB_DELETESTRING, index, 0);
}

int CCtrlListBox::FindString(TCHAR *str, int index, bool exact)
{	return SendMessage(m_hwnd, exact?LB_FINDSTRINGEXACT:LB_FINDSTRING, index, (LPARAM)str);
}

int CCtrlListBox::GetCount()
{	return SendMessage(m_hwnd, LB_GETCOUNT, 0, 0);
}

int CCtrlListBox::GetCurSel()
{	return SendMessage(m_hwnd, LB_GETCURSEL, 0, 0);
}

LPARAM CCtrlListBox::GetItemData(int index)
{	return SendMessage(m_hwnd, LB_GETITEMDATA, index, 0);
}

TCHAR* CCtrlListBox::GetItemText(int index)
{
	TCHAR *result = (TCHAR *)mir_alloc(sizeof(TCHAR) * (SendMessage(m_hwnd, LB_GETTEXTLEN, index, 0) + 1));
	SendMessage(m_hwnd, LB_GETTEXT, index, (LPARAM)result);
	return result;
}

TCHAR* CCtrlListBox::GetItemText(int index, TCHAR *buf, int size)
{
	TCHAR *result = (TCHAR *)_alloca(sizeof(TCHAR) * (SendMessage(m_hwnd, LB_GETTEXTLEN, index, 0) + 1));
	SendMessage(m_hwnd, LB_GETTEXT, index, (LPARAM)result);
	mir_tstrncpy(buf, result, size);
	return buf;
}

bool CCtrlListBox::GetSel(int index)
{	return SendMessage(m_hwnd, LB_GETSEL, index, 0) ? true : false;
}

int CCtrlListBox::GetSelCount()
{	return SendMessage(m_hwnd, LB_GETSELCOUNT, 0, 0);
}

int* CCtrlListBox::GetSelItems(int *items, int count)
{
	SendMessage(m_hwnd, LB_GETSELITEMS, count, (LPARAM)items);
	return items;
}

int* CCtrlListBox::GetSelItems()
{
	int count = GetSelCount() + 1;
	int *result = (int *)mir_alloc(sizeof(int) * count);
	SendMessage(m_hwnd, LB_GETSELITEMS, count, (LPARAM)result);
	result[count-1] = -1;
	return result;
}

int CCtrlListBox::InsertString(TCHAR *text, int pos, LPARAM data)
{
	int iItem = SendMessage(m_hwnd, CB_INSERTSTRING, pos, (LPARAM)text);
	SendMessage(m_hwnd, CB_SETITEMDATA, iItem, data);
	return iItem;
}

void CCtrlListBox::ResetContent()
{	SendMessage(m_hwnd, LB_RESETCONTENT, 0, 0);
}

int CCtrlListBox::SelectString(TCHAR *str)
{	return SendMessage(m_hwnd, LB_SELECTSTRING, 0, (LPARAM)str);
}

int CCtrlListBox::SetCurSel(int index)
{	return SendMessage(m_hwnd, LB_SETCURSEL, index, 0);
}

void CCtrlListBox::SetItemData(int index, LPARAM data)
{	SendMessage(m_hwnd, LB_SETITEMDATA, index, data);
}

void CCtrlListBox::SetSel(int index, bool sel)
{	SendMessage(m_hwnd, LB_SETSEL, sel ? TRUE : FALSE, index);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlCheck class

CCtrlCheck::CCtrlCheck(CDlgBase* dlg, int ctrlId)
	: CCtrlData(dlg, ctrlId)
{}

BOOL CCtrlCheck::OnCommand(HWND, WORD, WORD)
{
	NotifyChange();
	return TRUE;
}

void CCtrlCheck::OnApply()
{
	CSuper::OnApply();

	SaveInt(GetState());
}

void CCtrlCheck::OnReset()
{
	SetState(LoadInt());
}

int CCtrlCheck::GetState()
{	return SendMessage(m_hwnd, BM_GETCHECK, 0, 0);
}

void CCtrlCheck::SetState(int state)
{	SendMessage(m_hwnd, BM_SETCHECK, state, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlEdit class

CCtrlEdit::CCtrlEdit(CDlgBase* dlg, int ctrlId)
	: CCtrlData(dlg, ctrlId)
{}

BOOL CCtrlEdit::OnCommand(HWND, WORD, WORD idCode)
{
	if (idCode == EN_CHANGE)
		NotifyChange();
	return TRUE;
}

void CCtrlEdit::OnApply()
{
	CSuper::OnApply();

	if (GetDataType() == DBVT_TCHAR) {
		int len = GetWindowTextLength(m_hwnd) + 1;
		TCHAR *buf = (TCHAR *)_alloca(sizeof(TCHAR) * len);
		GetWindowText(m_hwnd, buf, len);
		SaveText(buf);
	}
	else if (GetDataType() != DBVT_DELETED) {
		SaveInt(GetInt());
	}
}

void CCtrlEdit::OnReset()
{
	if (GetDataType() == DBVT_TCHAR)
		SetText(LoadText());
	else if (GetDataType() != DBVT_DELETED)
		SetInt(LoadInt());
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlData class

CCtrlData::CCtrlData(CDlgBase *wnd, int idCtrl)
	: CCtrlBase(wnd, idCtrl),
	m_dbLink(NULL)
{}

CCtrlData::~CCtrlData()
{
	if (m_dbLink)
		delete m_dbLink;
}

void CCtrlData::OnInit()
{
	CCtrlBase::OnInit();
	OnReset();
}

void CCtrlData::CreateDbLink(const char* szModuleName, const char* szSetting, BYTE type, DWORD iValue)
{
	m_dbLink = new CDbLink(szModuleName, szSetting, type, iValue);
}

void CCtrlData::CreateDbLink(const char* szModuleName, const char* szSetting, TCHAR* szValue)
{
	m_dbLink = new CDbLink(szModuleName, szSetting, DBVT_TCHAR, szValue);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlMButton

CCtrlMButton::CCtrlMButton(CDlgBase* dlg, int ctrlId, HICON hIcon, const char* tooltip) 
	: CCtrlButton(dlg, ctrlId),
	m_hIcon(hIcon),
	m_toolTip(tooltip)
{}

CCtrlMButton::CCtrlMButton(CDlgBase* dlg, int ctrlId, int iCoreIcon, const char* tooltip)
	: CCtrlButton(dlg, ctrlId),
	m_hIcon(::LoadSkinnedIcon(iCoreIcon)),
	m_toolTip(tooltip)
{}

CCtrlMButton::~CCtrlMButton()
{
	::Skin_ReleaseIcon(m_hIcon);
}

void CCtrlMButton::OnInit()
{
	CCtrlButton::OnInit();

	SendMessage(m_hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_hIcon);
	SendMessage(m_hwnd, BUTTONADDTOOLTIP, (WPARAM)m_toolTip, 0);
	SendMessage(m_hwnd, BUTTONSETASFLATBTN, (WPARAM)m_toolTip, 0);
}

void CCtrlMButton::MakeFlat()
{
	SendMessage(m_hwnd, BUTTONSETASFLATBTN, TRUE, 0);
}

void CCtrlMButton::MakePush()
{
	SendMessage(m_hwnd, BUTTONSETASPUSHBTN, TRUE, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlButton

CCtrlButton::CCtrlButton(CDlgBase* wnd, int idCtrl)
	: CCtrlBase(wnd, idCtrl)
{}

BOOL CCtrlButton::OnCommand(HWND, WORD, WORD idCode)
{
	if (idCode == BN_CLICKED || idCode == STN_CLICKED)
		OnClick(this);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlHyperlink

CCtrlHyperlink::CCtrlHyperlink(CDlgBase* wnd, int idCtrl, const char* url)
	: CCtrlBase(wnd, idCtrl),
	m_url(url)
{}

BOOL CCtrlHyperlink::OnCommand(HWND, WORD, WORD)
{
	ShellExecuteA(m_hwnd, "open", m_url, "", "", SW_SHOW);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlClc
CCtrlClc::CCtrlClc(CDlgBase* dlg, int ctrlId)
	: CCtrlBase(dlg, ctrlId)
{}

BOOL CCtrlClc::OnNotify(int, NMHDR *pnmh)
{
	TEventInfo evt = { this, (NMCLISTCONTROL *)pnmh };
	switch (pnmh->code) {
		case CLN_EXPANDED:       OnExpanded(&evt); break;
		case CLN_LISTREBUILT:    OnListRebuilt(&evt); break;
		case CLN_ITEMCHECKED:    OnItemChecked(&evt); break;
		case CLN_DRAGGING:       OnDragging(&evt); break;
		case CLN_DROPPED:        OnDropped(&evt); break;
		case CLN_LISTSIZECHANGE: OnListSizeChange(&evt); break;
		case CLN_OPTIONSCHANGED: OnOptionsChanged(&evt); break;
		case CLN_DRAGSTOP:       OnDragStop(&evt); break;
		case CLN_NEWCONTACT:     OnNewContact(&evt); break;
		case CLN_CONTACTMOVED:   OnContactMoved(&evt); break;
		case CLN_CHECKCHANGED:   OnCheckChanged(&evt); break;
		case NM_CLICK:           OnClick(&evt); break;
	}
	return FALSE;
}

void CCtrlClc::AddContact(MCONTACT hContact)
{	SendMessage(m_hwnd, CLM_ADDCONTACT, hContact, 0);
}

void CCtrlClc::AddGroup(HANDLE hGroup)
{	SendMessage(m_hwnd, CLM_ADDGROUP, (WPARAM)hGroup, 0);
}

void CCtrlClc::AutoRebuild()
{	SendMessage(m_hwnd, CLM_AUTOREBUILD, 0, 0);
}

void CCtrlClc::DeleteItem(HANDLE hItem)
{	SendMessage(m_hwnd, CLM_DELETEITEM, (WPARAM)hItem, 0);
}

void CCtrlClc::EditLabel(HANDLE hItem)
{	SendMessage(m_hwnd, CLM_EDITLABEL, (WPARAM)hItem, 0);
}

void CCtrlClc::EndEditLabel(bool save)
{	SendMessage(m_hwnd, CLM_ENDEDITLABELNOW, save ? 0 : 1, 0);
}

void CCtrlClc::EnsureVisible(HANDLE hItem, bool partialOk)
{	SendMessage(m_hwnd, CLM_ENSUREVISIBLE, (WPARAM)hItem, partialOk ? TRUE : FALSE);
}

void CCtrlClc::Expand(HANDLE hItem, DWORD flags)
{	SendMessage(m_hwnd, CLM_EXPAND, (WPARAM)hItem, flags);
}

HANDLE CCtrlClc::FindContact(MCONTACT hContact)
{	return (HANDLE)SendMessage(m_hwnd, CLM_FINDCONTACT, hContact, 0);
}

HANDLE CCtrlClc::FindGroup(HANDLE hGroup)
{	return (HANDLE)SendMessage(m_hwnd, CLM_FINDGROUP, (WPARAM)hGroup, 0);
}

COLORREF CCtrlClc::GetBkColor()
{	return (COLORREF)SendMessage(m_hwnd, CLM_GETBKCOLOR, 0, 0);
}

bool CCtrlClc::GetCheck(HANDLE hItem)
{	return SendMessage(m_hwnd, CLM_GETCHECKMARK, (WPARAM)hItem, 0) ? true : false;
}

int CCtrlClc::GetCount()
{	return SendMessage(m_hwnd, CLM_GETCOUNT, 0, 0);
}

HWND CCtrlClc::GetEditControl()
{	return (HWND)SendMessage(m_hwnd, CLM_GETEDITCONTROL, 0, 0);
}

DWORD CCtrlClc::GetExpand(HANDLE hItem)
{	return SendMessage(m_hwnd, CLM_GETEXPAND, (WPARAM)hItem, 0);
}

int CCtrlClc::GetExtraColumns()
{	return SendMessage(m_hwnd, CLM_GETEXTRACOLUMNS, 0, 0);
}

BYTE CCtrlClc::GetExtraImage(HANDLE hItem, int iColumn)
{
	return (BYTE)(SendMessage(m_hwnd, CLM_GETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, 0)) & 0xFFFF);
}

HIMAGELIST CCtrlClc::GetExtraImageList()
{	return (HIMAGELIST)SendMessage(m_hwnd, CLM_GETEXTRAIMAGELIST, 0, 0);
}

HFONT CCtrlClc::GetFont(int iFontId)
{	return (HFONT)SendMessage(m_hwnd, CLM_GETFONT, (WPARAM)iFontId, 0);
}

HANDLE CCtrlClc::GetSelection()
{	return (HANDLE)SendMessage(m_hwnd, CLM_GETSELECTION, 0, 0);
}

HANDLE CCtrlClc::HitTest(int x, int y, DWORD *hitTest)
{	return (HANDLE)SendMessage(m_hwnd, CLM_HITTEST, (WPARAM)hitTest, MAKELPARAM(x,y));
}

void CCtrlClc::SelectItem(HANDLE hItem)
{	SendMessage(m_hwnd, CLM_SELECTITEM, (WPARAM)hItem, 0);
}

void CCtrlClc::SetBkBitmap(DWORD mode, HBITMAP hBitmap)
{	SendMessage(m_hwnd, CLM_SETBKBITMAP, mode, (LPARAM)hBitmap);
}

void CCtrlClc::SetBkColor(COLORREF clBack)
{	SendMessage(m_hwnd, CLM_SETBKCOLOR, (WPARAM)clBack, 0);
}

void CCtrlClc::SetCheck(HANDLE hItem, bool check)
{	SendMessage(m_hwnd, CLM_SETCHECKMARK, (WPARAM)hItem, check ? 1 : 0);
}

void CCtrlClc::SetExtraColumns(int iColumns)
{	SendMessage(m_hwnd, CLM_SETEXTRACOLUMNS, (WPARAM)iColumns, 0);
}

void CCtrlClc::SetExtraImage(HANDLE hItem, int iColumn, int iImage)
{	SendMessage(m_hwnd, CLM_SETEXTRAIMAGE, (WPARAM)hItem, MAKELPARAM(iColumn, iImage));
}

void CCtrlClc::SetExtraImageList(HIMAGELIST hImgList)
{	SendMessage(m_hwnd, CLM_SETEXTRAIMAGELIST, 0, (LPARAM)hImgList);
}

void CCtrlClc::SetFont(int iFontId, HANDLE hFont, bool bRedraw)
{	SendMessage(m_hwnd, CLM_SETFONT, (WPARAM)hFont, MAKELPARAM(bRedraw ? 1 : 0, iFontId));
}

void CCtrlClc::SetIndent(int iIndent)
{	SendMessage(m_hwnd, CLM_SETINDENT, (WPARAM)iIndent, 0);
}

void CCtrlClc::SetItemText(HANDLE hItem, char *szText)
{	SendMessage(m_hwnd, CLM_SETITEMTEXT, (WPARAM)hItem, (LPARAM)szText);
}

void CCtrlClc::SetHideEmptyGroups(bool state)
{	SendMessage(m_hwnd, CLM_SETHIDEEMPTYGROUPS, state ? 1 : 0, 0);
}

void CCtrlClc::SetGreyoutFlags(DWORD flags)
{	SendMessage(m_hwnd, CLM_SETGREYOUTFLAGS, (WPARAM)flags, 0);
}

bool CCtrlClc::GetHideOfflineRoot()
{	return SendMessage(m_hwnd, CLM_GETHIDEOFFLINEROOT, 0, 0) ? true : false;
}

void CCtrlClc::SetHideOfflineRoot(bool state)
{	SendMessage(m_hwnd, CLM_SETHIDEOFFLINEROOT, state ? 1 : 0, 9);
}

void CCtrlClc::SetUseGroups(bool state)
{	SendMessage(m_hwnd, CLM_SETUSEGROUPS, state ? 1 : 0, 0);
}

void CCtrlClc::SetOfflineModes(DWORD modes)
{	SendMessage(m_hwnd, CLM_SETOFFLINEMODES, modes, 0);
}

DWORD CCtrlClc::GetExStyle()
{	return SendMessage(m_hwnd, CLM_GETEXSTYLE, 0, 0);
}

void CCtrlClc::SetExStyle(DWORD exStyle)
{	SendMessage(m_hwnd, CLM_SETEXSTYLE, (WPARAM)exStyle, 0);
}

int CCtrlClc::GetLefrMargin()
{	return SendMessage(m_hwnd, CLM_GETLEFTMARGIN, 0, 0);
}

void CCtrlClc::SetLeftMargin(int iMargin)
{	SendMessage(m_hwnd, CLM_SETLEFTMARGIN, (WPARAM)iMargin, 0);
}

HANDLE CCtrlClc::AddInfoItem(CLCINFOITEM *cii)
{	return (HANDLE)SendMessage(m_hwnd, CLM_ADDINFOITEM, 0, (LPARAM)cii);
}

int CCtrlClc::GetItemType(HANDLE hItem)
{	return SendMessage(m_hwnd, CLM_GETITEMTYPE, (WPARAM)hItem, 0);
}

HANDLE CCtrlClc::GetNextItem(HANDLE hItem, DWORD flags)
{	return (HANDLE)SendMessage(m_hwnd, CLM_GETNEXTITEM, (WPARAM)flags, (LPARAM)hItem);
}

COLORREF CCtrlClc::GetTextColor(int iFontId)
{	return (COLORREF)SendMessage(m_hwnd, CLM_GETTEXTCOLOR, (WPARAM)iFontId, 0);
}

void CCtrlClc::SetTextColor(int iFontId, COLORREF clText)
{	SendMessage(m_hwnd, CLM_SETTEXTCOLOR, (WPARAM)iFontId, (LPARAM)clText);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlListView

CCtrlListView::CCtrlListView(CDlgBase* dlg, int ctrlId)
	: CCtrlBase(dlg, ctrlId)
{}

BOOL CCtrlListView::OnNotify(int, NMHDR *pnmh)
{
	TEventInfo evt = { this, pnmh };

	switch (pnmh->code) {
		case NM_DBLCLK:             OnDoubleClick(&evt);       return TRUE;
		case LVN_BEGINDRAG:         OnBeginDrag(&evt);         return TRUE;
		case LVN_BEGINLABELEDIT:    OnBeginLabelEdit(&evt);    return TRUE;
		case LVN_BEGINRDRAG:        OnBeginRDrag(&evt);        return TRUE;
		case LVN_BEGINSCROLL:       OnBeginScroll(&evt);       return TRUE;
		case LVN_COLUMNCLICK:       OnColumnClick(&evt);       return TRUE;
		case LVN_DELETEALLITEMS:    OnDeleteAllItems(&evt);    return TRUE;
		case LVN_DELETEITEM:        OnDeleteItem(&evt);        return TRUE;
		case LVN_ENDLABELEDIT:      OnEndLabelEdit(&evt);      return TRUE;
		case LVN_ENDSCROLL:         OnEndScroll(&evt);         return TRUE;
		case LVN_GETDISPINFO:       OnGetDispInfo(&evt);       return TRUE;
		case LVN_GETINFOTIP:        OnGetInfoTip(&evt);        return TRUE;
		case LVN_HOTTRACK:          OnHotTrack(&evt);          return TRUE;
		case LVN_INSERTITEM:        OnInsertItem(&evt);        return TRUE;
		case LVN_ITEMACTIVATE:      OnItemActivate(&evt);      return TRUE;
		case LVN_ITEMCHANGED:       OnItemChanged(&evt);       return TRUE;
		case LVN_ITEMCHANGING:      OnItemChanging(&evt);      return TRUE;
		case LVN_KEYDOWN:           OnKeyDown(&evt);           return TRUE;
		case LVN_MARQUEEBEGIN:      OnMarqueeBegin(&evt);      return TRUE;
		case LVN_SETDISPINFO:       OnSetDispInfo(&evt);       return TRUE;
	}

	return FALSE;
}

// additional api
HIMAGELIST CCtrlListView::CreateImageList(int iImageList)
{
	HIMAGELIST hIml;
	if (hIml = GetImageList(iImageList))
		return hIml;

	hIml = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 1);
	SetImageList(hIml, iImageList);
	return hIml;
}

void CCtrlListView::AddColumn(int iSubItem, TCHAR *name, int cx)
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	lvc.iImage = 0;
	lvc.pszText = name;
	lvc.cx = cx;
	lvc.iSubItem = iSubItem;
	InsertColumn(iSubItem, &lvc);
}

void CCtrlListView::AddGroup(int iGroupId, TCHAR *name)
{
	LVGROUP lvg = { 0 };
	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID;
	lvg.pszHeader = name;
	lvg.cchHeader = (int)mir_tstrlen(lvg.pszHeader);
	lvg.iGroupId = iGroupId;
	InsertGroup(-1, &lvg);
}

int CCtrlListView::AddItem(TCHAR *text, int iIcon, LPARAM lParam, int iGroupId)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
	lvi.iSubItem = 0;
	lvi.pszText = text;
	lvi.iImage = iIcon;
	lvi.lParam = lParam;
	if (iGroupId >= 0) {
		lvi.mask |= LVIF_GROUPID;
		lvi.iGroupId = iGroupId;
	}

	return InsertItem(&lvi);
}

void CCtrlListView::SetItem(int iItem, int iSubItem, TCHAR *text, int iIcon)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_TEXT;
	lvi.iItem = iItem;
	lvi.iSubItem = iSubItem;
	lvi.pszText = text;
	if (iIcon >= 0) {
		lvi.mask |= LVIF_IMAGE;
		lvi.iImage = iIcon;
	}

	SetItem(&lvi);
}

LPARAM CCtrlListView::GetItemData(int iItem)
{
	LVITEM lvi = { 0 };
	lvi.mask = LVIF_PARAM;
	lvi.iItem = iItem;
	GetItem(&lvi);
	return lvi.lParam;
}

// classic api
DWORD CCtrlListView::ApproximateViewRect(int cx, int cy, int iCount)
{	return ListView_ApproximateViewRect(m_hwnd, cx, cy, iCount);
}
void CCtrlListView::Arrange(UINT code)
{	ListView_Arrange(m_hwnd, code);
}
void CCtrlListView::CancelEditLabel()
{	ListView_CancelEditLabel(m_hwnd);
}
HIMAGELIST CCtrlListView::CreateDragImage(int iItem, LPPOINT lpptUpLeft)
{	return ListView_CreateDragImage(m_hwnd, iItem, lpptUpLeft);
}
void CCtrlListView::DeleteAllItems()
{	ListView_DeleteAllItems(m_hwnd);
}
void CCtrlListView::DeleteColumn(int iCol)
{	ListView_DeleteColumn(m_hwnd, iCol);
}
void CCtrlListView::DeleteItem(int iItem)
{	ListView_DeleteItem(m_hwnd, iItem);
}
HWND CCtrlListView::EditLabel(int iItem)
{	return ListView_EditLabel(m_hwnd, iItem);
}
int CCtrlListView::EnableGroupView(BOOL fEnable)
{	return ListView_EnableGroupView(m_hwnd, fEnable);
}
BOOL CCtrlListView::EnsureVisible(int i, BOOL fPartialOK)
{	return ListView_EnsureVisible(m_hwnd, i, fPartialOK);
}
int CCtrlListView::FindItem(int iStart, const LVFINDINFO *plvfi)
{	return ListView_FindItem(m_hwnd, iStart, plvfi);
}
COLORREF CCtrlListView::GetBkColor()
{	return ListView_GetBkColor(m_hwnd);
}
void CCtrlListView::GetBkImage(LPLVBKIMAGE plvbki)
{	ListView_GetBkImage(m_hwnd, plvbki);
}
UINT CCtrlListView::GetCallbackMask()
{	return ListView_GetCallbackMask(m_hwnd);
}
BOOL CCtrlListView::GetCheckState(UINT iIndex)
{	return ListView_GetCheckState(m_hwnd, iIndex);
}
void CCtrlListView::GetColumn(int iCol, LPLVCOLUMN pcol)
{	ListView_GetColumn(m_hwnd, iCol, pcol);
}
void CCtrlListView::GetColumnOrderArray(int iCount, int *lpiArray)
{	ListView_GetColumnOrderArray(m_hwnd, iCount, lpiArray);
}
int CCtrlListView::GetColumnWidth(int iCol)
{	return ListView_GetColumnWidth(m_hwnd, iCol);
}
int CCtrlListView::GetCountPerPage()
{	return ListView_GetCountPerPage(m_hwnd);
}
HWND CCtrlListView::GetEditControl()
{	return ListView_GetEditControl(m_hwnd);
}
DWORD CCtrlListView::GetExtendedListViewStyle()
{	return ListView_GetExtendedListViewStyle(m_hwnd);
}
void CCtrlListView::GetGroupMetrics(LVGROUPMETRICS *pGroupMetrics)
{	ListView_GetGroupMetrics(m_hwnd, pGroupMetrics);
}
HWND CCtrlListView::GetHeader()
{	return ListView_GetHeader(m_hwnd);
}
HCURSOR CCtrlListView::GetHotCursor()
{	return ListView_GetHotCursor(m_hwnd);
}
INT CCtrlListView::GetHotItem()
{	return ListView_GetHotItem(m_hwnd);
}
DWORD CCtrlListView::GetHoverTime()
{	return ListView_GetHoverTime(m_hwnd);
}
HIMAGELIST CCtrlListView::GetImageList(int iImageList)
{	return ListView_GetImageList(m_hwnd, iImageList);
}
BOOL CCtrlListView::GetInsertMark(LVINSERTMARK *plvim)
{	return ListView_GetInsertMark(m_hwnd, plvim);
}
COLORREF CCtrlListView::GetInsertMarkColor()
{	return ListView_GetInsertMarkColor(m_hwnd);
}
int CCtrlListView::GetInsertMarkRect(LPRECT prc)
{	return ListView_GetInsertMarkRect(m_hwnd, prc);
}
BOOL CCtrlListView::GetISearchString(LPSTR lpsz)
{	return ListView_GetISearchString(m_hwnd, lpsz);
}
bool CCtrlListView::GetItem(LPLVITEM pitem)
{	return ListView_GetItem(m_hwnd, pitem) == TRUE;
}
int CCtrlListView::GetItemCount()
{	return ListView_GetItemCount(m_hwnd);
}
void CCtrlListView::GetItemPosition(int i, POINT *ppt)
{	ListView_GetItemPosition(m_hwnd, i, ppt);
}
void CCtrlListView::GetItemRect(int i, RECT *prc, int code)
{	ListView_GetItemRect(m_hwnd, i, prc, code);
}
DWORD CCtrlListView::GetItemSpacing(BOOL fSmall)
{	return ListView_GetItemSpacing(m_hwnd, fSmall);
}
UINT CCtrlListView::GetItemState(int i, UINT mask)
{	return ListView_GetItemState(m_hwnd, i, mask);
}
void CCtrlListView::GetItemText(int iItem, int iSubItem, LPTSTR pszText, int cchTextMax)
{	ListView_GetItemText(m_hwnd, iItem, iSubItem, pszText, cchTextMax);
}
int CCtrlListView::GetNextItem(int iStart, UINT flags)
{	return ListView_GetNextItem(m_hwnd, iStart, flags);
}
BOOL CCtrlListView::GetNumberOfWorkAreas(LPUINT lpuWorkAreas)
{	return  ListView_GetNumberOfWorkAreas(m_hwnd, lpuWorkAreas);
}
BOOL CCtrlListView::GetOrigin(LPPOINT lpptOrg)
{	return ListView_GetOrigin(m_hwnd, lpptOrg);
}
COLORREF CCtrlListView::GetOutlineColor()
{	return ListView_GetOutlineColor(m_hwnd);
}
UINT CCtrlListView::GetSelectedColumn()
{	return ListView_GetSelectedColumn(m_hwnd);
}
UINT CCtrlListView::GetSelectedCount()
{	return ListView_GetSelectedCount(m_hwnd);
}
INT CCtrlListView::GetSelectionMark()
{	return ListView_GetSelectionMark(m_hwnd);
}
int CCtrlListView::GetStringWidth(LPCSTR psz)
{	return ListView_GetStringWidth(m_hwnd, psz);
}
BOOL CCtrlListView::GetSubItemRect(int iItem, int iSubItem, int code, LPRECT lpRect)
{	return ListView_GetSubItemRect(m_hwnd, iItem, iSubItem, code, lpRect);
}
COLORREF CCtrlListView::GetTextBkColor()
{	return ListView_GetTextBkColor(m_hwnd);
}
COLORREF CCtrlListView::GetTextColor()
{	return ListView_GetTextColor(m_hwnd);
}
void CCtrlListView::GetTileInfo(PLVTILEINFO plvtinfo)
{	ListView_GetTileInfo(m_hwnd, plvtinfo);
}
void CCtrlListView::GetTileViewInfo(PLVTILEVIEWINFO plvtvinfo)
{	ListView_GetTileViewInfo(m_hwnd, plvtvinfo);
}
HWND CCtrlListView::GetToolTips()
{	return ListView_GetToolTips(m_hwnd);
}
int CCtrlListView::GetTopIndex()
{	return ListView_GetTopIndex(m_hwnd);
}
BOOL CCtrlListView::GetUnicodeFormat()
{	return ListView_GetUnicodeFormat(m_hwnd);
}
DWORD CCtrlListView::GetView()
{	return ListView_GetView(m_hwnd);
}
BOOL CCtrlListView::GetViewRect(RECT *prc)
{	return ListView_GetViewRect(m_hwnd, prc);
}
void CCtrlListView::GetWorkAreas(INT nWorkAreas, LPRECT lprc)
{	ListView_GetWorkAreas(m_hwnd, nWorkAreas, lprc);
}
BOOL CCtrlListView::HasGroup(int dwGroupId)
{	return ListView_HasGroup(m_hwnd, dwGroupId);
}
int CCtrlListView::HitTest(LPLVHITTESTINFO pinfo)
{	return ListView_HitTest(m_hwnd, pinfo);
}
int CCtrlListView::InsertColumn(int iCol, const LPLVCOLUMN pcol)
{	return ListView_InsertColumn(m_hwnd, iCol, pcol);
}
int CCtrlListView::InsertGroup(int index, PLVGROUP pgrp)
{	return ListView_InsertGroup(m_hwnd, index, pgrp);
}
void CCtrlListView::InsertGroupSorted(PLVINSERTGROUPSORTED structInsert)
{	ListView_InsertGroupSorted(m_hwnd, structInsert);
}
int CCtrlListView::InsertItem(const LPLVITEM pitem)
{	return ListView_InsertItem(m_hwnd, pitem);
}
BOOL CCtrlListView::InsertMarkHitTest(LPPOINT point, LVINSERTMARK *plvim)
{	return ListView_InsertMarkHitTest(m_hwnd, point, plvim);
}
BOOL CCtrlListView::IsGroupViewEnabled()
{	return ListView_IsGroupViewEnabled(m_hwnd);
}
UINT CCtrlListView::MapIDToIndex(UINT id)
{	return ListView_MapIDToIndex(m_hwnd, id);
}
UINT CCtrlListView::MapIndexToID(UINT index)
{	return ListView_MapIndexToID(m_hwnd, index);
}
BOOL CCtrlListView::RedrawItems(int iFirst, int iLast)
{	return ListView_RedrawItems(m_hwnd, iFirst, iLast);
}
void CCtrlListView::RemoveAllGroups()
{	ListView_RemoveAllGroups(m_hwnd);
}
int CCtrlListView::RemoveGroup(int iGroupId)
{	return ListView_RemoveGroup(m_hwnd, iGroupId);
}
BOOL CCtrlListView::Scroll(int dx, int dy)
{	return ListView_Scroll(m_hwnd, dx, dy);
}
BOOL CCtrlListView::SetBkColor(COLORREF clrBk)
{	return ListView_SetBkColor(m_hwnd, clrBk);
}
BOOL CCtrlListView::SetBkImage(LPLVBKIMAGE plvbki)
{	return ListView_SetBkImage(m_hwnd, plvbki);
}
BOOL CCtrlListView::SetCallbackMask(UINT mask)
{	return ListView_SetCallbackMask(m_hwnd, mask);
}
void CCtrlListView::SetCheckState(UINT iIndex, BOOL fCheck)
{	ListView_SetCheckState(m_hwnd, iIndex, fCheck);
}
BOOL CCtrlListView::SetColumn(int iCol, LPLVCOLUMN pcol)
{	return ListView_SetColumn(m_hwnd, iCol, pcol);
}
BOOL CCtrlListView::SetColumnOrderArray(int iCount, int *lpiArray)
{	return ListView_SetColumnOrderArray(m_hwnd, iCount, lpiArray);
}
BOOL CCtrlListView::SetColumnWidth(int iCol, int cx)
{	return ListView_SetColumnWidth(m_hwnd, iCol, cx);
}
void CCtrlListView::SetExtendedListViewStyle(DWORD dwExStyle)
{	ListView_SetExtendedListViewStyle(m_hwnd, dwExStyle);
}
void CCtrlListView::SetExtendedListViewStyleEx(DWORD dwExMask, DWORD dwExStyle)
{	ListView_SetExtendedListViewStyleEx(m_hwnd, dwExMask, dwExStyle);
}
int CCtrlListView::SetGroupInfo(int iGroupId, PLVGROUP pgrp)
{	return ListView_SetGroupInfo(m_hwnd, iGroupId, pgrp);
}
void CCtrlListView::SetGroupMetrics(PLVGROUPMETRICS pGroupMetrics)
{	ListView_SetGroupMetrics(m_hwnd, pGroupMetrics);
}
HCURSOR CCtrlListView::SetHotCursor(HCURSOR hCursor)
{	return ListView_SetHotCursor(m_hwnd, hCursor);
}
INT CCtrlListView::SetHotItem(INT iIndex)
{	return ListView_SetHotItem(m_hwnd, iIndex);
}
void CCtrlListView::SetHoverTime(DWORD dwHoverTime)
{	ListView_SetHoverTime(m_hwnd, dwHoverTime);
}
DWORD CCtrlListView::SetIconSpacing(int cx, int cy)
{	return ListView_SetIconSpacing(m_hwnd, cx, cy);
}
HIMAGELIST CCtrlListView::SetImageList(HIMAGELIST himl, int iImageList)
{	return ListView_SetImageList(m_hwnd, himl, iImageList);
}
BOOL CCtrlListView::SetInfoTip(PLVSETINFOTIP plvSetInfoTip)
{	return ListView_SetInfoTip(m_hwnd, plvSetInfoTip);
}
BOOL CCtrlListView::SetInsertMark(LVINSERTMARK *plvim)
{	return ListView_SetInsertMark(m_hwnd, plvim);
}
COLORREF CCtrlListView::SetInsertMarkColor(COLORREF color)
{	return ListView_SetInsertMarkColor(m_hwnd, color);
}
BOOL CCtrlListView::SetItem(const LPLVITEM pitem)
{	return ListView_SetItem(m_hwnd, pitem);
}
void CCtrlListView::SetItemCount(int cItems)
{	ListView_SetItemCount(m_hwnd, cItems);
}
void CCtrlListView::SetItemCountEx(int cItems, DWORD dwFlags)
{	ListView_SetItemCountEx(m_hwnd, cItems, dwFlags);
}
BOOL CCtrlListView::SetItemPosition(int i, int x, int y)
{	return ListView_SetItemPosition(m_hwnd, i, x, y);
}
void CCtrlListView::SetItemPosition32(int iItem, int x, int y)
{	ListView_SetItemPosition32(m_hwnd, iItem, x, y);
}
void CCtrlListView::SetItemState(int i, UINT state, UINT mask)
{	ListView_SetItemState(m_hwnd, i, state, mask);
}
void CCtrlListView::SetItemText(int i, int iSubItem, TCHAR *pszText)
{	ListView_SetItemText(m_hwnd, i, iSubItem, pszText);
}
COLORREF CCtrlListView::SetOutlineColor(COLORREF color)
{	return ListView_SetOutlineColor(m_hwnd, color);
}
void CCtrlListView::SetSelectedColumn(int iCol)
{	ListView_SetSelectedColumn(m_hwnd, iCol);
}
INT CCtrlListView::SetSelectionMark(INT iIndex)
{	return ListView_SetSelectionMark(m_hwnd, iIndex);
}
BOOL CCtrlListView::SetTextBkColor(COLORREF clrText)
{	return ListView_SetTextBkColor(m_hwnd, clrText);
}
BOOL CCtrlListView::SetTextColor(COLORREF clrText)
{	return ListView_SetTextColor(m_hwnd, clrText);
}
BOOL CCtrlListView::SetTileInfo(PLVTILEINFO plvtinfo)
{	return ListView_SetTileInfo(m_hwnd, plvtinfo);
}
BOOL CCtrlListView::SetTileViewInfo(PLVTILEVIEWINFO plvtvinfo)
{	return ListView_SetTileViewInfo(m_hwnd, plvtvinfo);
}
HWND CCtrlListView::SetToolTips(HWND ToolTip)
{	return ListView_SetToolTips(m_hwnd, ToolTip);
}
BOOL CCtrlListView::SetUnicodeFormat(BOOL fUnicode)
{	return ListView_SetUnicodeFormat(m_hwnd, fUnicode);
}
int CCtrlListView::SetView(DWORD iView)
{	return ListView_SetView(m_hwnd, iView);
}
void CCtrlListView::SetWorkAreas(INT nWorkAreas, LPRECT lprc)
{	ListView_SetWorkAreas(m_hwnd, nWorkAreas, lprc);
}
int CCtrlListView::SortGroups(PFNLVGROUPCOMPARE pfnGroupCompare, LPVOID plv)
{	return ListView_SortGroups(m_hwnd, pfnGroupCompare, plv);
}
BOOL CCtrlListView::SortItems(PFNLVCOMPARE pfnCompare, LPARAM lParamSort)
{	return ListView_SortItems(m_hwnd, pfnCompare, lParamSort);
}
BOOL CCtrlListView::SortItemsEx(PFNLVCOMPARE pfnCompare, LPARAM lParamSort)
{	return ListView_SortItemsEx(m_hwnd, pfnCompare, lParamSort);
}
INT CCtrlListView::SubItemHitTest(LPLVHITTESTINFO pInfo)
{	return ListView_SubItemHitTest(m_hwnd, pInfo);
}
BOOL CCtrlListView::Update(int iItem)
{	return ListView_Update(m_hwnd, iItem);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlTreeView

CCtrlTreeView::CCtrlTreeView(CDlgBase* dlg, int ctrlId)
	: CCtrlBase(dlg, ctrlId),
	m_dwFlags(0)
{}

void CCtrlTreeView::SetFlags(uint32_t dwFlags)
{
	if (dwFlags & MTREE_CHECKBOX)
		m_bCheckBox = true;

	if (dwFlags & MTREE_MULTISELECT)
		m_bMultiSelect = true;

	if (dwFlags & MTREE_DND) {
		m_bDndEnabled = true;
		m_bDragging = false;
		m_hDragItem = NULL;
	}
}

void CCtrlTreeView::OnInit()
{
	CSuper::OnInit();

	if (m_bDndEnabled)
		Subclass();
}

HTREEITEM CCtrlTreeView::MoveItemAbove(HTREEITEM hItem, HTREEITEM hInsertAfter)
{
	if (hItem == NULL || hInsertAfter == NULL)
		return NULL;

	if (hItem == hInsertAfter)
		return hItem;

	TCHAR name[128];
	TVINSERTSTRUCT tvis = { 0 };
	tvis.itemex.mask = (UINT)-1;
	tvis.itemex.pszText = name;
	tvis.itemex.cchTextMax = SIZEOF(name);
	tvis.itemex.hItem = hItem;
	if (!GetItem(&tvis.itemex))
		return NULL;

	// the pointed lParam will be freed inside TVN_DELETEITEM
	// so lets substitute it with 0
	LPARAM saveOldData = tvis.itemex.lParam;
	tvis.itemex.lParam = 0;
	SetItem(&tvis.itemex);

	// now current item contain lParam = 0 we can delete it. the memory will be kept.
	DeleteItem(hItem);

	tvis.itemex.stateMask = tvis.itemex.state;
	tvis.itemex.lParam = saveOldData;
	tvis.hParent = NULL;
	tvis.hInsertAfter = hInsertAfter;
	return InsertItem(&tvis);
}

LRESULT CCtrlTreeView::CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	TVHITTESTINFO hti;

	switch (msg) {
	case WM_MOUSEMOVE:
		if (m_bDragging) {
			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam);
			HitTest(&hti);
			if (hti.flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) {
				HTREEITEM it = hti.hItem;
				hti.pt.y -= GetItemHeight() / 2;
				HitTest(&hti);
				if (!(hti.flags & TVHT_ABOVE))
					SetInsertMark(hti.hItem, 1);
				else
					SetInsertMark(it, 0);
			}
			else {
				if (hti.flags & TVHT_ABOVE) SendMsg(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
				if (hti.flags & TVHT_BELOW) SendMsg(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
				SetInsertMark(NULL, 0);
			}
		}
		break;

	case WM_LBUTTONUP:
		if (m_bDragging) {
			SetInsertMark(NULL, 0);
			m_bDragging = 0;
			ReleaseCapture();

			hti.pt.x = (short)LOWORD(lParam);
			hti.pt.y = (short)HIWORD(lParam) - GetItemHeight() / 2;
			HitTest(&hti);
			if (m_hDragItem == hti.hItem)
				break;

			if (hti.flags & TVHT_ABOVE)
				hti.hItem = TVI_FIRST;
			else if (hti.flags & TVHT_BELOW)
				hti.hItem = TVI_LAST;

			HTREEITEM FirstItem = NULL;
			if (m_bMultiSelect) {
				LIST<_TREEITEM> arItems(10);
				GetSelected(arItems);

				// Proceed moving
				HTREEITEM insertAfter = hti.hItem;
				for (int i = 0; i < arItems.getCount(); i++) {
					if (!insertAfter)
						break;

					insertAfter = MoveItemAbove(arItems[i], insertAfter);
					if (!i)
						FirstItem = insertAfter;
				}
			}
			else FirstItem = MoveItemAbove(m_hDragItem, hti.hItem);
			if (FirstItem)
				SelectItem(FirstItem);

			NotifyChange();
		}
		break;

	case WM_LBUTTONDOWN:
		if (!m_bMultiSelect)
			break;

		hti.pt.x = (short)LOWORD(lParam);
		hti.pt.y = (short)HIWORD(lParam);
		if (!TreeView_HitTest(m_hwnd, &hti)) {
			UnselectAll();
			break;
		}

		if (!m_bDndEnabled)
			if (!(wParam & (MK_CONTROL | MK_SHIFT)) || !(hti.flags & (TVHT_ONITEMICON | TVHT_ONITEMLABEL | TVHT_ONITEMRIGHT))) {
				UnselectAll();
				TreeView_SelectItem(m_hwnd, hti.hItem);
				break;
			}

		if (wParam & MK_CONTROL) {
			LIST<_TREEITEM> selected(1);
			GetSelected(selected);

			// Check if have to deselect it
			for (int i = 0; i < selected.getCount(); i++) {
				if (selected[i] == hti.hItem) {
					// Deselect it
					UnselectAll();
					selected.remove(i);

					if (i > 0)
						hti.hItem = selected[0];
					else if (i < selected.getCount())
						hti.hItem = selected[i];
					else
						hti.hItem = NULL;
					break;
				}
			}

			TreeView_SelectItem(m_hwnd, hti.hItem);
			Select(selected);
		}
		else if (wParam & MK_SHIFT) {
			HTREEITEM hItem = TreeView_GetSelection(m_hwnd);
			if (hItem == NULL)
				break;

			LIST<_TREEITEM> selected(1);
			GetSelected(selected);

			TreeView_SelectItem(m_hwnd, hti.hItem);
			Select(selected);
			SelectRange(hItem, hti.hItem);
		}
		break;
	}

	return CSuper::CustomWndProc(msg, wParam, lParam);
}

BOOL CCtrlTreeView::OnNotify(int, NMHDR *pnmh)
{
	TEventInfo evt = { this, pnmh };

	switch (pnmh->code) {
	case TVN_BEGINLABELEDIT: OnBeginLabelEdit(&evt);  return TRUE;
	case TVN_BEGINRDRAG:     OnBeginRDrag(&evt);      return TRUE;
	case TVN_DELETEITEM:     OnDeleteItem(&evt);      return TRUE;
	case TVN_ENDLABELEDIT:   OnEndLabelEdit(&evt);    return TRUE;
	case TVN_GETDISPINFO:    OnGetDispInfo(&evt);     return TRUE;
	case TVN_GETINFOTIP:     OnGetInfoTip(&evt);      return TRUE;
	case TVN_ITEMEXPANDED:   OnItemExpanded(&evt);    return TRUE;
	case TVN_ITEMEXPANDING:  OnItemExpanding(&evt);   return TRUE;
	case TVN_SELCHANGED:     OnSelChanged(&evt);      return TRUE;
	case TVN_SELCHANGING:    OnSelChanging(&evt);     return TRUE;
	case TVN_SETDISPINFO:    OnSetDispInfo(&evt);     return TRUE;
	case TVN_SINGLEEXPAND:   OnSingleExpand(&evt);    return TRUE;

	case TVN_BEGINDRAG:
		OnBeginDrag(&evt);

		// user-defined can clear the event code to disable dragging
		if (m_bDndEnabled && pnmh->code) {
			::SetCapture(m_hwnd);
			m_bDragging = true;
			m_hDragItem = evt.nmtv->itemNew.hItem;
			SelectItem(m_hDragItem);
		}
		return TRUE;

	case TVN_KEYDOWN:
		if (evt.nmtvkey->wVKey == VK_SPACE) {
			if (m_bCheckBox)
				InvertCheck(GetSelection());
			NotifyChange();
		}

		OnKeyDown(&evt);
		return TRUE;
	}

	if (pnmh->code == NM_CLICK) {
		TVHITTESTINFO hti;
		hti.pt.x = (short)LOWORD(GetMessagePos());
		hti.pt.y = (short)HIWORD(GetMessagePos());
		ScreenToClient(pnmh->hwndFrom, &hti.pt);
		if (HitTest(&hti)) {
			if (m_bCheckBox && (hti.flags & TVHT_ONITEMICON) || !m_bCheckBox && (hti.flags & TVHT_ONITEMSTATEICON)) {
				if (m_bCheckBox)
					InvertCheck(hti.hItem);
				NotifyChange();
			}
		}
	}

	return FALSE;
}

void CCtrlTreeView::InvertCheck(HTREEITEM hItem)
{
	TVITEMEX tvi;
	tvi.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvi.hItem = hItem;
	if (!GetItem(&tvi))
		return;

	tvi.iImage = tvi.iSelectedImage = !tvi.iImage;
	SetItem(&tvi);
}

void CCtrlTreeView::TranslateItem(HTREEITEM hItem)
{
	TVITEMEX tvi;
	TCHAR buf[128];
	GetItem(hItem, &tvi, buf, SIZEOF(buf));
	tvi.pszText = TranslateTS(tvi.pszText);
	SetItem(&tvi);
}

void CCtrlTreeView::TranslateTree()
{
	HTREEITEM hItem = GetRoot();
	while (hItem) {
		TranslateItem(hItem);

		HTREEITEM hItemTmp = 0;
		if (hItemTmp = GetChild(hItem))
			hItem = hItemTmp;
		else if (hItemTmp = GetNextSibling(hItem))
			hItem = hItemTmp;
		else {
			while (true) {
				if (!(hItem = GetParent(hItem)))
					break;
				if (hItemTmp = GetNextSibling(hItem)) {
					hItem = hItemTmp;
					break;
				}
			}
		}
	}
}

HTREEITEM CCtrlTreeView::FindNamedItem(HTREEITEM hItem, const TCHAR *name)
{
	TVITEMEX tvi = { 0 };
	TCHAR str[MAX_PATH];

	if (hItem)
		tvi.hItem = GetChild(hItem);
	else
		tvi.hItem = GetRoot();

	if (!name)
		return tvi.hItem;

	tvi.mask = TVIF_TEXT;
	tvi.pszText = str;
	tvi.cchTextMax = SIZEOF(str);

	while (tvi.hItem) {
		GetItem(&tvi);

		if (!mir_tstrcmp(tvi.pszText, name))
			return tvi.hItem;

		tvi.hItem = GetNextSibling(tvi.hItem);
	}
	return NULL;
}

void CCtrlTreeView::GetItem(HTREEITEM hItem, TVITEMEX *tvi)
{
	memset(tvi, 0, sizeof(*tvi));
	tvi->mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_INTEGRAL | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE;
	tvi->hItem = hItem;
	GetItem(tvi);
}

void CCtrlTreeView::GetItem(HTREEITEM hItem, TVITEMEX *tvi, TCHAR *szText, int iTextLength)
{
	memset(tvi, 0, sizeof(*tvi));
	tvi->mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_INTEGRAL | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
	tvi->hItem = hItem;
	tvi->pszText = szText;
	tvi->cchTextMax = iTextLength;
	GetItem(tvi);
}

bool CCtrlTreeView::IsSelected(HTREEITEM hItem)
{
	return (TVIS_SELECTED & TreeView_GetItemState(m_hwnd, hItem, TVIS_SELECTED)) == TVIS_SELECTED;
}

void CCtrlTreeView::Select(HTREEITEM hItem)
{
	TreeView_SetItemState(m_hwnd, hItem, TVIS_SELECTED, TVIS_SELECTED);
}

void CCtrlTreeView::Unselect(HTREEITEM hItem)
{
	TreeView_SetItemState(m_hwnd, hItem, 0, TVIS_SELECTED);
}

void CCtrlTreeView::DropHilite(HTREEITEM hItem)
{
	TreeView_SetItemState(m_hwnd, hItem, TVIS_DROPHILITED, TVIS_DROPHILITED);
}

void CCtrlTreeView::DropUnhilite(HTREEITEM hItem)
{
	TreeView_SetItemState(m_hwnd, hItem, 0, TVIS_DROPHILITED);
}

void CCtrlTreeView::SelectAll()
{
	TreeView_SelectItem(m_hwnd, NULL);

	HTREEITEM hItem = TreeView_GetRoot(m_hwnd);
	while (hItem) {
		Select(hItem);
		hItem = TreeView_GetNextSibling(m_hwnd, hItem);
	}
}

void CCtrlTreeView::UnselectAll()
{
	TreeView_SelectItem(m_hwnd, NULL);

	HTREEITEM hItem = TreeView_GetRoot(m_hwnd);
	while (hItem) {
		Unselect(hItem);
		hItem = TreeView_GetNextSibling(m_hwnd, hItem);
	}
}

void CCtrlTreeView::SelectRange(HTREEITEM hStart, HTREEITEM hEnd)
{
	int start = 0, end = 0, i = 0;
	HTREEITEM hItem = TreeView_GetRoot(m_hwnd);
	while (hItem) {
		if (hItem == hStart)
			start = i;
		if (hItem == hEnd)
			end = i;

		i++;
		hItem = TreeView_GetNextSibling(m_hwnd, hItem);
	}

	if (end < start) {
		int tmp = start;
		start = end;
		end = tmp;
	}

	i = 0;
	hItem = TreeView_GetRoot(m_hwnd);
	while (hItem) {
		if (i >= start)
			Select(hItem);
		if (i == end)
			break;

		i++;
		hItem = TreeView_GetNextSibling(m_hwnd, hItem);
	}
}

int CCtrlTreeView::GetNumSelected()
{
	int ret = 0;
	for (HTREEITEM hItem = TreeView_GetRoot(m_hwnd); hItem; hItem = TreeView_GetNextSibling(m_hwnd, hItem))
		if (IsSelected(hItem))
			ret++;

	return ret;
}

void CCtrlTreeView::GetSelected(LIST<_TREEITEM> &selected)
{
	HTREEITEM hItem = TreeView_GetRoot(m_hwnd);
	while (hItem) {
		if (IsSelected(hItem))
			selected.insert(hItem);
		hItem = TreeView_GetNextSibling(m_hwnd, hItem);
	}
}

void CCtrlTreeView::Select(LIST<_TREEITEM> &selected)
{
	for (int i = 0; i < selected.getCount(); i++)
		if (selected[i] != NULL)
			Select(selected[i]);
}

/////////////////////////////////////////////////////////////////////////////////////////

HIMAGELIST CCtrlTreeView::CreateDragImage(HTREEITEM hItem)
{	return TreeView_CreateDragImage(m_hwnd, hItem);
}

void CCtrlTreeView::DeleteAllItems()
{	TreeView_DeleteAllItems(m_hwnd);
}

void CCtrlTreeView::DeleteItem(HTREEITEM hItem)
{	TreeView_DeleteItem(m_hwnd, hItem);
}

HWND CCtrlTreeView::EditLabel(HTREEITEM hItem)
{	return TreeView_EditLabel(m_hwnd, hItem);
}

void CCtrlTreeView::EndEditLabelNow(BOOL cancel)
{	TreeView_EndEditLabelNow(m_hwnd, cancel);
}

void CCtrlTreeView::EnsureVisible(HTREEITEM hItem)
{	TreeView_EnsureVisible(m_hwnd, hItem);
}

void CCtrlTreeView::Expand(HTREEITEM hItem, DWORD flag)
{	TreeView_Expand(m_hwnd, hItem, flag);
}

COLORREF CCtrlTreeView::GetBkColor()
{	return TreeView_GetBkColor(m_hwnd);
}

DWORD CCtrlTreeView::GetCheckState(HTREEITEM hItem)
{	return TreeView_GetCheckState(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetChild(HTREEITEM hItem)
{	return TreeView_GetChild(m_hwnd, hItem);
}

int CCtrlTreeView::GetCount()
{	return TreeView_GetCount(m_hwnd);
}

HTREEITEM CCtrlTreeView::GetDropHilight()
{	return TreeView_GetDropHilight(m_hwnd);
}

HWND CCtrlTreeView::GetEditControl()
{	return TreeView_GetEditControl(m_hwnd);
}

HTREEITEM CCtrlTreeView::GetFirstVisible()
{	return TreeView_GetFirstVisible(m_hwnd);
}

HIMAGELIST CCtrlTreeView::GetImageList(int iImage)
{	return TreeView_GetImageList(m_hwnd, iImage);
}

int CCtrlTreeView::GetIndent()
{	return TreeView_GetIndent(m_hwnd);
}

COLORREF CCtrlTreeView::GetInsertMarkColor()
{	return TreeView_GetInsertMarkColor(m_hwnd);
}

bool CCtrlTreeView::GetItem(TVITEMEX *tvi)
{	return TreeView_GetItem(m_hwnd, tvi) == TRUE;
}

int CCtrlTreeView::GetItemHeight()
{	return TreeView_GetItemHeight(m_hwnd);
}

void CCtrlTreeView::GetItemRect(HTREEITEM hItem, RECT *rcItem, BOOL fItemRect)
{	TreeView_GetItemRect(m_hwnd, hItem, rcItem, fItemRect);
}

DWORD CCtrlTreeView::GetItemState(HTREEITEM hItem, DWORD stateMask)
{	return TreeView_GetItemState(m_hwnd, hItem, stateMask);
}

HTREEITEM CCtrlTreeView::GetLastVisible()
{	return TreeView_GetLastVisible(m_hwnd);
}

COLORREF CCtrlTreeView::GetLineColor()
{	return TreeView_GetLineColor(m_hwnd);
}

HTREEITEM CCtrlTreeView::GetNextItem(HTREEITEM hItem, DWORD flag)
{	return TreeView_GetNextItem(m_hwnd, hItem, flag);
}

HTREEITEM CCtrlTreeView::GetNextSibling(HTREEITEM hItem)
{	return TreeView_GetNextSibling(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetNextVisible(HTREEITEM hItem)
{	return TreeView_GetNextVisible(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetParent(HTREEITEM hItem)
{	return TreeView_GetParent(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetPrevSibling(HTREEITEM hItem)
{	return TreeView_GetPrevSibling(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetPrevVisible(HTREEITEM hItem)
{	return TreeView_GetPrevVisible(m_hwnd, hItem);
}

HTREEITEM CCtrlTreeView::GetRoot()
{	return TreeView_GetRoot(m_hwnd);
}

DWORD CCtrlTreeView::GetScrollTime()
{	return TreeView_GetScrollTime(m_hwnd);
}

HTREEITEM CCtrlTreeView::GetSelection()
{	return TreeView_GetSelection(m_hwnd);
}

COLORREF CCtrlTreeView::GetTextColor()
{	return TreeView_GetTextColor(m_hwnd);
}

HWND CCtrlTreeView::GetToolTips()
{	return TreeView_GetToolTips(m_hwnd);
}

BOOL CCtrlTreeView::GetUnicodeFormat()
{	return TreeView_GetUnicodeFormat(m_hwnd);
}

unsigned CCtrlTreeView::GetVisibleCount()
{	return TreeView_GetVisibleCount(m_hwnd);
}

HTREEITEM CCtrlTreeView::HitTest(TVHITTESTINFO *hti)
{	return TreeView_HitTest(m_hwnd, hti);
}

HTREEITEM CCtrlTreeView::InsertItem(TVINSERTSTRUCT *tvis)
{	return TreeView_InsertItem(m_hwnd, tvis);
}

void CCtrlTreeView::Select(HTREEITEM hItem, DWORD flag)
{	TreeView_Select(m_hwnd, hItem, flag);
}

void CCtrlTreeView::SelectDropTarget(HTREEITEM hItem)
{	TreeView_SelectDropTarget(m_hwnd, hItem);
}

void CCtrlTreeView::SelectItem(HTREEITEM hItem)
{	TreeView_SelectItem(m_hwnd, hItem);
}

void CCtrlTreeView::SelectSetFirstVisible(HTREEITEM hItem)
{	TreeView_SelectSetFirstVisible(m_hwnd, hItem);
}

COLORREF CCtrlTreeView::SetBkColor(COLORREF clBack)
{	return TreeView_SetBkColor(m_hwnd, clBack);
}

void CCtrlTreeView::SetCheckState(HTREEITEM hItem, DWORD state)
{	TreeView_SetCheckState(m_hwnd, hItem, state);
}

void CCtrlTreeView::SetImageList(HIMAGELIST hIml, int iImage)
{	TreeView_SetImageList(m_hwnd, hIml, iImage);
}

void CCtrlTreeView::SetIndent(int iIndent)
{	TreeView_SetIndent(m_hwnd, iIndent);
}

void CCtrlTreeView::SetInsertMark(HTREEITEM hItem, BOOL fAfter)
{	TreeView_SetInsertMark(m_hwnd, hItem, fAfter);
}

COLORREF CCtrlTreeView::SetInsertMarkColor(COLORREF clMark)
{	return TreeView_SetInsertMarkColor(m_hwnd, clMark);
}

void CCtrlTreeView::SetItem(TVITEMEX *tvi)
{	TreeView_SetItem(m_hwnd, tvi);
}

void CCtrlTreeView::SetItemHeight(short cyItem)
{	TreeView_SetItemHeight(m_hwnd, cyItem);
}

void CCtrlTreeView::SetItemState(HTREEITEM hItem, DWORD state, DWORD stateMask)
{	TreeView_SetItemState(m_hwnd, hItem, state, stateMask);
}

COLORREF CCtrlTreeView::SetLineColor(COLORREF clLine)
{	return TreeView_SetLineColor(m_hwnd, clLine);
}

void CCtrlTreeView::SetScrollTime(UINT uMaxScrollTime)
{	TreeView_SetScrollTime(m_hwnd, uMaxScrollTime);
}

COLORREF CCtrlTreeView::SetTextColor(COLORREF clText)
{	return TreeView_SetTextColor(m_hwnd, clText);
}

HWND CCtrlTreeView::SetToolTips(HWND hwndToolTips)
{	return TreeView_SetToolTips(m_hwnd, hwndToolTips);
}

BOOL CCtrlTreeView::SetUnicodeFormat(BOOL fUnicode)
{	return TreeView_SetUnicodeFormat(m_hwnd, fUnicode);
}

void CCtrlTreeView::SortChildren(HTREEITEM hItem, BOOL fRecurse)
{	TreeView_SortChildren(m_hwnd, hItem, fRecurse);
}

void CCtrlTreeView::SortChildrenCB(TVSORTCB *cb, BOOL fRecurse)
{	TreeView_SortChildrenCB(m_hwnd, cb, fRecurse);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlPages

CCtrlPages::CCtrlPages(CDlgBase* dlg, int ctrlId)
	: CCtrlBase(dlg, ctrlId),
	m_hIml(NULL),
	m_pActivePage(NULL)
{}

void CCtrlPages::OnInit()
{
	CSuper::OnInit();
	Subclass();
}

LRESULT CCtrlPages::CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_SIZE)
		ShowPage(m_pActivePage);

	return CSuper::CustomWndProc(msg, wParam, lParam);
}

void CCtrlPages::AddPage(TCHAR *ptszName, HICON hIcon, CCallback<void> onCreate, void *param)
{
	TPageInfo *info = new TPageInfo;
	info->m_onCreate = onCreate;
	info->m_param = param;
	info->m_pDlg = NULL;

	TCITEM tci = { 0 };
	tci.mask = TCIF_PARAM | TCIF_TEXT;
	tci.lParam = (LPARAM)info;
	tci.pszText = ptszName;
	if (hIcon) {
		if (!m_hIml) {
			m_hIml = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 1);
			TabCtrl_SetImageList(m_hwnd, m_hIml);
		}

		tci.mask |= TCIF_IMAGE;
		tci.iImage = ImageList_AddIcon(m_hIml, hIcon);
	}

	TabCtrl_InsertItem(m_hwnd, TabCtrl_GetItemCount(m_hwnd), &tci);
}

void CCtrlPages::AttachDialog(int iPage, CDlgBase *pDlg)
{
	if ((iPage < 0) || (iPage >= TabCtrl_GetItemCount(m_hwnd)))
		return;

	TCITEM tci = { 0 };
	tci.mask = TCIF_PARAM;
	TabCtrl_GetItem(m_hwnd, iPage, &tci);

	if (TPageInfo *info = (TPageInfo *)tci.lParam) {
		if (info->m_pDlg)
			info->m_pDlg->Close();

		info->m_pDlg = pDlg;
		if (pDlg->GetHwnd() == NULL) {
			pDlg->SetParent(m_hwnd);
			pDlg->Create();
		}

		if (iPage == TabCtrl_GetCurSel(m_hwnd)) {
			m_pActivePage = pDlg;
			ShowPage(pDlg);
		}
	}
}

void CCtrlPages::ShowPage(CDlgBase *pDlg)
{
	if (!pDlg) return;

	RECT rc;
	GetClientRect(m_hwnd, &rc);
	TabCtrl_AdjustRect(m_hwnd, FALSE, &rc);

	EnableThemeDialogTexture(pDlg->GetHwnd(), ETDT_ENABLETAB);
	SetWindowPos(pDlg->GetHwnd(), HWND_TOP, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
}

void CCtrlPages::ActivatePage(int iPage)
{
	TabCtrl_SetCurSel(m_hwnd, iPage);
}

BOOL CCtrlPages::OnNotify(int /*idCtrl*/, NMHDR *pnmh)
{
	switch (pnmh->code) {
	case TCN_SELCHANGING:
		{
			TCITEM tci = { 0 };
			tci.mask = TCIF_PARAM;
			TabCtrl_GetItem(m_hwnd, TabCtrl_GetCurSel(m_hwnd), &tci);

			if (TPageInfo *info = (TPageInfo *)tci.lParam) {
				if (info->m_pDlg) {
					m_pActivePage = NULL;
					ShowWindow(info->m_pDlg->GetHwnd(), SW_HIDE);
				}
			}
		}
		return TRUE;

	case TCN_SELCHANGE:
		{
			TCITEM tci = { 0 };
			tci.mask = TCIF_PARAM;
			TabCtrl_GetItem(m_hwnd, TabCtrl_GetCurSel(m_hwnd), &tci);

			if (TPageInfo *info = (TPageInfo *)tci.lParam) {
				if (info->m_pDlg) {
					m_pActivePage = info->m_pDlg;
					ShowPage(info->m_pDlg);
				}
				else {
					m_pActivePage = NULL;
					info->m_onCreate(info->m_param);
				}
			}
		}
		return TRUE;
	}

	return FALSE;
}

void CCtrlPages::OnDestroy()
{
	int count = TabCtrl_GetItemCount(m_hwnd);
	for (int i = 0; i < count; i++) {
		TCITEM tci = { 0 };
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem(m_hwnd, i, &tci);

		if (TPageInfo *info = (TPageInfo *)tci.lParam) {
			if (info->m_pDlg)
				info->m_pDlg->Close();

			delete info;
		}
	}

	TabCtrl_DeleteAllItems(m_hwnd);

	if (m_hIml) {
		TabCtrl_SetImageList(m_hwnd, NULL);
		ImageList_Destroy(m_hIml);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlBase

CCtrlBase::CCtrlBase(CDlgBase *wnd, int idCtrl)
	: m_parentWnd(wnd),
	m_idCtrl(idCtrl),
	m_hwnd(NULL),
	m_bChanged(false)
{
	if (wnd) {
		m_next = wnd->m_first;
		wnd->m_first = this;
	}
	else m_next = NULL;
}

void CCtrlBase::OnInit()
{
	m_hwnd = (m_idCtrl && m_parentWnd && m_parentWnd->GetHwnd()) ? GetDlgItem(m_parentWnd->GetHwnd(), m_idCtrl) : NULL;
}

void CCtrlBase::OnDestroy()
{
	m_hwnd = NULL;
}

void CCtrlBase::OnApply()
{
	m_bChanged = false;
}

void CCtrlBase::OnReset()
{}

void CCtrlBase::Enable(int bIsEnable)
{
	::EnableWindow(m_hwnd, bIsEnable);
}

BOOL CCtrlBase::Enabled() const
{
	return (m_hwnd) ? IsWindowEnabled(m_hwnd) : FALSE;
}

void CCtrlBase::NotifyChange()
{
	if (!m_parentWnd || m_parentWnd->IsInitialized())
		m_bChanged = true;

	if (m_parentWnd) {
		m_parentWnd->OnChange(this);
		if (m_parentWnd->IsInitialized())
			::SendMessage(::GetParent(m_parentWnd->GetHwnd()), PSM_CHANGED, 0, 0);
	}

	OnChange(this);
}

LRESULT CCtrlBase::SendMsg(UINT Msg, WPARAM wParam, LPARAM lParam)
{
	return ::SendMessage(m_hwnd, Msg, wParam, lParam);
}

void CCtrlBase::SetText(const TCHAR *text)
{
	::SetWindowText(m_hwnd, text);
}

void CCtrlBase::SetTextA(const char *text)
{
	::SetWindowTextA(m_hwnd, text);
}

void CCtrlBase::SetInt(int value)
{
	TCHAR buf[32] = { 0 };
	mir_sntprintf(buf, SIZEOF(buf), _T("%d"), value);
	SetWindowText(m_hwnd, buf);
}

TCHAR* CCtrlBase::GetText()
{
	int length = GetWindowTextLength(m_hwnd) + 1;
	TCHAR *result = (TCHAR *)mir_alloc(length * sizeof(TCHAR));
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

TCHAR* CCtrlBase::GetText(TCHAR *buf, int size)
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
	TCHAR *result = (TCHAR *)_alloca(length * sizeof(TCHAR));
	GetWindowText(m_hwnd, result, length);
	return _ttoi(result);
}

LRESULT CCtrlBase::CustomWndProc(UINT msg, WPARAM wParam, LPARAM lParam)
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
		if (msg == WM_DESTROY) {
			pCtrl->Unsubclass();
			arControls.remove(pCtrl);
		}

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

/////////////////////////////////////////////////////////////////////////////////////////
// CDbLink class

CDbLink::CDbLink(const char *szModule, const char *szSetting, BYTE type, DWORD iValue)
	: CDataLink(type)
{
	m_szModule = mir_strdup(szModule);
	m_szSetting = mir_strdup(szSetting);
	m_iDefault = iValue;
	m_szDefault = 0;
	dbv.type = DBVT_DELETED;
}

CDbLink::CDbLink(const char *szModule, const char *szSetting, BYTE type, TCHAR *szValue)
	: CDataLink(type)
{
	m_szModule = mir_strdup(szModule);
	m_szSetting = mir_strdup(szSetting);
	m_szDefault = mir_tstrdup(szValue);
	dbv.type = DBVT_DELETED;
}

CDbLink::~CDbLink()
{
	mir_free(m_szModule);
	mir_free(m_szSetting);
	mir_free(m_szDefault);
	if (dbv.type != DBVT_DELETED)
		db_free(&dbv);
}

DWORD CDbLink::LoadInt()
{
	switch (m_type) {
		case DBVT_BYTE:  return db_get_b(NULL, m_szModule, m_szSetting, m_iDefault);
		case DBVT_WORD:  return db_get_w(NULL, m_szModule, m_szSetting, m_iDefault);
		case DBVT_DWORD: return db_get_dw(NULL, m_szModule, m_szSetting, m_iDefault);
		default:         return m_iDefault;
	}
}

void CDbLink::SaveInt(DWORD value)
{
	switch (m_type) {
		case DBVT_BYTE:  db_set_b(NULL, m_szModule, m_szSetting, (BYTE)value); break;
		case DBVT_WORD:  db_set_w(NULL, m_szModule, m_szSetting, (WORD)value); break;
		case DBVT_DWORD: db_set_dw(NULL, m_szModule, m_szSetting, value); break;
	}
}

TCHAR* CDbLink::LoadText()
{
	if (dbv.type != DBVT_DELETED) db_free(&dbv);
	if (!db_get_ts(NULL, m_szModule, m_szSetting, &dbv)) {
		if (dbv.type == DBVT_TCHAR)
			return dbv.ptszVal;
		return m_szDefault;
	}

	dbv.type = DBVT_DELETED;
	return m_szDefault;
}

void CDbLink::SaveText(TCHAR *value)
{
	db_set_ts(NULL, m_szModule, m_szSetting, value);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Base protocol dialog

CProtoIntDlgBase::CProtoIntDlgBase(PROTO_INTERFACE *proto, int idDialog, bool show_label)
	: CDlgBase(::ProtoGetInstance(proto->m_szModuleName), idDialog),
	m_proto_interface(proto),
	m_show_label(show_label),
	m_hwndStatus(NULL)
{}

void CProtoIntDlgBase::CreateLink(CCtrlData& ctrl, char *szSetting, BYTE type, DWORD iValue)
{
	ctrl.CreateDbLink(m_proto_interface->m_szModuleName, szSetting, type, iValue);
}

void CProtoIntDlgBase::CreateLink(CCtrlData& ctrl, const char *szSetting, TCHAR *szValue)
{
	ctrl.CreateDbLink(m_proto_interface->m_szModuleName, szSetting, szValue);
}

void CProtoIntDlgBase::OnProtoRefresh(WPARAM, LPARAM) {}
void CProtoIntDlgBase::OnProtoActivate(WPARAM, LPARAM) {}
void CProtoIntDlgBase::OnProtoCheckOnline(WPARAM, LPARAM) {}

void CProtoIntDlgBase::SetStatusText(const TCHAR *statusText)
{
	if (m_hwndStatus)
		SendMessage(m_hwndStatus, SB_SETTEXT, 0, (LPARAM)statusText);
}

INT_PTR CProtoIntDlgBase::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result;

	switch (msg) {
	case WM_INITDIALOG: // call inherited init code first
		result = CSuper::DlgProc(msg, wParam, lParam);
		m_proto_interface->WindowSubscribe(m_hwnd);
		if (m_show_label) {
			m_hwndStatus = CreateStatusWindow(WS_CHILD | WS_VISIBLE, NULL, m_hwnd, 999);
			SetWindowPos(m_hwndStatus, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			UpdateStatusBar();
			UpdateProtoTitle();
		}
		return result;

	case WM_DESTROY:
		Skin_ReleaseIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, 0));
		Skin_ReleaseIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, 0));
		m_proto_interface->WindowUnsubscribe(m_hwnd);
		break;

	case WM_SETTEXT:
		if (m_show_label && IsWindowUnicode(m_hwnd)) {
			TCHAR *szTitle = (TCHAR *)lParam;
			if (!_tcsstr(szTitle, m_proto_interface->m_tszUserName)) {
				UpdateProtoTitle(szTitle);
				return TRUE;
			}
		}
		break;

	case WM_SIZE:
		if (m_hwndStatus) {
			RECT rcStatus; GetWindowRect(m_hwndStatus, &rcStatus);
			RECT rcClient; GetClientRect(m_hwnd, &rcClient);
			SetWindowPos(m_hwndStatus, NULL, 0, rcClient.bottom - (rcStatus.bottom - rcStatus.top), rcClient.right, (rcStatus.bottom - rcStatus.top), SWP_NOZORDER);
			UpdateStatusBar();
		}
		break;

		// Protocol events
	case WM_PROTO_ACTIVATE:
		OnProtoActivate(wParam, lParam);
		return m_lresult;

	case WM_PROTO_CHECK_ONLINE:
		if (m_hwndStatus)
			UpdateStatusBar();
		OnProtoCheckOnline(wParam, lParam);
		return m_lresult;

	case WM_PROTO_REFRESH:
		OnProtoRefresh(wParam, lParam);
		return m_lresult;
	}

	return CSuper::DlgProc(msg, wParam, lParam);
}

void CProtoIntDlgBase::UpdateProtoTitle(const TCHAR *szText)
{
	if (!m_show_label)
		return;

	int curLength;
	const TCHAR *curText;

	if (szText) {
		curText = szText;
		curLength = (int)mir_tstrlen(curText);
	}
	else {
		curLength = GetWindowTextLength(m_hwnd) + 1;
		TCHAR *tmp = (TCHAR *)_alloca(curLength * sizeof(TCHAR));
		GetWindowText(m_hwnd, tmp, curLength);
		curText = tmp;
	}

	if (!_tcsstr(curText, m_proto_interface->m_tszUserName)) {
		size_t length = curLength + mir_tstrlen(m_proto_interface->m_tszUserName) + 256;
		TCHAR *text = (TCHAR *)_alloca(length * sizeof(TCHAR));
		mir_sntprintf(text, length, _T("%s [%s: %s]"), curText, TranslateT("Account"), m_proto_interface->m_tszUserName);
		SetWindowText(m_hwnd, text);
	}
}

void CProtoIntDlgBase::UpdateStatusBar()
{
	SIZE sz;

	HDC hdc = GetDC(m_hwndStatus);
	HFONT hFntSave = (HFONT)SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	GetTextExtentPoint32(hdc, m_proto_interface->m_tszUserName, (int)mir_tstrlen(m_proto_interface->m_tszUserName), &sz);
	sz.cx += GetSystemMetrics(SM_CXSMICON) * 3;
	SelectObject(hdc, hFntSave);
	ReleaseDC(m_hwndStatus, hdc);

	RECT rcStatus; GetWindowRect(m_hwndStatus, &rcStatus);
	int parts[] = { rcStatus.right - rcStatus.left - sz.cx, -1 };
	SendMessage(m_hwndStatus, SB_SETPARTS, 2, (LPARAM)parts);
	SendMessage(m_hwndStatus, SB_SETICON, 1, (LPARAM)LoadSkinnedProtoIcon(m_proto_interface->m_szModuleName, m_proto_interface->m_iStatus));
	SendMessage(m_hwndStatus, SB_SETTEXT, 1, (LPARAM)m_proto_interface->m_tszUserName);
}
