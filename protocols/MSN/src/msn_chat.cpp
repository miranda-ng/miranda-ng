/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "msn_global.h"
#include "msn_proto.h"
#include <m_history.h>

static const TCHAR *m_ptszRoles[] = {
	_T("admin"),
	_T("user")
};

MCONTACT CMsnProto::MSN_GetChatInernalHandle(MCONTACT hContact)
{
	MCONTACT result = hContact;
	if (isChatRoom(hContact)) {
		DBVARIANT dbv;
		if (getString(hContact, "ChatRoomID", &dbv) == 0) {
			result = (MCONTACT)(-atol(dbv.pszVal));
			db_free(&dbv);
		}
	}
	return result;
}

int CMsnProto::MSN_ChatInit(GCThreadData *info, const char *pszID, const char *pszTopic)
{
	char *szNet, *szEmail;

	_tcsncpy(info->mChatID, _A2T(pszID), SIZEOF(info->mChatID));
	parseWLID(NEWSTR_ALLOCA(pszID), &szNet, &szEmail, NULL);
	info->netId = atoi(szNet);
	strncpy(info->szEmail, szEmail, sizeof(info->szEmail));

	TCHAR szName[512];
	InterlockedIncrement(&m_chatID);
	if (*pszTopic) _tcsncpy(szName, _A2T(pszTopic), SIZEOF(szName));
	else mir_sntprintf(szName, SIZEOF(szName), _T("%s %s%d"),
		m_tszUserName, TranslateT("Chat #"), m_chatID);

	GCSESSION gcw = { sizeof(gcw) };
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = szName;
	gcw.ptszID = info->mChatID;
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	GCDEST gcd = { m_szModuleName, info->mChatID, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };
	for (int j = 0; j < SIZEOF(m_ptszRoles); j++) {
		gce.ptszStatus = m_ptszRoles[j];
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	}

	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE, (LPARAM)&gce);

	mir_free((TCHAR*)gce.ptszUID);
	return 0;
}

void CMsnProto::MSN_ChatStart(ezxml_t xmli)
{
	const char *pszCreator;
	
	int j;

	if (!mir_strcmp(xmli->txt, "thread")) return;
	
	// If Chat ID already exists, don'T create a new one
	const char *pszID = ezxml_txt(ezxml_child(xmli, "id"));
	GCThreadData* info = MSN_GetThreadByChatId(_A2T(pszID));
	if (info == NULL) {
		info = new GCThreadData;
		{
			mir_cslock lck(m_csThreads);
			m_arGCThreads.insert(info);
		}

		MSN_ChatInit(info, pszID, ezxml_txt(ezxml_get(xmli, "properties", 0, "topic", -1)));
		MSN_StartStopTyping(info, false);
	} else  {
		GCDEST gcd = { m_szModuleName, info->mChatID, GC_EVENT_CONTROL };
		GCEVENT gce = { sizeof(gce), &gcd };
		CallServiceSync(MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce);
	}

	pszCreator = ezxml_txt(ezxml_get(xmli, "properties", 0, "creator", -1));

	for (ezxml_t memb = ezxml_get(xmli, "members", 0, "member", -1); memb != NULL; memb = ezxml_next(memb)) {
		const char *mri = ezxml_txt(ezxml_child(memb, "mri"));
		const char *role = ezxml_txt(ezxml_child(memb, "role"));
		GCUserItem *gcu = NULL;

		for (j = 0; j < info->mJoinedContacts.getCount(); j++) {
			if (!mir_strcmp(info->mJoinedContacts[j]->WLID, mri)) {
				gcu = info->mJoinedContacts[j];
				break;
			}
		}
		if (!gcu) {
			gcu = new GCUserItem;
			info->mJoinedContacts.insert(gcu);
			strncpy(gcu->WLID, mri, sizeof(gcu->WLID));
		}
		_tcscpy(gcu->role, _A2T(role));

		if (pszCreator && !mir_strcmp(mri, pszCreator)) info->mCreator = gcu;
		char* szEmail, *szNet;
		parseWLID(NEWSTR_ALLOCA(mri), &szNet, &szEmail, NULL);
		if (!stricmp(szEmail, GetMyUsername(atoi(szNet))))
			info->mMe = gcu;
		gcu->btag = 1;
	}

	// Remove contacts not on list (not tagged)
	for (j = 0; j < info->mJoinedContacts.getCount(); j++) {
		if (!info->mJoinedContacts[j]->btag) {
			info->mJoinedContacts.remove(j);
			j--;
		} else info->mJoinedContacts[j]->btag = 0;
	}
}


