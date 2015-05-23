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

void GGPROTO::dccstart()
{
	DWORD exitCode = 0;

	if (dcc) return;

	// Startup dcc thread
	GetExitCodeThread(pth_dcc.hThread, &exitCode);
	// Check if dcc thread isn't running already
	if (exitCode == STILL_ACTIVE)
	{
#ifdef DEBUGMODE
		debugLogA("dccstart(): DCC thread still active. Exiting...");
#endif
		// Signalize mainthread it's started
		if (hEvent) SetEvent(hEvent);
		return;
	}

	// Check if we wan't direct connections
	if (!getByte(GG_KEY_DIRECTCONNS, GG_KEYDEF_DIRECTCONNS))
	{
		debugLogA("dccstart(): No direct connections setup.");
		if (hEvent) SetEvent(hEvent);
		return;
	}

	// Start thread
#ifdef DEBUGMODE
	debugLogA("dccstart(): ForkThreadEx 4 GGPROTO::dccmainthread");
#endif
	pth_dcc.hThread = ForkThreadEx(&GGPROTO::dccmainthread, NULL, &pth_dcc.dwThreadId);
}

void GGPROTO::dccconnect(uin_t uin)
{
	struct gg_dcc *local_dcc;
	MCONTACT hContact = getcontact(uin, 0, 0, NULL);
	DWORD ip, myuin; WORD port;

	debugLogA("dccconnect(): Connecting to uin %d.", uin);

	// If unknown user or not on list ignore
	if (!hContact) return;

	// Read user IP and port
	ip = swap32(getDword(hContact, GG_KEY_CLIENTIP, 0));
	port = getWord(hContact, GG_KEY_CLIENTPORT, 0);
	myuin = getDword(GG_KEY_UIN, 0);

	// If not port nor ip nor my uin (?) specified
	if (!ip || !port || !uin) return;

	if (!(local_dcc = gg_dcc_get_file(ip, port, myuin, uin)))
		return;

	// Add client dcc to watches
	gg_EnterCriticalSection(&ft_mutex, "dccconnect", 36, "ft_mutex", 1);
	list_add(&watches, local_dcc, 0);
	gg_LeaveCriticalSection(&ft_mutex, "dccconnect", 36, 1, "ft_mutex", 1);
}

//////////////////////////////////////////////////////////
// THREAD: File transfer fail
struct ftfaildata
{
	MCONTACT hContact;
	HANDLE hProcess;
};

void __cdecl GGPROTO::ftfailthread(void *param)
{
	struct ftfaildata *ft = (struct ftfaildata *)param;
	debugLogA("ftfailthread(): started. Sending failed file transfer.");
	gg_sleep(100, FALSE, "ftfailthread", 102, 1);
	ProtoBroadcastAck(ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft->hProcess, 0);
	free(ft);
	debugLogA("ftfailthread(): end.");
}

HANDLE ftfail(GGPROTO *gg, MCONTACT hContact)
{
	ftfaildata *ft = (ftfaildata*)malloc(sizeof(struct ftfaildata));
#ifdef DEBUGMODE
	gg->debugLogA("ftfail(): Failing file transfer...");
#endif
	srand(time(NULL));
	ft->hProcess = (HANDLE)rand();
	ft->hContact = hContact;
#ifdef DEBUGMODE
	gg->debugLogA("ftfail(): ForkThread 5 GGPROTO::ftfailthread");
#endif
	gg->ForkThread(&GGPROTO::ftfailthread, ft);
	return ft->hProcess;
}

////////////////////////////////////////////////////////////
// Main DCC connection session thread

// Info refresh min time (msec) / half-sec
#define GGSTATREFRESHEVERY	500

