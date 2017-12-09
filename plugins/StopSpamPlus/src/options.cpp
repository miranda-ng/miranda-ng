#include "stdafx.h"

const wchar_t pluginDescription[] = LPGENW("No more spam! Robots can't go! Only human beings invited!\r\n\r\nThis plugin works pretty simple:\r\nWhile messages from users on your contact list go as there is no any anti-spam software, messages from unknown users are not delivered to you. But also they are not ignored, this plugin replies with a simple question, and if user gives the right answer, plugin adds him to your contact list so that he can contact you.");

class COptMainDlg : public CDlgBase
{
	CCtrlEdit edtCount;
	CCtrlCheck chk1, chk2, chk3, chk4, chk5, chk6;

public:
	COptMainDlg() :
		CDlgBase(hInst, IDD_MAIN),
		edtCount(this, ID_MAXQUESTCOUNT),
		chk1(this, ID_INFTALKPROT),
		chk2(this, ID_ADDPERMANENT),
		chk3(this, ID_HANDLEAUTHREQ),
		chk4(this, ID_NOTCASESENS),
		chk5(this, ID_REMOVE_TMP_ALL),
		chk6(this, ID_HISTORY_LOG)
	{}

	virtual void OnInitDialog() override
	{
		SetDlgItemText(m_hwnd, ID_DESCRIPTION, TranslateW(pluginDescription));

		edtCount.SetInt(plSets->MaxQuestCount.Get());
		chk1.SetState(plSets->InfTalkProtection.Get());
		chk2.SetState(plSets->AddPermanent.Get());
		chk3.SetState(plSets->HandleAuthReq.Get());
		chk4.SetState(plSets->AnswNotCaseSens.Get());
		chk5.SetState(plSets->RemTmpAll.Get());
		chk6.SetState(plSets->HistLog.Get());
	}

	virtual void OnApply() override
	{
		plSets->MaxQuestCount = edtCount.GetInt();
		plSets->InfTalkProtection = chk1.GetState();
		plSets->AddPermanent = chk2.GetState();
		plSets->HandleAuthReq = chk3.GetState();
		plSets->AnswNotCaseSens = chk4.GetState();
		plSets->RemTmpAll = chk5.GetState();
		plSets->HistLog = chk6.GetState();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptMessageDlg : public CDlgBase
{
	CCtrlButton btnHelp, btnRestore;
	CCtrlEdit edtQuestion, edtAnswer, edtCongrat, edtReply, edtDivider;

public:
	COptMessageDlg() :
		CDlgBase(hInst, IDD_MESSAGES),
		btnHelp(this, IDC_VARS),
		btnRestore(this, ID_RESTOREDEFAULTS),
		edtQuestion(this, ID_QUESTION),
		edtAnswer(this, ID_ANSWER),
		edtCongrat(this, ID_CONGRATULATION),
		edtReply(this, ID_AUTHREPL),
		edtDivider(this, ID_DIVIDER)
	{
		btnHelp.OnClick = Callback(this, &COptMessageDlg::onHelp);
		btnRestore.OnClick = Callback(this, &COptMessageDlg::onRestore);
	}

	virtual void OnInitDialog() override
	{
		edtQuestion.SetText(plSets->Question.Get().c_str());
		edtAnswer.SetText(plSets->Answer.Get().c_str());
		edtCongrat.SetText(plSets->Congratulation.Get().c_str());
		edtReply.SetText(plSets->AuthRepl.Get().c_str());
		edtDivider.SetText(plSets->AnswSplitString.Get().c_str());
		variables_skin_helpbutton(m_hwnd, IDC_VARS);
		btnHelp.Enable(ServiceExists(MS_VARS_FORMATSTRING));
	}

	virtual void OnApply() override
	{
		plSets->Question = ptrW(edtQuestion.GetText()).get();
		plSets->Answer = ptrW(edtAnswer.GetText()).get();
		plSets->AuthRepl = ptrW(edtCongrat.GetText()).get();
		plSets->Congratulation = ptrW(edtReply.GetText()).get();
		plSets->AnswSplitString = ptrW(edtDivider.GetText()).get();
	}

	void onHelp(CCtrlButton*)
	{
		variables_showhelp(m_hwnd, WM_COMMAND, VHF_FULLDLG | VHF_SETLASTSUBJECT, nullptr, nullptr);
	}

	void onRestore(CCtrlButton*)
	{
		edtQuestion.SetText(plSets->Question.GetDefault().c_str());
		edtAnswer.SetText(plSets->Answer.GetDefault().c_str());
		edtCongrat.SetText(plSets->Congratulation.GetDefault().c_str());
		edtReply.SetText(plSets->AuthRepl.GetDefault().c_str());
		edtDivider.SetText(plSets->AnswSplitString.GetDefault().c_str());
		NotifyChange();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptAccountDlg : public CDlgBase
{
	CCtrlListView m_accounts;

public:
	COptAccountDlg() :
		CDlgBase(hInst, IDD_PROTO),
		m_accounts(this, IDC_PROTO)
	{
		m_accounts.OnItemChanged = Callback(this, &COptAccountDlg::list_OnItemChanged);
	}

	virtual void OnInitDialog() override
	{
		m_accounts.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);
		m_accounts.DeleteAllItems();

		LVCOLUMN lvc = {};
		lvc.mask = LVCF_WIDTH;
		lvc.fmt = LVCFMT_IMAGE | LVCFMT_LEFT;
		lvc.cx = 250;
		m_accounts.InsertColumn(0, &lvc);

		LVITEM item = { 0 };
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.iItem = 1000;

		int n;
		PROTOACCOUNT **pa;
		Proto_EnumAccounts(&n, &pa);

		for (int i = 0; i < n; i++) {
			PROTOACCOUNT *p = pa[i];
			if (!Proto_IsAccountEnabled(p) || p->bIsVirtual)
				continue;

			item.lParam = (LPARAM)p->szModuleName;
			item.pszText = p->tszAccountName;
			int idx = m_accounts.InsertItem(&item);
			m_accounts.SetCheckState(idx, !plSets->ProtoDisabled(p->szModuleName));
		}
	}

	virtual void OnApply() override
	{
		std::ostringstream out;

		LVITEM item;
		item.mask = LVIF_PARAM;

		for (int i = 0; i < m_accounts.GetItemCount(); i++) {
			item.iItem = i;
			if (!m_accounts.GetItem(&item))
				continue;

			if (m_accounts.GetCheckState(i) != 0)
				out << (char*)item.lParam << " ";
		}

		plSets->DisabledProtoList = out.str();
	}

	void list_OnItemChanged(CCtrlListView::TEventInfo*)
	{
		if (m_initialized)
			NotifyChange();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// options initializer

int OnOptInit(WPARAM w, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = pluginName;

	odp.szTab.a = LPGEN("General");
	odp.pDialog = new COptMainDlg();
	Options_AddPage(w, &odp);

	odp.szTab.a = LPGEN("Messages");
	odp.pDialog = new COptMessageDlg();
	Options_AddPage(w, &odp);

	odp.szTab.a = LPGEN("Accounts");
	odp.pDialog = new COptAccountDlg();
	Options_AddPage(w, &odp);
	return 0;
}