void CMsnProto::MSN_KillChatSession(const TCHAR* id)
{
	GCDEST gcd = { m_szModuleName, id, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_REMOVECONTACT;
	CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);
}

void CMsnProto::MSN_Kickuser(GCHOOK *gch)
{
	GCThreadData* thread = MSN_GetThreadByChatId(gch->pDest->ptszID);
	msnNsThread->sendPacketPayload("DEL", "MSGR\\THREAD", 
		"<thread><id>%d:%s</id><members><member><mri>%s</mri></member></members></thread>",
		thread->netId, thread->szEmail, _T2A(gch->ptszUID));
}

void CMsnProto::MSN_Promoteuser(GCHOOK *gch, const char *pszRole)
{
	GCThreadData* thread = MSN_GetThreadByChatId(gch->pDest->ptszID);
	msnNsThread->sendPacketPayload("PUT", "MSGR\\THREAD", 
		"<thread><id>%d:%s</id><members><member><mri>%s</mri><role>%s</role></member></members></thread>",
		thread->netId, thread->szEmail, _T2A(gch->ptszUID), pszRole);
}

const TCHAR *CMsnProto::MSN_GCGetRole(GCThreadData* thread, const char *pszWLID) 
{
	if (thread) {
		for (int j = 0; j < thread->mJoinedContacts.getCount(); j++) {
			if (!mir_strcmp(thread->mJoinedContacts[j]->WLID, pszWLID)) {
				return thread->mJoinedContacts[j]->role;
			}
		}
	}
	return NULL;
}

