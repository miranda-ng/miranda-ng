////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2009 Adam Strzelecki <ono+miranda@java.pl>
// Copyright (c) 2009-2012 Bartosz Bia³ek
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"
#include <io.h>

//////////////////////////////////////////////////////////
// Status mode -> DB
char *gg_status2db(int status, const char *suffix)
{
	char *prefix;
	static char str[64];

	switch(status) {
		case ID_STATUS_AWAY:		prefix = "Away"; break;
		case ID_STATUS_NA:			prefix = "Na"; break;
		case ID_STATUS_DND:			prefix = "Dnd"; break;
		case ID_STATUS_OCCUPIED:	prefix = "Occupied"; break;
		case ID_STATUS_FREECHAT:	prefix = "FreeChat"; break;
		case ID_STATUS_ONLINE:		prefix = "On"; break;
		case ID_STATUS_OFFLINE:		prefix = "Off"; break;
		case ID_STATUS_INVISIBLE:	prefix = "Inv"; break;
		case ID_STATUS_ONTHEPHONE:	prefix = "Otp"; break;
		case ID_STATUS_OUTTOLUNCH:	prefix = "Otl"; break;
		default: return NULL;
	}
	strncpy(str, prefix, sizeof(str));
	strncat(str, suffix, sizeof(str) - strlen(str));
	return str;
}

//////////////////////////////////////////////////////////
// checks proto capabilities
DWORD_PTR gg_getcaps(PROTO_INTERFACE *proto, int type, HANDLE hContact)
{
	switch (type) {
		case PFLAGNUM_1:
			return PF1_IM | PF1_BASICSEARCH | PF1_EXTSEARCH | PF1_EXTSEARCHUI | PF1_SEARCHBYNAME |
				   PF1_MODEMSG | PF1_NUMERICUSERID | PF1_VISLIST | PF1_FILE;
		case PFLAGNUM_2:
			return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_FREECHAT | PF2_INVISIBLE |
				   PF2_LONGAWAY;
		case PFLAGNUM_3:
			return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND | PF2_FREECHAT | PF2_INVISIBLE;
		case PFLAGNUM_4:
			return PF4_NOCUSTOMAUTH | PF4_SUPPORTTYPING | PF4_AVATARS | PF4_IMSENDOFFLINE;
		case PFLAGNUM_5:
			return PF2_LONGAWAY;
		case PFLAG_UNIQUEIDTEXT:
			return (DWORD_PTR) Translate("Gadu-Gadu Number");
		case PFLAG_UNIQUEIDSETTING:
			return (DWORD_PTR) GG_KEY_UIN;
	}
	return 0;
}

//////////////////////////////////////////////////////////
// loads protocol icon
HICON gg_geticon(PROTO_INTERFACE *proto, int iconIndex)
{
	if (LOWORD(iconIndex) == PLI_PROTOCOL)
	{
		HICON hIcon;
		BOOL big;

		if (iconIndex & PLIF_ICOLIBHANDLE)
			return (HICON)GetIconHandle(IDI_GG);

		big = (iconIndex & PLIF_SMALL) == 0;
		hIcon = LoadIconEx("main", big);

		if (iconIndex & PLIF_ICOLIB)
			return hIcon;

		hIcon = CopyIcon(hIcon);
		ReleaseIconEx("main", big);
		return hIcon;
	}

	return (HICON)NULL;
}

//////////////////////////////////////////////////////////
// gets protocol status
GGINLINE char *gg_getstatusmsg(GGPROTO *gg, int status)
{
	switch(status)
	{
		case ID_STATUS_ONLINE:
			return gg->modemsg.online;
			break;
		case ID_STATUS_DND:
			return gg->modemsg.dnd;
			break;
		case ID_STATUS_FREECHAT:
			return gg->modemsg.freechat;
			break;
		case ID_STATUS_INVISIBLE:
			return gg->modemsg.invisible;
			break;
		case ID_STATUS_AWAY:
		default:
			return gg->modemsg.away;
	}
}

//////////////////////////////////////////////////////////
// sets specified protocol status
int gg_refreshstatus(GGPROTO *gg, int status)
{
	if(status == ID_STATUS_OFFLINE)
	{
		gg_disconnect(gg);
		return TRUE;
	}

	if(!gg_isonline(gg))
	{
		DWORD exitCode = 0;
		GetExitCodeThread(gg->pth_sess.hThread, &exitCode);
		if (exitCode == STILL_ACTIVE)
			return TRUE;
#ifdef DEBUGMODE
		gg_netlog(gg, "gg_refreshstatus(): Going to connect...");
#endif
		gg_threadwait(gg, &gg->pth_sess);
		gg->pth_sess.hThread = gg_forkthreadex(gg, gg_mainthread, NULL, &gg->pth_sess.dwThreadId);
	}
	else
	{
		char *szMsg = NULL;
		// Select proper msg
		EnterCriticalSection(&gg->modemsg_mutex);
		szMsg = mir_strdup(gg_getstatusmsg(gg, status));
		LeaveCriticalSection(&gg->modemsg_mutex);
		if(szMsg)
		{
			gg_netlog(gg, "gg_refreshstatus(): Setting status and away message.");
			EnterCriticalSection(&gg->sess_mutex);
			gg_change_status_descr(gg->sess, status_m2gg(gg, status, szMsg != NULL), szMsg);
			LeaveCriticalSection(&gg->sess_mutex);
		}
		else
		{
			gg_netlog(gg, "gg_refreshstatus(): Setting just status.");
			EnterCriticalSection(&gg->sess_mutex);
			gg_change_status(gg->sess, status_m2gg(gg, status, 0));
			LeaveCriticalSection(&gg->sess_mutex);
		}
		// Change status of the contact with our own UIN (if got yourself added to the contact list)
		gg_changecontactstatus(gg, DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0), status_m2gg(gg, status, szMsg != NULL), szMsg, 0, 0, 0, 0);
		gg_broadcastnewstatus(gg, status);
		mir_free(szMsg);
	}

	return TRUE;
}

