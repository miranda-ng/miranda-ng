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

GGPROTO::GGPROTO(const char* pszProtoName, const TCHAR* tszUserName) :
	PROTO<GGPROTO>(pszProtoName, tszUserName),
	avatar_requests(1, NumericKeySortT),
	avatar_transfers(1, NumericKeySortT)
{
#ifdef DEBUGMODE
	extendedLogging = 0;
#endif

	// Init mutexes
	InitializeCriticalSection(&sess_mutex);
	InitializeCriticalSection(&ft_mutex);
	InitializeCriticalSection(&img_mutex);
	InitializeCriticalSection(&modemsg_mutex);
	InitializeCriticalSection(&avatar_mutex);
	InitializeCriticalSection(&sessions_mutex);

	// Register m_hNetlibUser user
	TCHAR name[128];
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s connection"), m_tszUserName);

	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_TCHAR | NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS;
	nlu.szSettingsModule = m_szModuleName;
	nlu.ptszDescriptiveName = name;

	m_hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	// Register services
	CreateProtoService(PS_GETAVATARCAPS, &GGPROTO::getavatarcaps);
	CreateProtoService(PS_GETAVATARINFOT, &GGPROTO::getavatarinfo);
	CreateProtoService(PS_GETMYAVATART, &GGPROTO::getmyavatar);
	CreateProtoService(PS_SETMYAVATART, &GGPROTO::setmyavatar);

	CreateProtoService(PS_GETMYAWAYMSG, &GGPROTO::getmyawaymsg);
	CreateProtoService(PS_SETAWAYMSGT, (MyServiceFunc)&GGPROTO::SetAwayMsg);
	CreateProtoService(PS_CREATEACCMGRUI, &GGPROTO::get_acc_mgr_gui);

	CreateProtoService(PS_LEAVECHAT, &GGPROTO::leavechat);

	// Offline contacts and clear logon time
	setalloffline();
	setDword(GG_KEY_LOGONTIME, 0);

	db_set_resident(m_szModuleName, GG_KEY_AVATARREQUESTED);

	TCHAR szPath[MAX_PATH];
	mir_sntprintf(szPath, SIZEOF(szPath), _T("%s\\%s\\ImageCache"), (TCHAR*)VARST( _T("%miranda_userdata%")), m_tszUserName);
	hImagesFolder = FoldersRegisterCustomPathT(LPGEN("Images"), m_szModuleName, szPath, m_tszUserName);

	DWORD dwVersion;
	if ((dwVersion = getDword(GG_PLUGINVERSION, 0)) < pluginInfo.version)
		cleanuplastplugin(dwVersion);

	links_instance_init();
	initavatarrequestthread();
}

GGPROTO::~GGPROTO()
{
#ifdef DEBUGMODE
	debugLogA("~GGPROTO(): destroying protocol interface");
#endif

	// Destroy modules
	block_uninit();
	img_destroy();
	keepalive_destroy();
	gc_destroy();

	Popup_UnregisterClass(hPopupError);
	Popup_UnregisterClass(hPopupNotify);

	if (hMenuRoot)
		CallService(MO_REMOVEMENUITEM, (WPARAM)hMenuRoot, 0);

	// Close handles
	Netlib_CloseHandle(m_hNetlibUser);

	// Destroy mutexes
	DeleteCriticalSection(&sess_mutex);
	DeleteCriticalSection(&ft_mutex);
	DeleteCriticalSection(&img_mutex);
	DeleteCriticalSection(&modemsg_mutex);
	DeleteCriticalSection(&avatar_mutex);
	DeleteCriticalSection(&sessions_mutex);
#ifdef DEBUGMODE
	debugLogA("~GGPROTO(): DeleteCriticalSections. OK");
#endif

	// Free status messages
	if (modemsg.online)    mir_free(modemsg.online);
	if (modemsg.away)      mir_free(modemsg.away);
	if (modemsg.dnd)       mir_free(modemsg.dnd);
	if (modemsg.freechat)  mir_free(modemsg.freechat);
	if (modemsg.invisible) mir_free(modemsg.invisible);
	if (modemsg.offline)   mir_free(modemsg.offline);
}

//////////////////////////////////////////////////////////
// when contact is added to list

