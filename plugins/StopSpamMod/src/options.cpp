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
public:
	COptMainDlg() : CDlgBase(g_plugin, IDD_MAIN),
		chk_INFTALKPROT(this, ID_INFTALKPROT), chk_ADDPERMANENT(this, ID_ADDPERMANENT), chk_HANDLEAUTHREQ(this, ID_HANDLEAUTHREQ),
		chk_HIDECONTACTS(this, ID_HIDECONTACTS), chk_IGNORESPAMMERS(this, ID_IGNORESPAMMERS), chk_LOGSPAMTOFILE(this, ID_LOGSPAMTOFILE),
		ctrl_DESCRIPTION(this, ID_DESCRIPTION), edit_MAXQUESTCOUNT(this, ID_MAXQUESTCOUNT)
	{

	}
	virtual void OnInitDialog() override
	{
		ctrl_DESCRIPTION.SetText(TranslateW(pluginDescription));
		edit_MAXQUESTCOUNT.SetInt(gbMaxQuestCount);
		chk_INFTALKPROT.SetState(gbInfTalkProtection);
		chk_ADDPERMANENT.SetState(gbAddPermanent);
		chk_HANDLEAUTHREQ.SetState(gbHandleAuthReq);
		chk_HIDECONTACTS.SetState(gbHideContacts);
		chk_IGNORESPAMMERS.SetState(gbIgnoreContacts);
		chk_LOGSPAMTOFILE.SetState(gbLogToFile);

	}
	virtual void OnApply() override
	{
		db_set_dw(NULL, pluginName, "maxQuestCount", gbMaxQuestCount = edit_MAXQUESTCOUNT.GetInt());
		db_set_b(NULL, pluginName, "infTalkProtection", gbInfTalkProtection = chk_INFTALKPROT.GetState());
		db_set_b(NULL, pluginName, "addPermanent", gbAddPermanent = chk_ADDPERMANENT.GetState());
		db_set_b(NULL, pluginName, "handleAuthReq", gbHandleAuthReq = chk_HANDLEAUTHREQ.GetState());
		db_set_b(NULL, pluginName, "HideContacts", gbHideContacts = chk_HIDECONTACTS.GetState());
		db_set_b(NULL, pluginName, "IgnoreContacts", gbIgnoreContacts = chk_IGNORESPAMMERS.GetState());
		db_set_b(NULL, pluginName, "LogSpamToFile", gbLogToFile = chk_LOGSPAMTOFILE.GetState());
	}
private:
	CCtrlCheck chk_INFTALKPROT, chk_ADDPERMANENT, chk_HANDLEAUTHREQ, chk_HIDECONTACTS, chk_IGNORESPAMMERS, chk_LOGSPAMTOFILE;
	CCtrlData ctrl_DESCRIPTION;
	CCtrlSpin edit_MAXQUESTCOUNT;
};

