/*

Tlen Protocol Plugin for Miranda NG
Copyright (C) 2004-2007  Piotr Piastucki

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

#include "jabber.h"
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "jabber_list.h"
#include "tlen_p2p_old.h"

static void TlenFileReceiveParse(TLEN_FILE_TRANSFER *ft)
{
	int i;
	char *p;
	TLEN_FILE_PACKET *rpacket, *packet;

	rpacket = NULL;
	if (ft->state == FT_CONNECTING) {
		rpacket = TlenP2PPacketReceive(ft->s);
		if (rpacket != NULL) {
			p = rpacket->packet;
			if (rpacket->type == TLEN_FILE_PACKET_FILE_LIST) { // list of files (length & name)
				ft->fileCount = (int)(*((DWORD*)p));
				ft->files = (char **) mir_alloc(sizeof(char *) * ft->fileCount);
				ft->filesSize = (long *) mir_alloc(sizeof(long) * ft->fileCount);
				ft->currentFile = 0;
				ft->allFileTotalSize = 0;
				ft->allFileReceivedBytes = 0;
				p += sizeof(DWORD);
				for (i=0;i<ft->fileCount;i++) {
					ft->filesSize[i] = (long)(*((DWORD*)p));
					ft->allFileTotalSize += ft->filesSize[i];
					p += sizeof(DWORD);
					ft->files[i] = (char *)mir_alloc(256);
					memcpy(ft->files[i], p, 256);
					p += 256;
				}
				if ((packet=TlenP2PPacketCreate(3*sizeof(DWORD))) == NULL) {
					ft->state = FT_ERROR;
				}
				else {
					TlenP2PPacketSetType(packet, TLEN_FILE_PACKET_FILE_LIST_ACK);
					TlenP2PPacketSend(ft->s, packet);
					TlenP2PPacketFree(packet);
					ft->state = FT_INITIALIZING;
					JabberLog(ft->proto, "Change to FT_INITIALIZING");
				}
			}
			TlenP2PPacketFree(rpacket);
		}
		else {
			ft->state = FT_ERROR;
		}
	}
	else if (ft->state == FT_INITIALIZING) {
		char *fullFileName;
		if ((packet=TlenP2PPacketCreate(3*sizeof(DWORD))) != NULL) {
			TlenP2PPacketSetType(packet, TLEN_FILE_PACKET_FILE_REQUEST); // file request
			TlenP2PPacketPackDword(packet, ft->currentFile);
			TlenP2PPacketPackDword(packet, 0);
			TlenP2PPacketPackDword(packet, 0);
			TlenP2PPacketSend(ft->s, packet);
			TlenP2PPacketFree(packet);

			fullFileName = (char *) mir_alloc(strlen(ft->szSavePath) + strlen(ft->files[ft->currentFile]) + 2);
			strcpy(fullFileName, ft->szSavePath);
			if (fullFileName[strlen(fullFileName)-1] != '\\')
				strcat(fullFileName, "\\");
			strcat(fullFileName, ft->files[ft->currentFile]);
			ft->fileId = _open(fullFileName, _O_BINARY|_O_WRONLY|_O_CREAT|_O_TRUNC, _S_IREAD|_S_IWRITE);
			ft->fileReceivedBytes = 0;
			ft->fileTotalSize = ft->filesSize[ft->currentFile];
			JabberLog(ft->proto, "Saving to [%s] [%d]", fullFileName, ft->filesSize[ft->currentFile]);
			mir_free(fullFileName);
			ft->state = FT_RECEIVING;
			JabberLog(ft->proto, "Change to FT_RECEIVING");
		}
		else {
			ft->state = FT_ERROR;
		}
	}
	else if (ft->state == FT_RECEIVING) {
		PROTOFILETRANSFERSTATUS pfts;
		memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
		pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
		pfts.hContact = ft->hContact;
		pfts.pszFiles = ft->files;
		pfts.totalFiles = ft->fileCount;
		pfts.currentFileNumber = ft->currentFile;
		pfts.totalBytes = ft->allFileTotalSize;
		pfts.szWorkingDir = NULL;
		pfts.szCurrentFile = ft->files[ft->currentFile];
		pfts.currentFileSize = ft->filesSize[ft->currentFile];
		pfts.currentFileTime = 0;
		JabberLog(ft->proto, "Receiving data...");
		while (ft->state == FT_RECEIVING) {
			rpacket = TlenP2PPacketReceive(ft->s);
			if (rpacket != NULL) {
				p = rpacket->packet;
				if (rpacket->type == TLEN_FILE_PACKET_FILE_DATA) { // file data
					int writeSize;
					writeSize = rpacket->len - 2 * sizeof(DWORD) ; // skip file offset
					if (_write(ft->fileId, p + 2 * sizeof(DWORD), writeSize) != writeSize) {
						ft->state = FT_ERROR;
					}
					else {
						ft->fileReceivedBytes += writeSize;
						ft->allFileReceivedBytes += writeSize;
						pfts.totalProgress = ft->allFileReceivedBytes;
						pfts.currentFileProgress = ft->fileReceivedBytes;
						ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM) &pfts);
					}
				}
				else if (rpacket->type == TLEN_FILE_PACKET_END_OF_FILE) { // end of file
					_close(ft->fileId);
					JabberLog(ft->proto, "Finishing this file...");
					if (ft->currentFile >= ft->fileCount-1) {
						ft->state = FT_DONE;
					}
					else {
						ft->currentFile++;
						ft->state = FT_INITIALIZING;
						JabberLog(ft->proto, "File received, advancing to the next file...");
						ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
					}
				}
				TlenP2PPacketFree(rpacket);
			}
			else {
				ft->state = FT_ERROR;
			}
		}
	}
}

static void TlenFileReceivingConnection(JABBER_SOCKET hConnection, DWORD dwRemoteIP, void * pExtra)
{
	JABBER_SOCKET slisten;
	TLEN_FILE_TRANSFER *ft;

	TlenProtocol *proto = (TlenProtocol *)pExtra;
	ft = TlenP2PEstablishIncomingConnection(proto, hConnection, LIST_FILE, TRUE);
	if (ft != NULL) {
		slisten = ft->s;
		ft->s = hConnection;
		JabberLog(ft->proto, "Set ft->s to %d (saving %d)", hConnection, slisten);
		JabberLog(ft->proto, "Entering send loop for this file connection... (ft->s is hConnection)");
		while (ft->state != FT_DONE && ft->state != FT_ERROR) {
			TlenFileReceiveParse(ft);
		}
		if (ft->state == FT_DONE)
			ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
		else
			ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
		JabberLog(ft->proto, "Closing connection for this file transfer... (ft->s is now hBind)");
		ft->s = slisten;
		JabberLog(ft->proto, "ft->s is restored to %d", ft->s);
		if (ft->s != hConnection) {
			Netlib_CloseHandle(hConnection);
		}
		if (ft->hFileEvent != NULL)
			SetEvent(ft->hFileEvent);
	} else {
		Netlib_CloseHandle(hConnection);
	}
}


static void __cdecl TlenFileReceiveThread(TLEN_FILE_TRANSFER *ft)
{
	NETLIBOPENCONNECTION nloc;
	JABBER_SOCKET s;
	JabberLog(ft->proto, "Thread started: type=file_receive server='%s' port='%d'", ft->hostName, ft->wPort);
	ft->mode = FT_RECV;
	nloc.cbSize = NETLIBOPENCONNECTION_V1_SIZE;//sizeof(NETLIBOPENCONNECTION);
	nloc.szHost = ft->hostName;
	nloc.wPort = ft->wPort;
	nloc.flags = 0;
	ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, ft, 0);
	s = (HANDLE) CallService(MS_NETLIB_OPENCONNECTION, (WPARAM) ft->proto->hNetlibUser, (LPARAM) &nloc);
	if (s != NULL) {
		ft->s = s;
		JabberLog(ft->proto, "Entering file receive loop");
		TlenP2PEstablishOutgoingConnection(ft, TRUE);
		while (ft->state != FT_DONE && ft->state != FT_ERROR) {
			TlenFileReceiveParse(ft);
		}
		if (ft->s) {
			Netlib_CloseHandle(s);
		}
		ft->s = NULL;
	} else {
		ft->pfnNewConnectionV2 = TlenFileReceivingConnection;
		JabberLog(ft->proto, "Connection failed - receiving as server");
		s = TlenP2PListen(ft);
		if (s != NULL) {
			HANDLE hEvent;
			char *nick;
			ft->s = s;
			hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			ft->hFileEvent = hEvent;
			ft->currentFile = 0;
			ft->state = FT_CONNECTING;
			nick = JabberNickFromJID(ft->jid);
			JabberSend(ft->proto, "<f t='%s' i='%s' e='7' a='%s' p='%d'/>", nick, ft->iqId, ft->localName, ft->wLocalPort);
			mir_free(nick);
			JabberLog(ft->proto, "Waiting for the file to be received...");
			WaitForSingleObject(hEvent, INFINITE);
			ft->hFileEvent = NULL;
			CloseHandle(hEvent);
			JabberLog(ft->proto, "Finish all files");
			Netlib_CloseHandle(s);
		} else {
			ft->state = FT_ERROR;
		}
	}
	JabberListRemove(ft->proto, LIST_FILE, ft->iqId);
	if (ft->state == FT_DONE)
		ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
	else {
		char *nick;
		nick = JabberNickFromJID(ft->jid);
		JabberSend(ft->proto, "<f t='%s' i='%s' e='8'/>", nick, ft->iqId);
		mir_free(nick);
		ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
	}

	JabberLog(ft->proto, "Thread ended: type=file_receive server='%s'", ft->hostName);
	TlenP2PFreeFileTransfer(ft);
}



static void TlenFileSendParse(TLEN_FILE_TRANSFER *ft)
{
	int i;
	char *p, *t;
	int currentFile, numRead;
	char *fileBuffer;
	TLEN_FILE_PACKET *rpacket, *packet;


	if (ft->state == FT_CONNECTING) {
		char filename[256];	// Must be 256 (0x100)
		if ((packet=TlenP2PPacketCreate(sizeof(DWORD)+(ft->fileCount*(sizeof(filename)+sizeof(DWORD))))) != NULL) {
			// Must pause a bit, sending these two packets back to back
			// will break the session because the receiver cannot take it :)
			SleepEx(1000, TRUE);
			TlenP2PPacketSetLen(packet, 0); // Reuse packet
			TlenP2PPacketSetType(packet, TLEN_FILE_PACKET_FILE_LIST);
			TlenP2PPacketPackDword(packet, (DWORD) ft->fileCount);
			for (i=0; i<ft->fileCount; i++) {
//					struct _stat statbuf;
//					_stat(ft->files[i], &statbuf);
//					TlenP2PPacketPackDword(packet, statbuf.st_size);
				TlenP2PPacketPackDword(packet, ft->filesSize[i]);
				memset(filename, 0, sizeof(filename));
				if ((t=strrchr(ft->files[i], '\\')) != NULL)
					t++;
				else
					t = ft->files[i];
				mir_snprintf(filename, sizeof(filename)-1, t);
				TlenP2PPacketPackBuffer(packet, filename, sizeof(filename));
			}
			TlenP2PPacketSend(ft->s, packet);
			TlenP2PPacketFree(packet);

			ft->allFileReceivedBytes = 0;
			ft->state = FT_INITIALIZING;
			JabberLog(ft->proto, "Change to FT_INITIALIZING");
		}
		else {
			ft->state = FT_ERROR;
		}
	}
	else if (ft->state == FT_INITIALIZING) {	// FT_INITIALIZING
		rpacket = TlenP2PPacketReceive(ft->s);
		JabberLog(ft->proto, "FT_INITIALIZING: recv %d", rpacket);
		if (rpacket == NULL) {
			ft->state = FT_ERROR;
			return;
		}
		JabberLog(ft->proto, "FT_INITIALIZING: recv type %d", rpacket->type);
		p = rpacket->packet;
		// TYPE: TLEN_FILE_PACKET_FILE_LIST_ACK	will be ignored
		// LEN: 0
		if (rpacket->type == TLEN_FILE_PACKET_FILE_LIST_ACK) {

		}
		// Then the receiver will request each file
		// TYPE: TLEN_FILE_PACKET_REQUEST
		// LEN:
		// (DWORD) file number
		// (DWORD) 0
		// (DWORD) 0
		else if (rpacket->type == TLEN_FILE_PACKET_FILE_REQUEST) {
			PROTOFILETRANSFERSTATUS pfts;
			//struct _stat statbuf;

			currentFile = *((DWORD*)p);
			if (currentFile != ft->currentFile) {
				JabberLog(ft->proto, "Requested file (#%d) is invalid (must be %d)", currentFile, ft->currentFile);
				ft->state = FT_ERROR;
			}
			else {
			//	_stat(ft->files[currentFile], &statbuf);	// file size in statbuf.st_size
				JabberLog(ft->proto, "Sending [%s] [%d]", ft->files[currentFile], ft->filesSize[currentFile]);
				if ((ft->fileId=_open(ft->files[currentFile], _O_BINARY|_O_RDONLY)) < 0) {
					JabberLog(ft->proto, "File cannot be opened");
					ft->state = FT_ERROR;
				}
				else  {
					memset(&pfts, 0, sizeof(PROTOFILETRANSFERSTATUS));
					pfts.cbSize = sizeof(PROTOFILETRANSFERSTATUS);
					pfts.hContact = ft->hContact;
					pfts.flags = PFTS_SENDING;
					pfts.pszFiles = ft->files;
					pfts.totalFiles = ft->fileCount;
					pfts.currentFileNumber = ft->currentFile;
					pfts.totalBytes = ft->allFileTotalSize;
					pfts.szWorkingDir = NULL;
					pfts.szCurrentFile = ft->files[ft->currentFile];
					pfts.currentFileSize = ft->filesSize[ft->currentFile]; //statbuf.st_size;
					pfts.currentFileTime = 0;
					ft->fileReceivedBytes = 0;
					if ((packet = TlenP2PPacketCreate(2*sizeof(DWORD)+2048)) == NULL) {
						ft->state = FT_ERROR;
					}
					else {
						TlenP2PPacketSetType(packet, TLEN_FILE_PACKET_FILE_DATA);
						fileBuffer = (char *) mir_alloc(2048);
						JabberLog(ft->proto, "Sending file data...");
						while ((numRead=_read(ft->fileId, fileBuffer, 2048)) > 0) {
							TlenP2PPacketSetLen(packet, 0); // Reuse packet
							TlenP2PPacketPackDword(packet, (DWORD) ft->fileReceivedBytes);
							TlenP2PPacketPackDword(packet, 0);
							TlenP2PPacketPackBuffer(packet, fileBuffer, numRead);
							if (TlenP2PPacketSend(ft->s, packet)) {
								ft->fileReceivedBytes += numRead;
								ft->allFileReceivedBytes += numRead;
								pfts.totalProgress = ft->allFileReceivedBytes;
								pfts.currentFileProgress = ft->fileReceivedBytes;
								ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM) &pfts);
							}
							else {
								ft->state = FT_ERROR;
								break;
							}
						}
						mir_free(fileBuffer);
						_close(ft->fileId);
						if (ft->state != FT_ERROR) {
							if (ft->currentFile >= ft->fileCount-1)
								ft->state = FT_DONE;
							else {
								ft->currentFile++;
								ft->state = FT_INITIALIZING;
								JabberLog(ft->proto, "File sent, advancing to the next file...");
								ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
							}
						}
						JabberLog(ft->proto, "Finishing this file...");
						TlenP2PPacketSetLen(packet, 0); // Reuse packet
						TlenP2PPacketSetType(packet, TLEN_FILE_PACKET_END_OF_FILE);
						TlenP2PPacketPackDword(packet, currentFile);
						TlenP2PPacketSend(ft->s, packet);
						TlenP2PPacketFree(packet);
					}
				}
			}
			TlenP2PPacketFree(rpacket);
		}
		else {
			ft->state = FT_ERROR;
		}
	}
}

static void TlenFileSendingConnection(JABBER_SOCKET hConnection, DWORD dwRemoteIP, void * pExtra)
{
	JABBER_SOCKET slisten;
	TLEN_FILE_TRANSFER *ft;
	TlenProtocol *proto = (TlenProtocol *)pExtra;

	ft = TlenP2PEstablishIncomingConnection(proto, hConnection, LIST_FILE, TRUE);
	if (ft != NULL) {
		slisten = ft->s;
		ft->s = hConnection;
		JabberLog(ft->proto, "Set ft->s to %d (saving %d)", hConnection, slisten);

		JabberLog(ft->proto, "Entering send loop for this file connection... (ft->s is hConnection)");
		while (ft->state != FT_DONE && ft->state != FT_ERROR) {
			TlenFileSendParse(ft);
		}
		if (ft->state == FT_DONE)
			ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
		else
			ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
		JabberLog(ft->proto, "Closing connection for this file transfer... (ft->s is now hBind)");
		ft->s = slisten;
		JabberLog(ft->proto, "ft->s is restored to %d", ft->s);
		if (ft->s != hConnection) {
			Netlib_CloseHandle(hConnection);
		}
		if (ft->hFileEvent != NULL)
			SetEvent(ft->hFileEvent);
	} else {
		Netlib_CloseHandle(hConnection);
	}
}

int TlenFileCancelAll(TlenProtocol *proto)
{
	JABBER_LIST_ITEM *item;
	HANDLE hEvent;
	int i = 0;

	while ((i=JabberListFindNext(proto, LIST_FILE, 0)) >=0 ) {
		if ((item=JabberListGetItemPtrFromIndex(proto, i)) != NULL) {
			TLEN_FILE_TRANSFER *ft = item->ft;
			JabberListRemoveByIndex(proto, i);
			if (ft != NULL) {
				if (ft->s) {
					//ProtoBroadcastAck(m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
					JabberLog(ft->proto, "Closing ft->s = %d", ft->s);
					ft->state = FT_ERROR;
					Netlib_CloseHandle(ft->s);
					ft->s = NULL;
					if (ft->hFileEvent != NULL) {
						hEvent = ft->hFileEvent;
						ft->hFileEvent = NULL;
						SetEvent(hEvent);
					}
				} else {
					JabberLog(ft->proto, "freeing ft struct");
					TlenP2PFreeFileTransfer(ft);
				}
			}
		}
	}
	return 0;
}

static void __cdecl TlenFileSendingThread(TLEN_FILE_TRANSFER *ft)
{
	JABBER_SOCKET s = NULL;
	HANDLE hEvent;
	char *nick;

	JabberLog(ft->proto, "Thread started: type=tlen_file_send");
	ft->mode = FT_SEND;
	ft->pfnNewConnectionV2 = TlenFileSendingConnection;
	s = TlenP2PListen(ft);
	if (s != NULL) {
		ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, ft, 0);
		ft->s = s;
		//JabberLog("ft->s = %d", s);
		//JabberLog("fileCount = %d", ft->fileCount);

		hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		ft->hFileEvent = hEvent;
		ft->currentFile = 0;
		ft->state = FT_CONNECTING;

		nick = JabberNickFromJID(ft->jid);
		JabberSend(ft->proto, "<f t='%s' i='%s' e='6' a='%s' p='%d'/>", nick, ft->iqId, ft->localName, ft->wLocalPort);
		mir_free(nick);
		JabberLog(ft->proto, "Waiting for the file to be sent...");
		WaitForSingleObject(hEvent, INFINITE);
		ft->hFileEvent = NULL;
		CloseHandle(hEvent);
		JabberLog(ft->proto, "Finish all files");
		Netlib_CloseHandle(s);
		ft->s = NULL;
		JabberLog(ft->proto, "ft->s is NULL");

		if (ft->state == FT_SWITCH) {
			NETLIBOPENCONNECTION nloc;
			JABBER_SOCKET s;
			JabberLog(ft->proto, "Sending as client...");
			ft->state = FT_CONNECTING;
			nloc.cbSize = NETLIBOPENCONNECTION_V1_SIZE;//sizeof(NETLIBOPENCONNECTION);
			nloc.szHost = ft->hostName;
			nloc.wPort = ft->wPort;
			nloc.flags = 0;
			s = (HANDLE) CallService(MS_NETLIB_OPENCONNECTION, (WPARAM) ft->proto->hNetlibUser, (LPARAM) &nloc);
			if (s != NULL) {
				ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTING, ft, 0);
				ft->s = s;
				TlenP2PEstablishOutgoingConnection(ft, TRUE);
				JabberLog(ft->proto, "Entering send loop for this file connection...");
				while (ft->state != FT_DONE && ft->state != FT_ERROR) {
					TlenFileSendParse(ft);
				}
				if (ft->state == FT_DONE)
					ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
				else
					ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
				JabberLog(ft->proto, "Closing connection for this file transfer... ");
				Netlib_CloseHandle(s);
			} else {
				ft->state = FT_ERROR;
			}
		}
	} else {
		JabberLog(ft->proto, "Cannot allocate port to bind for file server thread, thread ended.");
		ft->state = FT_ERROR;
	}
	JabberListRemove(ft->proto, LIST_FILE, ft->iqId);
	switch (ft->state) {
	case FT_DONE:
		JabberLog(ft->proto, "Finish successfully");
		ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
		break;
	case FT_DENIED:
		ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_DENIED, ft, 0);
		break;
	default: // FT_ERROR:
		nick = JabberNickFromJID(ft->jid);
		JabberSend(ft->proto, "<f t='%s' i='%s' e='8'/>", nick, ft->iqId);
		mir_free(nick);
		JabberLog(ft->proto, "Finish with errors");
		ProtoBroadcastAck(ft->proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
		break;
	}
	JabberLog(ft->proto, "Thread ended: type=file_send");
	TlenP2PFreeFileTransfer(ft);
}


TLEN_FILE_TRANSFER *TlenFileCreateFT(TlenProtocol *proto, const char *jid) {
	TLEN_FILE_TRANSFER *ft;
	ft = (TLEN_FILE_TRANSFER *) mir_alloc(sizeof(TLEN_FILE_TRANSFER));
	memset(ft, 0, sizeof(TLEN_FILE_TRANSFER));
	ft->proto = proto;
	ft->jid = mir_strdup(jid);
	return ft;
}


/*
 * File transfer
 */
