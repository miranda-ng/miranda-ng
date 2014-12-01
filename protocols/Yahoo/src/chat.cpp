/*
 *
 * myYahoo Miranda Plugin
 *
 * Authors: Gennady Feldman (aka Gena01)
 *          Laurent Marechal (aka Peorth)
 *          Boris Krasnovskiy (aka borkra)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */
#include "yahoo.h"
#include "resource.h"
#include <m_protosvc.h>
#include <m_langpack.h>
#include <m_message.h>
#include <m_chat.h>
#include <m_userinfo.h>
#include <m_history.h>

struct InviteChatParam
{
	char* room;
	CYahooProto* ppro;


	InviteChatParam(const char* room, CYahooProto* ppro)
		: room(strdup(room)), ppro(ppro) {}

	~InviteChatParam()
	{ free(room); }
};

struct InviteChatReqParam
{
	char* room;
	char* who;
	TCHAR* msg;
	CYahooProto* ppro;

	InviteChatReqParam(const char* room, const char* who, const char* msg, CYahooProto* ppro)
		: room(mir_strdup(room)), who(mir_strdup(who)), msg(mir_utf8decodeT(msg)), ppro(ppro) {}

	~InviteChatReqParam()
	{ mir_free(room); mir_free(who); mir_free(msg); }
};

INT_PTR CALLBACK InviteToChatDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
void CALLBACK ConferenceRequestCB(PVOID dwParam);

/* Conference handlers */
void ext_yahoo_got_conf_invite(int id, const char *me, const char *who, const char *room, const char *msg, YList *members)
{
	char z[1024];
	mir_snprintf(z, SIZEOF(z), Translate("[miranda] Got conference invite to room: %s with msg: %s"), room ?room:"", msg ?msg:"");
	LOG(("[ext_yahoo_got_conf_invite] %s", z));

	CYahooProto* ppro = getProtoById(id);
	if (!ppro) return;

	bool freeList = true;
	CYahooProto::ChatRoom *cm = ppro->m_chatrooms.find((CYahooProto::ChatRoom*)&room);
	if (!cm)
	{
		if (strcmp(who, me))
		{
			cm = new CYahooProto::ChatRoom(room, members);
			ppro->m_chatrooms.insert(cm);

			InviteChatReqParam* req = new InviteChatReqParam(room, who, msg, ppro);
			CallFunctionAsync(ConferenceRequestCB, req);
			freeList = false;
		}
		else
		{
			cm = new CYahooProto::ChatRoom(room, NULL);
			ppro->m_chatrooms.insert(cm);
			ppro->ChatStart(room);

			yahoo_conference_logon(id, NULL, members, room);
		}
	}

	if (freeList)
	{
		for (YList *l = members; l; l = l->next) free(l->data);
		y_list_free(members);
	}
}

void ext_yahoo_conf_userdecline(int id, const char *me, const char *who, const char *room, const char *msg)
{
	TCHAR info[1024];
	TCHAR *whot = mir_utf8decodeT(who);
	TCHAR *msgt = mir_utf8decodeT(msg);
	mir_sntprintf(info, SIZEOF(info), TranslateT("%s denied invitation with message: %s"), whot, msgt ? msgt : _T(""));
	GETPROTOBYID(id)->ChatEvent(room, who, GC_EVENT_INFORMATION, info);
	mir_free(msgt);
	mir_free(whot);
}

void ext_yahoo_conf_userjoin(int id, const char *me, const char *who, const char *room)
{
	CYahooProto* ppro = getProtoById(id);
	if (!ppro) return;

	CYahooProto::ChatRoom *cm = ppro->m_chatrooms.find((CYahooProto::ChatRoom*)&room);
	for (YList *l = cm->members; l; l = l->next)
	{
		if (!strcmp(who, (char*)l->data))
			return;
	}

	cm->members = y_list_append(cm->members, strdup(who));
	ppro->ChatEvent(room, who, GC_EVENT_JOIN);
}

