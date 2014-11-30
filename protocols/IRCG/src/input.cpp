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

#include "irc.h"
#include "version.h"

#define NICKSUBSTITUTE _T("!_nick_!")

void CIrcProto::FormatMsg(CMString& text)
{
	TCHAR temp[30];
	mir_tstrncpy(temp, GetWord(text.c_str(), 0).c_str(), 29);
	CharLower(temp);
	CMString command = temp;
	CMString S = _T("");
	if (command == _T("/quit") || command == _T("/away"))
		S = GetWord(text.c_str(), 0) + _T(" :") + GetWordAddress(text.c_str(), 1);
	else if (command == _T("/privmsg") || command == _T("/part") || command == _T("/topic") || command == _T("/notice")) {
		S = GetWord(text.c_str(), 0) + _T(" ") + GetWord(text.c_str(), 1) + _T(" :");
		if (!GetWord(text.c_str(), 2).IsEmpty())
			S += CMString(GetWordAddress(text.c_str(), 2));
	}
	else if (command == _T("/kick")) {
		S = GetWord(text.c_str(), 0) + _T(" ") + GetWord(text.c_str(), 1) + _T(" ") + GetWord(text.c_str(), 2) + _T(" :") + GetWordAddress(text.c_str(), 3);
	}
	else if (command == _T("/nick")) {
		if (!_tcsstr(GetWord(text.c_str(), 1).c_str(), NICKSUBSTITUTE)) {
			sNick4Perform = GetWord(text.c_str(), 1);
			S = GetWordAddress(text.c_str(), 0);
		}
		else {
			CMString sNewNick = GetWord(text.c_str(), 1);
			if (sNick4Perform == _T("")) {
				DBVARIANT dbv;
				if (!getTString("PNick", &dbv)) {
					sNick4Perform = dbv.ptszVal;
					db_free(&dbv);
				}
			}

			sNewNick.Replace(NICKSUBSTITUTE, sNick4Perform.c_str());
			S = GetWord(text.c_str(), 0) + _T(" ") + sNewNick;
		}
	}
	else S = GetWordAddress(text.c_str(), 0);

	S.Delete(0, 1);
	text = S;
}

static void AddCR(CMString& text)
{
	text.Replace(_T("\n"), _T("\r\n"));
	text.Replace(_T("\r\r"), _T("\r"));
}

CMString CIrcProto::DoAlias(const TCHAR *text, TCHAR *window)
{
	CMString Messageout = _T("");
	const TCHAR* p1 = text;
	const TCHAR* p2 = text;
	bool LinebreakFlag = false, hasAlias = false;
	p2 = _tcsstr(p1, _T("\r\n"));
	if (!p2)
		p2 = _tcschr(p1, '\0');
	if (p1 == p2)
		return (CMString)text;

	do {
		if (LinebreakFlag)
			Messageout += _T("\r\n");

		TCHAR* line = new TCHAR[p2 - p1 + 1];
		mir_tstrncpy(line, p1, p2 - p1 + 1);
		TCHAR* test = line;
		while (*test == ' ')
			test++;
		if (*test == '/') {
			mir_tstrncpy(line, GetWordAddress(line, 0), p2 - p1 + 1);
			CMString S = line;
			delete[] line;
			line = new TCHAR[S.GetLength() + 2];
			mir_tstrncpy(line, S.c_str(), S.GetLength() + 1);
			CMString alias(m_alias);
			const TCHAR* p3 = _tcsstr(alias.c_str(), (GetWord(line, 0) + _T(" ")).c_str());
			if (p3 != alias.c_str()) {
				CMString S = _T("\r\n");
				S += GetWord(line, 0) + _T(" ");
				p3 = _tcsstr(alias.c_str(), S.c_str());
				if (p3)
					p3 += 2;
			}
			if (p3 != NULL) {
				hasAlias = true;
				const TCHAR* p4 = _tcsstr(p3, _T("\r\n"));
				if (!p4)
					p4 = _tcschr(p3, '\0');

				*(TCHAR*)p4 = 0;
				CMString S = p3;
				S.Replace(_T("##"), window);
				S.Replace(_T("$?"), _T("%question"));

				for (int index = 1; index < 8; index++) {
					TCHAR str[5];
					mir_sntprintf(str, SIZEOF(str), _T("#$%u"), index);
					if (!GetWord(line, index).IsEmpty() && IsChannel(GetWord(line, index)))
						S.Replace(str, GetWord(line, index).c_str());
					else {
						CMString S1 = _T("#");
						S1 += GetWord(line, index);
						S.Replace(str, S1.c_str());
					}
				}
				for (int index2 = 1; index2 < 8; index2++) {
					TCHAR str[5];
					mir_sntprintf(str, SIZEOF(str), _T("$%u-"), index2);
					S.Replace(str, GetWordAddress(line, index2));
				}
				for (int index3 = 1; index3 < 8; index3++) {
					TCHAR str[5];
					mir_sntprintf(str, SIZEOF(str), _T("$%u"), index3);
					S.Replace(str, GetWord(line, index3).c_str());
				}
				Messageout += GetWordAddress(S.c_str(), 1);
			}
			else Messageout += line;
		}
		else Messageout += line;

		p1 = p2;
		if (*p1 == '\r')
			p1 += 2;
		p2 = _tcsstr(p1, _T("\r\n"));
		if (!p2)
			p2 = _tcschr(p1, '\0');
		delete[] line;
		LinebreakFlag = true;
	}
	while (*p1 != '\0');

	return hasAlias ? DoIdentifiers(Messageout, window) : Messageout;
}