void __cdecl GGPROTO::dccmainthread(void*)
{
	uin_t uin;
	gg_event *e;
	struct timeval tv;
	fd_set rd, wd;
	int ret;
	SOCKET maxfd;
	DWORD tick;
	list_t l;
	char filename[MAX_PATH];

	// Zero up lists
	watches = transfers = requests = l = NULL;

	debugLogA("dccmainthread(): started. DCC Server Thread Starting");

	// Readup number
	if (!(uin = getDword(GG_KEY_UIN, 0)))
	{
		debugLogA("dccmainthread(): No Gadu-Gadu number specified. Exiting.");
		if (hEvent) SetEvent(hEvent);
#ifdef DEBUGMODE
		debugLogA("dccmainthread(): end 1.");
#endif
		return;
	}

	// Create listen socket on config direct port
	if (!(dcc = gg_dcc_socket_create(uin, (uint16_t)getWord(GG_KEY_DIRECTPORT, GG_KEYDEF_DIRECTPORT))))
	{
		debugLogA("dccmainthread(): Cannot create DCC listen socket. Exiting.");
		// Signalize mainthread we haven't start
		if (hEvent) SetEvent(hEvent);
#ifdef DEBUGMODE
		debugLogA("dccmainthread(): end 2.");
#endif
		return;
	}

	gg_dcc_port = dcc->port;
	gg_dcc_ip = inet_addr("255.255.255.255");
	debugLogA("dccmainthread(): Listening on port %d.", gg_dcc_port);

	// Signalize mainthread we started
	if (hEvent) SetEvent(hEvent);

	// Add main dcc handler to watches
	list_add(&watches, dcc, 0);

	// Do while we are in the main server thread
	while(pth_dcc.dwThreadId && dcc)
	{
		// Timeouts
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		// Prepare descriptiors for select
		FD_ZERO(&rd);
		FD_ZERO(&wd);

		for (maxfd = 0, l = watches; l; l = l->next)
		{
			gg_common *w = (gg_common*)l->data;

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
				debugLogA("dccmainthread(): Bad descriptor on select().");
			else if (errno != EINTR)
				debugLogA("dccmainthread(): Unknown error on select().");
			continue;
		}

		// Process watches (carefull with l)
		l = watches;
		gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
		while (l)
		{
			struct gg_common *local_c = (gg_common*)l->data;
			struct gg_dcc *local_dcc = (gg_dcc*)l->data;
			struct gg_dcc7 *local_dcc7 = (gg_dcc7*)l->data;
			l = l->next;

			switch (local_c->type)
			{
				default:
					if (!local_dcc || (!FD_ISSET(local_dcc->fd, &rd) && !FD_ISSET(local_dcc->fd, &wd)))
						continue;

					/////////////////////////////////////////////////////////////////
					// Process DCC events

					// Connection broken/closed
					gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 2, "ft_mutex", 1);
					if (!(e = gg_dcc_socket_watch_fd(local_dcc)))
					{
						debugLogA("dccmainthread(): Socket closed.");
						// Remove socket and _close
						gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
						list_remove(&watches, local_dcc, 0);
						gg_dcc_socket_free(local_dcc);

						// Check if it's main socket
						if (local_dcc == dcc) dcc = NULL;
						continue;
					} else {
						gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
						debugLogA("dccmainthread(): Event: %s", ggdebug_eventtype(e));
					}

					switch(e->type)
					{
						// Client connected
						case GG_EVENT_DCC_NEW:
							list_add(&watches, e->event.dcc_new, 0);
							e->event.dcc_new = NULL;
							break;

						//
						case GG_EVENT_NONE:
							// If transfer in progress do status
							if (local_dcc->file_fd != -1 && local_dcc->offset > 0 && (((tick = GetTickCount()) - local_dcc->tick) > GGSTATREFRESHEVERY))
							{
								PROTOFILETRANSFERSTATUS pfts;
								local_dcc->tick = tick;
								strncpy(filename, local_dcc->folder, sizeof(filename));
								mir_strncat(filename, (char*)local_dcc->file_info.filename, sizeof(filename) - mir_strlen(filename));
								memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
								pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
								pfts.hContact = (MCONTACT)local_dcc->contact;
								pfts.flags = (local_dcc->type == GG_SESSION_DCC_SEND);
								pfts.pszFiles = NULL;
								pfts.totalFiles = 1;
								pfts.currentFileNumber = 0;
								pfts.totalBytes = local_dcc->file_info.size;
								pfts.totalProgress = local_dcc->offset;
								pfts.szWorkingDir = local_dcc->folder;
								pfts.szCurrentFile = filename;
								pfts.currentFileSize = local_dcc->file_info.size;
								pfts.currentFileProgress = local_dcc->offset;
								pfts.currentFileTime = 0;
								gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 3, "ft_mutex", 1);
								ProtoBroadcastAck((MCONTACT)local_dcc->contact, ACKTYPE_FILE, ACKRESULT_DATA, local_dcc, (LPARAM)&pfts);
								gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
							}
							break;

						// Connection was successfuly ended
						case GG_EVENT_DCC_DONE:
							debugLogA("dccmainthread(): Client: %d, Transfer done ! Closing connection.", dcc->peer_uin);
							// Remove from watches
							list_remove(&watches, local_dcc, 0);
							// Close file & success
							if (local_dcc->file_fd != -1)
							{
								PROTOFILETRANSFERSTATUS pfts;
								strncpy(filename, local_dcc->folder, sizeof(filename));
								mir_strncat(filename, (char*)local_dcc->file_info.filename, sizeof(filename) - mir_strlen(filename));
								memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
								pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
								pfts.hContact = (MCONTACT)local_dcc->contact;
								pfts.flags = (local_dcc->type == GG_SESSION_DCC_SEND);
								pfts.pszFiles = NULL;
								pfts.totalFiles = 1;
								pfts.currentFileNumber = 0;
								pfts.totalBytes = local_dcc->file_info.size;
								pfts.totalProgress = local_dcc->file_info.size;
								pfts.szWorkingDir = local_dcc->folder;
								pfts.szCurrentFile = filename;
								pfts.currentFileSize = local_dcc->file_info.size;
								pfts.currentFileProgress = local_dcc->file_info.size;
								pfts.currentFileTime = 0;
								gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 4, "ft_mutex", 1);
								ProtoBroadcastAck((MCONTACT)local_dcc->contact, ACKTYPE_FILE, ACKRESULT_DATA, local_dcc, (LPARAM)&pfts);
								gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
								_close(local_dcc->file_fd); local_dcc->file_fd = -1;
								gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 5, "ft_mutex", 1);
								ProtoBroadcastAck((MCONTACT)local_dcc->contact, ACKTYPE_FILE, ACKRESULT_SUCCESS, local_dcc, 0);
								gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
							}
							// Free dcc
							gg_free_dcc(local_dcc); if (local_dcc == dcc) dcc = NULL;
							break;

						// Client error
						case GG_EVENT_DCC_ERROR:
							switch (e->event.dcc_error)
							{
								case GG_ERROR_DCC_HANDSHAKE:
									debugLogA("dccmainthread(): Client: %d, Handshake error.", local_dcc->peer_uin);
									break;
								case GG_ERROR_DCC_NET:
									debugLogA("dccmainthread(): Client: %d, Network error.", local_dcc->peer_uin);
									break;
								case GG_ERROR_DCC_FILE:
									debugLogA("dccmainthread(): Client: %d, File read/write error.", local_dcc->peer_uin);
									break;
								case GG_ERROR_DCC_EOF:
									debugLogA("dccmainthread(): Client: %d, End of file/connection error.", local_dcc->peer_uin);
									break;
								case GG_ERROR_DCC_REFUSED:
									debugLogA("dccmainthread(): Client: %d, Connection refused error.", local_dcc->peer_uin);
									break;
								default:
									debugLogA("dccmainthread(): Client: %d, Unknown error.", local_dcc->peer_uin);
							}
							// Don't do anything if it's main socket
							if (local_dcc == dcc) break;

							// Remove from watches
							list_remove(&watches, local_dcc, 0);

							// Close file & fail
							if (local_dcc->contact)
							{
								_close(local_dcc->file_fd); local_dcc->file_fd = -1;
								gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 6, "ft_mutex", 1);
								ProtoBroadcastAck((MCONTACT)local_dcc->contact, ACKTYPE_FILE, ACKRESULT_FAILED, local_dcc, 0);
								gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
							}
							// Free dcc
							gg_free_dcc(local_dcc); if (local_dcc == dcc) dcc = NULL;
							break;

						// Need file acknowledgement
						case GG_EVENT_DCC_NEED_FILE_ACK:
							debugLogA("dccmainthread(): Client: %d, File ack filename \"%s\" size %d.", local_dcc->peer_uin,
									local_dcc->file_info.filename, local_dcc->file_info.size);
							// Do not watch for transfer until user accept it
							list_remove(&watches, local_dcc, 0);
							// Add to waiting transfers
							list_add(&transfers, local_dcc, 0);

							//////////////////////////////////////////////////
							// Add file recv request
							{
								// Make new ggtransfer struct
								local_dcc->contact = (void*)getcontact(local_dcc->peer_uin, 0, 0, NULL);
								TCHAR* filenameT = mir_utf8decodeT((char*)dcc->file_info.filename);

								PROTORECVFILET pre = {0};
								pre.dwFlags = PRFF_TCHAR;
								pre.fileCount = 1;
								pre.timestamp = time(NULL);
								pre.tszDescription = filenameT;
								pre.ptszFiles = &filenameT;
								pre.lParam = (LPARAM)local_dcc;

								gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 7, "ft_mutex", 1);
								ProtoChainRecvFile((MCONTACT)local_dcc->contact, &pre);
								gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);

								mir_free(filenameT);
							}
							break;

						// Need client accept
						case GG_EVENT_DCC_CLIENT_ACCEPT:
							debugLogA("dccmainthread(): Client: %d, Client accept.", local_dcc->peer_uin);
							// Check if user is on the list and if it is my uin
							if (getcontact(local_dcc->peer_uin, 0, 0, NULL) &&
								getDword(GG_KEY_UIN, -1) == local_dcc->uin)
								break;

							// Kill unauthorized dcc
							list_remove(&watches, dcc, 0);
							gg_free_dcc(local_dcc); if (local_dcc == dcc) dcc = NULL;
							break;

						// Client connected as we wished to (callback)
						case GG_EVENT_DCC_CALLBACK:
						{
							int found = 0;
							debugLogA("dccmainthread(): Callback from client %d.", local_dcc->peer_uin);
							// Seek for stored callback request
							for (l = requests; l; l = l->next)
							{
								struct gg_dcc *req = (gg_dcc*)l->data;

								if (req && req->peer_uin == local_dcc->peer_uin)
								{
									gg_dcc_set_type(local_dcc, GG_SESSION_DCC_SEND);
									found = 1;

									// Copy data req ===> dcc
									local_dcc->folder = req->folder;
									local_dcc->contact = req->contact;
									local_dcc->file_fd = req->file_fd;
									memcpy(&local_dcc->file_info, &req->file_info, sizeof(struct gg_file_info));
									// Copy data back to dcc ===> req
									memcpy(req, local_dcc, sizeof(struct gg_dcc));

									// Remove request
									list_remove(&requests, req, 0);
									// Remove dcc from watches
									list_remove(&watches, local_dcc, 0);
									// Add request to watches
									list_add(&watches, req, 0);
									// Free old dat
									gg_free_dcc(local_dcc);
									debugLogA("dccmainthread(): Found stored request to client %d, filename \"%s\" size %d, folder \"%s\".",
										req->peer_uin, req->file_info.filename, req->file_info.size, req->folder);
									break;
								}
							}

							if (!found)
							{
								debugLogA("dccmainthread(): Unknown request to client %d.", local_dcc->peer_uin);
								// Kill unauthorized dcc
								list_remove(&watches, local_dcc, 0);
								gg_free_dcc(local_dcc); if (local_dcc == dcc) dcc = NULL;
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
					if (!local_dcc7 || (!FD_ISSET(local_dcc7->fd, &rd) && !FD_ISSET(local_dcc7->fd, &wd)))
						continue;

					/////////////////////////////////////////////////////////////////
					// Process DCC7 events

					// Connection broken/closed
					gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 8, "ft_mutex", 1);
					if (!(e = gg_dcc7_watch_fd(local_dcc7)))
					{
						debugLogA("dccmainthread(): Socket closed.");
						// Remove socket and _close
						gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
						list_remove(&watches, local_dcc7, 0);
						gg_dcc7_free(local_dcc7);
						continue;
					} else {
						gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
						debugLogA("dccmainthread(): Event: %s", ggdebug_eventtype(e));
					}

					switch(e->type)
					{
						//
						case GG_EVENT_NONE:
							// If transfer in progress do status
							if (local_dcc7->file_fd != -1 && local_dcc7->offset > 0 && (((tick = GetTickCount()) - local_dcc7->tick) > GGSTATREFRESHEVERY))
							{
								PROTOFILETRANSFERSTATUS pfts;
								local_dcc7->tick = tick;
								strncpy(filename, local_dcc7->folder, sizeof(filename));
								mir_strncat(filename, (char*)local_dcc7->filename, sizeof(filename) - mir_strlen(filename));
								memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
								pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
								pfts.hContact = (MCONTACT)local_dcc7->contact;
								pfts.flags = (local_dcc7->type == GG_SESSION_DCC7_SEND);
								pfts.pszFiles = NULL;
								pfts.totalFiles = 1;
								pfts.currentFileNumber = 0;
								pfts.totalBytes = local_dcc7->size;
								pfts.totalProgress = local_dcc7->offset;
								pfts.szWorkingDir = local_dcc7->folder;
								pfts.szCurrentFile = filename;
								pfts.currentFileSize = local_dcc7->size;
								pfts.currentFileProgress = local_dcc7->offset;
								pfts.currentFileTime = 0;
								gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 9, "ft_mutex", 1);
								ProtoBroadcastAck((MCONTACT)local_dcc7->contact, ACKTYPE_FILE, ACKRESULT_DATA, local_dcc7, (LPARAM)&pfts);
								gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
							}
							break;

						// Connection was successfuly ended
						case GG_EVENT_DCC7_DONE:
							debugLogA("dccmainthread(): Client: %d, Transfer done ! Closing connection.", dcc->peer_uin);
							// Remove from watches
							list_remove(&watches, local_dcc7, 0);
							// Close file & success
							if (local_dcc7->file_fd != -1)
							{
								PROTOFILETRANSFERSTATUS pfts;
								strncpy(filename, local_dcc7->folder, sizeof(filename));
								mir_strncat(filename, (char*)local_dcc7->filename, sizeof(filename) - mir_strlen(filename));
								memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
								pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
								pfts.hContact = (MCONTACT)local_dcc7->contact;
								pfts.flags = (local_dcc7->type == GG_SESSION_DCC7_SEND);
								pfts.pszFiles = NULL;
								pfts.totalFiles = 1;
								pfts.currentFileNumber = 0;
								pfts.totalBytes = local_dcc7->size;
								pfts.totalProgress = local_dcc7->size;
								pfts.szWorkingDir = local_dcc7->folder;
								pfts.szCurrentFile = filename;
								pfts.currentFileSize = local_dcc7->size;
								pfts.currentFileProgress = local_dcc7->size;
								pfts.currentFileTime = 0;
								gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 10, "ft_mutex", 1);
								ProtoBroadcastAck((MCONTACT)local_dcc7->contact, ACKTYPE_FILE, ACKRESULT_DATA, local_dcc7, (LPARAM)&pfts);
								gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
								_close(local_dcc7->file_fd); local_dcc7->file_fd = -1;
								gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 11, "ft_mutex", 1);
								ProtoBroadcastAck((MCONTACT)local_dcc7->contact, ACKTYPE_FILE, ACKRESULT_SUCCESS, local_dcc7, 0);
								gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
							}
							// Free dcc
							gg_dcc7_free(local_dcc7);
							break;

						// Client error
						case GG_EVENT_DCC7_ERROR:
							switch (e->event.dcc7_error)
							{
								case GG_ERROR_DCC7_HANDSHAKE:
									debugLogA("dccmainthread(): Client: %d, Handshake error.", local_dcc7->peer_uin);
									break;
								case GG_ERROR_DCC7_NET:
									debugLogA("dccmainthread(): Client: %d, Network error.", local_dcc7->peer_uin);
									break;
								case GG_ERROR_DCC7_FILE:
									debugLogA("dccmainthread(): Client: %d, File read/write error.", local_dcc7->peer_uin);
									break;
								case GG_ERROR_DCC7_EOF:
									debugLogA("dccmainthread(): Client: %d, End of file/connection error.", local_dcc7->peer_uin);
									break;
								case GG_ERROR_DCC7_REFUSED:
									debugLogA("dccmainthread(): Client: %d, Connection refused error.", local_dcc7->peer_uin);
									break;
								case GG_ERROR_DCC7_RELAY:
									debugLogA("dccmainthread(): Client: %d, Relay connection error.", local_dcc7->peer_uin);
									break;
								default:
									debugLogA("dccmainthread(): Client: %d, Unknown error.", local_dcc7->peer_uin);
							}
							// Remove from watches
							list_remove(&watches, local_dcc7, 0);

							// Close file & fail
							if (local_dcc7->file_fd != -1)
							{
								_close(local_dcc7->file_fd);
								local_dcc7->file_fd = -1;
							}

							if (local_dcc7->contact) {
								gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 12, "ft_mutex", 1);
								ProtoBroadcastAck((MCONTACT)local_dcc7->contact, ACKTYPE_FILE, ACKRESULT_FAILED, local_dcc7, 0);
								gg_EnterCriticalSection(&ft_mutex, "dccmainthread", 37, "ft_mutex", 1);
							}

							// Free dcc
							gg_dcc7_free(local_dcc7);
							break;
					}

					// Free event
					gg_free_event(e);
					break;
			}
		}
		gg_LeaveCriticalSection(&ft_mutex, "dccmainthread", 37, 1, "ft_mutex", 1);
	}

	// Close all dcc client sockets
	for (l = watches; l; l = l->next)
	{
		struct gg_common *c = (gg_common*)l->data;
		if (!c) continue;
		if (c->type == GG_SESSION_DCC7_SOCKET || c->type == GG_SESSION_DCC7_SEND || c->type == GG_SESSION_DCC7_GET)
		{
			struct gg_dcc7 *local_dcc7 = (gg_dcc7*)l->data;
			gg_dcc7_free(local_dcc7);
		}
		else
		{
			struct gg_dcc *local_dcc = (gg_dcc*)l->data;
			gg_dcc_socket_free(local_dcc);

			// Check if it's main socket
			if (local_dcc == dcc) dcc = NULL;
		}
	}
	// Close all waiting for aknowledgle transfers
	for (l = transfers; l; l = l->next)
	{
		struct gg_common *c = (gg_common*)l->data;
		if (!c) continue;
		if (c->type == GG_SESSION_DCC7_SOCKET || c->type == GG_SESSION_DCC7_SEND || c->type == GG_SESSION_DCC7_GET)
		{
			struct gg_dcc7 *local_dcc7 = (gg_dcc7*)l->data;
			gg_dcc7_free(local_dcc7);
		}
		else
		{
			struct gg_dcc *local_dcc = (gg_dcc*)l->data;
			gg_dcc_socket_free(local_dcc);
		}
	}
	// Close all waiting dcc requests
	for (l = requests; l; l = l->next)
	{
		struct gg_dcc *local_dcc = (gg_dcc*)l->data;
		if (local_dcc) gg_free_dcc(local_dcc);
	}
	list_destroy(watches, 0);
	list_destroy(transfers, 0);
	list_destroy(requests, 0);

	gg_dcc_port = 0;
	gg_dcc_ip = 0;
	debugLogA("dccmainthread(): end. DCC Server Thread Ending");
}