MCONTACT GGPROTO::AddToList(int flags, PROTOSEARCHRESULT *psr)
{
#ifdef DEBUGMODE
	debugLogA("AddToList(): id=%s");
#endif
	GGSEARCHRESULT *sr = (GGSEARCHRESULT *)psr;
	uin_t uin;

	if (psr->cbSize == sizeof(GGSEARCHRESULT))
		uin = sr->uin;
	else
		uin = _ttoi(psr->id);

	return getcontact(uin, 1, flags & PALF_TEMPORARY ? 0 : 1, sr->nick);
}

//////////////////////////////////////////////////////////
// checks proto capabilities

DWORD_PTR GGPROTO::GetCaps(int type, MCONTACT hContact)
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
// user info request

void __cdecl GGPROTO::cmdgetinfothread(void *hContact)
{
	debugLogA("cmdgetinfothread(): started. Failed info retreival.");
	gg_sleep(100, FALSE, "cmdgetinfothread", 103, 1);
	ProtoBroadcastAck((MCONTACT)hContact, ACKTYPE_GETINFO, ACKRESULT_FAILED, (HANDLE)1, 0);
	debugLogA("cmdgetinfothread(): end.");
}

int GGPROTO::GetInfo(MCONTACT hContact, int infoType)
{
	gg_pubdir50_t req;

	// Custom contact info
	if (hContact)
	{
		if (!(req = gg_pubdir50_new(GG_PUBDIR50_SEARCH)))
		{
#ifdef DEBUGMODE
			debugLogA("GetInfo(): ForkThread 6 GGPROTO::cmdgetinfothread");
#endif
			ForkThread(&GGPROTO::cmdgetinfothread, (void*)hContact);
			return 1;
		}

		// Add uin and search it
		gg_pubdir50_add(req, GG_PUBDIR50_UIN, ditoa((uin_t)getDword(hContact, GG_KEY_UIN, 0)));
		gg_pubdir50_seq_set(req, GG_SEQ_INFO);

		debugLogA("GetInfo(): Requesting user info.", req->seq);
		if (isonline())
		{
			gg_EnterCriticalSection(&sess_mutex, "GetInfo", 48, "sess_mutex", 1);
			if (!gg_pubdir50(sess, req))
			{
				gg_LeaveCriticalSection(&sess_mutex, "GetInfo", 48, 1, "sess_mutex", 1);
#ifdef DEBUGMODE
				debugLogA("GetInfo(): ForkThread 7 GGPROTO::cmdgetinfothread");
#endif
				ForkThread(&GGPROTO::cmdgetinfothread, (void*)hContact);
				return 1;
			}
			gg_LeaveCriticalSection(&sess_mutex, "GetInfo", 48, 2, "sess_mutex", 1);
		}
	}
	// Own contact info
	else
	{
		if (!(req = gg_pubdir50_new(GG_PUBDIR50_READ)))
		{
#ifdef DEBUGMODE
			debugLogA("GetInfo(): ForkThread 8 GGPROTO::cmdgetinfothread");
#endif
			ForkThread(&GGPROTO::cmdgetinfothread, (void*)hContact);
			return 1;
		}

		// Add seq
		gg_pubdir50_seq_set(req, GG_SEQ_CHINFO);

		debugLogA("GetInfo(): Requesting owner info.", req->seq);
		if (isonline())
		{
			gg_EnterCriticalSection(&sess_mutex, "GetInfo", 49, "sess_mutex", 1);
			if (!gg_pubdir50(sess, req))
			{
				gg_LeaveCriticalSection(&sess_mutex, "GetInfo", 49, 1, "sess_mutex", 1);
#ifdef DEBUGMODE
				debugLogA("GetInfo(): ForkThread 9 GGPROTO::cmdgetinfothread");
#endif
				ForkThread(&GGPROTO::cmdgetinfothread, (void*)hContact);
				return 1;
			}
			gg_LeaveCriticalSection(&sess_mutex, "GetInfo", 49, 2, "sess_mutex", 1);
		}
	}
	debugLogA("GetInfo(): Seq %d.", req->seq);
	gg_pubdir50_free(req);

	return 1;
}

//////////////////////////////////////////////////////////
// when basic search

