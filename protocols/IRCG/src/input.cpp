/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-09 George Hazan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"
#include "version.h"

#define NICKSUBSTITUTE L"!_nick_!"

void CIrcProto::FormatMsg(CMStringW& text)
{
	wchar_t temp[30];
	mir_wstrncpy(temp, GetWord(text.c_str(), 0).c_str(), 29);
	CharLower(temp);
	CMStringW command = temp;
	CMStringW S = L"";
	if (command == L"/quit" || command == L"/away")
		S = GetWord(text.c_str(), 0) + L" :" + GetWordAddress(text.c_str(), 1);
	else if (command == L"/privmsg" || command == L"/part" || command == L"/topic" || command == L"/notice") {
		S = GetWord(text.c_str(), 0) + L" " + GetWord(text.c_str(), 1) + L" :";
		if (!GetWord(text.c_str(), 2).IsEmpty())
			S += CMStringW(GetWordAddress(text.c_str(), 2));
	}
	else if (command == L"/kick") {
		S = GetWord(text.c_str(), 0) + L" " + GetWord(text.c_str(), 1) + L" " + GetWord(text.c_str(), 2) + L" :" + GetWordAddress(text.c_str(), 3);
	}
	else if (command == L"/nick") {
		if (!wcsstr(GetWord(text.c_str(), 1).c_str(), NICKSUBSTITUTE)) {
			sNick4Perform = GetWord(text.c_str(), 1);
			S = GetWordAddress(text.c_str(), 0);
		}
		else {
			CMStringW sNewNick = GetWord(text.c_str(), 1);
			if (sNick4Perform == L"") {
				DBVARIANT dbv;
				if (!getWString("PNick", &dbv)) {
					sNick4Perform = dbv.pwszVal;
					db_free(&dbv);
				}
			}

			sNewNick.Replace(NICKSUBSTITUTE, sNick4Perform.c_str());
			S = GetWord(text.c_str(), 0) + L" " + sNewNick;
		}
	}
	else S = GetWordAddress(text.c_str(), 0);

	S.Delete(0, 1);
	text = S;
}

static void AddCR(CMStringW& text)
{
	text.Replace(L"\n", L"\r\n");
	text.Replace(L"\r\r", L"\r");
}

CMStringW CIrcProto::DoAlias(const wchar_t *text, wchar_t *window)
{
	CMStringW Messageout = L"";
	const wchar_t* p1 = text;
	const wchar_t* p2 = text;
	bool LinebreakFlag = false, hasAlias = false;
	p2 = wcsstr(p1, L"\r\n");
	if (!p2)
		p2 = wcschr(p1, 0);
	if (p1 == p2)
		return (CMStringW)text;

	do {
		if (LinebreakFlag)
			Messageout += L"\r\n";

		wchar_t* line = new wchar_t[p2 - p1 + 1];
		mir_wstrncpy(line, p1, p2 - p1 + 1);
		wchar_t* test = line;
		while (*test == ' ')
			test++;
		if (*test == '/') {
			mir_wstrncpy(line, GetWordAddress(line, 0), p2 - p1 + 1);
			CMStringW S = line;
			delete[] line;
			line = new wchar_t[S.GetLength() + 2];
			mir_wstrncpy(line, S.c_str(), S.GetLength() + 1);
			CMStringW alias(m_alias);
			const wchar_t* p3 = wcsstr(alias.c_str(), (GetWord(line, 0) + L" ").c_str());
			if (p3 != alias.c_str()) {
				CMStringW str = L"\r\n";
				str += GetWord(line, 0) + L" ";
				p3 = wcsstr(alias.c_str(), str.c_str());
				if (p3)
					p3 += 2;
			}
			if (p3 != nullptr) {
				hasAlias = true;
				const wchar_t* p4 = wcsstr(p3, L"\r\n");
				if (!p4)
					p4 = wcschr(p3, 0);

				*(wchar_t*)p4 = 0;
				CMStringW str = p3;
				str.Replace(L"##", window);
				str.Replace(L"$?", L"%question");

				wchar_t buf[5];
				for (int index = 1; index < 8; index++) {
					mir_snwprintf(buf, L"#$%u", index);
					if (!GetWord(line, index).IsEmpty() && IsChannel(GetWord(line, index)))
						str.Replace(buf, GetWord(line, index).c_str());
					else {
						CMStringW S1 = L"#";
						S1 += GetWord(line, index);
						str.Replace(buf, S1.c_str());
					}
				}
				for (int index2 = 1; index2 < 8; index2++) {
					mir_snwprintf(buf, L"$%u-", index2);
					str.Replace(buf, GetWordAddress(line, index2));
				}
				for (int index3 = 1; index3 < 8; index3++) {
					mir_snwprintf(buf, L"$%u", index3);
					str.Replace(buf, GetWord(line, index3).c_str());
				}
				Messageout += GetWordAddress(str.c_str(), 1);
			}
			else Messageout += line;
		}
		else Messageout += line;

		p1 = p2;
		if (*p1 == '\r')
			p1 += 2;
		p2 = wcsstr(p1, L"\r\n");
		if (!p2)
			p2 = wcschr(p1, 0);
		delete[] line;
		LinebreakFlag = true;
	}
	while (*p1 != 0);

	return hasAlias ? DoIdentifiers(Messageout, window) : Messageout;
}

