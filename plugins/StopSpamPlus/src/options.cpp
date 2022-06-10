#include "stdafx.h"

const wchar_t pluginDescription[] = LPGENW("No more spam! Robots can't go! Only human beings invited!\r\n\r\nThis plugin works pretty simple:\r\nWhile messages from users on your contact list go as there is no any anti-spam software, messages from unknown users are not delivered to you. But also they are not ignored, this plugin replies with a simple question, and if user gives the right answer, plugin adds him to your contact list so that he can contact you.");

class COptMainDlg : public CDlgBase
{
	CCtrlEdit edtCount, edtDescr;
	CCtrlCheck chk1, chk2, chk3, chk4, chk5, chk6;

public:
	COptMainDlg() :
		CDlgBase(g_plugin, IDD_MAIN),
		edtCount(this, ID_MAXQUESTCOUNT),
		edtDescr(this, ID_DESCRIPTION),
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

	bool OnInitDialog() override
	{
		edtDescr.SetText(TranslateW(pluginDescription));
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptMessageDlg : public CDlgBase
{
	CCtrlButton btnHelp, btnRestore;
	CCtrlEdit edtQuestion, edtAnswer, edtCongrat, edtReply, edtDivider;

public:
	COptMessageDlg() :
		CDlgBase(g_plugin, IDD_MESSAGES),
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

	bool OnInitDialog() override
	{
		variables_skin_helpbutton(m_hwnd, IDC_VARS);
		btnHelp.Enable(ServiceExists(MS_VARS_FORMATSTRING));

		edtQuestion.SetText(g_sets.getQuestion());
		edtCongrat.SetText(g_sets.getCongrats());
		edtReply.SetText(g_sets.getReply());
		return true;
	}

	void onHelp(CCtrlButton*)
	{
		variables_showhelp(m_hwnd, WM_COMMAND, VHF_FULLDLG | VHF_SETLASTSUBJECT, nullptr, nullptr);
	}

	void onRestore(CCtrlButton*)
	{
		g_plugin.delSetting(g_sets.AuthRepl.GetDBSettingName());
		g_plugin.delSetting(g_sets.Question.GetDBSettingName());
		g_plugin.delSetting(g_sets.Congratulation.GetDBSettingName());

		edtQuestion.SetText(g_sets.getQuestion());
		edtAnswer.SetText(g_sets.Answer.Default());
		edtCongrat.SetText(g_sets.getCongrats());
		edtReply.SetText(g_sets.getReply());
		edtDivider.SetText(g_sets.AnswSplitString.Default());
		
		NotifyChange();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////

class COptAccountDlg : public CDlgBase
{
	CCtrlListView m_accounts;

public:
	COptAccountDlg() :
		CDlgBase(g_plugin, IDD_PROTO),
		m_accounts(this, IDC_PROTO)
	{
		m_accounts.OnItemChanged = Callback(this, &COptAccountDlg::list_OnItemChanged);
	}

	bool OnInitDialog() override
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

		for (auto &pa : Accounts()) {
			if (!pa->IsEnabled() || pa->bIsVirtual)
				continue;

			item.lParam = (LPARAM)pa->szModuleName;
			item.pszText = pa->tszAccountName;
			int idx = m_accounts.InsertItem(&item);
			m_accounts.SetCheckState(idx, !g_sets.ProtoDisabled(pa->szModuleName));
		}
		return true;
	}

	bool OnApply() override
	{
		std::ostringstream out;

		LVITEM item;
		item.mask = LVIF_PARAM;

		for (int i = 0; i < m_accounts.GetItemCount(); i++) {
			item.iItem = i;
			if (!m_accounts.GetItem(&item))
				continue;

			if (m_accounts.GetCheckState(i) == 0)
				out << (char*)item.lParam << " ";
		}

		g_sets.DisabledProtoList = (char*)out.str().c_str();
		return true;
	}

	void list_OnItemChanged(CCtrlListView::TEventInfo*)
	{
		if (m_bInitialized)
			NotifyChange();
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// options initializer

int OnOptInit(WPARAM w, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = MODULENAME;

	odp.szTab.a = LPGEN("General");
	odp.pDialog = new COptMainDlg();
	g_plugin.addOptions(w, &odp);

	odp.szTab.a = LPGEN("Messages");
	odp.pDialog = new COptMessageDlg();
	g_plugin.addOptions(w, &odp);

	odp.szTab.a = LPGEN("Accounts");
	odp.pDialog = new COptAccountDlg();
	g_plugin.addOptions(w, &odp);
	return 0;
}