void __cdecl GGPROTO::searchthread(void *)
{
	debugLogA("searchthread(): started. Failed search.");
	gg_sleep(100, FALSE, "searchthread", 104, 1);
	ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_FAILED, (HANDLE)1, 0);
#ifdef DEBUGMODE
	debugLogA("searchthread(): end.");
#endif
}

HANDLE GGPROTO::SearchBasic(const PROTOCHAR *id)
{
	if (!isonline())
		return (HANDLE)0;

	gg_pubdir50_t req;
	if (!(req = gg_pubdir50_new(GG_PUBDIR50_SEARCH))) {
#ifdef DEBUGMODE
		debugLogA("SearchBasic(): ForkThread 10 GGPROTO::searchthread");
#endif
		ForkThread(&GGPROTO::searchthread, NULL);
		return (HANDLE)1;
	}

	char *id_utf8 = mir_utf8encodeT(id);

	// Add uin and search it
	gg_pubdir50_add(req, GG_PUBDIR50_UIN, id_utf8);
	gg_pubdir50_seq_set(req, GG_SEQ_SEARCH);

	mir_free(id_utf8);

	gg_EnterCriticalSection(&sess_mutex, "SearchBasic", 50, "sess_mutex", 1);
	if (!gg_pubdir50(sess, req))
	{
		gg_LeaveCriticalSection(&sess_mutex, "SearchBasic", 50, 1, "sess_mutex", 1);
#ifdef DEBUGMODE
		debugLogA("SearchBasic(): ForkThread 11 GGPROTO::searchthread");
#endif
		ForkThread(&GGPROTO::searchthread, NULL);
		return (HANDLE)1;
	}
	gg_LeaveCriticalSection(&sess_mutex, "SearchBasic", 50, 2, "sess_mutex", 1);
	debugLogA("SearchBasic(): Seq %d.", req->seq);
	gg_pubdir50_free(req);

	return (HANDLE)1;
}

//////////////////////////////////////////////////////////
// search by details

HANDLE GGPROTO::SearchByName(const PROTOCHAR *nick, const PROTOCHAR *firstName, const PROTOCHAR *lastName)
{
	gg_pubdir50_t req;
	unsigned long crc;
	char data[512] = "\0";

	// Check if connected and if there's a search data
	if (!isonline())
		return 0;

	if (!nick && !firstName && !lastName)
		return 0;

	if (!(req = gg_pubdir50_new(GG_PUBDIR50_SEARCH)))
	{
#ifdef DEBUGMODE
		debugLogA("SearchByName(): ForkThread 12 GGPROTO::searchthread");
#endif
		ForkThread(&GGPROTO::searchthread, NULL);
		return (HANDLE)1;
	}

	// Add nick,firstName,lastName and search it
	if (nick)
	{
		char *nick_utf8 = mir_utf8encodeT(nick);
		gg_pubdir50_add(req, GG_PUBDIR50_NICKNAME, nick_utf8);
		strncat(data, nick_utf8, sizeof(data) - mir_strlen(data));
		mir_free(nick_utf8);
	}
	strncat(data, ".", sizeof(data) - mir_strlen(data));

	if (firstName)
	{
		char *firstName_utf8 = mir_utf8encodeT(firstName);
		gg_pubdir50_add(req, GG_PUBDIR50_FIRSTNAME, firstName_utf8);
		strncat(data, firstName_utf8, sizeof(data) - mir_strlen(data));
		mir_free(firstName_utf8);
	}
	strncat(data, ".", sizeof(data) - mir_strlen(data));

	if (lastName)
	{
		char *lastName_utf8 = mir_utf8encodeT(lastName);
		gg_pubdir50_add(req, GG_PUBDIR50_LASTNAME, lastName_utf8);
		strncat(data, lastName_utf8, sizeof(data) - mir_strlen(data));
		mir_free(lastName_utf8);
	}
	strncat(data, ".", sizeof(data) - mir_strlen(data));

	// Count crc & check if the data was equal if yes do same search with shift
	crc = crc_get(data);

	if (crc == last_crc && next_uin)
		gg_pubdir50_add(req, GG_PUBDIR50_START, ditoa(next_uin));
	else
		last_crc = crc;

	gg_pubdir50_seq_set(req, GG_SEQ_SEARCH);
	gg_EnterCriticalSection(&sess_mutex, "SearchByName", 51, "sess_mutex", 1);
	if (!gg_pubdir50(sess, req))
	{
		gg_LeaveCriticalSection(&sess_mutex, "SearchByName", 51, 1, "sess_mutex", 1);
#ifdef DEBUGMODE
		debugLogA("SearchByName(): ForkThread 13 GGPROTO::searchthread");
#endif
		ForkThread(&GGPROTO::searchthread, NULL);
		return (HANDLE)1;
	}
	gg_LeaveCriticalSection(&sess_mutex, "SearchByName", 51, 2, "sess_mutex", 1);
	debugLogA("SearchByName(): Seq %d.", req->seq);
	gg_pubdir50_free(req);

	return (HANDLE)1;
}