class COptMessagesDlg : public CDlgBase
{
public:
	COptMessagesDlg() : CDlgBase(g_plugin, IDD_MESSAGES),
		edit_QUESTION(this, ID_QUESTION), edit_ANSWER(this, ID_ANSWER), edit_CONGRATULATION(this, ID_CONGRATULATION), edit_AUTHREPL(this, ID_AUTHREPL),
		btn_RESTOREDEFAULTS(this, ID_RESTOREDEFAULTS), btn_VARS(this, IDC_VARS)
	{
		btn_RESTOREDEFAULTS.OnClick = Callback(this, &COptMessagesDlg::onClick_RESTOREDEFAULTS);
		btn_VARS.OnClick = Callback(this, &COptMessagesDlg::onClick_VARS);

	}
	virtual void OnInitDialog() override
	{
		edit_QUESTION.SetText(gbQuestion.c_str());
		edit_ANSWER.SetText(gbAnswer.c_str());
		edit_CONGRATULATION.SetText(gbCongratulation.c_str());
		edit_AUTHREPL.SetText(gbAuthRepl.c_str());
		if (gbMathExpression)
			edit_ANSWER.Disable();
		variables_skin_helpbutton(m_hwnd, IDC_VARS);
		gbVarsServiceExist ? btn_VARS.Enable() : btn_VARS.Disable();
	}
	virtual void OnApply() override
	{
		db_set_ws(NULL, pluginName, "question", edit_QUESTION.GetText());
		gbQuestion = DBGetContactSettingStringPAN(NULL, pluginName, "question", TranslateW(defQuestion));
		db_set_ws(NULL, pluginName, "answer", edit_ANSWER.GetText());
		gbAnswer = DBGetContactSettingStringPAN(NULL, pluginName, "answer", L"nospam");
		db_set_ws(NULL, pluginName, "authrepl", edit_AUTHREPL.GetText());
		gbAuthRepl = DBGetContactSettingStringPAN(NULL, pluginName, "authrepl", TranslateW(defAuthReply));
		db_set_ws(NULL, pluginName, "congratulation", edit_CONGRATULATION.GetText());
		gbCongratulation = DBGetContactSettingStringPAN(NULL, pluginName, "congratulation", TranslateW(defCongrats));
	}
	void onClick_RESTOREDEFAULTS(CCtrlButton*)
	{
		edit_QUESTION.SetText(TranslateW(defQuestion));
		edit_ANSWER.SetText(L"nospam");
		edit_AUTHREPL.SetText(TranslateW(defAuthReply));
		edit_CONGRATULATION.SetText(TranslateW(defCongrats));
	}
	void onClick_VARS(CCtrlButton*)
	{
		variables_showhelp(m_hwnd, WM_COMMAND, VHF_FULLDLG | VHF_SETLASTSUBJECT, nullptr, nullptr);
	}
private:
	CCtrlEdit edit_QUESTION, edit_ANSWER, edit_CONGRATULATION, edit_AUTHREPL;
	CCtrlButton btn_RESTOREDEFAULTS, btn_VARS;

};

class COptProtoDlg : public CDlgBase
{
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
	
	virtual void OnInitDialog() override
	{
		for (auto &pa : Accounts()) {
			wchar_t *str = mir_utf8decodeW(pa->szModuleName);
			ProtoInList(pa->szModuleName) ? list_USEDPROTO.AddString(str) : list_ALLPROTO.AddString(str);
			mir_free(str);
		}
	}
	
	virtual void OnApply() override
	{
		
		LRESULT count = list_USEDPROTO.GetCount();
		std::ostringstream out;
		for (int i = 0; i < count; ++i) {
			char *str = mir_utf8encodeW(list_USEDPROTO.GetItemText(i));
			out << str << "\r\n";
			mir_free(str);
		}
		db_set_s(NULL, pluginName, "protoList", out.str().c_str());
	}
	void onClick_ADD(CCtrlButton*)
	{
		list_USEDPROTO.AddString(list_ALLPROTO.GetItemText(list_ALLPROTO.GetCurSel()));
		list_ALLPROTO.DeleteString(list_ALLPROTO.GetCurSel());
	}
	void onClick_REMOVE(CCtrlButton*)
	{
		list_ALLPROTO.AddString(list_USEDPROTO.GetItemText(list_USEDPROTO.GetCurSel()));
		list_USEDPROTO.DeleteString(list_USEDPROTO.GetCurSel());
	}
	void onClick_ADDALL(CCtrlButton*)
	{
		for (;;) {
			int count = list_ALLPROTO.GetCount();
			if (!count || LB_ERR == count)
				break;
			list_USEDPROTO.AddString(list_ALLPROTO.GetItemText(0));
			list_ALLPROTO.DeleteString(0);
		}
	}
	void onClick_REMOVEALL(CCtrlButton*)
	{
		for (;;) {
			int count = list_USEDPROTO.GetCount();
			if (!count || LB_ERR == count)
				break;
			list_ALLPROTO.AddString(list_USEDPROTO.GetItemText(0));
			list_USEDPROTO.DeleteString(0);
		}
	}
private:
	CCtrlListBox list_USEDPROTO, list_ALLPROTO;
	CCtrlButton btn_ADD, btn_REMOVE, btn_ADDALL, btn_REMOVEALL;

};

