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

void CIrcProto::OnBuildProtoMenu()
{
	CMenuItem mi(&g_plugin);
	mi.root = Menu_GetProtocolRoot(this);
	if (m_iStatus != ID_STATUS_OFFLINE)
		mi.flags |= CMIF_GRAYED;

	mi.name.a = LPGEN("&Join channel");
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_CHAT_JOIN);
	mi.pszService = IRC_JOINCHANNEL;
	mi.position = 201002;
	hMenuJoin = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.name.a = LPGEN("&Change your nickname");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_RENAME);
	mi.pszService = IRC_CHANGENICK;
	mi.position = 201003;
	hMenuNick = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.name.a = LPGEN("Show the &list of available channels");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_LIST);
	mi.pszService = IRC_SHOWLIST;
	mi.position = 201004;
	hMenuList = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	if (m_useServer) mi.flags &= ~CMIF_GRAYED;
	mi.name.a = LPGEN("&Show the server window");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_SERVER);
	mi.pszService = IRC_SHOWSERVER;
	mi.position = 201005;
	hMenuServer = Menu_AddProtoMenuItem(&mi, m_szModuleName);
}

/////////////////////////////////////////////////////////////////////////////////////////

static HGENMENU hUMenuChanSettings, hUMenuWhois, hUMenuDisconnect, hUMenuIgnore;

static INT_PTR IrcMenuChanSettings(WPARAM hContact, LPARAM lParam)
{
	CIrcProto *ppro = CMPlugin::getInstance(hContact);
	return (ppro) ? ppro->OnMenuChanSettings(hContact, lParam) : 0;
}

static INT_PTR IrcMenuWhois(WPARAM hContact, LPARAM lParam)
{
	CIrcProto *ppro = CMPlugin::getInstance(hContact);
	return (ppro) ? ppro->OnMenuWhois(hContact, lParam) : 0;
}

static INT_PTR IrcMenuDisconnect(WPARAM hContact, LPARAM lParam)
{
	CIrcProto *ppro = CMPlugin::getInstance(hContact);
	return (ppro) ? ppro->OnMenuDisconnect(hContact, lParam) : 0;
}

static INT_PTR IrcMenuIgnore(WPARAM hContact, LPARAM lParam)
{
	CIrcProto *ppro = CMPlugin::getInstance(hContact);
	return (ppro) ? ppro->OnMenuIgnore(hContact, lParam) : 0;
}

int IrcPrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	Menu_ShowItem(hUMenuChanSettings, false);
	Menu_ShowItem(hUMenuWhois, false);
	Menu_ShowItem(hUMenuDisconnect, false);
	Menu_ShowItem(hUMenuIgnore, false);

	CIrcProto *ppro = CMPlugin::getInstance(hContact);
	return (ppro) ? ppro->OnMenuPreBuild(hContact, lParam) : 0;
}

void InitContactMenus(void)
{
	char temp[MAXMODULELABELLENGTH];
	char *d = temp + mir_snprintf(temp, "IRC");

	CMenuItem mi(&g_plugin);
	mi.pszService = temp;

	SET_UID(mi, 0x5f01196f, 0xfbcd, 0x4034, 0xbd, 0x90, 0x12, 0xa0, 0x20, 0x68, 0x15, 0xc0);
	mi.name.a = LPGEN("Channel &settings");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_MANAGER);
	mir_strcpy(d, IRC_UM_CHANSETTINGS);
	mi.position = 500090002;
	hUMenuChanSettings = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(temp, IrcMenuChanSettings);

	SET_UID(mi, 0x778eb1f6, 0x73c4, 0x4951, 0xb2, 0xca, 0xa1, 0x69, 0x94, 0x7b, 0xb7, 0x87);
	mi.name.a = LPGEN("&WhoIs info");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_WHOIS);
	mir_strcpy(d, IRC_UM_WHOIS);
	mi.position = 500090001;
	hUMenuWhois = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(temp, IrcMenuWhois);

	SET_UID(mi, 0x1c51ae05, 0x9eee, 0x4887, 0x88, 0x96, 0x55, 0xd2, 0xdd, 0xf9, 0x25, 0x6f);
	mi.name.a = LPGEN("Di&sconnect");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_DELETE);
	mir_strcpy(d, IRC_UM_DISCONNECT);
	mi.position = 500090001;
	hUMenuDisconnect = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(temp, IrcMenuDisconnect);

	SET_UID(mi, 0xc6169b8f, 0x53ab, 0x4242, 0xbe, 0x90, 0xe2, 0x4a, 0xa5, 0x73, 0x88, 0x32);
	mi.name.a = LPGEN("&Add to ignore list");
	mi.hIcolibItem = g_plugin.getIconHandle(IDI_BLOCK);
	mir_strcpy(d, IRC_UM_IGNORE);
	mi.position = 500090002;
	hUMenuIgnore = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(temp, IrcMenuIgnore);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, IrcPrebuildContactMenu);
}

void UninitContactMenus(void)
{
	Menu_RemoveItem(hUMenuChanSettings);
	Menu_RemoveItem(hUMenuWhois);
	Menu_RemoveItem(hUMenuDisconnect);
	Menu_RemoveItem(hUMenuIgnore);
}