void CMsnProto::MSN_GCProcessThreadActivity(ezxml_t xmli, const TCHAR *mChatID)
{
	if (!mir_strcmp(xmli->name, "topicupdate")) {
		ezxml_t initiator = ezxml_child(xmli, "initiator");
		GCDEST gcd = { m_szModuleName, mChatID, GC_EVENT_TOPIC};
		GCEVENT gce = { sizeof(gce), &gcd };
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.time = MsnTSToUnixtime(ezxml_txt(ezxml_child(xmli, "eventtime")));
		gce.ptszUID = initiator?mir_a2t(initiator->txt):NULL;
		MCONTACT hContInitiator = MSN_HContactFromEmail(initiator->txt);
		gce.ptszNick = GetContactNameT(hContInitiator);
		gce.ptszText = mir_a2t(ezxml_txt(ezxml_child(xmli, "value")));
		CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
		mir_free((TCHAR*)gce.ptszUID);
		mir_free((TCHAR*)gce.ptszText);
	}
	else if (ezxml_t target = ezxml_child(xmli, "target")) {
		MCONTACT hContInitiator = NULL;
		GCDEST gcd = { m_szModuleName, mChatID, 0};
		GCEVENT gce = { sizeof(gce), &gcd };
		gce.dwFlags = GCEF_ADDTOLOG;

		if (!mir_strcmp(xmli->name, "deletemember")) {
			gcd.iType = GC_EVENT_PART;
			if (ezxml_t initiator = ezxml_child(xmli, "initiator")) {
				if (mir_strcmp(initiator->txt, target->txt)) {
					hContInitiator = MSN_HContactFromEmail(initiator->txt);
					gce.ptszStatus = GetContactNameT(hContInitiator);
					gcd.iType = GC_EVENT_KICK;
				}
			}
		}
		else if (!mir_strcmp(xmli->name, "addmember")) {
			gcd.iType = GC_EVENT_JOIN;
		}
		else if (!mir_strcmp(xmli->name, "roleupdate")) {
			gcd.iType = GC_EVENT_ADDSTATUS;
			if (ezxml_t initiator = ezxml_child(xmli, "initiator")) {
				hContInitiator = MSN_HContactFromEmail(initiator->txt);
				gce.ptszText= GetContactNameT(hContInitiator);
			}
			gce.ptszStatus = _T("admin");
		}

		if (gcd.iType) {
			gce.time = MsnTSToUnixtime(ezxml_txt(ezxml_child(xmli, "eventtime")));
			const char *pszTarget = NULL;

			while (target) {
				switch (gcd.iType)
				{
				case GC_EVENT_JOIN:
					gce.ptszStatus = MSN_GCGetRole(MSN_GetThreadByChatId(mChatID), target->txt);
					// ..fall through.. //
				case GC_EVENT_KICK:
				case GC_EVENT_PART:
					pszTarget = target->txt;
					break;
				case GC_EVENT_ADDSTATUS:
				case GC_EVENT_REMOVESTATUS:
					gcd.iType = mir_strcmp(ezxml_txt(ezxml_child(target, "role")), "admin")==0?GC_EVENT_ADDSTATUS:GC_EVENT_REMOVESTATUS;
					pszTarget = ezxml_txt(ezxml_child(target, "id"));
					break;
				}
				char* szEmail, *szNet;
				parseWLID(NEWSTR_ALLOCA(pszTarget), &szNet, &szEmail, NULL);
				gce.bIsMe = !stricmp(szEmail, GetMyUsername(atoi(szNet)));
				gce.ptszUID = mir_a2t(pszTarget);
				MCONTACT hContTarget = MSN_HContactFromEmail(pszTarget);
				gce.ptszNick =GetContactNameT(hContTarget);
				CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
				if ((gcd.iType == GC_EVENT_PART || gcd.iType == GC_EVENT_KICK) && gce.bIsMe) {
					GCDEST gcd = { m_szModuleName, mChatID, GC_EVENT_CONTROL };
					GCEVENT gce = { sizeof(gce), &gcd };
					CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
					break;
				}
				mir_free((TCHAR*)gce.ptszUID);
				target = ezxml_next(target);
			}
		}
	}
}

void CMsnProto::MSN_GCRefreshThreadsInfo(void)
{
	CMStringA buf;
	MCONTACT hContact;
	int nThreads = 0;

	for (hContact = db_find_first(m_szModuleName); hContact; 
			hContact = db_find_next(hContact, m_szModuleName)) 
	{
		if (isChatRoom(hContact) != 0) {
			DBVARIANT dbv;
			if (getString(hContact, "ChatRoomID", &dbv) == 0) {
				buf.AppendFormat("<thread><id>%s</id></thread>", dbv.pszVal);
				nThreads++;
				db_free(&dbv);
			}
		}
	}
	if (nThreads)
		msnNsThread->sendPacketPayload("GET", "MSGR\\THREADS", "<threads>%s</threads>", buf);
}

void CMsnProto::MSN_GCAddMessage(TCHAR *mChatID, MCONTACT hContact, char *email, time_t ts, bool sentMsg, char *msgBody)
{
	GCDEST gcd = { m_szModuleName, mChatID, GC_EVENT_MESSAGE };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszUID = mir_a2t(email);
	gce.ptszNick = GetContactNameT(hContact);
	gce.time = ts;
	gce.bIsMe = sentMsg;

	TCHAR* p = mir_utf8decodeT(msgBody);
	gce.ptszText = EscapeChatTags(p);
	mir_free(p);

	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);
	mir_free((void*)gce.ptszUID);
	mir_free((void*)gce.ptszText);
}



