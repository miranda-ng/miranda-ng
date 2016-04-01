/*
Copyright (c) 2013-16 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

////////////////////////////////// IDD_CAPTCHAFORM ////////////////////////////////////////

CVkCaptchaForm::CVkCaptchaForm(CVkProto *proto, CAPTCHA_FORM_PARAMS* param) :
	CVkDlgBase(proto, IDD_CAPTCHAFORM, false),
	m_instruction(this, IDC_INSTRUCTION),
	m_edtValue(this, IDC_VALUE),
	m_btnOpenInBrowser(this, IDOPENBROWSER),
	m_btnOk(this, IDOK),
	m_param(param)
{
	m_btnOpenInBrowser.OnClick = Callback(this, &CVkCaptchaForm::On_btnOpenInBrowser_Click);
	m_btnOk.OnClick = Callback(this, &CVkCaptchaForm::On_btnOk_Click);
	m_edtValue.OnChange = Callback(this, &CVkCaptchaForm::On_edtValue_Change);
}

void CVkCaptchaForm::OnInitDialog()
{
	SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)IcoLib_GetIconByHandle(GetIconHandle(IDI_KEYS), TRUE));
	SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)IcoLib_GetIconByHandle(GetIconHandle(IDI_KEYS)));

	m_btnOk.Disable();
	m_btnOpenInBrowser.Enable((m_param->bmp != NULL));
	m_instruction.SetText(TranslateT("Enter the text you see"));
}

INT_PTR CVkCaptchaForm::DlgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_CTLCOLORSTATIC:
		switch (GetWindowLongPtr((HWND)lParam, GWL_ID)) {
		case IDC_WHITERECT:
		case IDC_INSTRUCTION:
		case IDC_TITLE:
			return (INT_PTR)GetStockObject(WHITE_BRUSH);
		}
		return NULL;

	case WM_PAINT:
		if (m_param->bmp) {
			PAINTSTRUCT ps;
			HDC hdc, hdcMem;
			RECT rc;

			GetClientRect(m_hwnd, &rc);
			hdc = BeginPaint(m_hwnd, &ps);
			hdcMem = CreateCompatibleDC(hdc);
			HGDIOBJ hOld = SelectObject(hdcMem, m_param->bmp);

			int y = (rc.bottom + rc.top - m_param->h) / 2;
			int x = (rc.right + rc.left - m_param->w) / 2;
			BitBlt(hdc, x, y, m_param->w, m_param->h, hdcMem, 0, 0, SRCCOPY);
			SelectObject(hdcMem, hOld);
			DeleteDC(hdcMem);
			EndPaint(m_hwnd, &ps);

			if (m_proto->getBool("AlwaysOpenCaptchaInBrowser", false))
				m_proto->ShowCaptchaInBrowser(m_param->bmp);
		}
		break;

	}
	return CDlgBase::DlgProc(msg, wParam, lParam);
}

void CVkCaptchaForm::OnDestroy()
{
	IcoLib_ReleaseIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, 0));
	IcoLib_ReleaseIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, 0));
}

void CVkCaptchaForm::On_btnOpenInBrowser_Click(CCtrlButton*)
{
	m_proto->ShowCaptchaInBrowser(m_param->bmp);
}

void CVkCaptchaForm::On_btnOk_Click(CCtrlButton*)
{
	m_edtValue.GetTextA(m_param->Result, _countof(m_param->Result));
	EndDialog(m_hwnd, 1);
}

void CVkCaptchaForm::On_edtValue_Change(CCtrlEdit*)
{
	m_btnOk.Enable(!IsEmpty(ptrA(m_edtValue.GetTextA())));
}

////////////////////////////////// IDD_WALLPOST ///////////////////////////////////////////

CVkWallPostForm::CVkWallPostForm(CVkProto* proto, WALLPOST_FORM_PARAMS* param) :
	CVkDlgBase(proto, IDD_WALLPOST, false),
	m_edtMsg(this, IDC_ED_MSG),
	m_edtUrl(this, IDC_ED_URL),
	m_cbOnlyForFriends(this, IDC_ONLY_FRIENDS),
	m_btnShare(this, IDOK),
	m_param(param)
{
	m_btnShare.OnClick = Callback(this, &CVkWallPostForm::On_btnShare_Click);
	m_edtMsg.OnChange = Callback(this, &CVkWallPostForm::On_edtValue_Change);
	m_edtUrl.OnChange = Callback(this, &CVkWallPostForm::On_edtValue_Change);
}

void CVkWallPostForm::OnInitDialog()
{
	SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)IcoLib_GetIconByHandle(GetIconHandle(IDI_WALL), TRUE));
	SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)IcoLib_GetIconByHandle(GetIconHandle(IDI_WALL)));
	
	CMString tszTitle(FORMAT, _T("%s %s"), TranslateT("Wall message for"), m_param->ptszNick);
	SetCaption(tszTitle);
	
	m_btnShare.Disable();
}

void CVkWallPostForm::OnDestroy()
{
	IcoLib_ReleaseIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_BIG, 0));
	IcoLib_ReleaseIcon((HICON)SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, 0));
}

void CVkWallPostForm::On_btnShare_Click(CCtrlButton*)
{
	m_param->ptszUrl = m_edtUrl.GetText();
	m_param->ptszMsg = m_edtMsg.GetText();
	m_param->bFriendsOnly = m_cbOnlyForFriends.GetState() != 0;

	EndDialog(m_hwnd, 1);
}

void CVkWallPostForm::On_edtValue_Change(CCtrlEdit*)
{
	m_btnShare.Enable(!IsEmpty(ptrT(m_edtMsg.GetText())) || !IsEmpty(ptrT(m_edtUrl.GetText())));
}

////////////////////////////////// IDD_INVITE /////////////////////////////////////////////

CVkInviteChatForm::CVkInviteChatForm(CVkProto* proto) :
	CVkDlgBase(proto, IDD_INVITE, false), 
	m_btnOk(this, IDOK),  
	m_cbxCombo(this, IDC_CONTACT), 
	m_hContact(NULL)
{
	m_btnOk.OnClick = Callback(this, &CVkInviteChatForm::btnOk_OnOk);
}

void CVkInviteChatForm::OnInitDialog()
{
	for (MCONTACT hContact = db_find_first(m_proto->m_szModuleName); hContact; hContact = db_find_next(hContact, m_proto->m_szModuleName)) {
		if (!m_proto->isChatRoom(hContact)) {
			TCHAR *ptszNick = pcli->pfnGetContactDisplayName(hContact, 0);
			m_cbxCombo.AddString(ptszNick, hContact);
		}	
	}
}

void CVkInviteChatForm::btnOk_OnOk(CCtrlButton*)
{
	m_hContact = m_cbxCombo.GetItemData(m_cbxCombo.GetCurSel());
	EndDialog(m_hwnd, 1);
}

////////////////////////////////// IDD_GC_CREATE //////////////////////////////////////////

CVkGCCreateForm::CVkGCCreateForm(CVkProto* proto) :
	CVkDlgBase(proto, IDD_GC_CREATE, false), 
	m_btnOk(this, IDOK),  
	m_clCList(this, IDC_CLIST), 
	m_edtTitle(this, IDC_TITLE)
{
	m_btnOk.OnClick = Callback(this, &CVkGCCreateForm::btnOk_OnOk);
}

void CVkGCCreateForm::OnInitDialog()
{
	SetWindowLongPtr(m_clCList.GetHwnd(), GWL_STYLE, GetWindowLongPtr(m_clCList.GetHwnd(), GWL_STYLE) 
		| CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
	m_clCList.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

	ResetListOptions(&m_clCList);
	FilterList(&m_clCList);
}

void CVkGCCreateForm::btnOk_OnOk(CCtrlButton*)
{
	CMStringA szUIds;
	for (MCONTACT hContact = db_find_first(m_proto->m_szModuleName); hContact; hContact = db_find_next(hContact, m_proto->m_szModuleName)) {
		if (m_proto->isChatRoom(hContact))
			continue;

		HANDLE hItem = m_clCList.FindContact(hContact);
		if (hItem && m_clCList.GetCheck(hItem)) {
			int uid = m_proto->getDword(hContact, "ID");
			if (uid != 0) {
				if (!szUIds.IsEmpty())
					szUIds.AppendChar(',');
				szUIds.AppendFormat("%d", uid);
			}
		}
	}

	bool bRes = !szUIds.IsEmpty();
	if (bRes)
		m_proto->CreateNewChat(szUIds, ptrT(m_edtTitle.GetText()));

	EndDialog(m_hwnd, bRes);
}

void CVkGCCreateForm::FilterList(CCtrlClc* clCList)
{
	if (!clCList)
		return;

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *proto = GetContactProto(hContact);
		if (mir_strcmp(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact))
			if (HANDLE hItem = clCList->FindContact(hContact))
				clCList->DeleteItem(hItem);
	}
}

void CVkGCCreateForm::ResetListOptions(CCtrlClc* clCList)
{
	if (!clCList)
		return;

	clCList->SetBkBitmap(0, NULL);
	clCList->SetBkColor(GetSysColor(COLOR_WINDOW));
	clCList->SetGreyoutFlags(0);
	clCList->SetLeftMargin(4);
	clCList->SetIndent(10);
	clCList->SetHideEmptyGroups(true);
	clCList->SetHideOfflineRoot(true);
	for (int i = 0; i <= FONTID_MAX; i++)
		clCList->SetTextColor(i, GetSysColor(COLOR_WINDOWTEXT));
}