//////////////////////////////////////////////////////////
// search by advanced

HWND GGPROTO::SearchAdvanced(HWND hwndDlg)
{
	gg_pubdir50_t req;
	TCHAR text[64];
	char data[800] = "\0";
	unsigned long crc;

	// Check if connected
	if (!isonline()) return (HWND)0;

	if (!(req = gg_pubdir50_new(GG_PUBDIR50_SEARCH)))
	{
#ifdef DEBUGMODE
		debugLogA("SearchAdvanced(): ForkThread 14 GGPROTO::searchthread");
#endif
		ForkThread(&GGPROTO::searchthread, NULL);
		return (HWND)1;
	}

	// Fetch search data
	GetDlgItemText(hwndDlg, IDC_FIRSTNAME, text, SIZEOF(text));
	if (mir_tstrlen(text))
	{
		char *firstName_utf8 = mir_utf8encodeT(text);
		gg_pubdir50_add(req, GG_PUBDIR50_FIRSTNAME, firstName_utf8);
		strncat(data, firstName_utf8, sizeof(data) - mir_strlen(data));
		mir_free(firstName_utf8);
	}
	/* 1 */ strncat(data, ".", sizeof(data) - mir_strlen(data));

	GetDlgItemText(hwndDlg, IDC_LASTNAME, text, SIZEOF(text));
	if (mir_tstrlen(text))
	{
		char *lastName_utf8 = mir_utf8encodeT(text);
		gg_pubdir50_add(req, GG_PUBDIR50_LASTNAME, lastName_utf8);
		strncat(data, lastName_utf8, sizeof(data) - mir_strlen(data));
		mir_free(lastName_utf8);
	}
	/* 2 */ strncat(data, ".", sizeof(data) - mir_strlen(data));

	GetDlgItemText(hwndDlg, IDC_NICKNAME, text, SIZEOF(text));
	if (mir_tstrlen(text))
	{
		char *nickName_utf8 = mir_utf8encodeT(text);
		gg_pubdir50_add(req, GG_PUBDIR50_NICKNAME, nickName_utf8);
		strncat(data, nickName_utf8, sizeof(data) - mir_strlen(data));
		mir_free(nickName_utf8);
	}
	/* 3 */ strncat(data, ".", sizeof(data) - mir_strlen(data));

	GetDlgItemText(hwndDlg, IDC_CITY, text, SIZEOF(text));
	if (mir_tstrlen(text))
	{
		char *city_utf8 = mir_utf8encodeT(text);
		gg_pubdir50_add(req, GG_PUBDIR50_CITY, city_utf8);
		strncat(data, city_utf8, sizeof(data) - mir_strlen(data));
		mir_free(city_utf8);
	}
	/* 4 */ strncat(data, ".", sizeof(data) - mir_strlen(data));

	GetDlgItemText(hwndDlg, IDC_AGEFROM, text, SIZEOF(text));
	if (mir_tstrlen(text))
	{
		int yearTo = _tstoi(text);
		int yearFrom;
		time_t t = time(NULL);
		struct tm *lt = localtime(&t);
		int ay = lt->tm_year + 1900;
		char age[16];

		GetDlgItemTextA(hwndDlg, IDC_AGETO, age, SIZEOF(age));
		yearFrom = atoi(age);

		// Count & fix ranges
		if (!yearTo)
			yearTo = ay;
		else
			yearTo = ay - yearTo;
		if (!yearFrom)
			yearFrom = 0;
		else
			yearFrom = ay - yearFrom;
		mir_sntprintf(text, SIZEOF(text), _T("%d %d"), yearFrom, yearTo);

		char *age_utf8 = mir_utf8encodeT(text);
		gg_pubdir50_add(req, GG_PUBDIR50_BIRTHYEAR, age_utf8);
		strncat(data, age_utf8, sizeof(data) - mir_strlen(data));
		mir_free(age_utf8);
	}
	/* 5 */ strncat(data, ".", sizeof(data) - mir_strlen(data));

	switch(SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_GETCURSEL, 0, 0))
	{
		case 1:
			gg_pubdir50_add(req, GG_PUBDIR50_GENDER, GG_PUBDIR50_GENDER_FEMALE);
			strncat(data, GG_PUBDIR50_GENDER_MALE, sizeof(data) - mir_strlen(data));
			break;
		case 2:
			gg_pubdir50_add(req, GG_PUBDIR50_GENDER, GG_PUBDIR50_GENDER_MALE);
			strncat(data, GG_PUBDIR50_GENDER_FEMALE, sizeof(data) - mir_strlen(data));
			break;
	}
	/* 6 */ strncat(data, ".", sizeof(data) - mir_strlen(data));

	if (IsDlgButtonChecked(hwndDlg, IDC_ONLYCONNECTED))
	{
		gg_pubdir50_add(req, GG_PUBDIR50_ACTIVE, GG_PUBDIR50_ACTIVE_TRUE);
		strncat(data, GG_PUBDIR50_ACTIVE_TRUE, sizeof(data) - mir_strlen(data));
	}
	/* 7 */ strncat(data, ".", sizeof(data) - mir_strlen(data));

	// No data entered
	if (mir_strlen(data) <= 7 || (mir_strlen(data) == 8 && IsDlgButtonChecked(hwndDlg, IDC_ONLYCONNECTED))) return (HWND)0;

	// Count crc & check if the data was equal if yes do same search with shift
	crc = crc_get(data);

	if (crc == last_crc && next_uin)
		gg_pubdir50_add(req, GG_PUBDIR50_START, ditoa(next_uin));
	else
		last_crc = crc;

	gg_pubdir50_seq_set(req, GG_SEQ_SEARCH);

	if (isonline())
	{
		gg_EnterCriticalSection(&sess_mutex, "SearchAdvanced", 52, "sess_mutex", 1);
		if (!gg_pubdir50(sess, req))
		{
			gg_LeaveCriticalSection(&sess_mutex, "SearchAdvanced", 52, 1, "sess_mutex", 1);
#ifdef DEBUGMODE
			debugLogA("SearchAdvanced(): ForkThread 15 GGPROTO::searchthread");
#endif
			ForkThread(&GGPROTO::searchthread, NULL);
			return (HWND)1;
		}
		gg_LeaveCriticalSection(&sess_mutex, "SearchAdvanced", 52, 2, "sess_mutex", 1);
	}
	debugLogA("SearchAdvanced(): Seq %d.", req->seq);
	gg_pubdir50_free(req);

	return (HWND)1;
}

