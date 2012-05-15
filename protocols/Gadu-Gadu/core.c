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
#include <errno.h>
#include <io.h>

////////////////////////////////////////////////////////////
// Swap bits in DWORD
uint32_t swap32(uint32_t x)
{
	return (uint32_t)
		(((x & (uint32_t) 0x000000ffU) << 24) |
				((x & (uint32_t) 0x0000ff00U) << 8) |
				((x & (uint32_t) 0x00ff0000U) >> 8) |
				((x & (uint32_t) 0xff000000U) >> 24));
}

////////////////////////////////////////////////////////////
// Is online function
GGINLINE int gg_isonline(GGPROTO *gg)
{
	int isonline;
	EnterCriticalSection(&gg->sess_mutex);
	isonline = (gg->sess != NULL);
	LeaveCriticalSection(&gg->sess_mutex);

	return isonline;
}

////////////////////////////////////////////////////////////
// Send disconnect request and wait for server thread to die
void gg_disconnect(GGPROTO *gg)
{
	// If main loop then send disconnect request
	if (gg_isonline(gg))
	{
		// Fetch proper status msg
		char *szMsg = NULL;

		// Loadup status
		if (DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_LEAVESTATUSMSG, GG_KEYDEF_LEAVESTATUSMSG))
		{
			DBVARIANT dbv;
			switch (DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_LEAVESTATUS, GG_KEYDEF_LEAVESTATUS))
			{
				case ID_STATUS_ONLINE:
					EnterCriticalSection(&gg->modemsg_mutex);
					szMsg = mir_strdup(gg->modemsg.online);
					LeaveCriticalSection(&gg->modemsg_mutex);
					if (!szMsg &&
						!DBGetContactSettingString(NULL, "SRAway", gg_status2db(ID_STATUS_ONLINE, "Default"), &dbv))
					{
						if (dbv.pszVal && *(dbv.pszVal))
							szMsg = mir_strdup(dbv.pszVal);
						DBFreeVariant(&dbv);
					}
					break;
				case ID_STATUS_AWAY:
					EnterCriticalSection(&gg->modemsg_mutex);
					szMsg = mir_strdup(gg->modemsg.away);
					LeaveCriticalSection(&gg->modemsg_mutex);
					if (!szMsg &&
						!DBGetContactSettingString(NULL, "SRAway", gg_status2db(ID_STATUS_AWAY, "Default"), &dbv))
					{
						if (dbv.pszVal && *(dbv.pszVal))
							szMsg = mir_strdup(dbv.pszVal);
						DBFreeVariant(&dbv);
					}
					break;
				case ID_STATUS_DND:
					EnterCriticalSection(&gg->modemsg_mutex);
					szMsg = mir_strdup(gg->modemsg.dnd);
					LeaveCriticalSection(&gg->modemsg_mutex);
					if (!szMsg &&
						!DBGetContactSettingString(NULL, "SRAway", gg_status2db(ID_STATUS_DND, "Default"), &dbv))
					{
						if (dbv.pszVal && *(dbv.pszVal))
							szMsg = mir_strdup(dbv.pszVal);
						DBFreeVariant(&dbv);
					}
					break;
				case ID_STATUS_FREECHAT:
					EnterCriticalSection(&gg->modemsg_mutex);
					szMsg = mir_strdup(gg->modemsg.freechat);
					LeaveCriticalSection(&gg->modemsg_mutex);
					if (!szMsg &&
						!DBGetContactSettingString(NULL, "SRAway", gg_status2db(ID_STATUS_FREECHAT, "Default"), &dbv))
					{
						if (dbv.pszVal && *(dbv.pszVal))
							szMsg = mir_strdup(dbv.pszVal);
						DBFreeVariant(&dbv);
					}
					break;
				case ID_STATUS_INVISIBLE:
					EnterCriticalSection(&gg->modemsg_mutex);
					szMsg = mir_strdup(gg->modemsg.invisible);
					LeaveCriticalSection(&gg->modemsg_mutex);
					if (!szMsg &&
						!DBGetContactSettingString(NULL, "SRAway", gg_status2db(ID_STATUS_INVISIBLE, "Default"), &dbv))
					{
						if (dbv.pszVal && *(dbv.pszVal))
							szMsg = mir_strdup(dbv.pszVal);
						DBFreeVariant(&dbv);
					}
					break;
				default:
					// Set last status
					EnterCriticalSection(&gg->modemsg_mutex);
					szMsg = mir_strdup(gg_getstatusmsg(gg, gg->proto.m_iStatus));
					LeaveCriticalSection(&gg->modemsg_mutex);
			}
		}

		EnterCriticalSection(&gg->sess_mutex);
		// Check if it has message
		if (szMsg)
		{
			gg_change_status_descr(gg->sess, GG_STATUS_NOT_AVAIL_DESCR, szMsg);
			mir_free(szMsg);
			// Wait for disconnection acknowledge
		}
		else
		{
			gg_change_status(gg->sess, GG_STATUS_NOT_AVAIL);
			// Send logoff immediately
			gg_logoff(gg->sess);
		}
		LeaveCriticalSection(&gg->sess_mutex);
	}
	// Else cancel connection attempt
	else if (gg->sock)
		closesocket(gg->sock);
}

////////////////////////////////////////////////////////////
// DNS lookup function
uint32_t gg_dnslookup(GGPROTO *gg, char *host)
{
	uint32_t ip;
	struct hostent *he;

	ip = inet_addr(host);
	if(ip != INADDR_NONE)
	{
#ifdef DEBUGMODE
		gg_netlog(gg, "gg_dnslookup(): Parameter \"%s\" is already IP number.", host);
#endif
		return ip;
	}
	he = gethostbyname(host);
	if(he)
	{
		ip = *(uint32_t *) he->h_addr_list[0];
#ifdef DEBUGMODE
		gg_netlog(gg, "gg_dnslookup(): Parameter \"%s\" was resolved to %d.%d.%d.%d.", host,
			LOBYTE(LOWORD(ip)), HIBYTE(LOWORD(ip)), LOBYTE(HIWORD(ip)), HIBYTE(HIWORD(ip)));
#endif
		return ip;
	}
	gg_netlog(gg, "gg_dnslookup(): Cannot resolve hostname \"%s\".", host);
	return 0;
}

////////////////////////////////////////////////////////////
// Host list decoder
typedef struct
{
	char hostname[128];
	int port;
} GGHOST;
#define ISHOSTALPHA(a) (((a) >= '0' && (a) <= '9') || ((a) >= 'a' && (a) <= 'z') || (a) == '.' || (a) == '-')
int gg_decodehosts(char *var, GGHOST *hosts, int max)
{
	int hp = 0;
	char *hostname = NULL;
	char *portname = NULL;

	while(var && *var && hp < max)
	{
		if(ISHOSTALPHA(*var))
		{
			hostname = var;

			while(var && *var && ISHOSTALPHA(*var)) var ++;

			if(var && *var == ':' && var++ && *var && isdigit(*var))
			{
				*(var - 1) = 0;
				portname = var;
				while(var && *var && isdigit(*var)) var++;
				if(*var) { *var = 0; var ++; }
			}
			else
				if(*var) { *var = 0; var ++; }

			// Insert new item
			hosts[hp].hostname[127] = 0;
			strncpy(hosts[hp].hostname, hostname, 127);
			hosts[hp].port = portname ? atoi(portname) : 443;
			hp ++;

			// Zero the names
			hostname = NULL;
			portname = NULL;
		}
		else
			var ++;
	}
	return hp;
}

////////////////////////////////////////////////////////////
// Main connection session thread
void __cdecl gg_mainthread(GGPROTO *gg, void *empty)
{
	// Miranda variables
	NETLIBUSERSETTINGS nlus = {0};
	DBVARIANT dbv;
	// Gadu-Gadu variables
	struct gg_login_params p = {0};
	struct gg_event *e;
	struct gg_session *sess;
	// Host cycling variables
	int hostnum = 0, hostcount = 0;
	GGHOST hosts[64];
	// Gadu-gadu login errors
	static const struct tagReason { int type; char *str; } reason[] = {
		{ GG_FAILURE_RESOLVING, 	"Miranda was unable to resolve the name of the Gadu-Gadu server to its numeric address." },
		{ GG_FAILURE_CONNECTING,	"Miranda was unable to make a connection with a server. It is likely that the server is down, in which case you should wait for a while and try again later." },
		{ GG_FAILURE_INVALID,		"Received invalid server response." },
		{ GG_FAILURE_READING,		"The connection with the server was abortively closed during the connection attempt. You may have lost your local network connection." },
		{ GG_FAILURE_WRITING,		"The connection with the server was abortively closed during the connection attempt. You may have lost your local network connection." },
		{ GG_FAILURE_PASSWORD,		"Your Gadu-Gadu number and password combination was rejected by the Gadu-Gadu server. Please check login details at M->Options->Network->Gadu-Gadu and try again." },
		{ GG_FAILURE_404,			"Connecting to Gadu-Gadu hub failed." },
		{ GG_FAILURE_TLS,			"Cannot establish secure connection." },
		{ GG_FAILURE_NEED_EMAIL,	"Server disconnected asking you for changing your e-mail." },
		{ GG_FAILURE_INTRUDER,		"Too many login attempts with invalid password." },
		{ GG_FAILURE_UNAVAILABLE,	"Gadu-Gadu servers are now down. Try again later." },
		{ 0,						"Unknown" }
	};
	time_t logonTime = 0;
	time_t timeDeviation = DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_TIMEDEVIATION, GG_KEYDEF_TIMEDEVIATION);
	int gg_failno = 0;

	gg_netlog(gg, "gg_mainthread(%x): Server Thread Starting", gg);