class COptAdvancedDlg : public CDlgBase
{
public:
	COptAdvancedDlg() : CDlgBase(g_plugin, IDD_ADVANCED),
		chk_INVIS_DISABLE(this, IDC_INVIS_DISABLE), chk_CASE_INSENSITIVE(this, IDC_CASE_INSENSITIVE), chk_SPECIALGROUP(this, ID_SPECIALGROUP), chk_EXCLUDE(this, ID_EXCLUDE),
		chk_REMOVE_TMP(this, ID_REMOVE_TMP), chk_REMOVE_TMP_ALL(this, ID_REMOVE_TMP_ALL), chk_IGNOREURL(this, ID_IGNOREURL), chk_AUTOAUTH(this, IDC_AUTOAUTH), chk_ADDTOSRVLST(this, IDC_ADDTOSRVLST),
		chk_REQAUTH(this, IDC_REQAUTH), chk_REGEX(this, IDC_REGEX), chk_HISTORY_LOG(this, IDC_HISTORY_LOG), chk_MATH_QUESTION(this, IDC_MATH_QUESTION),
		edit_SPECIALGROUPNAME(this, ID_SPECIALGROUPNAME), edit_AUTOADDGROUP(this, IDC_AUTOADDGROUP),
		btn_MATH_DETAILS(this, IDC_MATH_DETAILS)
	{
		btn_MATH_DETAILS.OnClick = Callback(this, &COptAdvancedDlg::onClick_MATH_DETAILS);
	}
	virtual void OnInitDialog() override
	{
		chk_INVIS_DISABLE.SetState(gbInvisDisable);
		chk_CASE_INSENSITIVE.SetState(gbCaseInsensitive);
		edit_SPECIALGROUPNAME.SetText(gbSpammersGroup.c_str());
		chk_SPECIALGROUP.SetState(gbSpecialGroup);
		chk_EXCLUDE.SetState(gbExclude);
		chk_REMOVE_TMP.SetState(gbDelExcluded);
		chk_REMOVE_TMP_ALL.SetState(gbDelAllTempory);
		chk_IGNOREURL.SetState(gbIgnoreURL);
		chk_AUTOAUTH.SetState(gbAutoAuth);
		chk_ADDTOSRVLST.SetState(gbAutoAddToServerList);
		chk_REQAUTH.SetState(gbAutoReqAuth);
		chk_REGEX.SetState(gbRegexMatch);
		chk_HISTORY_LOG.SetState(gbHistoryLog);
		chk_MATH_QUESTION.SetState(gbMathExpression);
		edit_AUTOADDGROUP.SetText(gbAutoAuthGroup.c_str());
	}
	virtual void OnApply() override
	{
		db_set_b(NULL, pluginName, "CaseInsensitive", gbCaseInsensitive = chk_CASE_INSENSITIVE.GetState());
		db_set_b(NULL, pluginName, "DisableInInvis", gbInvisDisable = chk_INVIS_DISABLE.GetState());
		{
			static wstring NewGroupName = edit_SPECIALGROUPNAME.GetText(), CurrentGroupName;
			CurrentGroupName = gbSpammersGroup = DBGetContactSettingStringPAN(NULL, pluginName, "SpammersGroup", L"0");
			if (mir_wstrcmp(CurrentGroupName.c_str(), NewGroupName.c_str()) != 0) {
				bool GroupExist = Clist_GroupExists(NewGroupName.c_str()) != NULL;
				db_set_ws(NULL, pluginName, "SpammersGroup", NewGroupName.c_str());
				gbSpammersGroup = DBGetContactSettingStringPAN(NULL, pluginName, "SpammersGroup", L"Spammers");
				if (!GroupExist && gbSpecialGroup)
					Clist_GroupCreate(0, gbSpammersGroup.c_str());
			}
		}
		db_set_b(NULL, pluginName, "SpecialGroup", gbSpecialGroup = chk_SPECIALGROUP.GetState());
		db_set_b(NULL, pluginName, "ExcludeContacts", gbExclude = chk_EXCLUDE.GetState());
		db_set_b(NULL, pluginName, "DelExcluded", gbDelExcluded = chk_REMOVE_TMP.GetState());
		db_set_b(NULL, pluginName, "DelAllTempory", gbDelAllTempory = chk_REMOVE_TMP_ALL.GetState());
		db_set_b(NULL, pluginName, "IgnoreURL", gbIgnoreURL = chk_IGNOREURL.GetState());

		db_set_b(NULL, pluginName, "AutoAuth", gbAutoAuth = chk_AUTOAUTH.GetState());
		db_set_b(NULL, pluginName, "AutoAddToServerList", gbAutoAddToServerList = chk_ADDTOSRVLST.GetState());
		db_set_b(NULL, pluginName, "AutoReqAuth", gbAutoReqAuth = chk_REQAUTH.GetState());
		db_set_b(NULL, pluginName, "RegexMatch", gbRegexMatch = chk_REGEX.GetState());
		db_set_b(NULL, pluginName, "HistoryLog", gbHistoryLog = chk_HISTORY_LOG.GetState());
		db_set_b(NULL, pluginName, "MathExpression", gbMathExpression = chk_MATH_QUESTION.GetState());
		{
			static wstring NewAGroupName = edit_AUTOADDGROUP.GetText(), CurrentAGroupName;
			CurrentAGroupName = gbAutoAuthGroup = DBGetContactSettingStringPAN(NULL, pluginName, "AutoAuthGroup", L"0");
			if (mir_wstrcmp(CurrentAGroupName.c_str(), NewAGroupName.c_str()) != 0) {
				bool GroupExist = Clist_GroupExists(NewAGroupName.c_str()) != NULL;
				db_set_ws(NULL, pluginName, "AutoAuthGroup", NewAGroupName.c_str());
				gbAutoAuthGroup = DBGetContactSettingStringPAN(NULL, pluginName, "AutoAuthGroup", L"Not Spammers");
				if (!GroupExist && gbAutoAddToServerList)
					Clist_GroupCreate(0, gbAutoAuthGroup.c_str());
			}
		}
	}
	void onClick_MATH_DETAILS(CCtrlButton*)
	{
		MessageBox(m_hwnd, TranslateT("If math expression is turned on, you can use following expression in message text:\nXX+XX-X/X*X\neach X will be replaced by one random number and answer will be expression result.\nMessage must contain only one expression without spaces."), TranslateT("Info"), MB_OK);
	}
private:
	CCtrlCheck chk_INVIS_DISABLE, chk_CASE_INSENSITIVE, chk_SPECIALGROUP, chk_EXCLUDE, chk_REMOVE_TMP, chk_REMOVE_TMP_ALL, chk_IGNOREURL, chk_AUTOAUTH, chk_ADDTOSRVLST, chk_REQAUTH, chk_REGEX, chk_HISTORY_LOG, chk_MATH_QUESTION;
	CCtrlEdit edit_SPECIALGROUPNAME, edit_AUTOADDGROUP;
	CCtrlButton btn_MATH_DETAILS;

};




int OnOptInit(WPARAM w, LPARAM l)
{
	UNREFERENCED_PARAMETER(l);

	OPTIONSDIALOGPAGE odp = { 0 };
	odp.szGroup.w = LPGENW("Message sessions");
	odp.szTitle.w = LPGENW("StopSpam");
	odp.position = -1;
	odp.flags = ODPF_UNICODE;

	odp.szTab.w = LPGENW("General");
	odp.pDialog = new COptMainDlg();
	Options_AddPage(w, &odp);


	odp.szTab.w = LPGENW("Messages");
	odp.pDialog = new COptMessagesDlg();
	Options_AddPage(w, &odp);

	odp.szTab.w = LPGENW("Accounts");
	odp.pDialog = new COptProtoDlg();
	Options_AddPage(w, &odp);

	odp.szTab.w = LPGENW("Advanced");
	odp.pDialog = new COptAdvancedDlg();
	Options_AddPage(w, &odp);
	return 0;
}
