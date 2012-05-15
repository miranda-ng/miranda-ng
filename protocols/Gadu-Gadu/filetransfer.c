////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2006 Adam Strzelecki <ono+miranda@java.pl>
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
#include <fcntl.h>

void __cdecl gg_dccmainthread(GGPROTO *gg, void *empty);

void gg_dccstart(GGPROTO *gg)
{
	DWORD exitCode = 0;

	if(gg->dcc) return;

	// Startup dcc thread
	GetExitCodeThread(gg->pth_dcc.hThread, &exitCode);
	// Check if dcc thread isn't running already
	if(exitCode == STILL_ACTIVE)
	{
#ifdef DEBUGMODE
		gg_netlog(gg, "gg_dccstart(): DCC thread still active. Exiting...");
#endif
		// Signalize mainthread it's started
		if(gg->event) SetEvent(gg->event);
		return;
	}

	// Check if we wan't direct connections
	if(!DBGetContactSettingByte(NULL, GG_PROTO, GG_KEY_DIRECTCONNS, GG_KEYDEF_DIRECTCONNS))
	{
		gg_netlog(gg, "gg_dccstart(): No direct connections setup.");
		if(gg->event) SetEvent(gg->event);
		return;
	}

	// Start thread
	gg->pth_dcc.hThread = gg_forkthreadex(gg, gg_dccmainthread, NULL, &gg->pth_dcc.dwThreadId);
}

void gg_dccconnect(GGPROTO *gg, uin_t uin)
{
	struct gg_dcc *dcc;
	HANDLE hContact = gg_getcontact(gg, uin, 0, 0, NULL);
	DWORD ip, myuin; WORD port;

	gg_netlog(gg, "gg_dccconnect(): Connecting to uin %d.", uin);

	// If unknown user or not on list ignore
	if(!hContact) return;

	// Read user IP and port
	ip = swap32(DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_CLIENTIP, 0));
	port = DBGetContactSettingWord(hContact, GG_PROTO, GG_KEY_CLIENTPORT, 0);
	myuin = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0);

	// If not port nor ip nor my uin (?) specified
	if(!ip || !port || !uin) return;

	if(!(dcc = gg_dcc_get_file(ip, port, myuin, uin)))
		return;

	// Add client dcc to watches
	EnterCriticalSection(&gg->ft_mutex);
	list_add(&gg->watches, dcc, 0);
	LeaveCriticalSection(&gg->ft_mutex);
}

//////////////////////////////////////////////////////////
// THREAD: File transfer fail
struct ftfaildata
{
	HANDLE hContact;
	HANDLE hProcess;
};
void __cdecl gg_ftfailthread(GGPROTO *gg, void *param)
{
	struct ftfaildata *ft = (struct ftfaildata *)param;
	SleepEx(100, FALSE);
	gg_netlog(gg, "gg_ftfailthread(): Sending failed file transfer.");
	ProtoBroadcastAck(GG_PROTO, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft->hProcess, 0);
	free(ft);
}
HANDLE ftfail(GGPROTO *gg, HANDLE hContact)
{
	struct ftfaildata *ft = malloc(sizeof(struct ftfaildata));
#ifdef DEBUGMODE
	gg_netlog(gg, "gg_ftfail(): Failing file transfer...");
#endif
	srand(time(NULL));
	ft->hProcess = (HANDLE)rand();
	ft->hContact = hContact;
	gg_forkthread(gg, gg_ftfailthread, ft);
	return ft->hProcess;
}

////////////////////////////////////////////////////////////
// Main DCC connection session thread

// Info refresh min time (msec) / half-sec
#define GGSTATREFRESHEVERY	500