//////////////////////////////////////////////////////////
// create adv search dialog

static INT_PTR CALLBACK gg_advancedsearchdlgproc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch(message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)_T(""));				// 0
		SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)TranslateT("Female"));	// 1
		SendDlgItemMessage(hwndDlg, IDC_GENDER, CB_ADDSTRING, 0, (LPARAM)TranslateT("Male"));	// 2
		return TRUE;
	}
	return FALSE;
}

HWND GGPROTO::CreateExtendedSearchUI(HWND owner)
{
	return CreateDialogParam(hInstance,
		MAKEINTRESOURCE(IDD_GGADVANCEDSEARCH), owner, gg_advancedsearchdlgproc, (LPARAM)this);
}

//////////////////////////////////////////////////////////
// when messsage sent

typedef struct
{
	MCONTACT hContact;
	int seq;
} GG_SEQ_ACK;

void __cdecl GGPROTO::sendackthread(void *ack)
{
	gg_sleep(100, FALSE, "sendackthread", 105, 1);
	ProtoBroadcastAck(((GG_SEQ_ACK *)ack)->hContact,
		ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE) ((GG_SEQ_ACK *)ack)->seq, 0);
	mir_free(ack);
}

int GGPROTO::SendMsg(MCONTACT hContact, int, const char *msg)
{
	uin_t uin = (uin_t)getDword(hContact, GG_KEY_UIN, 0);
	if (!isonline() || !uin)
		return 0;

	if (!msg)
		return 0;

	gg_EnterCriticalSection(&sess_mutex, "SendMsg", 53, "sess_mutex", 1);
	int seq = gg_send_message(sess, GG_CLASS_CHAT, uin, (BYTE*)msg);
	gg_LeaveCriticalSection(&sess_mutex, "SendMsg", 53, 1, "sess_mutex", 1);
	if (!getByte(GG_KEY_MSGACK, GG_KEYDEF_MSGACK))
	{
		// Auto-ack message without waiting for server ack
		GG_SEQ_ACK *ack = (GG_SEQ_ACK*)mir_alloc(sizeof(GG_SEQ_ACK));
		if (ack)
		{
			ack->seq = seq;
			ack->hContact = hContact;
#ifdef DEBUGMODE
			debugLogA("SendMsg(): ForkThread 16 GGPROTO::sendackthread");
#endif
			ForkThread(&GGPROTO::sendackthread, ack);
		}
	}
	return seq;
}

