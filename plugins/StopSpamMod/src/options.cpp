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

#include "headers.h"

int CreateCListGroup(TCHAR* szGroupName);

char * pluginDescription = LPGEN("No more spam! Robots can't go! Only human beings invited!\r\n\r\nThis plugin works pretty simple:\r\nWhile messages from users on your contact list go as there is no any anti-spam software, messages from unknown users are not delivered to you. But also they are not ignored, this plugin replies with a simple question, and if user gives the right answer, plugin adds him to your contact list so that he can contact you.");
TCHAR const * defQuestion = TranslateT("Spammers made me to install small anti-spam system you are now speaking with.\r\nPlease reply \"nospam\" without quotes and spaces if you want to contact me.");

INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			SetDlgItemTextA(hwnd, ID_DESCRIPTION, pluginDescription);
			TranslateDialogDefault(hwnd);
			SetDlgItemInt(hwnd, ID_MAXQUESTCOUNT, gbMaxQuestCount, FALSE);
			CheckDlgButton(hwnd, ID_INFTALKPROT, gbInfTalkProtection ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd, ID_ADDPERMANENT, gbAddPermanent ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd, ID_HANDLEAUTHREQ, gbHandleAuthReq ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd, ID_HIDECONTACTS, gbHideContacts ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd, ID_IGNORESPAMMERS, gbIgnoreContacts ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd, ID_LOGSPAMTOFILE, gbLogToFile ? BST_CHECKED : BST_UNCHECKED);
		}
		return TRUE;
	case WM_COMMAND:{
		switch (LOWORD(wParam))
		{
		case ID_MAXQUESTCOUNT:
			{
				if (EN_CHANGE != HIWORD(wParam) || (HWND)lParam != GetFocus())
					return FALSE;
				break;
			}
		}
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
					}
		break;
	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code)
			{
			case PSN_APPLY:
				{
					db_set_dw(NULL, pluginName, "maxQuestCount", gbMaxQuestCount =	GetDlgItemInt(hwnd, ID_MAXQUESTCOUNT, NULL, FALSE));
					db_set_b(NULL, pluginName, "infTalkProtection", gbInfTalkProtection =  BST_CHECKED == IsDlgButtonChecked(hwnd, ID_INFTALKPROT));
					db_set_b(NULL, pluginName, "addPermanent", gbAddPermanent = BST_CHECKED == IsDlgButtonChecked(hwnd, ID_ADDPERMANENT));
					db_set_b(NULL, pluginName, "handleAuthReq", gbHandleAuthReq = BST_CHECKED == IsDlgButtonChecked(hwnd, ID_HANDLEAUTHREQ));
					db_set_b(NULL, pluginName, "HideContacts",  gbHideContacts = BST_CHECKED == IsDlgButtonChecked(hwnd, ID_HIDECONTACTS));
					db_set_b(NULL, pluginName, "IgnoreContacts",  gbIgnoreContacts = BST_CHECKED == IsDlgButtonChecked(hwnd, ID_IGNORESPAMMERS));
					db_set_b(NULL, pluginName, "LogSpamToFile",  gbLogToFile = BST_CHECKED == IsDlgButtonChecked(hwnd, ID_LOGSPAMTOFILE));
				}
				return TRUE;
			}
		}
		break;			
	}
	return FALSE;
}