void __cdecl gg_dccmainthread(GGPROTO *gg, void *empty)
{
	uin_t uin;
	struct gg_event *e;
	struct timeval tv;
	fd_set rd, wd;
	int ret;
	SOCKET maxfd;
	DWORD tick;
	list_t l;
	char filename[MAX_PATH];

	// Zero up lists
	gg->watches = gg->transfers = gg->requests = l = NULL;

	gg_netlog(gg, "gg_dccmainthread(): DCC Server Thread Starting");

	// Readup number
	if (!(uin = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0)))
	{
		gg_netlog(gg, "gg_dccmainthread(): No Gadu-Gadu number specified. Exiting.");
		if(gg->event) SetEvent(gg->event);
		return;
	}

	// Create listen socket on config direct port
	if(!(gg->dcc = gg_dcc_socket_create(uin, (uint16_t)DBGetContactSettingWord(NULL, GG_PROTO, GG_KEY_DIRECTPORT, GG_KEYDEF_DIRECTPORT))))
	{
		gg_netlog(gg, "gg_dccmainthread(): Cannot create DCC listen socket. Exiting.");
		// Signalize mainthread we haven't start
		if(gg->event) SetEvent(gg->event);
		return;
	}

	gg_dcc_port = gg->dcc->port;
	gg_dcc_ip = inet_addr("255.255.255.255");
	gg_netlog(gg, "gg_dccmainthread(): Listening on port %d.", gg_dcc_port);

	// Signalize mainthread we started
	if(gg->event) SetEvent(gg->event);

	// Add main dcc handler to watches
	list_add(&gg->watches, gg->dcc, 0);

	// Do while we are in the main server thread
	while(gg->pth_dcc.dwThreadId && gg->dcc)
	{
		// Timeouts
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		// Prepare descriptiors for select
		FD_ZERO(&rd);
		FD_ZERO(&wd);

		for (maxfd = 0, l = gg->watches; l; l = l->next)
		{
			struct gg_common *w = l->data;

			if (!w || w->state == GG_STATE_ERROR || w->state == GG_STATE_IDLE || w->state == GG_STATE_DONE)
				continue;

			// Check if it's proper descriptor
			if (w->fd == -1) continue;

			if (w->fd > maxfd)
				maxfd = w->fd;
			if ((w->check & GG_CHECK_READ))
				FD_SET(w->fd, &rd);
			if ((w->check & GG_CHECK_WRITE))
				FD_SET(w->fd, &wd);
		}

		// Wait for data on selects
		ret = select(maxfd + 1, &rd, &wd, NULL, &tv);

		// Check for select error
		if (ret == -1)
		{
			if (errno == EBADF)
				gg_netlog(gg, "gg_dccmainthread(): Bad descriptor on select().");
			else if (errno != EINTR)
				gg_netlog(gg, "gg_dccmainthread(): Unknown error on select().");
			continue;
		}

		// Process watches (carefull with l)
		l = gg->watches;
		EnterCriticalSection(&gg->ft_mutex);
		while (l)
		{
			struct gg_common *c = l->data;
			struct gg_dcc *dcc = l->data;
			struct gg_dcc7 *dcc7 = l->data;
			l = l->next;

			switch (c->type)
			{
				default:
					if (!dcc || (!FD_ISSET(dcc->fd, &rd) && !FD_ISSET(dcc->fd, &wd)))
						continue;

					/////////////////////////////////////////////////////////////////
					// Process DCC events

					// Connection broken/closed
					if (!(e = gg_dcc_socket_watch_fd(dcc)))
					{
						gg_netlog(gg, "gg_dccmainthread(): Socket closed.");
						// Remove socket and _close
						list_remove(&gg->watches, dcc, 0);
						gg_dcc_socket_free(dcc);

						// Check if it's main socket
						if(dcc == gg->dcc) gg->dcc = NULL;
						continue;
					}
					else gg_netlog(gg, "gg_dccmainthread(): Event: %s", ggdebug_eventtype(e));

					switch(e->type)
					{
						// Client connected
						case GG_EVENT_DCC_NEW:
							list_add(&gg->watches, e->event.dcc_new, 0);
							e->event.dcc_new = NULL;
							break;

						//
						case GG_EVENT_NONE:
							// If transfer in progress do status
							if(dcc->file_fd != -1 && dcc->offset > 0 && (((tick = GetTickCount()) - dcc->tick) > GGSTATREFRESHEVERY))
							{
								PROTOFILETRANSFERSTATUS pfts;
								dcc->tick = tick;
								strncpy(filename, dcc->folder, sizeof(filename));
								strncat(filename, dcc->file_info.filename, sizeof(filename) - strlen(filename));
								memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
								pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
								pfts.hContact = (HANDLE)dcc->contact;
								pfts.flags = (dcc->type == GG_SESSION_DCC_SEND);
								pfts.pszFiles = NULL;
								pfts.totalFiles = 1;
								pfts.currentFileNumber = 0;
								pfts.totalBytes = dcc->file_info.size;
								pfts.totalProgress = dcc->offset;
								pfts.szWorkingDir = dcc->folder;
								pfts.szCurrentFile = filename;
								pfts.currentFileSize = dcc->file_info.size;
								pfts.currentFileProgress = dcc->offset;
								pfts.currentFileTime = 0;
								ProtoBroadcastAck(GG_PROTO, dcc->contact, ACKTYPE_FILE, ACKRESULT_DATA, dcc, (LPARAM)&pfts);
							}
							break;

						// Connection was successfuly ended
						case GG_EVENT_DCC_DONE:
							gg_netlog(gg, "gg_dccmainthread(): Client: %d, Transfer done ! Closing connection.", dcc->peer_uin);
							// Remove from watches
							list_remove(&gg->watches, dcc, 0);
							// Close file & success
							if(dcc->file_fd != -1)
							{
								PROTOFILETRANSFERSTATUS pfts;
								strncpy(filename, dcc->folder, sizeof(filename));
								strncat(filename, dcc->file_info.filename, sizeof(filename) - strlen(filename));
								memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
								pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
								pfts.hContact = (HANDLE)dcc->contact;
								pfts.flags = (dcc->type == GG_SESSION_DCC_SEND);
								pfts.pszFiles = NULL;
								pfts.totalFiles = 1;
								pfts.currentFileNumber = 0;
								pfts.totalBytes = dcc->file_info.size;
								pfts.totalProgress = dcc->file_info.size;
								pfts.szWorkingDir = dcc->folder;
								pfts.szCurrentFile = filename;
								pfts.currentFileSize = dcc->file_info.size;
								pfts.currentFileProgress = dcc->file_info.size;
								pfts.currentFileTime = 0;
								ProtoBroadcastAck(GG_PROTO, dcc->contact, ACKTYPE_FILE, ACKRESULT_DATA, dcc, (LPARAM)&pfts);
								_close(dcc->file_fd); dcc->file_fd = -1;
								ProtoBroadcastAck(GG_PROTO, dcc->contact, ACKTYPE_FILE, ACKRESULT_SUCCESS, dcc, 0);
							}
							// Free dcc
							gg_free_dcc(dcc); if(dcc == gg->dcc) gg->dcc = NULL;
							break;

						// Client error
						case GG_EVENT_DCC_ERROR:
							switch (e->event.dcc_error)
							{
								case GG_ERROR_DCC_HANDSHAKE:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, Handshake error.", dcc->peer_uin);
									break;
								case GG_ERROR_DCC_NET:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, Network error.", dcc->peer_uin);
									break;
								case GG_ERROR_DCC_FILE:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, File read/write error.", dcc->peer_uin);
									break;
								case GG_ERROR_DCC_EOF:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, End of file/connection error.", dcc->peer_uin);
									break;
								case GG_ERROR_DCC_REFUSED:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, Connection refused error.", dcc->peer_uin);
									break;
								default:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, Unknown error.", dcc->peer_uin);
							}
							// Don't do anything if it's main socket
							if(dcc == gg->dcc) break;

							// Remove from watches
							list_remove(&gg->watches, dcc, 0);

							// Close file & fail
							if(dcc->contact)
							{
								_close(dcc->file_fd); dcc->file_fd = -1;
								ProtoBroadcastAck(GG_PROTO, dcc->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc, 0);
							}
							// Free dcc
							gg_free_dcc(dcc); if(dcc == gg->dcc) gg->dcc = NULL;
							break;

						// Need file acknowledgement
						case GG_EVENT_DCC_NEED_FILE_ACK:
							gg_netlog(gg, "gg_dccmainthread(): Client: %d, File ack filename \"%s\" size %d.", dcc->peer_uin,
								dcc->file_info.filename, dcc->file_info.size);
							// Do not watch for transfer until user accept it
							list_remove(&gg->watches, dcc, 0);
							// Add to waiting transfers
							list_add(&gg->transfers, dcc, 0);

							//////////////////////////////////////////////////
							// Add file recv request
							{
								CCSDATA ccs;
								PROTORECVEVENT pre;
								char *szBlob;
								char *szFilename = dcc->file_info.filename;
								char *szMsg = dcc->file_info.filename;

								// Make new ggtransfer struct
								dcc->contact = gg_getcontact(gg, dcc->peer_uin, 0, 0, NULL);
								szBlob = (char *)malloc(sizeof(DWORD) + strlen(szFilename) + strlen(szMsg) + 2);
								// Store current dcc
								*(PDWORD)szBlob = (DWORD)dcc;
								// Store filename
								strcpy(szBlob + sizeof(DWORD), szFilename);
								// Store description
								strcpy(szBlob + sizeof(DWORD) + strlen(szFilename) + 1, szMsg);
								ccs.szProtoService = PSR_FILE;
								ccs.hContact = dcc->contact;
								ccs.wParam = 0;
								ccs.lParam = (LPARAM)&pre;
								pre.flags = 0;
								pre.timestamp = time(NULL);
								pre.szMessage = szBlob;
								pre.lParam = 0;
								CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);
								free(szBlob);
							}
							break;

						// Need client accept
						case GG_EVENT_DCC_CLIENT_ACCEPT:
							gg_netlog(gg, "gg_dccmainthread(): Client: %d, Client accept.", dcc->peer_uin);
							// Check if user is on the list and if it is my uin
							if(gg_getcontact(gg, dcc->peer_uin, 0, 0, NULL) &&
								DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, -1) == dcc->uin)
								break;

							// Kill unauthorized dcc
							list_remove(&gg->watches, dcc, 0);
							gg_free_dcc(dcc); if(dcc == gg->dcc) gg->dcc = NULL;
							break;

						// Client connected as we wished to (callback)
						case GG_EVENT_DCC_CALLBACK:
						{
							int found = 0;
							gg_netlog(gg, "gg_dccmainthread(): Callback from client %d.", dcc->peer_uin);
							// Seek for stored callback request
							for (l = gg->requests; l; l = l->next)
							{
								struct gg_dcc *req = l->data;

								if (req && req->peer_uin == dcc->peer_uin)
								{
									gg_dcc_set_type(dcc, GG_SESSION_DCC_SEND);
									found = 1;

									// Copy data req ===> dcc
									dcc->folder = req->folder;
									dcc->contact = req->contact;
									dcc->file_fd = req->file_fd;
									memcpy(&dcc->file_info, &req->file_info, sizeof(struct gg_file_info));
									// Copy data back to dcc ===> req
									memcpy(req, dcc, sizeof(struct gg_dcc));

									// Remove request
									list_remove(&gg->requests, req, 0);
									// Remove dcc from watches
									list_remove(&gg->watches, dcc, 0);
									// Add request to watches
									list_add(&gg->watches, req, 0);
									// Free old dat
									gg_free_dcc(dcc);
									gg_netlog(gg, "gg_dccmainthread(): Found stored request to client %d, filename \"%s\" size %d, folder \"%s\".",
										req->peer_uin, req->file_info.filename, req->file_info.size, req->folder);
									break;
								}
							}

							if (!found)
							{
								gg_netlog(gg, "gg_dccmainthread(): Unknown request to client %d.", dcc->peer_uin);
								// Kill unauthorized dcc
								list_remove(&gg->watches, dcc, 0);
								gg_free_dcc(dcc); if(dcc == gg->dcc) gg->dcc = NULL;
							}
							break;
						}
					}

					// Free event
					gg_free_event(e);
					break;

				case GG_SESSION_DCC7_SOCKET:
				case GG_SESSION_DCC7_GET:
				case GG_SESSION_DCC7_SEND:
				case GG_SESSION_DCC7_VOICE:
					if (!dcc7 || (!FD_ISSET(dcc7->fd, &rd) && !FD_ISSET(dcc7->fd, &wd)))
						continue;

					/////////////////////////////////////////////////////////////////
					// Process DCC7 events

					// Connection broken/closed
					if (!(e = gg_dcc7_watch_fd(dcc7)))
					{
						gg_netlog(gg, "gg_dccmainthread(): Socket closed.");
						// Remove socket and _close
						list_remove(&gg->watches, dcc7, 0);
						gg_dcc7_free(dcc7);
						continue;
					}
					else gg_netlog(gg, "gg_dccmainthread(): Event: %s", ggdebug_eventtype(e));

					switch(e->type)
					{
						//
						case GG_EVENT_NONE:
							// If transfer in progress do status
							if(dcc7->file_fd != -1 && dcc7->offset > 0 && (((tick = GetTickCount()) - dcc7->tick) > GGSTATREFRESHEVERY))
							{
								PROTOFILETRANSFERSTATUS pfts;
								dcc7->tick = tick;
								strncpy(filename, dcc7->folder, sizeof(filename));
								strncat(filename, dcc7->filename, sizeof(filename) - strlen(filename));
								memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
								pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
								pfts.hContact = (HANDLE)dcc7->contact;
								pfts.flags = (dcc7->type == GG_SESSION_DCC7_SEND);
								pfts.pszFiles = NULL;
								pfts.totalFiles = 1;
								pfts.currentFileNumber = 0;
								pfts.totalBytes = dcc7->size;
								pfts.totalProgress = dcc7->offset;
								pfts.szWorkingDir = dcc7->folder;
								pfts.szCurrentFile = filename;
								pfts.currentFileSize = dcc7->size;
								pfts.currentFileProgress = dcc7->offset;
								pfts.currentFileTime = 0;
								ProtoBroadcastAck(GG_PROTO, dcc7->contact, ACKTYPE_FILE, ACKRESULT_DATA, dcc7, (LPARAM)&pfts);
							}
							break;

						// Connection was successfuly ended
						case GG_EVENT_DCC7_DONE:
							gg_netlog(gg, "gg_dccmainthread(): Client: %d, Transfer done ! Closing connection.", dcc->peer_uin);
							// Remove from watches
							list_remove(&gg->watches, dcc7, 0);
							// Close file & success
							if(dcc7->file_fd != -1)
							{
								PROTOFILETRANSFERSTATUS pfts;
								strncpy(filename, dcc7->folder, sizeof(filename));
								strncat(filename, dcc7->filename, sizeof(filename) - strlen(filename));
								memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
								pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
								pfts.hContact = (HANDLE)dcc7->contact;
								pfts.flags = (dcc7->type == GG_SESSION_DCC7_SEND);
								pfts.pszFiles = NULL;
								pfts.totalFiles = 1;
								pfts.currentFileNumber = 0;
								pfts.totalBytes = dcc7->size;
								pfts.totalProgress = dcc7->size;
								pfts.szWorkingDir = dcc7->folder;
								pfts.szCurrentFile = filename;
								pfts.currentFileSize = dcc7->size;
								pfts.currentFileProgress = dcc7->size;
								pfts.currentFileTime = 0;
								ProtoBroadcastAck(GG_PROTO, dcc7->contact, ACKTYPE_FILE, ACKRESULT_DATA, dcc7, (LPARAM)&pfts);
								_close(dcc7->file_fd); dcc7->file_fd = -1;
								ProtoBroadcastAck(GG_PROTO, dcc7->contact, ACKTYPE_FILE, ACKRESULT_SUCCESS, dcc7, 0);
							}
							// Free dcc
							gg_dcc7_free(dcc7);
							break;

						// Client error
						case GG_EVENT_DCC7_ERROR:
							switch (e->event.dcc7_error)
							{
								case GG_ERROR_DCC7_HANDSHAKE:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, Handshake error.", dcc7->peer_uin);
									break;
								case GG_ERROR_DCC7_NET:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, Network error.", dcc7->peer_uin);
									break;
								case GG_ERROR_DCC7_FILE:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, File read/write error.", dcc7->peer_uin);
									break;
								case GG_ERROR_DCC7_EOF:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, End of file/connection error.", dcc7->peer_uin);
									break;
								case GG_ERROR_DCC7_REFUSED:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, Connection refused error.", dcc7->peer_uin);
									break;
								case GG_ERROR_DCC7_RELAY:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, Relay connection error.", dcc7->peer_uin);
									break;
								default:
									gg_netlog(gg, "gg_dccmainthread(): Client: %d, Unknown error.", dcc7->peer_uin);
							}
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
							break;
					}

					// Free event
					gg_free_event(e);
					break;
			}
		}
		LeaveCriticalSection(&gg->ft_mutex);
	}

	// Close all dcc client sockets
	for (l = gg->watches; l; l = l->next)
	{
		struct gg_common *c = l->data;
		if (!c) continue;
		if (c->type == GG_SESSION_DCC7_SOCKET || c->type == GG_SESSION_DCC7_SEND || c->type == GG_SESSION_DCC7_GET)
		{
			struct gg_dcc7 *dcc7 = l->data;
			gg_dcc7_free(dcc7);
		}
		else
		{
			struct gg_dcc *dcc = l->data;
			gg_dcc_socket_free(dcc);

			// Check if it's main socket
			if(dcc == gg->dcc) gg->dcc = NULL;
		}
	}
	// Close all waiting for aknowledgle transfers
	for (l = gg->transfers; l; l = l->next)
	{
		struct gg_common *c = l->data;
		if (!c) continue;
		if (c->type == GG_SESSION_DCC7_SOCKET || c->type == GG_SESSION_DCC7_SEND || c->type == GG_SESSION_DCC7_GET)
		{
			struct gg_dcc7 *dcc7 = l->data;
			gg_dcc7_free(dcc7);
		}
		else
		{
			struct gg_dcc *dcc = l->data;
			gg_dcc_socket_free(dcc);
		}
	}
	// Close all waiting dcc requests
	for (l = gg->requests; l; l = l->next)
	{
		struct gg_dcc *dcc = l->data;
		if(dcc) gg_free_dcc(dcc);
	}
	list_destroy(gg->watches, 0);
	list_destroy(gg->transfers, 0);
	list_destroy(gg->requests, 0);

	gg_dcc_port = 0;
	gg_dcc_ip = 0;
	gg_netlog(gg, "gg_dccmainthread(): DCC Server Thread Ending");
}