CMString CIrcProto::DoIdentifiers(CMString text, const TCHAR*)
{
	SYSTEMTIME time;
	TCHAR str[2];

	GetLocalTime(&time);
	text.Replace(_T("%mnick"), m_nick);
	text.Replace(_T("%anick"), m_alternativeNick);
	text.Replace(_T("%awaymsg"), m_statusMessage.c_str());
	text.Replace(_T("%module"), _A2T(m_szModuleName));
	text.Replace(_T("%name"), m_name);
	text.Replace(_T("%newl"), _T("\r\n"));
	text.Replace(_T("%network"), m_info.sNetwork.c_str());
	text.Replace(_T("%me"), m_info.sNick.c_str());

	char mirver[100];
	CallService(MS_SYSTEM_GETVERSIONTEXT, SIZEOF(mirver), LPARAM(mirver));
	text.Replace(_T("%mirver"), _A2T(mirver));

	text.Replace(_T("%version"), _T(__VERSION_STRING_DOTS));

	str[0] = 3; str[1] = '\0';
	text.Replace(_T("%color"), str);

	str[0] = 2;
	text.Replace(_T("%bold"), str);

	str[0] = 31;
	text.Replace(_T("%underline"), str);

	str[0] = 22;
	text.Replace(_T("%italics"), str);
	return text;
}

static void __stdcall sttSetTimerOn(void* _pro)
{
	CIrcProto *ppro = (CIrcProto*)_pro;
	ppro->DoEvent(GC_EVENT_INFORMATION, NULL, ppro->m_info.sNick.c_str(), TranslateT("The buddy check function is enabled"), NULL, NULL, NULL, true, false);
	ppro->SetChatTimer(ppro->OnlineNotifTimer, 500, OnlineNotifTimerProc);
	if (ppro->m_channelAwayNotification)
		ppro->SetChatTimer(ppro->OnlineNotifTimer3, 1500, OnlineNotifTimerProc3);
}

static void __stdcall sttSetTimerOff(void* _pro)
{
	CIrcProto *ppro = (CIrcProto*)_pro;
	ppro->DoEvent(GC_EVENT_INFORMATION, NULL, ppro->m_info.sNick.c_str(), TranslateT("The buddy check function is disabled"), NULL, NULL, NULL, true, false);
	ppro->KillChatTimer(ppro->OnlineNotifTimer);
	ppro->KillChatTimer(ppro->OnlineNotifTimer3);
}