//////////////////////////////////////////////////////////
// normalize gg status
int gg_normalizestatus(int status)
{
	switch(status)
	{
		case ID_STATUS_ONLINE:
			return ID_STATUS_ONLINE;
		case ID_STATUS_DND:
			return ID_STATUS_DND;
		case ID_STATUS_FREECHAT:
			return ID_STATUS_FREECHAT;
		case ID_STATUS_OFFLINE:
			return ID_STATUS_OFFLINE;
		case ID_STATUS_INVISIBLE:
			return ID_STATUS_INVISIBLE;
		default:
			return ID_STATUS_AWAY;
	}
}

//////////////////////////////////////////////////////////
// sets protocol status
int gg_setstatus(PROTO_INTERFACE *proto, int iNewStatus)
{
	GGPROTO *gg = (GGPROTO *)proto;
	int nNewStatus = gg_normalizestatus(iNewStatus);

	EnterCriticalSection(&gg->modemsg_mutex);
	gg->proto.m_iDesiredStatus = nNewStatus;
	LeaveCriticalSection(&gg->modemsg_mutex);

	// If waiting for connection retry attempt then signal to stop that
	if (gg->hConnStopEvent) SetEvent(gg->hConnStopEvent);

	if (gg->proto.m_iStatus == nNewStatus) return 0;
	gg_netlog(gg, "gg_setstatus(): PS_SETSTATUS(%d) normalized to %d.", iNewStatus, nNewStatus);
	gg_refreshstatus(gg, nNewStatus);

	return 0;
}

//////////////////////////////////////////////////////////
// when messsage received
int gg_recvmessage(PROTO_INTERFACE *proto, HANDLE hContact, PROTORECVEVENT *pre)
{
	CCSDATA ccs = { hContact, PSR_MESSAGE, 0, ( LPARAM )pre };
	return CallService(MS_PROTO_RECVMSG, 0, ( LPARAM )&ccs);
}

//////////////////////////////////////////////////////////
// when messsage sent
typedef struct
{
	HANDLE hContact;
	int seq;
} GG_SEQ_ACK;
void __cdecl gg_sendackthread(GGPROTO *gg, void *ack)
{
	SleepEx(100, FALSE);
	ProtoBroadcastAck(GG_PROTO, ((GG_SEQ_ACK *)ack)->hContact,
		ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE) ((GG_SEQ_ACK *)ack)->seq, 0);
	mir_free(ack);
}
int gg_sendmessage(PROTO_INTERFACE *proto, HANDLE hContact, int flags, const char *msg)
{
	GGPROTO *gg = (GGPROTO *)proto;
	uin_t uin;

	if (msg && gg_isonline(gg) && (uin = (uin_t)DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0)))
	{
		int seq;
		EnterCriticalSection(&gg->sess_mutex);
		seq = gg_send_message(gg->sess, GG_CLASS_CHAT, uin, msg);
		LeaveCriticalSection(&gg->sess_mutex);
		if (!DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_MSGACK, GG_KEYDEF_MSGACK))
		{
			// Auto-ack message without waiting for server ack
			GG_SEQ_ACK *ack = mir_alloc(sizeof(GG_SEQ_ACK));
			if (ack)
			{
				ack->seq = seq;
				ack->hContact = hContact;
				gg_forkthread(gg, gg_sendackthread, ack);
			}
		}
		return seq;
	}
	return 0;
}