////////////////////////////////////////////////////////////
// Called when received an file
int gg_recvfile(PROTO_INTERFACE *proto, HANDLE hContact, PROTOFILEEVENT* pre)
{
	CCSDATA ccs = { hContact, PSR_FILE, 0, ( LPARAM )pre };
	return CallService( MS_PROTO_RECVFILE, 0, ( LPARAM )&ccs );
}

////////////////////////////////////////////////////////////
// Called when user sends a file
HANDLE gg_sendfile(PROTO_INTERFACE *proto, HANDLE hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles)
{
	GGPROTO *gg = (GGPROTO *) proto;
	char *bslash, *filename;
	struct gg_dcc *dcc;
	DWORD ip, ver;
	WORD port;
	uin_t myuin, uin;

	// Check if main dcc thread is on
	if(!gg_isonline(gg)) return ftfail(gg, hContact);

	filename = gg_t2a(ppszFiles[0]);

	// Read user IP and port
	ip = swap32(DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_CLIENTIP, 0));
	port = DBGetContactSettingWord(hContact, GG_PROTO, GG_KEY_CLIENTPORT, 0);
	myuin = DBGetContactSettingDword(NULL, GG_PROTO, GG_KEY_UIN, 0);
	uin = DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_UIN, 0);
	ver = DBGetContactSettingDword(hContact, GG_PROTO, GG_KEY_CLIENTVERSION, 0);

	// Use DCC7 if a contact is using at least version 7.6 or unknown version
	if ((ver & 0x00ffffff) >= 0x29 || !ver) {
		struct gg_dcc7 *dcc7;

		EnterCriticalSection(&gg->sess_mutex);
		if (!(dcc7 = gg_dcc7_send_file(gg->sess, uin, filename, NULL, NULL))) {
			LeaveCriticalSection(&gg->sess_mutex);
			gg_netlog(gg, "gg_sendfile(): Failed to send file \"%s\".", filename);
			mir_free(filename);
			return ftfail(gg, hContact);
		}
		LeaveCriticalSection(&gg->sess_mutex);

		gg_netlog(gg, "gg_sendfile(): Sending file \"%s\" to %d.", filename, uin);

		// Add dcc to watches
		list_add(&gg->watches, dcc7, 0);

		// Store handle
		dcc7->contact = hContact;
		dcc7->folder = _strdup(filename);
		dcc7->tick = 0;
		// Make folder name
		bslash = strrchr(dcc7->folder, '\\');
		if(bslash)
			*(bslash + 1) = 0;
		else
			*(dcc7->folder) = 0;
		mir_free(filename);
		return dcc7;
	}

	// Return if bad connection info
	if(!port || !uin || !myuin)
	{
		gg_netlog(gg, "gg_sendfile(): Bad contact uin or my uin. Exit.");
		mir_free(filename);
		return ftfail(gg, hContact);
	}

	// Try to connect if not ask user to connect me
	if((ip && port >= 10 && !(dcc = gg_dcc_send_file(ip, port, myuin, uin))) || (port < 10 && port > 0))
	{
		// Make fake dcc structure
		dcc = malloc(sizeof(struct gg_dcc));
		memset(dcc, 0, sizeof(struct gg_dcc));
		// Fill up structures
		dcc->uin = myuin;
		dcc->peer_uin = uin;
		dcc->fd = -1;
		dcc->type = GG_SESSION_DCC_SEND;
		gg_netlog(gg, "gg_sendfile(): Requesting user to connect us and scheduling gg_dcc struct for a later use.");
		EnterCriticalSection(&gg->sess_mutex);
		gg_dcc_request(gg->sess, uin);
		LeaveCriticalSection(&gg->sess_mutex);
		list_add(&gg->requests, dcc, 0);
	}

	// Write filename
	if(gg_dcc_fill_file_info(dcc, filename) == -1)
	{
		gg_netlog(gg, "gg_sendfile(): Cannot open and file fileinfo \"%s\".", filename);
		gg_free_dcc(dcc);
		mir_free(filename);
		return ftfail(gg, hContact);
	}

	gg_netlog(gg, "gg_sendfile(): Sending file \"%s\" to %d in %s mode.", filename, uin, (dcc->fd != -1) ? "active" : "passive");

	// Add dcc to watches if not passive
	if(dcc->fd != -1) list_add(&gg->watches, dcc, 0);

	// Store handle
	dcc->contact = hContact;
	dcc->folder = _strdup(filename);
	dcc->tick = 0;
	// Make folder name
	bslash = strrchr(dcc->folder, '\\');
	if(bslash)
		*(bslash + 1) = 0;
	else
		*(dcc->folder) = 0;

	mir_free(filename);
	return dcc;
}

