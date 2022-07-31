////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2009 Adam Strzelecki <ono+miranda@java.pl>
// Copyright (c) 2009-2012 Bartosz Białek
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
#include <errno.h>
#include <io.h>

#pragma warning(disable : 4189)

////////////////////////////////////////////////////////////
// Swap bits in uint32_t
uint32_t swap32(uint32_t x)
{
	return (uint32_t)
		(((x & (uint32_t)0x000000ffU) << 24) |
		((x & (uint32_t)0x0000ff00U) << 8) |
			((x & (uint32_t)0x00ff0000U) >> 8) |
			((x & (uint32_t)0xff000000U) >> 24));
}

////////////////////////////////////////////////////////////
// Is online function

int GaduProto::isonline()
{
	int isonline;

	gg_EnterCriticalSection(&sess_mutex, "isonline", 76, "sess_mutex", 1);
	isonline = (m_sess != nullptr);
	gg_LeaveCriticalSection(&sess_mutex, "isonline", 76, 1, "sess_mutex", 1);

	return isonline;
}

////////////////////////////////////////////////////////////
// Send disconnect request and wait for server thread to die
void GaduProto::disconnect()
{
	// If main loop then send disconnect request
	if (isonline())
	{
		// Fetch proper status msg
		char *szMsg = nullptr;

		// Loadup status
		if (getByte(GG_KEY_LEAVESTATUSMSG, GG_KEYDEF_LEAVESTATUSMSG))
		{
			DBVARIANT dbv;
			switch (getWord(GG_KEY_LEAVESTATUS, GG_KEYDEF_LEAVESTATUS)) {
			case ID_STATUS_ONLINE:
				gg_EnterCriticalSection(&modemsg_mutex, "disconnect", 6, "modemsg_mutex", 1);
				szMsg = mir_utf8encodeW(modemsg.online);
				gg_LeaveCriticalSection(&modemsg_mutex, "disconnect", 6, 1, "modemsg_mutex", 1);
				if (!szMsg && !db_get_s(0, "SRAway", gg_status2db(ID_STATUS_ONLINE, "Default"), &dbv, DBVT_WCHAR)) {
					if (dbv.pwszVal && *(dbv.pwszVal))
						szMsg = mir_utf8encodeW(dbv.pwszVal);
					db_free(&dbv);
				}
				break;

			case ID_STATUS_AWAY:
				gg_EnterCriticalSection(&modemsg_mutex, "disconnect", 7, "modemsg_mutex", 1);
				szMsg = mir_utf8encodeW(modemsg.away);
				gg_LeaveCriticalSection(&modemsg_mutex, "disconnect", 7, 1, "modemsg_mutex", 1);
				if (!szMsg && !db_get_s(0, "SRAway", gg_status2db(ID_STATUS_AWAY, "Default"), &dbv, DBVT_WCHAR)) {
					if (dbv.pwszVal && *(dbv.pwszVal))
						szMsg = mir_utf8encodeW(dbv.pwszVal);
					db_free(&dbv);
				}
				break;

			case ID_STATUS_DND:
				gg_EnterCriticalSection(&modemsg_mutex, "disconnect", 8, "modemsg_mutex", 1);
				szMsg = mir_utf8encodeW(modemsg.dnd);
				gg_LeaveCriticalSection(&modemsg_mutex, "disconnect", 8, 1, "modemsg_mutex", 1);
				if (!szMsg && !db_get_s(0, "SRAway", gg_status2db(ID_STATUS_DND, "Default"), &dbv, DBVT_WCHAR)) {
					if (dbv.pwszVal && *(dbv.pwszVal))
						szMsg = mir_utf8encodeW(dbv.pwszVal);
					db_free(&dbv);
				}
				break;

			case ID_STATUS_FREECHAT:
				gg_EnterCriticalSection(&modemsg_mutex, "disconnect", 9, "modemsg_mutex", 1);
				szMsg = mir_utf8encodeW(modemsg.freechat);
				gg_LeaveCriticalSection(&modemsg_mutex, "disconnect", 9, 1, "modemsg_mutex", 1);
				if (!szMsg && !db_get_s(0, "SRAway", gg_status2db(ID_STATUS_FREECHAT, "Default"), &dbv, DBVT_WCHAR)) {
					if (dbv.pwszVal && *(dbv.pwszVal))
						szMsg = mir_utf8encodeW(dbv.pwszVal);
					db_free(&dbv);
				}
				break;

			case ID_STATUS_INVISIBLE:
				gg_EnterCriticalSection(&modemsg_mutex, "disconnect", 10, "modemsg_mutex", 1);
				szMsg = mir_utf8encodeW(modemsg.invisible);
				gg_LeaveCriticalSection(&modemsg_mutex, "disconnect", 10, 1, "modemsg_mutex", 1);
				if (!szMsg && !db_get_s(0, "SRAway", gg_status2db(ID_STATUS_INVISIBLE, "Default"), &dbv, DBVT_WCHAR)) {
					if (dbv.pwszVal && *(dbv.pwszVal))
						szMsg = mir_utf8encodeW(dbv.pwszVal);
					db_free(&dbv);
				}
				break;

			default:
				// Set last status
				gg_EnterCriticalSection(&modemsg_mutex, "disconnect", 11, "modemsg_mutex", 1);
				szMsg = mir_utf8encodeW(getstatusmsg(m_iStatus));
				gg_LeaveCriticalSection(&modemsg_mutex, "disconnect", 11, 1, "modemsg_mutex", 1);
			}
		}

		gg_EnterCriticalSection(&sess_mutex, "disconnect", 12, "sess_mutex", 1);
		// Check if it has message
		if (szMsg)
		{
			gg_change_status_descr(m_sess, GG_STATUS_NOT_AVAIL_DESCR, szMsg);
			mir_free(szMsg);
			// Wait for disconnection acknowledge
		}
		else
		{
			gg_change_status(m_sess, GG_STATUS_NOT_AVAIL);
			// Send logoff immediately
			gg_logoff(m_sess);
		}
		gg_LeaveCriticalSection(&sess_mutex, "disconnect", 12, 1, "sess_mutex", 1);
	}
	// Else cancel connection attempt
	else if (sock)
		closesocket(sock);
}

////////////////////////////////////////////////////////////
// DNS lookup function
uint32_t gg_dnslookup(GaduProto *gg, const char *host)
{
	uint32_t ip = inet_addr(host);
	if (ip != INADDR_NONE)
	{
#ifdef DEBUGMODE
		gg->debugLogA("gg_dnslookup(): Parameter \"%s\" is already IP number.", host);
#endif
		return ip;
	}

	hostent *he = gethostbyname(host);
	if (he)
	{
		ip = *(uint32_t *)he->h_addr_list[0];
#ifdef DEBUGMODE
		gg->debugLogA("gg_dnslookup(): Parameter \"%s\" was resolved to %d.%d.%d.%d.", host,
			LOBYTE(LOWORD(ip)), HIBYTE(LOWORD(ip)), LOBYTE(HIWORD(ip)), HIBYTE(HIWORD(ip)));
#endif
		return ip;
	}

	gg->debugLogA("gg_dnslookup(): Cannot resolve hostname \"%s\".", host);

	return 0;
}

////////////////////////////////////////////////////////////
// Host list decoder
struct GGHOST
{
	char hostname[128];
	int port;
};

#define ISHOSTALPHA(a) (((a) >= '0' && (a) <= '9') || ((a) >= 'a' && (a) <= 'z') || (a) == '.' || (a) == '-')

int gg_decodehosts(char *var, GGHOST *hosts, int max)
{
	int hp = 0;
	char *hostname = nullptr;
	char *portname = nullptr;

	while (var && *var && hp < max)
	{
		if (ISHOSTALPHA(*var))
		{
			hostname = var;

			while (var && *var && ISHOSTALPHA(*var)) var++;

			if (var && *var == ':' && var++ && *var && isdigit(*var))
			{
				*(var - 1) = 0;
				portname = var;
				while (var && *var && isdigit(*var)) var++;
				if (*var) { *var = 0; var++; }
			}
			else
				if (*var) { *var = 0; var++; }

			// Insert new item
			hosts[hp].hostname[127] = 0;
			strncpy(hosts[hp].hostname, hostname, 127);
			hosts[hp].port = portname ? atoi(portname) : 443;
			hp++;

			// Zero the names
			hostname = nullptr;
			portname = nullptr;
		}
		else
			var++;
	}
	return hp;
}