#ifdef DEBUGMODE
	gg_debug_level = GG_DEBUG_NET | GG_DEBUG_TRAFFIC | GG_DEBUG_FUNCTION | GG_DEBUG_MISC;
#else
	gg_debug_level = 0;
#endif

	// Broadcast that service is connecting
	gg_broadcastnewstatus(gg, ID_STATUS_CONNECTING);

	// Client version and misc settings
	p.client_version = GG_DEFAULT_CLIENT_VERSION;
	p.protocol_version = GG_DEFAULT_PROTOCOL_VERSION;
	p.protocol_features = GG_FEATURE_DND_FFC | GG_FEATURE_UNKNOWN_100 | GG_FEATURE_USER_DATA | GG_FEATURE_MSG_ACK | GG_FEATURE_TYPING_NOTIFICATION | GG_FEATURE_MULTILOGON;
	p.encoding = GG_ENCODING_CP1250;
	p.status_flags = GG_STATUS_FLAG_UNKNOWN;
	if (DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_SHOWLINKS, GG_KEYDEF_SHOWLINKS))
		p.status_flags |= GG_STATUS_FLAG_SPAM;

	// Use audio
	/* p.has_audio = 1; */

	// Use async connections
	/* p.async = 1; */

	// Send Era Omnix info if set
	p.era_omnix = DBGetContactSettingByte(NULL, GG_PROTO, "EraOmnix", 0);

	// Setup proxy
	nlus.cbSize = sizeof(nlus);
	if(CallService(MS_NETLIB_GETUSERSETTINGS, (WPARAM)gg->netlib, (LPARAM)&nlus))
	{
		if(nlus.useProxy)
			gg_netlog(gg, "gg_mainthread(%x): Using proxy %s:%d.", gg, nlus.szProxyServer, nlus.wProxyPort);
		gg_proxy_enabled = nlus.useProxy;
		gg_proxy_host = nlus.szProxyServer;
		gg_proxy_port = nlus.wProxyPort;
		if(nlus.useProxyAuth)
		{
			gg_proxy_username = nlus.szProxyAuthUser;
			gg_proxy_password = nlus.szProxyAuthPassword;
		}
		else
			gg_proxy_username = gg_proxy_password = NULL;
	}
	else
	{
		gg_netlog(gg, "gg_mainthread(%x): Failed loading proxy settings.", gg);
		gg_proxy_enabled = 0;
	}

	// Check out manual host setting
	if(DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_MANUALHOST, GG_KEYDEF_MANUALHOST))
	{
		if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_SERVERHOSTS, &dbv))
		{
			hostcount = gg_decodehosts(dbv.pszVal, hosts, 64);
			DBFreeVariant(&dbv);
		}
	}

	// Readup password
	if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_PASSWORD, &dbv))
	{
		CallService(MS_DB_CRYPT_DECODESTRING, strlen(dbv.pszVal) + 1, (LPARAM) dbv.pszVal);
		p.password = mir_strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	else
	{
		gg_netlog(gg, "gg_mainthread(%x): No password specified. Exiting.", gg);
		gg_broadcastnewstatus(gg, ID_STATUS_OFFLINE);
		return;
	}

	// Readup number
	if(!(p.uin = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0)))
	{
		gg_netlog(gg, "gg_mainthread(%x): No Gadu-Gadu number specified. Exiting.", gg);
		gg_broadcastnewstatus(gg, ID_STATUS_OFFLINE);
		mir_free(p.password);
		return;
	}

	// Readup SSL/TLS setting
	if(p.tls = DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_SSLCONN, GG_KEYDEF_SSLCONN))
		gg_netlog(gg, "gg_mainthread(%x): Using TLS/SSL for connections.", gg);

	// Gadu-Gadu accepts image sizes upto 255
	p.image_size = 255;

	////////////////////////////// DCC STARTUP /////////////////////////////
	// Uin is ok so startup dcc if not started already
	if(!gg->dcc)
	{
		gg->event = CreateEvent(NULL, TRUE, FALSE, NULL);
		gg_dccstart(gg);

		// Wait for DCC
#ifdef DEBUGMODE
		gg_netlog(gg, "gg_mainthread(%x): Waiting DCC service to start...", gg);
#endif
		while (WaitForSingleObjectEx(gg->event, INFINITE, TRUE) != WAIT_OBJECT_0);
		CloseHandle(gg->event); gg->event = NULL;
	}
	// Check if dcc is running and setup forwarding port
	if(gg->dcc && DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_FORWARDING, GG_KEYDEF_FORWARDING))
	{
		if(!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_FORWARDHOST, &dbv))
		{
			if(!(p.external_addr = gg_dnslookup(gg, dbv.pszVal)))
			{
				char error[128];
				mir_snprintf(error, sizeof(error), Translate("External direct connections hostname %s is invalid. Disabling external host forwarding."), dbv.pszVal);
				gg_showpopup(gg, GG_PROTONAME, error, GG_POPUP_WARNING | GG_POPUP_ALLOW_MSGBOX);
			}
			else
				gg_netlog(gg, "gg_mainthread(%x): Loading forwarding host %s and port %d.", dbv.pszVal, p.external_port, gg);
			if(p.external_addr)	p.external_port = DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_FORWARDPORT, GG_KEYDEF_FORWARDPORT);
			DBFreeVariant(&dbv);
		}
	}
	// Setup client port
	if(gg->dcc) p.client_port = gg->dcc->port;

