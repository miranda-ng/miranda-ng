#include "stdafx.h"

class COptionsDlg : public CLibreViewDlgBase
{
	bool bLoading = false;

	Account *m_pAcc = nullptr;
	CCtrlEdit edtEmail, edtPassword, edtApiUrl, edtInterval;
	CCtrlCheck radMmol, radMgdl;
	CCtrlCheck chkAutoUpdate;

	Account* CurrentAccount()
	{
		if (m_pAcc == nullptr)
			m_pAcc = m_proto->m_account;
		return m_pAcc;
	}

	void SaveCurrent()
	{
		Account *pAcc = CurrentAccount();
		if (pAcc == nullptr || bLoading)
			return;

		ptrW wszEmail(edtEmail.GetText());
		ptrW wszPassword(edtPassword.GetText());
		ptrW wszApiUrl(edtApiUrl.GetText());
		if (!mir_wstrlen(wszApiUrl))
			wszApiUrl = mir_wstrdup(_A2W(DEFAULT_API_URL));

		m_proto->setWString(pAcc->hContact, "Email", wszEmail);
		m_proto->setWString(pAcc->hContact, "Password", wszPassword);
		m_proto->setWString(pAcc->hContact, "ApiUrl", wszApiUrl);
		m_proto->setWString(pAcc->hContact, "Nick", wszEmail);

		pAcc->szApiUrl = _T2A(wszApiUrl);
		pAcc->ClearAuth();
	}

public:
	COptionsDlg(CLibreViewProto *ppro) :
		CLibreViewDlgBase(ppro, IDD_OPT),
		edtEmail(this, IDC_EMAIL),
		edtPassword(this, IDC_PASSWORD),
		edtApiUrl(this, IDC_APIURL),
		edtInterval(this, IDC_INTERVAL),
		radMmol(this, IDC_UNIT_MMOL),
		radMgdl(this, IDC_UNIT_MGDL),
		chkAutoUpdate(this, IDC_AUTOUPDATE)
	{
		CreateLink(edtInterval, m_proto->UpdateInterval);
		CreateLink(chkAutoUpdate, m_proto->AutoUpdate);

		edtEmail.OnChange = edtPassword.OnChange = edtApiUrl.OnChange = Callback(this, &COptionsDlg::onChange_Account);
		radMmol.OnChange = radMgdl.OnChange = Callback(this, &COptionsDlg::onChange_Units);
	}

	bool OnInitDialog() override
	{
		LoadAccount();
		if (m_proto->DisplayUnits == 1)
			radMgdl.SetState(true);
		else
			radMmol.SetState(true);
		OnChange();
		return true;
	}

	void OnChange() override
	{
		edtEmail.Enable(true);
		edtPassword.Enable(true);
		edtApiUrl.Enable(true);
	}

	bool OnApply() override
	{
		SaveCurrent();
		m_proto->DisplayUnits = radMgdl.IsChecked() ? 1 : 0;
		if (m_proto->UpdateInterval == 0)
			m_proto->UpdateInterval = 5;
		if (Account *pAcc = CurrentAccount())
			UpdateContactDisplay(pAcc->hContact);
		RestartTimer();
		return true;
	}

	void LoadAccount()
	{
		Account *pAcc = CurrentAccount();

		bLoading = true;
		if (pAcc) {
			edtEmail.SetText(m_proto->getMStringW(pAcc->hContact, "Email"));
			edtPassword.SetText(m_proto->getMStringW(pAcc->hContact, "Password"));

			CMStringW wszApiUrl(m_proto->getMStringW(pAcc->hContact, "ApiUrl"));
			if (wszApiUrl.IsEmpty())
				wszApiUrl = _A2W(DEFAULT_API_URL);
			edtApiUrl.SetText(wszApiUrl);
		}
		else {
			edtEmail.SetText(L"");
			edtPassword.SetText(L"");
			edtApiUrl.SetText(_A2W(DEFAULT_API_URL));
		}
		bLoading = false;
	}

	void onChange_Account(CCtrlEdit *)
	{
		SaveCurrent();
		NotifyChange();
	}

	void onChange_Units(CCtrlCheck *)
	{
		NotifyChange();
	}
};

int CLibreViewProto::OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.position = -790000000;
	odp.szTitle.w = m_tszUserName;
	odp.szGroup.w = LPGENW("Network");
	odp.pDialog = new COptionsDlg(this);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}