CMStringW CIrcProto::DoIdentifiers(CMStringW text, const wchar_t*)
{
	SYSTEMTIME time;
	wchar_t str[2];

	GetLocalTime(&time);
	text.Replace(L"%mnick", m_nick);
	text.Replace(L"%anick", m_alternativeNick);
	text.Replace(L"%awaymsg", m_statusMessage.c_str());
	text.Replace(L"%module", _A2T(m_szModuleName));
	text.Replace(L"%name", m_name);
	text.Replace(L"%newl", L"\r\n");
	text.Replace(L"%network", _A2T(m_szModuleName));
	text.Replace(L"%me", m_info.sNick.c_str());

	char mirver[100];
	Miranda_GetVersionText(mirver, _countof(mirver));
	text.Replace(L"%mirver", _A2T(mirver));
	text.Replace(L"%version", _A2T(__VERSION_STRING_DOTS));

	str[0] = 3; str[1] = 0;
	text.Replace(L"%color", str);

	str[0] = 2;
	text.Replace(L"%bold", str);

	str[0] = 31;
	text.Replace(L"%underline", str);

	str[0] = 22;
	text.Replace(L"%italics", str);
	return text;
}

static void __stdcall sttSetTimerOn(void* _pro)
{
	CIrcProto *ppro = (CIrcProto*)_pro;
	ppro->DoEvent(GC_EVENT_INFORMATION, nullptr, ppro->m_info.sNick.c_str(), TranslateT("The buddy check function is enabled"), nullptr, nullptr, NULL, true, false);
	ppro->SetChatTimer(ppro->OnlineNotifTimer, 500, OnlineNotifTimerProc);
	if (ppro->m_channelAwayNotification)
		ppro->SetChatTimer(ppro->OnlineNotifTimer3, 1500, OnlineNotifTimerProc3);
}

static void __stdcall sttSetTimerOff(void* _pro)
{
	CIrcProto *ppro = (CIrcProto*)_pro;
	ppro->DoEvent(GC_EVENT_INFORMATION, nullptr, ppro->m_info.sNick.c_str(), TranslateT("The buddy check function is disabled"), nullptr, nullptr, NULL, true, false);
	ppro->KillChatTimer(ppro->OnlineNotifTimer);
	ppro->KillChatTimer(ppro->OnlineNotifTimer3);
}