retry:
	// Loadup startup status & description
	EnterCriticalSection(&gg->modemsg_mutex);
	p.status_descr = mir_strdup(gg_getstatusmsg(gg, gg->proto.m_iDesiredStatus));
	p.status = status_m2gg(gg, gg->proto.m_iDesiredStatus, p.status_descr != NULL);

	gg_netlog(gg, "gg_mainthread(%x): Connecting with number %d, status %d and description \"%s\".", gg, p.uin, gg->proto.m_iDesiredStatus,
				p.status_descr ? p.status_descr : "<none>");
	LeaveCriticalSection(&gg->modemsg_mutex);

	// Check manual hosts
	if(hostnum < hostcount)
	{
		if(!(p.server_addr = gg_dnslookup(gg, hosts[hostnum].hostname)))
		{
			char error[128];
			mir_snprintf(error, sizeof(error), Translate("Server hostname %s is invalid. Using default hostname provided by the network."), hosts[hostnum].hostname);
			gg_showpopup(gg, GG_PROTONAME, error, GG_POPUP_WARNING | GG_POPUP_ALLOW_MSGBOX);
		}
		else
		{
			p.server_port = hosts[hostnum].port;
			gg_netlog(gg, "gg_mainthread(%x): Connecting to manually specified host %s (%d.%d.%d.%d) and port %d.", gg,
				hosts[hostnum].hostname, LOBYTE(LOWORD(p.server_addr)), HIBYTE(LOWORD(p.server_addr)),
				LOBYTE(HIWORD(p.server_addr)), HIBYTE(HIWORD(p.server_addr)), p.server_port);
		}
	}
	else
		p.server_port = p.server_addr = 0;

	// Send login request
	if (!(sess = gg_login(&p, &gg->sock, &gg_failno)))
	{
		gg_broadcastnewstatus(gg, ID_STATUS_OFFLINE);
		// Check if connection attempt wasn't cancelled by the user
		if (gg->proto.m_iDesiredStatus != ID_STATUS_OFFLINE)
		{
			char error[128], *perror = NULL;
			// Lookup for error desciption
			if (errno == EACCES)
			{
				int i;
				for (i = 0; reason[i].type; i++) if (reason[i].type == gg_failno)
				{
					perror = Translate(reason[i].str);
					break;
				}
			}
			if (!perror)
			{
				mir_snprintf(error, sizeof(error), Translate("Connection cannot be established because of error:\n\t%s"), strerror(errno));
				perror = error;
			}
			gg_netlog(gg, "gg_mainthread(%x): %s", gg, perror);
			if (DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_SHOWCERRORS, GG_KEYDEF_SHOWCERRORS))
				gg_showpopup(gg, GG_PROTONAME, perror, GG_POPUP_ERROR | GG_POPUP_ALLOW_MSGBOX | GG_POPUP_ONCE);

			// Check if we should reconnect
			if ((gg_failno >= GG_FAILURE_RESOLVING && gg_failno != GG_FAILURE_PASSWORD && gg_failno != GG_FAILURE_INTRUDER && gg_failno != GG_FAILURE_UNAVAILABLE)
				&& errno == EACCES
				&& (DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ARECONNECT, GG_KEYDEF_ARECONNECT) || (hostnum < hostcount - 1)))
			{
				DWORD dwInterval = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_RECONNINTERVAL, GG_KEYDEF_RECONNINTERVAL), dwResult;
				BOOL bRetry = TRUE;

				gg->hConnStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				dwResult = WaitForSingleObjectEx(gg->hConnStopEvent, dwInterval, TRUE);
				if ((dwResult == WAIT_OBJECT_0 && gg->proto.m_iDesiredStatus == ID_STATUS_OFFLINE)
					|| (dwResult == WAIT_IO_COMPLETION && Miranda_Terminated()))
					bRetry = FALSE;
				CloseHandle(gg->hConnStopEvent);
				gg->hConnStopEvent = NULL;

				// Reconnect to the next server on the list
				if (bRetry)
				{
					if (hostnum < hostcount - 1) hostnum++;
					mir_free(p.status_descr);
					gg_broadcastnewstatus(gg, ID_STATUS_CONNECTING);
					goto retry;
				}
			}
			// We cannot do more about this
			EnterCriticalSection(&gg->modemsg_mutex);
			gg->proto.m_iDesiredStatus = ID_STATUS_OFFLINE;
			LeaveCriticalSection(&gg->modemsg_mutex);
		}
		else
			gg_netlog(gg, "gg_mainthread(%x)): Connection attempt cancelled by the user.", gg);
	}
	else
	{
		// Successfully connected
		logonTime = time(NULL);
		DBWriteContactSettingDword(NULL, GG_PROTO, GG_KEY_LOGONTIME, logonTime);
		EnterCriticalSection(&gg->sess_mutex);
		gg->sess = sess;
		LeaveCriticalSection(&gg->sess_mutex);
		// Subscribe users status notifications
		gg_notifyall(gg);
		// Set startup status
		if (gg->proto.m_iDesiredStatus != status_gg2m(gg, p.status))
			gg_refreshstatus(gg, gg->proto.m_iDesiredStatus);
		else
		{
			gg_broadcastnewstatus(gg, gg->proto.m_iDesiredStatus);
			// Change status of the contact with our own UIN (if got yourself added to the contact list)
			gg_changecontactstatus(gg, p.uin, p.status, p.status_descr, 0, 0, 0, 0);
		}
		if (gg->check_first_conn) // First connection to the account
		{
			// Start search for user data
			gg_getinfo((PROTO_INTERFACE *)gg, NULL, 0);
			// Fetch user avatar
			gg_getuseravatar(gg);
			gg->check_first_conn = 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////////////
	// Main loop
	while(gg_isonline(gg))
	{
		// Connection broken/closed
		if(!(e = gg_watch_fd(gg->sess)))
		{
			gg_netlog(gg, "gg_mainthread(%x): Connection closed.", gg);
			EnterCriticalSection(&gg->sess_mutex);
			gg_free_session(gg->sess);
			gg->sess = NULL;
			LeaveCriticalSection(&gg->sess_mutex);
			break;
		}
		else
			gg_netlog(gg, "gg_mainthread(%x): Event: %s", gg, ggdebug_eventtype(e));

		switch(e->type)
		{
			// Client connected
			case GG_EVENT_CONN_SUCCESS:
				// Nada
				break;

			// Client disconnected or connection failure
			case GG_EVENT_CONN_FAILED:
			case GG_EVENT_DISCONNECT:
				EnterCriticalSection(&gg->sess_mutex);
				gg_free_session(gg->sess);
				gg->sess = NULL;
				LeaveCriticalSection(&gg->sess_mutex);
				break;

			// Client allowed to disconnect
			case GG_EVENT_DISCONNECT_ACK:
				// Send logoff
				gg_logoff(gg->sess);
				break;

			// Received ackowledge
			case GG_EVENT_ACK:
				if(e->event.ack.seq && e->event.ack.recipient)
				{
					ProtoBroadcastAck(GG_PROTO, gg_getcontact(gg, (DWORD)e->event.ack.recipient, 0, 0, NULL),
						ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE) e->event.ack.seq, 0);
				}
				break;

			// Statuslist notify (deprecated)
			case GG_EVENT_NOTIFY:
			case GG_EVENT_NOTIFY_DESCR:
			{
				struct gg_notify_reply *n;

				n = (e->type == GG_EVENT_NOTIFY) ? e->event.notify : e->event.notify_descr.notify;

				for (; n->uin; n++)
				{
					char *descr = (e->type == GG_EVENT_NOTIFY_DESCR) ? e->event.notify_descr.descr : NULL;
					gg_changecontactstatus(gg, n->uin, n->status, descr, 0, n->remote_ip, n->remote_port, n->version);
				}
				break;
			}
			// Statuslist notify (version >= 6.0)
			case GG_EVENT_NOTIFY60:
			{
				uin_t uin = (uin_t)DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0);
				int i;
				for(i = 0; e->event.notify60[i].uin; i++) {
					if (e->event.notify60[i].uin == uin) continue;
					gg_changecontactstatus(gg, e->event.notify60[i].uin, e->event.notify60[i].status, e->event.notify60[i].descr,
						e->event.notify60[i].time, e->event.notify60[i].remote_ip, e->event.notify60[i].remote_port,
						e->event.notify60[i].version);
					gg_requestavatar(gg, gg_getcontact(gg, e->event.notify60[i].uin, 0, 0, NULL), 0);
				}
				break;
			}

			// Pubdir search reply && read own data reply
			case GG_EVENT_PUBDIR50_SEARCH_REPLY:
			case GG_EVENT_PUBDIR50_READ:
			case GG_EVENT_PUBDIR50_WRITE:
			{
				gg_pubdir50_t res = e->event.pubdir50;
				int i, count;

				if (e->type == GG_EVENT_PUBDIR50_SEARCH_REPLY)
				{
					gg_netlog(gg, "gg_mainthread(%x): Got user info.", gg);
					// Store next search UIN
					if (res->seq == GG_SEQ_SEARCH)
						gg->next_uin = gg_pubdir50_next(res);
				}
				else if (e->type == GG_EVENT_PUBDIR50_READ)
				{
					gg_netlog(gg, "gg_mainthread(%x): Got owner info.", gg);
				}
				else if (e->type == GG_EVENT_PUBDIR50_WRITE)
				{
					gg_netlog(gg, "gg_mainthread(%x): Public directory save succesful.", gg);
					// Update user details
					gg_getinfo((PROTO_INTERFACE *)gg, NULL, 0);
				}

				if ((count = gg_pubdir50_count(res)) > 0)
				{
					for (i = 0; i < count; i++)
					{
						// Loadup fields
						const char *__fmnumber = gg_pubdir50_get(res, i, GG_PUBDIR50_UIN);
						const char *__nick = gg_pubdir50_get(res, i, GG_PUBDIR50_NICKNAME);
						const char *__firstname = gg_pubdir50_get(res, i, GG_PUBDIR50_FIRSTNAME);
						const char *__lastname = gg_pubdir50_get(res, i, GG_PUBDIR50_LASTNAME);
						const char *__familyname = gg_pubdir50_get(res, i, GG_PUBDIR50_FAMILYNAME);
						const char *__birthyear = gg_pubdir50_get(res, i, GG_PUBDIR50_BIRTHYEAR);
						const char *__city = gg_pubdir50_get(res, i, GG_PUBDIR50_CITY);
						const char *__origincity = gg_pubdir50_get(res, i, GG_PUBDIR50_FAMILYCITY);
						const char *__gender = gg_pubdir50_get(res, i, GG_PUBDIR50_GENDER);
						const char *__status = gg_pubdir50_get(res, i, GG_PUBDIR50_STATUS);
						uin_t uin = __fmnumber ? atoi(__fmnumber) : 0;

						HANDLE hContact = (res->seq == GG_SEQ_CHINFO) ? NULL : gg_getcontact(gg, uin, 0, 0, NULL);
						gg_netlog(gg, "gg_mainthread(%x): Search result for uin %d, seq %d.", gg, uin, res->seq);
						if (res->seq == GG_SEQ_SEARCH)
						{
							char strFmt1[64];
							char strFmt2[64];
							GGSEARCHRESULT sr = {0};

							mir_snprintf(strFmt2, sizeof(strFmt2), "%s", (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, status_gg2m(gg, atoi(__status)), 0));
							if (__city)
							{
								mir_snprintf(strFmt1, sizeof(strFmt1), ", %s %s", Translate("City:"), __city);
								strncat(strFmt2, strFmt1, sizeof(strFmt2) - strlen(strFmt2));
							}
							if (__birthyear)
							{
								time_t t = time(NULL);
								struct tm *lt = localtime(&t);
								int br = atoi(__birthyear);

								if (br < (lt->tm_year + 1900) && br > 1900)
								{
									mir_snprintf(strFmt1, sizeof(strFmt1), ", %s %d", Translate("Age:"), (lt->tm_year + 1900) - br);
									strncat(strFmt2, strFmt1, sizeof(strFmt2) - strlen(strFmt2));
								}
							}

							sr.hdr.cbSize = sizeof(sr);
							sr.hdr.nick = (char *)__nick;
							sr.hdr.firstName = (char *)__firstname;
							sr.hdr.lastName = (char *)__lastname;
							sr.hdr.email = strFmt2;
							sr.hdr.id = _ultoa(uin, strFmt1, 10);
							sr.uin = uin;

							ProtoBroadcastAck(GG_PROTO, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE) 1, (LPARAM)&sr);
						}

						if (((res->seq == GG_SEQ_INFO || res->seq == GG_SEQ_GETNICK) && hContact != NULL)
							|| res->seq == GG_SEQ_CHINFO)
						{
							// Change nickname if it's not present
							if (__nick && (res->seq == GG_SEQ_GETNICK || res->seq == GG_SEQ_CHINFO))
								DBWriteContactSettingString(hContact, GG_PROTO, GG_KEY_NICK, __nick);

							if (__nick)
								DBWriteContactSettingString(hContact, GG_PROTO, "NickName", __nick);
							else if (res->seq == GG_SEQ_CHINFO)
								DBDeleteContactSetting(NULL, GG_PROTO, "NickName");

							// Change other info
							if (__city)
								DBWriteContactSettingString(hContact, GG_PROTO, "City", __city);
							else if (res->seq == GG_SEQ_CHINFO)
								DBDeleteContactSetting(NULL, GG_PROTO, "City");

							if (__firstname)
								DBWriteContactSettingString(hContact, GG_PROTO, "FirstName", __firstname);
							else if (res->seq == GG_SEQ_CHINFO)
								DBDeleteContactSetting(NULL, GG_PROTO, "FirstName");

							if (__lastname)
								DBWriteContactSettingString(hContact, GG_PROTO, "LastName", __lastname);
							else if (res->seq == GG_SEQ_CHINFO)
								DBDeleteContactSetting(NULL, GG_PROTO, "LastName");

							if (__familyname)
								DBWriteContactSettingString(hContact, GG_PROTO, "FamilyName", __familyname);
							else if (res->seq == GG_SEQ_CHINFO)
								DBDeleteContactSetting(NULL, GG_PROTO, "FamilyName");

							if (__origincity)
								DBWriteContactSettingString(hContact, GG_PROTO, "CityOrigin", __origincity);
							else if (res->seq == GG_SEQ_CHINFO)
								DBDeleteContactSetting(NULL, GG_PROTO, "CityOrigin");

							if (__birthyear)
							{
								time_t t = time(NULL);
								struct tm *lt = localtime(&t);
								int br = atoi(__birthyear);
								if (br > 0)
								{
									DBWriteContactSettingWord(hContact, GG_PROTO, "Age", (WORD)(lt->tm_year + 1900 - br));
									DBWriteContactSettingWord(hContact, GG_PROTO, "BirthYear", (WORD)br);
								}
							}
							else if (res->seq == GG_SEQ_CHINFO)
							{
								DBDeleteContactSetting(NULL, GG_PROTO, "Age");
								DBDeleteContactSetting(NULL, GG_PROTO, "BirthYear");
							}

							// Gadu-Gadu Male <-> Female
							if (__gender)
							{
								if (res->seq == GG_SEQ_CHINFO)
									DBWriteContactSettingByte(hContact, GG_PROTO, "Gender",
									(BYTE)(!strcmp(__gender, GG_PUBDIR50_GENDER_SET_MALE) ? 'M' :
										  (!strcmp(__gender, GG_PUBDIR50_GENDER_SET_FEMALE) ? 'F' : '?')));
								else
									DBWriteContactSettingByte(hContact, GG_PROTO, "Gender",
									(BYTE)(!strcmp(__gender, GG_PUBDIR50_GENDER_MALE) ? 'M' :
										  (!strcmp(__gender, GG_PUBDIR50_GENDER_FEMALE) ? 'F' : '?')));
							}
							else if (res->seq == GG_SEQ_CHINFO)
							{
								DBDeleteContactSetting(NULL, GG_PROTO, "Gender");
							}

							gg_netlog(gg, "gg_mainthread(%x): Setting user info for uin %d.", gg, uin);
							ProtoBroadcastAck(GG_PROTO, hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
						}
					}
				}
				if (res->seq == GG_SEQ_SEARCH)
					ProtoBroadcastAck(GG_PROTO, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
				break;
			}

			// Status (deprecated)
			case GG_EVENT_STATUS:
				gg_changecontactstatus(gg, e->event.status.uin, e->event.status.status, e->event.status.descr, 0, 0, 0, 0);
				break;

			// Status (version >= 6.0)
			case GG_EVENT_STATUS60:
				{
					HANDLE hContact = gg_getcontact(gg, e->event.status60.uin, 0, 0, NULL);
					int oldstatus = DBGetContactSettingWord(hContact, GG_PROTO, GG_KEY_STATUS, (WORD)ID_STATUS_OFFLINE);
					uin_t uin = (uin_t)DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0);

					if (e->event.status60.uin == uin)
					{
						// Status was changed by the user simultaneously logged on using different Miranda account or IM client
						int iStatus = status_gg2m(gg, e->event.status60.status);
						CallProtoService(GG_PROTO, PS_SETAWAYMSG, iStatus, (LPARAM)e->event.status60.descr);
						CallProtoService(GG_PROTO, PS_SETSTATUS, iStatus, 0);
					}

					gg_changecontactstatus(gg, e->event.status60.uin, e->event.status60.status, e->event.status60.descr,
						e->event.status60.time, e->event.status60.remote_ip, e->event.status60.remote_port, e->event.status60.version);

					if (oldstatus == ID_STATUS_OFFLINE && DBGetContactSettingWord(hContact, GG_PROTO, GG_KEY_STATUS, (WORD)ID_STATUS_OFFLINE) != ID_STATUS_OFFLINE)
						gg_requestavatar(gg, hContact, 0);
				}
				break;

			// Received userlist / or put info
			case GG_EVENT_USERLIST:
				switch (e->event.userlist.type)
				{
					case GG_USERLIST_GET_REPLY:
						if(e->event.userlist.reply)
						{
							gg_parsecontacts(gg, e->event.userlist.reply);
							MessageBox(
								NULL,
								Translate("List import successful."),
								GG_PROTONAME,
								MB_OK | MB_ICONINFORMATION
							);
						}
						break;

					case GG_USERLIST_PUT_REPLY:
						if(gg->list_remove)
							MessageBox(
								NULL,
								Translate("List remove successful."),
								GG_PROTONAME,
								MB_OK | MB_ICONINFORMATION
							);
						else
							MessageBox(
								NULL,
								Translate("List export successful."),
								GG_PROTONAME,
								MB_OK | MB_ICONINFORMATION
							);

						break;
				}
				break;

			// Received message
			case GG_EVENT_MSG:
				// This is CTCP request
				if((e->event.msg.msgclass & GG_CLASS_CTCP))
				{
					gg_dccconnect(gg, e->event.msg.sender);
				}
				// Check if not conference and block
				else if(!e->event.msg.recipients_count || gg->gc_enabled)
				{
					// Check if groupchat
					if(e->event.msg.recipients_count && gg->gc_enabled && !DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_IGNORECONF, GG_KEYDEF_IGNORECONF))
					{
						char *chat = gg_gc_getchat(gg, e->event.msg.sender, e->event.msg.recipients, e->event.msg.recipients_count);
						if(chat)
						{
							char id[32];
							GCDEST gcdest = {GG_PROTO, chat, GC_EVENT_MESSAGE};
							GCEVENT gcevent = {sizeof(GCEVENT), &gcdest};
							time_t t = time(NULL);

							UIN2ID(e->event.msg.sender, id);

							gcevent.pszUID = id;
							gcevent.pszText = e->event.msg.message;
							gcevent.pszNick = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) gg_getcontact(gg, e->event.msg.sender, 1, 0, NULL), 0);
							gcevent.time = (!(e->event.msg.msgclass & GG_CLASS_OFFLINE) || e->event.msg.time > (t - timeDeviation)) ? t : e->event.msg.time;
							gcevent.dwFlags = GCEF_ADDTOLOG;
							gg_netlog(gg, "gg_mainthread(%x): Conference message to room %s & id %s.", gg, chat, id);
							CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
						}
					}
					// Check if not empty message ( who needs it? )
					else if(!e->event.msg.recipients_count && e->event.msg.message && *e->event.msg.message && strcmp(e->event.msg.message, "\xA0\0"))
					{
						CCSDATA ccs = {0};
						PROTORECVEVENT pre = {0};
						time_t t = time(NULL);
						ccs.szProtoService = PSR_MESSAGE;
						ccs.hContact = gg_getcontact(gg, e->event.msg.sender, 1, 0, NULL);
						ccs.lParam = (LPARAM)&pre;
						pre.timestamp = (!(e->event.msg.msgclass & GG_CLASS_OFFLINE) || e->event.msg.time > (t - timeDeviation)) ? t : e->event.msg.time;
						pre.szMessage = e->event.msg.message;
						CallService(MS_PROTO_CHAINRECV, 0, (LPARAM) &ccs);
					}

					// RichEdit format included (image)
					if(e->event.msg.formats_length &&
						DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_IMGRECEIVE, GG_KEYDEF_IMGRECEIVE) &&
						!(DBGetContactSettingDword(gg_getcontact(gg, e->event.msg.sender, 1, 0, NULL), "Ignore", "Mask1", 0) & IGNOREEVENT_MESSAGE))
					{
						char *formats = e->event.msg.formats;
						int len = 0, formats_len = e->event.msg.formats_length, add_ptr;

						while (len < formats_len)
						{
							add_ptr = sizeof(struct gg_msg_richtext_format);
							if (((struct gg_msg_richtext_format*)formats)->font & GG_FONT_IMAGE)
							{
								struct gg_msg_richtext_image *image = (struct gg_msg_richtext_image *)(formats + add_ptr);
								EnterCriticalSection(&gg->sess_mutex);
								gg_image_request(gg->sess, e->event.msg.sender, image->size, image->crc32);
								LeaveCriticalSection(&gg->sess_mutex);

								gg_netlog(gg, "gg_mainthread: image request sent!");
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
				if (e->event.multilogon_msg.recipients_count && gg->gc_enabled && !DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_IGNORECONF, GG_KEYDEF_IGNORECONF))
				{
					char *chat = gg_gc_getchat(gg, e->event.multilogon_msg.sender, e->event.multilogon_msg.recipients, e->event.multilogon_msg.recipients_count);
					if (chat)
					{
						char id[32];
						DBVARIANT dbv;
						GCDEST gcdest = {GG_PROTO, chat, GC_EVENT_MESSAGE};
						GCEVENT gcevent = {sizeof(GCEVENT), &gcdest};

						UIN2ID(DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0), id);

						gcevent.pszUID = id;
						gcevent.pszText = e->event.multilogon_msg.message;
						if (!DBGetContactSettingString(NULL, GG_PROTO, GG_KEY_NICK, &dbv))
							gcevent.pszNick = dbv.pszVal;
						else
							gcevent.pszNick = Translate("Me");
						gcevent.time = e->event.multilogon_msg.time;
						gcevent.bIsMe = 1;
						gcevent.dwFlags = GCEF_ADDTOLOG;
						gg_netlog(gg, "gg_mainthread(%x): Sent conference message to room %s.", gg, chat);
						CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
						if (gcevent.pszNick == dbv.pszVal) DBFreeVariant(&dbv);
					}
				}
				else if (!e->event.multilogon_msg.recipients_count && e->event.multilogon_msg.message && *e->event.multilogon_msg.message
					&& strcmp(e->event.multilogon_msg.message, "\xA0\0"))
				{
					DBEVENTINFO dbei = {0};
					dbei.cbSize = sizeof(dbei);
					dbei.szModule = GG_PROTO;
					dbei.timestamp = (DWORD)e->event.multilogon_msg.time;
					dbei.flags = DBEF_SENT;
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.cbBlob = (DWORD)strlen(e->event.multilogon_msg.message) + 1;
					dbei.pBlob = (PBYTE)e->event.multilogon_msg.message;
					CallService(MS_DB_EVENT_ADD, (WPARAM)gg_getcontact(gg, e->event.multilogon_msg.sender, 1, 0, NULL), (LPARAM)&dbei);
				}
				break;

			// Information on active concurrent sessions
			case GG_EVENT_MULTILOGON_INFO:
				{
					list_t l;
					int* iIndexes = NULL, i;
					gg_netlog(gg, "gg_mainthread(): Concurrent sessions count: %d.", e->event.multilogon_info.count);
					if (e->event.multilogon_info.count > 0)
						iIndexes = mir_calloc(e->event.multilogon_info.count * sizeof(int));
					EnterCriticalSection(&gg->sessions_mutex);
					for (l = gg->sessions; l; l = l->next)
					{
						struct gg_multilogon_session* sess = (struct gg_multilogon_session*)l->data;
						for (i = 0; i < e->event.multilogon_info.count; i++)
						{
							if (!memcmp(&sess->id, &e->event.multilogon_info.sessions[i].id, sizeof(gg_multilogon_id_t)) && iIndexes)
							{
								iIndexes[i]++;
								break;
							}
						}
						mir_free(sess->name);
						mir_free(sess);
					}
					list_destroy(gg->sessions, 0);
					gg->sessions = NULL;
					for (i = 0; i < e->event.multilogon_info.count; i++)
					{
						struct gg_multilogon_session* sess = mir_alloc(sizeof(struct gg_multilogon_session));
						memcpy(sess, &e->event.multilogon_info.sessions[i], sizeof(struct gg_multilogon_session));
						sess->name = mir_strdup(*e->event.multilogon_info.sessions[i].name != '\0'
												? e->event.multilogon_info.sessions[i].name
												: Translate("Unknown client"));
						list_add(&gg->sessions, sess, 0);
					}
					LeaveCriticalSection(&gg->sessions_mutex);
					gg_sessions_updatedlg(gg);
					if (ServiceExists(MS_POPUP_ADDPOPUPCLASS))
					{
						const char* szText = time(NULL) - logonTime > 3
							? Translate("You have logged in at another location")
							: Translate("You are logged in at another location");
						for (i = 0; i < e->event.multilogon_info.count; i++)
						{
							char szMsg[MAX_SECONDLINE];
							if (iIndexes && iIndexes[i]) continue;
							mir_snprintf(szMsg, SIZEOF(szMsg), "%s (%s)", szText,
										 *e->event.multilogon_info.sessions[i].name != '\0'
										 ? e->event.multilogon_info.sessions[i].name
										 : Translate("Unknown client"));
							gg_showpopup(gg, GG_PROTONAME, szMsg, GG_POPUP_MULTILOGON);
						}
					}
					mir_free(iIndexes);
				}
				break;

			// Image reply sent
			case GG_EVENT_IMAGE_REPLY:
				// Get rid of empty image
				if(e->event.image_reply.size && e->event.image_reply.image)
				{
					HANDLE hContact = gg_getcontact(gg, e->event.image_reply.sender, 1, 0, NULL);
					void *img = (void *)gg_img_loadpicture(gg, e, 0);

					if(!img)
						break;

					if(DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_IMGMETHOD, GG_KEYDEF_IMGMETHOD) == 1 || gg_img_opened(gg, e->event.image_reply.sender))
					{
						gg_img_display(gg, hContact, img);
					}
					else if(DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_IMGMETHOD, GG_KEYDEF_IMGMETHOD) == 2)
					{
						gg_img_displayasmsg(gg, hContact, img);
					}
					else
					{
						CLISTEVENT cle = {0};
						char service[128];
						mir_snprintf(service, sizeof(service), GGS_RECVIMAGE, GG_PROTO);

						cle.cbSize = sizeof(cle);
						cle.hContact = hContact;
						cle.hIcon = LoadIconEx("image", FALSE);
						cle.flags = CLEF_URGENT;
						cle.hDbEvent = (HANDLE)"img";
						cle.lParam = (LPARAM)img;
						cle.pszService = service;
						cle.pszTooltip = Translate("Incoming image");
						CallService(MS_CLIST_ADDEVENT, 0, (LPARAM)&cle);
						ReleaseIconEx("image", FALSE);
					}
				}
				break;

			// Image send request
			case GG_EVENT_IMAGE_REQUEST:
				gg_img_sendonrequest(gg, e);
				break;

			// Incoming direct connection
			case GG_EVENT_DCC7_NEW:
				{
					struct gg_dcc7 *dcc7 = e->event.dcc7_new;
					gg_netlog(gg, "gg_mainthread(%x): Incoming direct connection.", gg);
					dcc7->contact = gg_getcontact(gg, dcc7->peer_uin, 0, 0, NULL);

					// Check if user is on the list and if it is my uin
					if(!dcc7->contact || DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, -1) != dcc7->uin) {
						gg_dcc7_free(dcc7);
						e->event.dcc7_new = NULL;
						break;
					}

					// Add to waiting transfers
					EnterCriticalSection(&gg->ft_mutex);
					list_add(&gg->transfers, dcc7, 0);
					LeaveCriticalSection(&gg->ft_mutex);

					//////////////////////////////////////////////////
					// Add file recv request
					{
						CCSDATA ccs;
						PROTORECVEVENT pre;
						char *szBlob;
						char *szFilename = dcc7->filename;
						char *szMsg = dcc7->filename;
						gg_netlog(gg, "gg_mainthread(%x): Client: %d, File ack filename \"%s\" size %d.", gg, dcc7->peer_uin,
							dcc7->filename, dcc7->size);
						// Make new ggtransfer struct
						szBlob = (char *)malloc(sizeof(DWORD) + strlen(szFilename) + strlen(szMsg) + 2);
						// Store current dcc
						*(PDWORD)szBlob = (DWORD)dcc7;
						// Store filename
						strcpy(szBlob + sizeof(DWORD), szFilename);
						// Store description
						strcpy(szBlob + sizeof(DWORD) + strlen(szFilename) + 1, szMsg);
						ccs.szProtoService = PSR_FILE;
						ccs.hContact = dcc7->contact;
						ccs.wParam = 0;
						ccs.lParam = (LPARAM)&pre;
						pre.flags = 0;
						pre.timestamp = time(NULL);
						pre.szMessage = szBlob;
						pre.lParam = 0;
						CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
						free(szBlob);
					}
					e->event.dcc7_new = NULL;
				}
				break;

			// Direct connection rejected
			case GG_EVENT_DCC7_REJECT:
				{
					struct gg_dcc7 *dcc7 = e->event.dcc7_reject.dcc7;
					if (dcc7->type == GG_SESSION_DCC7_SEND)
					{
						gg_netlog(gg, "gg_mainthread(%x): File transfer denied by client %d (reason = %d).", gg, dcc7->peer_uin, e->event.dcc7_reject.reason);
						ProtoBroadcastAck(GG_PROTO, dcc7->contact, ACKTYPE_FILE, ACKRESULT_DENIED, dcc7, 0);

						// Remove from watches and free
						EnterCriticalSection(&gg->ft_mutex);
						list_remove(&gg->watches, dcc7, 0);
						LeaveCriticalSection(&gg->ft_mutex);
						gg_dcc7_free(dcc7);
					}
					else
					{
						gg_netlog(gg, "gg_mainthread(%x): File transfer aborted by client %d.", gg, dcc7->peer_uin);

						// Remove transfer from waiting list
						EnterCriticalSection(&gg->ft_mutex);
						list_remove(&gg->transfers, dcc7, 0);
						LeaveCriticalSection(&gg->ft_mutex);
					}
				}
				break;

			// Direct connection error
			case GG_EVENT_DCC7_ERROR:
				{
					struct gg_dcc7 *dcc7 = e->event.dcc7_error_ex.dcc7;
					switch (e->event.dcc7_error)
					{
						case GG_ERROR_DCC7_HANDSHAKE:
							gg_netlog(gg, "gg_mainthread(%x): Client: %d, Handshake error.", gg, dcc7 ? dcc7->peer_uin : 0);
							break;
						case GG_ERROR_DCC7_NET:
							gg_netlog(gg, "gg_mainthread(%x): Client: %d, Network error.", gg, dcc7 ? dcc7->peer_uin : 0);
							break;
						case GG_ERROR_DCC7_FILE:
							gg_netlog(gg, "gg_mainthread(%x): Client: %d, File read/write error.", gg, dcc7 ? dcc7->peer_uin : 0);
							break;
						case GG_ERROR_DCC7_EOF:
							gg_netlog(gg, "gg_mainthread(%x): Client: %d, End of file/connection error.", gg, dcc7 ? dcc7->peer_uin : 0);
							break;
						case GG_ERROR_DCC7_REFUSED:
							gg_netlog(gg, "gg_mainthread(%x): Client: %d, Connection refused error.", gg, dcc7 ? dcc7->peer_uin : 0);
							break;
						case GG_ERROR_DCC7_RELAY:
							gg_netlog(gg, "gg_mainthread(%x): Client: %d, Relay connection error.", gg, dcc7 ? dcc7->peer_uin : 0);
							break;
						default:
							gg_netlog(gg, "gg_mainthread(%x): Client: %d, Unknown error.", gg, dcc7 ? dcc7->peer_uin : 0);
					}
					if (!dcc7) break;

					// Remove from watches
					list_remove(&gg->watches, dcc7, 0);

					// Close file & fail
					if (dcc7->file_fd != -1)
					{
						_close(dcc7->file_fd);
						dcc7->file_fd = -1;
					}

					if (dcc7->contact)
						ProtoBroadcastAck(GG_PROTO, dcc7->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc7, 0);

					// Free dcc
					gg_dcc7_free(dcc7);
				}
				break;

			case GG_EVENT_XML_ACTION:
				if (DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ENABLEAVATARS, GG_KEYDEF_ENABLEAVATARS)) {
					HXML hXml;
					TCHAR *xmlAction;
					TCHAR *tag;

					xmlAction = gg_a2t(e->event.xml_action.data);
					tag = gg_a2t("events");
					hXml = xi.parseString(xmlAction, 0, tag);

					if (hXml != NULL) {
						HXML node;
						char *type, *sender;
						
						mir_free(tag);
						tag = gg_a2t("event/type");
						node = xi.getChildByPath(hXml, tag, 0);
						type = node != NULL ? gg_t2a(xi.getText(node)) : NULL;

						mir_free(tag);
						tag = gg_a2t("event/sender");
						node = xi.getChildByPath(hXml, tag, 0);
						sender = node != NULL ? gg_t2a(xi.getText(node)) : NULL;
						gg_netlog(gg, "gg_mainthread(%x): XML Action type: %s.", gg, type != NULL ? type : "unknown");
						// Avatar change notify
						if (type != NULL && !strcmp(type, "28")) {
							gg_netlog(gg, "gg_mainthread(%x): Client %s changed his avatar.", gg, sender);
							gg_requestavatar(gg, gg_getcontact(gg, atoi(sender), 0, 0, NULL), 0);
						}
						mir_free(type);
						mir_free(sender);
						xi.destroyNode(hXml);
					}
					mir_free(tag);
					mir_free(xmlAction);
				}
				break;

			case GG_EVENT_TYPING_NOTIFICATION:
				{
					HANDLE hContact = gg_getcontact(gg, e->event.typing_notification.uin, 0, 0, NULL);
#ifdef DEBUGMODE
					gg_netlog(gg, "gg_mainthread(%x): Typing notification from %d (%d).", gg,
						e->event.typing_notification.uin, e->event.typing_notification.length);
#endif
					CallService(MS_PROTO_CONTACTISTYPING, (WPARAM)hContact,
						e->event.typing_notification.length > 0 ? 7 : PROTOTYPE_CONTACTTYPING_OFF);
				}
				break;
		}
		// Free event struct
		gg_free_event(e);
	}

	gg_broadcastnewstatus(gg, ID_STATUS_OFFLINE);
	gg_setalloffline(gg);
	DBWriteContactSettingDword(NULL, GG_PROTO, GG_KEY_LOGONTIME, 0);

	// If it was unwanted disconnection reconnect
	if(gg->proto.m_iDesiredStatus != ID_STATUS_OFFLINE
		&& DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_ARECONNECT, GG_KEYDEF_ARECONNECT))
	{
		gg_netlog(gg, "gg_mainthread(%x): Unintentional disconnection detected. Going to reconnect...", gg);
		hostnum = 0;
		gg_broadcastnewstatus(gg, ID_STATUS_CONNECTING);
		mir_free(p.status_descr);
		goto retry;
	}

	mir_free(p.password);
	mir_free(p.status_descr);

	// Destroy concurrent sessions list
	{
		list_t l;
		EnterCriticalSection(&gg->sessions_mutex);
		for (l = gg->sessions; l; l = l->next)
		{
			struct gg_multilogon_session* sess = (struct gg_multilogon_session*)l->data;
			mir_free(sess->name);
			mir_free(sess);
		}
		list_destroy(gg->sessions, 0);
		gg->sessions = NULL;
		LeaveCriticalSection(&gg->sessions_mutex);
	}

	// Stop dcc server
	gg->pth_dcc.dwThreadId = 0;