void TlenProcessF(XmlNode *node, ThreadData *info)
{
	TLEN_FILE_TRANSFER *ft;
	char *from, *p, *e;
	char jid[128], szFilename[MAX_PATH];
	int numFiles;
	JABBER_LIST_ITEM *item;

//	if (!node->name || strcmp(node->name, "f")) return;
	if (info == NULL) return;

	if ((from=JabberXmlGetAttrValue(node, "f")) != NULL) {

		if (strchr(from, '@') == NULL) {
			mir_snprintf(jid, sizeof(jid), "%s@%s", from, info->server);
		} else {
			mir_snprintf(jid, sizeof(jid), "%s", from);
		}
		if ((e=JabberXmlGetAttrValue(node, "e")) != NULL) {

			if (!strcmp(e, "1")) {
				// FILE_RECV : e='1' : File transfer request
				ft = TlenFileCreateFT(info->proto, jid);
				ft->hContact = JabberHContactFromJID(info->proto, jid);

				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL)
					ft->iqId = mir_strdup(p);

				szFilename[0] = '\0';
				if ((p=JabberXmlGetAttrValue(node, "c")) != NULL) {
					numFiles = atoi(p);
					if (numFiles == 1) {
						if ((p=JabberXmlGetAttrValue(node, "n")) != NULL) {
							p = JabberTextDecode(p);
							strncpy(szFilename, p, sizeof(szFilename));
							mir_free(p);
						} else {
							strcpy(szFilename, Translate("1 File"));
						}
					}
					else if (numFiles > 1) {
						mir_snprintf(szFilename, sizeof(szFilename), Translate("%d Files"), numFiles);
					}
				}

				if (szFilename[0] != '\0' && ft->iqId != NULL) {
					TCHAR* filenameT = mir_utf8decodeT((char*)szFilename);
					PROTORECVFILET pre = {0};
					pre.flags = PREF_TCHAR;
					pre.fileCount = 1;
					pre.timestamp = time(NULL);
					pre.tszDescription = filenameT;
					pre.ptszFiles = &filenameT;
					pre.lParam = (LPARAM)ft;
					JabberLog(ft->proto, "sending chainrecv");
					ProtoChainRecvFile(ft->hContact, &pre);
					mir_free(filenameT);
				} else {
					// malformed <f/> request, reject
					if (ft->iqId)
						JabberSend(ft->proto, "<f i='%s' e='4' t='%s'/>", ft->iqId, from);
					else
						JabberSend(ft->proto, "<f e='4' t='%s'/>", from);
					TlenP2PFreeFileTransfer(ft);
				}
			}
			else if (!strcmp(e, "3")) {
				// FILE_RECV : e='3' : invalid transfer error
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_FILE, p)) != NULL) {
						if (item->ft != NULL) {
							ProtoBroadcastAck(info->proto->m_szModuleName, item->ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, item->ft, 0);
							info->proto->FileCancel(NULL, item->ft);
						}
						JabberListRemove(info->proto, LIST_FILE, p);
					}
				}
			}
			else if (!strcmp(e, "4")) {
				// FILE_SEND : e='4' : File sending request was denied by the remote client
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_FILE, p)) != NULL) {
						if (!strcmp(item->ft->jid, jid)) {
							ProtoBroadcastAck(info->proto->m_szModuleName, item->ft->hContact, ACKTYPE_FILE, ACKRESULT_DENIED, item->ft, 0);
							JabberListRemove(info->proto, LIST_FILE, p);
						}
					}
				}
			}
			else if (!strcmp(e, "5")) {
				// FILE_SEND : e='5' : File sending request was accepted
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_FILE, p)) != NULL) {
						if (!strcmp(item->ft->jid, jid))
							JabberForkThread((void (__cdecl *)(void*))TlenFileSendingThread, 0, item->ft);
					}
				}
			}
			else if (!strcmp(e, "6")) {
				// FILE_RECV : e='6' : IP and port information to connect to get file
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_FILE, p)) != NULL) {
						if ((p=JabberXmlGetAttrValue(node, "a")) != NULL) {
							item->ft->hostName = mir_strdup(p);
							if ((p=JabberXmlGetAttrValue(node, "p")) != NULL) {
								item->ft->wPort = atoi(p);
								JabberForkThread((void (__cdecl *)(void*))TlenFileReceiveThread, 0, item->ft);
							}
						}
					}
				}
			}
			else if (!strcmp(e, "7")) {
				// FILE_RECV : e='7' : IP and port information to connect to send file
				// in case the conection to the given server was not successful
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_FILE, p)) != NULL) {
						if ((p=JabberXmlGetAttrValue(node, "a")) != NULL) {
							if (item->ft->hostName != NULL) mir_free(item->ft->hostName);
							item->ft->hostName = mir_strdup(p);
							if ((p=JabberXmlGetAttrValue(node, "p")) != NULL) {
								item->ft->wPort = atoi(p);
								item->ft->state = FT_SWITCH;
								SetEvent(item->ft->hFileEvent);
							}
						}
					}
				}
			}
			else if (!strcmp(e, "8")) {
				// FILE_RECV : e='8' : transfer error
				if ((p=JabberXmlGetAttrValue(node, "i")) != NULL) {
					if ((item=JabberListGetItemPtr(info->proto, LIST_FILE, p)) != NULL) {
						item->ft->state = FT_ERROR;
						if (item->ft->hFileEvent != NULL) {
							SetEvent(item->ft->hFileEvent);
						} else {
							ProtoBroadcastAck(info->proto->m_szModuleName, item->ft->hContact, ACKTYPE_FILE, ACKRESULT_FAILED, item->ft, 0);
						}
					}
				}
			}
		}
	}
}