HANDLE gg_dccfileallow(GGPROTO *gg, HANDLE hTransfer, const PROTOCHAR* szPath)
{
	struct gg_dcc *dcc = (struct gg_dcc *) hTransfer;
	char fileName[MAX_PATH], *path = gg_t2a(szPath);
	strncpy(fileName, path, sizeof(fileName));
	strncat(fileName, dcc->file_info.filename, sizeof(fileName) - strlen(fileName));
	dcc->folder = _strdup((char *) path);
	dcc->tick = 0;
	mir_free(path);

	// Remove transfer from waiting list
	EnterCriticalSection(&gg->ft_mutex);
	list_remove(&gg->transfers, dcc, 0);
	LeaveCriticalSection(&gg->ft_mutex);

	// Open file for appending and check if ok
	if((dcc->file_fd = _open(fileName, _O_WRONLY | _O_APPEND | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE)) == -1)
	{
		gg_netlog(gg, "gg_dccfileallow(): Failed to create file \"%s\".", fileName);
		ProtoBroadcastAck(GG_PROTO, dcc->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc, 0);
		// Free transfer
		gg_free_dcc(dcc);
		return 0;
	}

	// Put an offset to the file
	dcc->offset = _lseek(dcc->file_fd, 0, SEEK_END);

	// Add to watches and start transfer
	EnterCriticalSection(&gg->ft_mutex);
	list_add(&gg->watches, dcc, 0);
	LeaveCriticalSection(&gg->ft_mutex);

	gg_netlog(gg, "gg_dccfileallow(): Receiving file \"%s\" from %d.", dcc->file_info.filename, dcc->peer_uin);

	return hTransfer;
}