#ifdef DEBUGMODE
	gg_netlog(gg, "gg_mainthread(%x): Waiting until DCC Server Thread finished, if needed.", gg);
#endif
	gg_threadwait(gg, &gg->pth_dcc);

	gg_netlog(gg, "gg_mainthread(%x): Server Thread Ending", gg);
	return;
}

////////////////////////////////////////////////////////////
// Change status function
void gg_broadcastnewstatus(GGPROTO *gg, int newStatus)
{
	int oldStatus;

	EnterCriticalSection(&gg->modemsg_mutex);
	oldStatus = gg->proto.m_iStatus;
	if(oldStatus == newStatus)
	{
		LeaveCriticalSection(&gg->modemsg_mutex);
		return;
	}
	gg->proto.m_iStatus = newStatus;
	LeaveCriticalSection(&gg->modemsg_mutex);

	ProtoBroadcastAck(GG_PROTO, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, newStatus);

	gg_netlog(gg, "gg_broadcastnewstatus(): Broadcast new status: %d.", newStatus);
}

////////////////////////////////////////////////////////////
// When contact is deleted
int gg_contactdeleted(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE) wParam;
	uin_t uin; int type;
	DBVARIANT dbv;

	uin = (uin_t)DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0);
	type = DBGetContactSettingByte(hContact, GG_PROTO, "ChatRoom", 0);

	// Terminate conference if contact is deleted
	if(type && !DBGetContactSetting(hContact, GG_PROTO, "ChatRoomID", &dbv) && gg->gc_enabled)
	{
		GCDEST gcdest = {GG_PROTO, dbv.pszVal, GC_EVENT_CONTROL};
		GCEVENT gcevent = {sizeof(GCEVENT), &gcdest};
		GGGC *chat = gg_gc_lookup(gg, dbv.pszVal);

		gg_netlog(gg, "gg_gc_event(): Terminating chat %x, id %s from contact list...", chat, dbv.pszVal);
		if(chat)
		{
			// Destroy chat entry
			free(chat->recipients);
			list_remove(&gg->chats, chat, 1);
			// Terminate chat window / shouldn't cascade entry is deleted
			CallServiceSync(MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gcevent);
			CallServiceSync(MS_GC_EVENT, SESSION_TERMINATE, (LPARAM)&gcevent);
		}

		DBFreeVariant(&dbv);
		return 0;
	}

	if(uin && gg_isonline(gg))
	{
		EnterCriticalSection(&gg->sess_mutex);
		gg_remove_notify_ex(gg->sess, uin, GG_USER_NORMAL);
		LeaveCriticalSection(&gg->sess_mutex);
	}

	return 0;
}

