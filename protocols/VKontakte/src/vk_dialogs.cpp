/*
Copyright (c) 2013-17 Miranda NG project (https://miranda-ng.org)

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

CVkCaptchaForm::CVkCaptchaForm(CVkProto *proto, CAPTCHA_FORM_PARAMS *param) :
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
	Window_SetIcon_IcoLib(m_hwnd, GetIconHandle(IDI_KEYS));

	m_btnOk.Disable();
	m_btnOpenInBrowser.Enable((m_param->bmp != nullptr));
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
		return 0;

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
	Window_FreeIcon_IcoLib(m_hwnd);
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

CVkWallPostForm::CVkWallPostForm(CVkProto *proto, WALLPOST_FORM_PARAMS *param) :
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
	Window_SetIcon_IcoLib(m_hwnd, GetIconHandle(IDI_WALL));

	CMStringW wszTitle(FORMAT, TranslateT("Wall message for %s"), m_param->pwszNick);
	SetCaption(wszTitle);

	m_btnShare.Disable();
}

void CVkWallPostForm::OnDestroy()
{
	Window_FreeIcon_IcoLib(m_hwnd);
}

void CVkWallPostForm::On_btnShare_Click(CCtrlButton*)
{
	m_param->pwszUrl = m_edtUrl.GetText();
	m_param->pwszMsg = m_edtMsg.GetText();
	m_param->bFriendsOnly = m_cbOnlyForFriends.GetState() != 0;

	EndDialog(m_hwnd, 1);
}

void CVkWallPostForm::On_edtValue_Change(CCtrlEdit*)
{
	m_btnShare.Enable(!IsEmpty(ptrW(m_edtMsg.GetText())) || !IsEmpty(ptrW(m_edtUrl.GetText())));
}

////////////////////////////////// IDD_INVITE /////////////////////////////////////////////

CVkInviteChatForm::CVkInviteChatForm(CVkProto *proto) :
	CVkDlgBase(proto, IDD_INVITE, false),
	m_btnOk(this, IDOK),
	m_cbxCombo(this, IDC_CONTACT),
	m_hContact(0)
{
	m_btnOk.OnClick = Callback(this, &CVkInviteChatForm::btnOk_OnOk);
}

void CVkInviteChatForm::OnInitDialog()
{
	for (MCONTACT hContact = db_find_first(m_proto->m_szModuleName); hContact; hContact = db_find_next(hContact, m_proto->m_szModuleName)) {
		if (!m_proto->isChatRoom(hContact)) {
			wchar_t *pwszNick = pcli->pfnGetContactDisplayName(hContact, 0);
			m_cbxCombo.AddString(pwszNick, hContact);
		}
	}
}

void CVkInviteChatForm::btnOk_OnOk(CCtrlButton*)
{
	m_hContact = m_cbxCombo.GetItemData(m_cbxCombo.GetCurSel());
	EndDialog(m_hwnd, 1);
}

////////////////////////////////// IDD_GC_CREATE //////////////////////////////////////////

CVkGCCreateForm::CVkGCCreateForm(CVkProto *proto) :
	CVkDlgBase(proto, IDD_GC_CREATE, false),
	m_btnOk(this, IDOK),
	m_clCList(this, IDC_CLIST),
	m_edtTitle(this, IDC_TITLE)
{
	m_btnOk.OnClick = Callback(this, &CVkGCCreateForm::btnOk_OnOk);
	m_clCList.OnListRebuilt = Callback(this, &CVkGCCreateForm::FilterList);
}

void CVkGCCreateForm::OnInitDialog()
{
	SetWindowLongPtr(m_clCList.GetHwnd(), GWL_STYLE, GetWindowLongPtr(m_clCList.GetHwnd(), GWL_STYLE)
		| CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
	m_clCList.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

	ResetListOptions(&m_clCList);
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
		m_proto->CreateNewChat(szUIds, ptrW(m_edtTitle.GetText()));

	EndDialog(m_hwnd, bRes);
}

void CVkGCCreateForm::FilterList(CCtrlClc*)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *proto = GetContactProto(hContact);
		if (mir_strcmp(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact) || m_proto->getDword(hContact, "ID") == VK_FEED_USER)
			if (HANDLE hItem = m_clCList.FindContact(hContact))
				m_clCList.DeleteItem(hItem);
	}
}

void CVkGCCreateForm::ResetListOptions(CCtrlClc *clCList)
{
	if (!clCList)
		return;

	clCList->SetBkBitmap(0, nullptr);
	clCList->SetBkColor(GetSysColor(COLOR_WINDOW));
	clCList->SetGreyoutFlags(0);
	clCList->SetLeftMargin(4);
	clCList->SetIndent(10);
	clCList->SetHideEmptyGroups(true);
	clCList->SetHideOfflineRoot(true);
	for (int i = 0; i <= FONTID_MAX; i++)
		clCList->SetTextColor(i, GetSysColor(COLOR_WINDOWTEXT));
}

////////////////////////////////// IDD_CONTACTDELETE //////////////////////////////////////

CVkContactDeleteForm::CVkContactDeleteForm(CVkProto *proto, CONTACTDELETE_FORM_PARAMS *param) :
	CVkDlgBase(proto, IDD_CONTACTDELETE, false),
	m_btnOk(this, IDOK),
	m_stText(this, IDC_STATIC_TXT),
	m_cbDeleteFromFriendlist(this, IDC_CH_REMOVE_FROM_FRIEND),
	m_cbDeleteDialog(this, IDC_CH_CLEARHISTORY),
	m_param(param)
{
	m_btnOk.OnClick = Callback(this, &CVkContactDeleteForm::btnOk_OnOk);
}

void CVkContactDeleteForm::OnInitDialog()
{
	CMStringW szText(FORMAT, TranslateT("You delete %s from the contact list.\nWhat needs to be done additionally?"),
		m_param->pwszNick);
	m_stText.SetText(szText.c_str());

	szText.Format(TranslateT("Remove %s from your friend list"), m_param->pwszNick);
	m_cbDeleteFromFriendlist.SetText(szText.c_str());
	m_cbDeleteFromFriendlist.SetState(m_param->bDeleteFromFriendlist && m_param->bEnableDeleteFromFriendlist);
	m_cbDeleteFromFriendlist.Enable(m_param->bEnableDeleteFromFriendlist);


	szText.Format(TranslateT("Clear server history with %s"), m_param->pwszNick);
	m_cbDeleteDialog.SetText(szText.c_str());
	m_cbDeleteDialog.SetState(m_param->bDeleteDialog);

	szText.Format(TranslateT("Deleting %s from contact list"), m_param->pwszNick);
	SetCaption(szText.c_str());
}

void CVkContactDeleteForm::btnOk_OnOk(CCtrlButton*)
{

	m_param->bDeleteDialog = m_cbDeleteDialog.GetState() != 0;
	m_param->bDeleteFromFriendlist = m_param->bEnableDeleteFromFriendlist
		&& (m_cbDeleteFromFriendlist.GetState() != 0);

	EndDialog(m_hwnd, 1);
}