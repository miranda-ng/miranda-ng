
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

BOOL CIrcProto::CList_AddDCCChat(const CMString& name, const CMString& hostmask, unsigned long adr, int port)
{
	MCONTACT hContact;
	TCHAR szNick[256];
	char szService[256];
	bool bFlag = false;

	CONTACT usertemp = { (TCHAR*)name.c_str(), NULL, NULL, false, false, true };
	MCONTACT hc = CList_FindContact(&usertemp);
	if (hc && db_get_b(hc, "CList", "NotOnList", 0) == 0 && db_get_b(hc, "CList", "Hidden", 0) == 0)
		bFlag = true;

	CMString contactname = name; contactname += DCCSTRING;

	CONTACT user = { (TCHAR*)contactname.c_str(), NULL, NULL, false, false, true };
	hContact = CList_AddContact(&user, false, false);
	setByte(hContact, "DCC", 1);

	DCCINFO* pdci = new DCCINFO;
	pdci->sHostmask = hostmask;
	pdci->hContact = hContact;
	pdci->dwAdr = (DWORD)adr;
	pdci->iPort = port;
	pdci->iType = DCC_CHAT;
	pdci->bSender = false;
	pdci->sContactName = name;

	if (m_DCCChatAccept == 3 || m_DCCChatAccept == 2 && bFlag) {
		CDccSession* dcc = new CDccSession(this, pdci);

		CDccSession* olddcc = FindDCCSession(hContact);
		if (olddcc)
			olddcc->Disconnect();

		AddDCCSession(hContact, dcc);
		dcc->Connect();
		if (getByte("MirVerAutoRequest", 1))
			PostIrcMessage(_T("/PRIVMSG %s \001VERSION\001"), name.c_str());
	}
	else {
		CLISTEVENT cle = { sizeof(cle) };
		cle.hContact = hContact;
		cle.hDbEvent = (HANDLE)"dccchat";
		cle.flags = CLEF_TCHAR;
		cle.hIcon = LoadIconEx(IDI_DCC);
		mir_snprintf(szService, SIZEOF(szService), "%s/DblClickEvent", m_szModuleName);
		cle.pszService = szService;
		mir_sntprintf(szNick, SIZEOF(szNick), TranslateT("CTCP chat request from %s"), name.c_str());
		cle.ptszTooltip = szNick;
		cle.lParam = (LPARAM)pdci;

		if (CallService(MS_CLIST_GETEVENT, hContact, 0))
			CallService(MS_CLIST_REMOVEEVENT, hContact, (LPARAM)"dccchat");
		CallService(MS_CLIST_ADDEVENT, hContact, (LPARAM)&cle);
	}
	return TRUE;
}

MCONTACT CIrcProto::CList_AddContact(CONTACT *user, bool InList, bool SetOnline)
{
	if (user->name == NULL)
		return 0;

	MCONTACT hContact = CList_FindContact(user);
	if (hContact) {
		if (InList)
			db_unset(hContact, "CList", "NotOnList");
		setTString(hContact, "Nick", user->name);
		db_unset(hContact, "CList", "Hidden");
		if (SetOnline && getWord(hContact, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
			setWord(hContact, "Status", ID_STATUS_ONLINE);
		return hContact;
	}

	// here we create a new one since no one is to be found
	hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if (hContact) {
		CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)m_szModuleName);

		if (InList)
			db_unset(hContact, "CList", "NotOnList");
		else
			db_set_b(hContact, "CList", "NotOnList", 1);
		db_unset(hContact, "CList", "Hidden");
		setTString(hContact, "Nick", user->name);
		setTString(hContact, "Default", user->name);
		setWord(hContact, "Status", SetOnline ? ID_STATUS_ONLINE : ID_STATUS_OFFLINE);
		if (!InList && getByte("MirVerAutoRequestTemp", 0))
			PostIrcMessage(_T("/PRIVMSG %s \001VERSION\001"), user->name);
		return hContact;
	}
	return false;
}

MCONTACT CIrcProto::CList_SetOffline(CONTACT *user)
{
	MCONTACT hContact = CList_FindContact(user);
	if (hContact) {
		DBVARIANT dbv;
		if (!getTString(hContact, "Default", &dbv)) {
			setString(hContact, "User", "");
			setString(hContact, "Host", "");
			setTString(hContact, "Nick", dbv.ptszVal);
			setWord(hContact, "Status", ID_STATUS_OFFLINE);
			db_free(&dbv);
			return hContact;
		}
	}

	return 0;
}

bool CIrcProto::CList_SetAllOffline(BYTE ChatsToo)
{
	DBVARIANT dbv;

	DisconnectAllDCCSessions(false);

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			continue;

		if (getByte(hContact, "DCC", 0) != 0) {
			if (ChatsToo)
				setWord(hContact, "Status", ID_STATUS_OFFLINE);
		}
		else if (!getTString(hContact, "Default", &dbv)) {
			setTString(hContact, "Nick", dbv.ptszVal);
			setWord(hContact, "Status", ID_STATUS_OFFLINE);
			db_free(&dbv);
		}
		db_unset(hContact, m_szModuleName, "IP");
		setString(hContact, "User", "");
		setString(hContact, "Host", "");
	}
	return true;
}

MCONTACT CIrcProto::CList_FindContact(CONTACT *user)
{
	if (!user || !user->name)
		return 0;

	TCHAR* lowercasename = mir_tstrdup(user->name);
	CharLower(lowercasename);

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (isChatRoom(hContact))
			continue;

		MCONTACT  hContact_temp = NULL;
		ptrT DBNick(getTStringA(hContact, "Nick"));
		ptrT DBUser(getTStringA(hContact, "UUser"));
		ptrT DBHost(getTStringA(hContact, "UHost"));
		ptrT DBDefault(getTStringA(hContact, "Default"));
		ptrT DBWildcard(getTStringA(hContact, "UWildcard"));

		if (DBWildcard)
			CharLower(DBWildcard);
		if (IsChannel(user->name)) {
			if (DBDefault && !mir_tstrcmpi(DBDefault, user->name))
				hContact_temp = (MCONTACT)-1;
		}
		else if (user->ExactNick && DBNick && !mir_tstrcmpi(DBNick, user->name))
			hContact_temp = hContact;

		else if (user->ExactOnly && DBDefault && !mir_tstrcmpi(DBDefault, user->name))
			hContact_temp = hContact;

		else if (user->ExactWCOnly) {
			if (DBWildcard && !mir_tstrcmpi(DBWildcard, lowercasename)
				|| (DBWildcard && !mir_tstrcmpi(DBNick, lowercasename) && !WCCmp(DBWildcard, lowercasename))
				|| (!DBWildcard && !mir_tstrcmpi(DBNick, lowercasename))) {
				hContact_temp = hContact;
			}
		}
		else if (_tcschr(user->name, ' ') == 0) {
			if ((DBDefault && !mir_tstrcmpi(DBDefault, user->name) || DBNick && !mir_tstrcmpi(DBNick, user->name) ||
				DBWildcard && WCCmp(DBWildcard, lowercasename))
				&& (WCCmp(DBUser, user->user) && WCCmp(DBHost, user->host))) {
				hContact_temp = hContact;
			}
		}

		if (hContact_temp != NULL) {
			mir_free(lowercasename);
			if (hContact_temp != (MCONTACT)-1)
				return hContact_temp;
			return 0;
		}
	}
	mir_free(lowercasename);
	return 0;
}