BOOL CIrcProto::DoHardcodedCommand(CMStringW text, wchar_t *window, MCONTACT hContact)
{
	CMStringW command(GetWord(text, 0)); command.MakeLower();
	CMStringW one = GetWord(text, 1);
	CMStringW two = GetWord(text, 2);
	CMStringW three = GetWord(text, 3);
	CMStringW therest = GetWordAddress(text, 4);

	if (command == L"/servershow" || command == L"/serverhide") {
		if (m_useServer)
			Chat_Control(m_szModuleName, SERVERWINDOW, command == L"/servershow" ? WINDOW_VISIBLE : WINDOW_HIDDEN);

		return true;
	}

	else if (command == L"/sleep" || command == L"/wait") {
		if (!one.IsEmpty()) {
			int ms;
			if (swscanf(one.c_str(), L"%d", &ms) == 1 && ms > 0 && ms <= 4000)
				Sleep(ms);
			else
				DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), TranslateT("Incorrect parameters. Usage: /sleep [ms], ms should be greater than 0 and less than 4000."), nullptr, nullptr, NULL, true, false);
		}
		return true;
	}

	if (command == L"/clear") {
		CMStringW S;
		if (!one.IsEmpty()) {
			if (one == L"server")
				S = SERVERWINDOW;
			else
				S = one;
		}
		else S = window;

		Chat_Control(m_szModuleName, S, WINDOW_CLEARLOG);
		return true;
	}

	if (command == L"/ignore") {
		if (IsConnected()) {
			CMStringW IgnoreFlags;
			wchar_t temp[500];
			if (one.IsEmpty()) {
				if (m_ignore)
					DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), TranslateT("Ignore system is enabled"), nullptr, nullptr, NULL, true, false);
				else
					DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), TranslateT("Ignore system is disabled"), nullptr, nullptr, NULL, true, false);
				return true;
			}
			if (!mir_wstrcmpi(one.c_str(), L"on")) {
				m_ignore = 1;
				DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), TranslateT("Ignore system is enabled"), nullptr, nullptr, NULL, true, false);
				return true;
			}
			if (!mir_wstrcmpi(one.c_str(), L"off")) {
				m_ignore = 0;
				DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), TranslateT("Ignore system is disabled"), nullptr, nullptr, NULL, true, false);
				return true;
			}
			if (!wcschr(one.c_str(), '!') && !wcschr(one.c_str(), '@'))
				one += L"!*@*";

			if (!two.IsEmpty() && two[0] == '+') {
				if (wcschr(two.c_str(), 'q'))
					IgnoreFlags += 'q';
				if (wcschr(two.c_str(), 'n'))
					IgnoreFlags += 'n';
				if (wcschr(two.c_str(), 'i'))
					IgnoreFlags += 'i';
				if (wcschr(two.c_str(), 'd'))
					IgnoreFlags += 'd';
				if (wcschr(two.c_str(), 'c'))
					IgnoreFlags += 'c';
				if (wcschr(two.c_str(), 'm'))
					IgnoreFlags += 'm';
			}
			else IgnoreFlags = L"qnidc";

			AddIgnore(one.c_str(), IgnoreFlags.c_str());

			mir_snwprintf(temp, TranslateT("%s is now ignored (+%s)"), one.c_str(), IgnoreFlags.c_str());
			DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), temp, nullptr, nullptr, NULL, true, false);
		}
		return true;
	}

	if (command == L"/unignore") {
		if (!wcschr(one.c_str(), '!') && !wcschr(one.c_str(), '@'))
			one += L"!*@*";

		wchar_t temp[500];
		if (RemoveIgnore(one.c_str()))
			mir_snwprintf(temp, TranslateT("%s is not ignored now"), one.c_str());
		else
			mir_snwprintf(temp, TranslateT("%s was not ignored"), one.c_str());
		DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), temp, nullptr, nullptr, NULL, true, false);
		return true;
	}

	if (command == L"/userhost") {
		if (one.IsEmpty())
			return true;

		DoUserhostWithReason(1, L"U", false, command);
		return false;
	}

	if (command == L"/joinx") {
		if (!one.IsEmpty()) {
			for (int i = 1;; i++) {
				CMStringW tmp = GetWord(text, i);
				if (tmp.IsEmpty())
					break;

				AddToJTemp('X', tmp);
			}

			PostIrcMessage(L"/JOIN %s", GetWordAddress(text, 1));
		}
		return true;
	}

	if (command == L"/joinm") {
		if (!one.IsEmpty()) {
			for (int i = 1;; i++) {
				CMStringW tmp = GetWord(text, i);
				if (tmp.IsEmpty())
					break;

				AddToJTemp('M', tmp);
			}

			PostIrcMessage(L"/JOIN %s", GetWordAddress(text, 1));
		}
		return true;
	}

	if (command == L"/nusers") {
		wchar_t szTemp[40];
		GC_INFO gci = { 0 };
		gci.Flags = GCF_BYID | GCF_NAME | GCF_COUNT;
		gci.pszModule = m_szModuleName;
		gci.pszID = window;
		if (!Chat_GetInfo(&gci))
			mir_snwprintf(szTemp, L"users: %u", gci.iCount);

		DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), szTemp, nullptr, nullptr, NULL, true, false);
		return true;
	}

	if (command == L"/echo") {
		if (one.IsEmpty())
			return true;

		if (!mir_wstrcmpi(one.c_str(), L"on")) {
			bEcho = TRUE;
			DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), TranslateT("Outgoing commands are shown"), nullptr, nullptr, NULL, true, false);
		}

		if (!mir_wstrcmpi(one.c_str(), L"off")) {
			DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), TranslateT("Outgoing commands are not shown"), nullptr, nullptr, NULL, true, false);
			bEcho = FALSE;
		}

		return true;
	}

	if (command == L"/buddycheck") {
		if (one.IsEmpty()) {
			if ((m_autoOnlineNotification && !bTempDisableCheck) || bTempForceCheck)
				DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), TranslateT("The buddy check function is enabled"), nullptr, nullptr, NULL, true, false);
			else
				DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), TranslateT("The buddy check function is disabled"), nullptr, nullptr, NULL, true, false);
			return true;
		}
		if (!mir_wstrcmpi(one.c_str(), L"on")) {
			bTempForceCheck = true;
			bTempDisableCheck = false;
			CallFunctionAsync(sttSetTimerOn, this);
		}
		if (!mir_wstrcmpi(one.c_str(), L"off")) {
			bTempForceCheck = false;
			bTempDisableCheck = true;
			CallFunctionAsync(sttSetTimerOff, this);
		}
		if (!mir_wstrcmpi(one.c_str(), L"time") && !two.IsEmpty()) {
			m_iTempCheckTime = _wtoi(two.c_str());
			if (m_iTempCheckTime < 10 && m_iTempCheckTime != 0)
				m_iTempCheckTime = 10;

			if (m_iTempCheckTime == 0)
				DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), TranslateT("The time interval for the buddy check function is now at default setting"), nullptr, nullptr, NULL, true, false);
			else {
				wchar_t temp[200];
				mir_snwprintf(temp, TranslateT("The time interval for the buddy check function is now %u seconds"), m_iTempCheckTime);
				DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), temp, nullptr, nullptr, NULL, true, false);
			}
		}
		return true;
	}

	if (command == L"/whois") {
		if (one.IsEmpty())
			return false;
		m_manualWhoisCount++;
		return false;
	}

	if (command == L"/channelmanager") {
		if (window && !hContact && IsChannel(window)) {
			if (IsConnected()) {
				if (m_managerDlg != nullptr) {
					SetActiveWindow(m_managerDlg->GetHwnd());
					m_managerDlg->Close();
				}
				else {
					m_managerDlg = new CManagerDlg(this);
					m_managerDlg->Show();
					m_managerDlg->InitManager(1, window);
				}
			}
		}

		return true;
	}

	if (command == L"/who") {
		if (one.IsEmpty())
			return true;

		DoUserhostWithReason(2, L"U", false, L"%s", one.c_str());
		return false;
	}

	if (command == L"/hop") {
		if (!IsChannel(window))
			return true;

		PostIrcMessage(L"/PART %s", window);

		if ((one.IsEmpty() || !IsChannel(one))) {
			CHANNELINFO *wi = (CHANNELINFO *)Chat_GetUserInfo(m_szModuleName, window);
			if (wi && wi->pszPassword)
				PostIrcMessage(L"/JOIN %s %s", window, wi->pszPassword);
			else
				PostIrcMessage(L"/JOIN %s", window);
			return true;
		}

		Chat_Terminate(m_szModuleName, window);

		PostIrcMessage(L"/JOIN %s", GetWordAddress(text, 1));
		return true;
	}

	if (command == L"/list") {
		{
			mir_cslock lck(m_csList);
			if (m_listDlg == nullptr) {
				m_listDlg = new CListDlg(this);
				m_listDlg->Show();
			}
		}
		SetActiveWindow(m_listDlg->GetHwnd());
		int minutes = (int)m_noOfChannels / 4000;
		int minutes2 = (int)m_noOfChannels / 9000;

		CMStringW szMsg(FORMAT, TranslateT("This command is not recommended on a network of this size!\r\nIt will probably cause high CPU usage and/or high bandwidth\r\nusage for around %u to %u minute(s).\r\n\r\nDo you want to continue?"), minutes2, minutes);
		if (m_noOfChannels < 4000 || (m_noOfChannels >= 4000 && MessageBox(nullptr, szMsg, TranslateT("IRC warning"), MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDYES)) {
			ListView_DeleteAllItems(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW));
			PostIrcMessage(L"/lusers");
			return false;
		}
		m_listDlg->m_status.SetText(TranslateT("Aborted"));
		return true;
	}

	if (command == L"/me") {
		if (one.IsEmpty())
			return true;

		wchar_t szTemp[4000];
		mir_snwprintf(szTemp, L"\001ACTION %s\001", GetWordAddress(text.c_str(), 1));
		PostIrcMessageWnd(window, hContact, szTemp);
		return true;
	}

	if (command == L"/ame") {
		if (one.IsEmpty())
			return true;

		CMStringW S = L"/ME " + DoIdentifiers(GetWordAddress(text.c_str(), 1), window);
		S.Replace(L"%", L"%%");
		Chat_SendUserMessage(m_szModuleName, nullptr, S);
		return true;
	}

	if (command == L"/amsg") {
		if (one.IsEmpty())
			return true;

		CMStringW S = DoIdentifiers(GetWordAddress(text.c_str(), 1), window);
		S.Replace(L"%", L"%%");
		Chat_SendUserMessage(m_szModuleName, nullptr, S);
		return true;
	}

	if (command == L"/msg") {
		if (one.IsEmpty() || two.IsEmpty())
			return true;

		wchar_t szTemp[4000];
		mir_snwprintf(szTemp, L"/PRIVMSG %s", GetWordAddress(text.c_str(), 1));

		PostIrcMessageWnd(window, hContact, szTemp);
		return true;
	}

	if (command == L"/query") {
		if (one.IsEmpty() || IsChannel(one.c_str()))
			return true;

		CONTACT user = { one, nullptr, nullptr, false, false, false };
		MCONTACT hContact2 = CList_AddContact(&user, false, false);
		if (hContact2) {
			if (getByte(hContact, "AdvancedMode", 0) == 0)
				DoUserhostWithReason(1, (L"S" + one).c_str(), true, one.c_str());
			else {
				DBVARIANT dbv1;
				if (!getWString(hContact, "UWildcard", &dbv1)) {
					CMStringW S = L"S";
					S += dbv1.pwszVal;
					DoUserhostWithReason(2, S.c_str(), true, dbv1.pwszVal);
					db_free(&dbv1);
				}
				else {
					CMStringW S = L"S";
					S += one;
					DoUserhostWithReason(2, S.c_str(), true, one.c_str());
				}
			}

			CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact2, 0);
		}

		if (!two.IsEmpty()) {
			wchar_t szTemp[4000];
			mir_snwprintf(szTemp, L"/PRIVMSG %s", GetWordAddress(text.c_str(), 1));
			PostIrcMessageWnd(window, hContact, szTemp);
		}
		return true;
	}

	if (command == L"/ctcp") {
		if (one.IsEmpty() || two.IsEmpty())
			return true;

		wchar_t szTemp[1000];
		unsigned long ulAdr = 0;
		if (m_manualHost)
			ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
		else
			ulAdr = ConvertIPToInteger(m_IPFromServer ? m_myHost : m_myLocalHost);

		// if it is not dcc or if it is dcc and a local ip exist
		if (mir_wstrcmpi(two.c_str(), L"dcc") != 0 || ulAdr) {
			if (mir_wstrcmpi(two.c_str(), L"ping") == 0)
				mir_snwprintf(szTemp, L"/PRIVMSG %s \001%s %u\001", one.c_str(), two.c_str(), time(0));
			else
				mir_snwprintf(szTemp, L"/PRIVMSG %s \001%s\001", one.c_str(), GetWordAddress(text.c_str(), 2));
			PostIrcMessageWnd(window, hContact, szTemp);
		}

		if (mir_wstrcmpi(two.c_str(), L"dcc") != 0) {
			mir_snwprintf(szTemp, TranslateT("CTCP %s request sent to %s"), two.c_str(), one.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, m_info.sNick.c_str(), szTemp, nullptr, nullptr, NULL, true, false);
		}

		return true;
	}

	if (command == L"/dcc") {
		if (one.IsEmpty() || two.IsEmpty())
			return true;

		if (mir_wstrcmpi(one.c_str(), L"send") == 0) {
			wchar_t szTemp[1000];
			unsigned long ulAdr = 0;

			if (m_manualHost)
				ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
			else
				ulAdr = ConvertIPToInteger(m_IPFromServer ? m_myHost : m_myLocalHost);

			if (ulAdr) {
				CONTACT user = { two, nullptr, nullptr, false, false, true };
				MCONTACT ccNew = CList_AddContact(&user, false, false);
				if (ccNew) {
					CMStringW s;

					if (getByte(ccNew, "AdvancedMode", 0) == 0)
						DoUserhostWithReason(1, (L"S" + two).c_str(), true, two.c_str());
					else {
						DBVARIANT dbv1;
						CMStringW S = L"S";
						if (!getWString(ccNew, "UWildcard", &dbv1)) {
							S += dbv1.pwszVal;
							DoUserhostWithReason(2, S.c_str(), true, dbv1.pwszVal);
							db_free(&dbv1);
						}
						else {
							S += two;
							DoUserhostWithReason(2, S.c_str(), true, two.c_str());
						}
					}

					if (three.IsEmpty())
						CallService(MS_FILE_SENDFILE, ccNew, 0);
					else {
						CMStringW temp = GetWordAddress(text.c_str(), 3);
						wchar_t* pp[2];
						wchar_t* p = (wchar_t*)temp.c_str();
						pp[0] = p;
						pp[1] = nullptr;
						CallService(MS_FILE_SENDSPECIFICFILEST, ccNew, (LPARAM)pp);
					}
				}
			}
			else {
				mir_snwprintf(szTemp, TranslateT("DCC ERROR: Unable to automatically resolve external IP"));
				DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), szTemp, nullptr, nullptr, NULL, true, false);
			}
			return true;
		}

		if (mir_wstrcmpi(one.c_str(), L"chat") == 0) {
			wchar_t szTemp[1000];

			unsigned long ulAdr = 0;
			if (m_manualHost)
				ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
			else
				ulAdr = ConvertIPToInteger(m_IPFromServer ? m_myHost : m_myLocalHost);

			if (ulAdr) {
				CMStringW contact = two;  contact += DCCSTRING;
				CONTACT user = { contact, nullptr, nullptr, false, false, true };
				MCONTACT ccNew = CList_AddContact(&user, false, false);
				setByte(ccNew, "DCC", 1);

				int iPort = 0;
				if (ccNew) {
					DCCINFO* dci = new DCCINFO;
					dci->hContact = ccNew;
					dci->sContactName = two;
					dci->iType = DCC_CHAT;
					dci->bSender = true;

					CDccSession *dcc = new CDccSession(this, dci);
					CDccSession* olddcc = FindDCCSession(ccNew);
					if (olddcc)
						olddcc->Disconnect();
					AddDCCSession(ccNew, dcc);
					iPort = dcc->Connect();
				}

				if (iPort != 0) {
					PostIrcMessage(L"/CTCP %s DCC CHAT chat %u %u", two.c_str(), ulAdr, iPort);
					mir_snwprintf(szTemp, TranslateT("DCC CHAT request sent to %s"), two.c_str(), one.c_str());
					DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), szTemp, nullptr, nullptr, NULL, true, false);
				}
				else {
					mir_snwprintf(szTemp, TranslateT("DCC ERROR: Unable to bind port"));
					DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), szTemp, nullptr, nullptr, NULL, true, false);
				}
			}
			else {
				mir_snwprintf(szTemp, TranslateT("DCC ERROR: Unable to automatically resolve external IP"));
				DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick.c_str(), szTemp, nullptr, nullptr, NULL, true, false);
			}
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct DoInputRequestParam
{
	DoInputRequestParam(CIrcProto* _pro, const wchar_t* _str) :
	ppro(_pro),
	str(mir_wstrdup(_str))
	{}

	CIrcProto *ppro;
	wchar_t* str;
};