//////////////////////////////////////////////////////////
// when basic search
void __cdecl gg_searchthread(GGPROTO *gg, void *empty)
{
	SleepEx(100, FALSE);
	gg_netlog(gg, "gg_searchthread(): Failed search.");
	ProtoBroadcastAck(GG_PROTO, NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)1, 0);
}
HANDLE gg_basicsearch(PROTO_INTERFACE *proto, const PROTOCHAR *id)
{
	GGPROTO *gg = (GGPROTO *)proto;
	gg_pubdir50_t req;
	char *ida;

	if(!gg_isonline(gg))
		return (HANDLE)0;

	if (!(req = gg_pubdir50_new(GG_PUBDIR50_SEARCH)))
	{
		gg_forkthread(gg, gg_searchthread, NULL);
		return (HANDLE)1;
	}

	ida = gg_t2a(id); 

	// Add uin and search it
	gg_pubdir50_add(req, GG_PUBDIR50_UIN, ida);
	gg_pubdir50_seq_set(req, GG_SEQ_SEARCH);

	mir_free(ida);

	EnterCriticalSection(&gg->sess_mutex);
	if(!gg_pubdir50(gg->sess, req))
	{
		LeaveCriticalSection(&gg->sess_mutex);
		gg_forkthread(gg, gg_searchthread, NULL);
		return (HANDLE)1;
	}
	LeaveCriticalSection(&gg->sess_mutex);
	gg_netlog(gg, "gg_basicsearch(): Seq %d.", req->seq);
	gg_pubdir50_free(req);

	return (HANDLE)1;
}
static HANDLE gg_searchbydetails(PROTO_INTERFACE *proto, const PROTOCHAR *nick, const PROTOCHAR *firstName, const PROTOCHAR *lastName)
{
	GGPROTO *gg = (GGPROTO *)proto;
	gg_pubdir50_t req;
	unsigned long crc;
	char data[512] = "\0";

	// Check if connected and if there's a search data
	if(!gg_isonline(gg))
		return 0;

	if(!nick && !firstName && !lastName)
		return 0;

	if (!(req = gg_pubdir50_new(GG_PUBDIR50_SEARCH)))
	{
		gg_forkthread(gg, gg_searchthread, NULL);
		return (HANDLE)1;
	}

	// Add uin and search it
	if(nick)
	{
		char *nickA = gg_t2a(nick); 
		gg_pubdir50_add(req, GG_PUBDIR50_NICKNAME, nickA);
		strncat(data, nickA, sizeof(data) - strlen(data));
		mir_free(nickA);
	}
	strncat(data, ".", sizeof(data) - strlen(data));

	if(firstName)
	{
		char *firstNameA = gg_t2a(firstName); 
		gg_pubdir50_add(req, GG_PUBDIR50_FIRSTNAME, firstNameA);
		strncat(data, firstNameA, sizeof(data) - strlen(data));
		mir_free(firstNameA);
	}
	strncat(data, ".", sizeof(data) - strlen(data));

	if(lastName)
	{
		char *lastNameA = gg_t2a(lastName); 
		gg_pubdir50_add(req, GG_PUBDIR50_LASTNAME, lastNameA);
		strncat(data, lastNameA, sizeof(data) - strlen(data));
		mir_free(lastNameA);
	}
	strncat(data, ".", sizeof(data) - strlen(data));

	// Count crc & check if the data was equal if yes do same search with shift
	crc = crc_get(data);

	if(crc == gg->last_crc && gg->next_uin)
		gg_pubdir50_add(req, GG_PUBDIR50_START, ditoa(gg->next_uin));
	else
		gg->last_crc = crc;

	gg_pubdir50_seq_set(req, GG_SEQ_SEARCH);

	EnterCriticalSection(&gg->sess_mutex);
	if(!gg_pubdir50(gg->sess, req))
	{
		LeaveCriticalSection(&gg->sess_mutex);
		gg_forkthread(gg, gg_searchthread, NULL);
		return (HANDLE)1;
	}
	LeaveCriticalSection(&gg->sess_mutex);
	gg_netlog(gg, "gg_searchbyname(): Seq %d.", req->seq);
	gg_pubdir50_free(req);

	return (HANDLE)1;
}

//////////////////////////////////////////////////////////
// when contact is added to list
HANDLE gg_addtolist(PROTO_INTERFACE *proto, int flags, PROTOSEARCHRESULT *psr)
{
	GGPROTO *gg = (GGPROTO *)proto;
	GGSEARCHRESULT *sr = (GGSEARCHRESULT *)psr;
	char *szNick = psr->flags & PSR_UNICODE ? mir_u2a((wchar_t *)sr->hdr.nick) : mir_strdup(sr->hdr.nick);
	uin_t uin;
	HANDLE hContact;

	if (psr->cbSize == sizeof(GGSEARCHRESULT))
		uin = sr->uin;
	else
		uin = psr->flags & PSR_UNICODE ? _wtoi((wchar_t*)psr->id) : atoi(psr->id);

	hContact = gg_getcontact(gg, uin, 1, flags & PALF_TEMPORARY ? 0 : 1, szNick);
	mir_free(szNick);

	return hContact;
}

