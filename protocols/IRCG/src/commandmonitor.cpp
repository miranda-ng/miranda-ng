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

// This file holds functions that are called upon receiving
// certain commands from the server.

#include "stdafx.h"
#include "version.h"

using namespace irc;

VOID CALLBACK IdentTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CIrcProto *ppro = GetTimerOwner(idEvent);
	if (ppro == nullptr)
		return;

	ppro->KillChatTimer(ppro->IdentTimer);
	if (ppro->m_iStatus == ID_STATUS_OFFLINE || ppro->m_iStatus == ID_STATUS_CONNECTING)
		return;

	if (ppro->IsConnected() && ppro->m_identTimer)
		ppro->KillIdent();
}

VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CIrcProto *ppro = GetTimerOwner(idEvent);
	if (ppro == nullptr)
		return;

	ppro->KillChatTimer(ppro->InitTimer);
	if (ppro->m_iStatus == ID_STATUS_OFFLINE || ppro->m_iStatus == ID_STATUS_CONNECTING)
		return;

	if (ppro->m_forceVisible)
		ppro->PostIrcMessage(L"/MODE %s -i", ppro->m_info.sNick.c_str());

	if (mir_strlen(ppro->m_myHost) == 0 && ppro->IsConnected())
		ppro->DoUserhostWithReason(2, (L"S" + ppro->m_info.sNick), true, L"%s", ppro->m_info.sNick.c_str());
}

VOID CALLBACK KeepAliveTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CIrcProto *ppro = GetTimerOwner(idEvent);
	if (ppro == nullptr)
		return;

	if (!ppro->m_sendKeepAlive || (ppro->m_iStatus == ID_STATUS_OFFLINE || ppro->m_iStatus == ID_STATUS_CONNECTING)) {
		ppro->KillChatTimer(ppro->KeepAliveTimer);
		return;
	}

	wchar_t temp2[270];
	if (!ppro->m_info.sServerName.IsEmpty())
		mir_snwprintf(temp2, L"PING %s", ppro->m_info.sServerName.c_str());
	else
		mir_snwprintf(temp2, L"PING %u", time(0));

	if (ppro->IsConnected())
		ppro->SendIrcMessage(temp2, false);
}

VOID CALLBACK OnlineNotifTimerProc3(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CIrcProto *ppro = GetTimerOwner(idEvent);
	if (ppro == nullptr)
		return;

	if (!ppro->m_channelAwayNotification ||
		ppro->m_iStatus == ID_STATUS_OFFLINE || ppro->m_iStatus == ID_STATUS_CONNECTING ||
		(!ppro->m_autoOnlineNotification && !ppro->bTempForceCheck) || ppro->bTempDisableCheck) {
		ppro->KillChatTimer(ppro->OnlineNotifTimer3);
		ppro->m_channelsToWho = L"";
		return;
	}

	CMStringW name = GetWord(ppro->m_channelsToWho, 0);
	if (name.IsEmpty()) {
		ppro->m_channelsToWho = L"";
		int count = g_chatApi.SM_GetCount(ppro->m_szModuleName);
		for (int i = 0; i < count; i++) {
			GC_INFO gci = { 0 };
			gci.Flags = GCF_BYINDEX | GCF_NAME | GCF_TYPE | GCF_COUNT;
			gci.iItem = i;
			gci.pszModule = ppro->m_szModuleName;
			if (!Chat_GetInfo(&gci) && gci.iType == GCW_CHATROOM)
			if (gci.iCount <= ppro->m_onlineNotificationLimit)
				ppro->m_channelsToWho += CMStringW(gci.pszName) + L" ";
		}
	}

	if (ppro->m_channelsToWho.IsEmpty()) {
		ppro->SetChatTimer(ppro->OnlineNotifTimer3, 60 * 1000, OnlineNotifTimerProc3);
		return;
	}

	name = GetWord(ppro->m_channelsToWho, 0);
	ppro->DoUserhostWithReason(2, L"S" + name, true, L"%s", name.c_str());
	CMStringW temp = GetWordAddress(ppro->m_channelsToWho, 1);
	ppro->m_channelsToWho = temp;
	if (ppro->m_iTempCheckTime)
		ppro->SetChatTimer(ppro->OnlineNotifTimer3, ppro->m_iTempCheckTime * 1000, OnlineNotifTimerProc3);
	else
		ppro->SetChatTimer(ppro->OnlineNotifTimer3, ppro->m_onlineNotificationTime * 1000, OnlineNotifTimerProc3);
}

VOID CALLBACK OnlineNotifTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CIrcProto *ppro = GetTimerOwner(idEvent);
	if (ppro == nullptr)
		return;

	if (ppro->m_iStatus == ID_STATUS_OFFLINE || ppro->m_iStatus == ID_STATUS_CONNECTING ||
		(!ppro->m_autoOnlineNotification && !ppro->bTempForceCheck) || ppro->bTempDisableCheck) {
		ppro->KillChatTimer(ppro->OnlineNotifTimer);
		ppro->m_namesToWho = L"";
		return;
	}

	CMStringW name = GetWord(ppro->m_namesToWho, 0);
	CMStringW name2 = GetWord(ppro->m_namesToUserhost, 0);

	if (name.IsEmpty() && name2.IsEmpty()) {
		DBVARIANT dbv;
		for (auto &hContact : ppro->AccContacts()) {
			if (ppro->isChatRoom(hContact))
				continue;

			uint8_t bDCC = ppro->getByte(hContact, "DCC", 0);
			bool bHidden = Contact::IsHidden(hContact);
			if (bDCC || bHidden)
				continue;
			if (ppro->getWString(hContact, "Default", &dbv))
				continue;

			uint8_t bAdvanced = ppro->getByte(hContact, "AdvancedMode", 0);
			if (!bAdvanced) {
				db_free(&dbv);
				if (!ppro->getWString(hContact, "Nick", &dbv)) {
					ppro->m_namesToUserhost += CMStringW(dbv.pwszVal) + L" ";
					db_free(&dbv);
				}
			}
			else {
				db_free(&dbv);
				DBVARIANT dbv2;

				wchar_t* DBNick = nullptr;
				wchar_t* DBWildcard = nullptr;
				if (!ppro->getWString(hContact, "Nick", &dbv))
					DBNick = dbv.pwszVal;
				if (!ppro->getWString(hContact, "UWildcard", &dbv2))
					DBWildcard = dbv2.pwszVal;

				if (DBNick && (!DBWildcard || !WCCmp(CharLower(DBWildcard), CharLower(DBNick))))
					ppro->m_namesToWho += CMStringW(DBNick) + L" ";
				else if (DBWildcard)
					ppro->m_namesToWho += CMStringW(DBWildcard) + L" ";

				if (DBNick)     db_free(&dbv);
				if (DBWildcard) db_free(&dbv2);
			}
		}
	}

	if (ppro->m_namesToWho.IsEmpty() && ppro->m_namesToUserhost.IsEmpty()) {
		ppro->SetChatTimer(ppro->OnlineNotifTimer, 60 * 1000, OnlineNotifTimerProc);
		return;
	}

	name = GetWord(ppro->m_namesToWho, 0);
	name2 = GetWord(ppro->m_namesToUserhost, 0);
	CMStringW temp;
	if (!name.IsEmpty()) {
		ppro->DoUserhostWithReason(2, L"S" + name, true, L"%s", name.c_str());
		temp = GetWordAddress(ppro->m_namesToWho, 1);
		ppro->m_namesToWho = temp;
	}

	if (!name2.IsEmpty()) {
		CMStringW params;
		for (int i = 0; i < 3; i++) {
			params = L"";
			for (int j = 0; j < 5; j++)
				params += GetWord(ppro->m_namesToUserhost, i * 5 + j) + L" ";

			if (params[0] != ' ')
				ppro->DoUserhostWithReason(1, CMStringW(L"S") + params, true, params);
		}
		temp = GetWordAddress(ppro->m_namesToUserhost, 15);
		ppro->m_namesToUserhost = temp;
	}

	if (ppro->m_iTempCheckTime)
		ppro->SetChatTimer(ppro->OnlineNotifTimer, ppro->m_iTempCheckTime * 1000, OnlineNotifTimerProc);
	else
		ppro->SetChatTimer(ppro->OnlineNotifTimer, ppro->m_onlineNotificationTime * 1000, OnlineNotifTimerProc);
}

int CIrcProto::AddOutgoingMessageToDB(MCONTACT hContact, const wchar_t *msg)
{
	if (m_iStatus == ID_STATUS_OFFLINE || m_iStatus == ID_STATUS_CONNECTING)
		return 0;

	CMStringW S = DoColorCodes(msg, TRUE, FALSE);

	DBEVENTINFO dbei = {};
	dbei.szModule = m_szModuleName;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.timestamp = (uint32_t)time(0);
	dbei.flags = DBEF_SENT | DBEF_UTF;
	dbei.pBlob = (uint8_t*)mir_utf8encodeW(S);
	dbei.cbBlob = (uint32_t)mir_strlen((char*)dbei.pBlob) + 1;
	db_event_add(hContact, &dbei);
	mir_free(dbei.pBlob);
	return 1;
}

void __cdecl CIrcProto::ResolveIPThread(void *di)
{
	Thread_SetName("IRC: ResolveIPThread");
	IPRESOLVE* ipr = (IPRESOLVE *)di;
	{
		mir_cslock lock(m_csResolve);

		if (ipr != nullptr && (ipr->iType == IP_AUTO && mir_strlen(m_myHost) == 0 || ipr->iType == IP_MANUAL)) {
			hostent* myhost = gethostbyname(ipr->sAddr);
			if (myhost) {
				IN_ADDR in;
				memcpy(&in, myhost->h_addr, 4);
				if (ipr->iType == IP_AUTO)
					strncpy_s(m_myHost, inet_ntoa(in), _TRUNCATE);
				else
					strncpy_s(m_mySpecifiedHostIP, inet_ntoa(in), _TRUNCATE);
			}
		}
	}

	delete ipr;
}

bool CIrcProto::OnIrc_PING(const CIrcMessage *pmsg)
{
	wchar_t szResponse[100];
	if (pmsg->parameters.getCount() > 0)
		mir_snwprintf(szResponse, L"PONG %s", pmsg->parameters[0].c_str());
	else
		wcscpy(szResponse, L"PONG");
	SendIrcMessage(szResponse);
	return false;
}

