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
	bool m_bHasSms, m_bHasTotp, m_bUseTotp = true;
	CMStringA m_szTicket;

	CCtrlBase m_label;
	CCtrlEdit edtCode;
	CCtrlButton btnCancel, btnAnother;

public:
	CMfaDialog(CDiscordProto *ppro, const JSONNode &pRoot) :
		CDiscordDlgBase(ppro, IDD_MFA),
		m_label(this, IDC_LABEL),
		edtCode(this, IDC_CODE),
		btnCancel(this, IDCANCEL),
		btnAnother(this, IDC_ANOTHER)
	{
		m_bHasSms = pRoot["sms"].as_bool();
		m_bHasTotp = pRoot["totp"].as_bool();
		m_szTicket = pRoot["ticket"].as_mstring();
	}

	bool OnInitDialog() override
	{
		if (m_bUseTotp)
			m_label.SetText(TranslateT("Enter Discord verification code:"));
		else
			m_label.SetText(TranslateT("Enter SMS code"));

		// if (!m_bHasSms)
		btnAnother.Disable();
		return true;
	}

	bool OnApply() override
	{
		ptrW wszCode(edtCode.GetText());

		if (m_bUseTotp) {
			JSONNode root;
			root << WCHAR_PARAM("code", wszCode) << CHAR_PARAM("ticket", m_szTicket);

			auto *pReq = new AsyncHttpRequest(m_proto, REQUEST_POST, "/auth/mfa/totp", &CDiscordProto::OnSendTotp, &root);
			pReq->pUserInfo = this;
			m_proto->Push(pReq);
		}
		return false;
	}

	void onClick_Cancel()
	{
		m_proto->ConnectionFailed(LOGINERR_WRONGPASSWORD);
	}

	void WrongCode()
	{
		edtCode.SetText(L"");
		Beep(470, 200);
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
	setString("AccessToken", m_szAccessToken);
	RetrieveMyInfo();
}