HANDLE gg_dcc7fileallow(GGPROTO *gg, HANDLE hTransfer, const PROTOCHAR* szPath)
{
	struct gg_dcc7 *dcc7 = (struct gg_dcc7 *) hTransfer;
	char fileName[MAX_PATH], *path = gg_t2a(szPath);
	int iFtRemoveRes;
	strncpy(fileName, path, sizeof(fileName));
	strncat(fileName, dcc7->filename, sizeof(fileName) - strlen(fileName));
	dcc7->folder = _strdup((char *) path);
	dcc7->tick = 0;
	mir_free(path);

	// Remove transfer from waiting list
	EnterCriticalSection(&gg->ft_mutex);
	iFtRemoveRes = list_remove(&gg->transfers, dcc7, 0);
	LeaveCriticalSection(&gg->ft_mutex);

	if (iFtRemoveRes == -1)
	{
		gg_netlog(gg, "gg_dcc7fileallow(): File transfer denied.");
		ProtoBroadcastAck(GG_PROTO, dcc7->contact, ACKTYPE_FILE, ACKRESULT_DENIED, dcc7, 0);
		// Free transfer
		gg_dcc7_free(dcc7);
		return 0;
	}

	// Open file for appending and check if ok
	if((dcc7->file_fd = _open(fileName, _O_WRONLY | _O_APPEND | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE)) == -1)
	{
		gg_netlog(gg, "gg_dcc7fileallow(): Failed to create file \"%s\".", fileName);
		gg_dcc7_reject(dcc7, GG_DCC7_REJECT_USER);
		ProtoBroadcastAck(GG_PROTO, dcc7->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc7, 0);
		// Free transfer
		gg_dcc7_free(dcc7);
		return 0;
	}

	// Put an offset to the file
	dcc7->offset = _lseek(dcc7->file_fd, 0, SEEK_END);
	gg_dcc7_accept(dcc7, dcc7->offset);

	// Add to watches and start transfer
	EnterCriticalSection(&gg->ft_mutex);
	list_add(&gg->watches, dcc7, 0);
	LeaveCriticalSection(&gg->ft_mutex);

	gg_netlog(gg, "gg_dcc7fileallow(): Receiving file \"%s\" from %d.", dcc7->filename, dcc7->peer_uin);

	return hTransfer;
}