//////////////////////////////////////////////////////////
// visible lists

int GGPROTO::SetApparentMode(MCONTACT hContact, int mode)
{
	setWord(hContact, GG_KEY_APPARENT, (WORD)mode);
	notifyuser(hContact, 1);
	return 0;
}

//////////////////////////////////////////////////////////
// sets protocol status

int GGPROTO::SetStatus(int iNewStatus)
{
	int nNewStatus = gg_normalizestatus(iNewStatus);

	gg_EnterCriticalSection(&modemsg_mutex, "SetStatus", 54, "modemsg_mutex", 1);
	m_iDesiredStatus = nNewStatus;
	gg_LeaveCriticalSection(&modemsg_mutex, "SetStatus", 54, 1, "modemsg_mutex", 1);

	// If waiting for connection retry attempt then signal to stop that
	if (hConnStopEvent) SetEvent(hConnStopEvent);

	if (m_iStatus == nNewStatus) return 0;
	debugLogA("SetStatus(): PS_SETSTATUS(%d) normalized to %d.", iNewStatus, nNewStatus);
	refreshstatus(nNewStatus);

	return 0;
}

//////////////////////////////////////////////////////////
// when away message is requested

void __cdecl GGPROTO::getawaymsgthread(void *hContact)
{
	DBVARIANT dbv;

	debugLogA("getawaymsgthread(): started");
	gg_sleep(100, FALSE, "getawaymsgthread", 106, 1);
	if (!db_get_s((MCONTACT)hContact, "CList", GG_KEY_STATUSDESCR, &dbv, DBVT_TCHAR))
	{
		ProtoBroadcastAck((MCONTACT)hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)dbv.ptszVal);
		debugLog(_T("getawaymsgthread(): Reading away msg <%s>."), dbv.ptszVal);
		db_free(&dbv);
	} else {
		ProtoBroadcastAck((MCONTACT)hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, (HANDLE)1, (LPARAM)NULL);
	}
	debugLogA("getawaymsgthread(): end");
}

HANDLE GGPROTO::GetAwayMsg(MCONTACT hContact)
{
#ifdef DEBUGMODE
	debugLogA("GetAwayMsg(): ForkThread 17 GGPROTO::getawaymsgthread");
#endif
	ForkThread(&GGPROTO::getawaymsgthread, (void*)hContact);
	return (HANDLE)1;
}

//////////////////////////////////////////////////////////
// when away message is being set
// registered as ProtoService PS_SETAWAYMSGT

