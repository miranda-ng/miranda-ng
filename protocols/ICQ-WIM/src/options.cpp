// -----------------------------------------------------------------------------
// ICQ plugin for Miranda NG
// -----------------------------------------------------------------------------
// Copyright © 2018-19 Miranda NG team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------

#include "stdafx.h"

struct CIcqRegistrationDlg : public CProtoDlgBase<CIcqProto>
{
	CMStringA szTrans, szMsisdn;
	int iErrorCode;

	CCtrlEdit edtPhone, edtCode;
	CCtrlButton btnSendSms;

	CIcqRegistrationDlg(CIcqProto *ppro) :
		CProtoDlgBase<CIcqProto>(ppro, IDD_REGISTER),
		edtPhone(this, IDC_PHONE),
		edtCode(this, IDC_CODE),
		btnSendSms(this, IDC_SENDSMS)
	{
		btnSendSms.OnClick = Callback(this, &CIcqRegistrationDlg::onClick_SendSms);
		edtPhone.OnChange = Callback(this, &CIcqRegistrationDlg::onChange_Phone);
	}

	bool OnApply() override
	{
		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "https://www.icq.com/smsreg/loginWithPhoneNumber.php", &CIcqProto::OnLoginViaPhone);
		pReq << CHAR_PARAM("locale", "en") << CHAR_PARAM("msisdn", szMsisdn) << CHAR_PARAM("trans_id", szTrans) << CHAR_PARAM("k", ICQ_APP_ID)
			<< CHAR_PARAM("r", pReq->m_reqId) << CHAR_PARAM("f", "json") << WCHAR_PARAM("sms_code", ptrW(edtCode.GetText())) << INT_PARAM("create_account", 1);
		pReq->pUserInfo = this;
		
		SetCursor(LoadCursor(0, IDC_WAIT));
		m_proto->ExecuteRequest(pReq);
		SetCursor(LoadCursor(0, IDC_ARROW));

		if (iErrorCode != 200)
			return false;

		EndDialog(m_hwnd, 1);
		return true;
	}

	void onChange_Phone(CCtrlEdit*)
	{
		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "https://clientapi.icq.net/fcgi-bin/smsphoneinfo", &CIcqProto::OnCheckPhone);
		pReq << CHAR_PARAM("service", "icq_registration") << CHAR_PARAM("info", "typing_check,score,iso_country_code")
			<< WCHAR_PARAM("phone", ptrW(edtPhone.GetText())) << CHAR_PARAM("id", pReq->m_reqId);
		pReq->pUserInfo = this;
		m_proto->Push(pReq);
	}

	void onClick_SendSms(CCtrlButton*)
	{
		auto *pReq = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "https://www.icq.com/smsreg/requestPhoneValidation.php", &CIcqProto::OnValidateSms);
		pReq << CHAR_PARAM("locale", "en") << CHAR_PARAM("msisdn", szMsisdn) << CHAR_PARAM("r", pReq->m_reqId)
			<< CHAR_PARAM("smsFormatType", "human") << CHAR_PARAM("k", ICQ_APP_ID);
		pReq->pUserInfo = this;
		m_proto->Push(pReq);
	}
};

void CIcqProto::OnCheckPhone(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	if (pReply == nullptr || pReply->resultCode != 200)
		return;

	CIcqRegistrationDlg *pDlg = (CIcqRegistrationDlg*)pReq->pUserInfo;
	pDlg->btnSendSms.Disable();
	pDlg->edtCode.Disable();

	JSONROOT root(pReply->pData);
	CMStringW wszStatus((*root)["status"].as_mstring());
	if (wszStatus != L"OK") {
		pDlg->edtCode.SetText((*root)["printable"].as_mstring());
		return;
	}

	CMStringA szPhoneNumber((*root)["typing_check"]["modified_phone_number"].as_mstring());
	CMStringA szPrefix((*root)["modified_prefix"].as_mstring());

	auto *pNew = new AsyncHttpRequest(CONN_MAIN, REQUEST_GET, "https://www.icq.com/smsreg/normalizePhoneNumber.php", &CIcqProto::OnNormalizePhone);
	pNew << CHAR_PARAM("countryCode", szPrefix) << CHAR_PARAM("phoneNumber", szPhoneNumber.c_str() + szPrefix.GetLength())
		<< CHAR_PARAM("k", ICQ_APP_ID) << CHAR_PARAM("r", pReq->m_reqId);
	pNew->pUserInfo = pDlg;
	Push(pNew);
}