static void __stdcall DoInputRequestAliasApcStub(void* _par)
{
	DoInputRequestParam* param = (DoInputRequestParam*)_par;
	CIrcProto *ppro = param->ppro;
	wchar_t* str = param->str;

	wchar_t* infotext = nullptr;
	wchar_t* title = nullptr;
	wchar_t* defaulttext = nullptr;
	CMStringW command = (wchar_t*)str;
	wchar_t* p = wcsstr((wchar_t*)str, L"%question");
	if (p[9] == '=' && p[10] == '\"') {
		infotext = &p[11];
		p = wcschr(infotext, '\"');
		if (p) {
			*p = 0;
			p++;
			if (*p == ',' && p[1] == '\"') {
				p++; p++;
				title = p;
				p = wcschr(title, '\"');
				if (p) {
					*p = 0;
					p++;
					if (*p == ',' && p[1] == '\"') {
						p++; p++;
						defaulttext = p;
						p = wcschr(defaulttext, '\"');
						if (p)
							*p = 0;
					}
				}
			}
		}
	}

	CQuestionDlg* dlg = new CQuestionDlg(ppro);
	dlg->Show();
	HWND question_hWnd = dlg->GetHwnd();

	if (title)
		SetDlgItemText(question_hWnd, IDC_CAPTION, title);
	else
		SetDlgItemText(question_hWnd, IDC_CAPTION, TranslateT("Input command"));

	if (infotext)
		SetDlgItemText(question_hWnd, IDC_TEXT, infotext);
	else
		SetDlgItemText(question_hWnd, IDC_TEXT, TranslateT("Please enter the reply"));

	if (defaulttext)
		SetDlgItemText(question_hWnd, IDC_EDIT, defaulttext);

	SetDlgItemText(question_hWnd, IDC_HIDDENEDIT, command.c_str());
	dlg->Activate();

	mir_free(str);
	delete param;
}

