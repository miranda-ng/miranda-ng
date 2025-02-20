/*
Copyright (c) 2013-25 Miranda NG team (https://miranda-ng.org)

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

////////////////////////////////// IDD_TOKENFORM //////////////////////////////////////////
CVkTokenForm::CVkTokenForm(CVkProto* proto, CMStringA& _szTokenReq) :
	CVkDlgBase(proto, IDD_TOKENFORM),
	m_instruction(this, IDC_INSTRUCTION),
	m_edtValue(this, IDC_TOKENVAL),
	m_btnTokenReq(this, IDC_TOKENREQ),
	m_btnOk(this, IDOK),
	m_TokenReq(_szTokenReq)
{
	m_btnTokenReq.OnClick = Callback(this, &CVkTokenForm::On_btnTokenReq_Click);
	m_edtValue.OnChange = Callback(this, &CVkTokenForm::On_edtValue_Change);
	m_szAccName = proto->m_szModuleName;
}

bool CVkTokenForm::OnInitDialog()
{
	Window_SetIcon_IcoLib(m_hwnd, Skin_GetIconHandle(SKINICON_OTHER_KEYS));

	CMStringW wszCaption(Translate("Logon for "));
	wszCaption += m_szAccName;
		
	SetCaption(wszCaption.c_str());

	m_btnOk.Disable();
	if (!m_TokenReq.IsEmpty()) {
		m_instruction.SetText(TranslateT("To log into the VKontakte protocol, you need to authorize in the browser and copy the received address from its address bar into the field below.\nDo not change anything when copying!\nYou will have to ignore the anti-copy message that you will see in your browser. Unfortunately, Miranda NG cannot authorize you in any other way right now."));
		Utils_OpenUrl(m_TokenReq.c_str());
		return true;
	}
	 
	return false;
}

void CVkTokenForm::OnDestroy()
{
	Window_FreeIcon_IcoLib(m_hwnd);
}

bool CVkTokenForm::OnApply()
{
	m_edtValue.GetTextA(Result, _countof(Result));
	return true;
}

void CVkTokenForm::On_edtValue_Change(CCtrlEdit*)
{
	extern char szVKTokenBeg[];
	ptrA pszUrlSring(m_edtValue.GetTextA());
	
	if (!IsEmpty(pszUrlSring) && strstr(pszUrlSring, szVKTokenBeg))
		m_btnOk.Enable();
	else
		m_btnOk.Disable();
}

void CVkTokenForm::On_btnTokenReq_Click(CCtrlButton*) 
{
	Utils_OpenUrl(m_TokenReq.c_str());
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

////////////////////////////////// IDD_VKUSERFORM //////////////////////////////////////////

CVkUserListForm::CVkUserListForm(CVkProto* proto) :
	CVkDlgBase(proto, IDD_VKUSERFORM),
	m_clc(this, IDC_CONTACTLIST),
	m_edtMessage(this, IDC_MESSAGE),
	m_stListCaption(this, IDC_STATIC_MARKCONTAKTS),
	m_stMessageCaption(this, IDC_STATIC_MESSAGE),
	lContacts(20, NumericKeySortT),
	uClcFilterFlag(0),
	uMaxLengthMessage(0)
{
	m_clc.OnNewContact = m_clc.OnListRebuilt = Callback(this, &CVkUserListForm::FilterList);
}


CVkUserListForm::CVkUserListForm(CVkProto* proto, CMStringW _wszMessage, CMStringW _wszFormCaption, CMStringW _wszListCaption, CMStringW _wszMessageCaption, uint8_t _uClcFilterFlag, UINT _uMaxLengthMessage) :
	CVkDlgBase(proto, IDD_VKUSERFORM),
	m_clc(this, IDC_CONTACTLIST),
	m_edtMessage(this, IDC_MESSAGE), 
	m_stListCaption(this, IDC_STATIC_MARKCONTAKTS),
	m_stMessageCaption(this, IDC_STATIC_MESSAGE),
	wszMessage(_wszMessage),
	wszFormCaption(_wszFormCaption), 
	wszListCaption(_wszListCaption), 
	wszMessageCaption(_wszMessageCaption),
	lContacts(5, PtrKeySortT),
	uClcFilterFlag(_uClcFilterFlag),
	uMaxLengthMessage(_uMaxLengthMessage)
{
	m_clc.OnNewContact = m_clc.OnListRebuilt = Callback(this, &CVkUserListForm::FilterList);
}

bool CVkUserListForm::OnInitDialog()
{
	SetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE, GetWindowLongPtr(m_clc.GetHwnd(), GWL_STYLE)
		| CLS_CHECKBOXES | CLS_HIDEEMPTYGROUPS | CLS_USEGROUPS | CLS_GREYALTERNATE);
	m_clc.SendMsg(CLM_SETEXSTYLE, CLS_EX_DISABLEDRAGDROP | CLS_EX_TRACKSELECT, 0);

	ResetListOptions();

	m_stListCaption.SetText(wszListCaption.c_str());
	m_stMessageCaption.SetText(wszMessageCaption.c_str());
	if (uMaxLengthMessage)
		m_edtMessage.SetMaxLength(uMaxLengthMessage);
	m_edtMessage.SetText(wszMessage.c_str());
	SetCaption(wszFormCaption.c_str());

	return true;
}

bool CVkUserListForm::OnApply()
{
	CMStringA szUIds;
	for (auto& hContact : m_proto->AccContacts()) {
		HANDLE hItem = m_clc.FindContact(hContact);
		if (hItem && m_clc.GetCheck(hItem))
			lContacts.insert((HANDLE)hContact);

	}

	wszMessage = m_edtMessage.GetText();

	return true;
}

void CVkUserListForm::FilterList(CCtrlClc*)
{
	for (auto& hContact : Contacts()) {
		char* proto = Proto_GetBaseAccountName(hContact);
		if (mir_strcmp(proto, m_proto->m_szModuleName) || Contact::IsReadonly(hContact) || m_proto->ReadVKUserID(hContact) == VK_FEED_USER || (m_proto->GetContactType(hContact) & uClcFilterFlag))
			if (HANDLE hItem = m_clc.FindContact(hContact))
				m_clc.DeleteItem(hItem);
	}
}

void CVkUserListForm::ResetListOptions()
{
	m_clc.SetBkColor(GetSysColor(COLOR_WINDOW));
	m_clc.SetHideEmptyGroups(true);
	m_clc.SetHideOfflineRoot(true);
}