void ext_yahoo_conf_userleave(int id, const char *me, const char *who, const char *room)
{
	CYahooProto* ppro = getProtoById(id);
	if (!ppro) return;

	CYahooProto::ChatRoom *cm = ppro->m_chatrooms.find((CYahooProto::ChatRoom*)&room);
	if (!cm) return;

	for (YList *l = cm->members; l; l = l->next)
	{
		if (strcmp((char*)l->data, who) == 0)
		{
			free(l->data);
			y_list_remove_link(cm->members, l);
			y_list_free_1(l);
			break;
		}
	}

	ppro->ChatEvent(room, who, GC_EVENT_PART);
}

void ext_yahoo_conf_message(int id, const char *me, const char *who, const char *room, const char *msg, int utf8)
{
	TCHAR *msgt = utf8 ? mir_utf8decodeT(msg) : mir_a2t(msg);
	GETPROTOBYID(id)->ChatEvent(room, who, GC_EVENT_MESSAGE, msgt);
	mir_free(msgt);
}

/* chat handlers */
void ext_yahoo_chat_cat_xml(int id, const char *xml)
{
}

void ext_yahoo_chat_join(int id, const char *me, const char *room, const char * topic, YList *members, INT_PTR fd)
{
	for (YList *l = members; l; l = l->next) free(l->data);
	y_list_free(members);
}

void ext_yahoo_chat_userjoin(int id, const char *me, const char *room, struct yahoo_chat_member *who)
{
}

void ext_yahoo_chat_userleave(int id, const char *me, const char *room, const char *who)
{
}
void ext_yahoo_chat_message(int id, const char *me, const char *who, const char *room, const char *msg, int msgtype, int utf8)
{
}

void ext_yahoo_chat_yahoologout(int id, const char *me)
{
 	LOG(("got chat logout"));
}
void ext_yahoo_chat_yahooerror(int id, const char *me)
{
 	LOG(("got chat error"));
}

static const COLORREF crCols[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

void CYahooProto::ChatRegister(void)
{
	GCREGISTER gcr = { sizeof(gcr) };
	gcr.dwFlags = GC_TYPNOTIF | GC_CHANMGR;
	gcr.nColors = 16;
	gcr.pColors = (COLORREF*)crCols;
	gcr.ptszDispName = m_tszUserName;
	gcr.pszModule = m_szModuleName;
	CallServiceSync(MS_GC_REGISTER, 0, (LPARAM)&gcr);

	HookProtoEvent(ME_GC_EVENT, &CYahooProto::OnGCEventHook);
	HookProtoEvent(ME_GC_BUILDMENU, &CYahooProto::OnGCMenuHook);
}

void CYahooProto::ChatStart(const char* room)
{
	TCHAR* idt = mir_a2t(room);

	GCSESSION gcw = { sizeof(gcw) };
	gcw.iType = GCW_CHATROOM;
	gcw.pszModule = m_szModuleName;
	gcw.ptszName = idt;
	gcw.ptszID = idt;
	CallServiceSync(MS_GC_NEWSESSION, 0, (LPARAM)&gcw);

	GCDEST gcd = { m_szModuleName, idt, GC_EVENT_ADDGROUP };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.ptszStatus = TranslateT("Me");
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_ADDGROUP;
	gce.ptszStatus = TranslateT("Others");
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	gcd.iType = GC_EVENT_CONTROL;
	CallServiceSync(MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_ONLINE,   (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE,   (LPARAM)&gce);

	mir_free(idt);
}

void CYahooProto::ChatLeave(const char* room)
{
	TCHAR* idt = mir_a2t(room);

	GCDEST gcd = { m_szModuleName, idt, GC_EVENT_CONTROL };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_REMOVECONTACT;
	CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce);
	CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gce);

	mir_free(idt);
}

void CYahooProto::ChatLeaveAll(void)
{
	for (int i = 0; i < m_chatrooms.getCount(); ++i)
		ChatLeave(m_chatrooms[i].name);
}