HANDLE GGPROTO::dccfileallow(HANDLE hTransfer, const PROTOCHAR* szPath)
{
	struct gg_dcc *dcc = (struct gg_dcc *) hTransfer;
	char fileName[MAX_PATH], *path = mir_t2a(szPath);
	strncpy(fileName, path, sizeof(fileName));
	mir_strncat(fileName, (char*)dcc->file_info.filename, sizeof(fileName) - mir_strlen(fileName));
	dcc->folder = _strdup((char *) path);
	dcc->tick = 0;
	mir_free(path);

	// Remove transfer from waiting list
	gg_EnterCriticalSection(&ft_mutex, "dccfileallow", 38, "ft_mutex", 1);
	list_remove(&transfers, dcc, 0);
	gg_LeaveCriticalSection(&ft_mutex, "dccfileallow", 38, 1, "ft_mutex", 1);

	// Open file for appending and check if ok
	if ((dcc->file_fd = _open(fileName, _O_WRONLY | _O_APPEND | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE)) == -1)
	{
		debugLogA("dccfileallow(): Failed to create file \"%s\". errno=%d: %s", fileName, errno, strerror(errno));
		TCHAR error[512];
		mir_sntprintf(error, SIZEOF(error), TranslateT("Cannot create transfer file. ERROR: %d: %s (dcc)\n%s"), errno, _tcserror(errno), szPath);
		showpopup(m_tszUserName, error, GG_POPUP_ERROR);
		ProtoBroadcastAck((MCONTACT)dcc->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc, 0);
		// Free transfer
		gg_free_dcc(dcc);
		return 0;
	}

	// Put an offset to the file
	dcc->offset = _lseek(dcc->file_fd, 0, SEEK_END);

	// Add to watches and start transfer
	gg_EnterCriticalSection(&ft_mutex, "dccfileallow", 39, "ft_mutex", 1);
	list_add(&watches, dcc, 0);
	gg_LeaveCriticalSection(&ft_mutex, "dccfileallow", 39, 1, "ft_mutex", 1);

	debugLogA("dccfileallow(): Receiving file \"%s\" from %d.", dcc->file_info.filename, dcc->peer_uin);

	return hTransfer;
}