////////////////////////////////////////////////////////////
// File receiving allowed
HANDLE gg_fileallow(PROTO_INTERFACE *proto, HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szPath)
{
	struct gg_common *c = (struct gg_common *) hTransfer;

	// Check if its proper dcc
	if (!c) return NULL;

	if (c->type == GG_SESSION_DCC7_GET)
		return gg_dcc7fileallow((GGPROTO *)proto, hTransfer, szPath);

	return gg_dccfileallow((GGPROTO *)proto, hTransfer, szPath);
}

int gg_dccfiledeny(GGPROTO *gg, HANDLE hTransfer)
{
	struct gg_dcc *dcc = (struct gg_dcc *) hTransfer;

	// Remove transfer from any list
	EnterCriticalSection(&gg->ft_mutex);
	if(gg->watches) list_remove(&gg->watches, dcc, 0);
	if(gg->requests) list_remove(&gg->requests, dcc, 0);
	if(gg->transfers) list_remove(&gg->transfers, dcc, 0);
	LeaveCriticalSection(&gg->ft_mutex);

	gg_netlog(gg, "gg_dccfiledeny(): Rejected file \"%s\" from/to %d.", dcc->file_info.filename, dcc->peer_uin);

	// Free transfer
	gg_free_dcc(dcc);

	return 0;
}