void CYahooProto::ChatEvent(const char* room, const char* who, int evt, const TCHAR* msg)
{
	TCHAR* idt = mir_a2t(room);
	TCHAR* snt = mir_a2t(who);

	MCONTACT hContact = getbuddyH(who);
	TCHAR* nick = hContact ? (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, WPARAM(hContact), GCDNF_TCHAR) : snt;

	GCDEST gcd = { m_szModuleName, idt, evt };
	GCEVENT gce = { sizeof(gce), &gcd };
	gce.dwFlags = GCEF_ADDTOLOG;
	gce.ptszNick = nick;
	gce.ptszUID = snt;
	gce.bIsMe = _stricmp(who, m_yahoo_id) == 0;
	gce.ptszStatus = gce.bIsMe ? TranslateT("Me") : TranslateT("Others");
	gce.ptszText = msg;
	gce.time = time(NULL);
	CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gce);

	mir_free(snt);
	mir_free(idt);
}

int __cdecl CYahooProto::OnGCEventHook(WPARAM, LPARAM lParam)
{
	GCHOOK *gch = (GCHOOK*) lParam;
	if (!gch) return 1;

	if (strcmp(gch->pDest->pszModule, m_szModuleName)) return 0;

	char* room = mir_t2a(gch->pDest->ptszID);
	char* who = mir_t2a(gch->ptszUID);

	switch (gch->pDest->iType)
	{
		case GC_SESSION_TERMINATE:
			{
				ChatRoom *cm = m_chatrooms.find((ChatRoom*)&room);
				if (cm)
				{
					yahoo_conference_logoff(m_id, NULL, cm->members, room);
					m_chatrooms.remove((ChatRoom*)&room);
				}
			}
			break;

		case GC_USER_MESSAGE:
			if (gch->ptszText && gch->ptszText[0])
			{
				char* msg = mir_utf8encodeT(gch->ptszText);
				ChatRoom *cm = m_chatrooms.find((ChatRoom*)&room);
				if (cm)
					yahoo_conference_message(m_id, NULL, cm->members, room, msg, 1);
				mir_free(msg);
			}
			break;

		case GC_USER_CHANMGR:
			DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, InviteToChatDialog,
				LPARAM(new InviteChatParam(room, this)));
			break;

		case GC_USER_PRIVMESS:
			CallService(MS_MSG_SENDMESSAGE, (WPARAM)getbuddyH(who), 0);
			break;

		case GC_USER_LOGMENU:
			switch(gch->dwData)
			{
			case 10:
				DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, InviteToChatDialog,
					LPARAM(new InviteChatParam(room, this)));
				break;

			case 20:
				ChatLeave(room);
				break;
			}
			break;

		case GC_USER_NICKLISTMENU:
			switch (gch->dwData)
			{
			case 10:
				CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)getbuddyH(who), 0);
				break;

			case 20:
				CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)getbuddyH(who), 0);
				break;

			case 110:
				ChatLeave(room);
				break;
			}
			break;

		case GC_USER_TYPNOTIFY:
			break;
	}
	mir_free(who);
	mir_free(room);

	return 0;
}