HANDLE GGPROTO::dcc7fileallow(HANDLE hTransfer, const PROTOCHAR* szPath)
{
	struct gg_dcc7 *dcc7 = (struct gg_dcc7 *) hTransfer;
	char fileName[MAX_PATH], *path = mir_t2a(szPath);
	int iFtRemoveRes;
	strncpy(fileName, path, sizeof(fileName));
	mir_strncat(fileName, (char*)dcc7->filename, sizeof(fileName) - mir_strlen(fileName));
	dcc7->folder = _strdup((char *) path);
	dcc7->tick = 0;
	mir_free(path);

	// Remove transfer from waiting list
	gg_EnterCriticalSection(&ft_mutex, "dcc7fileallow", 40, "ft_mutex", 1);
	iFtRemoveRes = list_remove(&transfers, dcc7, 0);
	gg_LeaveCriticalSection(&ft_mutex, "dcc7fileallow", 40, 1, "ft_mutex", 1);

	if (iFtRemoveRes == -1)
	{
		debugLogA("dcc7fileallow(): File transfer denied.");
		ProtoBroadcastAck((MCONTACT)dcc7->contact, ACKTYPE_FILE, ACKRESULT_DENIED, dcc7, 0);
		// Free transfer
		gg_dcc7_free(dcc7);
		return 0;
	}

	// Open file for appending and check if ok
	if ((dcc7->file_fd = _open(fileName, _O_WRONLY | _O_APPEND | _O_BINARY | _O_CREAT, _S_IREAD | _S_IWRITE)) == -1)
	{
		debugLogA("dcc7fileallow(): Failed to create file \"%s\". errno=%d: %s", fileName, errno, strerror(errno));
		TCHAR error[512];
		mir_sntprintf(error, SIZEOF(error), TranslateT("Cannot create transfer file. ERROR: %d: %s (dcc7)\n%s"), errno, _tcserror(errno), szPath);
		showpopup(m_tszUserName, error, GG_POPUP_ERROR);
		gg_dcc7_reject(dcc7, GG_DCC7_REJECT_USER);
		ProtoBroadcastAck((MCONTACT)dcc7->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc7, 0);
		// Free transfer
		gg_dcc7_free(dcc7);
		return 0;
	}

	// Put an offset to the file
	dcc7->offset = _lseek(dcc7->file_fd, 0, SEEK_END);
	gg_dcc7_accept(dcc7, dcc7->offset);

	// Add to watches and start transfer
	gg_EnterCriticalSection(&ft_mutex, "dcc7fileallow", 41, "ft_mutex", 1);
	list_add(&watches, dcc7, 0);
	gg_LeaveCriticalSection(&ft_mutex, "dcc7fileallow", 41, 1, "ft_mutex", 1);

	debugLogA("dcc7fileallow(): Receiving file \"%s\" from %d.", dcc7->filename, dcc7->peer_uin);

	return hTransfer;
}

