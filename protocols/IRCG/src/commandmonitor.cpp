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

#include "irc.h"

using namespace irc;

VOID CALLBACK IdentTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CIrcProto *ppro = GetTimerOwner(idEvent);
	if (ppro == NULL)
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
	if (ppro == NULL)
		return;

	ppro->KillChatTimer(ppro->InitTimer);
	if (ppro->m_iStatus == ID_STATUS_OFFLINE || ppro->m_iStatus == ID_STATUS_CONNECTING)
		return;

	if (ppro->m_forceVisible)
		ppro->PostIrcMessage(_T("/MODE %s -i"), ppro->m_info.sNick.c_str());

	if (mir_strlen(ppro->m_myHost) == 0 && ppro->IsConnected())
		ppro->DoUserhostWithReason(2, (_T("S") + ppro->m_info.sNick).c_str(), true, _T("%s"), ppro->m_info.sNick.c_str());
}

VOID CALLBACK KeepAliveTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CIrcProto *ppro = GetTimerOwner(idEvent);
	if (ppro == NULL)
		return;

	if (!ppro->m_sendKeepAlive || (ppro->m_iStatus == ID_STATUS_OFFLINE || ppro->m_iStatus == ID_STATUS_CONNECTING)) {
		ppro->KillChatTimer(ppro->KeepAliveTimer);
		return;
	}

	TCHAR temp2[270];
	if (!ppro->m_info.sServerName.IsEmpty())
		mir_sntprintf(temp2, SIZEOF(temp2), _T("PING %s"), ppro->m_info.sServerName.c_str());
	else
		mir_sntprintf(temp2, SIZEOF(temp2), _T("PING %u"), time(0));

	if (ppro->IsConnected())
		ppro->SendIrcMessage(temp2, false);
}

VOID CALLBACK OnlineNotifTimerProc3(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CIrcProto *ppro = GetTimerOwner(idEvent);
	if (ppro == NULL)
		return;

	if (!ppro->m_channelAwayNotification ||
		ppro->m_iStatus == ID_STATUS_OFFLINE || ppro->m_iStatus == ID_STATUS_CONNECTING ||
		(!ppro->m_autoOnlineNotification && !ppro->bTempForceCheck) || ppro->bTempDisableCheck) {
		ppro->KillChatTimer(ppro->OnlineNotifTimer3);
		ppro->m_channelsToWho = _T("");
		return;
	}

	CMString name = GetWord(ppro->m_channelsToWho.c_str(), 0);
	if (name.IsEmpty()) {
		ppro->m_channelsToWho = _T("");
		int count = (int)CallServiceSync(MS_GC_GETSESSIONCOUNT, 0, (LPARAM)ppro->m_szModuleName);
		for (int i = 0; i < count; i++) {
			GC_INFO gci = { 0 };
			gci.Flags = GCF_BYINDEX | GCF_NAME | GCF_TYPE | GCF_COUNT;
			gci.iItem = i;
			gci.pszModule = ppro->m_szModuleName;
			if (!CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci) && gci.iType == GCW_CHATROOM)
			if (gci.iCount <= ppro->m_onlineNotificationLimit)
				ppro->m_channelsToWho += CMString(gci.pszName) + _T(" ");
		}
	}

	if (ppro->m_channelsToWho.IsEmpty()) {
		ppro->SetChatTimer(ppro->OnlineNotifTimer3, 60 * 1000, OnlineNotifTimerProc3);
		return;
	}

	name = GetWord(ppro->m_channelsToWho.c_str(), 0);
	ppro->DoUserhostWithReason(2, _T("S") + name, true, _T("%s"), name.c_str());
	CMString temp = GetWordAddress(ppro->m_channelsToWho.c_str(), 1);
	ppro->m_channelsToWho = temp;
	if (ppro->m_iTempCheckTime)
		ppro->SetChatTimer(ppro->OnlineNotifTimer3, ppro->m_iTempCheckTime * 1000, OnlineNotifTimerProc3);
	else
		ppro->SetChatTimer(ppro->OnlineNotifTimer3, ppro->m_onlineNotificationTime * 1000, OnlineNotifTimerProc3);
}

VOID CALLBACK OnlineNotifTimerProc(HWND, UINT, UINT_PTR idEvent, DWORD)
{
	CIrcProto *ppro = GetTimerOwner(idEvent);
	if (ppro == NULL)
		return;

	if (ppro->m_iStatus == ID_STATUS_OFFLINE || ppro->m_iStatus == ID_STATUS_CONNECTING ||
		(!ppro->m_autoOnlineNotification && !ppro->bTempForceCheck) || ppro->bTempDisableCheck) {
		ppro->KillChatTimer(ppro->OnlineNotifTimer);
		ppro->m_namesToWho = _T("");
		return;
	}

	CMString name = GetWord(ppro->m_namesToWho.c_str(), 0);
	CMString name2 = GetWord(ppro->m_namesToUserhost.c_str(), 0);

	if (name.IsEmpty() && name2.IsEmpty()) {
		DBVARIANT dbv;
		for (MCONTACT hContact = db_find_first(ppro->m_szModuleName); hContact; hContact = db_find_next(hContact, ppro->m_szModuleName)) {
			if (ppro->isChatRoom(hContact))
				continue;

			BYTE bDCC = ppro->getByte(hContact, "DCC", 0);
			BYTE bHidden = db_get_b(hContact, "CList", "Hidden", 0);
			if (bDCC || bHidden)
				continue;
			if (ppro->getTString(hContact, "Default", &dbv))
				continue;

			BYTE bAdvanced = ppro->getByte(hContact, "AdvancedMode", 0);
			if (!bAdvanced) {
				db_free(&dbv);
				if (!ppro->getTString(hContact, "Nick", &dbv)) {
					ppro->m_namesToUserhost += CMString(dbv.ptszVal) + _T(" ");
					db_free(&dbv);
				}
			}
			else {
				db_free(&dbv);
				DBVARIANT dbv2;

				TCHAR* DBNick = NULL;
				TCHAR* DBWildcard = NULL;
				if (!ppro->getTString(hContact, "Nick", &dbv))
					DBNick = dbv.ptszVal;
				if (!ppro->getTString(hContact, "UWildcard", &dbv2))
					DBWildcard = dbv2.ptszVal;

				if (DBNick && (!DBWildcard || !WCCmp(CharLower(DBWildcard), CharLower(DBNick))))
					ppro->m_namesToWho += CMString(DBNick) + _T(" ");
				else if (DBWildcard)
					ppro->m_namesToWho += CMString(DBWildcard) + _T(" ");

				if (DBNick)     db_free(&dbv);
				if (DBWildcard) db_free(&dbv2);
			}
		}
	}

	if (ppro->m_namesToWho.IsEmpty() && ppro->m_namesToUserhost.IsEmpty()) {
		ppro->SetChatTimer(ppro->OnlineNotifTimer, 60 * 1000, OnlineNotifTimerProc);
		return;
	}

	name = GetWord(ppro->m_namesToWho.c_str(), 0);
	name2 = GetWord(ppro->m_namesToUserhost.c_str(), 0);
	CMString temp;
	if (!name.IsEmpty()) {
		ppro->DoUserhostWithReason(2, _T("S") + name, true, _T("%s"), name.c_str());
		temp = GetWordAddress(ppro->m_namesToWho.c_str(), 1);
		ppro->m_namesToWho = temp;
	}

	if (!name2.IsEmpty()) {
		CMString params;
		for (int i = 0; i < 3; i++) {
			params = _T("");
			for (int j = 0; j < 5; j++)
				params += GetWord(ppro->m_namesToUserhost, i * 5 + j) + _T(" ");

			if (params[0] != ' ')
				ppro->DoUserhostWithReason(1, CMString(_T("S")) + params, true, params);
		}
		temp = GetWordAddress(ppro->m_namesToUserhost.c_str(), 15);
		ppro->m_namesToUserhost = temp;
	}

	if (ppro->m_iTempCheckTime)
		ppro->SetChatTimer(ppro->OnlineNotifTimer, ppro->m_iTempCheckTime * 1000, OnlineNotifTimerProc);
	else
		ppro->SetChatTimer(ppro->OnlineNotifTimer, ppro->m_onlineNotificationTime * 1000, OnlineNotifTimerProc);
}

int CIrcProto::AddOutgoingMessageToDB(MCONTACT hContact, TCHAR* msg)
{
	if (m_iStatus == ID_STATUS_OFFLINE || m_iStatus == ID_STATUS_CONNECTING)
		return 0;

	CMString S = DoColorCodes(msg, TRUE, FALSE);

	DBEVENTINFO dbei = { sizeof(dbei) };
	dbei.szModule = m_szModuleName;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.timestamp = (DWORD)time(NULL);
	dbei.flags = DBEF_SENT + DBEF_UTF;
	dbei.pBlob = (PBYTE)mir_utf8encodeW(S.c_str());
	dbei.cbBlob = (DWORD)strlen((char*)dbei.pBlob) + 1;
	db_event_add(hContact, &dbei);
	mir_free(dbei.pBlob);
	return 1;
}

