#include "stdafx.h"

class COptionsDlg : public CLibreViewDlgBase
{
	CCtrlEdit edtEmail, edtPassword, edtApiUrl, edtInterval;
	CCtrlCheck radMmol, radMgdl;
	CCtrlCheck chkWriteHistory;

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
		ptrW wszEmail(edtEmail.GetText());
		ptrW wszPassword(edtPassword.GetText());
		ptrW wszApiUrl(edtApiUrl.GetText());
		
		// Reset to defaults if fields are cleared
		if (!mir_wstrlen(wszApiUrl))
			wszApiUrl = mir_wstrdup(_A2W(DEFAULT_API_URL));

		ptrW wszInterval(edtInterval.GetText());
		if (!mir_wstrlen(wszInterval)) {
			m_proto->delSetting("UpdateInterval");
		}

		if (m_proto->m_hContact == 0 && mir_wstrlen(wszEmail) && mir_wstrlen(wszPassword))
			m_proto->EnsureAccountContact();
		if (!mir_wstrlen(wszEmail)) {
			m_proto->delSetting("Email");
		}
		else {
			m_proto->setWString("Email", wszEmail);
		}
		
		if (!mir_wstrlen(wszPassword))
			m_proto->delSetting("Password");
		else
			m_proto->setWString("Password", wszPassword);
		
		if (!mir_wstrlen(wszApiUrl)) {
			m_proto->delSetting("ApiUrl");
			m_proto->szApiUrl = DEFAULT_API_URL;
		}
		else {
			m_proto->setWString("ApiUrl", wszApiUrl);
			m_proto->szApiUrl = _T2A(wszApiUrl);
		}

		m_proto->ClearAuth();

		m_proto->DisplayUnits = radMgdl.IsChecked() ? 1 : 0;
		UpdateContactDisplay(m_proto->m_hContact);

		RestartTimer();
		return true;
	}

	void LoadAccount()
	{
		edtEmail.SetText(m_proto->getMStringW("Email"));
		edtPassword.SetText(m_proto->getMStringW("Password"));

		CMStringW wszApiUrl(m_proto->getMStringW("ApiUrl"));
		if (wszApiUrl.IsEmpty())
			wszApiUrl = _A2W(DEFAULT_API_URL);
		edtApiUrl.SetText(wszApiUrl);
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