int gg_dcc7filedeny(GGPROTO *gg, HANDLE hTransfer)
{
	struct gg_dcc7 *dcc7 = (struct gg_dcc7 *) hTransfer;

	gg_dcc7_reject(dcc7, GG_DCC7_REJECT_USER);

	// Remove transfer from any list
	EnterCriticalSection(&gg->ft_mutex);
	if(gg->watches) list_remove(&gg->watches, dcc7, 0);
	if(gg->transfers) list_remove(&gg->transfers, dcc7, 0);
	LeaveCriticalSection(&gg->ft_mutex);

	gg_netlog(gg, "gg_dcc7filedeny(): Rejected file \"%s\" from/to %d.", dcc7->filename, dcc7->peer_uin);

	// Free transfer
	gg_dcc7_free(dcc7);

	return 0;
}

////////////////////////////////////////////////////////////
// File receiving denied
int gg_filedeny(PROTO_INTERFACE *proto, HANDLE hContact, HANDLE hTransfer, const PROTOCHAR* szReason)
{
	struct gg_common *c = (struct gg_common *) hTransfer;

	// Check if its proper dcc
	if (!c) return 0;

	if (c->type == GG_SESSION_DCC7_GET)
		return gg_dcc7filedeny((GGPROTO *)proto, hTransfer);

	return gg_dccfiledeny((GGPROTO *)proto, hTransfer);
}