int __cdecl CYahooProto::OnGCMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi = (GCMENUITEMS*) lParam;

	if (gcmi == NULL || _stricmp(gcmi->pszModule, m_szModuleName)) return 0;

	if (gcmi->Type == MENU_ON_LOG)
	{
		static const struct gc_item Items[] =
		{
			{ TranslateT("&Invite user..."), 10, MENU_ITEM, FALSE },
			{ TranslateT("&Leave chat session"), 20, MENU_ITEM, FALSE }
		};
		gcmi->nItems = SIZEOF(Items);
		gcmi->Item = (gc_item*)Items;
	}
	else if (gcmi->Type == MENU_ON_NICKLIST)
	{
		char* id = mir_t2a(gcmi->pszUID);
		if (!_stricmp(m_yahoo_id, id))
		{
			static const struct gc_item Items[] =
			{
				{ TranslateT("User &details"), 10, MENU_ITEM, FALSE },
				{ TranslateT("User &history"), 20, MENU_ITEM, FALSE },
				{ _T(""), 100, MENU_SEPARATOR, FALSE },
				{ TranslateT("&Leave chat session"), 110, MENU_ITEM, FALSE }
			};
			gcmi->nItems = SIZEOF(Items);
			gcmi->Item = (gc_item*)Items;
		}
		else
		{
			static const struct gc_item Items[] =
			{
				{ TranslateT("User &details"), 10, MENU_ITEM, FALSE },
				{ TranslateT("User &history"), 20, MENU_ITEM, FALSE }
			};
			gcmi->nItems = SIZEOF(Items);
			gcmi->Item = (gc_item*)Items;
		}
		mir_free(id);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Invite to chat dialog

static void clist_chat_invite_send(MCONTACT hItem, HWND hwndList, YList* &who, char* room, CYahooProto* ppro, TCHAR *msg)
{
	bool root = !hItem;
	if (root)
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem)
	{
		if (IsHContactGroup(hItem))
		{
			MCONTACT hItemT = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT)
				clist_chat_invite_send(hItemT, hwndList, who, room, ppro, msg);
		}
		else
		{
			int chk = SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0);
			if (chk)
			{
				if (IsHContactInfo(hItem))
				{
					TCHAR buf[128] = _T("");
					SendMessage(hwndList, CLM_GETITEMTEXT, (WPARAM)hItem, (LPARAM)buf);

					who = y_list_append(who, mir_t2a(buf));
				}
				else
				{
					DBVARIANT dbv;
					if (!ppro->getString(hItem, YAHOO_LOGINID, &dbv))
						who = y_list_append(who, dbv.pszVal);
				}
			}
		}
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);
	}

	if (root && who)
	{
		char *msg8 = mir_utf8encodeT(msg);
		CYahooProto::ChatRoom *cm = ppro->m_chatrooms.find((CYahooProto::ChatRoom*)&room);
		if (cm)
		{
			for (YList *l = who; l; l = l->next)
				yahoo_conference_addinvite(ppro->m_id, NULL, (char*)l->data, room, cm->members, msg8);
		}
		else
			yahoo_conference_invite(ppro->m_id, NULL, who, room, msg8);

		mir_free(msg8);

		for (YList *l = who; l; l = l->next) mir_free(l->data);
		y_list_free(who);
	}
}

static void ClistValidateContact(MCONTACT hItem, HWND hwndList, CYahooProto* ppro)
{
	if (!ppro->IsMyContact(hItem) || ppro->isChatRoom(hItem) ||
		ppro->getWord(hItem, "Status", ID_STATUS_OFFLINE) == ID_STATUS_ONTHEPHONE)
		SendMessage(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
}

static void ClistChatPrepare(MCONTACT hItem, HWND hwndList, CYahooProto* ppro)
{
	if (hItem == NULL)
		hItem = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_ROOT, 0);

	while (hItem) {
		MCONTACT hItemN = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_NEXT, (LPARAM)hItem);

		if (IsHContactGroup(hItem)) {
			MCONTACT hItemT = (MCONTACT)SendMessage(hwndList, CLM_GETNEXTITEM, CLGN_CHILD, (LPARAM)hItem);
			if (hItemT)
				ClistChatPrepare(hItemT, hwndList, ppro);
		}
		else if (IsHContactContact(hItem))
			ClistValidateContact(hItem, hwndList, ppro);

		hItem = hItemN;
   }
}

