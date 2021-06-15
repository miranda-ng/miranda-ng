#include "stdafx.h"

void getDefaultMessage(HWND hwnd, UINT control, MCONTACT hContact)
{
	DBVARIANT dbv;
	if (!g_plugin.getWString(hContact, "PounceMsg", &dbv)) {
		SetDlgItemText(hwnd, control, dbv.pwszVal);
		db_free(&dbv);
	}
	else if (!g_plugin.getWString("PounceMsg", &dbv)) {
		SetDlgItemText(hwnd, control, dbv.pwszVal);
		db_free(&dbv);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

CBuddyPounceBasicDlg::CBuddyPounceBasicDlg(int dlgId) :
	CDlgBase(g_plugin, dlgId),
	edtMessage(this, IDC_MESSAGE)
{
	edtMessage.OnChange = Callback(this, &CBuddyPounceBasicDlg::onChanged_Message);
}

bool CBuddyPounceBasicDlg::OnInitDialog()
{
	getDefaultMessage(m_hwnd, IDC_MESSAGE, hContact);
	onChanged_Message(0);
	return true;
}

bool CBuddyPounceBasicDlg::OnApply()
{
	ptrW wszMessage(edtMessage.GetText());
	if (mir_wstrlen(wszMessage))
		g_plugin.setWString(hContact, "PounceMsg", wszMessage);
	else
		g_plugin.delSetting(hContact, "PounceMsg");
	return true;
}

void CBuddyPounceBasicDlg::onChanged_Message(CCtrlEdit *)
{
	int length = GetWindowTextLength(edtMessage.GetHwnd());
	CMStringW str(FORMAT, TranslateT("The Message    (%d Characters)"), length);
	SetDlgItemTextW(m_hwnd, GRP_MSG, str);
}

/////////////////////////////////////////////////////////////////////////////////////////
// CBuddyPounceSimpleDlg class implementation

CBuddyPounceSimpleDlg::CBuddyPounceSimpleDlg(MCONTACT _1) :
	CSuper(IDD_POUNCE_SIMPLE),
	btnAdvanced(this, IDC_ADVANCED)
{
	hContact = _1;

	btnAdvanced.OnClick = Callback(this, &CBuddyPounceSimpleDlg::onClick_Advanced);
}

void CBuddyPounceSimpleDlg::onClick_Advanced(CCtrlButton *)
{
	(new CBuddyPounceDlg(hContact))->Create();

	OnApply();
	Close();
}

/////////////////////////////////////////////////////////////////////////////////////////

CBuddyPounceDlg::CBuddyPounceDlg(MCONTACT _1) :
	CSuper(IDD_POUNCE),
	m_contacts(this, IDC_CONTACTS),
	chkSimple(this, IDC_SIMPLE),
	btnDelete(this, IDC_DELETE),
	btnDefault(this, IDC_DEFAULT)
{
	hContact = _1;

	btnDelete.OnClick = Callback(this, &CBuddyPounceDlg::onClick_Delete);
	btnDefault.OnClick = Callback(this, &CBuddyPounceDlg::onClick_Default);

	chkSimple.OnChange = Callback(this, &CBuddyPounceDlg::onChange_Simple);
}

bool CBuddyPounceDlg::OnInitDialog()
{
	CSuper::OnInitDialog();

	// populate contacts
	for (auto &cc: Contacts()) {
		char *szProto = Proto_GetBaseAccountName(cc);
		if (szProto && (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM)) {
			wchar_t name[300];
			mir_snwprintf(name, L"%s (%S)", Clist_GetContactDisplayName(cc), szProto);
			int index = m_contacts.AddString(name, cc);
			if (hContact == cc)
				m_contacts.SetCurSel(index);
		}
	}

	return true;
}

void CBuddyPounceDlg::onChange_Simple(CCtrlCheck *)
{
	if (!m_bInitialized) return;

	(new CBuddyPounceSimpleDlg(hContact))->Create();

	OnApply();
	Close();
}

void CBuddyPounceDlg::onClick_Delete(CCtrlButton *)
{
	edtMessage.SetText(L"");
	onChanged_Message(0);

	g_plugin.delSetting(hContact, "PounceMsg");
	g_plugin.delSetting(hContact, "SendIfMyStatusIsFLAG");
	g_plugin.delSetting(hContact, "SendIfTheirStatusIsFLAG");
	g_plugin.delSetting(hContact, "Reuse");
	g_plugin.delSetting(hContact, "GiveUpDays");
	g_plugin.delSetting(hContact, "GiveUpDate");
	g_plugin.delSetting(hContact, "ConfirmTimeout");
	g_plugin.delSetting(hContact, "FileToSend");
}

void CBuddyPounceDlg::onClick_Default(CCtrlButton *)
{
	getDefaultMessage(m_hwnd, IDC_MESSAGE, hContact);
	onChanged_Message(0);

	g_plugin.setWord(hContact, "SendIfMyStatusIsFLAG", g_plugin.getWord("SendIfMyStatusIsFLAG", 0));
	g_plugin.setWord(hContact, "SendIfTheirStatusIsFLAG", g_plugin.getWord("SendIfTheirStatusIsFLAG", 0));
	g_plugin.setByte(hContact, "Reuse", g_plugin.getByte("Reuse", 0));
	g_plugin.setByte(hContact, "GiveUpDays", g_plugin.getByte("GiveUpDays", 0));
	g_plugin.setDword(hContact, "GiveUpDate", g_plugin.getDword("GiveUpDate", 0));
	g_plugin.setWord(hContact, "ConfirmTimeout", g_plugin.getWord("ConfirmTimeout", 0));
}

/////////////////////////////////////////////////////////////////////////////////////////
// Pounce send confirmation dialog

class CPounceSentDlg : public CDlgBase
{
	bool m_bSuccess;
	MCONTACT m_hContact;

	CCtrlEdit edtMessage;

public:
	CPounceSentDlg(MCONTACT hContact, bool bSuccess) :
		CDlgBase(g_plugin, IDD_CONFIRMSEND),
		m_bSuccess(bSuccess),
		m_hContact(hContact),
		edtMessage(this, IDC_MESSAGE)
	{}

	bool OnInitDialog() override
	{
		ptrW wszMessage(g_plugin.getWStringA(m_hContact, "PounceMsg"));
		if (wszMessage == nullptr)
			return false;

		edtMessage.SetText(wszMessage);

		wchar_t msg[256];
		if (m_bSuccess) {
			mir_snwprintf(msg, TranslateT("Message successfully sent to %s"), Clist_GetContactDisplayName(m_hContact));
			SetDlgItemText(m_hwnd, IDOK, TranslateT("OK"));
			ShowWindow(GetDlgItem(m_hwnd, IDCANCEL), 0);
		}
		else {
			mir_snwprintf(msg, TranslateT("Message failed to send to %s"), Clist_GetContactDisplayName(m_hContact));
			SetDlgItemText(m_hwnd, IDOK, TranslateT("Retry"));
		}
		SetDlgItemText(m_hwnd, LBL_CONTACT, msg);
		SetWindowText(m_hwnd, TranslateT(modFullname));
		return true;
	}

	bool OnApply() override
	{
		if (!m_bSuccess) {
			ptrW wszMessage(edtMessage.GetText());
			if (wszMessage)
				SendPounce(wszMessage, m_hContact);
		}
		return true;
	}
};

void CreateMessageAcknowlegedWindow(MCONTACT hContact, int SentSuccess)
{
	(new CPounceSentDlg(hContact, SentSuccess != 0))->Create();
}