////////////////////////////////////////////////////////////
// Main connection session thread
void __cdecl GaduProto::mainthread(void *)
{
	// Gadu-gadu login errors
	static const struct tagReason { int type; wchar_t *str; } reason[] = {
		{ GG_FAILURE_RESOLVING,   LPGENW("Miranda was unable to resolve the name of the Gadu-Gadu server to its numeric address.") },
		{ GG_FAILURE_CONNECTING,  LPGENW("Miranda was unable to make a connection with a server. It is likely that the server is down, in which case you should wait for a while and try again later.") },
		{ GG_FAILURE_INVALID,     LPGENW("Received invalid server response.") },
		{ GG_FAILURE_READING,     LPGENW("The connection with the server was abortively closed during the connection attempt. You may have lost your local network connection.") },
		{ GG_FAILURE_WRITING,     LPGENW("The connection with the server was abortively closed during the connection attempt. You may have lost your local network connection.") },
		{ GG_FAILURE_PASSWORD,    LPGENW("Your Gadu-Gadu number and password combination was rejected by the Gadu-Gadu server. Please check login details at Options -> Network -> Gadu-Gadu and try again.") },
		{ GG_FAILURE_404,         LPGENW("Connecting to Gadu-Gadu hub failed.") },
		{ GG_FAILURE_TLS,         LPGENW("Cannot establish secure connection.") },
		{ GG_FAILURE_NEED_EMAIL,  LPGENW("Server disconnected asking you for changing your e-mail.") },
		{ GG_FAILURE_INTRUDER,    LPGENW("Too many login attempts with invalid password.") },
		{ GG_FAILURE_UNAVAILABLE, LPGENW("Gadu-Gadu servers are now down. Try again later.") },
		{ 0,                      LPGENW("Unknown") }
	};
	time_t logonTime = 0;
	time_t timeDeviation = getWord(GG_KEY_TIMEDEVIATION, GG_KEYDEF_TIMEDEVIATION);
	int gg_failno = 0;

	debugLogA("mainthread(): started. (%x) Server Thread Starting", this);
#ifdef DEBUGMODE
	gg_debug_level = GG_DEBUG_NET | GG_DEBUG_TRAFFIC | GG_DEBUG_FUNCTION | GG_DEBUG_MISC;
#else
	gg_debug_level = 0;
#endif

	// Broadcast that service is connecting
	broadcastnewstatus(ID_STATUS_CONNECTING);

	// Client version and misc settings
	gg_login_params p = { 0 };
	p.client_version = GG_DEFAULT_CLIENT_VERSION;
	p.protocol_version = GG_DEFAULT_PROTOCOL_VERSION;
	p.protocol_features = GG_FEATURE_DND_FFC | GG_FEATURE_UNKNOWN_100 | GG_FEATURE_USER_DATA | GG_FEATURE_MSG_ACK | GG_FEATURE_TYPING_NOTIFICATION | GG_FEATURE_MULTILOGON;
	p.encoding = GG_ENCODING_UTF8;
	p.status_flags = GG_STATUS_FLAG_UNKNOWN;
	if (getByte(GG_KEY_SHOWLINKS, GG_KEYDEF_SHOWLINKS))
		p.status_flags |= GG_STATUS_FLAG_SPAM;

	// Send Era Omnix info if set
	p.era_omnix = getByte("EraOmnix", 0);

	// Setup proxy
	NETLIBUSERSETTINGS nlus = { 0 };
	nlus.cbSize = sizeof(nlus);
	if (Netlib_GetUserSettings(m_hNetlibUser, &nlus)) {
		if (nlus.useProxy)
			debugLogA("mainthread() (%x): Using proxy %s:%d.", this, nlus.szProxyServer, nlus.wProxyPort);
		gg_proxy_enabled = nlus.useProxy;
		gg_proxy_host = nlus.szProxyServer;
		gg_proxy_port = nlus.wProxyPort;
		if (nlus.useProxyAuth) {
			gg_proxy_username = nlus.szProxyAuthUser;
			gg_proxy_password = nlus.szProxyAuthPassword;
		}
		else gg_proxy_username = gg_proxy_password = nullptr;
	}
	else {
		debugLogA("mainthread() (%x): Failed loading proxy settings.", this);
		gg_proxy_enabled = 0;
	}

	// Check out manual host setting
	DBVARIANT dbv;
	int hostcount = 0;
	GGHOST hosts[64];

	if (m_gaduOptions.useManualHosts) {
		CMStringW serverHosts = m_gaduOptions.serverHosts;
		if (!serverHosts.IsEmpty()) {
			ptrA pHostsList(mir_u2a(serverHosts.c_str()));
			hostcount = gg_decodehosts(pHostsList, hosts, 64);
		}
	}

	// Readup password
	if (!getString(GG_KEY_PASSWORD, &dbv)) {
		p.password = mir_strdup(dbv.pszVal);
		db_free(&dbv);
	}
	else {
		debugLogA("mainthread() (%x): No password specified. Exiting.", this);
		broadcastnewstatus(ID_STATUS_OFFLINE);
#ifdef DEBUGMODE
		debugLogA("mainthread(): end. err1");
#endif
		return;
	}

	// Readup number
	if (!(p.uin = getDword(GG_KEY_UIN, 0))) {
		debugLogA("mainthread() (%x): No Gadu-Gadu number specified. Exiting.", this);
		broadcastnewstatus(ID_STATUS_OFFLINE);
		mir_free(p.password);
#ifdef DEBUGMODE
		debugLogA("mainthread(): end. err2");
#endif
		return;
	}

	// Readup SSL/TLS setting
	if (p.tls = m_gaduOptions.useSslConnection)
		debugLogA("mainthread() (%x): Using TLS/SSL for connections.", this);

	// Gadu-Gadu accepts image sizes upto 255
	p.image_size = 255;

	////////////////////////////// DCC STARTUP /////////////////////////////
	// Uin is ok so startup dcc if not started already
	if (!m_dcc) {
		hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		dccstart();

		// Wait for DCC
#ifdef DEBUGMODE
		debugLogA("mainthread() (%x): Waiting DCC service to start...", this);
#endif
		while (WaitForSingleObjectEx(hEvent, INFINITE, TRUE) != WAIT_OBJECT_0);
		CloseHandle(hEvent); hEvent = nullptr;
	}

	// Check if dcc is running and setup forwarding port
	if (m_dcc && m_gaduOptions.useForwarding) {
		CMStringW forwardHost = m_gaduOptions.forwardHost;
		ptrA pHost(mir_u2a(forwardHost.c_str()));
		if (!forwardHost.IsEmpty()) {
			if (!(p.external_addr = gg_dnslookup(this, pHost))) {
				CMStringW error(FORMAT, TranslateT("External direct connections hostname %s is invalid. Disabling external host forwarding."), forwardHost.c_str());
				showpopup(m_tszUserName, error.c_str(), GG_POPUP_WARNING | GG_POPUP_ALLOW_MSGBOX);
			}
			else
				debugLogA("mainthread() (%x): Loading forwarding host %s and port %d.", pHost.get(), p.external_port, this);

			if (p.external_addr)
				p.external_port = m_gaduOptions.forwardPort;
		}
	}
	// Setup client port
	if (m_dcc)
		p.client_port = m_dcc->port;

	int hostnum = 0;

retry:
	// Loadup startup status & description
	gg_EnterCriticalSection(&modemsg_mutex, "mainthread", 13, "modemsg_mutex", 1);

	p.status_descr = mir_utf8encodeW(getstatusmsg(m_iDesiredStatus));
	p.status = status_m2gg(m_iDesiredStatus, p.status_descr != nullptr);

	debugLogW(L"mainthread() (%x): Connecting with number %d, status %d and description \"%s\".", this, p.uin, m_iDesiredStatus,
		p.status_descr ? getstatusmsg(m_iDesiredStatus) : L"<none>");
	gg_LeaveCriticalSection(&modemsg_mutex, "mainthread", 13, 1, "modemsg_mutex", 1);

	// Check manual hosts
	if (hostnum < hostcount) {
		if (!(p.server_addr = gg_dnslookup(this, hosts[hostnum].hostname))) {
			wchar_t error[128];
			wchar_t* hostnameT = mir_a2u(hosts[hostnum].hostname);
			mir_snwprintf(error, TranslateT("Server hostname %s is invalid. Using default hostname provided by the network."), hostnameT);
			mir_free(hostnameT);
			showpopup(m_tszUserName, error, GG_POPUP_WARNING | GG_POPUP_ALLOW_MSGBOX);
		}
		else {
			p.server_port = hosts[hostnum].port;
			debugLogA("mainthread() (%x): Connecting to manually specified host %s (%d.%d.%d.%d) and port %d.", this,
				hosts[hostnum].hostname, LOBYTE(LOWORD(p.server_addr)), HIBYTE(LOWORD(p.server_addr)),
				LOBYTE(HIWORD(p.server_addr)), HIBYTE(HIWORD(p.server_addr)), p.server_port);
		}
	}
	else {
		p.server_port = 0;
		p.server_addr = 0;
	}

	// Send login request
	struct gg_session *local_sess;
	if (!(local_sess = gg_login(&p, &sock, &gg_failno))) {
		broadcastnewstatus(ID_STATUS_OFFLINE);
		// Check if connection attempt wasn't cancelled by the user
		if (m_iDesiredStatus != ID_STATUS_OFFLINE) {
			wchar_t error[128], *perror = nullptr;
			// Lookup for error desciption
			if (errno == EACCES) {
				for (int i = 0; reason[i].type; i++) {
					if (reason[i].type == gg_failno) {
						perror = TranslateW(reason[i].str);
						break;
					}
				}
			}
			if (!perror) {
				mir_snwprintf(error, TranslateT("Connection cannot be established. errno=%d: %s"), errno, ws_strerror(errno));
				perror = error;
			}
			debugLogW(L"mainthread() (%x): %s", this, perror);
			if (m_gaduOptions.showConnectionErrors)
				showpopup(m_tszUserName, perror, GG_POPUP_ERROR | GG_POPUP_ALLOW_MSGBOX | GG_POPUP_ONCE);

			// Check if we should reconnect
			if ((gg_failno >= GG_FAILURE_RESOLVING && gg_failno != GG_FAILURE_PASSWORD && gg_failno != GG_FAILURE_INTRUDER && gg_failno != GG_FAILURE_UNAVAILABLE)
				&& errno == EACCES
				&& (m_gaduOptions.autoRecconect || (hostnum < hostcount - 1)))
			{
				uint32_t dwInterval = getDword(GG_KEY_RECONNINTERVAL, GG_KEYDEF_RECONNINTERVAL);
				BOOL bRetry = TRUE;

				hConnStopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
				uint32_t dwResult = WaitForSingleObjectEx(hConnStopEvent, dwInterval, TRUE);
				if ((dwResult == WAIT_OBJECT_0 && m_iDesiredStatus == ID_STATUS_OFFLINE)
					|| (dwResult == WAIT_IO_COMPLETION && Miranda_IsTerminated()))
					bRetry = FALSE;
				CloseHandle(hConnStopEvent);
				hConnStopEvent = nullptr;

				// Reconnect to the next server on the list
				if (bRetry) {
					if (hostnum < hostcount - 1)
						++hostnum;

					mir_free(p.status_descr);
					broadcastnewstatus(ID_STATUS_CONNECTING);
					goto retry;
				}
			}
			// We cannot do more about this
			gg_EnterCriticalSection(&modemsg_mutex, "mainthread", 14, "modemsg_mutex", 1);
			m_iDesiredStatus = ID_STATUS_OFFLINE;
			gg_LeaveCriticalSection(&modemsg_mutex, "mainthread", 14, 1, "modemsg_mutex", 1);
		}
		else
			debugLogA("mainthread() (%x)): Connection attempt cancelled by the user.", this);
	}
	else {
		// Successfully connected
		logonTime = time(0);
		setDword(GG_KEY_LOGONTIME, logonTime);
		gg_EnterCriticalSection(&sess_mutex, "mainthread", 15, "sess_mutex", 1);
		m_sess = local_sess;
		gg_LeaveCriticalSection(&sess_mutex, "mainthread", 15, 1, "sess_mutex", 1);
		// Subscribe users status notifications
		notifyall();
		// Set startup status
		if (m_iDesiredStatus != status_gg2m(p.status))
			refreshstatus(m_iDesiredStatus);
		else {
			broadcastnewstatus(m_iDesiredStatus);
			// Change status of the contact with our own UIN (if got yourself added to the contact list)
			wchar_t *status_descr = mir_utf8decodeW(p.status_descr);
			changecontactstatus(p.uin, p.status, status_descr, 0, 0, 0, 0);
			mir_free(status_descr);
		}

		// First connection to the account
		if (check_first_conn) {
			// Start search for user data
			GetInfo(NULL, 0);
			// Fetch user avatar
			getOwnAvatar();
			check_first_conn = 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Main loop
	gg_event *e;

	while (isonline())
	{
		// Connection broken/closed
#ifdef DEBUGMODE
		debugLogA("mainthread(): waiting for gg_watch_fd");
#endif
		if (!(e = gg_watch_fd(m_sess))) {
#ifdef DEBUGMODE
			debugLogA("mainthread(): waiting for gg_watch_fd - DONE error");
#endif
			debugLogA("mainthread() (%x): Connection closed.", this);
			gg_EnterCriticalSection(&sess_mutex, "mainthread", 16, "sess_mutex", 1);
			gg_free_session(m_sess);
			m_sess = nullptr;
			gg_LeaveCriticalSection(&sess_mutex, "mainthread", 16, 1, "sess_mutex", 1);
			break;
		}
		else {
#ifdef DEBUGMODE
			debugLogA("mainthread(): waiting for gg_watch_fd - DONE");
#endif
			debugLogA("mainthread() (%x): Event: %s", this, ggdebug_eventtype(e));
		}

		switch (e->type) {
			// Client connected
		case GG_EVENT_CONN_SUCCESS:
			// Nada
			break;

			// Client disconnected or connection failure
		case GG_EVENT_CONN_FAILED:
		case GG_EVENT_DISCONNECT:
			gg_EnterCriticalSection(&sess_mutex, "mainthread", 17, "sess_mutex", 1);
			gg_free_session(m_sess);
			m_sess = nullptr;
			gg_LeaveCriticalSection(&sess_mutex, "mainthread", 17, 1, "sess_mutex", 1);
			break;

			// Client allowed to disconnect
		case GG_EVENT_DISCONNECT_ACK:
			// Send logoff
			gg_logoff(m_sess);
			break;

			// Received ackowledge
		case GG_EVENT_ACK:
			if (e->event.ack.seq && e->event.ack.recipient) {
				ProtoBroadcastAck(getcontact((uint32_t)e->event.ack.recipient, 0, 0, nullptr),
					ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)e->event.ack.seq, 0);
			}
			break;

			// Statuslist notify (deprecated)
		case GG_EVENT_NOTIFY:
		case GG_EVENT_NOTIFY_DESCR:
		{
			struct gg_notify_reply *n;

			n = (e->type == GG_EVENT_NOTIFY) ? e->event.notify : e->event.notify_descr.notify;

			for (; n->uin; n++) {
				wchar_t *descrT = (e->type == GG_EVENT_NOTIFY_DESCR) ? mir_utf8decodeW(e->event.notify_descr.descr) : nullptr;
				changecontactstatus(n->uin, n->status, descrT, 0, n->remote_ip, n->remote_port, n->version);
				if (descrT)
					mir_free(descrT);
			}
			break;
		}
		// Statuslist notify (version >= 6.0)
		case GG_EVENT_NOTIFY60:
		{
			uin_t uin = (uin_t)getDword(GG_KEY_UIN, 0);
			for (int i = 0; e->event.notify60[i].uin; i++) {
				if (e->event.notify60[i].uin == uin)
					continue;

				wchar_t *descrT = mir_utf8decodeW(e->event.notify60[i].descr);
				changecontactstatus(e->event.notify60[i].uin, e->event.notify60[i].status, descrT,
					e->event.notify60[i].time, e->event.notify60[i].remote_ip, e->event.notify60[i].remote_port,
					e->event.notify60[i].version);
				mir_free(descrT);
				requestAvatarInfo(getcontact(e->event.notify60[i].uin, 0, 0, nullptr), 0);
			}
			break;
		}

		// Pubdir search reply && read own data reply
		case GG_EVENT_PUBDIR50_SEARCH_REPLY:
		case GG_EVENT_PUBDIR50_READ:
		case GG_EVENT_PUBDIR50_WRITE:
		{
			gg_pubdir50_t res = e->event.pubdir50;

			if (e->type == GG_EVENT_PUBDIR50_SEARCH_REPLY) {
				debugLogA("mainthread() (%x): Got user info.", this);
				// Store next search UIN
				if (res->seq == GG_SEQ_SEARCH)
					next_uin = gg_pubdir50_next(res);
			}
			else if (e->type == GG_EVENT_PUBDIR50_READ) {
				debugLogA("mainthread() (%x): Got owner info.", this);
			}
			else if (e->type == GG_EVENT_PUBDIR50_WRITE) {
				debugLogA("mainthread() (%x): Public directory save succesful.", this);
				// Update user details
				GetInfo(NULL, 0);
			}

			int count = gg_pubdir50_count(res);
			if (count > 0) {
				for (int i = 0; i < count; i++) {
					// Loadup fields
					const char *__fmnumber = gg_pubdir50_get(res, i, GG_PUBDIR50_UIN);
					wchar_t *__nickname = mir_utf8decodeW(gg_pubdir50_get(res, i, GG_PUBDIR50_NICKNAME));
					wchar_t *__firstname = mir_utf8decodeW(gg_pubdir50_get(res, i, GG_PUBDIR50_FIRSTNAME));
					wchar_t *__lastname = mir_utf8decodeW(gg_pubdir50_get(res, i, GG_PUBDIR50_LASTNAME));
					wchar_t *__familyname = mir_utf8decodeW(gg_pubdir50_get(res, i, GG_PUBDIR50_FAMILYNAME));
					wchar_t *__city = mir_utf8decodeW(gg_pubdir50_get(res, i, GG_PUBDIR50_CITY));
					wchar_t *__familycity = mir_utf8decodeW(gg_pubdir50_get(res, i, GG_PUBDIR50_FAMILYCITY));
					const char *__birthyear = gg_pubdir50_get(res, i, GG_PUBDIR50_BIRTHYEAR);
					const char *__gender = gg_pubdir50_get(res, i, GG_PUBDIR50_GENDER);
					const char *__status = gg_pubdir50_get(res, i, GG_PUBDIR50_STATUS);
					uin_t uin = __fmnumber ? atoi(__fmnumber) : 0;

					MCONTACT hContact = (res->seq == GG_SEQ_CHINFO) ? NULL : getcontact(uin, 0, 0, nullptr);
					debugLogA("mainthread() (%x): Search result for uin %d, seq %d.", this, uin, res->seq);
					if (res->seq == GG_SEQ_SEARCH) {
						wchar_t strFmt1[64];
						wchar_t strFmt2[64];

						wcsncpy_s(strFmt2, Clist_GetStatusModeDescription(status_gg2m(atoi(__status)), 0), _TRUNCATE);
						if (__city) {
							mir_snwprintf(strFmt1, L", %s %s", TranslateT("City:"), __city);
							mir_wstrncat(strFmt2, strFmt1, _countof(strFmt2) - mir_wstrlen(strFmt2));
						}
						if (__birthyear) {
							time_t t = time(0);
							struct tm *lt = localtime(&t);
							int br = atoi(__birthyear);

							if (br < (lt->tm_year + 1900) && br > 1900) {
								mir_snwprintf(strFmt1, L", %s %d", TranslateT("Age:"), (lt->tm_year + 1900) - br);
								mir_wstrncat(strFmt2, strFmt1, _countof(strFmt2) - mir_wstrlen(strFmt2));
							}
						}

						GGSEARCHRESULT psr;
						memset(&psr, 0, sizeof(psr));
						psr.cbSize = sizeof(psr);
						psr.flags = PSR_UNICODE;
						psr.nick.w = __nickname;
						psr.firstName.w = __firstname;
						psr.lastName.w = __lastname;
						psr.email.w = strFmt2;
						psr.id.w = _ultow(uin, strFmt1, 10);
						psr.uin = uin;
						ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);
					}

					if (((res->seq == GG_SEQ_INFO || res->seq == GG_SEQ_GETNICK) && hContact != NULL)
						|| res->seq == GG_SEQ_CHINFO)
					{
						// Change nickname if it's not present
						if (__nickname && (res->seq == GG_SEQ_GETNICK || res->seq == GG_SEQ_CHINFO))
							setWString(hContact, GG_KEY_NICK, __nickname);

						if (__nickname)
							setWString(hContact, GG_KEY_PD_NICKNAME, __nickname);
						else if (res->seq == GG_SEQ_CHINFO)
							delSetting(GG_KEY_PD_NICKNAME);

						// Change other info
						if (__city)
							setWString(hContact, GG_KEY_PD_CITY, __city);
						else if (res->seq == GG_SEQ_CHINFO)
							delSetting(GG_KEY_PD_CITY);

						if (__firstname)
							setWString(hContact, GG_KEY_PD_FIRSTNAME, __firstname);
						else if (res->seq == GG_SEQ_CHINFO)
							delSetting(GG_KEY_PD_FIRSTNAME);

						if (__lastname)
							setWString(hContact, GG_KEY_PD_LASTNAME, __lastname);
						else if (res->seq == GG_SEQ_CHINFO)
							delSetting(GG_KEY_PD_LASTNAME);

						if (__familyname)
							setWString(hContact, GG_KEY_PD_FAMILYNAME, __familyname);
						else if (res->seq == GG_SEQ_CHINFO)
							delSetting(GG_KEY_PD_FAMILYNAME);

						if (__familycity)
							setWString(hContact, GG_KEY_PD_FAMILYCITY, __familycity);
						else if (res->seq == GG_SEQ_CHINFO)
							delSetting(GG_KEY_PD_FAMILYCITY);

						if (__birthyear) {
							time_t t = time(0);
							struct tm *lt = localtime(&t);
							int br = atoi(__birthyear);
							if (br > 0)
							{
								setWord(hContact, GG_KEY_PD_AGE, (uint16_t)(lt->tm_year + 1900 - br));
								setWord(hContact, GG_KEY_PD_BIRTHYEAR, (uint16_t)br);
							}
						}
						else if (res->seq == GG_SEQ_CHINFO) {
							delSetting(GG_KEY_PD_AGE);
							delSetting(GG_KEY_PD_BIRTHYEAR);
						}

						// Gadu-Gadu Male <-> Female
						if (__gender) {
							if (res->seq == GG_SEQ_CHINFO)
								setByte(hContact, GG_KEY_PD_GANDER,
								(uint8_t)(!mir_strcmp(__gender, GG_PUBDIR50_GENDER_SET_MALE) ? 'M' :
									(!mir_strcmp(__gender, GG_PUBDIR50_GENDER_SET_FEMALE) ? 'F' : '?')));
							else
								setByte(hContact, GG_KEY_PD_GANDER,
								(uint8_t)(!mir_strcmp(__gender, GG_PUBDIR50_GENDER_MALE) ? 'M' :
									(!mir_strcmp(__gender, GG_PUBDIR50_GENDER_FEMALE) ? 'F' : '?')));
						}
						else if (res->seq == GG_SEQ_CHINFO) {
							delSetting(GG_KEY_PD_GANDER);
						}

						debugLogA("mainthread() (%x): Setting user info for uin %d.", this, uin);
						ProtoBroadcastAck(hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1);
					}

					if (__nickname) mir_free(__nickname);
					if (__firstname) mir_free(__firstname);
					if (__lastname) mir_free(__lastname);
					if (__familyname) mir_free(__familyname);
					if (__city) mir_free(__city);
					if (__familycity) mir_free(__familycity);
				}
			}
			if (res->seq == GG_SEQ_SEARCH)
				ProtoBroadcastAck(NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE)1, 0);

			break;
		}

		// Status (deprecated)
		case GG_EVENT_STATUS:
		{
			wchar_t *descrT = mir_utf8decodeW(e->event.status.descr);
			changecontactstatus(e->event.status.uin, e->event.status.status, descrT, 0, 0, 0, 0);
			mir_free(descrT);
		}
		break;

		// Status (version >= 6.0)
		case GG_EVENT_STATUS60:
		{
			MCONTACT hContact = getcontact(e->event.status60.uin, 0, 0, nullptr);
			int oldstatus = getWord(hContact, GG_KEY_STATUS, (uint16_t)ID_STATUS_OFFLINE);
			uin_t uin = (uin_t)getDword(GG_KEY_UIN, 0);

			wchar_t *descrT = mir_utf8decodeW(e->event.status60.descr);

			if (e->event.status60.uin == uin) {
				// Status was changed by the user simultaneously logged on using different Miranda account or IM client
				int iStatus = status_gg2m(e->event.status60.status);
				CallProtoService(m_szModuleName, PS_SETAWAYMSG, iStatus, (LPARAM)descrT);
				CallProtoService(m_szModuleName, PS_SETSTATUS, iStatus, 0);
			}

			changecontactstatus(e->event.status60.uin, e->event.status60.status, descrT,
				e->event.status60.time, e->event.status60.remote_ip, e->event.status60.remote_port, e->event.status60.version);

			mir_free(descrT);

			if (oldstatus == ID_STATUS_OFFLINE && getWord(hContact, GG_KEY_STATUS, (uint16_t)ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
				requestAvatarInfo(hContact, 0);
		}
		break;

		// Received userlist / or put info
		case GG_EVENT_USERLIST:
			switch (e->event.userlist.type) {
			case GG_USERLIST_GET_REPLY:
				if (e->event.userlist.reply) {
					parsecontacts(e->event.userlist.reply);
					MessageBox(nullptr, TranslateT("List import successful."), m_tszUserName, MB_OK | MB_ICONINFORMATION);
				}
				break;

			case GG_USERLIST_PUT_REPLY:
				if (is_list_remove)
					MessageBox(nullptr, TranslateT("List remove successful."), m_tszUserName, MB_OK | MB_ICONINFORMATION);
				else
					MessageBox(nullptr, TranslateT("List export successful."), m_tszUserName, MB_OK | MB_ICONINFORMATION);
				break;
			}
			break;

			// Received message
		case GG_EVENT_MSG:
			// This is CTCP request
			if ((e->event.msg.msgclass & GG_CLASS_CTCP)) {
				dccconnect(e->event.msg.sender);
			}
			// Check if not conference and block
			else if (!e->event.msg.recipients_count || gc_enabled) {
				// Check if groupchat
				if (e->event.msg.recipients_count && gc_enabled && !getByte(GG_KEY_IGNORECONF, GG_KEYDEF_IGNORECONF)) {
					wchar_t *chat = gc_getchat(e->event.msg.sender, e->event.msg.recipients, e->event.msg.recipients_count);
					if (chat) {
						wchar_t id[32];
						UIN2IDT(e->event.msg.sender, id);

						GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
						time_t t = time(0);
						gce.pszID.w = chat;
						gce.pszUID.w = id;
						wchar_t* messageT = mir_utf8decodeW(e->event.msg.message);
						gce.pszText.w = messageT;
						gce.pszNick.w = (wchar_t*)Clist_GetContactDisplayName(getcontact(e->event.msg.sender, 1, 0, nullptr));
						gce.time = (!(e->event.msg.msgclass & GG_CLASS_OFFLINE) || e->event.msg.time > (t - timeDeviation)) ? t : e->event.msg.time;
						gce.dwFlags = GCEF_ADDTOLOG;
						debugLogW(L"mainthread() (%x): Conference message to room %s & id %s.", this, chat, id);
						Chat_Event(&gce);
						mir_free(messageT);
					}
				}
				// Check if not empty message ( who needs it? )
				else if (!e->event.msg.recipients_count && e->event.msg.message && *e->event.msg.message && mir_strcmp(e->event.msg.message, "\xA0\0")) {
					PROTORECVEVENT pre = { 0 };
					time_t t = time(0);
					pre.timestamp = (!(e->event.msg.msgclass & GG_CLASS_OFFLINE) || e->event.msg.time > (t - timeDeviation)) ? t : e->event.msg.time;
					pre.szMessage = e->event.msg.message;
					ProtoChainRecvMsg(getcontact(e->event.msg.sender, 1, 0, nullptr), &pre);
				}

				// RichEdit format included (image)
				if (e->event.msg.formats_length &&
					getByte(GG_KEY_IMGRECEIVE, GG_KEYDEF_IMGRECEIVE) &&
					!(db_get_dw(getcontact(e->event.msg.sender, 1, 0, nullptr), "Ignore", "Mask1", 0) & IGNOREEVENT_MESSAGE))
				{
					char *formats = (char*)e->event.msg.formats;
					int len = 0, formats_len = e->event.msg.formats_length, add_ptr;

					while (len < formats_len)
					{
						add_ptr = sizeof(struct gg_msg_richtext_format);
						if (((struct gg_msg_richtext_format*)formats)->font & GG_FONT_IMAGE) {
							struct gg_msg_richtext_image *image = (struct gg_msg_richtext_image *)(formats + add_ptr);
							gg_EnterCriticalSection(&sess_mutex, "mainthread", 18, "sess_mutex", 1);
							gg_image_request(m_sess, e->event.msg.sender, image->size, image->crc32);
							gg_LeaveCriticalSection(&sess_mutex, "mainthread", 18, 1, "sess_mutex", 1);

							debugLogA("mainthread(): image request sent!");
							add_ptr += sizeof(struct gg_msg_richtext_image);
						}
						if (((struct gg_msg_richtext_format*)formats)->font & GG_FONT_COLOR)
							add_ptr += sizeof(struct gg_msg_richtext_color);
						len += add_ptr;
						formats += add_ptr;
					}
				}
			}
			break;

			// Message sent from concurrent user session
		case GG_EVENT_MULTILOGON_MSG:
			if (e->event.multilogon_msg.recipients_count && gc_enabled && !getByte(GG_KEY_IGNORECONF, GG_KEYDEF_IGNORECONF))
			{
				wchar_t *chat = gc_getchat(e->event.multilogon_msg.sender, e->event.multilogon_msg.recipients, e->event.multilogon_msg.recipients_count);
				if (chat)
				{
					wchar_t id[32];
					UIN2IDT(getDword(GG_KEY_UIN, 0), id);

					GCEVENT gce = { m_szModuleName, 0, GC_EVENT_MESSAGE };
					gce.pszID.w = chat;
					gce.pszUID.w = id;
					wchar_t* messageT = mir_utf8decodeW(e->event.multilogon_msg.message);
					gce.pszText.w = messageT;
					wchar_t* nickT;
					if (!getWString(GG_KEY_NICK, &dbv)) {
						nickT = mir_wstrdup(dbv.pwszVal);
						db_free(&dbv);
					}
					else
						nickT = mir_wstrdup(TranslateT("Me"));

					gce.pszNick.w = nickT;
					gce.time = e->event.multilogon_msg.time;
					gce.bIsMe = 1;
					gce.dwFlags = GCEF_ADDTOLOG;
					debugLogW(L"mainthread() (%x): Sent conference message to room %s.", this, chat);
					Chat_Event(&gce);
					mir_free(messageT);
					mir_free(nickT);
				}
			}
			else if (!e->event.multilogon_msg.recipients_count && e->event.multilogon_msg.message && *e->event.multilogon_msg.message
				&& mir_strcmp(e->event.multilogon_msg.message, "\xA0\0"))
			{
				DBEVENTINFO dbei = {};
				dbei.szModule = m_szModuleName;
				dbei.timestamp = (uint32_t)e->event.multilogon_msg.time;
				dbei.flags = DBEF_SENT | DBEF_UTF;
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.cbBlob = (uint32_t)mir_strlen(e->event.multilogon_msg.message) + 1;
				dbei.pBlob = (uint8_t*)e->event.multilogon_msg.message;
				db_event_add(getcontact(e->event.multilogon_msg.sender, 1, 0, nullptr), &dbei);
			}
			break;

			// Information on active concurrent sessions
		case GG_EVENT_MULTILOGON_INFO:
		{
			list_t l;
			int* iIndexes = nullptr, i;
			debugLogA("mainthread(): Concurrent sessions count: %d.", e->event.multilogon_info.count);
			if (e->event.multilogon_info.count > 0)
				iIndexes = (int*)mir_calloc(e->event.multilogon_info.count * sizeof(int));
			gg_EnterCriticalSection(&sessions_mutex, "mainthread", 19, "sess_mutex", 1);
			for (l = sessions; l; l = l->next)
			{
				struct gg_multilogon_session* msess = (struct gg_multilogon_session*)l->data;
				for (i = 0; i < e->event.multilogon_info.count; i++)
				{
					if (!memcmp(&msess->id, &e->event.multilogon_info.sessions[i].id, sizeof(gg_multilogon_id_t)) && iIndexes)
					{
						iIndexes[i]++;
						break;
					}
				}
				mir_free(msess->name);
				mir_free(msess);
			}
			list_destroy(sessions, 0);
			sessions = nullptr;
			for (i = 0; i < e->event.multilogon_info.count; i++)
			{
				gg_multilogon_session* msess = (gg_multilogon_session*)mir_alloc(sizeof(struct gg_multilogon_session));
				memcpy(msess, &e->event.multilogon_info.sessions[i], sizeof(struct gg_multilogon_session));
				msess->name = mir_strdup(*e->event.multilogon_info.sessions[i].name != '\0'
					? e->event.multilogon_info.sessions[i].name
					: Translate("Unknown client"));
				list_add(&sessions, msess, 0);
			}
			gg_LeaveCriticalSection(&sessions_mutex, "mainthread", 19, 1, "sessions_mutex", 1);
			sessions_updatedlg();

			const wchar_t* szText = time(0) - logonTime > 3
				? TranslateT("You have logged in at another location")
				: TranslateT("You are logged in at another location");
			for (i = 0; i < e->event.multilogon_info.count; i++) {
				wchar_t szMsg[MAX_SECONDLINE];
				if (iIndexes && iIndexes[i])
					continue;

				mir_snwprintf(szMsg, L"%s (%s)", szText,
					*e->event.multilogon_info.sessions[i].name != '\0' ?
					_A2T(e->event.multilogon_info.sessions[i].name).get() : TranslateT("Unknown client"));
				showpopup(m_tszUserName, szMsg, GG_POPUP_MULTILOGON);
			}
			mir_free(iIndexes);
		}
		break;

		// Image reply sent
		case GG_EVENT_IMAGE_REPLY:
			// Get rid of empty image
			if (e->event.image_reply.size && e->event.image_reply.image)
			{
				MCONTACT hContact = getcontact(e->event.image_reply.sender, 1, 0, nullptr);
				void *img = (void *)img_loadpicture(e, nullptr);

				if (!img)
					break;

				if (getByte(GG_KEY_IMGMETHOD, GG_KEYDEF_IMGMETHOD) == 1 || img_opened(e->event.image_reply.sender))
				{
					img_display(hContact, img);
				}
				else if (getByte(GG_KEY_IMGMETHOD, GG_KEYDEF_IMGMETHOD) == 2)
				{
					img_displayasmsg(hContact, img);
				}
				else
				{
					char service[128];
					mir_snprintf(service, GGS_RECVIMAGE, m_szModuleName);

					CLISTEVENT cle = { 0 };
					cle.hContact = hContact;
					cle.hIcon = g_plugin.getIcon(IDI_IMAGE);
					cle.flags = CLEF_URGENT;
					cle.hDbEvent = -98;
					cle.lParam = (LPARAM)img;
					cle.pszService = service;
					cle.szTooltip.a = Translate("Incoming image");
					g_clistApi.pfnAddEvent(&cle);
					g_plugin.releaseIcon(IDI_IMAGE);
				}
			}
			break;

			// Image send request
		case GG_EVENT_IMAGE_REQUEST:
			img_sendonrequest(e);
			break;

			// Incoming direct connection
		case GG_EVENT_DCC7_NEW:
		{
			struct gg_dcc7 *dcc7 = e->event.dcc7_new;
			debugLogA("mainthread() (%x): Incoming direct connection.", this);
			dcc7->contact = (void*)getcontact(dcc7->peer_uin, 0, 0, nullptr);

			// Check if user is on the list and if it is my uin
			if (!dcc7->contact || getDword(GG_KEY_UIN, -1) != dcc7->uin) {
				gg_dcc7_free(dcc7);
				e->event.dcc7_new = nullptr;
				break;
			}

			// Add to waiting transfers
			gg_EnterCriticalSection(&ft_mutex, "mainthread", 20, "ft_mutex", 1);
			list_add(&transfers, dcc7, 0);
			gg_LeaveCriticalSection(&ft_mutex, "mainthread", 20, 1, "ft_mutex", 1);

			//////////////////////////////////////////////////
			// Add file recv request

			debugLogA("mainthread() (%x): Client: %d, File ack filename \"%s\" size %d.", this, dcc7->peer_uin,
				dcc7->filename, dcc7->size);

			const char *fileName = (const char*)dcc7->filename;

			PROTORECVFILE pre = { 0 };
			pre.fileCount = 1;
			pre.timestamp = time(0);
			pre.descr.a = fileName;
			pre.files.a = &fileName;
			pre.lParam = (LPARAM)dcc7;
			ProtoChainRecvFile((UINT_PTR)dcc7->contact, &pre);

			e->event.dcc7_new = nullptr;
		}
		break;

		// Direct connection rejected
		case GG_EVENT_DCC7_REJECT:
		{
			struct gg_dcc7 *dcc7 = e->event.dcc7_reject.dcc7;
			if (dcc7->type == GG_SESSION_DCC7_SEND)
			{
				debugLogA("mainthread() (%x): File transfer denied by client %d (reason = %d).", this, dcc7->peer_uin, e->event.dcc7_reject.reason);
				ProtoBroadcastAck((UINT_PTR)dcc7->contact, ACKTYPE_FILE, ACKRESULT_DENIED, dcc7);

				// Remove from watches and free
				gg_EnterCriticalSection(&ft_mutex, "mainthread", 21, "ft_mutex", 1);
				list_remove(&watches, dcc7, 0);
				gg_LeaveCriticalSection(&ft_mutex, "mainthread", 21, 1, "ft_mutex", 1);
				gg_dcc7_free(dcc7);
			}
			else
			{
				debugLogA("mainthread() (%x): File transfer aborted by client %d.", this, dcc7->peer_uin);

				// Remove transfer from waiting list
				gg_EnterCriticalSection(&ft_mutex, "mainthread", 22, "ft_mutex", 1);
				list_remove(&transfers, dcc7, 0);
				gg_LeaveCriticalSection(&ft_mutex, "mainthread", 22, 1, "ft_mutex", 1);
			}
		}
		break;

		// Direct connection error
		case GG_EVENT_DCC7_ERROR:
		{
			struct gg_dcc7 *dcc7 = e->event.dcc7_error_ex.dcc7;
			switch (e->event.dcc7_error) {
			case GG_ERROR_DCC7_HANDSHAKE:
				debugLogA("mainthread() (%x): Client: %d, Handshake error.", this, dcc7 ? dcc7->peer_uin : 0);
				break;
			case GG_ERROR_DCC7_NET:
				debugLogA("mainthread() (%x): Client: %d, Network error.", this, dcc7 ? dcc7->peer_uin : 0);
				break;
			case GG_ERROR_DCC7_FILE:
				debugLogA("mainthread() (%x): Client: %d, File read/write error.", this, dcc7 ? dcc7->peer_uin : 0);
				break;
			case GG_ERROR_DCC7_EOF:
				debugLogA("mainthread() (%x): Client: %d, End of file/connection error.", this, dcc7 ? dcc7->peer_uin : 0);
				break;
			case GG_ERROR_DCC7_REFUSED:
				debugLogA("mainthread() (%x): Client: %d, Connection refused error.", this, dcc7 ? dcc7->peer_uin : 0);
				break;
			case GG_ERROR_DCC7_RELAY:
				debugLogA("mainthread() (%x): Client: %d, Relay connection error.", this, dcc7 ? dcc7->peer_uin : 0);
				break;
			default:
				debugLogA("mainthread() (%x): Client: %d, Unknown error.", this, dcc7 ? dcc7->peer_uin : 0);
			}

			if (!dcc7)
				break;

			// Remove from watches
			list_remove(&watches, dcc7, 0);

			// Close file & fail
			if (dcc7->file_fd != -1) {
				_close(dcc7->file_fd);
				dcc7->file_fd = -1;
			}

			if (dcc7->contact)
				ProtoBroadcastAck((UINT_PTR)dcc7->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc7);

			// Free dcc
			gg_dcc7_free(dcc7);
		}
		break;

		case GG_EVENT_TYPING_NOTIFICATION:
		{
			MCONTACT hContact = getcontact(e->event.typing_notification.uin, 0, 0, nullptr);
#ifdef DEBUGMODE
			debugLogA("mainthread() (%x): Typing notification from %d (%d).", this,
				e->event.typing_notification.uin, e->event.typing_notification.length);
#endif
			CallService(MS_PROTO_CONTACTISTYPING, hContact,
				e->event.typing_notification.length > 0 ? 7 : PROTOTYPE_CONTACTTYPING_OFF);
		}
		break;
		}
		// Free event struct
		gg_free_event(e);
	}

	broadcastnewstatus(ID_STATUS_OFFLINE);
	setalloffline();
	setDword(GG_KEY_LOGONTIME, 0);

	// If it was unwanted disconnection reconnect
	if (m_iDesiredStatus != ID_STATUS_OFFLINE
		&& m_gaduOptions.autoRecconect)
	{
		debugLogA("mainthread() (%x): Unintentional disconnection detected. Going to reconnect...", this);
		hostnum = 0;
		broadcastnewstatus(ID_STATUS_CONNECTING);
		mir_free(p.status_descr);
		goto retry;
	}

	mir_free(p.password);
	mir_free(p.status_descr);

	// Destroy concurrent sessions list
	{
		list_t l;
		gg_EnterCriticalSection(&sessions_mutex, "mainthread", 23, "sessions_mutex", 1);
		for (l = sessions; l; l = l->next)
		{
			struct gg_multilogon_session* msess = (struct gg_multilogon_session*)l->data;
			mir_free(msess->name);
			mir_free(msess);
		}
		list_destroy(sessions, 0);
		sessions = nullptr;
		gg_LeaveCriticalSection(&sessions_mutex, "mainthread", 23, 1, "sessions_mutex", 1);
	}

	// Stop dcc server
	pth_dcc.dwThreadId = 0;
#ifdef DEBUGMODE
	debugLogA("mainthread() (%x): Waiting pth_dcc thread. Waiting until DCC Server Thread finished, if needed.", this);
#endif
	threadwait(&pth_dcc);
#ifdef DEBUGMODE
	debugLogA("mainthread() (%x): Waiting pth_dcc thread - OK", this);
#endif

	debugLogA("mainthread(): end. (%x) Server Thread Ending", this);
	return;
}

////////////////////////////////////////////////////////////
// Change status function
void GaduProto::broadcastnewstatus(int newStatus)
{
	gg_EnterCriticalSection(&modemsg_mutex, "broadcastnewstatus", 24, "modemsg_mutex", 1);
	int oldStatus = m_iStatus;
	if (oldStatus == newStatus)
	{
		gg_LeaveCriticalSection(&modemsg_mutex, "broadcastnewstatus", 24, 1, "modemsg_mutex", 1);
		return;
	}
	m_iStatus = newStatus;
	gg_LeaveCriticalSection(&modemsg_mutex, "broadcastnewstatus", 24, 2, "modemsg_mutex", 1);

	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, newStatus);

	debugLogA("broadcastnewstatus(): Broadcast new status: %d.", newStatus);
}