//////////////////////////////////////////////////////////
// user info request
void __cdecl gg_cmdgetinfothread(GGPROTO *gg, void *hContact)
{
	SleepEx(100, FALSE);
	gg_netlog(gg, "gg_cmdgetinfothread(): Failed info retreival.");
	ProtoBroadcastAck(GG_PROTO, hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE) 1, 0);
}
int gg_getinfo(PROTO_INTERFACE *proto, HANDLE hContact, int infoType)
{
	GGPROTO *gg = (GGPROTO *)proto;
	gg_pubdir50_t req;

	// Custom contact info
	if(hContact)
	{
		if (!(req = gg_pubdir50_new(GG_PUBDIR50_SEARCH)))
		{
			gg_forkthread(gg, gg_cmdgetinfothread, hContact);
			return 1;
		}

		// Add uin and search it
		gg_pubdir50_add(req, GG_PUBDIR50_UIN, ditoa((uin_t)DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0)));
		gg_pubdir50_seq_set(req, GG_SEQ_INFO);

		gg_netlog(gg, "gg_getinfo(): Requesting user info.", req->seq);
		if(gg_isonline(gg))
		{
			EnterCriticalSection(&gg->sess_mutex);
			if(!gg_pubdir50(gg->sess, req))
			{
				LeaveCriticalSection(&gg->sess_mutex);
				gg_forkthread(gg, gg_cmdgetinfothread, hContact);
				return 1;
			}
			LeaveCriticalSection(&gg->sess_mutex);
		}
	}
	// Own contact info
	else
	{
		if (!(req = gg_pubdir50_new(GG_PUBDIR50_READ)))
		{
			gg_forkthread(gg, gg_cmdgetinfothread, hContact);
			return 1;
		}

		// Add seq
		gg_pubdir50_seq_set(req, GG_SEQ_CHINFO);

		gg_netlog(gg, "gg_getinfo(): Requesting owner info.", req->seq);
		if(gg_isonline(gg))
		{
			EnterCriticalSection(&gg->sess_mutex);
			if(!gg_pubdir50(gg->sess, req))
			{
				LeaveCriticalSection(&gg->sess_mutex);
				gg_forkthread(gg, gg_cmdgetinfothread, hContact);
				return 1;
			}
			LeaveCriticalSection(&gg->sess_mutex);
		}
	}
	gg_netlog(gg, "gg_getinfo(): Seq %d.", req->seq);
	gg_pubdir50_free(req);

	return 1;
}

//////////////////////////////////////////////////////////
// when away message is requested
void __cdecl gg_getawaymsgthread(GGPROTO *gg, void *hContact)
{
	DBVARIANT dbv;

	SleepEx(100, FALSE);
	if (!DBGetContactSettingString(hContact, "CList", GG_KEY_STATUSDESCR, &dbv))
	{
		ProtoBroadcastAck(GG_PROTO, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE) 1, (LPARAM) dbv.pszVal);
		gg_netlog(gg, "gg_getawaymsg(): Reading away msg \"%s\".", dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	else
		ProtoBroadcastAck(GG_PROTO, hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE) 1, (LPARAM) NULL);
}
HANDLE gg_getawaymsg(PROTO_INTERFACE *proto, HANDLE hContact)
{
	gg_forkthread((GGPROTO *)proto, gg_getawaymsgthread, hContact);

	return (HANDLE)1;
}

//////////////////////////////////////////////////////////
// when away message is being set
int gg_setawaymsg(PROTO_INTERFACE *proto, int iStatus, const PROTOCHAR *msgt)
{
	GGPROTO *gg = (GGPROTO *)proto;
	int status = gg_normalizestatus(iStatus);
	char **szMsg;
	char *msg = gg_t2a(msgt);

	gg_netlog(gg, "gg_setawaymsg(): PS_SETAWAYMSG(%d, \"%s\").", iStatus, msg);

	EnterCriticalSection(&gg->modemsg_mutex);
	// Select proper msg
	switch(status)
	{
		case ID_STATUS_ONLINE:
			szMsg = &gg->modemsg.online;
			break;
		case ID_STATUS_AWAY:
			szMsg = &gg->modemsg.away;
			break;
		case ID_STATUS_DND:
			szMsg = &gg->modemsg.dnd;
			break;
		case ID_STATUS_FREECHAT:
			szMsg = &gg->modemsg.freechat;
			break;
		case ID_STATUS_INVISIBLE:
			szMsg = &gg->modemsg.invisible;
			break;
		default:
			LeaveCriticalSection(&gg->modemsg_mutex);
			mir_free(msg);
			return 1;
	}

	// Check if we change status here somehow
	if (*szMsg && msg && !strcmp(*szMsg, msg)
		|| !*szMsg && (!msg || !*msg))	
	{
		if (status == gg->proto.m_iDesiredStatus && gg->proto.m_iDesiredStatus == gg->proto.m_iStatus)
		{
			gg_netlog(gg, "gg_setawaymsg(): Message hasn't been changed, return.");
			LeaveCriticalSection(&gg->modemsg_mutex);
			mir_free(msg);
			return 0;
		}
	}
	else
	{
		if (*szMsg)
			mir_free(*szMsg);
		*szMsg = msg && *msg ? mir_strdup(msg) : NULL;
#ifdef DEBUGMODE
		gg_netlog(gg, "gg_setawaymsg(): Message changed.");
#endif
	}
	LeaveCriticalSection(&gg->modemsg_mutex);

	// Change the status if it was desired by PS_SETSTATUS
	if (status == gg->proto.m_iDesiredStatus)
		gg_refreshstatus(gg, status);

	mir_free(msg);
	return 0;
}

//////////////////////////////////////////////////////////
// visible lists
int gg_setapparentmode(PROTO_INTERFACE *proto, HANDLE hContact, int mode)
{
	GGPROTO *gg = (GGPROTO *)proto;
	DBWriteContactSettingWord(hContact, GG_PROTO, GG_KEY_APPARENT, (WORD)mode);
	gg_notifyuser(gg, hContact, 1);
	return 0;
}

