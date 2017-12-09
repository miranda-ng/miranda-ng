#include "stdafx.h"

const wchar_t pluginDescription[] = LPGENW("No more spam! Robots can't go! Only human beings invited!\r\n\r\nThis plugin works pretty simple:\r\nWhile messages from users on your contact list go as there is no any anti-spam software, messages from unknown users are not delivered to you. But also they are not ignored, this plugin replies with a simple question, and if user gives the right answer, plugin adds him to your contact list so that he can contact you.");

class COptMainDlg : public CPluginDlgBase
{
	CCtrlEdit edtCount;
	CCtrlCheck chk1, chk2, chk3, chk4, chk5, chk6;

public:
	COptMainDlg() :
		CPluginDlgBase(hInst, IDD_MAIN, pluginName),
		edtCount(this, ID_MAXQUESTCOUNT),
		chk1(this, ID_INFTALKPROT),
		chk2(this, ID_ADDPERMANENT),
		chk3(this, ID_HANDLEAUTHREQ),
		chk4(this, ID_NOTCASESENS),
		chk5(this, ID_REMOVE_TMP_ALL),
		chk6(this, ID_HISTORY_LOG)
	{
		CreateLink(edtCount, g_sets.MaxQuestCount);

		CreateLink(chk1, g_sets.InfTalkProtection);
		CreateLink(chk2, g_sets.AddPermanent);
		CreateLink(chk3, g_sets.HandleAuthReq);
		CreateLink(chk4, g_sets.AnswNotCaseSens);
		CreateLink(chk5, g_sets.RemTmpAll);
		CreateLink(chk6, g_sets.HistLog);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptMessageDlg : public CPluginDlgBase
{
	CCtrlButton btnHelp, btnRestore;
	CCtrlEdit edtQuestion, edtAnswer, edtCongrat, edtReply, edtDivider;

public:
	COptMessageDlg() :
		CPluginDlgBase(hInst, IDD_MESSAGES, pluginName),
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

		CreateLink(edtReply, g_sets.AuthRepl);
		CreateLink(edtAnswer, g_sets.Answer);
		CreateLink(edtQuestion, g_sets.Question);
		CreateLink(edtCongrat, g_sets.Congratulation);
		CreateLink(edtDivider, g_sets.AnswSplitString);
	}

	virtual void OnInitDialog() override
	{
		variables_skin_helpbutton(m_hwnd, IDC_VARS);
		btnHelp.Enable(ServiceExists(MS_VARS_FORMATSTRING));
	}

	void onHelp(CCtrlButton*)
	{
		variables_showhelp(m_hwnd, WM_COMMAND, VHF_FULLDLG | VHF_SETLASTSUBJECT, nullptr, nullptr);
	}

	void onRestore(CCtrlButton*)
	{
		edtQuestion.SetText(g_sets.Question.Default());
		edtAnswer.SetText(g_sets.Answer.Default());
		edtCongrat.SetText(g_sets.Congratulation.Default());
		edtReply.SetText(g_sets.AuthRepl.Default());
		edtDivider.SetText(g_sets.AnswSplitString.Default());
		
		NotifyChange();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptAccountDlg : public CPluginDlgBase
{
	CCtrlListView m_accounts;

public:
	COptAccountDlg() :
		CPluginDlgBase(hInst, IDD_PROTO, pluginName),
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
			m_accounts.SetCheckState(idx, !g_sets.ProtoDisabled(p->szModuleName));
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

		g_sets.DisabledProtoList = (char*)out.str().c_str();
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