INT_PTR __cdecl CIrcProto::OnDoubleclicked(WPARAM, LPARAM lParam)
{
	if (!lParam)
		return 0;

	CLISTEVENT* pcle = (CLISTEVENT*)lParam;

	if (getByte(pcle->hContact, "DCC", 0) != 0) {
		DCCINFO *pdci = (DCCINFO*)pcle->lParam;
		CMessageBoxDlg* dlg = new CMessageBoxDlg(this, pdci);
		dlg->Show();
		HWND hWnd = dlg->GetHwnd();
		wchar_t szTemp[500];
		mir_snwprintf(szTemp, TranslateT("%s (%s) is requesting a client-to-client chat connection."),
			pdci->sContactName.c_str(), pdci->sHostmask.c_str());
		SetDlgItemText(hWnd, IDC_TEXT, szTemp);
		ShowWindow(hWnd, SW_SHOW);
		return 1;
	}
	return 0;
}

void CIrcProto::OnContactDeleted(MCONTACT hContact)
{
	if (!hContact)
		return;

	DBVARIANT dbv;
	if (!getWString(hContact, "Nick", &dbv)) {
		int type = getByte(hContact, "ChatRoom", 0);
		if (type != 0) {
			CMStringW S;
			if (type == GCW_CHATROOM)
				S = dbv.pwszVal;
			if (type == GCW_SERVER)
				S = SERVERWINDOW;
			int i = Chat_Terminate(m_szModuleName, S);
			if (i && type == GCW_CHATROOM)
				PostIrcMessage(L"/PART %s %s", dbv.pwszVal, m_userInfo);
		}
		else {
			uint8_t bDCC = getByte(hContact, "DCC", 0);
			if (bDCC) {
				CDccSession *dcc = FindDCCSession(hContact);
				if (dcc)
					dcc->Disconnect();
			}
		}

		db_free(&dbv);
	}
}