bool CIrcProto::PostIrcMessage(const wchar_t* fmt, ...)
{
	if (!fmt || mir_wstrlen(fmt) < 1 || mir_wstrlen(fmt) > 4000)
		return 0;

	va_list marker;
	va_start(marker, fmt);
	static wchar_t szBuf[4 * 1024];
	mir_vsnwprintf(szBuf, _countof(szBuf), fmt, marker);
	va_end(marker);

	return PostIrcMessageWnd(nullptr, NULL, szBuf);
}

bool CIrcProto::PostIrcMessageWnd(wchar_t *window, MCONTACT hContact, const wchar_t *szBuf)
{
	DBVARIANT dbv;
	wchar_t windowname[256];
	uint8_t bDCC = 0;

	if (hContact)
		bDCC = getByte(hContact, "DCC", 0);

	if (!IsConnected() && !bDCC || !szBuf || mir_wstrlen(szBuf) < 1)
		return 0;

	if (hContact && !getWString(hContact, "Nick", &dbv)) {
		mir_wstrncpy(windowname, dbv.pwszVal, 255);
		db_free(&dbv);
	}
	else if (window)
		mir_wstrncpy(windowname, window, 255);
	else
		mir_wstrncpy(windowname, SERVERWINDOW, 255);

	if (mir_wstrcmpi(window, SERVERWINDOW) != 0) {
		wchar_t* p1 = wcschr(windowname, ' ');
		if (p1)
			*p1 = 0;
	}

	// remove unecessary linebreaks, and do the aliases
	CMStringW Message = szBuf;
	AddCR(Message);
	RemoveLinebreaks(Message);
	if (!hContact && IsConnected()) {
		Message = DoAlias(Message.c_str(), windowname);

		if (Message.Find(L"%question") != -1) {
			CallFunctionAsync(DoInputRequestAliasApcStub, new DoInputRequestParam(this, Message));
			return 1;
		}

		Message.Replace(L"%newl", L"\r\n");
		RemoveLinebreaks(Message);
	}

	if (Message.IsEmpty())
		return 0;

	CHANNELINFO *wi = (CHANNELINFO *)Chat_GetUserInfo(m_szModuleName, windowname);
	int cp = (wi) ? wi->codepage : getCodepage();

	// process the message
	while (!Message.IsEmpty()) {
		// split the text into lines, and do an automatic textsplit on long lies as well
		bool flag = false;
		CMStringW DoThis = L"";
		int index = Message.Find(L"\r\n", 0);
		if (index == -1)
			index = Message.GetLength();

		if (index > 464)
			index = 432;
		DoThis = Message.Mid(0, index);
		Message.Delete(0, index);
		if (Message.Find(L"\r\n", 0) == 0)
			Message.Delete(0, 2);

		//do this if it's a /raw
		if (IsConnected() && (GetWord(DoThis.c_str(), 0) == L"/raw" || GetWord(DoThis.c_str(), 0) == L"/quote")) {
			if (GetWord(DoThis.c_str(), 1).IsEmpty())
				continue;

			CMStringW S = GetWordAddress(DoThis.c_str(), 1);
			SendIrcMessage(S.c_str(), true, cp);
			continue;
		}

		// Do this if the message is not a command
		if ((GetWord(DoThis.c_str(), 0)[0] != '/') ||													// not a command
			((GetWord(DoThis.c_str(), 0)[0] == '/') && (GetWord(DoThis.c_str(), 0)[1] == '/')) ||		// or double backslash at the beginning
			hContact) {
			CMStringW S = L"/PRIVMSG ";
			if (mir_wstrcmpi(window, SERVERWINDOW) == 0 && !m_info.sServerName.IsEmpty())
				S += m_info.sServerName + L" " + DoThis;
			else
				S += CMStringW(windowname) + L" " + DoThis;

			DoThis = S;
			flag = true;
		}

		// and here we send it unless the command was a hardcoded one that should not be sent
		if (DoHardcodedCommand(DoThis, windowname, hContact))
			continue;

		if (!IsConnected() && !bDCC)
			continue;

		if (!flag && IsConnected())
			DoThis = DoIdentifiers(DoThis, windowname);

		if (hContact) {
			if (flag && bDCC) {
				CDccSession *dcc = FindDCCSession(hContact);
				if (dcc) {
					FormatMsg(DoThis);
					CMStringW mess = GetWordAddress(DoThis.c_str(), 2);
					if (mess[0] == ':')
						mess.Delete(0, 1);
					mess += '\n';
					dcc->SendStuff(mess.c_str());
				}
			}
			else if (IsConnected()) {
				FormatMsg(DoThis);
				SendIrcMessage(DoThis.c_str(), false, cp);
			}
		}
		else {
			FormatMsg(DoThis);
			SendIrcMessage(DoThis.c_str(), true, cp);
		}
	}

	return 1;
}