//////////////////////////////////////////////////////////
// create adv search dialog proc
INT_PTR CALLBACK gg_advancedsearchdlgproc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			TranslateDialogDefault(hwndDlg);
			SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)_T(""));				// 0
			SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)Translate("Female"));	// 1
			SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)Translate("Male"));	// 2
			return TRUE;
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDOK:
						SendMessage(GetParent(hwndDlg), WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED), (LPARAM)GetDlgItem(GetParent(hwndDlg),IDOK));
						break;
				case IDCANCEL:
//						  CheckDlgButton(GetParent(hwndDlg),IDC_ADVANCED,BST_UNCHECKED);
//						  SendMessage(GetParent(hwndDlg),WM_COMMAND,MAKEWPARAM(IDC_ADVANCED,BN_CLICKED),(LPARAM)GetDlgItem(GetParent(hwndDlg),IDC_ADVANCED));
						break;
			}
			break;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////
// create adv search dialog
HWND gg_createadvsearchui(PROTO_INTERFACE *proto, HWND owner)
{
	return CreateDialogParam(hInstance,
		MAKEINTRESOURCE(IDD_GGADVANCEDSEARCH), owner, gg_advancedsearchdlgproc, (LPARAM)(GGPROTO *)proto);
}

//////////////////////////////////////////////////////////
// search by advanced
HWND gg_searchbyadvanced(PROTO_INTERFACE *proto, HWND hwndDlg)
{
	GGPROTO *gg = (GGPROTO *)proto;
	gg_pubdir50_t req;
	char text[64], data[512] = "\0";
	unsigned long crc;

	// Check if connected
	if(!gg_isonline(gg)) return (HWND)0;

	if (!(req = gg_pubdir50_new(GG_PUBDIR50_SEARCH)))
	{
		gg_forkthread(gg, gg_searchthread, NULL);
		return (HWND)1;
	}

	// Fetch search data
	GetDlgItemText(hwndDlg, IDC_FIRSTNAME, text, sizeof(text));
	if(strlen(text))
	{
		gg_pubdir50_add(req, GG_PUBDIR50_FIRSTNAME, text);
		strncat(data, text, sizeof(data) - strlen(data));
	}
	/* 1 */ strncat(data, ".", sizeof(data) - strlen(data));

	GetDlgItemText(hwndDlg, IDC_LASTNAME, text, sizeof(text));
	if(strlen(text))
	{
		gg_pubdir50_add(req, GG_PUBDIR50_LASTNAME, text);
		strncat(data, text, sizeof(data) - strlen(data));
	}
	/* 2 */ strncat(data, ".", sizeof(data) - strlen(data));

	GetDlgItemText(hwndDlg, IDC_NICKNAME, text, sizeof(text));
	if(strlen(text))
	{
		gg_pubdir50_add(req, GG_PUBDIR50_NICKNAME, text);
		strncat(data, text, sizeof(data) - strlen(data));
	}
	/* 3 */ strncat(data, ".", sizeof(data) - strlen(data));

	GetDlgItemText(hwndDlg, IDC_CITY, text, sizeof(text));
	if(strlen(text))
	{
		gg_pubdir50_add(req, GG_PUBDIR50_CITY, text);
		strncat(data, text, sizeof(data) - strlen(data));
	}
	/* 4 */ strncat(data, ".", sizeof(data) - strlen(data));

	GetDlgItemText(hwndDlg, IDC_AGEFROM, text, sizeof(text));
	if(strlen(text))
	{
		int yearTo = atoi(text);
		int yearFrom;
		time_t t = time(NULL);
		struct tm *lt = localtime(&t);
		int ay = lt->tm_year + 1900;
		char age[16];

		GetDlgItemText(hwndDlg, IDC_AGETO, age, sizeof(age));
		yearFrom = atoi(age);

		// Count & fix ranges
		if(!yearTo)
			yearTo = ay;
		else
			yearTo = ay - yearTo;
		if(!yearFrom)
			yearFrom = 0;
		else
			yearFrom = ay - yearFrom;
		mir_snprintf(text, sizeof(text), "%d %d", yearFrom, yearTo);

		gg_pubdir50_add(req, GG_PUBDIR50_BIRTHYEAR, text);
		strncat(data, text, sizeof(data) - strlen(data));
	}
	/* 5 */ strncat(data, ".", sizeof(data) - strlen(data));

	switch(SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_GETCURSEL, 0, 0))
	{
		case 1:
			gg_pubdir50_add(req, GG_PUBDIR50_GENDER, GG_PUBDIR50_GENDER_FEMALE);
			strncat(data, GG_PUBDIR50_GENDER_MALE, sizeof(data) - strlen(data));
			break;
		case 2:
			gg_pubdir50_add(req, GG_PUBDIR50_GENDER, GG_PUBDIR50_GENDER_MALE);
			strncat(data, GG_PUBDIR50_GENDER_FEMALE, sizeof(data) - strlen(data));
			break;
	}
	/* 6 */ strncat(data, ".", sizeof(data) - strlen(data));

	if(IsDlgButtonChecked(hwndDlg, IDC_ONLYCONNECTED))
	{
		gg_pubdir50_add(req, GG_PUBDIR50_ACTIVE, GG_PUBDIR50_ACTIVE_TRUE);
		strncat(data, GG_PUBDIR50_ACTIVE_TRUE, sizeof(data) - strlen(data));
	}
	/* 7 */ strncat(data, ".", sizeof(data) - strlen(data));

	// No data entered
	if(strlen(data) <= 7 || (strlen(data) == 8 && IsDlgButtonChecked(hwndDlg, IDC_ONLYCONNECTED))) return (HWND)0;

	// Count crc & check if the data was equal if yes do same search with shift
	crc = crc_get(data);

	if(crc == gg->last_crc && gg->next_uin)
		gg_pubdir50_add(req, GG_PUBDIR50_START, ditoa(gg->next_uin));
	else
		gg->last_crc = crc;

	gg_pubdir50_seq_set(req, GG_SEQ_SEARCH);

	if(gg_isonline(gg))
	{
		EnterCriticalSection(&gg->sess_mutex);
		if(!gg_pubdir50(gg->sess, req))
		{
			LeaveCriticalSection(&gg->sess_mutex);
			gg_forkthread(gg, gg_searchthread, NULL);
			return (HWND)1;
		}
		LeaveCriticalSection(&gg->sess_mutex);
	}
	gg_netlog(gg, "gg_searchbyadvanced(): Seq %d.", req->seq);
	gg_pubdir50_free(req);

	return (HWND)1;
}