static void ChatInviteUser(ThreadData *thread, GCThreadData* info, const char* wlid)
{
	if (info->mJoinedContacts.getCount()) {
		for (int j = 0; j < info->mJoinedContacts.getCount(); j++) {
			if (_stricmp(info->mJoinedContacts[j]->WLID, wlid) == 0)
				return;
		}
	}
	thread->sendPacketPayload("PUT", "MSGR\\THREAD", 
		"<thread><id>%d:%s</id><members><member><mri>%s</mri><role>user</role></member></members></thread>", 
		info->netId, info->szEmail, wlid);
}

static void ChatInviteSend(HANDLE hItem, HWND hwndList, STRLIST &str, CMsnProto *ppro)
{
	if (hItem == NULL)
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) {
		if (IsHContactGroup(hItem)) {
			HANDLE hItemT = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT) ChatInviteSend(hItemT, hwndList, str, ppro);
		}
		else {
			int chk = SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0);
			if (chk) {
				if (IsHContactInfo(hItem)) {
					TCHAR buf[128] = _T("");
					SendMessage(hwndList, CLM_GETITEMTEXT, (WPARAM)hItem, (LPARAM)buf);

					if (buf[0]) str.insert(mir_t2a(buf));
				}
				else {
					MsnContact *msc = ppro->Lists_Get((MCONTACT)hItem);
					if (msc) {
						char szContact[MSN_MAX_EMAIL_LEN];

						sprintf(szContact, "%d:%s", msc->netId, msc->email);
						str.insertn(szContact);
					}
				}
			}
		}
		hItem = (HANDLE)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);
	}
}


static void ChatValidateContact(MCONTACT hItem, HWND hwndList, CMsnProto* ppro)
{
	if (!ppro->MSN_IsMyContact(hItem) || ppro->isChatRoom(hItem) || ppro->MSN_IsMeByContact(hItem))
		SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
}

static void ChatPrepare(MCONTACT hItem, HWND hwndList, CMsnProto* ppro)
{
	if (hItem == NULL)
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) {
		MCONTACT hItemN = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);

		if (IsHContactGroup(hItem)) {
			MCONTACT hItemT = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT)
				ChatPrepare(hItemT, hwndList, ppro);
		}
		else if (IsHContactContact(hItem))
			ChatValidateContact(hItem, hwndList, ppro);

		hItem = hItemN;
	}
}