bool CIrcProto::OnIrc_WELCOME(const CIrcMessage *pmsg)
{
	if (pmsg->parameters[0] != m_info.sNick)
		m_info.sNick = pmsg->parameters[0];

	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
		static wchar_t host[1024];
		int i = 0;
		CMStringW word = GetWord(pmsg->parameters[1], i);
		while (!word.IsEmpty()) {
			if (wcschr(word, '!') && wcschr(word, '@')) {
				mir_wstrncpy(host, word, _countof(host));
				wchar_t* p1 = wcschr(host, '@');
				if (p1)
					ForkThread(&CIrcProto::ResolveIPThread, new IPRESOLVE(_T2A(p1 + 1), IP_AUTO));
			}

			word = GetWord(pmsg->parameters[1], ++i);
		}
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOTOOLONG(const CIrcMessage *pmsg)
{
	CMStringW command = GetNextUserhostReason(2);
	if (command[0] == 'U')
		ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_BACKFROMAWAY(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming) {
		int Temp = m_iStatus;
		m_iStatus = m_iDesiredStatus = ID_STATUS_ONLINE;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)Temp, ID_STATUS_ONLINE);

		if (m_perform)
			DoPerform("Event: Available");
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_SETAWAY(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming) {
		int Temp = m_iDesiredStatus;
		m_iStatus = m_iDesiredStatus = ID_STATUS_AWAY;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)Temp, ID_STATUS_AWAY);

		if (m_perform) {
			switch (Temp) {
			case ID_STATUS_AWAY:
				DoPerform("Event: Away");
				break;
			case ID_STATUS_NA:
				DoPerform("Event: Not available");
				break;
			case ID_STATUS_DND:
				DoPerform("Event: Do not disturb");
				break;
			case ID_STATUS_OCCUPIED:
				DoPerform("Event: Occupied");
				break;
			default:
				m_iStatus = ID_STATUS_AWAY;
				DoPerform("Event: Away");
				break;
			}
		}
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_JOIN(const CIrcMessage *pmsg)
{
	if (pmsg->parameters.getCount() > 0 && pmsg->m_bIncoming && pmsg->prefix.sNick != m_info.sNick) {
		CMStringW host = pmsg->prefix.sUser + L"@" + pmsg->prefix.sHost;
		DoEvent(GC_EVENT_JOIN, pmsg->parameters[0], pmsg->prefix.sNick, nullptr, TranslateT("Normal"), host, NULL, true, false);
		DoEvent(GC_EVENT_SETCONTACTSTATUS, pmsg->parameters[0], pmsg->prefix.sNick, nullptr, nullptr, nullptr, ID_STATUS_ONLINE, FALSE, FALSE);
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_QUIT(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming) {
		CMStringW host = pmsg->prefix.sUser + L"@" + pmsg->prefix.sHost;
		DoEvent(GC_EVENT_QUIT, nullptr, pmsg->prefix.sNick, pmsg->parameters.getCount() > 0 ? pmsg->parameters[0].c_str() : nullptr, nullptr, host, NULL, true, false);
		CONTACT user = { pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, false, false, false };
		CList_SetOffline(&user);
		if (pmsg->prefix.sNick == m_info.sNick)
			Chat_Control(m_szModuleName, nullptr, SESSION_OFFLINE);
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_PART(const CIrcMessage *pmsg)
{
	if (pmsg->parameters.getCount() > 0 && pmsg->m_bIncoming) {
		CMStringW host = pmsg->prefix.sUser + L"@" + pmsg->prefix.sHost;
		DoEvent(GC_EVENT_PART, pmsg->parameters[0], pmsg->prefix.sNick, pmsg->parameters.getCount() > 1 ? pmsg->parameters[1].c_str() : nullptr, nullptr, host, NULL, true, false);
		if (pmsg->prefix.sNick == m_info.sNick)
			Chat_Control(m_szModuleName, pmsg->parameters[0], SESSION_OFFLINE);
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_KICK(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1)
		DoEvent(GC_EVENT_KICK, pmsg->parameters[0], pmsg->parameters[1], pmsg->parameters.getCount() > 2 ? pmsg->parameters[2].c_str() : nullptr, pmsg->prefix.sNick, nullptr, NULL, true, false);
	else
		ShowMessage(pmsg);

	if (pmsg->parameters[1] == m_info.sNick) {
		Chat_Control(m_szModuleName, pmsg->parameters[0], SESSION_OFFLINE);

		if (m_rejoinIfKicked) {
			CHANNELINFO *wi = (CHANNELINFO *)Chat_GetUserInfo(m_szModuleName, pmsg->parameters[0]);
			if (wi && wi->pszPassword)
				PostIrcMessage(L"/JOIN %s %s", pmsg->parameters[0].c_str(), wi->pszPassword);
			else
				PostIrcMessage(L"/JOIN %s", pmsg->parameters[0].c_str());
		}
	}

	return true;
}

bool CIrcProto::OnIrc_MODEQUERY(const CIrcMessage *pmsg)
{
	if (pmsg->parameters.getCount() > 2 && pmsg->m_bIncoming && IsChannel(pmsg->parameters[1])) {
		CMStringW sPassword = L"";
		CMStringW sLimit = L"";
		bool bAdd = false;
		int iParametercount = 3;

		LPCTSTR p1 = pmsg->parameters[2];
		while (*p1 != 0) {
			if (*p1 == '+')
				bAdd = true;
			if (*p1 == '-')
				bAdd = false;
			if (*p1 == 'l' && bAdd) {
				if ((int)pmsg->parameters.getCount() > iParametercount)
					sLimit = pmsg->parameters[iParametercount];
				iParametercount++;
			}
			if (*p1 == 'k' && bAdd) {
				if ((int)pmsg->parameters.getCount() > iParametercount)
					sPassword = pmsg->parameters[iParametercount];
				iParametercount++;
			}

			p1++;
		}

		AddWindowItemData(pmsg->parameters[1], sLimit.IsEmpty() ? nullptr : sLimit.c_str(), pmsg->parameters[2], sPassword.IsEmpty() ? nullptr : sPassword.c_str(), nullptr);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_MODE(const CIrcMessage *pmsg)
{
	bool flag = false;
	bool bContainsValidModes = false;
	CMStringW sModes = L"";
	CMStringW sParams = L"";

	if (pmsg->parameters.getCount() > 1 && pmsg->m_bIncoming) {
		if (IsChannel(pmsg->parameters[0])) {
			bool bAdd = false;
			int  iParametercount = 2;
			LPCTSTR p1 = pmsg->parameters[1];

			while (*p1 != 0) {
				if (*p1 == '+') {
					bAdd = true;
					sModes += L"+";
				}
				if (*p1 == '-') {
					bAdd = false;
					sModes += L"-";
				}
				if (*p1 == 'l' && bAdd && iParametercount < (int)pmsg->parameters.getCount()) {
					bContainsValidModes = true;
					sModes += L"l";
					sParams += L" " + pmsg->parameters[iParametercount];
					iParametercount++;
				}
				if (*p1 == 'b' || *p1 == 'k' && iParametercount < (int)pmsg->parameters.getCount()) {
					bContainsValidModes = true;
					sModes += *p1;
					sParams += L" " + pmsg->parameters[iParametercount];
					iParametercount++;
				}
				if (strchr(sUserModes, (char)*p1)) {
					CMStringW sStatus = ModeToStatus(*p1);
					if ((int)pmsg->parameters.getCount() > iParametercount) {
						if (!mir_wstrcmp(pmsg->parameters[2], m_info.sNick)) {
							char cModeBit = -1;
							CHANNELINFO *wi = (CHANNELINFO*)Chat_GetUserInfo(m_szModuleName, pmsg->parameters[0]);
							switch (*p1) {
							case 'v':      cModeBit = 0;       break;
							case 'h':      cModeBit = 1;       break;
							case 'o':      cModeBit = 2;       break;
							case 'a':      cModeBit = 3;       break;
							case 'q':      cModeBit = 4;       break;
							}

							// set bit for own mode on this channel (voice/hop/op/admin/owner)
							if (wi && cModeBit >= 0) {
								if (bAdd)
									wi->OwnMode |= (1 << cModeBit);
								else
									wi->OwnMode &= ~(1 << cModeBit);
							}

							Chat_SetUserInfo(m_szModuleName, pmsg->parameters[0], wi);
						}
						DoEvent(bAdd ? GC_EVENT_ADDSTATUS : GC_EVENT_REMOVESTATUS, pmsg->parameters[0], pmsg->parameters[iParametercount], pmsg->prefix.sNick, sStatus, nullptr, NULL, m_oldStyleModes ? false : true, false);
						iParametercount++;
					}
				}
				else if (*p1 != 'b' && *p1 != ' ' && *p1 != '+' && *p1 != '-') {
					bContainsValidModes = true;
					if (*p1 != 'l' && *p1 != 'k')
						sModes += *p1;
					flag = true;
				}

				p1++;
			}

			if (m_oldStyleModes) {
				wchar_t temp[256];
				mir_snwprintf(temp, TranslateT("%s sets mode %s"), pmsg->prefix.sNick.c_str(), pmsg->parameters[1].c_str());

				CMStringW sMessage = temp;
				for (int i = 2; i < (int)pmsg->parameters.getCount(); i++)
					sMessage += L" " + pmsg->parameters[i];

				DoEvent(GC_EVENT_INFORMATION, pmsg->parameters[0], pmsg->prefix.sNick, sMessage, nullptr, nullptr, NULL, true, false);
			}
			else if (bContainsValidModes) {
				for (int i = iParametercount; i < (int)pmsg->parameters.getCount(); i++)
					sParams += L" " + pmsg->parameters[i];

				wchar_t temp[4000];
				mir_snwprintf(temp, TranslateT("%s sets mode %s%s"), pmsg->prefix.sNick.c_str(), sModes.c_str(), sParams.c_str());
				DoEvent(GC_EVENT_INFORMATION, pmsg->parameters[0].c_str(), pmsg->prefix.sNick, temp, nullptr, nullptr, NULL, true, false);
			}

			if (flag)
				PostIrcMessage(L"/MODE %s", pmsg->parameters[0].c_str());
		}
		else {
			wchar_t temp[256];
			mir_snwprintf(temp, TranslateT("%s sets mode %s"), pmsg->prefix.sNick.c_str(), pmsg->parameters[1].c_str());

			CMStringW sMessage = temp;
			for (int i = 2; i < (int)pmsg->parameters.getCount(); i++)
				sMessage += L" " + pmsg->parameters[i];

			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, pmsg->prefix.sNick, sMessage, nullptr, nullptr, NULL, true, false);
		}
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_NICK(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 0) {
		bool bIsMe = pmsg->prefix.sNick == m_info.sNick ? true : false;

		if (m_info.sNick == pmsg->prefix.sNick && pmsg->parameters.getCount() > 0) {
			m_info.sNick = pmsg->parameters[0];
			setWString("Nick", m_info.sNick);
		}

		CMStringW host = pmsg->prefix.sUser + L"@" + pmsg->prefix.sHost;
		DoEvent(GC_EVENT_NICK, nullptr, pmsg->prefix.sNick, pmsg->parameters[0], nullptr, host, NULL, true, bIsMe);
		Chat_ChangeUserId(m_szModuleName, nullptr, pmsg->prefix.sNick, pmsg->parameters[0]);

		CONTACT user = { pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, false, false, false };
		MCONTACT hContact = CList_FindContact(&user);
		if (hContact) {
			if (getWord(hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
				setWord(hContact, "Status", ID_STATUS_ONLINE);
			setWString(hContact, "Nick", pmsg->parameters[0]);
			setWString(hContact, "User", pmsg->prefix.sUser);
			setWString(hContact, "Host", pmsg->prefix.sHost);
		}
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_NOTICE(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
		if (IsCTCP(pmsg))
			return true;

		if (!m_ignore || !IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'n')) {
			CMStringW S, S2, S3 = m_szModuleName;

			if (pmsg->prefix.sNick.GetLength() > 0)
				S = pmsg->prefix.sNick;
			else
				S = m_szModuleName;

			if (IsChannel(pmsg->parameters[0]))
				S2 = pmsg->parameters[0];
			else {
				GC_INFO gci = { 0 };
				gci.Flags = GCF_BYID | GCF_TYPE;
				gci.pszModule = m_szModuleName;

				CMStringW str = GetWord(pmsg->parameters[1], 0);
				if (str[0] == '[' && str[1] == '#' && str[str.GetLength() - 1] == ']') {
					str.Delete(str.GetLength() - 1, 1);
					str.Delete(0, 1);
					gci.pszID = str;
					if (!Chat_GetInfo(&gci) && gci.iType == GCW_CHATROOM)
						S2 = GetWord(gci.pszID, 0);
				}
			}
			DoEvent(GC_EVENT_NOTICE, S2.IsEmpty() ? nullptr : S2.c_str(), S, pmsg->parameters[1], nullptr, S3, NULL, true, false);
		}
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_YOURHOST(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming) {
		static const wchar_t* lpszFmt = L"Your host is %99[^ \x5b,], running version %99s";
		wchar_t szHostName[100], szVersion[100];
		if (swscanf(pmsg->parameters[1], lpszFmt, &szHostName, &szVersion) > 0)
			m_info.sServerName = szHostName;
		if (pmsg->parameters[0] != m_info.sNick)
			m_info.sNick = pmsg->parameters[0];
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_INVITE(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && (m_ignore && IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'i')))
		return true;

	if (pmsg->m_bIncoming && m_joinOnInvite && pmsg->parameters.getCount() > 1 && mir_wstrcmpi(pmsg->parameters[0], m_info.sNick) == 0)
		PostIrcMessage(L"/JOIN %s", pmsg->parameters[1].c_str());

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_PINGPONG(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->sCommand == L"PING") {
		wchar_t szResponse[100];
		mir_snwprintf(szResponse, L"PONG %s", pmsg->parameters[0].c_str());
		SendIrcMessage(szResponse);
	}

	return true;
}

bool CIrcProto::OnIrc_PRIVMSG(const CIrcMessage *pmsg)
{
	if (pmsg->parameters.getCount() > 1) {
		if (IsCTCP(pmsg))
			return true;

		CMStringW mess = pmsg->parameters[1];
		bool bIsChannel = IsChannel(pmsg->parameters[0]);

		if (pmsg->m_bIncoming && !bIsChannel) {
			mess = DoColorCodes(mess, TRUE, FALSE);

			CONTACT user = { pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, false, false, false };

			if (Ignore_IsIgnored(NULL, IGNOREEVENT_MESSAGE))
			if (!CList_FindContact(&user))
				return true;

			if ((m_ignore && IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'q'))) {
				MCONTACT hContact = CList_FindContact(&user);
				if (!hContact || (hContact && Contact::IsHidden(hContact)))
					return true;
			}

			MCONTACT hContact = CList_AddContact(&user, false, true);

			PROTORECVEVENT pre = { 0 };
			pre.timestamp = (uint32_t)time(0);
			pre.szMessage = mir_utf8encodeW(mess);
			setWString(hContact, "User", pmsg->prefix.sUser);
			setWString(hContact, "Host", pmsg->prefix.sHost);
			ProtoChainRecvMsg(hContact, &pre);
			mir_free(pre.szMessage);
			return true;
		}

		if (bIsChannel) {
			if (!(pmsg->m_bIncoming && m_ignore && IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'm')))
				DoEvent(GC_EVENT_MESSAGE, pmsg->parameters[0], pmsg->m_bIncoming ? pmsg->prefix.sNick : m_info.sNick, mess, nullptr, nullptr, NULL, true, pmsg->m_bIncoming ? false : true);
			return true;
		}
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::IsCTCP(const CIrcMessage *pmsg)
{
	// is it a ctcp command, i e is the first and last characer of a PRIVMSG or NOTICE text ASCII 1
	CMStringW mess = pmsg->parameters[1];
	if (!(mess.GetLength() > 3 && mess[0] == 1 && mess[mess.GetLength() - 1] == 1))
		return false;

	// set mess to contain the ctcp command, excluding the leading and trailing  ASCII 1
	mess.Delete(0, 1);
	mess.Delete(mess.GetLength() - 1, 1);

	// exploit???
	if (mess.Find(1) != -1 || mess.Find(L"%newl") != -1) {
		CMStringW temp(FORMAT, 
			TranslateT("CTCP ERROR: Malformed CTCP command received from %s!%s@%s. Possible attempt to take control of your IRC client registered"), 
			pmsg->prefix.sNick.c_str(), pmsg->prefix.sUser.c_str(), pmsg->prefix.sHost.c_str());
		DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick, temp, nullptr, nullptr, NULL, true, false);
		return true;
	}

	// extract the type of ctcp command
	CMStringW ocommand = GetWord(mess, 0);
	CMStringW command = GetWord(mess, 0);
	command.MakeLower();

	// should it be ignored?
	if (m_ignore) {
		if (IsChannel(pmsg->parameters[0])) {
			if (command == L"action" && IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'm'))
				return true;
		}
		else {
			if (command == L"action") {
				if (IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'q'))
					return true;
			}
			else if (command == L"dcc") {
				if (IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'd'))
					return true;
			}
			else if (IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'c'))
				return true;
		}
	}

	if (pmsg->sCommand == L"PRIVMSG") {
		// incoming ACTION
		if (command == L"action") {
			mess.Delete(0, 6);

			if (IsChannel(pmsg->parameters[0])) {
				if (mess.GetLength() > 1) {
					mess.Delete(0, 1);
					if (!pmsg->m_bIncoming)
						mess.Replace(L"%%", L"%");

					DoEvent(GC_EVENT_ACTION, pmsg->parameters[0], pmsg->m_bIncoming ? pmsg->prefix.sNick : m_info.sNick, mess, nullptr, nullptr, NULL, true, pmsg->m_bIncoming ? false : true);
				}
			}
			else if (pmsg->m_bIncoming) {
				mess.Insert(0, pmsg->prefix.sNick);
				mess.Insert(0, L"* ");
				mess.Insert(mess.GetLength(), L" *");
				CIrcMessage msg = *pmsg;
				msg.parameters[1] = mess;
				OnIrc_PRIVMSG(&msg);
			}
		}
		// incoming FINGER
		else if (pmsg->m_bIncoming && command == L"finger") {
			PostIrcMessage(L"/NOTICE %s \001FINGER %s (%s)\001", pmsg->prefix.sNick.c_str(), m_name, m_userID);

			wchar_t temp[300];
			mir_snwprintf(temp, TranslateT("CTCP FINGER requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, temp, nullptr, nullptr, NULL, true, false);
		}

		// incoming VERSION
		else if (pmsg->m_bIncoming && command == L"version") {
			PostIrcMessage(L"/NOTICE %s \001VERSION Miranda NG %%mirver (IRC v.%%version)" L", " _A2W(__COPYRIGHT) L"\001", pmsg->prefix.sNick.c_str());

			wchar_t temp[300];
			mir_snwprintf(temp, TranslateT("CTCP VERSION requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, temp, nullptr, nullptr, NULL, true, false);
		}

		// incoming SOURCE
		else if (pmsg->m_bIncoming && command == L"source") {
			PostIrcMessage(L"/NOTICE %s \001SOURCE Get Miranda IRC here: https://miranda-ng.org/ \001", pmsg->prefix.sNick.c_str());

			wchar_t temp[300];
			mir_snwprintf(temp, TranslateT("CTCP SOURCE requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, temp, nullptr, nullptr, NULL, true, false);
		}

		// incoming USERINFO
		else if (pmsg->m_bIncoming && command == L"userinfo") {
			PostIrcMessage(L"/NOTICE %s \001USERINFO %s\001", pmsg->prefix.sNick.c_str(), m_userInfo);

			wchar_t temp[300];
			mir_snwprintf(temp, TranslateT("CTCP USERINFO requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, temp, nullptr, nullptr, NULL, true, false);
		}

		// incoming PING
		else if (pmsg->m_bIncoming && command == L"ping") {
			PostIrcMessage(L"/NOTICE %s \001%s\001", pmsg->prefix.sNick.c_str(), mess.c_str());

			wchar_t temp[300];
			mir_snwprintf(temp, TranslateT("CTCP PING requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, temp, nullptr, nullptr, NULL, true, false);
		}

		// incoming TIME
		else if (pmsg->m_bIncoming && command == L"time") {
			wchar_t temp[300];
			time_t tim = time(0);
			mir_wstrncpy(temp, _wctime(&tim), 25);
			PostIrcMessage(L"/NOTICE %s \001TIME %s\001", pmsg->prefix.sNick.c_str(), temp);

			mir_snwprintf(temp, TranslateT("CTCP TIME requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, temp, nullptr, nullptr, NULL, true, false);
		}

		// incoming DCC request... lots of stuff happening here...
		else if (pmsg->m_bIncoming && command == L"dcc") {
			CMStringW type = GetWord(mess, 1);
			type.MakeLower();

			// components of a dcc message
			CMStringW sFile = L"";
			uint32_t dwAdr = 0;
			int iPort = 0;
			unsigned __int64 dwSize = 0;
			CMStringW sToken = L"";
			bool bIsChat = (type == L"chat");

			// 1. separate the dcc command into the correct pieces
			if (bIsChat || type == L"send") {
				// if the filename is surrounded by quotes, do this
				if (GetWord(mess, 2)[0] == '\"') {
					int end = 0;
					int begin = mess.Find('\"', 0);
					if (begin >= 0) {
						end = mess.Find('\"', begin + 1);
						if (end >= 0) {
							sFile = mess.Mid(begin + 1, end - begin - 1);

							begin = mess.Find(' ', end);
							if (begin >= 0) {
								CMStringW rest = mess.Mid(begin);
								dwAdr = wcstoul(GetWord(rest, 0), nullptr, 10);
								iPort = _wtoi(GetWord(rest, 1));
								dwSize = _wtoi64(GetWord(rest, 2));
								sToken = GetWord(rest, 3);
							}
						}
					}
				}
				// ... or try another method of separating the dcc command
				else if (!GetWord(mess, (bIsChat) ? 4 : 5).IsEmpty()) {
					int index = (bIsChat) ? 4 : 5;
					bool bFlag = false;

					// look for the part of the ctcp command that contains adress, port and size
					while (!bFlag && !GetWord(mess, index).IsEmpty()) {
						CMStringW sTemp;

						if (type == L"chat")
							sTemp = GetWord(mess, index - 1) + GetWord(mess, index);
						else
							sTemp = GetWord(mess, index - 2) + GetWord(mess, index - 1) + GetWord(mess, index);

						// if all characters are number it indicates we have found the adress, port and size parameters
						int ind = 0;
						while (sTemp[ind] != 0) {
							if (!iswdigit(sTemp[ind]))
								break;
							ind++;
						}

						if (sTemp[ind] == 0 && GetWord(mess, index + ((bIsChat) ? 1 : 2)).IsEmpty())
							bFlag = true;
						index++;
					}

					if (bFlag) {
						wchar_t* p1 = wcsdup(GetWordAddress(mess, 2));
						wchar_t* p2 = (wchar_t*)GetWordAddress(p1, index - 5);

						if (type == L"send") {
							if (p2 > p1) {
								p2--;
								while (p2 != p1 && *p2 == ' ') {
									*p2 = 0;
									p2--;
								}
								sFile = p1;
							}
						}
						else sFile = L"chat";

						free(p1);

						dwAdr = wcstoul(GetWord(mess, index - (bIsChat ? 2 : 3)), nullptr, 10);
						iPort = _wtoi(GetWord(mess, index - (bIsChat ? 1 : 2)));
						dwSize = _wtoi64(GetWord(mess, index - 1));
						sToken = GetWord(mess, index);
					}
				}
			}
			else if (type == L"accept" || type == L"resume") {
				// if the filename is surrounded by quotes, do this
				if (GetWord(mess, 2)[0] == '\"') {
					int end = 0;
					int begin = mess.Find('\"', 0);
					if (begin >= 0) {
						end = mess.Find('\"', begin + 1);
						if (end >= 0) {
							sFile = mess.Mid(begin + 1, end);

							begin = mess.Find(' ', end);
							if (begin >= 0) {
								CMStringW rest = mess.Mid(begin);
								iPort = _wtoi(GetWord(rest, 0));
								dwSize = _wtoi(GetWord(rest, 1));
								sToken = GetWord(rest, 2);
							}
						}
					}
				}
				// ... or try another method of separating the dcc command
				else if (!GetWord(mess, 4).IsEmpty()) {
					int index = 4;
					bool bFlag = false;

					// look for the part of the ctcp command that contains adress, port and size
					while (!bFlag && !GetWord(mess, index).IsEmpty()) {
						CMStringW sTemp = GetWord(mess, index - 1) + GetWord(mess, index);

						// if all characters are number it indicates we have found the adress, port and size parameters
						int ind = 0;

						while (sTemp[ind] != 0) {
							if (!iswdigit(sTemp[ind]))
								break;
							ind++;
						}

						if (sTemp[ind] == 0 && GetWord(mess, index + 2).IsEmpty())
							bFlag = true;
						index++;
					}
					if (bFlag) {
						wchar_t* p1 = wcsdup(GetWordAddress(mess, 2));
						wchar_t* p2 = (wchar_t*)GetWordAddress(p1, index - 4);

						if (p2 > p1) {
							p2--;
							while (p2 != p1 && *p2 == ' ') {
								*p2 = 0;
								p2--;
							}
							sFile = p1;
						}

						free(p1);

						iPort = _wtoi(GetWord(mess, index - 2));
						dwSize = _wtoi64(GetWord(mess, index - 1));
						sToken = GetWord(mess, index);
					}
				}
			}
			// end separating dcc commands

			// 2. Check for malformed dcc commands or other errors
			if (bIsChat || type == L"send") {
				wchar_t szTemp[256];
				szTemp[0] = 0;

				unsigned long ulAdr = 0;
				if (m_manualHost)
					ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
				else
					ulAdr = ConvertIPToInteger(m_IPFromServer ? m_myHost : m_myLocalHost);

				if (bIsChat && !m_DCCChatEnabled)
					mir_snwprintf(szTemp, TranslateT("DCC: Chat request from %s denied"), pmsg->prefix.sNick.c_str());

				else if (type == L"send" && !m_DCCFileEnabled)
					mir_snwprintf(szTemp, TranslateT("DCC: File transfer request from %s denied"), pmsg->prefix.sNick.c_str());

				else if (type == L"send" && !iPort && ulAdr == 0)
					mir_snwprintf(szTemp, TranslateT("DCC: Reverse file transfer request from %s denied [No local IP]"), pmsg->prefix.sNick.c_str());

				if (sFile.IsEmpty() || dwAdr == 0 || dwSize == 0 || iPort == 0 && sToken.IsEmpty())
					mir_snwprintf(szTemp, TranslateT("DCC ERROR: Malformed CTCP request from %s [%s]"), pmsg->prefix.sNick.c_str(), mess.c_str());

				if (szTemp[0]) {
					DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick, szTemp, nullptr, nullptr, NULL, true, false);
					return true;
				}

				// remove path from the filename if the remote client (stupidly) sent it
				CMStringW sFileCorrected = sFile;
				int i = sFile.ReverseFind('\\');
				if (i != -1)
					sFileCorrected = sFile.Mid(i + 1);
				sFile = sFileCorrected;
			}
			else if (type == L"accept" || type == L"resume") {
				wchar_t szTemp[256];
				szTemp[0] = 0;

				if (type == L"resume" && !m_DCCFileEnabled)
					mir_snwprintf(szTemp, TranslateT("DCC: File transfer resume request from %s denied"), pmsg->prefix.sNick.c_str());

				if (sToken.IsEmpty() && iPort == 0 || sFile.IsEmpty())
					mir_snwprintf(szTemp, TranslateT("DCC ERROR: Malformed CTCP request from %s [%s]"), pmsg->prefix.sNick.c_str(), mess.c_str());

				if (szTemp[0]) {
					DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick, szTemp, nullptr, nullptr, NULL, true, false);
					return true;
				}

				// remove path from the filename if the remote client (stupidly) sent it
				CMStringW sFileCorrected = sFile;
				int i = sFile.ReverseFind('\\');
				if (i != -1)
					sFileCorrected = sFile.Mid(i + 1);
				sFile = sFileCorrected;
			}

			// 3. Take proper actions considering type of command

			// incoming chat request
			if (bIsChat) {
				CONTACT user = { pmsg->prefix.sNick, nullptr, nullptr, false, false, true };
				MCONTACT hContact = CList_FindContact(&user);

				// check if it should be ignored
				if (m_DCCChatIgnore == 1 || m_DCCChatIgnore == 2 && hContact && Contact::OnList(hContact) && !Contact::IsHidden(hContact)) {
					CMStringW host = pmsg->prefix.sUser + L"@" + pmsg->prefix.sHost;
					CList_AddDCCChat(pmsg->prefix.sNick, host, dwAdr, iPort); // add a CHAT event to the clist
				}
				else {
					wchar_t szTemp[512];
					mir_snwprintf(szTemp, TranslateT("DCC: Chat request from %s denied"), pmsg->prefix.sNick.c_str());
					DoEvent(GC_EVENT_INFORMATION, nullptr, m_info.sNick, szTemp, nullptr, nullptr, NULL, true, false);
				}
			}

			// remote requested that the file should be resumed
			if (type == L"resume") {
				CDccSession *dcc;
				if (sToken.IsEmpty())
					dcc = FindDCCSendByPort(iPort);
				else
					dcc = FindPassiveDCCSend(_wtoi(sToken)); // reverse ft

				if (dcc) {
					InterlockedExchange(&dcc->dwWhatNeedsDoing, (long)FILERESUME_RESUME);
					dcc->dwResumePos = dwSize; // dwSize is the resume position
					PostIrcMessage(L"/PRIVMSG %s \001DCC ACCEPT %s\001", pmsg->prefix.sNick.c_str(), GetWordAddress(mess, 2));
				}
			}

			// remote accepted your request for a file resume
			if (type == L"accept") {
				CDccSession *dcc;
				if (sToken.IsEmpty())
					dcc = FindDCCRecvByPortAndName(iPort, pmsg->prefix.sNick);
				else
					dcc = FindPassiveDCCRecv(pmsg->prefix.sNick, sToken); // reverse ft

				if (dcc) {
					InterlockedExchange(&dcc->dwWhatNeedsDoing, (long)FILERESUME_RESUME);
					dcc->dwResumePos = dwSize;	// dwSize is the resume position					
					SetEvent(dcc->hEvent);
				}
			}

			if (type == L"send") {
				CMStringW sTokenBackup = sToken;
				bool bTurbo = false; // TDCC indicator

				if (!sToken.IsEmpty() && sToken[sToken.GetLength() - 1] == 'T') {
					bTurbo = true;
					sToken.Delete(sToken.GetLength() - 1, 1);
				}

				// if a token exists and the port is non-zero it is the remote
				// computer telling us that is has accepted to act as server for
				// a reverse filetransfer. The plugin should connect to that computer
				// and start sedning the file (if the token is valid). Compare to DCC RECV
				if (!sToken.IsEmpty() && iPort) {
					CDccSession *dcc = FindPassiveDCCSend(_wtoi(sToken));
					if (dcc) {
						dcc->SetupPassive(dwAdr, iPort);
						dcc->Connect();
					}
				}
				else {
					CONTACT user = { pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, false, false, false };
					if (Ignore_IsIgnored(NULL, IGNOREEVENT_FILE))
					if (!CList_FindContact(&user))
						return true;

					MCONTACT hContact = CList_AddContact(&user, false, true);
					if (hContact) {
						DCCINFO* di = new DCCINFO;
						di->hContact = hContact;
						di->sFile = sFile;
						di->dwSize = dwSize;
						di->sContactName = pmsg->prefix.sNick;
						di->dwAdr = dwAdr;
						di->iPort = iPort;
						di->iType = DCC_SEND;
						di->bSender = false;
						di->bTurbo = bTurbo;
						di->bSSL = false;
						di->bReverse = (iPort == 0 && !sToken.IsEmpty()) ? true : false;
						if (di->bReverse)
							di->sToken = sTokenBackup;

						setWString(hContact, "User", pmsg->prefix.sUser);
						setWString(hContact, "Host", pmsg->prefix.sHost);

						const wchar_t* tszTemp = sFile;

						PROTORECVFILE pre = { 0 };
						pre.dwFlags = PRFF_UNICODE;
						pre.timestamp = (uint32_t)time(0);
						pre.fileCount = 1;
						pre.files.w = &tszTemp;
						pre.lParam = (LPARAM)di;
						ProtoChainRecvFile(hContact, &pre);
					}
				}
			}
			// end type == "send"
		}
		else if (pmsg->m_bIncoming) {
			wchar_t temp[300];
			mir_snwprintf(temp, TranslateT("CTCP %s requested by %s"), ocommand.c_str(), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, temp, nullptr, nullptr, NULL, true, false);
		}
	}

	// handle incoming ctcp in notices. This technique is used for replying to CTCP queries
	else if (pmsg->sCommand == L"NOTICE") {
		wchar_t szTemp[300];
		szTemp[0] = 0;

		//if we got incoming CTCP Version for contact in CList - then write its as MirVer for that contact!
		if (pmsg->m_bIncoming && command == L"version") {
			CONTACT user = { pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, false, false, false };
			MCONTACT hContact = CList_FindContact(&user);
			if (hContact)
				setWString(hContact, "MirVer", DoColorCodes(GetWordAddress(mess, 1), TRUE, FALSE));
		}

		// if the whois window is visible and the ctcp reply belongs to the user in it, then show the reply in the whois window
		if (m_whoisDlg && IsWindowVisible(m_whoisDlg->GetHwnd())) {
			m_whoisDlg->m_InfoNick.GetText(szTemp, _countof(szTemp));
			if (mir_wstrcmpi(szTemp, pmsg->prefix.sNick) == 0) {
				if (pmsg->m_bIncoming && (command == L"version" || command == L"userinfo" || command == L"time")) {
					SetActiveWindow(m_whoisDlg->GetHwnd());
					m_whoisDlg->m_Reply.SetText(DoColorCodes(GetWordAddress(mess, 1), TRUE, FALSE));
					return true;
				}
				if (pmsg->m_bIncoming && command == L"ping") {
					SetActiveWindow(m_whoisDlg->GetHwnd());
					int s = (int)time(0) - (int)_wtol(GetWordAddress(mess, 1));
					wchar_t szTmp[30];
					if (s == 1)
						mir_snwprintf(szTmp, TranslateT("%u second"), s);
					else
						mir_snwprintf(szTmp, TranslateT("%u seconds"), s);

					m_whoisDlg->m_Reply.SetText(DoColorCodes(szTmp, TRUE, FALSE));
					return true;
				}
			}
		}

		//... else show the reply in the current window
		if (pmsg->m_bIncoming && command == L"ping") {
			int s = (int)time(0) - (int)_wtol(GetWordAddress(mess, 1));
			mir_snwprintf(szTemp, TranslateT("CTCP PING reply from %s: %u sec(s)"), pmsg->prefix.sNick.c_str(), s);
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, szTemp, nullptr, nullptr, NULL, true, false);
		}
		else {
			mir_snwprintf(szTemp, TranslateT("CTCP %s reply from %s: %s"), ocommand.c_str(), pmsg->prefix.sNick.c_str(), GetWordAddress(mess, 1));
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, szTemp, nullptr, nullptr, NULL, true, false);
		}
	}

	return true;
}

bool CIrcProto::OnIrc_NAMES(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 3)
		sNamesList += pmsg->parameters[3] + L" ";
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_ENDNAMES(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
		CMStringW name = L"a";
		int i = 0;
		BOOL bFlag = false;

		// Is the user on the names list?
		while (!name.IsEmpty()) {
			name = GetWord(sNamesList, i);
			i++;
			if (!name.IsEmpty()) {
				int index = 0;
				while (wcschr(sUserModePrefixes, name[index]))
					index++;

				if (!mir_wstrcmpi(name.Mid(index), m_info.sNick)) {
					bFlag = true;
					break;
				}
			}
		}

		if (bFlag) {
			const wchar_t* sChanName = pmsg->parameters[1];
			if (sChanName[0] == '@' || sChanName[0] == '*' || sChanName[0] == '=')
				sChanName++;

			// Add a new chat window
			uint8_t btOwnMode = 0;

			SESSION_INFO *si = Chat_NewSession(GCW_CHATROOM, m_szModuleName, sChanName, sChanName);
			if (si) {
				PostIrcMessage(L"/MODE %s", sChanName);

				// register the statuses
				Chat_AddGroup(si, TranslateT("Owner"));
				Chat_AddGroup(si, TranslateT("Admin"));
				Chat_AddGroup(si, TranslateT("Op"));
				Chat_AddGroup(si, TranslateT("Halfop"));
				Chat_AddGroup(si, TranslateT("Voice"));
				Chat_AddGroup(si, TranslateT("Normal"));
				{
					int k = 0;
					CMStringW sTemp = GetWord(sNamesList, k);
					const wchar_t *pwszNormal = TranslateT("Normal");

					// Fill the nicklist
					while (!sTemp.IsEmpty()) {
						CMStringW sStat;
						CMStringW sTemp2 = sTemp;
						sStat = PrefixToStatus(sTemp[0]);

						// fix for networks like freshirc where they allow more than one prefix
						while (PrefixToStatus(sTemp[0]) != pwszNormal)
							sTemp.Delete(0, 1);

						GCEVENT gce = { m_szModuleName, 0, GC_EVENT_JOIN };
						gce.pszID.w = sChanName;
						gce.pszUID.w = sTemp;
						gce.pszNick.w = sTemp;
						gce.pszStatus.w = sStat;
						gce.bIsMe = (!mir_wstrcmpi(gce.pszNick.w, m_info.sNick)) ? TRUE : FALSE;
						if (gce.bIsMe) {
							char BitNr = -1;
							switch (sTemp2[0]) {
							case '+':   BitNr = 0;   break;
							case '%':   BitNr = 1;   break;
							case '@':   BitNr = 2;   break;
							case '!':   BitNr = 3;   break;
							case '*':   BitNr = 4;   break;
							}
							if (BitNr >= 0)
								btOwnMode = (1 << BitNr);
							else
								btOwnMode = 0;
						}
						gce.time = gce.bIsMe ? time(0) : 0;
						Chat_Event(&gce);
						
						DoEvent(GC_EVENT_SETCONTACTSTATUS, sChanName, sTemp, nullptr, nullptr, nullptr, ID_STATUS_ONLINE, FALSE, FALSE);
						
						// fix for networks like freshirc where they allow more than one prefix
						if (PrefixToStatus(sTemp2[0]) != pwszNormal) {
							sTemp2.Delete(0, 1);
							sStat = PrefixToStatus(sTemp2[0]);
							while (sStat != pwszNormal) {
								DoEvent(GC_EVENT_ADDSTATUS, sChanName, sTemp, L"system", sStat, nullptr, NULL, false, false, 0);
								sTemp2.Delete(0, 1);
								sStat = PrefixToStatus(sTemp2[0]);
							}
						}

						k++;
						sTemp = GetWord(sNamesList, k);
					}
				}

				//Set the item data for the window
				{
					CHANNELINFO *wi = (CHANNELINFO *)Chat_GetUserInfo(m_szModuleName, sChanName);
					if (!wi)
						wi = new CHANNELINFO;
					wi->OwnMode = btOwnMode;
					wi->pszLimit = nullptr;
					wi->pszMode = nullptr;
					wi->pszPassword = nullptr;
					wi->pszTopic = nullptr;
					wi->codepage = getCodepage();
					Chat_SetUserInfo(m_szModuleName, sChanName, wi);

					if (!sTopic.IsEmpty() && !mir_wstrcmpi(GetWord(sTopic, 0), sChanName)) {
						DoEvent(GC_EVENT_TOPIC, sChanName, sTopicName.IsEmpty() ? nullptr : sTopicName.c_str(), GetWordAddress(sTopic, 1), nullptr, sTopicTime.IsEmpty() ? nullptr : sTopicTime.c_str(), NULL, true, false);
						AddWindowItemData(sChanName, nullptr, nullptr, nullptr, GetWordAddress(sTopic, 1));
						sTopic = L"";
						sTopicName = L"";
						sTopicTime = L"";
					}
				}

				DBVARIANT dbv;
				if (!getWString("JTemp", &dbv)) {
					CMStringW command = L"a";
					CMStringW save = L"";
					int k = 0;

					while (!command.IsEmpty()) {
						command = GetWord(dbv.pwszVal, k);
						k++;
						if (!command.IsEmpty()) {
							CMStringW S = command.Mid(1);
							if (!mir_wstrcmpi(sChanName, S))
								break;

							save += command + L" ";
						}
					}

					if (!command.IsEmpty()) {
						save += GetWordAddress(dbv.pwszVal, k);
						switch (command[0]) {
						case 'M':
							Chat_Control(m_szModuleName, sChanName, WINDOW_HIDDEN);
							break;
						case 'X':
							Chat_Control(m_szModuleName, sChanName, WINDOW_VISIBLE);
							break;
						default:
							Chat_Control(m_szModuleName, sChanName, SESSION_INITDONE);
							break;
						}
					}
					else Chat_Control(m_szModuleName, sChanName, SESSION_INITDONE);

					if (save.IsEmpty())
						db_unset(0, m_szModuleName, "JTemp");
					else
						setWString("JTemp", save);
					db_free(&dbv);
				}
				else Chat_Control(m_szModuleName, sChanName, SESSION_INITDONE);

				Chat_Control(m_szModuleName, sChanName, SESSION_ONLINE);
			}
		}
	}

	sNamesList = L"";
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_INITIALTOPIC(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 2) {
		AddWindowItemData(pmsg->parameters[1], nullptr, nullptr, nullptr, pmsg->parameters[2]);
		sTopic = pmsg->parameters[1] + L" " + pmsg->parameters[2];
		sTopicName = L"";
		sTopicTime = L"";
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_INITIALTOPICNAME(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 3) {
		wchar_t tTimeBuf[128], *tStopStr;
		time_t ttTopicTime;
		sTopicName = pmsg->parameters[2];
		ttTopicTime = wcstol(pmsg->parameters[3], &tStopStr, 10);
		wcsftime(tTimeBuf, 128, L"%#c", localtime(&ttTopicTime));
		sTopicTime = tTimeBuf;
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_TOPIC(const CIrcMessage *pmsg)
{
	if (pmsg->parameters.getCount() > 1 && pmsg->m_bIncoming) {
		DoEvent(GC_EVENT_TOPIC, pmsg->parameters[0], pmsg->prefix.sNick, pmsg->parameters[1], nullptr, sTopicTime.IsEmpty() ? nullptr : sTopicTime.c_str(), NULL, true, false);
		AddWindowItemData(pmsg->parameters[0], nullptr, nullptr, nullptr, pmsg->parameters[1]);
	}
	ShowMessage(pmsg);
	return true;
}

static INT_PTR __stdcall sttShowDlgList(void* param)
{
	CIrcProto *ppro = (CIrcProto*)param;
	
	mir_cslock lck(ppro->m_csList);
	if (ppro->m_listDlg == nullptr) {
		ppro->m_listDlg = new CListDlg(ppro);
		ppro->m_listDlg->Show();
	}
	return 0;
}

bool CIrcProto::OnIrc_LISTSTART(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming) {
		CallFunctionSync(sttShowDlgList, this);
		m_channelNumber = 0;
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_LIST(const CIrcMessage *pmsg)
{
	if (!pmsg->m_bIncoming || pmsg->parameters.getCount() <= 2)
		return true;

	mir_cslockfull lck(m_csList);
	if (!m_listDlg)
		return true;
		
	m_channelNumber++;

	HWND hListView = GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW);
	HWND hStatusWnd = m_listDlg->m_status.GetHwnd();
	lck.unlock();

	LVITEM lvItem;
	lvItem.iItem = ListView_GetItemCount(hListView);
	lvItem.mask = LVIF_TEXT | LVIF_PARAM;
	lvItem.iSubItem = 0;
	lvItem.pszText = pmsg->parameters[1].GetBuffer();
	lvItem.lParam = lvItem.iItem;
	lvItem.iItem = ListView_InsertItem(hListView, &lvItem);
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	lvItem.pszText = pmsg->parameters[pmsg->parameters.getCount() - 2].GetBuffer();
	ListView_SetItem(hListView, &lvItem);

	wchar_t* temp = mir_wstrdup(pmsg->parameters[pmsg->parameters.getCount() - 1]);
	wchar_t* find = wcsstr(temp, L"[+");
	wchar_t* find2 = wcschr(temp, ']');
	wchar_t* save = temp;
	if (find == temp && find2 != nullptr && find + 8 >= find2) {
		temp = wcschr(temp, ']');
		if (mir_wstrlen(temp) > 1) {
			temp++;
			temp[0] = 0;
			lvItem.iSubItem = 2;
			lvItem.pszText = save;
			ListView_SetItem(hListView, &lvItem);
			temp[0] = ' ';
			temp++;
		}
		else temp = save;
	}

	lvItem.iSubItem = 3;
	CMStringW S = DoColorCodes(temp, TRUE, FALSE);
	lvItem.pszText = S.GetBuffer();
	ListView_SetItem(hListView, &lvItem);
	temp = save;
	mir_free(temp);

	int percent = 100;
	if (m_noOfChannels > 0)
		percent = (int)(m_channelNumber * 100) / m_noOfChannels;

	wchar_t text[100];
	if (percent < 100)
		mir_snwprintf(text, TranslateT("Downloading list (%u%%) - %u channels"), percent, m_channelNumber);
	else
		mir_snwprintf(text, TranslateT("Downloading list - %u channels"), m_channelNumber);
	SetWindowText(hStatusWnd, text);
	return true;
}

bool CIrcProto::OnIrc_LISTEND(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming) {
		mir_cslock lck(m_csList);
		if (m_listDlg) {
			EnableWindow(GetDlgItem(m_listDlg->GetHwnd(), IDC_JOIN), true);
			ListView_SetSelectionMark(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW), 0);
			ListView_SetColumnWidth(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW), 1, LVSCW_AUTOSIZE);
			ListView_SetColumnWidth(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW), 2, LVSCW_AUTOSIZE);
			ListView_SetColumnWidth(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW), 3, LVSCW_AUTOSIZE);
			m_listDlg->UpdateList();

			wchar_t text[100];
			mir_snwprintf(text, TranslateT("Done: %u channels"), m_channelNumber);
			int percent = 100;
			if (m_noOfChannels > 0)
				percent = (int)(m_channelNumber * 100) / m_noOfChannels;
			if (percent < 70) {
				mir_wstrcat(text, L" ");
				mir_wstrcat(text, TranslateT("(probably truncated by server)"));
			}
			SetDlgItemText(m_listDlg->GetHwnd(), IDC_TEXT, text);
		}
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_BANLIST(const CIrcMessage *pmsg)
{
	if (m_managerDlg && pmsg->m_bIncoming && pmsg->parameters.getCount() > 2) {
		if (m_managerDlg->GetHwnd() && (
			m_managerDlg->m_radio1.GetState() && pmsg->sCommand == L"367" ||
			m_managerDlg->m_radio2.GetState() && pmsg->sCommand == L"346" ||
			m_managerDlg->m_radio3.GetState() && pmsg->sCommand == L"348") &&
			!m_managerDlg->m_radio1.Enabled() && !m_managerDlg->m_radio2.Enabled() && !m_managerDlg->m_radio3.Enabled()) {
			CMStringW S = pmsg->parameters[2];
			if (pmsg->parameters.getCount() > 3) {
				S += L"   - ";
				S += pmsg->parameters[3];
				if (pmsg->parameters.getCount() > 4) {
					S += L" -  ( ";
					time_t time = _wtoi(pmsg->parameters[4]);
					S += _wctime(&time);
					S.Replace(L"\n", L" ");
					S += L")";
				}
			}

			SendDlgItemMessage(m_managerDlg->GetHwnd(), IDC_LIST, LB_ADDSTRING, 0, (LPARAM)S.c_str());
		}
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_BANLISTEND(const CIrcMessage *pmsg)
{
	if (m_managerDlg && pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
		if (m_managerDlg->GetHwnd() &&
			(m_managerDlg->m_radio1.GetState() && pmsg->sCommand == L"368"
			|| m_managerDlg->m_radio2.GetState() && pmsg->sCommand == L"347"
			|| m_managerDlg->m_radio3.GetState() && pmsg->sCommand == L"349") &&
			!m_managerDlg->m_radio1.Enabled() && !m_managerDlg->m_radio2.Enabled() && !m_managerDlg->m_radio3.Enabled()) {
			if (strchr(sChannelModes, 'b'))
				m_managerDlg->m_radio1.Enable();
			if (strchr(sChannelModes, 'I'))
				m_managerDlg->m_radio2.Enable();
			if (strchr(sChannelModes, 'e'))
				m_managerDlg->m_radio3.Enable();
			if (BST_UNCHECKED == IsDlgButtonChecked(m_managerDlg->GetHwnd(), IDC_NOTOP))
				m_managerDlg->m_add.Enable();
		}
	}

	ShowMessage(pmsg);
	return true;
}

static INT_PTR __stdcall sttShowWhoisWnd(void* param)
{
	CIrcMessage* pmsg = (CIrcMessage*)param;
	CIrcProto *ppro = (CIrcProto*)pmsg->m_proto;
	if (ppro->m_whoisDlg == nullptr) {
		ppro->m_whoisDlg = new CWhoisDlg(ppro);
		ppro->m_whoisDlg->Show();
	}

	ppro->m_whoisDlg->ShowMessage(pmsg);
	return 0;
}

bool CIrcProto::OnIrc_WHOIS_NAME(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 5 && m_manualWhoisCount > 0)
		CallFunctionSync(sttShowWhoisWnd, (void*)pmsg);

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_CHANNELS(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0)
		m_whoisDlg->m_InfoChannels.SetText(pmsg->parameters[2]);
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_AWAY(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0)
		m_whoisDlg->m_InfoAway2.SetText(pmsg->parameters[2]);
	if (m_manualWhoisCount < 1 && pmsg->m_bIncoming && pmsg->parameters.getCount() > 2)
		WhoisAwayReply = pmsg->parameters[2];
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_OTHER(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0) {
		wchar_t temp[1024], temp2[1024];
		m_whoisDlg->m_InfoOther.GetText(temp, 1000);
		mir_wstrcat(temp, L"%s\r\n");
		mir_snwprintf(temp2, temp, pmsg->parameters[2].c_str());
		m_whoisDlg->m_InfoOther.SetText(temp2);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_END(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1 && m_manualWhoisCount < 1) {
		CONTACT user = { pmsg->parameters[1], nullptr, nullptr, false, false, true };
		MCONTACT hContact = CList_FindContact(&user);
		if (hContact)
			ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)WhoisAwayReply.c_str());
	}

	m_manualWhoisCount--;
	if (m_manualWhoisCount < 0)
		m_manualWhoisCount = 0;
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_IDLE(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0) {
		int S = _wtoi(pmsg->parameters[2]);
		int D = S / (60 * 60 * 24);
		S -= (D * 60 * 60 * 24);
		int H = S / (60 * 60);
		S -= (H * 60 * 60);
		int M = S / 60;
		S -= (M * 60);

		wchar_t temp[100];
		if (D)
			mir_snwprintf(temp, TranslateT("%ud, %uh, %um, %us"), D, H, M, S);
		else if (H)
			mir_snwprintf(temp, TranslateT("%uh, %um, %us"), H, M, S);
		else if (M)
			mir_snwprintf(temp, TranslateT("%um, %us"), M, S);
		else if (S)
			mir_snwprintf(temp, TranslateT("%us"), S);
		else
			temp[0] = 0;

		wchar_t temp3[256];
		wchar_t tTimeBuf[128], *tStopStr;
		time_t ttTime = wcstol(pmsg->parameters[3], &tStopStr, 10);
		wcsftime(tTimeBuf, 128, L"%c", localtime(&ttTime));
		mir_snwprintf(temp3, TranslateT("online since %s, idle %s"), tTimeBuf, temp);
		m_whoisDlg->m_AwayTime.SetText(temp3);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_SERVER(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0)
		m_whoisDlg->m_InfoServer.SetText(pmsg->parameters[2]);
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_AUTH(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0) {
		if (pmsg->sCommand == L"330")
			m_whoisDlg->m_InfoAuth.SetText(pmsg->parameters[2]);
		else if (pmsg->parameters[2] == L"is an identified user" || pmsg->parameters[2] == L"is a registered nick")
			m_whoisDlg->m_InfoAuth.SetText(pmsg->parameters[2]);
		else
			OnIrc_WHOIS_OTHER(pmsg);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_NO_USER(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 2 && !IsChannel(pmsg->parameters[1])) {
		if (m_whoisDlg)
			m_whoisDlg->ShowMessageNoUser(pmsg);

		CONTACT user = { pmsg->parameters[1], nullptr, nullptr, false, false, false };
		MCONTACT hContact = CList_FindContact(&user);
		if (hContact) {
			AddOutgoingMessageToDB(hContact, (CMStringW)L"> " + pmsg->parameters[2] + (CMStringW)L": " + pmsg->parameters[1]);

			DBVARIANT dbv;
			if (!getWString(hContact, "Default", &dbv)) {
				setWString(hContact, "Nick", dbv.pwszVal);

				DBVARIANT dbv2;
				if (getByte(hContact, "AdvancedMode", 0) == 0)
					DoUserhostWithReason(1, ((CMStringW)L"S" + dbv.pwszVal), true, dbv.pwszVal);
				else {
					if (!getWString(hContact, "UWildcard", &dbv2)) {
						DoUserhostWithReason(2, ((CMStringW)L"S" + dbv2.pwszVal), true, dbv2.pwszVal);
						db_free(&dbv2);
					}
					else DoUserhostWithReason(2, ((CMStringW)L"S" + dbv.pwszVal), true, dbv.pwszVal);
				}
				setString(hContact, "User", "");
				setString(hContact, "Host", "");
				db_free(&dbv);
			}
		}
	}

	ShowMessage(pmsg);
	return true;
}

static INT_PTR __stdcall sttShowNickWnd(void* param)
{
	CIrcMessage* pmsg = (CIrcMessage*)param;
	CIrcProto *ppro = pmsg->m_proto;
	if (ppro->m_nickDlg == nullptr) {
		ppro->m_nickDlg = new CNickDlg(ppro);
		ppro->m_nickDlg->Show();
	}

	SetDlgItemText(ppro->m_nickDlg->GetHwnd(), IDC_CAPTION, TranslateT("Change nickname"));
	SetDlgItemText(ppro->m_nickDlg->GetHwnd(), IDC_TEXT, pmsg->parameters.getCount() > 2 ? pmsg->parameters[2] : L"");
	ppro->m_nickDlg->m_Enick.SetText(pmsg->parameters[1]);
	ppro->m_nickDlg->m_Enick.SendMsg(CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
	return 0;
}

bool CIrcProto::OnIrc_NICK_ERR(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming) {
		if (nickflag && ((m_alternativeNick[0] != 0)) && (pmsg->parameters.getCount() > 2 && mir_wstrcmp(pmsg->parameters[1], m_alternativeNick))) {
			wchar_t m[200];
			mir_snwprintf(m, L"NICK %s", m_alternativeNick);
			if (IsConnected())
				SendIrcMessage(m);
		}
		else CallFunctionSync(sttShowNickWnd, (void*)pmsg);
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_JOINERROR(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming) {
		DBVARIANT dbv;
		if (!getWString("JTemp", &dbv)) {
			CMStringW command = L"a";
			CMStringW save = L"";
			int i = 0;

			while (!command.IsEmpty()) {
				command = GetWord(dbv.pwszVal, i);
				i++;

				if (!command.IsEmpty() && pmsg->parameters[0] == command.Mid(1))
					save += command + L" ";
			}

			db_free(&dbv);

			if (save.IsEmpty())
				db_unset(0, m_szModuleName, "JTemp");
			else
				setWString("JTemp", save);
		}
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_UNKNOWN(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 0) {
		if (pmsg->parameters[0] == L"WHO" && GetNextUserhostReason(2) != L"U")
			return true;
		if (pmsg->parameters[0] == L"USERHOST" && GetNextUserhostReason(1) != L"U")
			return true;
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_ENDMOTD(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && !bPerformDone)
		DoOnConnect(pmsg);
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_NOOFCHANNELS(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1)
		m_noOfChannels = _wtoi(pmsg->parameters[1]);

	if (pmsg->m_bIncoming && !bPerformDone)
		DoOnConnect(pmsg);

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_ERROR(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && !m_disableErrorPopups && m_iDesiredStatus != ID_STATUS_OFFLINE) {
		CMStringW S;
		if (pmsg->parameters.getCount() > 0)
			S = DoColorCodes(pmsg->parameters[0], TRUE, FALSE);
		else
			S = TranslateT("Unknown");
		Clist_TrayNotifyW(m_szModuleName, TranslateT("IRC error"), S, NIIF_ERROR, 15000);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHO_END(const CIrcMessage *pmsg)
{
	CMStringW command = GetNextUserhostReason(2);
	if (command[0] == 'S') {
		if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
			// is it a channel?
			if (IsChannel(pmsg->parameters[1])) {
				CMStringW S;
				CMStringW User = GetWord(m_whoReply, 0);
				while (!User.IsEmpty()) {
					if (GetWord(m_whoReply, 3)[0] == 'G') {
						S += User;
						S += L"\t";
						DoEvent(GC_EVENT_SETCONTACTSTATUS, pmsg->parameters[1], User, nullptr, nullptr, nullptr, ID_STATUS_AWAY, FALSE, FALSE);
					}
					else DoEvent(GC_EVENT_SETCONTACTSTATUS, pmsg->parameters[1], User, nullptr, nullptr, nullptr, ID_STATUS_ONLINE, FALSE, FALSE);

					CMStringW SS = GetWordAddress(m_whoReply, 4);
					if (SS.IsEmpty())
						break;
					m_whoReply = SS;
					User = GetWord(m_whoReply, 0);
				}

				Chat_SetStatusEx(m_szModuleName, pmsg->parameters[1], GC_SSE_TABDELIMITED, S.IsEmpty() ? nullptr : S.c_str());
				return true;
			}

			/// if it is not a channel
			ptrW UserList(mir_wstrdup(m_whoReply));
			const wchar_t* p1 = UserList;
			m_whoReply = L"";
			CONTACT ccUser = { pmsg->parameters[1], nullptr, nullptr, false, true, false };
			MCONTACT hContact = CList_FindContact(&ccUser);

			if (hContact && getByte(hContact, "AdvancedMode", 0) == 1) {
				ptrW DBHost(getWStringA(hContact, "UHost"));
				ptrW DBNick(getWStringA(hContact, "Nick"));
				ptrW DBUser(getWStringA(hContact, "UUser"));
				ptrW DBDefault(getWStringA(hContact, "Default"));
				ptrW DBManUser(getWStringA(hContact, "User"));
				ptrW DBManHost(getWStringA(hContact, "Host"));
				ptrW DBWildcard(getWStringA(hContact, "UWildcard"));
				if (DBWildcard)
					CharLower(DBWildcard);

				CMStringW nick;
				CMStringW user;
				CMStringW host;
				CMStringW away = GetWord(p1, 3);

				while (!away.IsEmpty()) {
					nick = GetWord(p1, 0);
					user = GetWord(p1, 1);
					host = GetWord(p1, 2);
					if ((DBWildcard && WCCmp(DBWildcard, nick) || DBNick && !mir_wstrcmpi(DBNick, nick) || DBDefault && !mir_wstrcmpi(DBDefault, nick))
						&& (WCCmp(DBUser, user) && WCCmp(DBHost, host))) {
						if (away[0] == 'G' && getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_AWAY)
							setWord(hContact, "Status", ID_STATUS_AWAY);
						else if (away[0] == 'H' && getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_ONLINE)
							setWord(hContact, "Status", ID_STATUS_ONLINE);

						if ((DBNick && mir_wstrcmpi(nick, DBNick)) || !DBNick)
							setWString(hContact, "Nick", nick);
						if ((DBManUser && mir_wstrcmpi(user, DBManUser)) || !DBManUser)
							setWString(hContact, "User", user);
						if ((DBManHost && mir_wstrcmpi(host, DBManHost)) || !DBManHost)
							setWString(hContact, "Host", host);
						return true;
					}
					p1 = GetWordAddress(p1, 4);
					away = GetWord(p1, 3);
				}

				if (DBWildcard && DBNick && !WCCmp(CharLower(DBWildcard), CharLower(DBNick))) {
					setWString(hContact, "Nick", DBDefault);

					DoUserhostWithReason(2, ((CMStringW)L"S" + DBWildcard), true, (wchar_t*)DBWildcard);

					setString(hContact, "User", "");
					setString(hContact, "Host", "");
					return true;
				}

				if (getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
					setWord(hContact, "Status", ID_STATUS_OFFLINE);
					setWString(hContact, "Nick", DBDefault);
					setString(hContact, "User", "");
					setString(hContact, "Host", "");
				}
			}
		}
	}
	else ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHO_REPLY(const CIrcMessage *pmsg)
{
	CMStringW command = PeekAtReasons(2);
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 6 && command[0] == 'S') {
		m_whoReply.AppendFormat(L"%s %s %s %s ", pmsg->parameters[5].c_str(), pmsg->parameters[2].c_str(), pmsg->parameters[3].c_str(), pmsg->parameters[6].c_str());
		if (mir_wstrcmpi(pmsg->parameters[5], m_info.sNick) == 0) {
			wchar_t host[1024];
			mir_wstrncpy(host, pmsg->parameters[3], 1024);
			ForkThread(&CIrcProto::ResolveIPThread, new IPRESOLVE(_T2A(host), IP_AUTO));
		}
	}

	if (command[0] == 'U')
		ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_TRYAGAIN(const CIrcMessage *pmsg)
{
	CMStringW command = L"";
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
		if (pmsg->parameters[1] == L"WHO")
			command = GetNextUserhostReason(2);

		if (pmsg->parameters[1] == L"USERHOST")
			command = GetNextUserhostReason(1);
	}
	if (command[0] == 'U')
		ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_USERHOST_REPLY(const CIrcMessage *pmsg)
{
	CMStringW command;
	if (pmsg->m_bIncoming) {
		command = GetNextUserhostReason(1);
		if (!command.IsEmpty() && command != L"U" && pmsg->parameters.getCount() > 1) {
			CONTACT finduser = { nullptr, nullptr, nullptr, false, false, false };
			int awaystatus = 0;
			CMStringW sTemp;
			CMStringW host;
			CMStringW user;
			CMStringW nick;
			CMStringW mask;
			CMStringW mess;
			CMStringW channel;

			// Status-check pre-processing: Setup check-list
			OBJLIST<CMStringW> checklist(10);
			if (command[0] == 'S') {
				sTemp = GetWord(command, 0);
				sTemp.Delete(0, 1);
				for (int j = 1; !sTemp.IsEmpty(); j++) {
					checklist.insert(new CMStringW(sTemp));
					sTemp = GetWord(command, j);
				}
			}

			// Cycle through results
			for (int j = 0;; j++) {
				sTemp = GetWord(pmsg->parameters[1], j);
				if (sTemp.IsEmpty())
					break;

				wchar_t *p1 = mir_wstrdup(sTemp);

				// Pull out host, user and nick
				wchar_t *p2 = wcschr(p1, '@');
				if (p2) {
					*p2 = 0;
					p2++;
					host = p2;
				}
				p2 = wcschr(p1, '=');
				if (p2) {
					if (*(p2 - 1) == '*')
						*(p2 - 1) = 0;  //  remove special char for IRCOps
					*p2 = 0;
					p2++;
					awaystatus = *p2;
					p2++;
					user = p2;
					nick = p1;
				}
				mess = L"";
				mask = nick + L"!" + user + L"@" + host;
				if (host.IsEmpty() || user.IsEmpty() || nick.IsEmpty()) {
					mir_free(p1);
					continue;
				}

				// Do command
				switch (command[0]) {
				case 'S': // Status check
					finduser.name = nick.GetBuffer();
					finduser.host = host.GetBuffer();
					finduser.user = user.GetBuffer();
					{
						MCONTACT hContact = CList_FindContact(&finduser);
						if (hContact && getByte(hContact, "AdvancedMode", 0) == 0) {
							setWord(hContact, "Status", awaystatus == '-' ? ID_STATUS_AWAY : ID_STATUS_ONLINE);
							setWString(hContact, "User", user);
							setWString(hContact, "Host", host);
							setWString(hContact, "Nick", nick);
	
							// If user found, remove from checklist
							for (auto &it : checklist.rev_iter())
								if (!mir_wstrcmpi(it->GetString(), nick))
									checklist.removeItem(&it);
						}
					}
					break;

				case 'I': // m_ignore
					mess = L"/IGNORE %question=\"";
					mess += TranslateT("Please enter the hostmask (nick!user@host)\nNOTE! Contacts on your contact list are never ignored");
					mess += (CMStringW)L"\",\"" + TranslateT("Ignore") + L"\",\"*!*@" + host + L"\"";
					if (m_ignoreChannelDefault)
						mess += L" +qnidcm";
					else
						mess += L" +qnidc";
					break;

				case 'J': // Unignore
					mess = L"/UNIGNORE *!*@" + host;
					break;

				case 'B': // Ban
					channel = (command.c_str() + 1);
					mess = L"/MODE " + channel + L" +b *!*@" + host;
					break;

				case 'K': // Ban & Kick
					channel = (command.c_str() + 1);
					mess.Format(L"/MODE %s +b *!*@%s%%newl/KICK %s %s", channel.c_str(), host.c_str(), channel.c_str(), nick.c_str());
					break;

				case 'L': // Ban & Kick with reason
					channel = (command.c_str() + 1);
					mess.Format(L"/MODE %s +b *!*@%s%%newl/KICK %s %s %%question=\"%s\",\"%s\",\"%s\"",
						channel.c_str(), host.c_str(), channel.c_str(), nick.c_str(),
						TranslateT("Please enter the reason"), TranslateT("Ban'n Kick"), "");
					break;
				}

				mir_free(p1);

				// Post message
				if (!mess.IsEmpty())
					PostIrcMessageWnd(nullptr, NULL, mess);
			}

			// Status-check post-processing: make buddies in ckeck-list offline
			if (command[0] == 'S') {
				for (auto &it : checklist) {
					finduser.name = it->GetBuffer();
					finduser.ExactNick = true;
					CList_SetOffline(&finduser);
				}
			}

			return true;
		}
	}

	if (!pmsg->m_bIncoming || command == L"U")
		ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_AUTH_OK(const CIrcMessage *pmsg)
{
	if (pmsg->m_bIncoming && !bPerformDone)
		DoOnConnect(pmsg);

	return true;
}

bool CIrcProto::OnIrc_AUTH_FAIL(const CIrcMessage*)
{
	int Temp = m_iDesiredStatus;
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_WRONGPASSWORD);
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)Temp, ID_STATUS_OFFLINE);
	return false;
}

bool CIrcProto::OnIrc_AUTHENTICATE(const CIrcMessage *pmsg)
{
	if (m_bUseSASL && pmsg->parameters[0] == "+") {
		CMStringA payload(FORMAT, "%S%c%S%c%s%c", m_nick, 0, m_nick, 0, m_password, 0);
		NLSend("AUTHENTICATE %s\r\n", ptrA(mir_base64_encode(payload, payload.GetLength())).get());
		NLSend("CAP END\r\n");
	}

	return true;
}

bool CIrcProto::OnIrc_CAP(const CIrcMessage *pmsg)
{
	if (pmsg->parameters.getCount() < 3)
		return true;

	if (m_bUseSASL && pmsg->parameters[1] == "ACK" && pmsg->parameters[2].Trim() == "sasl") {
		NLSend("AUTHENTICATE PLAIN\r\n");
	}
	return true;
}

bool CIrcProto::OnIrc_SUPPORT(const CIrcMessage *pmsg)
{
	static const wchar_t *lpszFmt = L"Try server %99[^ ,], port %19s";
	wchar_t szAltServer[100];
	wchar_t szAltPort[20];
	if (pmsg->parameters.getCount() > 1 && swscanf(pmsg->parameters[1], lpszFmt, &szAltServer, &szAltPort) == 2) {
		ShowMessage(pmsg);
		mir_strncpy(m_serverName, _T2A(szAltServer), 99);
		mir_strncpy(m_portStart, _T2A(szAltPort), 9);

		m_noOfChannels = 0;
		ConnectToServer();
		return true;
	}

	if (pmsg->m_bIncoming && !bPerformDone)
		DoOnConnect(pmsg);

	if (pmsg->m_bIncoming) {
		for (auto &it : pmsg->parameters) {
			wchar_t* temp = mir_wstrdup(it->GetString());
			if (wcsstr(temp, L"CHANTYPES=")) {
				wchar_t* p1 = wcschr(temp, '=');
				p1++;
				if (mir_wstrlen(p1) > 0)
					sChannelPrefixes = p1;
			}
			if (wcsstr(temp, L"CHANMODES=")) {
				wchar_t* p1 = wcschr(temp, '=');
				p1++;
				if (mir_wstrlen(p1) > 0)
					sChannelModes = (char*)_T2A(p1);
			}
			if (wcsstr(temp, L"PREFIX=")) {
				wchar_t* p1 = wcschr(temp, '(');
				wchar_t* p2 = wcschr(temp, ')');
				if (p1 && p2) {
					p1++;
					if (p1 != p2)
						sUserModes = (char*)_T2A(p1);
					sUserModes = sUserModes.Mid(0, p2 - p1);
					p2++;
					if (*p2 != 0)
						sUserModePrefixes = p2;
				}
				else {
					p1 = wcschr(temp, '=');
					p1++;
					sUserModePrefixes = p1;
					for (int n = 0; n < sUserModePrefixes.GetLength() + 1; n++) {
						if (sUserModePrefixes[n] == '@')
							sUserModes.SetAt(n, 'o');
						else if (sUserModePrefixes[n] == '+')
							sUserModes.SetAt(n, 'v');
						else if (sUserModePrefixes[n] == '-')
							sUserModes.SetAt(n, 'u');
						else if (sUserModePrefixes[n] == '%')
							sUserModes.SetAt(n, 'h');
						else if (sUserModePrefixes[n] == '!')
							sUserModes.SetAt(n, 'a');
						else if (sUserModePrefixes[n] == '*')
							sUserModes.SetAt(n, 'q');
						else if (sUserModePrefixes[n] == 0)
							sUserModes.SetAt(n, 0);
						else
							sUserModes.SetAt(n, '_');
					}
				}
			}

			mir_free(temp);
		}
	}

	ShowMessage(pmsg);
	return true;
}

void CIrcProto::OnIrcDefault(const CIrcMessage *pmsg)
{
	ShowMessage(pmsg);
}

void CIrcProto::OnIrcDisconnected()
{
	m_statusMessage = L"";
	db_unset(0, m_szModuleName, "JTemp");
	bTempDisableCheck = false;
	bTempForceCheck = false;
	m_iTempCheckTime = 0;

	m_myHost[0] = 0;

	int Temp = m_iStatus;
	KillIdent();
	KillChatTimer(OnlineNotifTimer);
	KillChatTimer(OnlineNotifTimer3);
	KillChatTimer(KeepAliveTimer);
	KillChatTimer(InitTimer);
	KillChatTimer(IdentTimer);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)Temp, ID_STATUS_OFFLINE);

	CMStringW sDisconn = L"\035\002";
	sDisconn += TranslateT("*Disconnected*");
	DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, sDisconn, nullptr, nullptr, NULL, true, false);

	Chat_Control(m_szModuleName, nullptr, SESSION_OFFLINE);

	if (!Miranda_IsTerminated())
		CList_SetAllOffline(m_disconnectDCCChats);

	// restore the original nick, cause it might be changed
	memcpy(m_nick, m_pNick, sizeof(m_nick));
	setWString("Nick", m_pNick);

	Menu_EnableItem(hMenuJoin, false);
	Menu_EnableItem(hMenuList, false);
	Menu_EnableItem(hMenuNick, false);
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnConnect

static void __stdcall sttMainThrdOnConnect(void* param)
{
	CIrcProto *ppro = (CIrcProto*)param;

	ppro->SetChatTimer(ppro->InitTimer, 1 * 1000, TimerProc);
	if (ppro->m_identTimer)
		ppro->SetChatTimer(ppro->IdentTimer, 60 * 1000, IdentTimerProc);
	if (ppro->m_sendKeepAlive)
		ppro->SetChatTimer(ppro->KeepAliveTimer, 60 * 1000, KeepAliveTimerProc);
	if (ppro->m_autoOnlineNotification && !ppro->bTempDisableCheck || ppro->bTempForceCheck) {
		ppro->SetChatTimer(ppro->OnlineNotifTimer, 1000, OnlineNotifTimerProc);
		if (ppro->m_channelAwayNotification)
			ppro->SetChatTimer(ppro->OnlineNotifTimer3, 3000, OnlineNotifTimerProc3);
	}
}

bool CIrcProto::DoOnConnect(const CIrcMessage*)
{
	bPerformDone = true;
	nickflag = true;

	Menu_ModifyItem(hMenuJoin, nullptr, INVALID_HANDLE_VALUE, 0);
	Menu_ModifyItem(hMenuList, nullptr, INVALID_HANDLE_VALUE, 0);
	Menu_ModifyItem(hMenuNick, nullptr, INVALID_HANDLE_VALUE, 0);

	int Temp = m_iStatus;
	m_iStatus = ID_STATUS_ONLINE;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)Temp, m_iStatus);

	if (m_iDesiredStatus == ID_STATUS_AWAY)
		PostIrcMessage(L"/AWAY " + m_statusMessage.Mid(0, 450));

	if (m_perform) {
		if (IsConnected()) {
			DoPerform("Event: Connect");
			switch (Temp) {
				case ID_STATUS_FREECHAT:   DoPerform("Event: Free for chat");   break;
				case ID_STATUS_ONLINE:     DoPerform("Event: Available");       break;
			}
		}
	}

	if (m_rejoinChannels) {
		int count = g_chatApi.SM_GetCount(m_szModuleName);
		for (int i = 0; i < count; i++) {
			GC_INFO gci = { 0 };
			gci.Flags = GCF_BYINDEX | GCF_DATA | GCF_NAME | GCF_TYPE;
			gci.iItem = i;
			gci.pszModule = m_szModuleName;
			if (!Chat_GetInfo(&gci) && gci.iType == GCW_CHATROOM) {
				CHANNELINFO *wi = (CHANNELINFO*)gci.pItemData;
				if (wi && wi->pszPassword)
					PostIrcMessage(L"/JOIN %s %s", gci.pszName, wi->pszPassword);
				else
					PostIrcMessage(L"/JOIN %s", gci.pszName);
			}
		}
	}

	Chat_AddGroup(m_pServer, TranslateT("Normal"));
	Chat_Control(m_szModuleName, SERVERWINDOW, SESSION_ONLINE);

	CallFunctionAsync(sttMainThrdOnConnect, this);
	nickflag = false;
	return 0;
}

void __cdecl CIrcProto::DoPerformThread(void *param)
{
	wchar_t *pwszPerform = (wchar_t *)param;
	PostIrcMessageWnd(nullptr, NULL, pwszPerform);
	mir_free(pwszPerform);
}

static void __cdecl AwayWarningThread(LPVOID)
{
	Thread_SetName("IRC: AwayWarningThread");
	MessageBox(nullptr, TranslateT("The usage of /AWAY in your perform buffer is restricted\n as IRC sends this command automatically."), TranslateT("IRC Error"), MB_OK);
}

int CIrcProto::DoPerform(const char* event)
{
	CMStringA sSetting = CMStringA("PERFORM:") + event;
	sSetting.MakeUpper();

	wchar_t *pwszPerform = getWStringA(sSetting);
	if (pwszPerform == nullptr)
		return 0;

	if (mir_wstrstri(pwszPerform, L"/away")) {
		mir_free(pwszPerform);
		mir_forkthread(AwayWarningThread);
	}
	else ForkThread(&CIrcProto::DoPerformThread, pwszPerform);

	return 1;
}

int CIrcProto::IsIgnored(const CMStringW& nick, const CMStringW& address, const CMStringW& host, char type)
{
	return IsIgnored(nick + L"!" + address + L"@" + host, type);
}

int CIrcProto::IsIgnored(CMStringW user, char type)
{
	for (int i = 0; i < m_ignoreItems.getCount(); i++) {
		const CIrcIgnoreItem &C = m_ignoreItems[i];

		if (type == 0 && !mir_wstrcmpi(user, C.mask))
			return i + 1;

		bool bUserContainsWild = (wcschr(user, '*') != nullptr || wcschr(user, '?') != nullptr);
		if (!bUserContainsWild && WCCmp(C.mask, user) || bUserContainsWild && !mir_wstrcmpi(user, C.mask)) {
			if (C.flags.IsEmpty() || C.flags[0] != '+')
				continue;

			if (!wcschr(C.flags, type))
				continue;

			return i + 1;
		}
	}

	return 0;
}

bool CIrcProto::AddIgnore(const wchar_t* mask, const wchar_t* flags)
{
	RemoveIgnore(mask);
	m_ignoreItems.insert(new CIrcIgnoreItem(mask, (L"+" + CMStringW(flags))));
	RewriteIgnoreSettings();

	if (m_ignoreDlg)
		m_ignoreDlg->Update();
	return true;
}

bool CIrcProto::RemoveIgnore(const wchar_t* mask)
{
	int idx;
	while ((idx = IsIgnored(mask, 0)) != 0)
		m_ignoreItems.remove(idx - 1);

	RewriteIgnoreSettings();

	if (m_ignoreDlg)
		m_ignoreDlg->Update();
	return true;
}