INT_PTR CALLBACK MessagesDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch(msg)
	{
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwnd);
			SetDlgItemText(hwnd, ID_QUESTION, gbQuestion.c_str());
			SetDlgItemText(hwnd, ID_ANSWER, gbAnswer.c_str());			
			SetDlgItemText(hwnd, ID_CONGRATULATION, gbCongratulation.c_str());
			SetDlgItemText(hwnd, ID_AUTHREPL, gbAuthRepl.c_str());
			EnableWindow(GetDlgItem(hwnd, ID_ANSWER), !gbMathExpression);
			variables_skin_helpbutton(hwnd, IDC_VARS);
			gbVarsServiceExist?EnableWindow(GetDlgItem(hwnd, IDC_VARS),1):EnableWindow(GetDlgItem(hwnd, IDC_VARS),0);
		}
		return TRUE;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case ID_QUESTION:
			case ID_ANSWER:
			case ID_AUTHREPL:
			case ID_CONGRATULATION:
				{
					if (EN_CHANGE != HIWORD(wParam) || (HWND)lParam != GetFocus())
						return FALSE;
					break;
				}
			case ID_RESTOREDEFAULTS:
				SetDlgItemText(hwnd, ID_QUESTION, defQuestion);
				SetDlgItemText(hwnd, ID_ANSWER,  _T("nospam"));
				SetDlgItemText(hwnd, ID_AUTHREPL, TranslateT("StopSpam: send a message and reply to an anti-spam bot question."));
				SetDlgItemText(hwnd, ID_CONGRATULATION, TranslateT("Congratulations! You just passed human/robot test. Now you can write me a message."));
				SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
				return TRUE;
			case IDC_VARS:
				variables_showhelp(hwnd, msg, VHF_FULLDLG|VHF_SETLASTSUBJECT, NULL, NULL);
				return TRUE;
			}
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		}
		break;
	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code)
			{
			case PSN_APPLY:
				{
					db_set_ws(NULL, pluginName, "question", GetDlgItemString(hwnd, ID_QUESTION).c_str());
					gbQuestion = DBGetContactSettingStringPAN(NULL, pluginName, "question", defQuestion);
					db_set_ws(NULL, pluginName, "answer", GetDlgItemString(hwnd, ID_ANSWER).c_str());
					gbAnswer = DBGetContactSettingStringPAN(NULL, pluginName, "answer", _T("nospam"));
					db_set_ws(NULL, pluginName, "authrepl", GetDlgItemString(hwnd, ID_AUTHREPL).c_str());
					gbAuthRepl = DBGetContactSettingStringPAN(NULL, pluginName, "authrepl", TranslateT("StopSpam: send a message and reply to an anti-spam bot question."));
					db_set_ws(NULL, pluginName, "congratulation", GetDlgItemString(hwnd, ID_CONGRATULATION).c_str());
					gbCongratulation = DBGetContactSettingStringPAN(NULL, pluginName, "congratulation", TranslateT("Congratulations! You just passed human/robot test. Now you can write me a message."));
				}
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK ProtoDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwnd);
			int n;
			PROTOACCOUNT **pppd;
			if(!ProtoEnumAccounts(&n, &pppd))
				for (int i = 0; i < n; ++i) {
					SendDlgItemMessageA(hwnd, (ProtoInList(pppd[i]->szModuleName) ? ID_USEDPROTO : ID_ALLPROTO), LB_ADDSTRING, 0, (LPARAM)pppd[i]->szModuleName);
				}
		}
		return TRUE;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case ID_ADD:
			{
				WPARAM n = (WPARAM)SendDlgItemMessage(hwnd, ID_ALLPROTO, LB_GETCURSEL, 0, 0);
				if(LB_ERR != n) {
					size_t len = SendDlgItemMessage(hwnd, ID_ALLPROTO, LB_GETTEXTLEN, n, 0);
					if(LB_ERR != len) {
						TCHAR * buf = new TCHAR[len + 1];
						SendDlgItemMessage(hwnd, ID_ALLPROTO, LB_GETTEXT, n, (LPARAM)buf);
						SendDlgItemMessage(hwnd, ID_USEDPROTO, LB_ADDSTRING, 0, (LPARAM)buf);
						delete []buf;
						SendDlgItemMessage(hwnd, ID_ALLPROTO, LB_DELETESTRING, n, 0);
					}
				}
			}
			break;
		case ID_REMOVE:
			{
				WPARAM n = (WPARAM)SendDlgItemMessage(hwnd, ID_USEDPROTO, LB_GETCURSEL, 0, 0);
				if(LB_ERR != n) {
					size_t len = SendDlgItemMessage(hwnd, ID_USEDPROTO, LB_GETTEXTLEN, n, 0);
					if(LB_ERR != len) {
						TCHAR * buf = new TCHAR[len + 1];
						SendDlgItemMessage(hwnd, ID_USEDPROTO, LB_GETTEXT, n, (LPARAM)buf);
						SendDlgItemMessage(hwnd, ID_ALLPROTO, LB_ADDSTRING, 0, (LPARAM)buf);
						delete []buf;
						SendDlgItemMessage(hwnd, ID_USEDPROTO, LB_DELETESTRING, n, 0);
					}
				}
			}
			break;
		case ID_ADDALL:
			for(;;)
			{
				LRESULT count = SendDlgItemMessage(hwnd, ID_ALLPROTO, LB_GETCOUNT, 0, 0);
				if(!count || LB_ERR == count)
					break;
				SendDlgItemMessage(hwnd, ID_ALLPROTO, LB_SETCURSEL, 0, 0);
				SendMessage(hwnd, WM_COMMAND, ID_ADD, 0);
			}
			break;
		case ID_REMOVEALL:
			for(;;)
			{
				LRESULT count = SendDlgItemMessage(hwnd, ID_USEDPROTO, LB_GETCOUNT, 0, 0);
				if(!count || LB_ERR == count)
					break;
				SendDlgItemMessage(hwnd, ID_USEDPROTO, LB_SETCURSEL, 0, 0);
				SendMessage(hwnd, WM_COMMAND, ID_REMOVE, 0);
			}
			break;
		default:
			return FALSE;
		}
		SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
		return TRUE;
	case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code)
			{
			case PSN_APPLY:
				{
					LRESULT count = SendDlgItemMessage(hwnd, ID_USEDPROTO, LB_GETCOUNT, 0, 0);
					std::ostringstream out;
					for(int i = 0; i < count; ++i) {
						size_t len = SendDlgItemMessageA(hwnd, ID_USEDPROTO, LB_GETTEXTLEN, i, 0);
						if(LB_ERR != len) {
							char * buf = new char[len + 1];
							SendDlgItemMessageA(hwnd, ID_USEDPROTO, LB_GETTEXT, i, (LPARAM)buf);
							out << buf << "\r\n";
							delete []buf;
						}
					}
					db_set_s(NULL, pluginName, "protoList", out.str().c_str());
				}
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK AdvancedDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwnd);
		CheckDlgButton(hwnd, IDC_INVIS_DISABLE, gbInvisDisable ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_CASE_INSENSITIVE, gbCaseInsensitive ? BST_CHECKED : BST_UNCHECKED);
		gbDosServiceExist?EnableWindow(GetDlgItem(hwnd, ID_DOS_INTEGRATION),1):EnableWindow(GetDlgItem(hwnd, ID_DOS_INTEGRATION),0);
		CheckDlgButton(hwnd, ID_DOS_INTEGRATION, gbDosServiceIntegration ? BST_CHECKED : BST_UNCHECKED);
		SetDlgItemText(hwnd, ID_SPECIALGROUPNAME, gbSpammersGroup.c_str());
		CheckDlgButton(hwnd, ID_SPECIALGROUP, gbSpecialGroup ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, ID_EXCLUDE, gbExclude ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, ID_REMOVE_TMP, gbDelExcluded ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, ID_REMOVE_TMP_ALL, gbDelAllTempory ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, ID_IGNOREURL, gbIgnoreURL ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_AUTOAUTH, gbAutoAuth ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_ADDTOSRVLST, gbAutoAddToServerList ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_REQAUTH, gbAutoReqAuth ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_REGEX, gbRegexMatch ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_HISTORY_LOG, gbHistoryLog ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_MATH_QUESTION, gbMathExpression ? BST_CHECKED : BST_UNCHECKED);

		SetDlgItemText(hwnd, IDC_AUTOADDGROUP, gbAutoAuthGroup.c_str());
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MATH_DETAILS:
			MessageBox(hwnd, TranslateT("If math expression is turned on, you can use following expression in message text:\nXX+XX-X/X*X\neach X will be replaced by one random number and answer will be expression result.\nMessage must contain only one expression without spaces."), TranslateT("Info"), MB_OK);
			break;

		case IDC_INVIS_DISABLE: 
		case IDC_CASE_INSENSITIVE: 
		case ID_DOS_INTEGRATION:
		case ID_SPECIALGROUPNAME: 
		case ID_SPECIALGROUP: 
		case ID_EXCLUDE: 
		case ID_REMOVE_TMP: 
		case ID_REMOVE_TMP_ALL:
		case ID_IGNOREURL:
		case IDC_AUTOAUTH:
		case IDC_ADDTOSRVLST:
		case IDC_REQAUTH:
		case IDC_AUTOADDGROUP:
		case IDC_REGEX:
		case IDC_HISTORY_LOG:
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
			break;
		}
		break;

	case WM_NOTIFY:
		NMHDR* nmhdr = (NMHDR*)lParam;
		switch (nmhdr->code) {
		case PSN_APPLY:
			db_set_b(NULL, pluginName, "CaseInsensitive", gbCaseInsensitive = BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_CASE_INSENSITIVE));
			db_set_b(NULL, pluginName, "DisableInInvis", gbInvisDisable = BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_INVIS_DISABLE));
			db_set_b(NULL, pluginName, "DOSIntegration",  gbDosServiceIntegration = BST_CHECKED == IsDlgButtonChecked(hwnd, ID_DOS_INTEGRATION));
			{
				static tstring NewGroupName, CurrentGroupName;
				NewGroupName = GetDlgItemString(hwnd, ID_SPECIALGROUPNAME);
				CurrentGroupName = gbSpammersGroup = DBGetContactSettingStringPAN(NULL, pluginName, "SpammersGroup", _T("0"));
				if (wcscmp(CurrentGroupName.c_str(), NewGroupName.c_str()) != 0) {
					bool GroupExist = Clist_GroupExists(NewGroupName.c_str()) != NULL;
					db_set_ws(NULL,pluginName, "SpammersGroup", NewGroupName.c_str());
					gbSpammersGroup = DBGetContactSettingStringPAN(NULL,pluginName,"SpammersGroup", _T("Spammers"));
					if(!GroupExist && gbSpecialGroup)
						CreateCListGroup((TCHAR*)gbSpammersGroup.c_str());
				}
			}
			db_set_b(NULL, pluginName, "SpecialGroup",  gbSpecialGroup = BST_CHECKED == IsDlgButtonChecked(hwnd, ID_SPECIALGROUP));
			db_set_b(NULL, pluginName, "ExcludeContacts",  gbExclude = BST_CHECKED == IsDlgButtonChecked(hwnd, ID_EXCLUDE));
			db_set_b(NULL, pluginName, "DelExcluded",  gbDelExcluded = BST_CHECKED == IsDlgButtonChecked(hwnd, ID_REMOVE_TMP));
			db_set_b(NULL, pluginName, "DelAllTempory",  gbDelAllTempory = BST_CHECKED == IsDlgButtonChecked(hwnd, ID_REMOVE_TMP_ALL));
			db_set_b(NULL, pluginName, "IgnoreURL",  gbIgnoreURL =	BST_CHECKED == IsDlgButtonChecked(hwnd, ID_IGNOREURL));

			db_set_b(NULL, pluginName, "AutoAuth",  gbAutoAuth = BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_AUTOAUTH));
			db_set_b(NULL, pluginName, "AutoAddToServerList",  gbAutoAddToServerList = BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_ADDTOSRVLST));
			db_set_b(NULL, pluginName, "AutoReqAuth",  gbAutoReqAuth = BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_REQAUTH));
			db_set_b(NULL, pluginName, "RegexMatch",  gbRegexMatch = BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_REGEX));
			db_set_b(NULL, pluginName, "HistoryLog",  gbHistoryLog = BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_HISTORY_LOG));
			db_set_b(NULL, pluginName, "MathExpression",  gbMathExpression = BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_MATH_QUESTION));

			{
				static tstring NewAGroupName, CurrentAGroupName;
				NewAGroupName = GetDlgItemString(hwnd, IDC_AUTOADDGROUP);
				CurrentAGroupName = gbAutoAuthGroup = DBGetContactSettingStringPAN(NULL, pluginName, "AutoAuthGroup", _T("0"));
				if (wcscmp(CurrentAGroupName.c_str(), NewAGroupName.c_str()) != 0) {
					bool GroupExist = Clist_GroupExists(NewAGroupName.c_str()) != NULL;
					db_set_ws(NULL,pluginName, "AutoAuthGroup", NewAGroupName.c_str());
					gbAutoAuthGroup = DBGetContactSettingStringPAN(NULL,pluginName,"AutoAuthGroup", _T("Not Spammers"));
					if(!GroupExist && gbAutoAddToServerList)
						CreateCListGroup((TCHAR*)gbAutoAuthGroup.c_str());
				}
			}
			return TRUE;
		}
		break;			
	}
	return FALSE;
}


HINSTANCE hInst;
MIRANDA_HOOK_EVENT(ME_OPT_INITIALISE, w, l)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.ptszGroup = LPGENT("Message sessions");
	odp.ptszTitle = LPGENT("StopSpam");
	odp.position = -1;
	odp.hInstance = hInst;
	odp.flags = ODPF_TCHAR;

	odp.ptszTab = LPGENT("General");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MAIN);
	odp.pfnDlgProc = MainDlgProc;
	Options_AddPage(w, &odp);


	odp.ptszTab = LPGENT("Messages");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_MESSAGES);
	odp.pfnDlgProc = MessagesDlgProc;
	Options_AddPage(w, &odp);

	odp.ptszTab = LPGENT("Accounts");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PROTO);
	odp.pfnDlgProc = ProtoDlgProc;
	Options_AddPage(w, &odp);

	odp.ptszTab = LPGENT("Advanced");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_ADVANCED);
	odp.pfnDlgProc = AdvancedDlgProc;
	Options_AddPage(w, &odp);
	return 0;
}