INT_PTR __cdecl CIrcProto::OnJoinChat(WPARAM wp, LPARAM)
{
	if (!wp)
		return 0;

	DBVARIANT dbv;
	if (!getWString((MCONTACT)wp, "Nick", &dbv)) {
		if (getByte((MCONTACT)wp, "ChatRoom", 0) == GCW_CHATROOM)
			PostIrcMessage(L"/JOIN %s", dbv.pwszVal);
		db_free(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnLeaveChat(WPARAM wp, LPARAM)
{
	if (!wp)
		return 0;

	DBVARIANT dbv;
	if (!getWString((MCONTACT)wp, "Nick", &dbv)) {
		if (getByte((MCONTACT)wp, "ChatRoom", 0) == GCW_CHATROOM) {
			PostIrcMessage(L"/PART %s %s", dbv.pwszVal, m_userInfo);
			Chat_Terminate(m_szModuleName, dbv.pwszVal);
		}
		db_free(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnMenuChanSettings(WPARAM wp, LPARAM)
{
	if (!wp)
		return 0;

	MCONTACT hContact = (MCONTACT)wp;
	DBVARIANT dbv;
	if (!getWString(hContact, "Nick", &dbv)) {
		PostIrcMessageWnd(dbv.pwszVal, NULL, L"/CHANNELMANAGER");
		db_free(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnMenuWhois(WPARAM wp, LPARAM)
{
	if (!wp)
		return 0;

	DBVARIANT dbv;

	if (!getWString((MCONTACT)wp, "Nick", &dbv)) {
		PostIrcMessage(L"/WHOIS %s %s", dbv.pwszVal, dbv.pwszVal);
		db_free(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnMenuDisconnect(WPARAM wp, LPARAM)
{
	CDccSession *dcc = FindDCCSession((MCONTACT)wp);
	if (dcc)
		dcc->Disconnect();
	return 0;
}

INT_PTR __cdecl CIrcProto::OnMenuIgnore(WPARAM wp, LPARAM)
{
	if (!wp)
		return 0;

	MCONTACT hContact = (MCONTACT)wp;
	DBVARIANT dbv;
	if (!getWString(hContact, "Nick", &dbv)) {
		if (!isChatRoom(hContact)) {
			char* host = nullptr;
			DBVARIANT dbv1;
			if (!getString(hContact, "Host", &dbv1))
				host = dbv1.pszVal;

			if (host) {
				CMStringA S;
				if (m_ignoreChannelDefault)
					S = "+qnidcm";
				else
					S = "+qnidc";
				PostIrcMessage(L"/IGNORE %%question=\"%s\",\"%s\",\"*!*@%S\" %s",
					TranslateT("Please enter the hostmask (nick!user@host)\nNOTE! Contacts on your contact list are never ignored"),
					TranslateT("Ignore"), host, S.c_str());
				db_free(&dbv1);
			}
		}
		db_free(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnJoinMenuCommand(WPARAM, LPARAM)
{
	if (!m_joinDlg) {
		m_joinDlg = new CJoinDlg(this);
		m_joinDlg->Show();
	}

	SetDlgItemText(m_joinDlg->GetHwnd(), IDC_CAPTION, TranslateT("Join channel"));
	SetDlgItemText(m_joinDlg->GetHwnd(), IDC_TEXT, TranslateT("Please enter a channel to join"));
	SendDlgItemMessage(m_joinDlg->GetHwnd(), IDC_ENICK, EM_SETSEL, 0, MAKELPARAM(0, -1));
	ShowWindow(m_joinDlg->GetHwnd(), SW_SHOW);
	SetActiveWindow(m_joinDlg->GetHwnd());
	return 0;
}

INT_PTR __cdecl CIrcProto::OnShowListMenuCommand(WPARAM, LPARAM)
{
	PostIrcMessage(L"/LIST");
	return 0;
}

INT_PTR __cdecl CIrcProto::OnShowServerMenuCommand(WPARAM, LPARAM)
{
	Chat_Control(m_szModuleName, SERVERWINDOW, WINDOW_VISIBLE);
	return 0;
}

INT_PTR __cdecl CIrcProto::OnChangeNickMenuCommand(WPARAM, LPARAM)
{
	if (!m_nickDlg) {
		m_nickDlg = new CNickDlg(this);
		m_nickDlg->Show();
	}

	SetDlgItemText(m_nickDlg->GetHwnd(), IDC_CAPTION, TranslateT("Change nickname"));
	SetDlgItemText(m_nickDlg->GetHwnd(), IDC_TEXT, TranslateT("Please enter a unique nickname"));
	m_nickDlg->m_Enick.SetText(m_info.sNick.c_str());
	m_nickDlg->m_Enick.SendMsg(CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
	ShowWindow(m_nickDlg->GetHwnd(), SW_SHOW);
	SetActiveWindow(m_nickDlg->GetHwnd());
	return 0;
}

static int mapSrmm2irc[] = { 1, 2, 10, 6, 3, 7, 5, 14, 15, 12, 11, 13, 9, 8, 4, 0 };

static wchar_t* DoPrintColor(wchar_t *pDest, int iFG, int iBG)
{
	*pDest = irc::COLOR;
	if (iFG == -1) {
		if (iBG == -1)
			pDest[1] = 0;
		else
			mir_snwprintf(pDest+1, 49, L",%d", iBG);
	}
	else {
		if (iBG == -1)
			mir_snwprintf(pDest+1, 49, L"%d", iFG);
		else
			mir_snwprintf(pDest+1, 49, L"%d,%d", iFG, iBG);
	}

	return pDest;
}

static void DoChatFormatting(CMStringW &wszText)
{
	int iFG = -1, iBG = -1;
	wchar_t InsertThis[50];

	for (int i = 0; i < wszText.GetLength(); i++) {
		if (wszText[i] != '%')
			continue;

		switch (wszText[i + 1]) {
		case 'B':
		case 'b':
			wszText.Delete(i, 2);
			wszText.Insert(i, irc::BOLD);
			break;
		case 'I':
		case 'i':
			wszText.Delete(i, 2);
			wszText.Insert(i, irc::ITALICS);
			break;
		case 'U':
		case 'u':
			wszText.Delete(i, 2);
			wszText.Insert(i, irc::UNDERLINE);
			break;

		case 'c':
			wszText.Delete(i, 2);
			iFG = _wtoi(wszText.GetString() + i);
			wszText.Delete(i, (iFG < 10) ? 1 : 2);

			iFG = (iFG < 0 || iFG >= _countof(mapSrmm2irc)) ? -1 : mapSrmm2irc[iFG];
			wszText.Insert(i, DoPrintColor(InsertThis, iFG, iBG));
			break;

		case 'C':
			if (wszText[i + 2] == '%' && wszText[i + 3] == 'F') {
				wszText.Delete(i, 4);
				iBG = -1;
			}
			else wszText.Delete(i, 2);
			iFG = -1;
			wszText.Insert(i, DoPrintColor(InsertThis, iFG, iBG));
			break;

		case 'f':
			wszText.Delete(i, 2);
			iBG = _wtoi(wszText.GetString() + i);
			wszText.Delete(i, (iBG < 10) ? 1 : 2);

			iBG = (iBG < 0 || iBG >= _countof(mapSrmm2irc)) ? -1 : mapSrmm2irc[iBG];
			wszText.Insert(i, DoPrintColor(InsertThis, iFG, iBG));
			break;

		case 'F':
			wszText.Delete(i, 2);
			iBG = -1;
			wszText.Insert(i, DoPrintColor(InsertThis, iFG, iBG));
			break;

		case '%':
			wszText.Delete(i, 1);
			i++;
			break;

		default:
			wszText.Delete(i, 2);
			break;
		}
	}
}

int __cdecl CIrcProto::GCEventHook(WPARAM, LPARAM lParam)
{
	// handle the hook
	GCHOOK *gch = (GCHOOK*)lParam;
	if (mir_strcmpi(gch->si->pszModule, m_szModuleName))
		return 0;

	CMStringW S = L"";
	mir_cslock lock(m_csGcHook);

	wchar_t *p1 = mir_wstrdup(gch->si->ptszID);
	wchar_t *p2 = wcsstr(p1, L" - ");
	if (p2)
		*p2 = 0;

	switch (gch->iType) {
	case GC_SESSION_TERMINATE:
		FreeWindowItemData(p1, (CHANNELINFO*)gch->dwData);
		break;

	case GC_USER_MESSAGE:
		if (gch && gch->ptszText && *gch->ptszText) {
			CMStringW wszText(gch->ptszText);
			DoChatFormatting(wszText);
			PostIrcMessageWnd(p1, NULL, wszText);
		}
		break;

	case GC_USER_CHANMGR:
		PostIrcMessageWnd(p1, NULL, L"/CHANNELMANAGER");
		break;

	case GC_USER_PRIVMESS:
		PostIrcMessageWnd(p1, NULL, CMStringW(FORMAT, L"/QUERY %s", gch->ptszUID));
		break;

	case GC_USER_LOGMENU:
		switch (gch->dwData) {
		case 1:
			OnChangeNickMenuCommand(NULL, NULL);
			break;
		case 2:
			PostIrcMessageWnd(p1, NULL, L"/CHANNELMANAGER");
			break;

		case 3:
			PostIrcMessage(L"/PART %s %s", p1, m_userInfo);
			Chat_Terminate(m_szModuleName, p1);
			break;

		case 4:		// show server window
			PostIrcMessageWnd(p1, NULL, L"/SERVERSHOW");
			break;
			/*					case 5:		// nickserv register nick
			PostIrcMessage( L"/nickserv REGISTER %%question=\"%s\",\"%s\"",
			TranslateT("Please enter your authentication code"), TranslateT("Authenticate nick"));
			break;
			*/
		case 6:		// nickserv Identify
			PostIrcMessage(L"/nickserv AUTH %%question=\"%s\",\"%s\"",
				TranslateT("Please enter your authentication code"), TranslateT("Authenticate nick"));
			break;
		case 7:		// nickserv drop nick
			if (MessageBox(nullptr, TranslateT("Are you sure you want to unregister your current nick?"), TranslateT("Delete nick"),
				MB_ICONERROR + MB_YESNO + MB_DEFBUTTON2) == IDYES)
				PostIrcMessage(L"/nickserv DROP");
			break;
		case 8:		// nickserv Identify
			{
				CQuestionDlg* dlg = new CQuestionDlg(this);
				dlg->Show();
				HWND question_hWnd = dlg->GetHwnd();
				HWND hEditCtrl = GetDlgItem(question_hWnd, IDC_EDIT);
				SetDlgItemText(question_hWnd, IDC_CAPTION, TranslateT("Identify nick"));
				SetDlgItemText(question_hWnd, IDC_TEXT, TranslateT("Please enter your password"));
				SetDlgItemText(question_hWnd, IDC_HIDDENEDIT, L"/nickserv IDENTIFY %question=\"%s\",\"%s\"");
				SetWindowLongPtr(GetDlgItem(question_hWnd, IDC_EDIT), GWL_STYLE,
					(LONG)GetWindowLongPtr(GetDlgItem(question_hWnd, IDC_EDIT), GWL_STYLE) | ES_PASSWORD);
				SendMessage(hEditCtrl, EM_SETPASSWORDCHAR, (WPARAM)'*', 0);
				SetFocus(hEditCtrl);
				dlg->Activate();
			}
			break;
		case 9:		// nickserv remind password
			{
				DBVARIANT dbv;
				if (!getWString("Nick", &dbv)) {
					PostIrcMessage(L"/nickserv SENDPASS %s", dbv.pwszVal);
					db_free(&dbv);
				}
			}
			break;
		case 10:		// nickserv set new password
			PostIrcMessage(L"/nickserv SET PASSWORD %%question=\"%s\",\"%s\"",
				TranslateT("Please enter your new password"), TranslateT("Set new password"));
			break;
		case 11:		// nickserv set language
			PostIrcMessage(L"/nickserv SET LANGUAGE %%question=\"%s\",\"%s\"",
				TranslateT("Please enter desired language ID (numeric value, depends on server)"), TranslateT("Change language of NickServ messages"));
			break;
		case 12:		// nickserv set homepage
			PostIrcMessage(L"/nickserv SET URL %%question=\"%s\",\"%s\"",
				TranslateT("Please enter URL that will be linked to your nick"), TranslateT("Set URL, linked to nick"));
			break;
		case 13:		// nickserv set email
			PostIrcMessage(L"/nickserv SET EMAIL %%question=\"%s\",\"%s\"",
				TranslateT("Please enter your e-mail, that will be linked to your nick"), TranslateT("Set e-mail, linked to nick"));
			break;
		case 14:		// nickserv set info
			PostIrcMessage(L"/nickserv SET INFO %%question=\"%s\",\"%s\"",
				TranslateT("Please enter some information about your nick"), TranslateT("Set information for nick"));
			break;
		case 15:		// nickserv kill unauth off
			PostIrcMessage(L"/nickserv SET KILL OFF");
			break;
		case 16:		// nickserv kill unauth on
			PostIrcMessage(L"/nickserv SET KILL ON");
			break;
		case 17:		// nickserv kill unauth quick
			PostIrcMessage(L"/nickserv SET KILL QUICK");
			break;
		case 18:		// nickserv hide nick from /LIST
			PostIrcMessage(L"/nickserv SET PRIVATE ON");
			break;
		case 19:		// nickserv show nick to /LIST
			PostIrcMessage(L"/nickserv SET PRIVATE OFF");
			break;
		case 20:		// nickserv Hide e-mail from info
			PostIrcMessage(L"/nickserv SET HIDE EMAIL ON");
			break;
		case 21:		// nickserv Show e-mail in info
			PostIrcMessage(L"/nickserv SET HIDE EMAIL OFF");
			break;
		case 22:		// nickserv Set security for nick
			PostIrcMessage(L"/nickserv SET SECURE ON");
			break;
		case 23:		// nickserv Remove security for nick
			PostIrcMessage(L"/nickserv SET SECURE OFF");
			break;
		case 24:		// nickserv Link nick to current
			PostIrcMessage(L"/nickserv LINK %%question=\"%s\",\"%s\"",
				TranslateT("Please enter nick you want to link to your current nick"), TranslateT("Link another nick to current nick"));
			break;
		case 25:		// nickserv Unlink nick from current
			PostIrcMessage(L"/nickserv LINK %%question=\"%s\",\"%s\"",
				TranslateT("Please enter nick you want to unlink from your current nick"), TranslateT("Unlink another nick from current nick"));
			break;
		case 26:		// nickserv Set main nick
			PostIrcMessage(L"/nickserv LINK %%question=\"%s\",\"%s\"",
				TranslateT("Please enter nick you want to set as your main nick"), TranslateT("Set main nick"));
			break;
		case 27:		// nickserv list all linked nicks
			PostIrcMessage(L"/nickserv LISTLINKS");
			break;
		case 28:		// nickserv list all channels owned
			PostIrcMessage(L"/nickserv LISTCHANS");
			break;
		}
		break;

	case GC_USER_NICKLISTMENU:
		switch (gch->dwData) {
		case 1:
			PostIrcMessage(L"/MODE %s +o %s", p1, gch->ptszUID);
			break;
		case 2:
			PostIrcMessage(L"/MODE %s -o %s", p1, gch->ptszUID);
			break;
		case 3:
			PostIrcMessage(L"/MODE %s +v %s", p1, gch->ptszUID);
			break;
		case 4:
			PostIrcMessage(L"/MODE %s -v %s", p1, gch->ptszUID);
			break;
		case 5:
			PostIrcMessage(L"/KICK %s %s", p1, gch->ptszUID);
			break;
		case 6:
			PostIrcMessage(L"/KICK %s %s %%question=\"%s\",\"%s\",\"%s\"",
				p1, gch->ptszUID, TranslateT("Please enter the reason"), TranslateT("Kick"), "");
			break;
		case 7:
			DoUserhostWithReason(1, L"B" + (CMStringW)p1, true, L"%s", gch->ptszUID);
			break;
		case 8:
			DoUserhostWithReason(1, L"K" + (CMStringW)p1, true, L"%s", gch->ptszUID);
			break;
		case 9:
			DoUserhostWithReason(1, L"L" + (CMStringW)p1, true, L"%s", gch->ptszUID);
			break;
		case 10:
			PostIrcMessage(L"/WHOIS %s %s", gch->ptszUID, gch->ptszUID);
			break;
			//	case 11:
			//		DoUserhostWithReason(1, "I", true, "%s", gch->ptszUID );
			//		break;
			//	case 12:
			//		DoUserhostWithReason(1, "J", true, "%s", gch->ptszUID );
			//		break;
		case 13:
			PostIrcMessage(L"/DCC CHAT %s", gch->ptszUID);
			break;
		case 14:
			PostIrcMessage(L"/DCC SEND %s", gch->ptszUID);
			break;
		case 15:
			DoUserhostWithReason(1, L"I", true, L"%s", gch->ptszUID);
			break;
		case 16:
			PostIrcMessage(L"/MODE %s +h %s", p1, gch->ptszUID);
			break;
		case 17:
			PostIrcMessage(L"/MODE %s -h %s", p1, gch->ptszUID);
			break;
		case 18:
			PostIrcMessage(L"/MODE %s +q %s", p1, gch->ptszUID);
			break;
		case 19:
			PostIrcMessage(L"/MODE %s -q %s", p1, gch->ptszUID);
			break;
		case 20:
			PostIrcMessage(L"/MODE %s +a %s", p1, gch->ptszUID);
			break;
		case 21:
			PostIrcMessage(L"/MODE %s -a %s", p1, gch->ptszUID);
			break;
		case 22:
			PostIrcMessage(L"/NOTICE %s %%question=\"%s\",\"%s\"",
				gch->ptszUID, TranslateT("Please enter the notice text"), TranslateT("Send notice"));
			break;
		case 23:
			PostIrcMessage(L"/INVITE %s %%question=\"%s\",\"%s\"",
				gch->ptszUID, TranslateT("Please enter the channel name to invite to"), TranslateT("Invite to channel"));
			break;
		case 30:
			{
				PROTOSEARCHRESULT psr = {};
				psr.cbSize = sizeof(psr);
				psr.flags = PSR_UNICODE;
				psr.id.w = gch->ptszUID;
				psr.nick.w = gch->ptszUID;
				Contact::AddBySearch(m_szModuleName, &psr);
			}
			break;
		case 31:	//slap
			PostIrcMessageWnd(p1, NULL, CMStringW(FORMAT, L"/slap %s", gch->ptszUID));
			break;
		case 32:  //nickserv info
			PostIrcMessageWnd(p1, NULL, CMStringW(FORMAT, L"/nickserv INFO %s ALL", gch->ptszUID));
			break;
		case 33:  //nickserv ghost
			PostIrcMessageWnd(p1, NULL, CMStringW(FORMAT, L"/nickserv GHOST %s", gch->ptszUID));
			break;
		}
		break;
	}
	mir_free(p1);

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////

static gc_item logItems[] = {
		{ LPGENW("&Change your nickname"), 1, MENU_ITEM, FALSE },
		{ LPGENW("Channel &settings"), 2, MENU_ITEM, FALSE },
		{ L"", 0, MENU_SEPARATOR, FALSE },
		{ LPGENW("NickServ"), 0, MENU_NEWPOPUP, FALSE },
		{ LPGENW("Register nick"), 5, MENU_POPUPITEM, TRUE },
		{ LPGENW("Auth nick"), 6, MENU_POPUPITEM, FALSE },
		{ LPGENW("Delete nick"), 7, MENU_POPUPITEM, FALSE },
		{ LPGENW("Identify nick"), 8, MENU_POPUPITEM, FALSE },
		{ LPGENW("Remind password"), 9, MENU_POPUPITEM, FALSE },
		{ LPGENW("Set new password"), 10, MENU_POPUPITEM, TRUE },
		{ LPGENW("Set language"), 11, MENU_POPUPITEM, FALSE },
		{ LPGENW("Set homepage"), 12, MENU_POPUPITEM, FALSE },
		{ LPGENW("Set e-mail"), 13, MENU_POPUPITEM, FALSE },
		{ LPGENW("Set info"), 14, MENU_POPUPITEM, FALSE },
		{ L"", 0, MENU_POPUPSEPARATOR, FALSE },
		{ LPGENW("Hide e-mail from info"), 20, MENU_POPUPITEM, FALSE },
		{ LPGENW("Show e-mail in info"), 21, MENU_POPUPITEM, FALSE },
		{ L"", 0, MENU_POPUPSEPARATOR, FALSE },
		{ LPGENW("Set security for nick"), 22, MENU_POPUPITEM, FALSE },
		{ LPGENW("Remove security for nick"), 23, MENU_POPUPITEM, FALSE },
		{ L"", 0, MENU_POPUPSEPARATOR, FALSE },
		{ LPGENW("Link nick to current"), 24, MENU_POPUPITEM, FALSE },
		{ LPGENW("Unlink nick from current"), 25, MENU_POPUPITEM, FALSE },
		{ LPGENW("Set main nick"), 26, MENU_POPUPITEM, FALSE },
		{ LPGENW("List all your nicks"), 27, MENU_POPUPITEM, FALSE },
		{ LPGENW("List your channels"), 28, MENU_POPUPITEM, FALSE },
		{ L"", 0, MENU_POPUPSEPARATOR, FALSE },
		{ LPGENW("Kill unauthorized: off"), 15, MENU_POPUPITEM, FALSE },
		{ LPGENW("Kill unauthorized: on"), 16, MENU_POPUPITEM, FALSE },
		{ LPGENW("Kill unauthorized: quick"), 17, MENU_POPUPITEM, FALSE },
		{ L"", 0, MENU_POPUPSEPARATOR, FALSE },
		{ LPGENW("Hide nick from list"), 18, MENU_POPUPITEM, FALSE },
		{ LPGENW("Show nick to list"), 19, MENU_POPUPITEM, FALSE },
		{ LPGENW("Show the server &window"), 4, MENU_ITEM, FALSE },
		{ L"", 0, MENU_SEPARATOR, FALSE },
		{ LPGENW("&Leave the channel"), 3, MENU_ITEM, FALSE }
};

static gc_item nickItems[] = {
		{ LPGENW("&WhoIs info"), 10, MENU_ITEM, FALSE },       //0
		{ LPGENW("&Invite to channel"), 23, MENU_ITEM, FALSE },
		{ LPGENW("Send &notice"), 22, MENU_ITEM, FALSE },
		{ LPGENW("&Slap"), 31, MENU_ITEM, FALSE },
		{ LPGENW("Nickserv info"), 32, MENU_ITEM, FALSE },
		{ LPGENW("Nickserv kill ghost"), 33, MENU_ITEM, FALSE },      //5
		{ LPGENW("&Control"), 0, MENU_NEWPOPUP, FALSE },
		{ LPGENW("Give Owner"), 18, MENU_POPUPITEM, FALSE },      //7
		{ LPGENW("Take Owner"), 19, MENU_POPUPITEM, FALSE },
		{ LPGENW("Give Admin"), 20, MENU_POPUPITEM, FALSE },
		{ LPGENW("Take Admin"), 21, MENU_POPUPITEM, FALSE },      //10
		{ LPGENW("Give &Op"), 1, MENU_POPUPITEM, FALSE },
		{ LPGENW("Take O&p"), 2, MENU_POPUPITEM, FALSE },
		{ LPGENW("Give &Halfop"), 16, MENU_POPUPITEM, FALSE },
		{ LPGENW("Take H&alfop"), 17, MENU_POPUPITEM, FALSE },
		{ LPGENW("Give &Voice"), 3, MENU_POPUPITEM, FALSE },      //15
		{ LPGENW("Take V&oice"), 4, MENU_POPUPITEM, FALSE },
		{ L"", 0, MENU_POPUPSEPARATOR, FALSE },
		{ LPGENW("&Kick"), 5, MENU_POPUPITEM, FALSE },
		{ LPGENW("Ki&ck (reason)"), 6, MENU_POPUPITEM, FALSE },
		{ LPGENW("&Ban"), 7, MENU_POPUPITEM, FALSE },      //20
		{ LPGENW("Ban'&n kick"), 8, MENU_POPUPITEM, FALSE },
		{ LPGENW("Ban'n kick (&reason)"), 9, MENU_POPUPITEM, FALSE },
		{ LPGENW("&Direct Connection"), 0, MENU_NEWPOPUP, FALSE },
		{ LPGENW("Request &Chat"), 13, MENU_POPUPITEM, FALSE },
		{ LPGENW("Send &File"), 14, MENU_POPUPITEM, FALSE },      //25
		{ LPGENW("Add to &ignore list"), 15, MENU_ITEM, FALSE },
		{ L"", 12, MENU_SEPARATOR, FALSE },
		{ LPGENW("&Add User"), 30, MENU_ITEM, FALSE }
};

int __cdecl CIrcProto::GCMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;
	if (gcmi) {
		if (!mir_strcmpi(gcmi->pszModule, m_szModuleName)) {
			if (gcmi->Type == MENU_ON_LOG) {
				if (mir_wstrcmpi(gcmi->pszID, SERVERWINDOW))
					Chat_AddMenuItems(gcmi->hMenu, _countof(logItems), logItems, &g_plugin);
			}

			if (gcmi->Type == MENU_ON_NICKLIST) {
				CONTACT user = { (wchar_t*)gcmi->pszUID, nullptr, nullptr, false, false, false };
				MCONTACT hContact = CList_FindContact(&user);

				BOOL bIsInList = hContact && Contact::OnList(hContact);
				nickItems[_countof(nickItems)-1].bDisabled = bIsInList;

				unsigned long ulAdr = 0;
				if (m_manualHost)
					ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
				else
					ulAdr = ConvertIPToInteger(m_IPFromServer ? m_myHost : m_myLocalHost);
				nickItems[23].bDisabled = ulAdr == 0 ? TRUE : FALSE;	// DCC submenu

				CHANNELINFO *wi = (CHANNELINFO *)Chat_GetUserInfo(m_szModuleName, gcmi->pszID);
				BOOL bServOwner = strchr(sUserModes.c_str(), 'q') == nullptr ? FALSE : TRUE;
				BOOL bServAdmin = strchr(sUserModes.c_str(), 'a') == nullptr ? FALSE : TRUE;
				BOOL bOwner = bServOwner ? ((wi->OwnMode >> 4) & 01) : FALSE;
				BOOL bAdmin = bServAdmin ? ((wi->OwnMode >> 3) & 01) : FALSE;
				BOOL bOp = strchr(sUserModes.c_str(), 'o') == nullptr ? FALSE : ((wi->OwnMode >> 2) & 01);
				BOOL bHalfop = strchr(sUserModes.c_str(), 'h') == nullptr ? FALSE : ((wi->OwnMode >> 1) & 01);

				BOOL bForceEnable = GetAsyncKeyState(VK_CONTROL);

				nickItems[6].bDisabled = !(bForceEnable || bHalfop || bOp || bAdmin || bOwner);
				nickItems[7].uType = nickItems[8].uType = bServOwner ? MENU_POPUPITEM : 0;
				nickItems[9].uType = nickItems[10].uType = bServAdmin ? MENU_POPUPITEM : 0;
				nickItems[7].bDisabled = nickItems[8].bDisabled = nickItems[9].bDisabled = nickItems[10].bDisabled = !(bForceEnable || bOwner);
				nickItems[11].bDisabled = nickItems[12].bDisabled = nickItems[13].bDisabled = nickItems[14].bDisabled = !(bForceEnable || bOp || bAdmin || bOwner);

				Chat_AddMenuItems(gcmi->hMenu, _countof(nickItems), nickItems, &g_plugin);
			}
		}
	}

	return 0;
}

void CIrcProto::OnShutdown()
{
	mir_cslock lock(m_csSession);

	if (m_perform && IsConnected())
		if (DoPerform("Event: Disconnect"))
			Sleep(200);

	DisconnectAllDCCSessions(true);

	if (IsConnected())
		Disconnect();
	if (m_listDlg)
		m_listDlg->Close();
	if (m_nickDlg)
		m_nickDlg->Close();
	if (m_joinDlg)
		m_joinDlg->Close();
}

int __cdecl CIrcProto::OnMenuPreBuild(WPARAM hContact, LPARAM)
{
	DBVARIANT dbv;
	if (hContact == NULL)
		return 0;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto && !mir_strcmpi(szProto, m_szModuleName)) {
		bool bIsOnline = getWord(hContact, "Status", ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE;

		// context menu for chatrooms
		if (getByte(hContact, "ChatRoom", 0) == GCW_CHATROOM)
			Menu_ShowItem(hUMenuChanSettings, true);

		// context menu for contact
		else if (!getWString(hContact, "Default", &dbv)) {
			Menu_ShowItem(hUMenuChanSettings, false);

			// for DCC contact
			uint8_t bDcc = getByte(hContact, "DCC", 0);
			if (bDcc)
				Menu_ShowItem(hUMenuDisconnect, true);
			else {
				// for normal contact
				Menu_ShowItem(hUMenuWhois, IsConnected());

				bool bEnabled = true;
				if (bIsOnline) {
					DBVARIANT dbv3;
					if (!getString(hContact, "Host", &dbv3)) {
						if (dbv3.pszVal[0] == 0)
							bEnabled = false;
						db_free(&dbv3);
					}
				}
				Menu_ShowItem(hUMenuIgnore, bEnabled);
			}
			db_free(&dbv);
		}
	}

	return 0;
}

int __cdecl CIrcProto::OnDbSettingChanged(WPARAM hContact, LPARAM lParam)
{
	if (hContact == NULL || !IsConnected())
		return 0;

	DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;
	if (strcmp(cws->szModule, "CList"))
		return 0;

	if (cws->value.type != DBVT_DELETED && !(cws->value.type == DBVT_BYTE && cws->value.bVal == 0))
		return 0;

	if (!strcmp(cws->szSetting, "NotOnList")) {
		DBVARIANT dbv;
		if (!getWString(hContact, "Nick", &dbv)) {
			if (getByte("MirVerAutoRequest", 1))
				PostIrcMessage(L"/PRIVMSG %s \001VERSION\001", dbv.pwszVal);
			db_free(&dbv);
		}
	}
	return 0;
}
void __cdecl CIrcProto::ConnectServerThread(void*)
{
	Thread_SetName("IRC: ConnectServer");

	InterlockedIncrement((long *)&m_bConnectThreadRunning);
	InterlockedIncrement((long *)&m_bConnectRequested);
	while (!Miranda_IsTerminated() && m_bConnectRequested > 0) {
		while (m_bConnectRequested > 0)
			InterlockedDecrement((long *)&m_bConnectRequested);
		if (IsConnected()) {
			Sleep(200);
			Disconnect();
		}

		m_info.bNickFlag = false;
		int Temp = m_iStatus;
		m_iStatus = ID_STATUS_CONNECTING;
		nickflag = true;
		ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)Temp, ID_STATUS_CONNECTING);
		Sleep(100);
		{
			mir_cslock lock(m_csSession);
			Connect(m_sessionInfo);
		}
		if (IsConnected()) {
			if (m_mySpecifiedHost[0])
				ForkThread(&CIrcProto::ResolveIPThread, new IPRESOLVE(m_mySpecifiedHost, IP_MANUAL));
		}
		else {
			Temp = m_iDesiredStatus;
			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, nullptr, LOGINERR_NONETWORK);
			ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)Temp, ID_STATUS_OFFLINE);
			Sleep(100);
		}
	}

	InterlockedDecrement((long *)&m_bConnectThreadRunning);
}

void __cdecl CIrcProto::DisconnectServerThread(void*)
{
	Thread_SetName("IRC: DisconnectServer");

	mir_cslock lck(m_csSession);
	if (IsConnected())
		Disconnect();
}

void CIrcProto::ConnectToServer(void)
{
	m_portCount = atoi(m_portStart);
	m_sessionInfo.sServer = GetWord(m_serverName, 0);
	m_sessionInfo.iPort = m_portCount;
	m_sessionInfo.sNick = m_nick;
	m_sessionInfo.sUserID = m_userID;
	m_sessionInfo.sFullName = m_name;
	m_sessionInfo.sPassword = m_password;
	m_sessionInfo.bIdentServer = ((m_ident) ? (true) : (false));
	m_sessionInfo.iIdentServerPort = _wtoi(m_identPort);
	m_sessionInfo.sIdentServerType = m_identSystem;
	m_sessionInfo.m_iSSL = m_iSSL;

	bPerformDone = false;
	bTempDisableCheck = false;
	bTempForceCheck = false;
	m_iTempCheckTime = 0;
	sChannelPrefixes = L"&#";
	sUserModes = "ov";
	sUserModePrefixes = L"@+";
	sChannelModes = "btnimklps";

	if (!m_bConnectThreadRunning)
		ForkThread(&CIrcProto::ConnectServerThread, nullptr);
	else if (m_bConnectRequested < 1)
		InterlockedIncrement((long *)&m_bConnectRequested);

	wchar_t szTemp[300];
	mir_snwprintf(szTemp, L"\033%s %c%s%c (%S: %u)", TranslateT("Connecting to"), irc::BOLD, m_tszUserName, irc::BOLD, m_sessionInfo.sServer.c_str(), m_sessionInfo.iPort);
	DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, nullptr, szTemp, nullptr, nullptr, NULL, true, false);
}

void CIrcProto::DisconnectFromServer(void)
{
	if (m_perform && IsConnected())
		DoPerform("Event: Disconnect");

	Chat_Terminate(m_szModuleName, nullptr);
	ForkThread(&CIrcProto::DisconnectServerThread, nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetMyAwayMsg - obtain the current away message

INT_PTR __cdecl CIrcProto::GetMyAwayMsg(WPARAM wParam, LPARAM lParam)
{
	if ((int)wParam != m_iStatus)
		return 0;

	const wchar_t* p = m_statusMessage.c_str();

	return (lParam & SGMA_UNICODE) ? (INT_PTR)mir_wstrdup(p) : (INT_PTR)mir_u2a(p);
}