INT_PTR CALLBACK DlgInviteToChat(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	InviteChatParam *param = (InviteChatParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		param = (InviteChatParam*)lParam;

		//		WindowSetIcon(hwndDlg, "msn");
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_NCDESTROY:
		//		WindowFreeIcon(hwndDlg);
		delete param;
		break;

	case WM_NOTIFY:
		NMCLISTCONTROL* nmc;
		{
			nmc = (NMCLISTCONTROL*)lParam;
			if (nmc->hdr.idFrom == IDC_CCLIST) {
				switch (nmc->hdr.code) {
				case CLN_NEWCONTACT:
					if (param && (nmc->flags & (CLNF_ISGROUP | CLNF_ISINFO)) == 0)
						ChatValidateContact((MCONTACT)nmc->hItem, nmc->hdr.hwndFrom, param->ppro);
					break;

				case CLN_LISTREBUILT:
					if (param)
						ChatPrepare(NULL, nmc->hdr.hwndFrom, param->ppro);
					break;
				}
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ADDSCR:
			if (param->ppro->msnLoggedIn) {
				TCHAR email[MSN_MAX_EMAIL_LEN];
				GetDlgItemText(hwndDlg, IDC_EDITSCR, email, SIZEOF(email));

				CLCINFOITEM cii = { 0 };
				cii.cbSize = sizeof(cii);
				cii.flags = CLCIIF_CHECKBOX | CLCIIF_BELOWCONTACTS;
				cii.pszText = _tcslwr(email);

				HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CCLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
				SendDlgItemMessage(hwndDlg, IDC_CCLIST, CLM_SETCHECKMARK, (LPARAM)hItem, 1);
			}
			break;

		case IDCANCEL:
			EndDialog(hwndDlg, IDCANCEL);
			break;

		case IDOK:
			char tEmail[MSN_MAX_EMAIL_LEN]; tEmail[0] = 0;
			GCThreadData *info = NULL;
			if (param->id)
				info = param->ppro->MSN_GetThreadByChatId(param->id);
			/*else if (param->hContact) {
				if (!param->ppro->MSN_IsMeByContact(param->hContact, tEmail))
					info = param->ppro->MSN_GetThreadByContact(tEmail);
			}*/

			HWND hwndList = GetDlgItem(hwndDlg, IDC_CCLIST);
			STRLIST *cont = new STRLIST;
			ChatInviteSend(NULL, hwndList, *cont, param->ppro);

			if (info) {
				for (int i = 0; i < cont->getCount(); ++i)
					ChatInviteUser(param->ppro->msnNsThread, info, (*cont)[i]);
				delete cont;
			}
			else {
				/* Group chats only work for Skype users */
				CMStringA buf;
				buf.AppendFormat("<thread><id></id><members><member><mri>%d:%s</mri><role>admin</role></member>",
					NETID_SKYPE, param->ppro->GetMyUsername(NETID_SKYPE));
				for (int i = 0; i < cont->getCount(); ++i) {
					// TODO: Add support for assigning role in invite dialog maybe?
					buf.AppendFormat("<member><mri>%s</mri><role>user</role></member>", (*cont)[i]);
				}
				buf.Append("</members></thread>");
				param->ppro->msnNsThread->sendPacketPayload("PUT", "MSGR\\THREAD", buf);
			}

			EndDialog(hwndDlg, IDOK);
		}
		break;
	}
	return FALSE;
}

int CMsnProto::MSN_GCEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*)lParam;
	if (!gch)
		return 1;

	if (_stricmp(gch->pDest->pszModule, m_szModuleName)) return 0;

	switch (gch->pDest->iType) {
	case GC_SESSION_TERMINATE:
		{
			GCThreadData* thread = MSN_GetThreadByChatId(gch->pDest->ptszID);
			if (thread != NULL) {
				m_arGCThreads.remove(thread);
				for (int i=0; i < thread->mJoinedContacts.getCount(); i++)
					delete thread->mJoinedContacts[i];
				delete thread;
			}
		}
		break;

	case GC_USER_MESSAGE:
		if (gch->ptszText && gch->ptszText[0]) {
			GCThreadData* thread = MSN_GetThreadByChatId(gch->pDest->ptszID);
			if (thread) {
				TCHAR* pszMsg = UnEscapeChatTags(NEWTSTR_ALLOCA(gch->ptszText));
				rtrimt(pszMsg); // remove the ending linebreak
				msnNsThread->sendMessage('N', thread->szEmail, thread->netId, UTF8(pszMsg), 0);

				DBVARIANT dbv;
				int bError = getTString("Nick", &dbv);

				GCDEST gcd = { m_szModuleName, gch->pDest->ptszID, GC_EVENT_MESSAGE };
				GCEVENT gce = { sizeof(gce), &gcd };
				gce.dwFlags = GCEF_ADDTOLOG;
				gce.ptszNick = bError ? _T("") : dbv.ptszVal;
				gce.ptszUID = mir_a2t(MyOptions.szEmail);
				gce.time = time(NULL);
				gce.ptszText = gch->ptszText;
				gce.bIsMe = TRUE;
				CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

				mir_free((void*)gce.ptszUID);
				if (!bError)
					db_free(&dbv);
			}
		}
		break;

	case GC_USER_CHANMGR:
		DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, DlgInviteToChat,
			LPARAM(new InviteChatParam(gch->pDest->ptszID, NULL, this)));
		break;

	case GC_USER_PRIVMESS:
	{
		char *email = mir_t2a(gch->ptszUID);
		MCONTACT hContact = MSN_HContactFromEmail(email);
		CallService(MS_MSG_SENDMESSAGE, hContact, 0);
		mir_free(email);
		break;
	}

	case GC_USER_LOGMENU:
		switch (gch->dwData) {
		case 10:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, DlgInviteToChat,
				LPARAM(new InviteChatParam(gch->pDest->ptszID, NULL, this)));
			break;

		case 20:
			MSN_KillChatSession(gch->pDest->ptszID);
			break;
		}
		break;

	case GC_USER_NICKLISTMENU:
		MCONTACT hContact = MSN_HContactFromEmail(_T2A(gch->ptszUID));

		switch (gch->dwData) {
		case 10:
			CallService(MS_USERINFO_SHOWDIALOG, hContact, 0);
			break;

		case 20:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, hContact, 0);
			break;

		case 30:
			MSN_Kickuser(gch);
			break;

		case 40:
			{
				const TCHAR *pszRole = MSN_GCGetRole(MSN_GetThreadByChatId(gch->pDest->ptszID), _T2A(gch->ptszUID));
				MSN_Promoteuser(gch, (pszRole && !mir_tstrcmp(pszRole, _T("admin")))?"user":"admin");
				break;
			}
		case 110:
			MSN_KillChatSession(gch->pDest->ptszID);
			break;
		}
		break;

	/*	haven't implemented in chat.dll
			case GC_USER_TYPNOTIFY:
			{
			int chatID = atoi(p);
			ThreadData* thread = MSN_GetThreadByContact((HANDLE)-chatID);
			for (int j=0; j < thread->mJoinedCount; j++)
			{
			if ((long)thread->mJoinedContacts[j] > 0)
			CallService(MS_PROTO_SELFISTYPING, (WPARAM) thread->mJoinedContacts[j], (LPARAM) PROTOTYPE_SELFTYPING_ON);
			}
			break;
			}
			*/
	}

	return 0;
}