int GGPROTO::dccfiledeny(HANDLE hTransfer)
{
	struct gg_dcc *dcc = (struct gg_dcc *) hTransfer;

	// Remove transfer from any list
	gg_EnterCriticalSection(&ft_mutex, "dccfiledeny", 42, "ft_mutex", 1);
	if (watches) list_remove(&watches, dcc, 0);
	if (requests) list_remove(&requests, dcc, 0);
	if (transfers) list_remove(&transfers, dcc, 0);
	gg_LeaveCriticalSection(&ft_mutex, "dccfiledeny", 42, 1, "ft_mutex", 1);

	debugLogA("dccfiledeny(): Rejected file \"%s\" from/to %d.", dcc->file_info.filename, dcc->peer_uin);

	// Free transfer
	gg_free_dcc(dcc);

	return 0;
}

int GGPROTO::dcc7filedeny(HANDLE hTransfer)
{
	struct gg_dcc7 *dcc7 = (struct gg_dcc7 *) hTransfer;

	gg_dcc7_reject(dcc7, GG_DCC7_REJECT_USER);

	// Remove transfer from any list
	gg_EnterCriticalSection(&ft_mutex, "dcc7filedeny", 43, "ft_mutex", 1);
	if (watches) list_remove(&watches, dcc7, 0);
	if (transfers) list_remove(&transfers, dcc7, 0);
	gg_LeaveCriticalSection(&ft_mutex, "dcc7filedeny", 43, 1, "ft_mutex", 1);

	debugLogA("dcc7filedeny(): Rejected file \"%s\" from/to %d.", dcc7->filename, dcc7->peer_uin);

	// Free transfer
	gg_dcc7_free(dcc7);

	return 0;
}