////////////////////////////////////////////////////////////
// When contact is deleted

void GaduProto::OnContactDeleted(MCONTACT hContact)
{
	uin_t uin = (uin_t)getDword(hContact, GG_KEY_UIN, 0);

	// Terminate conference if contact is deleted
	ptrW wszRoomId(getWStringA(hContact, "ChatRoomID"));
	if (isChatRoom(hContact) && wszRoomId != NULL && gc_enabled)
	{
		GGGC *chat = gc_lookup(wszRoomId);
		debugLogA("contactdeleted(): Terminating chat %x, id %s from contact list...", chat, wszRoomId.get());
		if (chat)
		{
			// Destroy chat entry
			free(chat->recipients);
			list_remove(&chats, chat, 1);
			// Terminate chat window / shouldn't cascade entry is deleted
			Chat_Control(m_szModuleName, wszRoomId, SESSION_OFFLINE);
			Chat_Terminate(m_szModuleName, wszRoomId);
		}
		return;
	}

	if (uin && isonline())
	{
		gg_EnterCriticalSection(&sess_mutex, "contactdeleted", 25, "sess_mutex", 1);
		gg_remove_notify_ex(m_sess, uin, GG_USER_NORMAL);
		gg_LeaveCriticalSection(&sess_mutex, "contactdeleted", 25, 1, "sess_mutex", 1);
	}
}