int GGPROTO::SetAwayMsg(int iStatus, const PROTOCHAR *newMsg)
{
	int status = gg_normalizestatus(iStatus);
	TCHAR **msgPtr;

	debugLog(_T("SetAwayMsg(): PS_SETAWAYMSG(%d, \"%s\")."), iStatus, newMsg);

	gg_EnterCriticalSection(&modemsg_mutex, "SetAwayMsg", 55, "modemsg_mutex", 1);
	// Select proper our msg ptr
	switch(status)
	{
		case ID_STATUS_ONLINE:
			msgPtr = &modemsg.online;
			break;
		case ID_STATUS_AWAY:
			msgPtr = &modemsg.away;
			break;
		case ID_STATUS_DND:
			msgPtr = &modemsg.dnd;
			break;
		case ID_STATUS_FREECHAT:
			msgPtr = &modemsg.freechat;
			break;
		case ID_STATUS_INVISIBLE:
			msgPtr = &modemsg.invisible;
			break;
		default:
			gg_LeaveCriticalSection(&modemsg_mutex, "SetAwayMsg", 55, 1, "modemsg_mutex", 1);
			return 1;
	}

	// Check if we change status here somehow
	if (*msgPtr && newMsg && !_tcscmp(*msgPtr, newMsg)
		|| !*msgPtr && (!newMsg || !*newMsg))
	{
		if (status == m_iDesiredStatus && m_iDesiredStatus == m_iStatus)
		{
			debugLogA("SetAwayMsg(): Message hasn't been changed, return.");
			gg_LeaveCriticalSection(&modemsg_mutex, "SetAwayMsg", 55, 2, "modemsg_mutex", 1);
			return 0;
		}
	}
	else
	{
		if (*msgPtr)
			mir_free(*msgPtr);
		*msgPtr = newMsg && *newMsg ? mir_tstrdup(newMsg) : NULL;
#ifdef DEBUGMODE
		debugLogA("SetAwayMsg(): Message changed.");
#endif
	}
	gg_LeaveCriticalSection(&modemsg_mutex, "SetAwayMsg", 55, 3, "modemsg_mutex", 1);

	// Change the status if it was desired by PS_SETSTATUS
	if (status == m_iDesiredStatus)
		refreshstatus(status);

	return 0;
}

//////////////////////////////////////////////////////////
// sends a notification that the user is typing a message

int GGPROTO::UserIsTyping(MCONTACT hContact, int type)
{
	uin_t uin = getDword(hContact, GG_KEY_UIN, 0);
	if (!uin || !isonline())
		return 0;

	if (type == PROTOTYPE_SELFTYPING_ON || type == PROTOTYPE_SELFTYPING_OFF) {
		gg_EnterCriticalSection(&sess_mutex, "UserIsTyping", 56, "sess_mutex", 1);
		gg_typing_notification(sess, uin, (type == PROTOTYPE_SELFTYPING_ON));
		gg_LeaveCriticalSection(&sess_mutex, "UserIsTyping", 56, 1, "sess_mutex", 1);
	}

	return 0;
}

//////////////////////////////////////////////////////////
// Custom protocol event

int GGPROTO::OnEvent(PROTOEVENTTYPE eventType, WPARAM wParam, LPARAM lParam)
{
	switch( eventType ) {
	case EV_PROTO_ONLOAD:
		HookProtoEvent(ME_OPT_INITIALISE, &GGPROTO::options_init);
		HookProtoEvent(ME_USERINFO_INITIALISE, &GGPROTO::details_init);

		// Init misc stuff
		gg_icolib_init();
		initpopups();
		gc_init();
		keepalive_init();
		img_init();
		block_init();

		// Try to fetch user avatar
		getOwnAvatar();
		break;

	case EV_PROTO_ONEXIT:
		// Stop avatar request thread
		pth_avatar.dwThreadId = 0;

		// Stop main connection session thread
		pth_sess.dwThreadId = 0;

		img_shutdown();
		sessions_closedlg();
		break;

	case EV_PROTO_ONOPTIONS:
		return options_init(wParam, lParam);

	case EV_PROTO_ONMENU:
		menus_init();
		break;

	case EV_PROTO_ONRENAME:
		if (hMenuRoot) {
			CLISTMENUITEM mi = { sizeof(mi) };
			mi.flags = CMIM_NAME | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
			mi.ptszName = m_tszUserName;
			Menu_ModifyItem(hMenuRoot, &mi);
		}
		break;

	case EV_PROTO_ONCONTACTDELETED:
		return contactdeleted(wParam, lParam);

	case EV_PROTO_DBSETTINGSCHANGED:
		return dbsettingchanged(wParam, lParam);
	}
	return TRUE;
}