//////////////////////////////////////////////////////////
// gets avatar capabilities
INT_PTR gg_getavatarcaps(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
		case AF_MAXSIZE:
			((POINT *)lParam)->x = ((POINT *)lParam)->y = 200;
			return 0;
		case AF_FORMATSUPPORTED:
			return (lParam == PA_FORMAT_JPEG || lParam == PA_FORMAT_GIF || lParam == PA_FORMAT_PNG);
		case AF_ENABLED:
			return DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS);
		case AF_DONTNEEDDELAYS:
			return 1;
		case AF_MAXFILESIZE:
			return 307200;
		case AF_FETCHALWAYS:
			return 1;
	}
	return 0;
}

//////////////////////////////////////////////////////////
// gets avatar information
INT_PTR gg_getavatarinfo(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	PROTO_AVATAR_INFORMATION *pai = (PROTO_AVATAR_INFORMATION *)lParam;
	char *AvatarURL = NULL, *AvatarHash = NULL, *AvatarSavedHash = NULL;
	INT_PTR result = GAIR_NOAVATAR;
	DBVARIANT dbv;
	uin_t uin = (uin_t)DBGetContactSettingDword(pai->hContact, GG_PROTO, GG_KEY_UIN, 0);

	gg_netlog(gg, "gg_getavatarinfo(): Requesting avatar information for %d.", uin);

	pai->filename[0] = 0;
	pai->format = PA_FORMAT_UNKNOWN;

	if (!uin || !DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS))
		return GAIR_NOAVATAR;

	if (!DBGetContactSettingByte(pai->hContact, GG_PROTO, GG_KEY_AVATARREQUESTED, GG_KEYDEF_AVATARREQUESTED)) {
		gg_requestavatar(gg, pai->hContact, 1);
		return (wParam & GAIF_FORCE) != 0 ? GAIR_WAITFOR : GAIR_NOAVATAR;
	}
	DBDeleteContactSetting(pai->hContact, GG_PROTO, GG_KEY_AVATARREQUESTED);

	pai->format = DBGetContactSettingByte(pai->hContact, GG_PROTO, GG_KEY_AVATARTYPE, GG_KEYDEF_AVATARTYPE);

	if (!DBGetContactSettingString(pai->hContact, GG_PROTO, GG_KEY_AVATARURL, &dbv)) {
		AvatarURL = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	if (AvatarURL != NULL && strlen(AvatarURL) > 0) {
		char *AvatarName = strrchr(AvatarURL, '/');
		AvatarName++;
		AvatarHash = gg_avatarhash(AvatarName);
	}

	if (!DBGetContactSettingString(pai->hContact, GG_PROTO, GG_KEY_AVATARHASH, &dbv)) {
		AvatarSavedHash = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	if (AvatarHash != NULL && AvatarSavedHash != NULL) {
		gg_getavatarfilename(gg, pai->hContact, pai->filename, sizeof(pai->filename));
		if (!strcmp(AvatarHash, AvatarSavedHash) && !_access(pai->filename, 0)) {
			result = GAIR_SUCCESS;
		}
		else if ((wParam & GAIF_FORCE) != 0) {
			gg_netlog(gg, "gg_getavatarinfo(): Contact %d changed avatar.", uin);
			remove(pai->filename);
			DBWriteContactSettingString(pai->hContact, GG_PROTO, GG_KEY_AVATARHASH, AvatarHash);
			gg_getavatar(gg, pai->hContact, AvatarURL);
			result = GAIR_WAITFOR;
		}
	}
	else if ((wParam & GAIF_FORCE) != 0) {
		if (AvatarHash == NULL && AvatarSavedHash != NULL) {
			gg_netlog(gg, "gg_getavatarinfo(): Contact %d deleted avatar.", uin);
			gg_getavatarfilename(gg, pai->hContact, pai->filename, sizeof(pai->filename));
			remove(pai->filename);
			DBDeleteContactSetting(pai->hContact, GG_PROTO, GG_KEY_AVATARHASH);
			DBDeleteContactSetting(pai->hContact, GG_PROTO, GG_KEY_AVATARURL);
			DBDeleteContactSetting(pai->hContact, GG_PROTO, GG_KEY_AVATARTYPE);
		}
		else if (AvatarHash != NULL && AvatarSavedHash == NULL) {
			gg_netlog(gg, "gg_getavatarinfo(): Contact %d set avatar.", uin);
			DBWriteContactSettingString(pai->hContact, GG_PROTO, GG_KEY_AVATARHASH, AvatarHash);
			gg_getavatar(gg, pai->hContact, AvatarURL);
			result = GAIR_WAITFOR;
		}
	}

	mir_free(AvatarHash);
	mir_free(AvatarSavedHash);
	mir_free(AvatarURL);

	return result;
}

//////////////////////////////////////////////////////////
// gets avatar
INT_PTR gg_getmyavatar(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	char *szFilename = (char *)wParam;
	int len = (int)lParam;

	gg_netlog(gg, "gg_getmyavatar(): Requesting user avatar.");

	if (szFilename == NULL || len <= 0)
		return -1;

	if (!DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS))
		return -2;

	gg_getavatarfilename(gg, NULL, szFilename, len);
	return _access(szFilename, 0);
}