////////////////////////////////////////////////////////////
// When db settings changed

static wchar_t* sttSettingToTchar(DBVARIANT* value)
{
	switch (value->type) {
	case DBVT_ASCIIZ:
		return mir_a2u(value->pszVal);
	case DBVT_UTF8:
		return mir_utf8decodeW(value->pszVal);
	case DBVT_WCHAR:
		return mir_wstrdup(value->pwszVal);
	}
	return nullptr;
}

int GaduProto::dbsettingchanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;

#ifdef DEBUGMODE
	debugLogA("dbsettingchanged(): fired. szModule=%s szSetting=%s", cws->szModule, cws->szSetting);
#endif

	// Check if the contact is NULL or we are not online
	if (!isonline())
		return 0;

	// If contact has been blocked
	if (!strcmp(cws->szModule, m_szModuleName) && !mir_strcmp(cws->szSetting, GG_KEY_BLOCK)) {
		notifyuser(hContact, 1);
		return 0;
	}

	// Contact is being renamed
	if (gc_enabled && !strcmp(cws->szModule, m_szModuleName) && !strcmp(cws->szSetting, GG_KEY_NICK)) {
		wchar_t* ptszVal = sttSettingToTchar(&(cws->value));
		if (ptszVal == nullptr)
			return 0;

		// Groupchat window contact is being renamed
		DBVARIANT dbv;
		if (isChatRoom(hContact) && !getWString(hContact, "ChatRoomID", &dbv))
		{
			// Most important... check redundancy (fucking cascading)
			static int cascade = 0;
			if (!cascade && dbv.pwszVal)
			{
				debugLogA("dbsettingchanged(): Conference %s was renamed.", dbv.pszVal);
				// Mark cascading
				/* FIXME */ cascade = 1;
				Chat_ChangeSessionName(m_szModuleName, dbv.pwszVal, ptszVal);
				/* FIXME */ cascade = 0;
			}
			db_free(&dbv);
		}
		else {
			// Change contact name on all chats
			gc_changenick(hContact, ptszVal);
		}
		mir_free(ptszVal);
	}

	// Contact list changes
	if (!strcmp(cws->szModule, "CList"))
	{
		// If name changed... change nick
		if (!strcmp(cws->szSetting, "MyHandle")) {
			wchar_t* ptszVal = sttSettingToTchar(&(cws->value));
			if (ptszVal == nullptr)
				return 0;

			setWString(hContact, GG_KEY_NICK, ptszVal);
			mir_free(ptszVal);
		}

		// If not on list changed
		if (!strcmp(cws->szSetting, "NotOnList"))
		{
			if (Contact::IsHidden(hContact))
				return 0;
			
			// Notify user normally this time if added to the list permanently
			if (cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_BYTE && cws->value.bVal == 0))
				notifyuser(hContact, 1);
		}
	}