BOOL CIrcProto::DoHardcodedCommand(CMString text, TCHAR* window, MCONTACT hContact)
{
	TCHAR temp[30];
	mir_tstrncpy(temp, GetWord(text.c_str(), 0).c_str(), 29);
	CharLower(temp);
	CMString command = temp;
	CMString one = GetWord(text.c_str(), 1);
	CMString two = GetWord(text.c_str(), 2);
	CMString three = GetWord(text.c_str(), 3);
	CMString therest = GetWordAddress(text.c_str(), 4);

	if (command == _T("/servershow") || command == _T("/serverhide")) {
		if (m_useServer) {
			GCDEST gcd = { m_szModuleName, SERVERWINDOW, GC_EVENT_CONTROL };
			GCEVENT gce = { sizeof(gce), &gcd };
			CallChatEvent(command == _T("/servershow") ? WINDOW_VISIBLE : WINDOW_HIDDEN, (LPARAM)&gce);
		}
		return true;
	}

	else if (command == _T("/sleep") || command == _T("/wait")) {
		if (!one.IsEmpty()) {
			int ms;
			if (_stscanf(one.c_str(), _T("%d"), &ms) == 1 && ms > 0 && ms <= 4000)
				Sleep(ms);
			else
				DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), TranslateT("Incorrect parameters. Usage: /sleep [ms], ms should be greater than 0 and less than 4000."), NULL, NULL, NULL, true, false);
		}
		return true;
	}

	if (command == _T("/clear")) {
		CMString S;
		if (!one.IsEmpty()) {
			if (one == _T("server"))
				S = SERVERWINDOW;
			else
				S = MakeWndID(one.c_str());
		}
		else if (lstrcmpi(window, SERVERWINDOW) == 0)
			S = window;
		else
			S = MakeWndID(window);

		GCDEST gcd = { m_szModuleName, S.c_str(), GC_EVENT_CONTROL };
		GCEVENT gce = { sizeof(gce), &gcd };
		CallChatEvent(WINDOW_CLEARLOG, (LPARAM)&gce);
		return true;
	}

	if (command == _T("/ignore")) {
		if (IsConnected()) {
			CMString IgnoreFlags;
			TCHAR temp[500];
			if (one.IsEmpty()) {
				if (m_ignore)
					DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), TranslateT("Ignore system is enabled"), NULL, NULL, NULL, true, false);
				else
					DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), TranslateT("Ignore system is disabled"), NULL, NULL, NULL, true, false);
				return true;
			}
			if (!lstrcmpi(one.c_str(), _T("on"))) {
				m_ignore = 1;
				DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), TranslateT("Ignore system is enabled"), NULL, NULL, NULL, true, false);
				return true;
			}
			if (!lstrcmpi(one.c_str(), _T("off"))) {
				m_ignore = 0;
				DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), TranslateT("Ignore system is disabled"), NULL, NULL, NULL, true, false);
				return true;
			}
			if (!_tcschr(one.c_str(), '!') && !_tcschr(one.c_str(), '@'))
				one += _T("!*@*");

			if (!two.IsEmpty() && two[0] == '+') {
				if (_tcschr(two.c_str(), 'q'))
					IgnoreFlags += 'q';
				if (_tcschr(two.c_str(), 'n'))
					IgnoreFlags += 'n';
				if (_tcschr(two.c_str(), 'i'))
					IgnoreFlags += 'i';
				if (_tcschr(two.c_str(), 'd'))
					IgnoreFlags += 'd';
				if (_tcschr(two.c_str(), 'c'))
					IgnoreFlags += 'c';
				if (_tcschr(two.c_str(), 'm'))
					IgnoreFlags += 'm';
			}
			else IgnoreFlags = _T("qnidc");

			CMString m_network;
			if (three.IsEmpty())
				m_network = m_info.sNetwork;
			else
				m_network = three;

			AddIgnore(one.c_str(), IgnoreFlags.c_str(), m_network.c_str());

			mir_sntprintf(temp, SIZEOF(temp), TranslateT("%s on %s is now ignored (+%s)"), one.c_str(), m_network.c_str(), IgnoreFlags.c_str());
			DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), temp, NULL, NULL, NULL, true, false);
		}
		return true;
	}

	if (command == _T("/unignore")) {
		if (!_tcschr(one.c_str(), '!') && !_tcschr(one.c_str(), '@'))
			one += _T("!*@*");

		TCHAR temp[500];
		if (RemoveIgnore(one.c_str()))
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("%s is not ignored now"), one.c_str());
		else
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("%s was not ignored"), one.c_str());
		DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), temp, NULL, NULL, NULL, true, false);
		return true;
	}

	if (command == _T("/userhost")) {
		if (one.IsEmpty())
			return true;

		DoUserhostWithReason(1, _T("U"), false, temp);
		return false;
	}

	if (command == _T("/joinx")) {
		if (!one.IsEmpty()) {
			for (int i = 1;; i++) {
				CMString tmp = GetWord(text.c_str(), i);
				if (tmp.IsEmpty())
					break;

				AddToJTemp('X', tmp);
			}

			PostIrcMessage(_T("/JOIN %s"), GetWordAddress(text.c_str(), 1));
		}
		return true;
	}

	if (command == _T("/joinm")) {
		if (!one.IsEmpty()) {
			for (int i = 1;; i++) {
				CMString tmp = GetWord(text.c_str(), i);
				if (tmp.IsEmpty())
					break;

				AddToJTemp('M', tmp);
			}

			PostIrcMessage(_T("/JOIN %s"), GetWordAddress(text.c_str(), 1));
		}
		return true;
	}

	if (command == _T("/nusers")) {
		TCHAR szTemp[40];
		CMString S = MakeWndID(window);
		GC_INFO gci = { 0 };
		gci.Flags = GCF_BYID | GCF_NAME | GCF_COUNT;
		gci.pszModule = m_szModuleName;
		gci.pszID = S.c_str();
		if (!CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci))
			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("users: %u"), gci.iCount);

		DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);
		return true;
	}

	if (command == _T("/echo")) {
		if (one.IsEmpty())
			return true;

		if (!lstrcmpi(one.c_str(), _T("on"))) {
			bEcho = TRUE;
			DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), TranslateT("Outgoing commands are shown"), NULL, NULL, NULL, true, false);
		}

		if (!lstrcmpi(one.c_str(), _T("off"))) {
			DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), TranslateT("Outgoing commands are not shown"), NULL, NULL, NULL, true, false);
			bEcho = FALSE;
		}

		return true;
	}

	if (command == _T("/buddycheck")) {
		if (one.IsEmpty()) {
			if ((m_autoOnlineNotification && !bTempDisableCheck) || bTempForceCheck)
				DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), TranslateT("The buddy check function is enabled"), NULL, NULL, NULL, true, false);
			else
				DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), TranslateT("The buddy check function is disabled"), NULL, NULL, NULL, true, false);
			return true;
		}
		if (!lstrcmpi(one.c_str(), _T("on"))) {
			bTempForceCheck = true;
			bTempDisableCheck = false;
			CallFunctionAsync(sttSetTimerOn, this);
		}
		if (!lstrcmpi(one.c_str(), _T("off"))) {
			bTempForceCheck = false;
			bTempDisableCheck = true;
			CallFunctionAsync(sttSetTimerOff, this);
		}
		if (!lstrcmpi(one.c_str(), _T("time")) && !two.IsEmpty()) {
			m_iTempCheckTime = StrToInt(two.c_str());
			if (m_iTempCheckTime < 10 && m_iTempCheckTime != 0)
				m_iTempCheckTime = 10;

			if (m_iTempCheckTime == 0)
				DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), TranslateT("The time interval for the buddy check function is now at default setting"), NULL, NULL, NULL, true, false);
			else {
				TCHAR temp[200];
				mir_sntprintf(temp, SIZEOF(temp), TranslateT("The time interval for the buddy check function is now %u seconds"), m_iTempCheckTime);
				DoEvent(GC_EVENT_INFORMATION, NULL, m_info.sNick.c_str(), temp, NULL, NULL, NULL, true, false);
			}
		}
		return true;
	}

	if (command == _T("/whois")) {
		if (one.IsEmpty())
			return false;
		m_manualWhoisCount++;
		return false;
	}

	if (command == _T("/channelmanager")) {
		if (window && !hContact && IsChannel(window)) {
			if (IsConnected()) {
				if (m_managerDlg != NULL) {
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

	if (command == _T("/who")) {
		if (one.IsEmpty())
			return true;

		DoUserhostWithReason(2, _T("U"), false, _T("%s"), one.c_str());
		return false;
	}

	if (command == _T("/hop")) {
		if (!IsChannel(window))
			return true;

		PostIrcMessage(_T("/PART %s"), window);

		if ((one.IsEmpty() || !IsChannel(one))) {
			CHANNELINFO *wi = (CHANNELINFO *)DoEvent(GC_EVENT_GETITEMDATA, window, NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, 0);
			if (wi && wi->pszPassword)
				PostIrcMessage(_T("/JOIN %s %s"), window, wi->pszPassword);
			else
				PostIrcMessage(_T("/JOIN %s"), window);
			return true;
		}

		CMString S = MakeWndID(window);
		GCDEST gcd = { m_szModuleName, S.c_str(), GC_EVENT_CONTROL };
		GCEVENT gce = { sizeof(gce), &gcd };
		CallChatEvent(SESSION_TERMINATE, (LPARAM)&gce);

		PostIrcMessage(_T("/JOIN %s"), GetWordAddress(text.c_str(), 1));
		return true;
	}

	if (command == _T("/list")) {
		if (m_listDlg == NULL) {
			m_listDlg = new CListDlg(this);
			m_listDlg->Show();
		}
		SetActiveWindow(m_listDlg->GetHwnd());
		int minutes = (int)m_noOfChannels / 4000;
		int minutes2 = (int)m_noOfChannels / 9000;

		TCHAR text[256];
		mir_sntprintf(text, SIZEOF(text), TranslateT("This command is not recommended on a network of this size!\r\nIt will probably cause high CPU usage and/or high bandwidth\r\nusage for around %u to %u minute(s).\r\n\r\nDo you want to continue?"), minutes2, minutes);
		if (m_noOfChannels < 4000 || (m_noOfChannels >= 4000 && MessageBox(NULL, text, TranslateT("IRC warning"), MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) == IDYES)) {
			ListView_DeleteAllItems(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW));
			PostIrcMessage(_T("/lusers"));
			return false;
		}
		m_listDlg->m_status.SetText(TranslateT("Aborted"));
		return true;
	}

	if (command == _T("/me")) {
		if (one.IsEmpty())
			return true;

		TCHAR szTemp[4000];
		mir_sntprintf(szTemp, SIZEOF(szTemp), _T("\001ACTION %s\001"), GetWordAddress(text.c_str(), 1));
		PostIrcMessageWnd(window, hContact, szTemp);
		return true;
	}

	if (command == _T("/ame")) {
		if (one.IsEmpty())
			return true;

		CMString S = _T("/ME ") + DoIdentifiers(GetWordAddress(text.c_str(), 1), window);
		S.Replace(_T("%"), _T("%%"));
		DoEvent(GC_EVENT_SENDMESSAGE, NULL, NULL, S.c_str(), NULL, NULL, NULL, FALSE, FALSE);
		return true;
	}

	if (command == _T("/amsg")) {
		if (one.IsEmpty())
			return true;

		CMString S = DoIdentifiers(GetWordAddress(text.c_str(), 1), window);
		S.Replace(_T("%"), _T("%%"));
		DoEvent(GC_EVENT_SENDMESSAGE, NULL, NULL, S.c_str(), NULL, NULL, NULL, FALSE, FALSE);
		return true;
	}

	if (command == _T("/msg")) {
		if (one.IsEmpty() || two.IsEmpty())
			return true;

		TCHAR szTemp[4000];
		mir_sntprintf(szTemp, SIZEOF(szTemp), _T("/PRIVMSG %s"), GetWordAddress(text.c_str(), 1));

		PostIrcMessageWnd(window, hContact, szTemp);
		return true;
	}

	if (command == _T("/query")) {
		if (one.IsEmpty() || IsChannel(one.c_str()))
			return true;

		CONTACT user = { (TCHAR*)one.c_str(), NULL, NULL, false, false, false };
		MCONTACT hContact2 = CList_AddContact(&user, false, false);
		if (hContact2) {
			if (getByte(hContact, "AdvancedMode", 0) == 0)
				DoUserhostWithReason(1, (_T("S") + one).c_str(), true, one.c_str());
			else {
				DBVARIANT dbv1;
				if (!getTString(hContact, "UWildcard", &dbv1)) {
					CMString S = _T("S");
					S += dbv1.ptszVal;
					DoUserhostWithReason(2, S.c_str(), true, dbv1.ptszVal);
					db_free(&dbv1);
				}
				else {
					CMString S = _T("S");
					S += one;
					DoUserhostWithReason(2, S.c_str(), true, one.c_str());
				}
			}

			CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact2, 0);
		}

		if (!two.IsEmpty()) {
			TCHAR szTemp[4000];
			mir_sntprintf(szTemp, SIZEOF(szTemp), _T("/PRIVMSG %s"), GetWordAddress(text.c_str(), 1));
			PostIrcMessageWnd(window, hContact, szTemp);
		}
		return true;
	}

	if (command == _T("/ctcp")) {
		if (one.IsEmpty() || two.IsEmpty())
			return true;

		TCHAR szTemp[1000];
		unsigned long ulAdr = 0;
		if (m_manualHost)
			ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
		else
			ulAdr = ConvertIPToInteger(m_IPFromServer ? m_myHost : m_myLocalHost);

		// if it is not dcc or if it is dcc and a local ip exist
		if (lstrcmpi(two.c_str(), _T("dcc")) != 0 || ulAdr) {
			if (lstrcmpi(two.c_str(), _T("ping")) == 0)
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("/PRIVMSG %s \001%s %u\001"), one.c_str(), two.c_str(), time(0));
			else
				mir_sntprintf(szTemp, SIZEOF(szTemp), _T("/PRIVMSG %s \001%s\001"), one.c_str(), GetWordAddress(text.c_str(), 2));
			PostIrcMessageWnd(window, hContact, szTemp);
		}

		if (lstrcmpi(two.c_str(), _T("dcc")) != 0) {
			mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("CTCP %s request sent to %s"), two.c_str(), one.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);
		}

		return true;
	}

	if (command == _T("/dcc")) {
		if (one.IsEmpty() || two.IsEmpty())
			return true;

		if (lstrcmpi(one.c_str(), _T("send")) == 0) {
			TCHAR szTemp[1000];
			unsigned long ulAdr = 0;

			if (m_manualHost)
				ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
			else
				ulAdr = ConvertIPToInteger(m_IPFromServer ? m_myHost : m_myLocalHost);

			if (ulAdr) {
				CONTACT user = { (TCHAR*)two.c_str(), NULL, NULL, false, false, true };
				MCONTACT hContact = CList_AddContact(&user, false, false);
				if (hContact) {
					CMString s;

					if (getByte(hContact, "AdvancedMode", 0) == 0)
						DoUserhostWithReason(1, (_T("S") + two).c_str(), true, two.c_str());
					else {
						DBVARIANT dbv1;
						CMString S = _T("S");
						if (!getTString(hContact, "UWildcard", &dbv1)) {
							S += dbv1.ptszVal;
							DoUserhostWithReason(2, S.c_str(), true, dbv1.ptszVal);
							db_free(&dbv1);
						}
						else {
							S += two;
							DoUserhostWithReason(2, S.c_str(), true, two.c_str());
						}
					}

					if (three.IsEmpty())
						CallService(MS_FILE_SENDFILE, hContact, 0);
					else {
						CMString temp = GetWordAddress(text.c_str(), 3);
						TCHAR* pp[2];
						TCHAR* p = (TCHAR*)temp.c_str();
						pp[0] = p;
						pp[1] = NULL;
						CallService(MS_FILE_SENDSPECIFICFILEST, hContact, (LPARAM)pp);
					}
				}
			}
			else {
				mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC ERROR: Unable to automatically resolve external IP"));
				DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);
			}
			return true;
		}

		if (lstrcmpi(one.c_str(), _T("chat")) == 0) {
			TCHAR szTemp[1000];

			unsigned long ulAdr = 0;
			if (m_manualHost)
				ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
			else
				ulAdr = ConvertIPToInteger(m_IPFromServer ? m_myHost : m_myLocalHost);

			if (ulAdr) {
				CMString contact = two;  contact += DCCSTRING;
				CONTACT user = { (TCHAR*)contact.c_str(), NULL, NULL, false, false, true };
				MCONTACT hContact = CList_AddContact(&user, false, false);
				setByte(hContact, "DCC", 1);

				int iPort = 0;
				if (hContact) {
					DCCINFO* dci = new DCCINFO;
					dci->hContact = hContact;
					dci->sContactName = two;
					dci->iType = DCC_CHAT;
					dci->bSender = true;

					CDccSession* dcc = new CDccSession(this, dci);
					CDccSession* olddcc = FindDCCSession(hContact);
					if (olddcc)
						olddcc->Disconnect();
					AddDCCSession(hContact, dcc);
					iPort = dcc->Connect();
				}

				if (iPort != 0) {
					PostIrcMessage(_T("/CTCP %s DCC CHAT chat %u %u"), two.c_str(), ulAdr, iPort);
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC CHAT request sent to %s"), two.c_str(), one.c_str());
					DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);
				}
				else {
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC ERROR: Unable to bind port"));
					DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);
				}
			}
			else {
				mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC ERROR: Unable to automatically resolve external IP"));
				DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);
			}
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