int GGPROTO::dccfilecancel(HANDLE hTransfer)
{
	struct gg_dcc *dcc = (struct gg_dcc *) hTransfer;

	// Remove transfer from any list
	gg_EnterCriticalSection(&ft_mutex, "dccfilecancel", 44, "ft_mutex", 1);
	if (watches) list_remove(&watches, dcc, 0);
	if (requests) list_remove(&requests, dcc, 0);
	if (transfers) list_remove(&transfers, dcc, 0);
	gg_LeaveCriticalSection(&ft_mutex, "dccfilecancel", 44, 1, "ft_mutex", 1);

	// Send failed info
	ProtoBroadcastAck((MCONTACT)dcc->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc, 0);
	// Close file
	if (dcc->file_fd != -1)
	{
		_close(dcc->file_fd);
		dcc->file_fd = -1;
	}

	debugLogA("dccfilecancel(): Canceled file \"%s\" from/to %d.", dcc->file_info.filename, dcc->peer_uin);

	// Free transfer
	gg_free_dcc(dcc);

	return 0;
}

int GGPROTO::dcc7filecancel(HANDLE hTransfer)
{
	struct gg_dcc7 *dcc7 = (struct gg_dcc7 *) hTransfer;

	if (dcc7->type == GG_SESSION_DCC7_SEND && dcc7->state == GG_STATE_WAITING_FOR_ACCEPT)
		gg_dcc7_abort(dcc7);

	// Remove transfer from any list
	gg_EnterCriticalSection(&ft_mutex, "dcc7filecancel", 45, "ft_mutex", 1);
	if (watches) list_remove(&watches, dcc7, 0);
	if (transfers) list_remove(&transfers, dcc7, 0);
	gg_LeaveCriticalSection(&ft_mutex, "dcc7filecancel", 45, 1, "ft_mutex", 1);

	// Send failed info
	ProtoBroadcastAck((MCONTACT)dcc7->contact, ACKTYPE_FILE, ACKRESULT_FAILED, dcc7, 0);
	// Close file
	if (dcc7->file_fd != -1)
	{
		_close(dcc7->file_fd);
		dcc7->file_fd = -1;
	}

	debugLogA("dcc7filecancel(): Canceled file \"%s\" from/to %d.", dcc7->filename, dcc7->peer_uin);

	// Free transfer
	gg_dcc7_free(dcc7);

	return 0;
}