int CMsnProto::MSN_GCMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*)lParam;

	if (gcmi == NULL || _stricmp(gcmi->pszModule, m_szModuleName)) return 0;

	if (gcmi->Type == MENU_ON_LOG) {
		static const struct gc_item Items[] =
		{
			{ LPGENT("&Invite user..."), 10, MENU_ITEM, FALSE },
			{ LPGENT("&Leave chat session"), 20, MENU_ITEM, FALSE }
		};
		gcmi->nItems = SIZEOF(Items);
		gcmi->Item = (gc_item*)Items;
	}
	else if (gcmi->Type == MENU_ON_NICKLIST) {
		char *email = mir_t2a(gcmi->pszUID);
		if (!_stricmp(GetMyUsername(NETID_SKYPE), email)) {
			static const struct gc_item Items[] =
			{
				{ LPGENT("User &details"), 10, MENU_ITEM, FALSE },
				{ LPGENT("User &history"), 20, MENU_ITEM, FALSE },
				{ _T(""), 100, MENU_SEPARATOR, FALSE },
				{ LPGENT("&Leave chat session"), 110, MENU_ITEM, FALSE }
			};
			gcmi->nItems = SIZEOF(Items);
			gcmi->Item = (gc_item*)Items;
		}
		else {
			static struct gc_item Items[] =
			{
				{ LPGENT("User &details"), 10, MENU_ITEM, FALSE },
				{ LPGENT("User &history"), 20, MENU_ITEM, FALSE },
				{ LPGENT("&Kick user")   , 30, MENU_ITEM, FALSE },
				{ LPGENT("&Op user")     , 40, MENU_ITEM, FALSE }
			};
			GCThreadData* thread = MSN_GetThreadByChatId(gcmi->pszID);
			if (thread && thread->mMe && _tcsicmp(thread->mMe->role, _T("admin"))) {
				Items[2].bDisabled = TRUE;
				Items[3].bDisabled = TRUE;
			} else {
				const TCHAR *pszRole = MSN_GCGetRole(thread, email);
				if (pszRole && !_tcsicmp(pszRole, _T("admin")))
					Items[3].pszDesc = LPGENT("&Deop user");
			}
			gcmi->nItems = SIZEOF(Items);
			gcmi->Item = (gc_item*)Items;
		}
		mir_free(email);
	}

	return 0;
}
