/* Copyright (C) Miklashevsky Roman, sss, elzor
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

wchar_t const *pluginDescription = LPGENW("No more spam! Robots can't go! Only human beings invited!\r\n\r\nThis plugin works pretty simple:\r\nWhile messages from users on your contact list go as there is no any anti-spam software, messages from unknown users are not delivered to you. But also they are not ignored, this plugin replies with a simple question, and if user gives the right answer, plugin adds him to your contact list so that he can contact you.");

const wchar_t *defQuestion = LPGENW("Spammers made me to install small anti-spam system you are now speaking with. Please reply \"nospam\" without quotes and spaces if you want to contact me.");
const wchar_t *defCongrats = LPGENW("Congratulations! You just passed human/robot test. Now you can write me a message.");
const wchar_t *defAuthReply = LPGENW("StopSpam: send a message and reply to an anti-spam bot question.");

class COptMainDlg : public CDlgBase
{
	CCtrlCheck chk_INFTALKPROT, chk_ADDPERMANENT, chk_HANDLEAUTHREQ, chk_HIDECONTACTS, chk_IGNORESPAMMERS, chk_LOGSPAMTOFILE;
	CCtrlData ctrl_DESCRIPTION;
	CCtrlSpin edit_MAXQUESTCOUNT;

public:
	COptMainDlg() : CDlgBase(g_plugin, IDD_MAIN),
		chk_INFTALKPROT(this, ID_INFTALKPROT), chk_ADDPERMANENT(this, ID_ADDPERMANENT), chk_HANDLEAUTHREQ(this, ID_HANDLEAUTHREQ),
		chk_HIDECONTACTS(this, ID_HIDECONTACTS), chk_IGNORESPAMMERS(this, ID_IGNORESPAMMERS), chk_LOGSPAMTOFILE(this, ID_LOGSPAMTOFILE),
		ctrl_DESCRIPTION(this, ID_DESCRIPTION), edit_MAXQUESTCOUNT(this, ID_MAXQUESTCOUNT)
	{}

	bool OnInitDialog() override
	{
		ctrl_DESCRIPTION.SetText(TranslateW(pluginDescription));
		edit_MAXQUESTCOUNT.SetInt(gbMaxQuestCount);
		chk_INFTALKPROT.SetState(gbInfTalkProtection);
		chk_ADDPERMANENT.SetState(gbAddPermanent);
		chk_HANDLEAUTHREQ.SetState(gbHandleAuthReq);
		chk_HIDECONTACTS.SetState(gbHideContacts);
		chk_IGNORESPAMMERS.SetState(gbIgnoreContacts);
		chk_LOGSPAMTOFILE.SetState(gbLogToFile);
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setDword("maxQuestCount", gbMaxQuestCount = edit_MAXQUESTCOUNT.GetInt());
		g_plugin.setByte("infTalkProtection", gbInfTalkProtection = chk_INFTALKPROT.GetState());
		g_plugin.setByte("addPermanent", gbAddPermanent = chk_ADDPERMANENT.GetState());
		g_plugin.setByte("handleAuthReq", gbHandleAuthReq = chk_HANDLEAUTHREQ.GetState());
		g_plugin.setByte("HideContacts", gbHideContacts = chk_HIDECONTACTS.GetState());
		g_plugin.setByte("IgnoreContacts", gbIgnoreContacts = chk_IGNORESPAMMERS.GetState());
		g_plugin.setByte("LogSpamToFile", gbLogToFile = chk_LOGSPAMTOFILE.GetState());
		return true;
	}
};

class COptMessagesDlg : public CDlgBase
{
	CCtrlEdit edit_QUESTION, edit_ANSWER, edit_CONGRATULATION, edit_AUTHREPL;
	CCtrlButton btn_RESTOREDEFAULTS, btn_VARS;

public:
	COptMessagesDlg() : CDlgBase(g_plugin, IDD_MESSAGES),
		edit_QUESTION(this, ID_QUESTION), edit_ANSWER(this, ID_ANSWER), edit_CONGRATULATION(this, ID_CONGRATULATION), edit_AUTHREPL(this, ID_AUTHREPL),
		btn_RESTOREDEFAULTS(this, ID_RESTOREDEFAULTS), btn_VARS(this, IDC_VARS)
	{
		btn_RESTOREDEFAULTS.OnClick = Callback(this, &COptMessagesDlg::onClick_RESTOREDEFAULTS);
		btn_VARS.OnClick = Callback(this, &COptMessagesDlg::onClick_VARS);

	}

	bool OnInitDialog() override
	{
		edit_QUESTION.SetText(gbQuestion.c_str());
		edit_ANSWER.SetText(gbAnswer.c_str());
		edit_CONGRATULATION.SetText(gbCongratulation.c_str());
		edit_AUTHREPL.SetText(gbAuthRepl.c_str());
		if (gbMathExpression)
			edit_ANSWER.Disable();
		variables_skin_helpbutton(m_hwnd, IDC_VARS);
		gbVarsServiceExist ? btn_VARS.Enable() : btn_VARS.Disable();
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setWString("question", edit_QUESTION.GetText());
		gbQuestion = DBGetContactSettingStringPAN(0, MODULENAME, "question", TranslateW(defQuestion));
		g_plugin.setWString("answer", edit_ANSWER.GetText());
		gbAnswer = DBGetContactSettingStringPAN(0, MODULENAME, "answer", L"nospam");
		g_plugin.setWString("authrepl", edit_AUTHREPL.GetText());
		gbAuthRepl = DBGetContactSettingStringPAN(0, MODULENAME, "authrepl", TranslateW(defAuthReply));
		g_plugin.setWString("congratulation", edit_CONGRATULATION.GetText());
		gbCongratulation = DBGetContactSettingStringPAN(0, MODULENAME, "congratulation", TranslateW(defCongrats));
		return true;
	}

	void onClick_RESTOREDEFAULTS(CCtrlButton *)
	{
		edit_QUESTION.SetText(TranslateW(defQuestion));
		edit_ANSWER.SetText(L"nospam");
		edit_AUTHREPL.SetText(TranslateW(defAuthReply));
		edit_CONGRATULATION.SetText(TranslateW(defCongrats));
	}
	void onClick_VARS(CCtrlButton *)
	{
		variables_showhelp(m_hwnd, WM_COMMAND, VHF_FULLDLG | VHF_SETLASTSUBJECT, nullptr, nullptr);
	}
};

class COptProtoDlg : public CDlgBase
{
	CCtrlListBox list_USEDPROTO, list_ALLPROTO;
	CCtrlButton btn_ADD, btn_REMOVE, btn_ADDALL, btn_REMOVEALL;

public:
	COptProtoDlg() : CDlgBase(g_plugin, IDD_PROTO),
		list_USEDPROTO(this, ID_USEDPROTO), list_ALLPROTO(this, ID_ALLPROTO),
		btn_ADD(this, ID_ADD), btn_REMOVE(this, ID_REMOVE), btn_ADDALL(this, ID_ADDALL), btn_REMOVEALL(this, ID_REMOVEALL)
	{
		btn_ADD.OnClick = Callback(this, &COptProtoDlg::onClick_ADD);
		btn_REMOVE.OnClick = Callback(this, &COptProtoDlg::onClick_REMOVE);
		btn_ADDALL.OnClick = Callback(this, &COptProtoDlg::onClick_ADDALL);
		btn_REMOVEALL.OnClick = Callback(this, &COptProtoDlg::onClick_REMOVEALL);
	}

	bool OnInitDialog() override
	{
		for (auto &pa : Accounts()) {
			wchar_t *str = mir_utf8decodeW(pa->szModuleName);
			ProtoInList(pa->szModuleName) ? list_USEDPROTO.AddString(str) : list_ALLPROTO.AddString(str);
			mir_free(str);
		}
		return true;
	}

	bool OnApply() override
	{
		LRESULT count = list_USEDPROTO.GetCount();
		std::ostringstream out;
		for (int i = 0; i < count; ++i) {
			char *str = mir_utf8encodeW(list_USEDPROTO.GetItemText(i));
			out << str << "\r\n";
			mir_free(str);
		}
		g_plugin.setString("protoList", out.str().c_str());
		return true;
	}

	void onClick_ADD(CCtrlButton *)
	{
		list_USEDPROTO.AddString(list_ALLPROTO.GetItemText(list_ALLPROTO.GetCurSel()));
		list_ALLPROTO.DeleteString(list_ALLPROTO.GetCurSel());
		this->NotifyChange();
	}

	void onClick_REMOVE(CCtrlButton *)
	{
		list_ALLPROTO.AddString(list_USEDPROTO.GetItemText(list_USEDPROTO.GetCurSel()));
		list_USEDPROTO.DeleteString(list_USEDPROTO.GetCurSel());
		this->NotifyChange();
	}

	void onClick_ADDALL(CCtrlButton *)
	{
		for (;;) {
			int count = list_ALLPROTO.GetCount();
			if (!count || LB_ERR == count)
				break;
			list_USEDPROTO.AddString(list_ALLPROTO.GetItemText(0));
			list_ALLPROTO.DeleteString(0);
		}
		this->NotifyChange();
	}

	void onClick_REMOVEALL(CCtrlButton *)
	{
		for (;;) {
			int count = list_USEDPROTO.GetCount();
			if (!count || LB_ERR == count)
				break;
			list_ALLPROTO.AddString(list_USEDPROTO.GetItemText(0));
			list_USEDPROTO.DeleteString(0);
		}
		this->NotifyChange();
	}
};

class COptAdvancedDlg : public CDlgBase
{
	CCtrlEdit edit_SPECIALGROUPNAME;
	CCtrlCheck chk_INVIS_DISABLE, chk_CASE_INSENSITIVE, chk_SPECIALGROUP, chk_EXCLUDE, chk_REMOVE_TMP;
	CCtrlCheck chk_IGNOREURL, chk_REGEX, chk_HISTORY_LOG, chk_MATH_QUESTION;
	CCtrlButton btn_MATH_DETAILS;

public:
	COptAdvancedDlg() : CDlgBase(g_plugin, IDD_ADVANCED),
		chk_INVIS_DISABLE(this, IDC_INVIS_DISABLE),
		chk_CASE_INSENSITIVE(this, IDC_CASE_INSENSITIVE),
		chk_SPECIALGROUP(this, ID_SPECIALGROUP),
		chk_EXCLUDE(this, ID_EXCLUDE),
		chk_REMOVE_TMP(this, ID_REMOVE_TMP),
		chk_IGNOREURL(this, ID_IGNOREURL),
		chk_REGEX(this, IDC_REGEX),
		chk_HISTORY_LOG(this, IDC_HISTORY_LOG),
		chk_MATH_QUESTION(this, IDC_MATH_QUESTION),
		edit_SPECIALGROUPNAME(this, ID_SPECIALGROUPNAME),
		btn_MATH_DETAILS(this, IDC_MATH_DETAILS)
	{
		btn_MATH_DETAILS.OnClick = Callback(this, &COptAdvancedDlg::onClick_MATH_DETAILS);
	}
	bool OnInitDialog() override
	{
		chk_INVIS_DISABLE.SetState(gbInvisDisable);
		chk_CASE_INSENSITIVE.SetState(gbCaseInsensitive);
		edit_SPECIALGROUPNAME.SetText(gbSpammersGroup.c_str());
		chk_SPECIALGROUP.SetState(gbSpecialGroup);
		chk_EXCLUDE.SetState(gbExclude);
		chk_REMOVE_TMP.SetState(gbDelExcluded);
		chk_IGNOREURL.SetState(gbIgnoreURL);
		chk_REGEX.SetState(gbRegexMatch);
		chk_HISTORY_LOG.SetState(gbHistoryLog);
		chk_MATH_QUESTION.SetState(gbMathExpression);
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setByte("CaseInsensitive", gbCaseInsensitive = chk_CASE_INSENSITIVE.GetState());
		g_plugin.setByte("DisableInInvis", gbInvisDisable = chk_INVIS_DISABLE.GetState());
		{
			static wstring NewGroupName = edit_SPECIALGROUPNAME.GetText(), CurrentGroupName;
			CurrentGroupName = gbSpammersGroup = DBGetContactSettingStringPAN(0, MODULENAME, "SpammersGroup", L"0");
			if (mir_wstrcmp(CurrentGroupName.c_str(), NewGroupName.c_str()) != 0) {
				bool GroupExist = Clist_GroupExists(NewGroupName.c_str()) != NULL;
				g_plugin.setWString("SpammersGroup", NewGroupName.c_str());
				gbSpammersGroup = DBGetContactSettingStringPAN(0, MODULENAME, "SpammersGroup", L"Spammers");
				if (!GroupExist && gbSpecialGroup)
					Clist_GroupCreate(0, gbSpammersGroup.c_str());
			}
		}
		g_plugin.setByte("SpecialGroup", gbSpecialGroup = chk_SPECIALGROUP.GetState());
		g_plugin.setByte("ExcludeContacts", gbExclude = chk_EXCLUDE.GetState());
		g_plugin.setByte("DelExcluded", gbDelExcluded = chk_REMOVE_TMP.GetState());
		g_plugin.setByte("IgnoreURL", gbIgnoreURL = chk_IGNOREURL.GetState());

		g_plugin.setByte("RegexMatch", gbRegexMatch = chk_REGEX.GetState());
		g_plugin.setByte("HistoryLog", gbHistoryLog = chk_HISTORY_LOG.GetState());
		g_plugin.setByte("MathExpression", gbMathExpression = chk_MATH_QUESTION.GetState());
		return true;
	}

	void onClick_MATH_DETAILS(CCtrlButton *)
	{
		MessageBox(m_hwnd, TranslateT("If math expression is turned on, you can use following expression in message text:\nXX+XX-X/X*X\neach X will be replaced by one random number and answer will be expression result.\nMessage must contain only one expression without spaces."), TranslateT("Info"), MB_OK);
	}
};

int OnOptInit(WPARAM w, LPARAM l)
{
	UNREFERENCED_PARAMETER(l);

	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Message sessions");
	odp.szTitle.w = LPGENW("StopSpam");
	odp.position = -1;
	odp.flags = ODPF_UNICODE;

	odp.szTab.w = LPGENW("General");
	odp.pDialog = new COptMainDlg();
	g_plugin.addOptions(w, &odp);


	odp.szTab.w = LPGENW("Messages");
	odp.pDialog = new COptMessagesDlg();
	g_plugin.addOptions(w, &odp);

	odp.szTab.w = LPGENW("Accounts");
	odp.pDialog = new COptProtoDlg();
	g_plugin.addOptions(w, &odp);

	odp.szTab.w = LPGENW("Advanced");
	odp.pDialog = new COptAdvancedDlg();
	g_plugin.addOptions(w, &odp);
	return 0;
}