#ifdef DEBUGMODE
	debugLogA("dbsettingchanged(): end. szModule=%s szSetting=%s", cws->szModule, cws->szSetting);
#endif

	return 0;
}

////////////////////////////////////////////////////////////
// All users set offline
//
void GaduProto::setalloffline()
{
	debugLogA("setalloffline(): started. Setting buddies offline");
	setWord(GG_KEY_STATUS, ID_STATUS_OFFLINE);

	for (auto &hContact : AccContacts()) {
		setWord(hContact, GG_KEY_STATUS, ID_STATUS_OFFLINE);
		// Clear IP and port settings
		delSetting(hContact, GG_KEY_CLIENTIP);
		delSetting(hContact, GG_KEY_CLIENTPORT);
		// Delete status descr
		db_unset(hContact, "CList", GG_KEY_STATUSDESCR);
	}
#ifdef DEBUGMODE
	debugLogA("setalloffline(): End.");
#endif
}

////////////////////////////////////////////////////////////
// All users set offline
//
void GaduProto::notifyuser(MCONTACT hContact, int refresh)
{
	uin_t uin;
	if (!hContact)
		return;

	if (isonline() && (uin = (uin_t)getDword(hContact, GG_KEY_UIN, 0)))
	{
		// Check if user should be invisible
		// Or be blocked ?
		if ((getWord(hContact, GG_KEY_APPARENT, (uint16_t)ID_STATUS_ONLINE) == ID_STATUS_OFFLINE) || !Contact::OnList(hContact))
		{
			gg_EnterCriticalSection(&sess_mutex, "notifyuser", 77, "sess_mutex", 1);
			if (refresh) {
				gg_remove_notify_ex(m_sess, uin, GG_USER_NORMAL);
				gg_remove_notify_ex(m_sess, uin, GG_USER_BLOCKED);
			}

			gg_add_notify_ex(m_sess, uin, GG_USER_OFFLINE);
			gg_LeaveCriticalSection(&sess_mutex, "notifyuser", 77, 1, "sess_mutex", 1);
		}
		else if (getByte(hContact, GG_KEY_BLOCK, 0))
		{
			gg_EnterCriticalSection(&sess_mutex, "notifyuser", 78, "sess_mutex", 1);
			if (refresh)
				gg_remove_notify_ex(m_sess, uin, GG_USER_OFFLINE);

			gg_add_notify_ex(m_sess, uin, GG_USER_BLOCKED);
			gg_LeaveCriticalSection(&sess_mutex, "notifyuser", 78, 1, "sess_mutex", 1);
		}
		else {
			gg_EnterCriticalSection(&sess_mutex, "notifyuser", 79, "sess_mutex", 1);
			if (refresh)
				gg_remove_notify_ex(m_sess, uin, GG_USER_BLOCKED);

			gg_add_notify_ex(m_sess, uin, GG_USER_NORMAL);
			gg_LeaveCriticalSection(&sess_mutex, "notifyuser", 79, 1, "sess_mutex", 1);
		}
	}
}

