#include "stdafx.h"

class COptionsDlg : public CLibreViewDlgBase
{
	bool bLoading = false;

	CCtrlEdit edtEmail, edtPassword, edtApiUrl, edtInterval;
	CCtrlCheck radMmol, radMgdl;
	CCtrlCheck chkWriteHistory;

	void SaveCurrent()
	{
		if (bLoading)
			return;

		ptrW wszEmail(edtEmail.GetText());
		ptrW wszPassword(edtPassword.GetText());
		ptrW wszApiUrl(edtApiUrl.GetText());
		if (!mir_wstrlen(wszApiUrl))
			wszApiUrl = mir_wstrdup(_A2W(DEFAULT_API_URL));

		if (m_proto->m_hContact == 0 && mir_wstrlen(wszEmail) && mir_wstrlen(wszPassword))
			m_proto->EnsureAccountContact();

		m_proto->setWString((MCONTACT)0, "Email", wszEmail);
		m_proto->setWString((MCONTACT)0, "Password", wszPassword);
		m_proto->setWString((MCONTACT)0, "ApiUrl", wszApiUrl);
		if (mir_wstrlen(wszEmail))
			m_proto->setWString((MCONTACT)0, "Nick", wszEmail);

		m_proto->szApiUrl = _T2A(wszApiUrl);
		m_proto->ClearAuth();
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
		chkWriteHistory(this, IDC_WRITE_HISTORY)
	{
		CreateLink(edtInterval, m_proto->UpdateInterval);
		CreateLink(chkWriteHistory, m_proto->WriteHistory);

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
		if (m_proto->m_hContact)
			UpdateContactDisplay(m_proto->m_hContact);
		RestartTimer();
		return true;
	}

	void LoadAccount()
	{
		bLoading = true;
		edtEmail.SetText(m_proto->getMStringW((MCONTACT)0, "Email"));
		edtPassword.SetText(m_proto->getMStringW((MCONTACT)0, "Password"));

		CMStringW wszApiUrl(m_proto->getMStringW((MCONTACT)0, "ApiUrl"));
		if (wszApiUrl.IsEmpty())
			wszApiUrl = _A2W(DEFAULT_API_URL);
		edtApiUrl.SetText(wszApiUrl);
		
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