//////////////////////////////////////////////////////////
// sets avatar
INT_PTR gg_setmyavatar(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	char *szFilename = (char *)lParam;

	if (!DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS))
		return -2;

	if (szFilename == NULL) {
		MessageBox(
			NULL,
			Translate("To remove your Gadu-Gadu avatar, you must use the MojaGeneracja.pl website."),
			GG_PROTONAME, MB_OK | MB_ICONINFORMATION);
		return -1;
	}
	else {
		char szMyFilename[MAX_PATH];
		gg_getavatarfilename(gg, NULL, szMyFilename, sizeof(szMyFilename));
		if (strcmp(szFilename, szMyFilename) && !CopyFileA(szFilename, szMyFilename, FALSE)) {
			gg_netlog(gg, "gg_setmyavatar(): Failed to set user avatar. File %s could not be created/overwritten.", szMyFilename);
			return -1;
		}
		gg_setavatar(gg, szMyFilename);
	}

	return 0;
}

//////////////////////////////////////////////////////////
// gets protocol status message
INT_PTR gg_getmyawaymsg(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR res = 0;
	char *szMsg;

	EnterCriticalSection(&gg->modemsg_mutex);
	szMsg = gg_getstatusmsg(gg, wParam ? gg_normalizestatus(wParam) : gg->proto.m_iStatus);
	if(gg_isonline(gg) && szMsg)
		res = (lParam & SGMA_UNICODE) ? (INT_PTR)mir_a2u(szMsg) : (INT_PTR)mir_strdup(szMsg);
	LeaveCriticalSection(&gg->modemsg_mutex);
	return res;
}

//////////////////////////////////////////////////////////
// gets account manager GUI
extern INT_PTR CALLBACK gg_acc_mgr_guidlgproc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

static INT_PTR gg_get_acc_mgr_gui(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	return (INT_PTR) CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_ACCMGRUI), (HWND)lParam, gg_acc_mgr_guidlgproc, (LPARAM) gg);
}

//////////////////////////////////////////////////////////
// leaves (terminates) conference
INT_PTR gg_leavechat(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if(hContact)
		CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);

	return 0;
}

//////////////////////////////////////////////////////////
// sends a notification that the user is typing a message
int gg_useristyping(PROTO_INTERFACE *proto, HANDLE hContact, int type)
{
	GGPROTO *gg = (GGPROTO *)proto;
	uin_t uin = DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0);

	if (!uin || !gg_isonline(gg)) return 0;

	if (type == PROTOTYPE_SELFTYPING_ON || type == PROTOTYPE_SELFTYPING_OFF) {
		EnterCriticalSection(&gg->sess_mutex);
		gg_typing_notification(gg->sess, uin, (type == PROTOTYPE_SELFTYPING_ON));
		LeaveCriticalSection(&gg->sess_mutex);
	}

	return 0;
}

//////////////////////////////////////////////////////////
// Dummies for function that have to be implemented

