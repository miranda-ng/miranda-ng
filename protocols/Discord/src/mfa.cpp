/*
Copyright © 2016-22 Miranda NG team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#define WM_ERROR (WM_USER + 1)

/////////////////////////////////////////////////////////////////////////////////////////

class CMfaDialog : public CDiscordDlgBase
{
	bool m_bHasSms, m_bHasTotp;
	int m_mode = 0;
	CMStringA m_szTicket;

	CCtrlBase m_label;
	CCtrlEdit edtCode;
	CCtrlCombo cmbAnother;
	CCtrlButton btnCancel, btnOk;

	UI_MESSAGE_MAP(CMfaDialog, CDiscordDlgBase);
		UI_MESSAGE(WM_ERROR, OnError);
	UI_MESSAGE_MAP_END();


public:
	CMfaDialog(CDiscordProto *ppro, const JSONNode &pRoot) :
		CDiscordDlgBase(ppro, IDD_MFA),
		btnOk(this, IDOK),
		btnCancel(this, IDCANCEL),
		m_label(this, IDC_LABEL),
		edtCode(this, IDC_CODE),
		cmbAnother(this, IDC_ANOTHER)
	{
		btnCancel.OnClick = Callback(this, &CMfaDialog::onClick_Cancel);

		edtCode.OnChange = Callback(this, &CMfaDialog::onChange_Edit);
		cmbAnother.OnChange = Callback(this, &CMfaDialog::onChange_Combo);

		m_bHasSms = pRoot["sms"].as_bool();
		m_bHasTotp = pRoot["totp"].as_bool();
		m_szTicket = pRoot["ticket"].as_mstring();
	}

	bool OnInitDialog() override
	{
		m_proto->pMfaDialog = this;

		if (m_bHasTotp)
			cmbAnother.AddString(TranslateT("Use your authenticator app"), 0);
		if (m_bHasSms)
			cmbAnother.AddString(TranslateT("Use a code sent to your phone"), 1);
		cmbAnother.AddString(TranslateT("Use a backup code"), 2);
		cmbAnother.SetCurSel(0);
		onChange_Combo(0);
		return true;
	}

	bool OnApply() override
	{
		CMStringW wszCode(ptrW(edtCode.GetText()));
		if (wszCode.IsEmpty())
			return false;
		wszCode.Replace(L"-", L"");

		JSONNode root;
		root << CHAR_PARAM("ticket", m_szTicket) << WCHAR_PARAM("code", wszCode);

		const char *pszUrl;
		switch (m_mode) {
		case 0: pszUrl = "/auth/mfa/totp"; break;
		case 1: pszUrl = "/auth/mfa/sms"; break;
		case 2: pszUrl = "/auth/mfa/backup"; break;
		default:
			return false;
		}

		auto *pReq = new AsyncHttpRequest(m_proto, REQUEST_POST, pszUrl, &CDiscordProto::OnSendTotp, &root);
		pReq->AddHeader("Origin", "https://discord.com");
		pReq->AddHeader("Referer", "https://discord.com/login");
		pReq->pUserInfo = this;
		m_proto->Push(pReq);
		return false;
	}

	void OnDestroy() override
	{
		m_proto->pMfaDialog = nullptr;
	}

	void onChange_Combo(CCtrlCombo *)
	{
		edtCode.SetText(L"");

		switch (m_mode = cmbAnother.GetCurData()) {
		case 0:
			m_label.SetText(TranslateT("6-digit authentication code:")); break;
		case 1:
			m_label.SetText(TranslateT("6-digit authentication code:"));
			{
				JSONNode root; root << CHAR_PARAM("ticket", m_szTicket);
				auto *pReq = new AsyncHttpRequest(m_proto, REQUEST_POST, "/auth/mfa/sms/send", 0, &root);
				if (m_proto->m_szCookie)
					pReq->AddHeader("Cookie", m_proto->m_szCookie);
				m_proto->Push(pReq);
			}
			break;
		default:
			m_label.SetText(TranslateT("8-digit backup code:"));
		}
	}

	void onChange_Edit(CCtrlEdit *)
	{
		ptrW wszCode(edtCode.GetText());
		btnOk.Enable(mir_wstrlen(wszCode) > 0);
	}

	void onClick_Cancel(CCtrlButton *)
	{
		m_proto->ConnectionFailed(LOGINERR_WRONGPASSWORD);
	}

	INT_PTR OnError(UINT, WPARAM, LPARAM)
	{
		edtCode.SetText(L"");
		MessageBox(m_hwnd, TranslateT("Invalid two-factor code"), TranslateT("MFA initialization"), MB_OK | MB_ICONERROR);
		return 0;
	}
};

static void CALLBACK LaunchDialog(void *param)
{
	((CMfaDialog *)param)->Show();
}

void CDiscordProto::ShowMfaDialog(const JSONNode &pRoot)
{
	CallFunctionAsync(LaunchDialog, new CMfaDialog(this, pRoot));
}

/////////////////////////////////////////////////////////////////////////////////////////

void CDiscordProto::OnSendTotp(MHttpResponse *pReply, struct AsyncHttpRequest *pReq)
{
	auto *pDlg = (CMfaDialog *)pReq->pUserInfo;

	JsonReply root(pReply);
	if (!root) {
		PostMessage(pDlg->GetHwnd(), WM_ERROR, 0, 0);
		return;
	}

	pDlg->Close();

	auto &data = root.data();
	CMStringA szToken = data["token"].as_mstring();
	m_szAccessToken = szToken.Detach();
	setString(DB_KEY_TOKEN, m_szAccessToken);
	RetrieveMyInfo();
}