////////////////////////////////////////////////////////////
// When db settings changed
int gg_dbsettingchanged(GGPROTO *gg, WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	HANDLE hContact = (HANDLE) wParam;
	char *szProto = NULL;

	// Check if the contact is NULL or we are not online
	if(!gg_isonline(gg))
		return 0;

	// If contact has been blocked
	if(!strcmp(cws->szModule, GG_PROTO) && !strcmp(cws->szSetting, GG_KEY_BLOCK))
	{
		gg_notifyuser(gg, hContact, 1);
		return 0;
	}

	// Contact is being renamed
	if(gg->gc_enabled && !strcmp(cws->szModule, GG_PROTO) && !strcmp(cws->szSetting, GG_KEY_NICK)
		&& cws->value.pszVal)
	{
		// Groupchat window contact is being renamed
		DBVARIANT dbv;
		int type = DBGetContactSettingByte(hContact, GG_PROTO, "ChatRoom", 0);
		if(type && !DBGetContactSetting(hContact, GG_PROTO, "ChatRoomID", &dbv))
		{
			// Most important... check redundancy (fucking cascading)
			static int cascade = 0;
			if(!cascade && dbv.pszVal)
			{
				GCDEST gcdest = {GG_PROTO, dbv.pszVal, GC_EVENT_CHANGESESSIONAME};
				GCEVENT gcevent = {sizeof(GCEVENT), &gcdest};
				gcevent.pszText = cws->value.pszVal;
				gg_netlog(gg, "gg_dbsettingchanged(): Conference %s was renamed to %s.", dbv.pszVal, cws->value.pszVal);
				// Mark cascading
				/* FIXME */ cascade = 1;
				CallServiceSync(MS_GC_EVENT, 0, (LPARAM)&gcevent);
				/* FIXME */ cascade = 0;
			}
			DBFreeVariant(&dbv);
		}
		else
			// Change contact name on all chats
			gg_gc_changenick(gg, hContact, cws->value.pszVal);
	}

	// Contact list changes
	if(!strcmp(cws->szModule, "CList"))
	{
		// If name changed... change nick
		if(!strcmp(cws->szSetting, "MyHandle") && cws->value.type == DBVT_ASCIIZ && cws->value.pszVal)
			DBWriteContactSettingString(hContact, GG_PROTO, GG_KEY_NICK, cws->value.pszVal);

		// If not on list changed
		if(!strcmp(cws->szSetting, "NotOnList"))
		{
			if(DBGetContactSettingByte(hContact, "CList", "Hidden", 0))
				return 0;
			// Notify user normally this time if added to the list permanently
			if(cws->value.type == DBVT_DELETED || (cws->value.type == DBVT_BYTE && cws->value.bVal == 0))
				gg_notifyuser(gg, hContact, 1);
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////
// All users set offline
void gg_setalloffline(GGPROTO *gg)
{
	HANDLE hContact;
	char *szProto;

	gg_netlog(gg, "gg_setalloffline(): Setting buddies offline");
	DBWriteContactSettingWord(NULL, GG_PROTO, GG_KEY_STATUS, ID_STATUS_OFFLINE);
	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if(szProto != NULL && !strcmp(szProto, GG_PROTO))
		{
			DBWriteContactSettingWord(hContact, GG_PROTO, GG_KEY_STATUS, ID_STATUS_OFFLINE);
			// Clear IP and port settings
			DBDeleteContactSetting(hContact, GG_PROTO, GG_KEY_CLIENTIP);
			DBDeleteContactSetting(hContact, GG_PROTO, GG_KEY_CLIENTPORT);
			// Delete status descr
			DBDeleteContactSetting(hContact, "CList", GG_KEY_STATUSDESCR);
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
#ifdef DEBUGMODE
	gg_netlog(gg, "gg_setalloffline(): End");
#endif
}

////////////////////////////////////////////////////////////
// All users set offline
void gg_notifyuser(GGPROTO *gg, HANDLE hContact, int refresh)
{
	uin_t uin;
	if (!hContact) return;
	if (gg_isonline(gg) && (uin = (uin_t)DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0)))
	{
		// Check if user should be invisible
		// Or be blocked ?
		if ((DBGetContactSettingWord(hContact, GG_PROTO, GG_KEY_APPARENT, (WORD) ID_STATUS_ONLINE) == ID_STATUS_OFFLINE) ||
			DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
		{
			EnterCriticalSection(&gg->sess_mutex);
			if (refresh)
			{
				gg_remove_notify_ex(gg->sess, uin, GG_USER_NORMAL);
				gg_remove_notify_ex(gg->sess, uin, GG_USER_BLOCKED);
			}

			gg_add_notify_ex(gg->sess, uin, GG_USER_OFFLINE);
			LeaveCriticalSection(&gg->sess_mutex);
		}
		else if (DBGetContactSettingByte(hContact, GG_PROTO, GG_KEY_BLOCK, 0))
		{
			EnterCriticalSection(&gg->sess_mutex);
			if (refresh)
			{
				gg_remove_notify_ex(gg->sess, uin, GG_USER_OFFLINE);
			}

			gg_add_notify_ex(gg->sess, uin, GG_USER_BLOCKED);
			LeaveCriticalSection(&gg->sess_mutex);
		}
		else
		{
			EnterCriticalSection(&gg->sess_mutex);
			if (refresh)
			{
				gg_remove_notify_ex(gg->sess, uin, GG_USER_BLOCKED);
			}

			gg_add_notify_ex(gg->sess, uin, GG_USER_NORMAL);
			LeaveCriticalSection(&gg->sess_mutex);
		}
	}
}
void gg_notifyall(GGPROTO *gg)
{
	HANDLE hContact;
	char *szProto;
	int count = 0, cc = 0;
	uin_t *uins;
	char *types;

	gg_netlog(gg, "gg_notifyall(): Subscribing notification to all users");
	// Readup count
	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (szProto != NULL && !strcmp(szProto, GG_PROTO)) count ++;
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}

	// Readup list
	/* FIXME: If we have nothing on the list but we omit gg_notify_ex we have problem with receiving any contacts */
	if (count == 0)
	{
		if(gg_isonline(gg))
		{
			EnterCriticalSection(&gg->sess_mutex);
			gg_notify_ex(gg->sess, NULL, NULL, 0);
			LeaveCriticalSection(&gg->sess_mutex);
		}
		return;
	}
	uins = calloc(sizeof(uin_t), count);
	types = calloc(sizeof(char), count);

	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact && cc < count)
	{
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (szProto != NULL && !strcmp(szProto, GG_PROTO) && (uins[cc] = DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0)))
		{
			if ((DBGetContactSettingWord(hContact, GG_PROTO, GG_KEY_APPARENT, (WORD) ID_STATUS_ONLINE) == ID_STATUS_OFFLINE) ||
				DBGetContactSettingByte(hContact, "CList", "NotOnList", 0))
				types[cc] = GG_USER_OFFLINE;
			else if (DBGetContactSettingByte(hContact, GG_PROTO, GG_KEY_BLOCK, 0))
				types[cc] = GG_USER_BLOCKED;
			else
				types[cc] = GG_USER_NORMAL;
			cc ++;
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	if (cc < count) count = cc;

	// Send notification
	if (gg_isonline(gg))
	{
		EnterCriticalSection(&gg->sess_mutex);
		gg_notify_ex(gg->sess, uins, types, count);
		LeaveCriticalSection(&gg->sess_mutex);
	}

	// Free variables
	free(uins); free(types);
}

////////////////////////////////////////////////////////////
// Get contact by uin
HANDLE gg_getcontact(GGPROTO *gg, uin_t uin, int create, int inlist, char *szNick)
{
	HANDLE hContact;
	char *szProto;

	// Look for contact in DB
	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact)
	{
		szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if(szProto != NULL && !strcmp(szProto, GG_PROTO))
		{
			if((uin_t)DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0) == uin
				&& DBGetContactSettingByte(hContact, GG_PROTO, "ChatRoom", 0) == 0)
			{
				if(inlist)
				{
					DBDeleteContactSetting(hContact, "CList", "NotOnList");
					DBDeleteContactSetting(hContact, "CList", "Hidden");
				}
				return hContact;
			}
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	if(!create) return NULL;

	hContact = (HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0);

	if(!hContact)
	{
		gg_netlog(gg, "gg_getcontact(): Failed to create Gadu-Gadu contact %s", szNick);
		return NULL;
	}

	if(CallService(MS_PROTO_ADDTOCONTACT, (WPARAM) hContact, (LPARAM) GG_PROTO) != 0)
	{
		// For some reason we failed to register the protocol for this contact
		CallService(MS_DB_CONTACT_DELETE, (WPARAM) hContact, 0);
		gg_netlog(gg, "Failed to register GG contact %d", uin);
		return NULL;
	}

	gg_netlog(gg, "gg_getcontact(): Added buddy: %d", uin);
	if(!inlist)
	{
		DBWriteContactSettingByte(hContact, "CList", "NotOnList", 1);
		//DBWriteContactSettingByte(hContact, "CList", "Hidden", 1);
	}

	DBWriteContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, (DWORD) uin);
	DBWriteContactSettingWord(hContact, GG_PROTO, GG_KEY_STATUS, ID_STATUS_OFFLINE);

	// If nick specified use it
	if(szNick)
		DBWriteContactSettingString(hContact, GG_PROTO, GG_KEY_NICK, szNick);
	else if(gg_isonline(gg))
	{
		gg_pubdir50_t req;

		// Search for that nick
		if(req = gg_pubdir50_new(GG_PUBDIR50_SEARCH))
		{
			// Add uin and search it
			gg_pubdir50_add(req, GG_PUBDIR50_UIN, ditoa(uin));
			gg_pubdir50_seq_set(req, GG_SEQ_GETNICK);
			EnterCriticalSection(&gg->sess_mutex);
			gg_pubdir50(gg->sess, req);
			LeaveCriticalSection(&gg->sess_mutex);
			gg_pubdir50_free(req);
			DBWriteContactSettingString(hContact, GG_PROTO, GG_KEY_NICK, ditoa(uin));
			gg_netlog(gg, "gg_getcontact(): Search for nick on uin: %d", uin);
		}
	}

	// Add to notify list and pull avatar for the new contact
	if(gg_isonline(gg))
	{
		PROTO_AVATAR_INFORMATION pai = {0};

		EnterCriticalSection(&gg->sess_mutex);
		gg_add_notify_ex(gg->sess, uin, (char)(inlist ? GG_USER_NORMAL : GG_USER_OFFLINE));
		LeaveCriticalSection(&gg->sess_mutex);

		pai.cbSize = sizeof(pai);
		pai.hContact = hContact;
		gg_getavatarinfo(gg, (WPARAM)GAIF_FORCE, (LPARAM)&pai);

		// Change status of the contact with our own UIN (if got yourself added to the contact list)
		if (DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0) == uin)
		{
			char *szMsg;
			EnterCriticalSection(&gg->modemsg_mutex);
			szMsg = mir_strdup(gg_getstatusmsg(gg, gg->proto.m_iStatus));
			LeaveCriticalSection(&gg->modemsg_mutex);
			gg_changecontactstatus(gg, uin, status_m2gg(gg, gg->proto.m_iStatus, szMsg != NULL), szMsg, 0, 0, 0, 0);
			mir_free(szMsg);
		}
	}

	// TODO server side list & add buddy
	return hContact;
}

////////////////////////////////////////////////////////////
// Status conversion
int status_m2gg(GGPROTO *gg, int status, int descr)
{
	// check frends only
	int mask = DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_FRIENDSONLY, GG_KEYDEF_FRIENDSONLY) ? GG_STATUS_FRIENDS_MASK : 0;

	if(descr)
	{
		switch(status)
		{
			case ID_STATUS_OFFLINE:
				return GG_STATUS_NOT_AVAIL_DESCR | mask;

			case ID_STATUS_ONLINE:
				return GG_STATUS_AVAIL_DESCR | mask;

			case ID_STATUS_AWAY:
				return GG_STATUS_BUSY_DESCR | mask;

			case ID_STATUS_DND:
				return GG_STATUS_DND_DESCR | mask;

			case ID_STATUS_FREECHAT:
				return GG_STATUS_FFC_DESCR | mask;

			case ID_STATUS_INVISIBLE:
				return GG_STATUS_INVISIBLE_DESCR | mask;

			default:
				return GG_STATUS_BUSY_DESCR | mask;
		}
	}
	else
	{
		switch(status)
		{
			case ID_STATUS_OFFLINE:
				return GG_STATUS_NOT_AVAIL | mask;

			case ID_STATUS_ONLINE:
				return GG_STATUS_AVAIL | mask;

			case ID_STATUS_AWAY:
				return GG_STATUS_BUSY | mask;

			case ID_STATUS_DND:
				return GG_STATUS_DND | mask;

			case ID_STATUS_FREECHAT:
				return GG_STATUS_FFC | mask;

			case ID_STATUS_INVISIBLE:
				return GG_STATUS_INVISIBLE | mask;

			default:
				return GG_STATUS_BUSY | mask;
		}
	}
}
int status_gg2m(GGPROTO *gg, int status)
{
	// ignore additional flags
	status = GG_S(status);

	// when user has status description but is offline (show it invisible)
	if(status == GG_STATUS_NOT_AVAIL_DESCR && DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_SHOWINVISIBLE, GG_KEYDEF_SHOWINVISIBLE))
		return ID_STATUS_INVISIBLE;

	// rest of cases
	switch(status)
	{
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
void gg_changecontactstatus(GGPROTO *gg, uin_t uin, int status, const char *idescr, int time, uint32_t remote_ip, uint16_t remote_port, uint32_t version)
{
	HANDLE hContact = gg_getcontact(gg, uin, 0, 0, NULL);

	// Check if contact is on list
	if(!hContact) return;

	// Write contact status
	DBWriteContactSettingWord(hContact, GG_PROTO, GG_KEY_STATUS, (WORD)status_gg2m(gg, status));

	// Check if there's description and if it's not empty
	if(idescr && *idescr)
	{
		gg_netlog(gg, "gg_changecontactstatus(): Saving for %d status descr \"%s\".", uin, idescr);
		DBWriteContactSettingString(hContact, "CList", GG_KEY_STATUSDESCR, idescr);
	}
	else
		// Remove status if there's nothing
		DBDeleteContactSetting(hContact, "CList", GG_KEY_STATUSDESCR);

	// Store contact ip and port
	if(remote_ip) DBWriteContactSettingDword(hContact, GG_PROTO, GG_KEY_CLIENTIP, (DWORD) swap32(remote_ip));
	if(remote_port) DBWriteContactSettingWord(hContact, GG_PROTO, GG_KEY_CLIENTPORT, (WORD) remote_port);
	if(version)
	{
		char sversion[48];
		DBWriteContactSettingDword(hContact, GG_PROTO, GG_KEY_CLIENTVERSION, (DWORD) version);
		mir_snprintf(sversion, sizeof(sversion), "%sGadu-Gadu %s", (version & 0x00ffffff) > 0x2b ? "Nowe " : "", gg_version2string(version));
		DBWriteContactSettingString(hContact, GG_PROTO, "MirVer", sversion);
	}
}

////////////////////////////////////////////////////////////
// Returns GG client version string from packet version
const char *gg_version2string(int v)
{
	const char *pstr = "???";
	v &= 0x00ffffff;
	switch(v)
	{
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