void GaduProto::notifyall()
{
	debugLogA("notifyall(): Subscribing notification to all users");
	// Readup count
	int count = 0;
	for (auto &hContact : AccContacts())
		count++;

	// Readup list
	/* FIXME: If we have nothing on the list but we omit gg_notify_ex we have problem with receiving any contacts */
	if (count == 0) {
		if (isonline()) {
			gg_EnterCriticalSection(&sess_mutex, "notifyall", 29, "sess_mutex", 1);
			gg_notify_ex(m_sess, nullptr, nullptr, 0);
			gg_LeaveCriticalSection(&sess_mutex, "notifyall", 29, 1, "sess_mutex", 1);
		}
		return;
	}
	uin_t *uins = (uin_t*)calloc(sizeof(uin_t), count);
	char *types = (char*)calloc(sizeof(char), count);

	int cc = 0;
	for (auto &hContact : AccContacts()) {
		if (uins[cc] = getDword(hContact, GG_KEY_UIN, 0)) {
			if ((getWord(hContact, GG_KEY_APPARENT, (uint16_t)ID_STATUS_ONLINE) == ID_STATUS_OFFLINE) || !Contact::OnList(hContact))
				types[cc] = GG_USER_OFFLINE;
			else if (getByte(hContact, GG_KEY_BLOCK, 0))
				types[cc] = GG_USER_BLOCKED;
			else
				types[cc] = GG_USER_NORMAL;
			++cc;
		}
	}
	if (cc < count)
		count = cc;

	// Send notification
	if (isonline()) {
		gg_EnterCriticalSection(&sess_mutex, "notifyall", 30, "sess_mutex", 1);
		gg_notify_ex(m_sess, uins, types, count);
		gg_LeaveCriticalSection(&sess_mutex, "notifyall", 30, 1, "sess_mutex", 1);
	}

	// Free variables
	free(uins);
	free(types);
}