////////////////////////////////////////////////////////////
// File receiving allowed

HANDLE GGPROTO::FileAllow(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szPath)
{
	// Check if its proper dcc
	struct gg_common *c = (struct gg_common *) hTransfer;
	if (!c)
		return NULL;

	if (c->type == GG_SESSION_DCC7_GET)
		return dcc7fileallow(hTransfer, szPath);

	return dccfileallow(hTransfer, szPath);
}

////////////////////////////////////////////////////////////
// File transfer canceled

int GGPROTO::FileCancel(MCONTACT hContact, HANDLE hTransfer)
{
	// Check if its proper dcc
	struct gg_common *c = (struct gg_common *) hTransfer;
	if (!c)
		return 0;

	if (c->type == GG_SESSION_DCC7_SEND || c->type == GG_SESSION_DCC7_GET)
		return dcc7filecancel(hTransfer);

	return dccfilecancel(hTransfer);
}

////////////////////////////////////////////////////////////
// File receiving denied

int GGPROTO::FileDeny(MCONTACT hContact, HANDLE hTransfer, const PROTOCHAR* szReason)
{
	// Check if its proper dcc
	struct gg_common *c = (struct gg_common *) hTransfer;
	if (!c)
		return 0;

	if (c->type == GG_SESSION_DCC7_GET)
		return dcc7filedeny(hTransfer);

	return dccfiledeny(hTransfer);
}