INT_PTR CALLBACK InviteToChatDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	InviteChatParam* param = (InviteChatParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		param = (InviteChatParam*)lParam;

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)param->ppro->LoadIconEx("yahoo", true));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)param->ppro->LoadIconEx("yahoo"));

		SetDlgItemTextA(hwndDlg, IDC_ROOMNAME, param->room);
		SetDlgItemText(hwndDlg, IDC_MSG, TranslateT("Join My Conference..."));
		break;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		break;

	case WM_NCDESTROY:
		param->ppro->ReleaseIconEx("yahoo", true);
		param->ppro->ReleaseIconEx("yahoo");
		delete param;
		break;

	case WM_NOTIFY:
	{
		NMCLISTCONTROL* nmc = (NMCLISTCONTROL*)lParam;
		if (nmc->hdr.idFrom == IDC_CCLIST)
		{
			switch (nmc->hdr.code)
			{
			case CLN_NEWCONTACT:
				if (param && (nmc->flags & (CLNF_ISGROUP | CLNF_ISINFO)) == 0)
					ClistValidateContact((MCONTACT)nmc->hItem, nmc->hdr.hwndFrom, param->ppro);
				break;

			case CLN_LISTREBUILT:
				if (param)
					ClistChatPrepare(NULL, nmc->hdr.hwndFrom, param->ppro);
				break;
			}
		}
	}
	break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_ADDSCR:
				if (param->ppro->m_bLoggedIn)
				{
					TCHAR sn[64];
					GetDlgItemText(hwndDlg, IDC_EDITSCR, sn, SIZEOF(sn));

					CLCINFOITEM cii = {0};
					cii.cbSize = sizeof(cii);
					cii.flags = CLCIIF_CHECKBOX | CLCIIF_BELOWCONTACTS;
					cii.pszText = sn;

					HANDLE hItem = (HANDLE)SendDlgItemMessage(hwndDlg, IDC_CCLIST, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
					SendDlgItemMessage(hwndDlg, IDC_CCLIST, CLM_SETCHECKMARK, (LPARAM)hItem, 1);
				}
				break;

			case IDOK:
				{
					TCHAR msg[1024];
					GetDlgItemText(hwndDlg, IDC_MSG, msg, SIZEOF(msg));

					HWND hwndList = GetDlgItem(hwndDlg, IDC_CCLIST);
					YList *who = NULL;
					clist_chat_invite_send(NULL, hwndList, who, param->room, param->ppro, msg);

					EndDialog(hwndDlg, IDOK);
					PostMessage(hwndDlg, WM_DESTROY, 0, 0);
				}
				break;

			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				PostMessage(hwndDlg, WM_DESTROY, 0, 0);
				break;
			}
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Chat request dialog

INT_PTR CALLBACK ChatRequestDialog(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	InviteChatReqParam* param = (InviteChatReqParam*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		param = (InviteChatReqParam*)lParam;

		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)param->ppro->LoadIconEx("yahoo", true));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)param->ppro->LoadIconEx("yahoo"));

		SetDlgItemTextA(hwndDlg, IDC_SCREENNAME, param->who);
		SetDlgItemText(hwndDlg, IDC_MSG, param->msg);
		SetDlgItemText(hwndDlg, IDC_MSG2, TranslateT("No, thank you..."));
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_NCDESTROY:
		param->ppro->ReleaseIconEx("yahoo", true);
		param->ppro->ReleaseIconEx("yahoo");
		delete param;
		break;

	case WM_COMMAND:
		{
			CYahooProto::ChatRoom *cm = param->ppro->m_chatrooms.find((CYahooProto::ChatRoom*)&param->room);
			switch (LOWORD(wParam))
			{
			case IDOK:
				if (cm)
				{
					param->ppro->ChatStart(param->room);

					for (YList *l = cm->members; l; l = l->next)
						param->ppro->ChatEvent(param->room, (char*)l->data, GC_EVENT_JOIN);

					yahoo_conference_logon(param->ppro->m_id, NULL, cm->members, param->room);
				}
				DestroyWindow(hwndDlg);
				break;

			case IDCANCEL:
				if (cm)
				{
					TCHAR msg[1024];
					GetDlgItemText(hwndDlg, IDC_MSG2, msg, SIZEOF(msg));
					char *msg8 = mir_utf8encodeT(msg);
					yahoo_conference_decline(param->ppro->m_id, NULL, cm->members, param->room, msg8);
					mir_free(msg8);

					param->ppro->m_chatrooms.remove((CYahooProto::ChatRoom*)&param->room);
				}
				DestroyWindow(hwndDlg);
				break;
			}
		}
		break;
	}
	return FALSE;
}

static void CALLBACK ConferenceRequestCB(PVOID pParam)
{
	CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_CHATROOM_INVITE_REQ),
		 NULL, ChatRequestDialog, (LPARAM)pParam);
}

INT_PTR __cdecl CYahooProto::CreateConference(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	char room[128];
	mir_snprintf(room, SIZEOF(room), "%s-%u", m_yahoo_id, time(NULL));

	InviteChatParam* param = new InviteChatParam(room, this);
	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL,
		InviteToChatDialog, LPARAM(param));
	return 0;
}