////////////////////////////////////////////////////////////
// Get contact by uin
//
MCONTACT GaduProto::getcontact(uin_t uin, int create, int inlist, wchar_t *szNick)
{
#ifdef DEBUGMODE
	debugLogA("getcontact(): uin=%d create=%d inlist=%d", uin, create, inlist);
#endif
	// Look for contact in DB
	for (auto &hContact : AccContacts()) {
		if ((uin_t)getDword(hContact, GG_KEY_UIN, 0) == uin && !isChatRoom(hContact)) {
			if (inlist) {
				Contact::PutOnList(hContact);
				Contact::Hide(hContact, false);
			}
			return hContact;
		}
	}
	if (!create)
		return NULL;

	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, m_szModuleName);

	debugLogA("getcontact(): Added buddy: %d", uin);
	if (!inlist)
		Contact::RemoveFromList(hContact);

	setDword(hContact, GG_KEY_UIN, (uint32_t)uin);
	setWord(hContact, GG_KEY_STATUS, ID_STATUS_OFFLINE);

	// If nick specified use it
	if (szNick)
		setWString(hContact, GG_KEY_NICK, szNick);
	else if (isonline()) {
		// Search for that nick
		gg_pubdir50_t req = gg_pubdir50_new(GG_PUBDIR50_SEARCH);
		if (req) {
			// Add uin and search it
			gg_pubdir50_add(req, GG_PUBDIR50_UIN, ditoa(uin));
			gg_pubdir50_seq_set(req, GG_SEQ_GETNICK);
			gg_EnterCriticalSection(&sess_mutex, "getcontact", 31, "sess_mutex", 1);
			gg_pubdir50(m_sess, req);
			gg_LeaveCriticalSection(&sess_mutex, "getcontact", 31, 1, "sess_mutex", 1);
			gg_pubdir50_free(req);
			wchar_t* uinT = mir_a2u(ditoa(uin));
			setWString(hContact, GG_KEY_NICK, uinT);
			mir_free(uinT);
			debugLogA("getcontact(): Search for nick on uin: %d", uin);
		}
	}

	// Add to notify list and pull avatar for the new contact
	if (isonline()) {
		gg_EnterCriticalSection(&sess_mutex, "getcontact", 32, "sess_mutex", 1);
		gg_add_notify_ex(m_sess, uin, (char)(inlist ? GG_USER_NORMAL : GG_USER_OFFLINE));
		gg_LeaveCriticalSection(&sess_mutex, "getcontact", 32, 1, "sess_mutex", 1);

		PROTO_AVATAR_INFORMATION ai = { 0 };
		ai.hContact = hContact;
		getavatarinfo((WPARAM)GAIF_FORCE, (LPARAM)&ai);

		// Change status of the contact with our own UIN (if got yourself added to the contact list)
		if (getDword(GG_KEY_UIN, 0) == uin) {
			gg_EnterCriticalSection(&modemsg_mutex, "getcontact", 33, "modemsg_mutex", 1);
			wchar_t *szMsg = mir_wstrdup(getstatusmsg(m_iStatus));
			gg_LeaveCriticalSection(&modemsg_mutex, "getcontact", 33, 1, "modemsg_mutex", 1);
			changecontactstatus(uin, status_m2gg(m_iStatus, szMsg != nullptr), szMsg, 0, 0, 0, 0);
			mir_free(szMsg);
		}
	}

	// TODO server side list & add buddy
	return hContact;
}