int gg_dccfilecancel(GGPROTO *gg, HANDLE hTransfer)
{
	struct gg_dcc *dcc = (struct gg_dcc *) hTransfer;

	// Remove transfer from any list
	EnterCriticalSection(&gg->ft_mutex);
	if(gg->watches) list_remove(&gg->watches, dcc, 0);
	if(gg->requests) list_remove(&gg->requests, dcc, 0);
	if(gg->transfers) list_remove(&gg->transfers, dcc, 0);
	LeaveCriticalSection(&gg->ft_mutex);

	// Send failed info
	ProtoBroadcastAck(GG_PROTO, dcc->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc, 0);
	// Close file
	if(dcc->file_fd != -1)
	{
		_close(dcc->file_fd);
		dcc->file_fd = -1;
	}

	gg_netlog(gg, "gg_dccfilecancel(): Canceled file \"%s\" from/to %d.", dcc->file_info.filename, dcc->peer_uin);

	// Free transfer
	gg_free_dcc(dcc);

	return 0;
}

int gg_dcc7filecancel(GGPROTO *gg, HANDLE hTransfer)
{
	struct gg_dcc7 *dcc7 = (struct gg_dcc7 *) hTransfer;

	if (dcc7->type == GG_SESSION_DCC7_SEND && dcc7->state == GG_STATE_WAITING_FOR_ACCEPT)
		gg_dcc7_abort(dcc7);

	// Remove transfer from any list
	EnterCriticalSection(&gg->ft_mutex);
	if(gg->watches) list_remove(&gg->watches, dcc7, 0);
	if(gg->transfers) list_remove(&gg->transfers, dcc7, 0);
	LeaveCriticalSection(&gg->ft_mutex);

	// Send failed info
	ProtoBroadcastAck(GG_PROTO, dcc7->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc7, 0);
	// Close file
	if(dcc7->file_fd != -1)
	{
		_close(dcc7->file_fd);
		dcc7->file_fd = -1;
	}

	gg_netlog(gg, "gg_dcc7filecancel(): Canceled file \"%s\" from/to %d.", dcc7->filename, dcc7->peer_uin);

	// Free transfer
	gg_dcc7_free(dcc7);

	return 0;
}

////////////////////////////////////////////////////////////
// File transfer canceled
int gg_filecancel(PROTO_INTERFACE *proto, HANDLE hContact, HANDLE hTransfer)
{
	GGPROTO *gg = (GGPROTO *) proto;
	struct gg_common *c = (struct gg_common *) hTransfer;

	// Check if its proper dcc
	if(!c) return 0;

	if (c->type == GG_SESSION_DCC7_SEND || c->type == GG_SESSION_DCC7_GET)
		return gg_dcc7filecancel((GGPROTO *) proto, hTransfer);

	return gg_dccfilecancel((GGPROTO *) proto, hTransfer);
}