HANDLE gg_dummy_addtolistbyevent(PROTO_INTERFACE *proto, int flags, int iContact, HANDLE hDbEvent) { return NULL; }
int    gg_dummy_authorize(PROTO_INTERFACE *proto, HANDLE hContact) { return 0; }
int    gg_dummy_authdeny(PROTO_INTERFACE *proto, HANDLE hContact, const TCHAR *szReason) { return 0; }
int    gg_dummy_authrecv(PROTO_INTERFACE *proto, HANDLE hContact, PROTORECVEVENT *pre) { return 0; }
int    gg_dummy_authrequest(PROTO_INTERFACE *proto, HANDLE hContact, const TCHAR *szMessage) { return 0; }
HANDLE gg_dummy_changeinfo(PROTO_INTERFACE *proto, int iInfoType, void *pInfoData) { return NULL; }
int    gg_dummy_fileresume(PROTO_INTERFACE *proto, HANDLE hTransfer, int *action, const PROTOCHAR** szFilename) { return 0; }
HANDLE gg_dummy_searchbyemail(PROTO_INTERFACE *proto, const PROTOCHAR *email) { return NULL; }
int    gg_dummy_recvcontacts(PROTO_INTERFACE *proto, HANDLE hContact, PROTORECVEVENT *pre) { return 0; }
int    gg_dummy_recvurl(PROTO_INTERFACE *proto, HANDLE hContact, PROTORECVEVENT *pre) { return 0; }
int    gg_dummy_sendcontacts(PROTO_INTERFACE *proto, HANDLE hContact, int flags, int nContacts, HANDLE *hContactsList) { return 0; }
int    gg_dummy_sendurl(PROTO_INTERFACE *proto, HANDLE hContact, int flags, const char *url) { return 0; }
int    gg_dummy_recvawaymsg(PROTO_INTERFACE *proto, HANDLE hContact, int mode, PROTORECVEVENT *evt) { return 0; }
int    gg_dummy_sendawaymsg(PROTO_INTERFACE *proto, HANDLE hContact, HANDLE hProcess, const char *msg) { return 0; }

//////////////////////////////////////////////////////////
// Register services
void gg_registerservices(GGPROTO *gg)
{
	gg->proto.vtbl->AddToList              = gg_addtolist;
	gg->proto.vtbl->AddToListByEvent       = gg_dummy_addtolistbyevent;

	gg->proto.vtbl->Authorize              = gg_dummy_authorize;
	gg->proto.vtbl->AuthDeny               = gg_dummy_authdeny;
	gg->proto.vtbl->AuthRecv               = gg_dummy_authrecv;
	gg->proto.vtbl->AuthRequest            = gg_dummy_authrequest;

	gg->proto.vtbl->ChangeInfo             = gg_dummy_changeinfo;

	gg->proto.vtbl->FileAllow              = gg_fileallow;
	gg->proto.vtbl->FileCancel             = gg_filecancel;
	gg->proto.vtbl->FileDeny               = gg_filedeny;
	gg->proto.vtbl->FileResume             = gg_dummy_fileresume;

	gg->proto.vtbl->GetCaps                = gg_getcaps;
	gg->proto.vtbl->GetIcon                = gg_geticon;
	gg->proto.vtbl->GetInfo                = gg_getinfo;

	gg->proto.vtbl->SearchBasic            = gg_basicsearch;
	gg->proto.vtbl->SearchByEmail          = gg_dummy_searchbyemail;
	gg->proto.vtbl->SearchByName           = gg_searchbydetails;
	gg->proto.vtbl->SearchAdvanced         = gg_searchbyadvanced;
	gg->proto.vtbl->CreateExtendedSearchUI = gg_createadvsearchui;

	gg->proto.vtbl->RecvContacts           = gg_dummy_recvcontacts;
	gg->proto.vtbl->RecvFile               = gg_recvfile;
	gg->proto.vtbl->RecvMsg                = gg_recvmessage;
	gg->proto.vtbl->RecvUrl                = gg_dummy_recvurl;

	gg->proto.vtbl->SendContacts           = gg_dummy_sendcontacts;
	gg->proto.vtbl->SendFile               = gg_sendfile;
	gg->proto.vtbl->SendMsg                = gg_sendmessage;
	gg->proto.vtbl->SendUrl                = gg_dummy_sendurl;

	gg->proto.vtbl->SetApparentMode        = gg_setapparentmode;
	gg->proto.vtbl->SetStatus              = gg_setstatus;

	gg->proto.vtbl->GetAwayMsg             = gg_getawaymsg;
	gg->proto.vtbl->RecvAwayMsg            = gg_dummy_recvawaymsg;
	gg->proto.vtbl->SendAwayMsg            = gg_dummy_sendawaymsg;
	gg->proto.vtbl->SetAwayMsg             = gg_setawaymsg;

	gg->proto.vtbl->UserIsTyping           = gg_useristyping;

	gg->proto.vtbl->OnEvent                = gg_event;

	CreateProtoService(PS_GETAVATARCAPS, gg_getavatarcaps, gg);
	CreateProtoService(PS_GETAVATARINFO, gg_getavatarinfo, gg);
	CreateProtoService(PS_GETMYAVATAR, gg_getmyavatar, gg);
	CreateProtoService(PS_SETMYAVATAR, gg_setmyavatar, gg);

	CreateProtoService(PS_GETMYAWAYMSG, gg_getmyawaymsg, gg);
	CreateProtoService(PS_CREATEACCMGRUI, gg_get_acc_mgr_gui, gg);

	CreateProtoService(PS_LEAVECHAT, gg_leavechat, gg);
}