void __cdecl CIrcProto::ResolveIPThread(LPVOID di)
{
	IPRESOLVE* ipr = (IPRESOLVE *)di;
	{
		mir_cslock lock(m_resolve);

		if (ipr != NULL && (ipr->iType == IP_AUTO && mir_strlen(m_myHost) == 0 || ipr->iType == IP_MANUAL)) {
			hostent* myhost = gethostbyname(ipr->sAddr.c_str());
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

bool CIrcProto::OnIrc_PING(const CIrcMessage* pmsg)
{
	TCHAR szResponse[100];
	mir_sntprintf(szResponse, SIZEOF(szResponse), _T("PONG %s"), pmsg->parameters[0].c_str());
	SendIrcMessage(szResponse);
	return false;
}

bool CIrcProto::OnIrc_WELCOME(const CIrcMessage* pmsg)
{
	if (pmsg->parameters[0] != m_info.sNick)
		m_info.sNick = pmsg->parameters[0];

	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
		static TCHAR host[1024];
		int i = 0;
		CMString word = GetWord(pmsg->parameters[1].c_str(), i);
		while (!word.IsEmpty()) {
			if (_tcschr(word.c_str(), '!') && _tcschr(word.c_str(), '@')) {
				mir_tstrncpy(host, word.c_str(), SIZEOF(host));
				TCHAR* p1 = _tcschr(host, '@');
				if (p1)
					ForkThread(&CIrcProto::ResolveIPThread, new IPRESOLVE(_T2A(p1 + 1), IP_AUTO));
			}

			word = GetWord(pmsg->parameters[1].c_str(), ++i);
		}
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOTOOLONG(const CIrcMessage* pmsg)
{
	CMString command = GetNextUserhostReason(2);
	if (command[0] == 'U')
		ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_BACKFROMAWAY(const CIrcMessage* pmsg)
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

bool CIrcProto::OnIrc_SETAWAY(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming) {
		int Temp = m_iDesiredStatus;
		m_iStatus = m_iDesiredStatus = ID_STATUS_AWAY;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)Temp, ID_STATUS_AWAY);

		if (m_perform) {
			switch (m_iStatus) {
			case ID_STATUS_AWAY:
				DoPerform("Event: Away");
				break;
			case ID_STATUS_NA:
				DoPerform("Event: N/A");
				break;
			case ID_STATUS_DND:
				DoPerform("Event: DND");
				break;
			case ID_STATUS_OCCUPIED:
				DoPerform("Event: Occupied");
				break;
			case ID_STATUS_OUTTOLUNCH:
				DoPerform("Event: Out for lunch");
				break;
			case ID_STATUS_ONTHEPHONE:
				DoPerform("Event: On the phone");
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

bool CIrcProto::OnIrc_JOIN(const CIrcMessage* pmsg)
{
	if (pmsg->parameters.getCount() > 0 && pmsg->m_bIncoming && pmsg->prefix.sNick != m_info.sNick) {
		CMString host = pmsg->prefix.sUser + _T("@") + pmsg->prefix.sHost;
		DoEvent(GC_EVENT_JOIN, pmsg->parameters[0].c_str(), pmsg->prefix.sNick.c_str(), NULL, _T("Normal"), host.c_str(), NULL, true, false);
		DoEvent(GC_EVENT_SETCONTACTSTATUS, pmsg->parameters[0].c_str(), pmsg->prefix.sNick.c_str(), NULL, NULL, NULL, ID_STATUS_ONLINE, FALSE, FALSE);
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_QUIT(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming) {
		CMString host = pmsg->prefix.sUser + _T("@") + pmsg->prefix.sHost;
		DoEvent(GC_EVENT_QUIT, NULL, pmsg->prefix.sNick.c_str(), pmsg->parameters.getCount() > 0 ? pmsg->parameters[0].c_str() : NULL, NULL, host.c_str(), NULL, true, false);
		struct CONTACT user = { (LPTSTR)pmsg->prefix.sNick.c_str(), (LPTSTR)pmsg->prefix.sUser.c_str(), (LPTSTR)pmsg->prefix.sHost.c_str(), false, false, false };
		CList_SetOffline(&user);
		if (pmsg->prefix.sNick == m_info.sNick) {
			GCDEST gcd = { m_szModuleName, NULL, GC_EVENT_CONTROL };
			GCEVENT gce = { sizeof(gce), &gcd };
			CallChatEvent(SESSION_OFFLINE, (LPARAM)&gce);
		}
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_PART(const CIrcMessage* pmsg)
{
	if (pmsg->parameters.getCount() > 0 && pmsg->m_bIncoming) {
		CMString host = pmsg->prefix.sUser + _T("@") + pmsg->prefix.sHost;
		DoEvent(GC_EVENT_PART, pmsg->parameters[0].c_str(), pmsg->prefix.sNick.c_str(), pmsg->parameters.getCount() > 1 ? pmsg->parameters[1].c_str() : NULL, NULL, host.c_str(), NULL, true, false);
		if (pmsg->prefix.sNick == m_info.sNick) {
			CMString S = MakeWndID(pmsg->parameters[0].c_str());
			GCDEST gcd = { m_szModuleName, S.c_str(), GC_EVENT_CONTROL };
			GCEVENT gce = { sizeof(gce), &gcd };
			CallChatEvent(SESSION_OFFLINE, (LPARAM)&gce);
		}
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_KICK(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1)
		DoEvent(GC_EVENT_KICK, pmsg->parameters[0].c_str(), pmsg->parameters[1].c_str(), pmsg->parameters.getCount() > 2 ? pmsg->parameters[2].c_str() : NULL, pmsg->prefix.sNick.c_str(), NULL, NULL, true, false);
	else
		ShowMessage(pmsg);

	if (pmsg->parameters[1] == m_info.sNick) {
		CMString S = MakeWndID(pmsg->parameters[0].c_str());
		GCDEST gcd = { m_szModuleName, S.c_str(), GC_EVENT_CONTROL };
		GCEVENT gce = { sizeof(gce), &gcd };
		CallChatEvent(SESSION_OFFLINE, (LPARAM)&gce);

		if (m_rejoinIfKicked) {
			CHANNELINFO *wi = (CHANNELINFO *)DoEvent(GC_EVENT_GETITEMDATA, pmsg->parameters[0].c_str(), NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, 0);
			if (wi && wi->pszPassword)
				PostIrcMessage(_T("/JOIN %s %s"), pmsg->parameters[0].c_str(), wi->pszPassword);
			else
				PostIrcMessage(_T("/JOIN %s"), pmsg->parameters[0].c_str());
		}
	}

	return true;
}

bool CIrcProto::OnIrc_MODEQUERY(const CIrcMessage* pmsg)
{
	if (pmsg->parameters.getCount() > 2 && pmsg->m_bIncoming && IsChannel(pmsg->parameters[1])) {
		CMString sPassword = _T("");
		CMString sLimit = _T("");
		bool bAdd = false;
		int iParametercount = 3;

		LPCTSTR p1 = pmsg->parameters[2].c_str();
		while (*p1 != '\0') {
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

		AddWindowItemData(pmsg->parameters[1].c_str(), sLimit.IsEmpty() ? 0 : sLimit.c_str(), pmsg->parameters[2].c_str(), sPassword.IsEmpty() ? 0 : sPassword.c_str(), 0);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_MODE(const CIrcMessage* pmsg)
{
	bool flag = false;
	bool bContainsValidModes = false;
	CMString sModes = _T("");
	CMString sParams = _T("");

	if (pmsg->parameters.getCount() > 1 && pmsg->m_bIncoming) {
		if (IsChannel(pmsg->parameters[0])) {
			bool bAdd = false;
			int  iParametercount = 2;
			LPCTSTR p1 = pmsg->parameters[1].c_str();

			while (*p1 != '\0') {
				if (*p1 == '+') {
					bAdd = true;
					sModes += _T("+");
				}
				if (*p1 == '-') {
					bAdd = false;
					sModes += _T("-");
				}
				if (*p1 == 'l' && bAdd && iParametercount < (int)pmsg->parameters.getCount()) {
					bContainsValidModes = true;
					sModes += _T("l");
					sParams += _T(" ") + pmsg->parameters[iParametercount];
					iParametercount++;
				}
				if (*p1 == 'b' || *p1 == 'k' && iParametercount < (int)pmsg->parameters.getCount()) {
					bContainsValidModes = true;
					sModes += *p1;
					sParams += _T(" ") + pmsg->parameters[iParametercount];
					iParametercount++;
				}
				if (strchr(sUserModes.c_str(), (char)*p1)) {
					CMString sStatus = ModeToStatus(*p1);
					if ((int)pmsg->parameters.getCount() > iParametercount) {
						if (!_tcscmp(pmsg->parameters[2].c_str(), m_info.sNick.c_str())) {
							char cModeBit = -1;
							CHANNELINFO *wi = (CHANNELINFO *)DoEvent(GC_EVENT_GETITEMDATA, pmsg->parameters[0].c_str(), NULL, NULL, NULL, NULL, NULL, false, false, 0);
							switch (*p1) {
							case 'v':      cModeBit = 0;       break;
							case 'h':      cModeBit = 1;       break;
							case 'o':      cModeBit = 2;       break;
							case 'a':      cModeBit = 3;       break;
							case 'q':      cModeBit = 4;       break;
							}

							// set bit for own mode on this channel (voice/hop/op/admin/owner)
							if (bAdd && cModeBit >= 0)
								wi->OwnMode |= (1 << cModeBit);
							else
								wi->OwnMode &= ~(1 << cModeBit);

							DoEvent(GC_EVENT_SETITEMDATA, pmsg->parameters[0].c_str(), NULL, NULL, NULL, NULL, (DWORD_PTR)wi, false, false, 0);
						}
						DoEvent(bAdd ? GC_EVENT_ADDSTATUS : GC_EVENT_REMOVESTATUS, pmsg->parameters[0].c_str(), pmsg->parameters[iParametercount].c_str(), pmsg->prefix.sNick.c_str(), sStatus.c_str(), NULL, NULL, m_oldStyleModes ? false : true, false);
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
				TCHAR temp[256];
				mir_sntprintf(temp, SIZEOF(temp), TranslateT("%s sets mode %s"),
					pmsg->prefix.sNick.c_str(), pmsg->parameters[1].c_str());

				CMString sMessage = temp;
				for (int i = 2; i < (int)pmsg->parameters.getCount(); i++)
					sMessage += _T(" ") + pmsg->parameters[i];

				DoEvent(GC_EVENT_INFORMATION, pmsg->parameters[0].c_str(), pmsg->prefix.sNick.c_str(), sMessage.c_str(), NULL, NULL, NULL, true, false);
			}
			else if (bContainsValidModes) {
				for (int i = iParametercount; i < (int)pmsg->parameters.getCount(); i++)
					sParams += _T(" ") + pmsg->parameters[i];

				TCHAR temp[4000];
				mir_sntprintf(temp, SIZEOF(temp), TranslateT("%s sets mode %s%s"), pmsg->prefix.sNick.c_str(), sModes.c_str(), sParams.c_str());
				DoEvent(GC_EVENT_INFORMATION, pmsg->parameters[0].c_str(), pmsg->prefix.sNick.c_str(), temp, NULL, NULL, NULL, true, false);
			}

			if (flag)
				PostIrcMessage(_T("/MODE %s"), pmsg->parameters[0].c_str());
		}
		else {
			TCHAR temp[256];
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("%s sets mode %s"), pmsg->prefix.sNick.c_str(), pmsg->parameters[1].c_str());

			CMString sMessage = temp;
			for (int i = 2; i < (int)pmsg->parameters.getCount(); i++)
				sMessage += _T(" ") + pmsg->parameters[i];

			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, pmsg->prefix.sNick.c_str(), sMessage.c_str(), NULL, NULL, NULL, true, false);
		}
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_NICK(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 0) {
		bool bIsMe = pmsg->prefix.sNick.c_str() == m_info.sNick ? true : false;

		if (m_info.sNick == pmsg->prefix.sNick && pmsg->parameters.getCount() > 0) {
			m_info.sNick = pmsg->parameters[0];
			setTString("Nick", m_info.sNick.c_str());
		}

		CMString host = pmsg->prefix.sUser + _T("@") + pmsg->prefix.sHost;
		DoEvent(GC_EVENT_NICK, NULL, pmsg->prefix.sNick.c_str(), pmsg->parameters[0].c_str(), NULL, host.c_str(), NULL, true, bIsMe);
		DoEvent(GC_EVENT_CHUID, NULL, pmsg->prefix.sNick.c_str(), pmsg->parameters[0].c_str(), NULL, NULL, NULL, true, false);

		struct CONTACT user = { (TCHAR*)pmsg->prefix.sNick.c_str(), (TCHAR*)pmsg->prefix.sUser.c_str(), (TCHAR*)pmsg->prefix.sHost.c_str(), false, false, false };
		MCONTACT hContact = CList_FindContact(&user);
		if (hContact) {
			if (getWord(hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
				setWord(hContact, "Status", ID_STATUS_ONLINE);
			setTString(hContact, "Nick", pmsg->parameters[0].c_str());
			setTString(hContact, "User", pmsg->prefix.sUser.c_str());
			setTString(hContact, "Host", pmsg->prefix.sHost.c_str());
		}
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_NOTICE(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
		if (IsCTCP(pmsg))
			return true;

		if (!m_ignore || !IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'n')) {
			CMString S;
			CMString S2;
			CMString S3;
			if (pmsg->prefix.sNick.GetLength() > 0)
				S = pmsg->prefix.sNick;
			else
				S = m_info.sNetwork;
			S3 = m_info.sNetwork;
			if (IsChannel(pmsg->parameters[0]))
				S2 = pmsg->parameters[0].c_str();
			else {
				GC_INFO gci = { 0 };
				gci.Flags = GCF_BYID | GCF_TYPE;
				gci.pszModule = m_szModuleName;

				CMString S3 = GetWord(pmsg->parameters[1].c_str(), 0);
				if (S3[0] == '[' && S3[1] == '#' && S3[S3.GetLength() - 1] == ']') {
					S3.Delete(S3.GetLength() - 1, 1);
					S3.Delete(0, 1);
					CMString Wnd = MakeWndID(S3.c_str());
					gci.pszID = Wnd.c_str();
					if (!CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci) && gci.iType == GCW_CHATROOM)
						S2 = GetWord(gci.pszID, 0);
					else
						S2 = _T("");
				}
				else S2 = _T("");
			}
			DoEvent(GC_EVENT_NOTICE, S2.IsEmpty() ? 0 : S2.c_str(), S.c_str(), pmsg->parameters[1].c_str(), NULL, S3.c_str(), NULL, true, false);
		}
	}
	else ShowMessage(pmsg);

	return true;
}

bool CIrcProto::OnIrc_YOURHOST(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming) {
		static const TCHAR* lpszFmt = _T("Your host is %99[^ \x5b,], running version %99s");
		TCHAR szHostName[100], szVersion[100];
		if (_stscanf(pmsg->parameters[1].c_str(), lpszFmt, &szHostName, &szVersion) > 0)
			m_info.sServerName = szHostName;
		if (pmsg->parameters[0] != m_info.sNick)
			m_info.sNick = pmsg->parameters[0];
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_INVITE(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && (m_ignore && IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'i')))
		return true;

	if (pmsg->m_bIncoming && m_joinOnInvite && pmsg->parameters.getCount() > 1 && mir_tstrcmpi(pmsg->parameters[0].c_str(), m_info.sNick.c_str()) == 0)
		PostIrcMessage(_T("/JOIN %s"), pmsg->parameters[1].c_str());

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_PINGPONG(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->sCommand == _T("PING")) {
		TCHAR szResponse[100];
		mir_sntprintf(szResponse, SIZEOF(szResponse), _T("PONG %s"), pmsg->parameters[0].c_str());
		SendIrcMessage(szResponse);
	}

	return true;
}

bool CIrcProto::OnIrc_PRIVMSG(const CIrcMessage* pmsg)
{
	if (pmsg->parameters.getCount() > 1) {
		if (IsCTCP(pmsg))
			return true;

		CMString mess = pmsg->parameters[1];
		bool bIsChannel = IsChannel(pmsg->parameters[0]);

		if (pmsg->m_bIncoming && !bIsChannel) {
			mess = DoColorCodes(mess.c_str(), TRUE, FALSE);

			struct CONTACT user = { (TCHAR*)pmsg->prefix.sNick.c_str(), (TCHAR*)pmsg->prefix.sUser.c_str(), (TCHAR*)pmsg->prefix.sHost.c_str(), false, false, false };

			if (CallService(MS_IGNORE_ISIGNORED, NULL, IGNOREEVENT_MESSAGE))
			if (!CList_FindContact(&user))
				return true;

			if ((m_ignore && IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'q'))) {
				MCONTACT hContact = CList_FindContact(&user);
				if (!hContact || (hContact && db_get_b(hContact, "CList", "Hidden", 0) == 1))
					return true;
			}

			MCONTACT hContact = CList_AddContact(&user, false, true);

			PROTORECVEVENT pre = { 0 };
			pre.timestamp = (DWORD)time(NULL);
			pre.flags = PREF_UTF;
			pre.szMessage = mir_utf8encodeW(mess.c_str());
			setTString(hContact, "User", pmsg->prefix.sUser.c_str());
			setTString(hContact, "Host", pmsg->prefix.sHost.c_str());
			ProtoChainRecvMsg(hContact, &pre);
			mir_free(pre.szMessage);
			return true;
		}

		if (bIsChannel) {
			if (!(pmsg->m_bIncoming && m_ignore && IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'm'))) {
				if (!pmsg->m_bIncoming)
					mess.Replace(_T("%%"), _T("%"));
				DoEvent(GC_EVENT_MESSAGE, pmsg->parameters[0].c_str(), pmsg->m_bIncoming ? pmsg->prefix.sNick.c_str() : m_info.sNick.c_str(), mess.c_str(), NULL, NULL, NULL, true, pmsg->m_bIncoming ? false : true);
			}
			return true;
		}
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::IsCTCP(const CIrcMessage* pmsg)
{
	// is it a ctcp command, i e is the first and last characer of a PRIVMSG or NOTICE text ASCII 1
	CMString mess = pmsg->parameters[1];
	if (!(mess.GetLength() > 3 && mess[0] == 1 && mess[mess.GetLength() - 1] == 1))
		return false;

	// set mess to contain the ctcp command, excluding the leading and trailing  ASCII 1
	mess.Delete(0, 1);
	mess.Delete(mess.GetLength() - 1, 1);

	// exploit???
	if (mess.Find(1) != -1 || mess.Find(_T("%newl")) != -1) {
		TCHAR temp[4096];
		mir_sntprintf(temp, SIZEOF(temp), TranslateT("CTCP ERROR: Malformed CTCP command received from %s!%s@%s. Possible attempt to take control of your IRC client registered"), pmsg->prefix.sNick.c_str(), pmsg->prefix.sUser.c_str(), pmsg->prefix.sHost.c_str());
		DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), temp, NULL, NULL, NULL, true, false);
		return true;
	}

	// extract the type of ctcp command
	CMString ocommand = GetWord(mess.c_str(), 0);
	CMString command = GetWord(mess.c_str(), 0);
	command.MakeLower();

	// should it be ignored?
	if (m_ignore) {
		if (IsChannel(pmsg->parameters[0])) {
			if (command == _T("action") && IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'm'))
				return true;
		}
		else {
			if (command == _T("action")) {
				if (IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'q'))
					return true;
			}
			else if (command == _T("dcc")) {
				if (IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'd'))
					return true;
			}
			else if (IsIgnored(pmsg->prefix.sNick, pmsg->prefix.sUser, pmsg->prefix.sHost, 'c'))
				return true;
		}
	}

	if (pmsg->sCommand == _T("PRIVMSG")) {
		// incoming ACTION
		if (command == _T("action")) {
			mess.Delete(0, 6);

			if (IsChannel(pmsg->parameters[0])) {
				if (mess.GetLength() > 1) {
					mess.Delete(0, 1);
					if (!pmsg->m_bIncoming)
						mess.Replace(_T("%%"), _T("%"));

					DoEvent(GC_EVENT_ACTION, pmsg->parameters[0].c_str(), pmsg->m_bIncoming ? pmsg->prefix.sNick.c_str() : m_info.sNick.c_str(), mess.c_str(), NULL, NULL, NULL, true, pmsg->m_bIncoming ? false : true);
				}
			}
			else if (pmsg->m_bIncoming) {
				mess.Insert(0, pmsg->prefix.sNick.c_str());
				mess.Insert(0, _T("* "));
				mess.Insert(mess.GetLength(), _T(" *"));
				CIrcMessage msg = *pmsg;
				msg.parameters[1] = mess;
				OnIrc_PRIVMSG(&msg);
			}
		}
		// incoming FINGER
		else if (pmsg->m_bIncoming && command == _T("finger")) {
			PostIrcMessage(_T("/NOTICE %s \001FINGER %s (%s)\001"), pmsg->prefix.sNick.c_str(), m_name, m_userID);

			TCHAR temp[300];
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("CTCP FINGER requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, temp, NULL, NULL, NULL, true, false);
		}

		// incoming VERSION
		else if (pmsg->m_bIncoming && command == _T("version")) {
			PostIrcMessage(_T("/NOTICE %s \001VERSION Miranda NG %%mirver (IRC v.%%version), (c) 2003-2014 J.Persson, G.Hazan\001"), pmsg->prefix.sNick.c_str());

			TCHAR temp[300];
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("CTCP VERSION requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, temp, NULL, NULL, NULL, true, false);
		}

		// incoming SOURCE
		else if (pmsg->m_bIncoming && command == _T("source")) {
			PostIrcMessage(_T("/NOTICE %s \001SOURCE Get Miranda IRC here: http://miranda-ng.org/ \001"), pmsg->prefix.sNick.c_str());

			TCHAR temp[300];
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("CTCP SOURCE requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, temp, NULL, NULL, NULL, true, false);
		}

		// incoming USERINFO
		else if (pmsg->m_bIncoming && command == _T("userinfo")) {
			PostIrcMessage(_T("/NOTICE %s \001USERINFO %s\001"), pmsg->prefix.sNick.c_str(), m_userInfo);

			TCHAR temp[300];
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("CTCP USERINFO requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, temp, NULL, NULL, NULL, true, false);
		}

		// incoming PING
		else if (pmsg->m_bIncoming && command == _T("ping")) {
			PostIrcMessage(_T("/NOTICE %s \001%s\001"), pmsg->prefix.sNick.c_str(), mess.c_str());

			TCHAR temp[300];
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("CTCP PING requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, temp, NULL, NULL, NULL, true, false);
		}

		// incoming TIME
		else if (pmsg->m_bIncoming && command == _T("time")) {
			TCHAR temp[300];
			time_t tim = time(NULL);
			mir_tstrncpy(temp, _tctime(&tim), 25);
			PostIrcMessage(_T("/NOTICE %s \001TIME %s\001"), pmsg->prefix.sNick.c_str(), temp);

			mir_sntprintf(temp, SIZEOF(temp), TranslateT("CTCP TIME requested by %s"), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, temp, NULL, NULL, NULL, true, false);
		}

		// incoming DCC request... lots of stuff happening here...
		else if (pmsg->m_bIncoming && command == _T("dcc")) {
			CMString type = GetWord(mess.c_str(), 1);
			type.MakeLower();

			// components of a dcc message
			CMString sFile = _T("");
			DWORD dwAdr = 0;
			int iPort = 0;
			unsigned __int64 dwSize = 0;
			CMString sToken = _T("");
			bool bIsChat = (type == _T("chat"));

			// 1. separate the dcc command into the correct pieces
			if (bIsChat || type == _T("send")) {
				// if the filename is surrounded by quotes, do this
				if (GetWord(mess.c_str(), 2)[0] == '\"') {
					int end = 0;
					int begin = mess.Find('\"', 0);
					if (begin >= 0) {
						end = mess.Find('\"', begin + 1);
						if (end >= 0) {
							sFile = mess.Mid(begin + 1, end - begin - 1);

							begin = mess.Find(' ', end);
							if (begin >= 0) {
								CMString rest = mess.Mid(begin, mess.GetLength());
								dwAdr = _tcstoul(GetWord(rest.c_str(), 0).c_str(), NULL, 10);
								iPort = _ttoi(GetWord(rest.c_str(), 1).c_str());
								dwSize = _ttoi64(GetWord(rest.c_str(), 2).c_str());
								sToken = GetWord(rest.c_str(), 3);
							}
						}
					}
				}
				// ... or try another method of separating the dcc command
				else if (!GetWord(mess.c_str(), (bIsChat) ? 4 : 5).IsEmpty()) {
					int index = (bIsChat) ? 4 : 5;
					bool bFlag = false;

					// look for the part of the ctcp command that contains adress, port and size
					while (!bFlag && !GetWord(mess.c_str(), index).IsEmpty()) {
						CMString sTemp;

						if (type == _T("chat"))
							sTemp = GetWord(mess.c_str(), index - 1) + GetWord(mess.c_str(), index);
						else
							sTemp = GetWord(mess.c_str(), index - 2) + GetWord(mess.c_str(), index - 1) + GetWord(mess.c_str(), index);

						// if all characters are number it indicates we have found the adress, port and size parameters
						int ind = 0;
						while (sTemp[ind] != '\0') {
							if (!_istdigit(sTemp[ind]))
								break;
							ind++;
						}

						if (sTemp[ind] == '\0' && GetWord(mess.c_str(), index + ((bIsChat) ? 1 : 2)).IsEmpty())
							bFlag = true;
						index++;
					}

					if (bFlag) {
						TCHAR* p1 = _tcsdup(GetWordAddress(mess.c_str(), 2));
						TCHAR* p2 = (TCHAR*)GetWordAddress(p1, index - 5);

						if (type == _T("send")) {
							if (p2 > p1) {
								p2--;
								while (p2 != p1 && *p2 == ' ') {
									*p2 = '\0';
									p2--;
								}
								sFile = p1;
							}
						}
						else sFile = _T("chat");

						free(p1);

						dwAdr = _tcstoul(GetWord(mess.c_str(), index - (bIsChat ? 2 : 3)).c_str(), NULL, 10);
						iPort = _ttoi(GetWord(mess.c_str(), index - (bIsChat ? 1 : 2)).c_str());
						dwSize = _ttoi64(GetWord(mess.c_str(), index - 1).c_str());
						sToken = GetWord(mess.c_str(), index);
					}
				}
			}
			else if (type == _T("accept") || type == _T("resume")) {
				// if the filename is surrounded by quotes, do this
				if (GetWord(mess.c_str(), 2)[0] == '\"') {
					int end = 0;
					int begin = mess.Find('\"', 0);
					if (begin >= 0) {
						end = mess.Find('\"', begin + 1);
						if (end >= 0) {
							sFile = mess.Mid(begin + 1, end);

							begin = mess.Find(' ', end);
							if (begin >= 0) {
								CMString rest = mess.Mid(begin, mess.GetLength());
								iPort = _ttoi(GetWord(rest.c_str(), 0).c_str());
								dwSize = _ttoi(GetWord(rest.c_str(), 1).c_str());
								sToken = GetWord(rest.c_str(), 2);
							}
						}
					}
				}
				// ... or try another method of separating the dcc command
				else if (!GetWord(mess.c_str(), 4).IsEmpty()) {
					int index = 4;
					bool bFlag = false;

					// look for the part of the ctcp command that contains adress, port and size
					while (!bFlag && !GetWord(mess.c_str(), index).IsEmpty()) {
						CMString sTemp = GetWord(mess.c_str(), index - 1) + GetWord(mess.c_str(), index);

						// if all characters are number it indicates we have found the adress, port and size parameters
						int ind = 0;

						while (sTemp[ind] != '\0') {
							if (!_istdigit(sTemp[ind]))
								break;
							ind++;
						}

						if (sTemp[ind] == '\0' && GetWord(mess.c_str(), index + 2).IsEmpty())
							bFlag = true;
						index++;
					}
					if (bFlag) {
						TCHAR* p1 = _tcsdup(GetWordAddress(mess.c_str(), 2));
						TCHAR* p2 = (TCHAR*)GetWordAddress(p1, index - 4);

						if (p2 > p1) {
							p2--;
							while (p2 != p1 && *p2 == ' ') {
								*p2 = '\0';
								p2--;
							}
							sFile = p1;
						}

						free(p1);

						iPort = _ttoi(GetWord(mess.c_str(), index - 2).c_str());
						dwSize = _ttoi64(GetWord(mess.c_str(), index - 1).c_str());
						sToken = GetWord(mess.c_str(), index);
					}
				}
			}
			// end separating dcc commands

			// 2. Check for malformed dcc commands or other errors
			if (bIsChat || type == _T("send")) {
				TCHAR szTemp[256];
				szTemp[0] = '\0';

				unsigned long ulAdr = 0;
				if (m_manualHost)
					ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
				else
					ulAdr = ConvertIPToInteger(m_IPFromServer ? m_myHost : m_myLocalHost);

				if (bIsChat && !m_DCCChatEnabled)
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC: Chat request from %s denied"), pmsg->prefix.sNick.c_str());

				else if (type == _T("send") && !m_DCCFileEnabled)
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC: File transfer request from %s denied"), pmsg->prefix.sNick.c_str());

				else if (type == _T("send") && !iPort && ulAdr == 0)
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC: Reverse file transfer request from %s denied [No local IP]"), pmsg->prefix.sNick.c_str());

				if (sFile.IsEmpty() || dwAdr == 0 || dwSize == 0 || iPort == 0 && sToken.IsEmpty())
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC ERROR: Malformed CTCP request from %s [%s]"), pmsg->prefix.sNick.c_str(), mess.c_str());

				if (szTemp[0]) {
					DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);
					return true;
				}

				// remove path from the filename if the remote client (stupidly) sent it
				CMString sFileCorrected = sFile;
				int i = sFile.ReverseFind('\\');
				if (i != -1)
					sFileCorrected = sFile.Mid(i + 1, sFile.GetLength());
				sFile = sFileCorrected;
			}
			else if (type == _T("accept") || type == _T("resume")) {
				TCHAR szTemp[256];
				szTemp[0] = '\0';

				if (type == _T("resume") && !m_DCCFileEnabled)
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC: File transfer resume request from %s denied"), pmsg->prefix.sNick.c_str());

				if (sToken.IsEmpty() && iPort == 0 || sFile.IsEmpty())
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC ERROR: Malformed CTCP request from %s [%s]"), pmsg->prefix.sNick.c_str(), mess.c_str());

				if (szTemp[0]) {
					DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);
					return true;
				}

				// remove path from the filename if the remote client (stupidly) sent it
				CMString sFileCorrected = sFile;
				int i = sFile.ReverseFind('\\');
				if (i != -1)
					sFileCorrected = sFile.Mid(i + 1, sFile.GetLength());
				sFile = sFileCorrected;
			}

			// 3. Take proper actions considering type of command

			// incoming chat request
			if (bIsChat) {
				CONTACT user = { (TCHAR*)pmsg->prefix.sNick.c_str(), 0, 0, false, false, true };
				MCONTACT hContact = CList_FindContact(&user);

				// check if it should be ignored
				if (m_DCCChatIgnore == 1 ||
					m_DCCChatIgnore == 2 && hContact &&
					db_get_b(hContact, "CList", "NotOnList", 0) == 0 &&
					db_get_b(hContact, "CList", "Hidden", 0) == 0) {
					CMString host = pmsg->prefix.sUser + _T("@") + pmsg->prefix.sHost;
					CList_AddDCCChat(pmsg->prefix.sNick, host, dwAdr, iPort); // add a CHAT event to the clist
				}
				else {
					TCHAR szTemp[512];
					mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("DCC: Chat request from %s denied"), pmsg->prefix.sNick.c_str());
					DoEvent(GC_EVENT_INFORMATION, 0, m_info.sNick.c_str(), szTemp, NULL, NULL, NULL, true, false);
				}
			}

			// remote requested that the file should be resumed
			if (type == _T("resume")) {
				CDccSession* dcc;
				if (sToken.IsEmpty())
					dcc = FindDCCSendByPort(iPort);
				else
					dcc = FindPassiveDCCSend(_ttoi(sToken.c_str())); // reverse ft

				if (dcc) {
					InterlockedExchange(&dcc->dwWhatNeedsDoing, (long)FILERESUME_RESUME);
					dcc->dwResumePos = dwSize; // dwSize is the resume position
					PostIrcMessage(_T("/PRIVMSG %s \001DCC ACCEPT %s\001"), pmsg->prefix.sNick.c_str(), GetWordAddress(mess.c_str(), 2));
				}
			}

			// remote accepted your request for a file resume
			if (type == _T("accept")) {
				CDccSession* dcc;
				if (sToken.IsEmpty())
					dcc = FindDCCRecvByPortAndName(iPort, pmsg->prefix.sNick.c_str());
				else
					dcc = FindPassiveDCCRecv(pmsg->prefix.sNick, sToken); // reverse ft

				if (dcc) {
					InterlockedExchange(&dcc->dwWhatNeedsDoing, (long)FILERESUME_RESUME);
					dcc->dwResumePos = dwSize;	// dwSize is the resume position					
					SetEvent(dcc->hEvent);
				}
			}

			if (type == _T("send")) {
				CMString sTokenBackup = sToken;
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
					CDccSession* dcc = FindPassiveDCCSend(_ttoi(sToken.c_str()));
					if (dcc) {
						dcc->SetupPassive(dwAdr, iPort);
						dcc->Connect();
					}
				}
				else {
					struct CONTACT user = { (TCHAR*)pmsg->prefix.sNick.c_str(), (TCHAR*)pmsg->prefix.sUser.c_str(), (TCHAR*)pmsg->prefix.sHost.c_str(), false, false, false };
					if (CallService(MS_IGNORE_ISIGNORED, NULL, IGNOREEVENT_FILE))
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

						setTString(hContact, "User", pmsg->prefix.sUser.c_str());
						setTString(hContact, "Host", pmsg->prefix.sHost.c_str());

						TCHAR* tszTemp = (TCHAR*)sFile.c_str();

						PROTORECVFILET pre = { 0 };
						pre.flags = PREF_TCHAR;
						pre.timestamp = (DWORD)time(NULL);
						pre.fileCount = 1;
						pre.ptszFiles = &tszTemp;
						pre.lParam = (LPARAM)di;
						ProtoChainRecvFile(hContact, &pre);
					}
				}
			}
			// end type == "send"
		}
		else if (pmsg->m_bIncoming) {
			TCHAR temp[300];
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("CTCP %s requested by %s"), ocommand.c_str(), pmsg->prefix.sNick.c_str());
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, temp, NULL, NULL, NULL, true, false);
		}
	}

	// handle incoming ctcp in notices. This technique is used for replying to CTCP queries
	else if (pmsg->sCommand == _T("NOTICE")) {
		TCHAR szTemp[300];
		szTemp[0] = '\0';

		//if we got incoming CTCP Version for contact in CList - then write its as MirVer for that contact!
		if (pmsg->m_bIncoming && command == _T("version")) {
			struct CONTACT user = { (TCHAR*)pmsg->prefix.sNick.c_str(), (TCHAR*)pmsg->prefix.sUser.c_str(), (TCHAR*)pmsg->prefix.sHost.c_str(), false, false, false };
			MCONTACT hContact = CList_FindContact(&user);
			if (hContact)
				setTString(hContact, "MirVer", DoColorCodes(GetWordAddress(mess.c_str(), 1), TRUE, FALSE));
		}

		// if the whois window is visible and the ctcp reply belongs to the user in it, then show the reply in the whois window
		if (m_whoisDlg && IsWindowVisible(m_whoisDlg->GetHwnd())) {
			m_whoisDlg->m_InfoNick.GetText(szTemp, SIZEOF(szTemp));
			if (mir_tstrcmpi(szTemp, pmsg->prefix.sNick.c_str()) == 0) {
				if (pmsg->m_bIncoming && (command == _T("version") || command == _T("userinfo") || command == _T("time"))) {
					SetActiveWindow(m_whoisDlg->GetHwnd());
					m_whoisDlg->m_Reply.SetText(DoColorCodes(GetWordAddress(mess.c_str(), 1), TRUE, FALSE));
					return true;
				}
				if (pmsg->m_bIncoming && command == _T("ping")) {
					SetActiveWindow(m_whoisDlg->GetHwnd());
					int s = (int)time(0) - (int)_ttol(GetWordAddress(mess.c_str(), 1));
					TCHAR szTemp[30];
					if (s == 1)
						mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%u second"), s);
					else
						mir_sntprintf(szTemp, SIZEOF(szTemp), _T("%u seconds"), s);

					m_whoisDlg->m_Reply.SetText(DoColorCodes(szTemp, TRUE, FALSE));
					return true;
				}
			}
		}

		//... else show the reply in the current window
		if (pmsg->m_bIncoming && command == _T("ping")) {
			int s = (int)time(0) - (int)_ttol(GetWordAddress(mess.c_str(), 1));
			mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("CTCP PING reply from %s: %u sec(s)"), pmsg->prefix.sNick.c_str(), s);
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, szTemp, NULL, NULL, NULL, true, false);
		}
		else {
			mir_sntprintf(szTemp, SIZEOF(szTemp), TranslateT("CTCP %s reply from %s: %s"), ocommand.c_str(), pmsg->prefix.sNick.c_str(), GetWordAddress(mess.c_str(), 1));
			DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, szTemp, NULL, NULL, NULL, true, false);
		}
	}

	return true;
}

bool CIrcProto::OnIrc_NAMES(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 3)
		sNamesList += pmsg->parameters[3] + _T(" ");
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_ENDNAMES(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
		CMString name = _T("a");
		int i = 0;
		BOOL bFlag = false;

		// Is the user on the names list?
		while (!name.IsEmpty()) {
			name = GetWord(sNamesList.c_str(), i);
			i++;
			if (!name.IsEmpty()) {
				int index = 0;
				while (_tcschr(sUserModePrefixes.c_str(), name[index]))
					index++;

				if (!mir_tstrcmpi(name.Mid(index, name.GetLength()).c_str(), m_info.sNick.c_str())) {
					bFlag = true;
					break;
				}
			}
		}

		if (bFlag) {
			const TCHAR* sChanName = pmsg->parameters[1].c_str();
			if (sChanName[0] == '@' || sChanName[0] == '*' || sChanName[0] == '=')
				sChanName++;

			// Add a new chat window
			CMString sID = MakeWndID(sChanName);
			BYTE btOwnMode = 0;

			GCSESSION gcw = { sizeof(gcw) };
			gcw.iType = GCW_CHATROOM;
			gcw.ptszID = sID.c_str();
			gcw.pszModule = m_szModuleName;
			gcw.ptszName = sChanName;
			if (!CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw)) {
				DBVARIANT dbv;
				GCDEST gcd = { m_szModuleName, sID.c_str(), GC_EVENT_ADDGROUP };
				GCEVENT gce = { sizeof(gce), &gcd };

				PostIrcMessage(_T("/MODE %s"), sChanName);

				// register the statuses
				gce.ptszStatus = _T("Owner");
				CallChatEvent(0, (LPARAM)&gce);
				gce.ptszStatus = _T("Admin");
				CallChatEvent(0, (LPARAM)&gce);
				gce.ptszStatus = _T("Op");
				CallChatEvent(0, (LPARAM)&gce);
				gce.ptszStatus = _T("Halfop");
				CallChatEvent(0, (LPARAM)&gce);
				gce.ptszStatus = _T("Voice");
				CallChatEvent(0, (LPARAM)&gce);
				gce.ptszStatus = _T("Normal");
				CallChatEvent(0, (LPARAM)&gce);

				int i = 0;
				CMString sTemp = GetWord(sNamesList.c_str(), i);

				// Fill the nicklist
				while (!sTemp.IsEmpty()) {
					CMString sStat;
					CMString sTemp2 = sTemp;
					sStat = PrefixToStatus(sTemp[0]);

					// fix for networks like freshirc where they allow more than one prefix
					while (PrefixToStatus(sTemp[0]) != _T("Normal"))
						sTemp.Delete(0, 1);

					gcd.iType = GC_EVENT_JOIN;
					gce.ptszUID = sTemp.c_str();
					gce.ptszNick = sTemp.c_str();
					gce.ptszStatus = sStat.c_str();
					BOOL bIsMe = (!mir_tstrcmpi(gce.ptszNick, m_info.sNick.c_str())) ? TRUE : FALSE;
					if (bIsMe) {
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
					gce.bIsMe = bIsMe;
					gce.time = bIsMe ? time(0) : 0;
					CallChatEvent(0, (LPARAM)&gce);
					DoEvent(GC_EVENT_SETCONTACTSTATUS, sChanName, sTemp.c_str(), NULL, NULL, NULL, ID_STATUS_ONLINE, FALSE, FALSE);
					// fix for networks like freshirc where they allow more than one prefix
					if (PrefixToStatus(sTemp2[0]) != _T("Normal")) {
						sTemp2.Delete(0, 1);
						sStat = PrefixToStatus(sTemp2[0]);
						while (sStat != _T("Normal")) {
							DoEvent(GC_EVENT_ADDSTATUS, sID.c_str(), sTemp.c_str(), _T("system"), sStat.c_str(), NULL, NULL, false, false, 0);
							sTemp2.Delete(0, 1);
							sStat = PrefixToStatus(sTemp2[0]);
						}
					}

					i++;
					sTemp = GetWord(sNamesList.c_str(), i);
				}

				//Set the item data for the window
				{
					CHANNELINFO *wi = (CHANNELINFO *)DoEvent(GC_EVENT_GETITEMDATA, sChanName, NULL, NULL, NULL, NULL, NULL, FALSE, FALSE, 0);
					if (!wi)
						wi = new CHANNELINFO;
					wi->OwnMode = btOwnMode;
					wi->pszLimit = 0;
					wi->pszMode = 0;
					wi->pszPassword = 0;
					wi->pszTopic = 0;
					wi->codepage = getCodepage();
					DoEvent(GC_EVENT_SETITEMDATA, sChanName, NULL, NULL, NULL, NULL, (DWORD_PTR)wi, false, false, 0);

					if (!sTopic.IsEmpty() && !mir_tstrcmpi(GetWord(sTopic.c_str(), 0).c_str(), sChanName)) {
						DoEvent(GC_EVENT_TOPIC, sChanName, sTopicName.IsEmpty() ? NULL : sTopicName.c_str(), GetWordAddress(sTopic.c_str(), 1), NULL, sTopicTime.IsEmpty() ? NULL : sTopicTime.c_str(), NULL, true, false);
						AddWindowItemData(sChanName, 0, 0, 0, GetWordAddress(sTopic.c_str(), 1));
						sTopic = _T("");
						sTopicName = _T("");
						sTopicTime = _T("");
					}	}

				gcd.ptszID = (TCHAR*)sID.c_str();
				gcd.iType = GC_EVENT_CONTROL;
				gce.cbSize = sizeof(GCEVENT);
				gce.dwFlags = 0;
				gce.bIsMe = false;
				gce.dwItemData = false;
				gce.ptszNick = NULL;
				gce.ptszStatus = NULL;
				gce.ptszText = NULL;
				gce.ptszUID = NULL;
				gce.ptszUserInfo = NULL;
				gce.time = time(0);
				gce.pDest = &gcd;

				if (!getTString("JTemp", &dbv)) {
					CMString command = _T("a");
					CMString save = _T("");
					int i = 0;

					while (!command.IsEmpty()) {
						command = GetWord(dbv.ptszVal, i);
						i++;
						if (!command.IsEmpty()) {
							CMString S = command.Mid(1, command.GetLength());
							if (!mir_tstrcmpi(sChanName, S.c_str()))
								break;

							save += command + _T(" ");
						}
					}

					if (!command.IsEmpty()) {
						save += GetWordAddress(dbv.ptszVal, i);
						switch (command[0]) {
						case 'M':
							CallChatEvent(WINDOW_HIDDEN, (LPARAM)&gce);
							break;
						case 'X':
							CallChatEvent(WINDOW_MAXIMIZE, (LPARAM)&gce);
							break;
						default:
							CallChatEvent(SESSION_INITDONE, (LPARAM)&gce);
							break;
						}
					}
					else CallChatEvent(SESSION_INITDONE, (LPARAM)&gce);

					if (save.IsEmpty())
						db_unset(NULL, m_szModuleName, "JTemp");
					else
						setTString("JTemp", save.c_str());
					db_free(&dbv);
				}
				else CallChatEvent(SESSION_INITDONE, (LPARAM)&gce);

				gcd.iType = GC_EVENT_CONTROL;
				gce.pDest = &gcd;
				CallChatEvent(SESSION_ONLINE, (LPARAM)&gce);
			}
		}
	}

	sNamesList = _T("");
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_INITIALTOPIC(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 2) {
		AddWindowItemData(pmsg->parameters[1].c_str(), 0, 0, 0, pmsg->parameters[2].c_str());
		sTopic = pmsg->parameters[1] + _T(" ") + pmsg->parameters[2];
		sTopicName = _T("");
		sTopicTime = _T("");
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_INITIALTOPICNAME(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 3) {
		TCHAR tTimeBuf[128], *tStopStr;
		time_t ttTopicTime;
		sTopicName = pmsg->parameters[2];
		ttTopicTime = _tcstol(pmsg->parameters[3].c_str(), &tStopStr, 10);
		_tcsftime(tTimeBuf, 128, _T("%#c"), localtime(&ttTopicTime));
		sTopicTime = tTimeBuf;
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_TOPIC(const CIrcMessage* pmsg)
{
	if (pmsg->parameters.getCount() > 1 && pmsg->m_bIncoming) {
		DoEvent(GC_EVENT_TOPIC, pmsg->parameters[0].c_str(), pmsg->prefix.sNick.c_str(), pmsg->parameters[1].c_str(), NULL, sTopicTime.IsEmpty() ? NULL : sTopicTime.c_str(), NULL, true, false);
		AddWindowItemData(pmsg->parameters[0].c_str(), 0, 0, 0, pmsg->parameters[1].c_str());
	}
	ShowMessage(pmsg);
	return true;
}

static void __stdcall sttShowDlgList(void* param)
{
	CIrcProto *ppro = (CIrcProto*)param;
	if (ppro->m_listDlg == NULL) {
		ppro->m_listDlg = new CListDlg(ppro);
		ppro->m_listDlg->Show();
	}
	SetEvent(ppro->m_evWndCreate);
}

bool CIrcProto::OnIrc_LISTSTART(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming) {
		CallFunctionAsync(sttShowDlgList, this);
		WaitForSingleObject(m_evWndCreate, INFINITE);
		m_channelNumber = 0;
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_LIST(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming == 1 && m_listDlg && pmsg->parameters.getCount() > 2) {
		m_channelNumber++;
		LVITEM lvItem;
		HWND hListView = GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW);
		lvItem.iItem = ListView_GetItemCount(hListView);
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iSubItem = 0;
		lvItem.pszText = (TCHAR*)pmsg->parameters[1].c_str();
		lvItem.lParam = lvItem.iItem;
		lvItem.iItem = ListView_InsertItem(hListView, &lvItem);
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 1;
		lvItem.pszText = (TCHAR*)pmsg->parameters[pmsg->parameters.getCount() - 2].c_str();
		ListView_SetItem(hListView, &lvItem);

		TCHAR* temp = mir_tstrdup(pmsg->parameters[pmsg->parameters.getCount() - 1]);
		TCHAR* find = _tcsstr(temp, _T("[+"));
		TCHAR* find2 = _tcsstr(temp, _T("]"));
		TCHAR* save = temp;
		if (find == temp && find2 != NULL && find + 8 >= find2) {
			temp = _tcsstr(temp, _T("]"));
			if (mir_tstrlen(temp) > 1) {
				temp++;
				temp[0] = '\0';
				lvItem.iSubItem = 2;
				lvItem.pszText = save;
				ListView_SetItem(hListView, &lvItem);
				temp[0] = ' ';
				temp++;
			}
			else temp = save;
		}

		lvItem.iSubItem = 3;
		CMString S = DoColorCodes(temp, TRUE, FALSE);
		lvItem.pszText = (TCHAR*)S.c_str();
		ListView_SetItem(hListView, &lvItem);
		temp = save;
		mir_free(temp);

		int percent = 100;
		if (m_noOfChannels > 0)
			percent = (int)(m_channelNumber * 100) / m_noOfChannels;

		TCHAR text[100];
		if (percent < 100)
			mir_sntprintf(text, SIZEOF(text), TranslateT("Downloading list (%u%%) - %u channels"), percent, m_channelNumber);
		else
			mir_sntprintf(text, SIZEOF(text), TranslateT("Downloading list - %u channels"), m_channelNumber);
		m_listDlg->m_status.SetText(text);
	}

	return true;
}

bool CIrcProto::OnIrc_LISTEND(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && m_listDlg) {
		EnableWindow(GetDlgItem(m_listDlg->GetHwnd(), IDC_JOIN), true);
		ListView_SetSelectionMark(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW), 0);
		ListView_SetColumnWidth(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW), 1, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW), 2, LVSCW_AUTOSIZE);
		ListView_SetColumnWidth(GetDlgItem(m_listDlg->GetHwnd(), IDC_INFO_LISTVIEW), 3, LVSCW_AUTOSIZE);
		m_listDlg->UpdateList();

		TCHAR text[100];
		mir_sntprintf(text, SIZEOF(text), TranslateT("Done: %u channels"), m_channelNumber);
		int percent = 100;
		if (m_noOfChannels > 0)
			percent = (int)(m_channelNumber * 100) / m_noOfChannels;
		if (percent < 70) {
			mir_tstrcat(text, _T(" "));
			mir_tstrcat(text, TranslateT("(probably truncated by server)"));
		}
		SetDlgItemText(m_listDlg->GetHwnd(), IDC_TEXT, text);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_BANLIST(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 2) {
		if (m_managerDlg->GetHwnd() && (
			m_managerDlg->m_radio1.GetState() && pmsg->sCommand == _T("367") ||
			m_managerDlg->m_radio2.GetState() && pmsg->sCommand == _T("346") ||
			m_managerDlg->m_radio3.GetState() && pmsg->sCommand == _T("348")) &&
			!m_managerDlg->m_radio1.Enabled() && !m_managerDlg->m_radio2.Enabled() && !m_managerDlg->m_radio3.Enabled()) {
			CMString S = pmsg->parameters[2];
			if (pmsg->parameters.getCount() > 3) {
				S += _T("   - ");
				S += pmsg->parameters[3];
				if (pmsg->parameters.getCount() > 4) {
					S += _T(" -  ( ");
					time_t time = StrToInt(pmsg->parameters[4].c_str());
					S += _tctime(&time);
					S.Replace(_T("\n"), _T(" "));
					S += _T(")");
				}
			}

			SendDlgItemMessage(m_managerDlg->GetHwnd(), IDC_LIST, LB_ADDSTRING, 0, (LPARAM)S.c_str());
		}
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_BANLISTEND(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
		if (m_managerDlg->GetHwnd() &&
			(m_managerDlg->m_radio1.GetState() && pmsg->sCommand == _T("368")
			|| m_managerDlg->m_radio2.GetState() && pmsg->sCommand == _T("347")
			|| m_managerDlg->m_radio3.GetState() && pmsg->sCommand == _T("349")) &&
			!m_managerDlg->m_radio1.Enabled() && !m_managerDlg->m_radio2.Enabled() && !m_managerDlg->m_radio3.Enabled()) {
			if (strchr(sChannelModes.c_str(), 'b'))
				m_managerDlg->m_radio1.Enable();
			if (strchr(sChannelModes.c_str(), 'I'))
				m_managerDlg->m_radio2.Enable();
			if (strchr(sChannelModes.c_str(), 'e'))
				m_managerDlg->m_radio3.Enable();
			if (BST_UNCHECKED == IsDlgButtonChecked(m_managerDlg->GetHwnd(), IDC_NOTOP))
				m_managerDlg->m_add.Enable();
		}
	}

	ShowMessage(pmsg);
	return true;
}

static void __stdcall sttShowWhoisWnd(void* param)
{
	CIrcMessage* pmsg = (CIrcMessage*)param;
	CIrcProto *ppro = (CIrcProto*)pmsg->m_proto;
	if (ppro->m_whoisDlg == NULL) {
		ppro->m_whoisDlg = new CWhoisDlg(ppro);
		ppro->m_whoisDlg->Show();
	}
	SetEvent(ppro->m_evWndCreate);

	ppro->m_whoisDlg->ShowMessage(pmsg);
	delete pmsg;
}

bool CIrcProto::OnIrc_WHOIS_NAME(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 5 && m_manualWhoisCount > 0) {
		CallFunctionAsync(sttShowWhoisWnd, new CIrcMessage(*pmsg));
		WaitForSingleObject(m_evWndCreate, INFINITE);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_CHANNELS(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0)
		m_whoisDlg->m_InfoChannels.SetText(pmsg->parameters[2].c_str());
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_AWAY(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0)
		m_whoisDlg->m_InfoAway2.SetText(pmsg->parameters[2].c_str());
	if (m_manualWhoisCount < 1 && pmsg->m_bIncoming && pmsg->parameters.getCount() > 2)
		WhoisAwayReply = pmsg->parameters[2];
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_OTHER(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0) {
		TCHAR temp[1024], temp2[1024];
		m_whoisDlg->m_InfoOther.GetText(temp, 1000);
		mir_tstrcat(temp, _T("%s\r\n"));
		mir_sntprintf(temp2, SIZEOF(temp2), temp, pmsg->parameters[2].c_str());
		m_whoisDlg->m_InfoOther.SetText(temp2);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_END(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1 && m_manualWhoisCount < 1) {
		CONTACT user = { (TCHAR*)pmsg->parameters[1].c_str(), NULL, NULL, false, false, true };
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

bool CIrcProto::OnIrc_WHOIS_IDLE(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0) {
		int S = StrToInt(pmsg->parameters[2].c_str());
		int D = S / (60 * 60 * 24);
		S -= (D * 60 * 60 * 24);
		int H = S / (60 * 60);
		S -= (H * 60 * 60);
		int M = S / 60;
		S -= (M * 60);

		TCHAR temp[100];
		if (D)
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("%ud, %uh, %um, %us"), D, H, M, S);
		else if (H)
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("%uh, %um, %us"), H, M, S);
		else if (M)
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("%um, %us"), M, S);
		else if (S)
			mir_sntprintf(temp, SIZEOF(temp), TranslateT("%us"), S);
		else
			temp[0] = 0;

		TCHAR temp3[256];
		TCHAR tTimeBuf[128], *tStopStr;
		time_t ttTime = _tcstol(pmsg->parameters[3].c_str(), &tStopStr, 10);
		_tcsftime(tTimeBuf, 128, _T("%c"), localtime(&ttTime));
		mir_sntprintf(temp3, SIZEOF(temp3), TranslateT("online since %s, idle %s"), tTimeBuf, temp);
		m_whoisDlg->m_AwayTime.SetText(temp3);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_SERVER(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0)
		m_whoisDlg->m_InfoServer.SetText(pmsg->parameters[2].c_str());
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_AUTH(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && m_whoisDlg && pmsg->parameters.getCount() > 2 && m_manualWhoisCount > 0) {
		if (pmsg->sCommand == _T("330"))
			m_whoisDlg->m_InfoAuth.SetText(pmsg->parameters[2].c_str());
		else if (pmsg->parameters[2] == _T("is an identified user") || pmsg->parameters[2] == _T("is a registered nick"))
			m_whoisDlg->m_InfoAuth.SetText(pmsg->parameters[2].c_str());
		else
			OnIrc_WHOIS_OTHER(pmsg);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHOIS_NO_USER(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 2 && !IsChannel(pmsg->parameters[1])) {
		if (m_whoisDlg)
			m_whoisDlg->ShowMessageNoUser(pmsg);

		CONTACT user = { (TCHAR*)pmsg->parameters[1].c_str(), NULL, NULL, false, false, false };
		MCONTACT hContact = CList_FindContact(&user);
		if (hContact) {
			AddOutgoingMessageToDB(hContact, (TCHAR*)((CMString)_T("> ") + pmsg->parameters[2] + (CMString)_T(": ") + pmsg->parameters[1]).c_str());

			DBVARIANT dbv;
			if (!getTString(hContact, "Default", &dbv)) {
				setTString(hContact, "Nick", dbv.ptszVal);

				DBVARIANT dbv2;
				if (getByte(hContact, "AdvancedMode", 0) == 0)
					DoUserhostWithReason(1, ((CMString)_T("S") + dbv.ptszVal).c_str(), true, dbv.ptszVal);
				else {
					if (!getTString(hContact, "UWildcard", &dbv2)) {
						DoUserhostWithReason(2, ((CMString)_T("S") + dbv2.ptszVal).c_str(), true, dbv2.ptszVal);
						db_free(&dbv2);
					}
					else DoUserhostWithReason(2, ((CMString)_T("S") + dbv.ptszVal).c_str(), true, dbv.ptszVal);
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

static void __stdcall sttShowNickWnd(void* param)
{
	CIrcMessage* pmsg = (CIrcMessage*)param;
	CIrcProto *ppro = pmsg->m_proto;
	if (ppro->m_nickDlg == NULL) {
		ppro->m_nickDlg = new CNickDlg(ppro);
		ppro->m_nickDlg->Show();
	}
	SetEvent(ppro->m_evWndCreate);
	SetDlgItemText(ppro->m_nickDlg->GetHwnd(), IDC_CAPTION, TranslateT("Change nickname"));
	SetDlgItemText(ppro->m_nickDlg->GetHwnd(), IDC_TEXT, pmsg->parameters.getCount() > 2 ? pmsg->parameters[2].c_str() : _T(""));
	ppro->m_nickDlg->m_Enick.SetText(pmsg->parameters[1].c_str());
	ppro->m_nickDlg->m_Enick.SendMsg(CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
	delete pmsg;
}

bool CIrcProto::OnIrc_NICK_ERR(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming) {
		if (nickflag && ((m_alternativeNick[0] != 0)) && (pmsg->parameters.getCount() > 2 && _tcscmp(pmsg->parameters[1].c_str(), m_alternativeNick))) {
			TCHAR m[200];
			mir_sntprintf(m, SIZEOF(m), _T("NICK %s"), m_alternativeNick);
			if (IsConnected())
				SendIrcMessage(m);
		}
		else {
			CallFunctionAsync(sttShowNickWnd, new CIrcMessage(*pmsg));
			WaitForSingleObject(m_evWndCreate, INFINITE);
		}
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_JOINERROR(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming) {
		DBVARIANT dbv;
		if (!getTString("JTemp", &dbv)) {
			CMString command = _T("a");
			CMString save = _T("");
			int i = 0;

			while (!command.IsEmpty()) {
				command = GetWord(dbv.ptszVal, i);
				i++;

				if (!command.IsEmpty() && pmsg->parameters[0] == command.Mid(1, command.GetLength()))
					save += command + _T(" ");
			}

			db_free(&dbv);

			if (save.IsEmpty())
				db_unset(NULL, m_szModuleName, "JTemp");
			else
				setTString("JTemp", save.c_str());
		}
	}

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_UNKNOWN(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 0) {
		if (pmsg->parameters[0] == _T("WHO") && GetNextUserhostReason(2) != _T("U"))
			return true;
		if (pmsg->parameters[0] == _T("USERHOST") && GetNextUserhostReason(1) != _T("U"))
			return true;
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_ENDMOTD(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && !bPerformDone)
		DoOnConnect(pmsg);
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_NOOFCHANNELS(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1)
		m_noOfChannels = StrToInt(pmsg->parameters[1].c_str());

	if (pmsg->m_bIncoming && !bPerformDone)
		DoOnConnect(pmsg);

	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_ERROR(const CIrcMessage* pmsg)
{
	if (pmsg->m_bIncoming && !m_disableErrorPopups && m_iDesiredStatus != ID_STATUS_OFFLINE) {
		MIRANDASYSTRAYNOTIFY msn;
		msn.cbSize = sizeof(MIRANDASYSTRAYNOTIFY);
		msn.szProto = m_szModuleName;
		msn.tszInfoTitle = TranslateT("IRC error");

		CMString S;
		if (pmsg->parameters.getCount() > 0)
			S = DoColorCodes(pmsg->parameters[0].c_str(), TRUE, FALSE);
		else
			S = TranslateT("Unknown");

		msn.tszInfo = (TCHAR*)S.c_str();
		msn.dwInfoFlags = NIIF_ERROR | NIIF_INTERN_UNICODE;
		msn.uTimeout = 15000;
		CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&msn);
	}
	ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHO_END(const CIrcMessage* pmsg)
{
	CMString command = GetNextUserhostReason(2);
	if (command[0] == 'S') {
		if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
			// is it a channel?
			if (IsChannel(pmsg->parameters[1])) {
				CMString S;
				CMString User = GetWord(m_whoReply.c_str(), 0);
				while (!User.IsEmpty()) {
					if (GetWord(m_whoReply.c_str(), 3)[0] == 'G') {
						S += User;
						S += _T("\t");
						DoEvent(GC_EVENT_SETCONTACTSTATUS, pmsg->parameters[1].c_str(), User.c_str(), NULL, NULL, NULL, ID_STATUS_AWAY, FALSE, FALSE);
					}
					else DoEvent(GC_EVENT_SETCONTACTSTATUS, pmsg->parameters[1].c_str(), User.c_str(), NULL, NULL, NULL, ID_STATUS_ONLINE, FALSE, FALSE);

					CMString SS = GetWordAddress(m_whoReply.c_str(), 4);
					if (SS.IsEmpty())
						break;
					m_whoReply = SS;
					User = GetWord(m_whoReply.c_str(), 0);
				}

				DoEvent(GC_EVENT_SETSTATUSEX, pmsg->parameters[1].c_str(), NULL, S.IsEmpty() ? NULL : S.c_str(), NULL, NULL, GC_SSE_TABDELIMITED, FALSE, FALSE);
				return true;
			}

			/// if it is not a channel
			ptrT UserList(mir_tstrdup(m_whoReply.c_str()));
			const TCHAR* p1 = UserList;
			m_whoReply = _T("");
			CONTACT user = { (TCHAR*)pmsg->parameters[1].c_str(), NULL, NULL, false, true, false };
			MCONTACT hContact = CList_FindContact(&user);

			if (hContact && getByte(hContact, "AdvancedMode", 0) == 1) {
				ptrT DBHost(getTStringA(hContact, "UHost"));
				ptrT DBNick(getTStringA(hContact, "Nick"));
				ptrT DBUser(getTStringA(hContact, "UUser"));
				ptrT DBDefault(getTStringA(hContact, "Default"));
				ptrT DBManUser(getTStringA(hContact, "User"));
				ptrT DBManHost(getTStringA(hContact, "Host"));
				ptrT DBWildcard(getTStringA(hContact, "UWildcard"));
				if (DBWildcard)
					CharLower(DBWildcard);

				CMString nick;
				CMString user;
				CMString host;
				CMString away = GetWord(p1, 3);

				while (!away.IsEmpty()) {
					nick = GetWord(p1, 0);
					user = GetWord(p1, 1);
					host = GetWord(p1, 2);
					if ((DBWildcard && WCCmp(DBWildcard, nick.c_str()) || DBNick && !mir_tstrcmpi(DBNick, nick.c_str()) || DBDefault && !mir_tstrcmpi(DBDefault, nick.c_str()))
						&& (WCCmp(DBUser, user.c_str()) && WCCmp(DBHost, host.c_str()))) {
						if (away[0] == 'G' && getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_AWAY)
							setWord(hContact, "Status", ID_STATUS_AWAY);
						else if (away[0] == 'H' && getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_ONLINE)
							setWord(hContact, "Status", ID_STATUS_ONLINE);

						if ((DBNick && mir_tstrcmpi(nick.c_str(), DBNick)) || !DBNick)
							setTString(hContact, "Nick", nick.c_str());
						if ((DBManUser && mir_tstrcmpi(user.c_str(), DBManUser)) || !DBManUser)
							setTString(hContact, "User", user.c_str());
						if ((DBManHost && mir_tstrcmpi(host.c_str(), DBManHost)) || !DBManHost)
							setTString(hContact, "Host", host.c_str());
						return true;
					}
					p1 = GetWordAddress(p1, 4);
					away = GetWord(p1, 3);
				}

				if (DBWildcard && DBNick && !WCCmp(CharLower(DBWildcard), CharLower(DBNick))) {
					setTString(hContact, "Nick", DBDefault);

					DoUserhostWithReason(2, ((CMString)_T("S") + DBWildcard).c_str(), true, (TCHAR*)DBWildcard);

					setString(hContact, "User", "");
					setString(hContact, "Host", "");
					return true;
				}

				if (getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE) {
					setWord(hContact, "Status", ID_STATUS_OFFLINE);
					setTString(hContact, "Nick", DBDefault);
					setString(hContact, "User", "");
					setString(hContact, "Host", "");
				}
			}
		}
	}
	else ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_WHO_REPLY(const CIrcMessage* pmsg)
{
	CMString command = PeekAtReasons(2);
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 6 && command[0] == 'S') {
		m_whoReply.AppendFormat(_T("%s %s %s %s "), pmsg->parameters[5].c_str(), pmsg->parameters[2].c_str(), pmsg->parameters[3].c_str(), pmsg->parameters[6].c_str());
		if (mir_tstrcmpi(pmsg->parameters[5].c_str(), m_info.sNick.c_str()) == 0) {
			TCHAR host[1024];
			mir_tstrncpy(host, pmsg->parameters[3].c_str(), 1024);
			ForkThread(&CIrcProto::ResolveIPThread, new IPRESOLVE(_T2A(host), IP_AUTO));
		}
	}

	if (command[0] == 'U')
		ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_TRYAGAIN(const CIrcMessage* pmsg)
{
	CMString command = _T("");
	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 1) {
		if (pmsg->parameters[1] == _T("WHO"))
			command = GetNextUserhostReason(2);

		if (pmsg->parameters[1] == _T("USERHOST"))
			command = GetNextUserhostReason(1);
	}
	if (command[0] == 'U')
		ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_USERHOST_REPLY(const CIrcMessage* pmsg)
{
	CMString command;
	if (pmsg->m_bIncoming) {
		command = GetNextUserhostReason(1);
		if (!command.IsEmpty() && command != _T("U") && pmsg->parameters.getCount() > 1) {
			CONTACT finduser = { NULL, NULL, NULL, false, false, false };
			int awaystatus = 0;
			CMString sTemp;
			CMString host;
			CMString user;
			CMString nick;
			CMString mask;
			CMString mess;
			CMString channel;

			// Status-check pre-processing: Setup check-list
			OBJLIST<CMString> checklist(10);
			if (command[0] == 'S') {
				sTemp = GetWord(command.c_str(), 0);
				sTemp.Delete(0, 1);
				for (int j = 1; !sTemp.IsEmpty(); j++) {
					checklist.insert(new CMString(sTemp));
					sTemp = GetWord(command.c_str(), j);
				}
			}

			// Cycle through results
			for (int j = 0;; j++) {
				sTemp = GetWord(pmsg->parameters[1].c_str(), j);
				if (sTemp.IsEmpty())
					break;

				TCHAR *p1 = mir_tstrdup(sTemp.c_str());

				// Pull out host, user and nick
				TCHAR *p2 = _tcschr(p1, '@');
				if (p2) {
					*p2 = '\0';
					p2++;
					host = p2;
				}
				p2 = _tcschr(p1, '=');
				if (p2) {
					if (*(p2 - 1) == '*')
						*(p2 - 1) = '\0';  //  remove special char for IRCOps
					*p2 = '\0';
					p2++;
					awaystatus = *p2;
					p2++;
					user = p2;
					nick = p1;
				}
				mess = _T("");
				mask = nick + _T("!") + user + _T("@") + host;
				if (host.IsEmpty() || user.IsEmpty() || nick.IsEmpty()) {
					mir_free(p1);
					continue;
				}

				// Do command
				switch (command[0]) {
				case 'S': // Status check
					finduser.name = (TCHAR*)nick.c_str();
					finduser.host = (TCHAR*)host.c_str();
					finduser.user = (TCHAR*)user.c_str();
					{
						MCONTACT hContact = CList_FindContact(&finduser);
						if (hContact && getByte(hContact, "AdvancedMode", 0) == 0) {
							setWord(hContact, "Status", awaystatus == '-' ? ID_STATUS_AWAY : ID_STATUS_ONLINE);
							setTString(hContact, "User", user.c_str());
							setTString(hContact, "Host", host.c_str());
							setTString(hContact, "Nick", nick.c_str());
	
							// If user found, remove from checklist
							for (int i = 0; i < checklist.getCount(); i++)
								if (!mir_tstrcmpi(checklist[i].c_str(), nick.c_str()))
									checklist.remove(i);
						}
					}
					break;

				case 'I': // m_ignore
					mess = _T("/IGNORE %question=\"");
					mess += TranslateT("Please enter the hostmask (nick!user@host)\nNOTE! Contacts on your contact list are never ignored");
					mess += (CMString)_T("\",\"") + TranslateT("Ignore") + _T("\",\"*!*@") + host + _T("\"");
					if (m_ignoreChannelDefault)
						mess += _T(" +qnidcm");
					else
						mess += _T(" +qnidc");
					break;

				case 'J': // Unignore
					mess = _T("/UNIGNORE *!*@") + host;
					break;

				case 'B': // Ban
					channel = (command.c_str() + 1);
					mess = _T("/MODE ") + channel + _T(" +b *!*@") + host;
					break;

				case 'K': // Ban & Kick
					channel = (command.c_str() + 1);
					mess.Format(_T("/MODE %s +b *!*@%s%%newl/KICK %s %s"), channel.c_str(), host.c_str(), channel.c_str(), nick.c_str());
					break;

				case 'L': // Ban & Kick with reason
					channel = (command.c_str() + 1);
					mess.Format(_T("/MODE %s +b *!*@%s%%newl/KICK %s %s %%question=\"%s\",\"%s\",\"%s\""),
						channel.c_str(), host.c_str(), channel.c_str(), nick.c_str(),
						TranslateT("Please enter the reason"), TranslateT("Ban'n Kick"), TranslateT("Jerk"));
					break;
				}

				mir_free(p1);

				// Post message
				if (!mess.IsEmpty())
					PostIrcMessageWnd(NULL, NULL, mess.c_str());
			}

			// Status-check post-processing: make buddies in ckeck-list offline
			if (command[0] == 'S') {
				for (int i = 0; i < checklist.getCount(); i++) {
					finduser.name = (TCHAR*)checklist[i].c_str();
					finduser.ExactNick = true;
					CList_SetOffline(&finduser);
				}
			}

			return true;
		}
	}

	if (!pmsg->m_bIncoming || command == _T("U"))
		ShowMessage(pmsg);
	return true;
}

bool CIrcProto::OnIrc_SUPPORT(const CIrcMessage* pmsg)
{
	static const TCHAR *lpszFmt = _T("Try server %99[^ ,], port %19s");
	TCHAR szAltServer[100];
	TCHAR szAltPort[20];
	if (pmsg->parameters.getCount() > 1 && _stscanf(pmsg->parameters[1].c_str(), lpszFmt, &szAltServer, &szAltPort) == 2) {
		ShowMessage(pmsg);
		mir_strncpy(m_serverName, _T2A(szAltServer), 99);
		mir_strncpy(m_portStart, _T2A(szAltPort), 9);

		m_noOfChannels = 0;
		ConnectToServer();
		return true;
	}

	if (pmsg->m_bIncoming && !bPerformDone)
		DoOnConnect(pmsg);

	if (pmsg->m_bIncoming && pmsg->parameters.getCount() > 0) {
		CMString S;
		for (int i = 0; i < pmsg->parameters.getCount(); i++) {
			TCHAR* temp = mir_tstrdup(pmsg->parameters[i].c_str());
			if (_tcsstr(temp, _T("CHANTYPES="))) {
				TCHAR* p1 = _tcschr(temp, '=');
				p1++;
				if (mir_tstrlen(p1) > 0)
					sChannelPrefixes = p1;
			}
			if (_tcsstr(temp, _T("CHANMODES="))) {
				TCHAR* p1 = _tcschr(temp, '=');
				p1++;
				if (mir_tstrlen(p1) > 0)
					sChannelModes = (char*)_T2A(p1);
			}
			if (_tcsstr(temp, _T("PREFIX="))) {
				TCHAR* p1 = _tcschr(temp, '(');
				TCHAR* p2 = _tcschr(temp, ')');
				if (p1 && p2) {
					p1++;
					if (p1 != p2)
						sUserModes = (char*)_T2A(p1);
					sUserModes = sUserModes.Mid(0, p2 - p1);
					p2++;
					if (*p2 != '\0')
						sUserModePrefixes = p2;
				}
				else {
					p1 = _tcschr(temp, '=');
					p1++;
					sUserModePrefixes = p1;
					for (int i = 0; i < sUserModePrefixes.GetLength() + 1; i++) {
						if (sUserModePrefixes[i] == '@')
							sUserModes.SetAt(i, 'o');
						else if (sUserModePrefixes[i] == '+')
							sUserModes.SetAt(i, 'v');
						else if (sUserModePrefixes[i] == '-')
							sUserModes.SetAt(i, 'u');
						else if (sUserModePrefixes[i] == '%')
							sUserModes.SetAt(i, 'h');
						else if (sUserModePrefixes[i] == '!')
							sUserModes.SetAt(i, 'a');
						else if (sUserModePrefixes[i] == '*')
							sUserModes.SetAt(i, 'q');
						else if (sUserModePrefixes[i] == '\0')
							sUserModes.SetAt(i, '\0');
						else
							sUserModes.SetAt(i, '_');
					}
				}
			}

			mir_free(temp);
		}
	}

	ShowMessage(pmsg);
	return true;
}

void CIrcProto::OnIrcDefault(const CIrcMessage* pmsg)
{
	ShowMessage(pmsg);
}

void CIrcProto::OnIrcDisconnected()
{
	m_statusMessage = _T("");
	db_unset(NULL, m_szModuleName, "JTemp");
	bTempDisableCheck = false;
	bTempForceCheck = false;
	m_iTempCheckTime = 0;

	m_myHost[0] = '\0';

	int Temp = m_iStatus;
	KillIdent();
	KillChatTimer(OnlineNotifTimer);
	KillChatTimer(OnlineNotifTimer3);
	KillChatTimer(KeepAliveTimer);
	KillChatTimer(InitTimer);
	KillChatTimer(IdentTimer);
	m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)Temp, ID_STATUS_OFFLINE);

	CMString sDisconn = _T("\0035\002");
	sDisconn += TranslateT("*Disconnected*");
	DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, sDisconn.c_str(), NULL, NULL, NULL, true, false);

	GCDEST gcd = { m_szModuleName, 0, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	CallChatEvent(SESSION_OFFLINE, (LPARAM)&gce);

	if (!Miranda_Terminated())
		CList_SetAllOffline(m_disconnectDCCChats);

	// restore the original nick, cause it might be changed
	memcpy(m_nick, m_pNick, sizeof(m_nick));
	setTString("Nick", m_pNick);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | CMIF_GRAYED;
	Menu_ModifyItem(hMenuJoin, &mi);
	Menu_ModifyItem(hMenuList, &mi);
	Menu_ModifyItem(hMenuNick, &mi);
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

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS;
	Menu_ModifyItem(hMenuJoin, &mi);
	Menu_ModifyItem(hMenuList, &mi);
	Menu_ModifyItem(hMenuNick, &mi);

	int Temp = m_iStatus;
	m_iStatus = ID_STATUS_ONLINE;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)Temp, m_iStatus);

	if (m_iDesiredStatus == ID_STATUS_AWAY)
		PostIrcMessage(_T("/AWAY %s"), m_statusMessage.Mid(0, 450).c_str());

	if (m_perform) {
		DoPerform("ALL NETWORKS");
		if (IsConnected()) {
			DoPerform(_T2A(m_info.sNetwork.c_str()));
			switch (m_iStatus) {
				case ID_STATUS_FREECHAT:   DoPerform("Event: Free for chat");   break;
				case ID_STATUS_ONLINE:     DoPerform("Event: Available");       break;
			}
		}
	}

	if (m_rejoinChannels) {
		int count = CallServiceSync(MS_GC_GETSESSIONCOUNT, 0, (LPARAM)m_szModuleName);
		for (int i = 0; i < count; i++) {
			GC_INFO gci = { 0 };
			gci.Flags = GCF_BYINDEX | GCF_DATA | GCF_NAME | GCF_TYPE;
			gci.iItem = i;
			gci.pszModule = m_szModuleName;
			if (!CallServiceSync(MS_GC_GETINFO, 0, (LPARAM)&gci) && gci.iType == GCW_CHATROOM) {
				CHANNELINFO *wi = (CHANNELINFO*)gci.dwItemData;
				if (wi && wi->pszPassword)
					PostIrcMessage(_T("/JOIN %s %s"), gci.pszName, wi->pszPassword);
				else
					PostIrcMessage(_T("/JOIN %s"), gci.pszName);
			}
		}
	}

	DoEvent(GC_EVENT_ADDGROUP, SERVERWINDOW, NULL, NULL, _T("Normal"), NULL, NULL, FALSE, TRUE);
	{
		GCDEST gcd = { m_szModuleName, SERVERWINDOW, GC_EVENT_CONTROL };
		GCEVENT gce = { sizeof(gce), &gcd };
		CallChatEvent(SESSION_ONLINE, (LPARAM)&gce);
	}

	CallFunctionAsync(sttMainThrdOnConnect, this);
	nickflag = false;
	return 0;
}

static void __cdecl AwayWarningThread(LPVOID)
{
	MessageBox(NULL, TranslateT("The usage of /AWAY in your perform buffer is restricted\n as IRC sends this command automatically."), TranslateT("IRC Error"), MB_OK);
}

int CIrcProto::DoPerform(const char* event)
{
	CMStringA sSetting = CMStringA("PERFORM:") + event;
	sSetting.MakeUpper();

	DBVARIANT dbv;
	if (!getTString(sSetting.c_str(), &dbv)) {
		if (!my_strstri(dbv.ptszVal, _T("/away")))
			PostIrcMessageWnd(NULL, NULL, dbv.ptszVal);
		else
			mir_forkthread(AwayWarningThread, NULL);
		db_free(&dbv);
		return 1;
	}
	return 0;
}

int CIrcProto::IsIgnored(const CMString& nick, const CMString& address, const CMString& host, char type)
{
	return IsIgnored(nick + _T("!") + address + _T("@") + host, type);
}

int CIrcProto::IsIgnored(CMString user, char type)
{
	for (int i = 0; i < m_ignoreItems.getCount(); i++) {
		const CIrcIgnoreItem& C = m_ignoreItems[i];

		if (type == 0 && !mir_tstrcmpi(user.c_str(), C.mask.c_str()))
			return i + 1;

		bool bUserContainsWild = (_tcschr(user.c_str(), '*') != NULL || _tcschr(user.c_str(), '?') != NULL);
		if (!bUserContainsWild && WCCmp(C.mask.c_str(), user.c_str()) ||
			bUserContainsWild && !mir_tstrcmpi(user.c_str(), C.mask.c_str())) {
			if (C.flags.IsEmpty() || C.flags[0] != '+')
				continue;

			if (!_tcschr(C.flags.c_str(), type))
				continue;

			if (C.network.IsEmpty())
				return i + 1;

			if (IsConnected() && !mir_tstrcmpi(C.network.c_str(), m_info.sNetwork.c_str()))
				return i + 1;
		}
	}

	return 0;
}

bool CIrcProto::AddIgnore(const TCHAR* mask, const TCHAR* flags, const TCHAR* network)
{
	RemoveIgnore(mask);
	m_ignoreItems.insert(new CIrcIgnoreItem(mask, (_T("+") + CMString(flags)).c_str(), network));
	RewriteIgnoreSettings();

	if (m_ignoreDlg)
		m_ignoreDlg->RebuildList();
	return true;
}

bool CIrcProto::RemoveIgnore(const TCHAR* mask)
{
	int idx;
	while ((idx = IsIgnored(mask, '\0')) != 0)
		m_ignoreItems.remove(idx - 1);

	RewriteIgnoreSettings();

	if (m_ignoreDlg)
		m_ignoreDlg->RebuildList();
	return true;
}