////////////////////////////////////////////////////////////
// Status conversion
//
int GaduProto::status_m2gg(int status, int descr)
{
	// check frends only
	int mask = getByte(GG_KEY_FRIENDSONLY, GG_KEYDEF_FRIENDSONLY) ? GG_STATUS_FRIENDS_MASK : 0;

	if (descr) {
		switch (status) {
		case ID_STATUS_OFFLINE:   return GG_STATUS_NOT_AVAIL_DESCR | mask;
		case ID_STATUS_ONLINE:    return GG_STATUS_AVAIL_DESCR | mask;
		case ID_STATUS_AWAY:      return GG_STATUS_BUSY_DESCR | mask;
		case ID_STATUS_DND:       return GG_STATUS_DND_DESCR | mask;
		case ID_STATUS_FREECHAT:  return GG_STATUS_FFC_DESCR | mask;
		case ID_STATUS_INVISIBLE: return GG_STATUS_INVISIBLE_DESCR | mask;
		default:                  return GG_STATUS_BUSY_DESCR | mask;
		}
	}
	else {
		switch (status) {
		case ID_STATUS_OFFLINE:   return GG_STATUS_NOT_AVAIL | mask;
		case ID_STATUS_ONLINE:    return GG_STATUS_AVAIL | mask;
		case ID_STATUS_AWAY:      return GG_STATUS_BUSY | mask;
		case ID_STATUS_DND:       return GG_STATUS_DND | mask;
		case ID_STATUS_FREECHAT:  return GG_STATUS_FFC | mask;
		case ID_STATUS_INVISIBLE: return GG_STATUS_INVISIBLE | mask;
		default:                  return GG_STATUS_BUSY | mask;
		}
	}
}

int GaduProto::status_gg2m(int status)
{
	// ignore additional flags
	status = GG_S(status);

	// when user has status description but is offline (show it invisible)
	if (status == GG_STATUS_NOT_AVAIL_DESCR && getByte(GG_KEY_SHOWINVISIBLE, GG_KEYDEF_SHOWINVISIBLE))
		return ID_STATUS_INVISIBLE;

	// rest of cases
	switch (status) {
	case GG_STATUS_NOT_AVAIL:
	case GG_STATUS_NOT_AVAIL_DESCR:
		return ID_STATUS_OFFLINE;

	case GG_STATUS_AVAIL:
	case GG_STATUS_AVAIL_DESCR:
		return ID_STATUS_ONLINE;

	case GG_STATUS_BUSY:
	case GG_STATUS_BUSY_DESCR:
		return ID_STATUS_AWAY;

	case GG_STATUS_DND:
	case GG_STATUS_DND_DESCR:
		return ID_STATUS_DND;
		
	case GG_STATUS_FFC:
	case GG_STATUS_FFC_DESCR:
		return ID_STATUS_FREECHAT;

	case GG_STATUS_INVISIBLE:
	case GG_STATUS_INVISIBLE_DESCR:
		return ID_STATUS_INVISIBLE;

	case GG_STATUS_BLOCKED:
		return ID_STATUS_NA;

	default:
		return ID_STATUS_OFFLINE;
	}
}

////////////////////////////////////////////////////////////
// Called when contact status is changed
//
void GaduProto::changecontactstatus(uin_t uin, int status, const wchar_t *idescr, int, uint32_t remote_ip, uint16_t remote_port, uint32_t version)
{
#ifdef DEBUGMODE
	debugLogA("changecontactstatus(): uin=%d status=%d", uin, status);
#endif
	MCONTACT hContact = getcontact(uin, 0, 0, nullptr);

	// Check if contact is on list
	if (!hContact)
		return;

	// Write contact status
	setWord(hContact, GG_KEY_STATUS, (uint16_t)status_gg2m(status));

	// Check if there's description and if it's not empty
	if (idescr && *idescr) {
		debugLogW(L"changecontactstatus(): Saving for %d status descr \"%s\".", uin, idescr);
		db_set_ws(hContact, "CList", GG_KEY_STATUSDESCR, idescr);
	}
	else {
		// Remove status if there's nothing
		db_unset(hContact, "CList", GG_KEY_STATUSDESCR);
	}

	// Store contact ip and port
	if (remote_ip)
		setDword(hContact, GG_KEY_CLIENTIP, (uint32_t)swap32(remote_ip));
	if (remote_port)
		setWord(hContact, GG_KEY_CLIENTPORT, (uint16_t)remote_port);
	if (version) {
		char sversion[48];
		setDword(hContact, GG_KEY_CLIENTVERSION, (uint32_t)version);
		mir_snprintf(sversion, "%sGadu-Gadu %s", (version & 0x00ffffff) > 0x2b ? "Nowe " : "", gg_version2string(version));
		setString(hContact, "MirVer", sversion);
	}
}

////////////////////////////////////////////////////////////
// Returns GG client version string from packet version
const char *gg_version2string(int v)
{
	const char *pstr = "???";
	v &= 0x00ffffff;
	switch (v) {
	case 0x2e:
		pstr = "8.0 build 8283"; break;
	case 0x2d:
		pstr = "8.0 build 4881"; break;
	case 0x2b:
		pstr = "< 8.0"; break;
	case 0x2a:
		pstr = "7.7 build 3315"; break;
	case 0x29:
		pstr = "7.6 build 1688"; break;
	case 0x28:
		pstr = "7.5 build 2201"; break;
	case 0x27:
		pstr = "7.0 build 22"; break;
	case 0x26:
		pstr = "7.0 build 20"; break;
	case 0x25:
		pstr = "7.0 build 1"; break;
	case 0x24:
		pstr = "6.1 (155) / 7.6 (1359)"; break;
	case 0x22:
		pstr = "6.0 build 140"; break;
	case 0x21:
		pstr = "6.0 build 133"; break;
	case 0x20:
		pstr = "6.0b"; break;
	case 0x1e:
		pstr = "5.7b build 121"; break;
	case 0x1c:
		pstr = "5.7b"; break;
	case 0x1b:
		pstr = "5.0.5"; break;
	case 0x19:
		pstr = "5.0.3"; break;
	case 0x18:
		pstr = "5.0.0-1"; break;
	case 0x17:
		pstr = "4.9.2"; break;
	case 0x16:
		pstr = "4.9.1"; break;
	case 0x15:
		pstr = "4.8.9"; break;
	case 0x14:
		pstr = "4.8.1-3"; break;
	case 0x11:
		pstr = "4.6.1-10"; break;
	case 0x10:
		pstr = "4.5.15-22"; break;
	case 0x0f:
		pstr = "4.5.12"; break;
	case 0x0b:
		pstr = "4.0.25-30"; break;
	default:
		if (v < 0x0b)
			pstr = "< 4.0.25";
		else if (v > 0x2e)
			pstr = ">= 8.0";
		break;
	}
	return pstr;
}