////////////////////////////////////////////////////////////
// Called when received an file

int GGPROTO::RecvFile(MCONTACT hContact, PROTOFILEEVENT* pre)
{
	return Proto_RecvFile(hContact, pre);
}

////////////////////////////////////////////////////////////
// Called when user sends a file

HANDLE GGPROTO::SendFile(MCONTACT hContact, const PROTOCHAR* szDescription, PROTOCHAR** ppszFiles)
{
	char *bslash, *filename;
	struct gg_dcc *dcc;
	DWORD ip, ver;
	WORD port;
	uin_t myuin, uin;

	// Check if main dcc thread is on
	if (!isonline())
		return ftfail(this, hContact);

	filename = mir_t2a(ppszFiles[0]);

	// Read user IP and port
	ip = swap32(getDword(hContact, GG_KEY_CLIENTIP, 0));
	port = getWord(hContact, GG_KEY_CLIENTPORT, 0);
	myuin = getDword(GG_KEY_UIN, 0);
	uin = getDword(hContact, GG_KEY_UIN, 0);
	ver = getDword(hContact, GG_KEY_CLIENTVERSION, 0);

	// Use DCC7 if a contact is using at least version 7.6 or unknown version
	if ((ver & 0x00ffffff) >= 0x29 || !ver) {
		struct gg_dcc7 *dcc7;

		gg_EnterCriticalSection(&sess_mutex, "SendFile", 46, "sess_mutex", 1);
		if (!(dcc7 = gg_dcc7_send_file(sess, uin, filename, NULL, NULL))) {
			gg_LeaveCriticalSection(&sess_mutex, "SendFile", 46, 1, "sess_mutex", 1);
			debugLogA("SendFile(): Failed to send file \"%s\".", filename);
			mir_free(filename);
			return ftfail(this, hContact);
		}
		gg_LeaveCriticalSection(&sess_mutex, "SendFile", 46, 2, "sess_mutex", 1);

		debugLogA("SendFile(): Sending file \"%s\" to %d.", filename, uin);

		// Add dcc to watches
		list_add(&watches, dcc7, 0);

		// Store handle
		dcc7->contact = (void*)hContact;
		dcc7->folder = _strdup(filename);
		dcc7->tick = 0;
		// Make folder name
		bslash = strrchr(dcc7->folder, '\\');
		if (bslash)
			*(bslash + 1) = 0;
		else
			*(dcc7->folder) = 0;
		mir_free(filename);
		return dcc7;
	}

	// Return if bad connection info
	if (!port || !uin || !myuin)
	{
		debugLogA("SendFile(): Bad contact uin or my uin. Exit.");
		mir_free(filename);
		return ftfail(this, hContact);
	}

	// Try to connect if not ask user to connect me
	if ((ip && port >= 10 && !(dcc = gg_dcc_send_file(ip, port, myuin, uin))) || (port < 10 && port > 0))
	{
		// Make fake dcc structure
		dcc = (gg_dcc*)malloc(sizeof(struct gg_dcc));
		memset(dcc, 0, sizeof(struct gg_dcc));
		// Fill up structures
		dcc->uin = myuin;
		dcc->peer_uin = uin;
		dcc->fd = -1;
		dcc->type = GG_SESSION_DCC_SEND;
		debugLogA("SendFile(): Requesting user to connect us and scheduling gg_dcc struct for a later use.");
		gg_EnterCriticalSection(&sess_mutex, "SendFile", 47, "sess_mutex", 1);
		gg_dcc_request(sess, uin);
		gg_LeaveCriticalSection(&sess_mutex, "SendFile", 47, 1, "sess_mutex", 1);
		list_add(&requests, dcc, 0);
	}

	// Write filename
	if (gg_dcc_fill_file_info(dcc, filename) == -1)
	{
		debugLogA("SendFile(): Cannot open and file fileinfo \"%s\".", filename);
		gg_free_dcc(dcc);
		mir_free(filename);
		return ftfail(this, hContact);
	}

	debugLogA("SendFile(): Sending file \"%s\" to %d in %s mode.", filename, uin, (dcc->fd != -1) ? "active" : "passive");

	// Add dcc to watches if not passive
	if (dcc->fd != -1) list_add(&watches, dcc, 0);

	// Store handle
	dcc->contact = (void*)hContact;
	dcc->folder = _strdup(filename);
	dcc->tick = 0;
	// Make folder name
	bslash = strrchr(dcc->folder, '\\');
	if (bslash)
		*(bslash + 1) = 0;
	else
		*(dcc->folder) = 0;

	mir_free(filename);
	return dcc;
}