void CIcqProto::OnNormalizePhone(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	CIcqRegistrationDlg *pDlg = (CIcqRegistrationDlg*)pReq->pUserInfo;

	JsonReply root(pReply);
	pDlg->iErrorCode = root.error();
	if (root.error() != 200)
		return;

	const JSONNode &data = root.data();
	pDlg->szMsisdn = data["msisdn"].as_mstring();
	pDlg->btnSendSms.Enable();
}

void CIcqProto::OnValidateSms(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	JsonReply root(pReply);
	if (root.error() != 200)
		return;

	CIcqRegistrationDlg *pDlg = (CIcqRegistrationDlg*)pReq->pUserInfo;
	const JSONNode &data = root.data();
	pDlg->szTrans = data["trans_id"].as_mstring();

	pDlg->edtCode.Enable();
	pDlg->edtCode.SetText(L"");
}

void CIcqProto::OnLoginViaPhone(NETLIBHTTPREQUEST *pReply, AsyncHttpRequest *pReq)
{
	CIcqRegistrationDlg *pDlg = (CIcqRegistrationDlg*)pReq->pUserInfo;

	JsonReply root(pReply);
	pDlg->iErrorCode = root.error();
	if (root.error() != 200)
		return;

	const JSONNode &data = root.data();
	m_szAToken = data["token"]["a"].as_mstring();
	m_szAToken = ptrA(mir_urlDecode(m_szAToken));
	setString(DB_KEY_ATOKEN, m_szAToken);

	m_szSessionKey = data["sessionKey"].as_mstring();
	m_szSessionKey = ptrA(mir_urlDecode(m_szSessionKey));
	setString(DB_KEY_SESSIONKEY, m_szSessionKey);

	m_dwUin = _wtoi(data["loginId"].as_mstring());
	setByte(DB_KEY_PHONEREG, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////

class CIcqOptionsDlg : public CProtoDlgBase<CIcqProto>
{
	CCtrlEdit edtUin, edtPassword;
	CCtrlCheck chkHideChats;
	CCtrlButton btnCreate;
	CMStringW wszOldPass;

public:
	CIcqOptionsDlg(CIcqProto *ppro, int iDlgID, bool bFullDlg) :
		CProtoDlgBase<CIcqProto>(ppro, iDlgID),
		edtUin(this, IDC_UIN),
		btnCreate(this, IDC_REGISTER),
		edtPassword(this, IDC_PASSWORD),
		chkHideChats(this, IDC_HIDECHATS)
	{
		btnCreate.OnClick = Callback(this, &CIcqOptionsDlg::onClick_Register);

		CreateLink(edtUin, ppro->m_dwUin);
		CreateLink(edtPassword, ppro->m_szPassword);
		if (bFullDlg)
			CreateLink(chkHideChats, ppro->m_bHideGroupchats);

		wszOldPass = ppro->m_szPassword;
	}

	bool OnApply() override
	{
		if (wszOldPass != ptrW(edtPassword.GetText())) {
			m_proto->delSetting(DB_KEY_ATOKEN);
			m_proto->delSetting(DB_KEY_SESSIONKEY);
			m_proto->delSetting(DB_KEY_PHONEREG);
		}
		return true;
	}

	void onClick_Register(CCtrlButton*)
	{
		CIcqRegistrationDlg dlg(m_proto);
		dlg.SetParent(m_hwnd);
		if (dlg.DoModal()) // force exit to avoid data corruption
			PostMessage(m_hwndParent, WM_COMMAND, MAKELONG(IDCANCEL, BN_CLICKED), 0);
	}
};

INT_PTR CIcqProto::CreateAccMgrUI(WPARAM, LPARAM hwndParent)
{
	CIcqOptionsDlg *pDlg = new CIcqOptionsDlg(this, IDD_OPTIONS_ACCMGR, false);
	pDlg->SetParent((HWND)hwndParent);
	pDlg->Create();
	return (INT_PTR)pDlg->GetHwnd();
}

int CIcqProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.w = m_tszUserName;
	odp.flags = ODPF_UNICODE;
	odp.szGroup.w = LPGENW("Network");
	odp.position = 1;
	odp.pDialog = new CIcqOptionsDlg(this, IDD_OPTIONS_FULL, true);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
