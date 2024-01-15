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

/////////////////////////////////////////////////////////////////////////////////////////

class CMfaDialog : public CDiscordDlgBase
{
	bool m_bHasSms, m_bHasTotp;
	int m_mode = 0;
	CMStringA m_szTicket;

	CCtrlBase m_label;
	CCtrlEdit edtCode;
	CCtrlCombo cmbAnother;
	CCtrlButton btnCancel;

public:
	CMfaDialog(CDiscordProto *ppro, const JSONNode &pRoot) :
		CDiscordDlgBase(ppro, IDD_MFA),
		m_label(this, IDC_LABEL),
		edtCode(this, IDC_CODE),
		btnCancel(this, IDCANCEL),
		cmbAnother(this, IDC_ANOTHER)
	{
		btnCancel.OnClick = Callback(this, &CMfaDialog::onClick_Cancel);

		cmbAnother.OnChange = Callback(this, &CMfaDialog::onChange_Combo);

		m_bHasSms = pRoot["sms"].as_bool();
		m_bHasTotp = pRoot["totp"].as_bool();
		m_szTicket = pRoot["ticket"].as_mstring();
	}

	bool OnInitDialog() override
	{
		if (m_bHasTotp)
			cmbAnother.AddString(TranslateT("Use authentication app"), 0);
		if (m_bHasSms)
			cmbAnother.AddString(TranslateT("Use a code sent to your phone"), 1);
		cmbAnother.AddString(TranslateT("Use a backup code"), 2);
		cmbAnother.SetCurSel(0);
		return true;
	}

	bool OnApply() override
	{
		JSONNode root;
		root << CHAR_PARAM("ticket", m_szTicket);

		const char *wszUrl;
		ptrW wszCode(edtCode.GetText());
		if (mir_wstrlen(wszCode)) {
			wszUrl = (m_mode == 1) ? "/auth/mfa/sms" : "/auth/mfa/totp";
			root << WCHAR_PARAM("code", wszCode);
		}
		else wszUrl = "/auth/mfa/sms/send";

		auto *pReq = new AsyncHttpRequest(m_proto, REQUEST_POST, wszUrl, &CDiscordProto::OnSendTotp, &root);
		pReq->pUserInfo = this;
		m_proto->Push(pReq);
		return false;
	}

	void onChange_Combo(CCtrlCombo *)
	{
		edtCode.SetText(L"");

		switch (m_mode = cmbAnother.GetCurData()) {
		case 0:
			m_label.SetText(TranslateT("Enter Discord verification code:")); break;
		case 1:
			m_label.SetText(TranslateT("Enter SMS code you received:")); 
			OnApply();
			break;
		default:
			m_label.SetText(TranslateT("Enter one of your backup codes"));
		}
	}

	void onClick_Cancel(CCtrlButton *)
	{
		m_proto->ConnectionFailed(LOGINERR_WRONGPASSWORD);
	}

	void WrongCode()
	{
		edtCode.SetText(L"");
		MessageBeep(MB_ICONERROR);
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
		pDlg->WrongCode();
		return;
	}

	pDlg->Close();

	auto &data = root.data();
	CMStringA szToken = data["token"].as_mstring();
	m_szAccessToken = szToken.Detach();
	setString(DB_KEY_TOKEN, m_szAccessToken);
	RetrieveMyInfo();
}
