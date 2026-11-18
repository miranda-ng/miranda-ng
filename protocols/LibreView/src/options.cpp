#include "stdafx.h"

class COptionsDlg : public CLibreViewDlgBase
{
	CCtrlEdit edtEmail, edtPassword, edtApiUrl, edtInterval, edtOffset;
	CCtrlCheck radMmol, radMgdl;
	CCtrlCheck chkWriteHistory;

public:
	COptionsDlg(CLibreViewProto *ppro) :
		CLibreViewDlgBase(ppro, IDD_OPT),
		edtEmail(this, IDC_EMAIL),
		edtPassword(this, IDC_PASSWORD),
		edtApiUrl(this, IDC_APIURL),
		edtInterval(this, IDC_INTERVAL),
		edtOffset(this, IDC_OFFSET),
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
		
		// Load offset value (convert from uint32_t to int for display)
		int offsetValue = (int)m_proto->Offset;
		CMStringW wszOffset(FORMAT, L"%d", offsetValue);
		edtOffset.SetText(wszOffset);
		
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
		
		CMStringA szApiUrl = NormalizeBaseUrl(mir_wstrlen(wszApiUrl) ? _T2A(wszApiUrl) : "");
		if (!mir_wstrlen(wszApiUrl) || szApiUrl == DEFAULT_API_URL) {
			m_proto->delSetting("ApiUrl");
			m_proto->szApiUrl = DEFAULT_API_URL;
		}
		else {
			m_proto->setString("ApiUrl", szApiUrl);
			m_proto->szApiUrl = szApiUrl;
		}

		m_proto->ClearAuth();

		m_proto->DisplayUnits = radMgdl.IsChecked() ? 1 : 0;
		
		// Parse and save offset value
		ptrW wszOffset(edtOffset.GetText());
		if (!mir_wstrlen(wszOffset)) {
			m_proto->Offset = 0;
		}
		else {
			m_proto->Offset = _wtoi(wszOffset);
		}
		
		UpdateContactDisplay(m_proto->m_hContact);

		// Refresh graph if open (event 3: DisplayUnits or Offset setting changes)
		RefreshGraphWindow(m_proto->m_hContact);

		RestartTimer();
		return true;
	}

	void LoadAccount()
	{
		edtEmail.SetText(m_proto->getMStringW("Email"));
		edtPassword.SetText(m_proto->getMStringW("Password"));

		CMStringA szApiUrl(m_proto->getMStringA("ApiUrl"));
		if (szApiUrl.IsEmpty())
			szApiUrl = DEFAULT_API_URL;
		edtApiUrl.SetTextA(szApiUrl);
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
