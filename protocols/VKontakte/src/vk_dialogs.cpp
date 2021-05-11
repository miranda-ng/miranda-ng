/*
Copyright (c) 2013-21 Miranda NG team (https://miranda-ng.org)

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
	CVkDlgBase(proto, IDD_CAPTCHAFORM),
	m_instruction(this, IDC_INSTRUCTION),
	m_edtValue(this, IDC_VALUE),
	m_btnOpenInBrowser(this, IDOPENBROWSER),
	m_btnOk(this, IDOK),
	m_param(param)
{
	m_btnOpenInBrowser.OnClick = Callback(this, &CVkCaptchaForm::On_btnOpenInBrowser_Click);
	m_edtValue.OnChange = Callback(this, &CVkCaptchaForm::On_edtValue_Change);
}

bool CVkCaptchaForm::OnInitDialog()
{
	Window_SetIcon_IcoLib(m_hwnd, Skin_GetIconHandle(SKINICON_OTHER_KEYS));

	m_btnOk.Disable();
	m_btnOpenInBrowser.Enable((m_param->bmp != nullptr));
	m_instruction.SetText(TranslateT("Enter the text you see"));
	return true;
}

bool CVkCaptchaForm::OnApply()
{
	m_edtValue.GetTextA(m_param->Result, _countof(m_param->Result));
	return true;
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

void CVkCaptchaForm::On_edtValue_Change(CCtrlEdit*)
{
	m_btnOk.Enable(!IsEmpty(ptrA(m_edtValue.GetTextA())));
}

////////////////////////////////// IDD_WALLPOST ///////////////////////////////////////////

CVkWallPostForm::CVkWallPostForm(CVkProto *proto, WALLPOST_FORM_PARAMS *param) :
	CVkDlgBase(proto, IDD_WALLPOST),
	m_edtMsg(this, IDC_ED_MSG),
	m_edtUrl(this, IDC_ED_URL),
	m_cbOnlyForFriends(this, IDC_ONLY_FRIENDS),
	m_btnShare(this, IDOK),
	m_param(param)
{
	m_edtMsg.OnChange = Callback(this, &CVkWallPostForm::On_edtValue_Change);
	m_edtUrl.OnChange = Callback(this, &CVkWallPostForm::On_edtValue_Change);
}

bool CVkWallPostForm::OnInitDialog()
{
	Window_SetIcon_IcoLib(m_hwnd, g_plugin.getIconHandle(IDI_WALL));

	CMStringW wszTitle(FORMAT, TranslateT("Wall message for %s"), m_param->pwszNick);
	SetCaption(wszTitle);

	m_btnShare.Disable();
	return true;
}

bool CVkWallPostForm::OnApply()
{
	m_param->pwszUrl = m_edtUrl.GetText();
	m_param->pwszMsg = m_edtMsg.GetText();
	m_param->bFriendsOnly = m_cbOnlyForFriends.GetState() != 0;
	return true;
}

void CVkWallPostForm::OnDestroy()
{
	Window_FreeIcon_IcoLib(m_hwnd);
}

void CVkWallPostForm::On_edtValue_Change(CCtrlEdit*)
{
	m_btnShare.Enable(!IsEmpty(ptrW(m_edtMsg.GetText())) || !IsEmpty(ptrW(m_edtUrl.GetText())));
}

////////////////////////////////// IDD_INVITE /////////////////////////////////////////////

CVkInviteChatForm::CVkInviteChatForm(CVkProto *proto) :
	CVkDlgBase(proto, IDD_INVITE),
	m_cbxCombo(this, IDC_CONTACT),
	m_hContact(0)
{
}

bool CVkInviteChatForm::OnInitDialog()
{
	for (auto &hContact : m_proto->AccContacts())
		if (!m_proto->isChatRoom(hContact))
			m_cbxCombo.AddString(Clist_GetContactDisplayName(hContact), hContact);
	return true;
}

bool CVkInviteChatForm::OnApply()
{
	m_hContact = m_cbxCombo.GetCurData();
	return true;
}

////////////////////////////////// IDD_GC_CREATE //////////////////////////////////////////

CVkGCCreateForm::CVkGCCreateForm(CVkProto *proto) :
	CVkDlgBase(proto, IDD_GC_CREATE),
	m_clc(this, IDC_CLIST),
	m_edtTitle(this, IDC_TITLE)
{
	m_clc.OnListRebuilt = Callback(this, &CVkGCCreateForm::FilterList);
}

bool CVkGCCreateForm::OnInitDialog()
{
	SetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE, GetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE)
		| CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
	m_clc.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

	ResetListOptions();
	return true;
}

bool CVkGCCreateForm::OnApply()
{
	CMStringA szUIds;
	for (auto &hContact : m_proto->AccContacts()) {
		if (m_proto->isChatRoom(hContact))
			continue;

		HANDLE hItem = m_clc.FindContact(hContact);
		if (hItem && m_clc.GetCheck(hItem)) {
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
	return true;
}

void CVkGCCreateForm::FilterList(CCtrlClc*)
{
	for (auto &hContact : Contacts()) {
		char *proto = Proto_GetBaseAccountName(hContact);
		if (mir_strcmp(proto, m_proto->m_szModuleName) || m_proto->isChatRoom(hContact) || m_proto->getDword(hContact, "ID") == VK_FEED_USER)
			if (HANDLE hItem = m_clc.FindContact(hContact))
				m_clc.DeleteItem(hItem);
	}
}

void CVkGCCreateForm::ResetListOptions()
{
	m_clc.SetBkColor(GetSysColor(COLOR_WINDOW));
	m_clc.SetHideEmptyGroups(true);
	m_clc.SetHideOfflineRoot(true);
}

////////////////////////////////// IDD_CONTACTDELETE //////////////////////////////////////

CVkContactDeleteForm::CVkContactDeleteForm(CVkProto *proto, CONTACTDELETE_FORM_PARAMS *param) :
	CVkDlgBase(proto, IDD_CONTACTDELETE),
	m_stText(this, IDC_STATIC_TXT),
	m_cbDeleteFromFriendlist(this, IDC_CH_REMOVE_FROM_FRIEND),
	m_cbDeleteDialog(this, IDC_CH_CLEARHISTORY),
	m_param(param)
{
}

bool CVkContactDeleteForm::OnInitDialog()
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
	return true;
}

bool CVkContactDeleteForm::OnApply()
{
	m_param->bDeleteDialog = m_cbDeleteDialog.GetState() != 0;
	m_param->bDeleteFromFriendlist = m_param->bEnableDeleteFromFriendlist && (m_cbDeleteFromFriendlist.GetState() != 0);
	return true;
}