struct DoInputRequestParam
{
	DoInputRequestParam(CIrcProto* _pro, const TCHAR* _str) :
	ppro(_pro),
	str(mir_tstrdup(_str))
	{}

	CIrcProto *ppro;
	TCHAR* str;
};

static void __stdcall DoInputRequestAliasApcStub(void* _par)
{
	DoInputRequestParam* param = (DoInputRequestParam*)_par;
	CIrcProto *ppro = param->ppro;
	TCHAR* str = param->str;

	TCHAR* infotext = NULL;
	TCHAR* title = NULL;
	TCHAR* defaulttext = NULL;
	CMString command = (TCHAR*)str;
	TCHAR* p = _tcsstr((TCHAR*)str, _T("%question"));
	if (p[9] == '=' && p[10] == '\"') {
		infotext = &p[11];
		p = _tcschr(infotext, '\"');
		if (p) {
			*p = '\0';
			p++;
			if (*p == ',' && p[1] == '\"') {
				p++; p++;
				title = p;
				p = _tcschr(title, '\"');
				if (p) {
					*p = '\0';
					p++;
					if (*p == ',' && p[1] == '\"') {
						p++; p++;
						defaulttext = p;
						p = _tcschr(defaulttext, '\"');
						if (p)
							*p = '\0';
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
		SetWindowText(GetDlgItem(question_hWnd, IDC_TEXT), infotext);
	else
		SetWindowText(GetDlgItem(question_hWnd, IDC_TEXT), TranslateT("Please enter the reply"));

	if (defaulttext)
		SetWindowText(GetDlgItem(question_hWnd, IDC_EDIT), defaulttext);

	SetDlgItemText(question_hWnd, IDC_HIDDENEDIT, command.c_str());
	dlg->Activate();

	mir_free(str);
	delete param;
}

bool CIrcProto::PostIrcMessage(const TCHAR* fmt, ...)
{
	if (!fmt || mir_tstrlen(fmt) < 1 || mir_tstrlen(fmt) > 4000)
		return 0;

	va_list marker;
	va_start(marker, fmt);
	static TCHAR szBuf[4 * 1024];
	mir_vsntprintf(szBuf, SIZEOF(szBuf), fmt, marker);
	va_end(marker);

	return PostIrcMessageWnd(NULL, NULL, szBuf);
}

bool CIrcProto::PostIrcMessageWnd(TCHAR* window, MCONTACT hContact, const TCHAR* szBuf)
{
	DBVARIANT dbv;
	TCHAR windowname[256];
	BYTE bDCC = 0;

	if (hContact)
		bDCC = getByte(hContact, "DCC", 0);

	if (!IsConnected() && !bDCC || !szBuf || mir_tstrlen(szBuf) < 1)
		return 0;

	if (hContact && !getTString(hContact, "Nick", &dbv)) {
		mir_tstrncpy(windowname, dbv.ptszVal, 255);
		db_free(&dbv);
	}
	else if (window)
		mir_tstrncpy(windowname, window, 255);
	else
		mir_tstrncpy(windowname, SERVERWINDOW, 255);

	if (lstrcmpi(window, SERVERWINDOW) != 0) {
		TCHAR* p1 = _tcschr(windowname, ' ');
		if (p1)
			*p1 = '\0';
	}

	// remove unecessary linebreaks, and do the aliases
	CMString Message = szBuf;
	AddCR(Message);
	RemoveLinebreaks(Message);
	if (!hContact && IsConnected()) {
		Message = DoAlias(Message.c_str(), windowname);

		if (Message.Find(_T("%question")) != -1) {
			CallFunctionAsync(DoInputRequestAliasApcStub, new DoInputRequestParam(this, Message));
			return 1;
		}

		Message.Replace(_T("%newl"), _T("\r\n"));
		RemoveLinebreaks(Message);
	}

	if (Message.IsEmpty())
		return 0;

	CHANNELINFO *wi = (CHANNELINFO *)DoEvent(GC_EVENT_GETITEMDATA, windowname, NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, 0);
	int codepage = (wi) ? wi->codepage : getCodepage();

	// process the message
	while (!Message.IsEmpty()) {
		// split the text into lines, and do an automatic textsplit on long lies as well
		bool flag = false;
		CMString DoThis = _T("");
		int index = Message.Find(_T("\r\n"), 0);
		if (index == -1)
			index = Message.GetLength();

		if (index > 464)
			index = 432;
		DoThis = Message.Mid(0, index);
		Message.Delete(0, index);
		if (Message.Find(_T("\r\n"), 0) == 0)
			Message.Delete(0, 2);

		//do this if it's a /raw
		if (IsConnected() && (GetWord(DoThis.c_str(), 0) == _T("/raw") || GetWord(DoThis.c_str(), 0) == _T("/quote"))) {
			if (GetWord(DoThis.c_str(), 1).IsEmpty())
				continue;

			CMString S = GetWordAddress(DoThis.c_str(), 1);
			SendIrcMessage(S.c_str(), true, codepage);
			continue;
		}

		// Do this if the message is not a command
		if ((GetWord(DoThis.c_str(), 0)[0] != '/') ||													// not a command
			((GetWord(DoThis.c_str(), 0)[0] == '/') && (GetWord(DoThis.c_str(), 0)[1] == '/')) ||		// or double backslash at the beginning
			hContact) {
			CMString S = _T("/PRIVMSG ");
			if (lstrcmpi(window, SERVERWINDOW) == 0 && !m_info.sServerName.IsEmpty())
				S += m_info.sServerName + _T(" ") + DoThis;
			else
				S += CMString(windowname) + _T(" ") + DoThis;

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
				CDccSession* dcc = FindDCCSession(hContact);
				if (dcc) {
					FormatMsg(DoThis);
					CMString mess = GetWordAddress(DoThis.c_str(), 2);
					if (mess[0] == ':')
						mess.Delete(0, 1);
					mess += '\n';
					dcc->SendStuff(mess.c_str());
				}
			}
			else if (IsConnected()) {
				FormatMsg(DoThis);
				SendIrcMessage(DoThis.c_str(), false, codepage);
			}
		}
		else {
			FormatMsg(DoThis);
			SendIrcMessage(DoThis.c_str(), true, codepage);
		}
	}

	return 1;
}
