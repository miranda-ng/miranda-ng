#include "stdafx.h"

const wchar_t pluginDescription[] = LPGENW("No more spam! Robots can't go! Only human beings invited!\r\n\r\nThis plugin works pretty simple:\r\nWhile messages from users on your contact list go as there is no any anti-spam software, messages from unknown users are not delivered to you. But also they are not ignored, this plugin replies with a simple question, and if user gives the right answer, plugin adds him to your contact list so that he can contact you.");

class COptMainDlg : public CDlgBase
{
	CCtrlEdit edtDescr;
	CCtrlSpin spinCount, spinTimeout;
	CCtrlCheck chk1, chk2, chk3, chk4, chk6;

public:
	COptMainDlg() :
		CDlgBase(g_plugin, IDD_MAIN),
		edtDescr(this, ID_DESCRIPTION),
		spinCount(this, IDC_SPIN1, 10),
		spinTimeout(this, IDC_SPIN2, 60),
		chk1(this, ID_INFTALKPROT),
		chk2(this, ID_ADDPERMANENT),
		chk3(this, ID_HANDLEAUTHREQ),
		chk4(this, ID_NOTCASESENS),
		chk6(this, ID_HISTORY_LOG)
	{
		CreateLink(spinCount, g_plugin.iMaxQuestCount);
		CreateLink(spinTimeout, g_plugin.iAnswerTimeout);

		CreateLink(chk1, g_plugin.bInfTalkProtection);
		CreateLink(chk2, g_plugin.bAddPermanent);
		CreateLink(chk3, g_plugin.bHandleAuthReq);
		CreateLink(chk4, g_plugin.bAnswNotCaseSens);
		CreateLink(chk6, g_plugin.bHistLog);
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

		CreateLink(edtReply, g_plugin.AuthRepl);
		CreateLink(edtAnswer, g_plugin.Answer);
		CreateLink(edtQuestion, g_plugin.Question);
		CreateLink(edtCongrat, g_plugin.Congratulation);
		CreateLink(edtDivider, g_plugin.AnswSplitString);
	}

	bool OnInitDialog() override
	{
		variables_skin_helpbutton(m_hwnd, IDC_VARS);
		btnHelp.Enable(ServiceExists(MS_VARS_FORMATSTRING));

		edtQuestion.SetText(g_plugin.getQuestion());
		edtCongrat.SetText(g_plugin.getCongrats());
		edtReply.SetText(g_plugin.getReply());
		return true;
	}

	void onHelp(CCtrlButton *)
	{
		variables_showhelp(m_hwnd, WM_COMMAND, VHF_FULLDLG | VHF_SETLASTSUBJECT, nullptr, nullptr);
	}

	void onRestore(CCtrlButton *)
	{
		g_plugin.delSetting(g_plugin.AuthRepl.GetDBSettingName());
		g_plugin.delSetting(g_plugin.Question.GetDBSettingName());
		g_plugin.delSetting(g_plugin.Congratulation.GetDBSettingName());

		edtQuestion.SetText(g_plugin.getQuestion());
		edtAnswer.SetText(g_plugin.Answer.Default());
		edtCongrat.SetText(g_plugin.getCongrats());
		edtReply.SetText(g_plugin.getReply());
		edtDivider.SetText(g_plugin.AnswSplitString.Default());

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
			m_accounts.SetCheckState(idx, !g_plugin.ProtoDisabled(pa->szModuleName));
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
				out << (char *)item.lParam << " ";
		}

		g_plugin.DisabledProtoList = (char *)out.str().c_str();
		return true;
	}

	void list_OnItemChanged(CCtrlListView::TEventInfo *)
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
