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
#include "tlen_p2p_old.h"
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tlen_list.h"


void TlenP2PFreeFileTransfer(TLEN_FILE_TRANSFER *ft)
{
	int i;
	if (ft->jid) mir_free(ft->jid);
	if (ft->iqId) mir_free(ft->iqId);
	if (ft->id2) mir_free(ft->id2);
	if (ft->hostName) mir_free(ft->hostName);
	if (ft->localName) mir_free(ft->localName);
	if (ft->szSavePath) mir_free(ft->szSavePath);
	if (ft->szDescription) mir_free(ft->szDescription);
	if (ft->filesSize) mir_free(ft->filesSize);
	if (ft->files) {
		for (i=0; i<ft->fileCount; i++) {
			if (ft->files[i]) mir_free(ft->files[i]);
		}
		mir_free(ft->files);
	}
	mir_free(ft);
}

TLEN_FILE_PACKET *TlenP2PPacketCreate(int datalen)
{
	TLEN_FILE_PACKET *packet;

	if ((packet=(TLEN_FILE_PACKET *) mir_alloc(sizeof(TLEN_FILE_PACKET))) == NULL)
		return NULL;
	packet->packet = NULL;
	if (datalen > 0) {
		if ((packet->packet=(char *) mir_alloc(datalen)) == NULL) {
			mir_free(packet);
			return NULL;
		}
	}
	packet->maxDataLen = datalen;
	packet->type=0;
	packet->len=0;
	return packet;
}

void TlenP2PPacketFree(TLEN_FILE_PACKET *packet)
{
	if (packet != NULL) {
		if (packet->packet != NULL)
			mir_free(packet->packet);
		mir_free(packet);
	}
}

void TlenP2PPacketSetType(TLEN_FILE_PACKET *packet, DWORD type)
{
	if (packet != NULL) {
		packet->type = type;
	}
}

void TlenP2PPacketSetLen(TLEN_FILE_PACKET *packet, DWORD len)
{
	if (packet != NULL) {
		packet->len = len;
	}
}

void TlenP2PPacketPackDword(TLEN_FILE_PACKET *packet, DWORD data)
{
	if (packet != NULL && packet->packet != NULL) {
		if (packet->len + sizeof(DWORD) <= packet->maxDataLen) {
			(*((DWORD*)((packet->packet)+(packet->len)))) = data;
			packet->len += sizeof(DWORD);
		}
	}
}

void TlenP2PPacketPackBuffer(TLEN_FILE_PACKET *packet, char *buffer, int len)
{
	if (packet != NULL && packet->packet != NULL) {
		if (packet->len + len <= packet->maxDataLen) {
			memcpy((packet->packet)+(packet->len), buffer, len);
			packet->len += len;
		}
	}
}

void TlenP2PInit() {
}

void TlenP2PUninit() {
}

int TlenP2PPacketSend(HANDLE s, TLEN_FILE_PACKET *packet)
{
	DWORD sendResult;
	if (packet != NULL && packet->packet != NULL) {
		Netlib_Send(s, (char *)&packet->type, 4, MSG_NODUMP);
		Netlib_Send(s, (char *)&packet->len, 4, MSG_NODUMP);
		sendResult=Netlib_Send(s, packet->packet, packet->len, MSG_NODUMP);
		if (sendResult == SOCKET_ERROR || sendResult != packet->len) return 0;
	}
	return 1;
}

TLEN_FILE_PACKET* TlenP2PPacketReceive(HANDLE s)
{
	TLEN_FILE_PACKET *packet;
	DWORD recvResult;
	DWORD type, len, pos;
	recvResult = Netlib_Recv(s, (char *)&type, 4, MSG_NODUMP);
	if (recvResult == 0 || recvResult == SOCKET_ERROR) return NULL;
	recvResult = Netlib_Recv(s, (char *)&len, 4, MSG_NODUMP);
	if (recvResult == 0 || recvResult == SOCKET_ERROR) return NULL;
	packet = TlenP2PPacketCreate(len);
	TlenP2PPacketSetType(packet, type);
	TlenP2PPacketSetLen(packet, len);
	pos = 0;
	while (len > 0) {
		recvResult = Netlib_Recv(s, packet->packet+pos, len, MSG_NODUMP);
		if (recvResult == 0 || recvResult == SOCKET_ERROR) {
			TlenP2PPacketFree(packet);
			return NULL;
		}
		len -= recvResult;
		pos += recvResult;
	}
	return packet;
}

void TlenP2PEstablishOutgoingConnection(TLEN_FILE_TRANSFER *ft, BOOL sendAck)
{
	char *hash;
	char str[300];
	size_t srt_len;
	TLEN_FILE_PACKET *packet;
	TlenProtocol *proto = ft->proto;

	proto->debugLogA("Establishing outgoing connection.");
	ft->state = FT_ERROR;
	if ((packet = TlenP2PPacketCreate(2*sizeof(DWORD) + 20)) != NULL) {
		TlenP2PPacketSetType(packet, TLEN_FILE_PACKET_CONNECTION_REQUEST);
		TlenP2PPacketPackDword(packet, 1);
		TlenP2PPacketPackDword(packet, (DWORD) atoi(ft->iqId));
		srt_len = mir_snprintf(str, SIZEOF(str), "%08X%s%d", atoi(ft->iqId), proto->threadData->username, atoi(ft->iqId));
		hash = TlenSha1(str, (int)srt_len);
		TlenP2PPacketPackBuffer(packet, hash, 20);
		mir_free(hash);
		TlenP2PPacketSend(ft->s, packet);
		TlenP2PPacketFree(packet);
		packet = TlenP2PPacketReceive(ft->s);
		if (packet != NULL) {
			if (packet->type == TLEN_FILE_PACKET_CONNECTION_REQUEST_ACK) { // acknowledge
				if ((int)(*((DWORD*)packet->packet)) == atoi(ft->iqId)) {
					ft->state = FT_CONNECTING;
					if (sendAck) {
						ProtoBroadcastAck(proto->m_szModuleName, ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, ft, 0);
					}
				}
			}
			TlenP2PPacketFree(packet);
		}
	}
}

TLEN_FILE_TRANSFER* TlenP2PEstablishIncomingConnection(TlenProtocol *proto, HANDLE s, TLEN_LIST list, BOOL sendAck)
{
	TLEN_LIST_ITEM *item = NULL;
	TLEN_FILE_PACKET *packet;
	int i;
	char str[300];
	DWORD iqId;
	// TYPE: 0x1
	// LEN:
	// (DWORD) 0x1
	// (DWORD) id
	// (BYTE) hash[20]
	packet = TlenP2PPacketReceive(s);
	if (packet == NULL || packet->type != TLEN_FILE_PACKET_CONNECTION_REQUEST || packet->len<28) {
		if (packet != NULL) {
			TlenP2PPacketFree(packet);
		}
		return NULL;
	}
	iqId = *((DWORD *)(packet->packet+sizeof(DWORD)));
	i = 0;
	while ((i=TlenListFindNext(proto, list, i)) >= 0) {
		if ((item=TlenListGetItemPtrFromIndex(proto, i)) != NULL) {
			mir_snprintf(str, SIZEOF(str), "%d", iqId);
			if (!strcmp(item->ft->iqId, str)) {
				char *hash, *nick;
				int j;
				nick = TlenNickFromJID(item->ft->jid);
				j = mir_snprintf(str, SIZEOF(str), "%08X%s%d", iqId, nick, iqId);
				mir_free(nick);
				hash = TlenSha1(str, j);
				for (j=0;j<20;j++) {
					if (hash[j] != packet->packet[2*sizeof(DWORD)+j]) break;
				}
				mir_free(hash);
				if (j == 20) break;
			}
		}
		i++;
	}
	TlenP2PPacketFree(packet);
	if (i >=0) {
		if ((packet=TlenP2PPacketCreate(sizeof(DWORD))) != NULL) {
			// Send connection establishment acknowledgement
			TlenP2PPacketSetType(packet, TLEN_FILE_PACKET_CONNECTION_REQUEST_ACK);
			TlenP2PPacketPackDword(packet, (DWORD) atoi(item->ft->iqId));
			TlenP2PPacketSend(s, packet);
			TlenP2PPacketFree(packet);
			item->ft->state = FT_CONNECTING;
			if (sendAck) {
				ProtoBroadcastAck(item->ft->proto->m_szModuleName, item->ft->hContact, ACKTYPE_FILE, ACKRESULT_CONNECTED, item->ft, 0);
			}
			return item->ft;
		}
	}
	return NULL;
}

static void __cdecl TlenFileBindSocks4Thread(TLEN_FILE_TRANSFER* ft)
{
	BYTE buf[8];
	int status;

//	TlenLog("Waiting for the file to be sent via SOCKS...");
	status = Netlib_Recv(ft->s, (char*)buf, 8, MSG_NODUMP);
//	TlenLog("accepted connection !!!");
	if ( status == SOCKET_ERROR || status < 8 || buf[1] != 90) {
		status = 1;
	} else {
		status = 0;
	}
	if (!status) {
		ft->pfnNewConnectionV2(ft->s, 0, NULL);
	} else {
		if (ft->state != FT_SWITCH) {
			ft->state = FT_ERROR;
		}
	}
	ft->proto->debugLogA("Closing connection for this file transfer...");
//	Netlib_CloseHandle(ft->s);
	if (ft->hFileEvent != NULL)
		SetEvent(ft->hFileEvent);

}
static void __cdecl TlenFileBindSocks5Thread(TLEN_FILE_TRANSFER* ft)
{
	BYTE buf[256];
	int status;

//	TlenLog("Waiting for the file to be sent via SOCKS...");
	status = Netlib_Recv(ft->s, (char*)buf, sizeof(buf), MSG_NODUMP);
//	TlenLog("accepted connection !!!");
	if ( status == SOCKET_ERROR || status < 7 || buf[1] != 0) {
		status = 1;
	} else {
		status = 0;
	}
	if (!status) {
		ft->pfnNewConnectionV2(ft->s, 0, NULL);
	} else {
		if (ft->state != FT_SWITCH) {
			ft->state = FT_ERROR;
		}
	}
//	TlenLog("Closing connection for this file transfer...");
//	Netlib_CloseHandle(ft->s);
	if (ft->hFileEvent != NULL)
		SetEvent(ft->hFileEvent);

}

static HANDLE TlenP2PBindSocks4(SOCKSBIND * sb, TLEN_FILE_TRANSFER *ft)
{	//rfc1928
	int len;
	BYTE buf[256];
	int status;
	struct in_addr in;

	NETLIBOPENCONNECTION nloc = { sizeof(nloc) };
	nloc.szHost = sb->szHost;
	nloc.wPort = sb->wPort;
	HANDLE s = (HANDLE) CallService(MS_NETLIB_OPENCONNECTION, (WPARAM) ft->proto->hFileNetlibUser, (LPARAM) &nloc);
	if (s == NULL) {
//		TlenLog("Connection failed (%d), thread ended", WSAGetLastError());
		return NULL;
	}
	buf[0] = 4;  //socks4
	buf[1] = 2;  //2-bind, 1-connect
	*(PWORD)(buf+2) = htons(0); // port
	*(PDWORD)(buf+4) = INADDR_ANY;
	if (sb->useAuth) {
		strcpy((char*)buf+8, sb->szUser);
		len = (int)strlen(sb->szUser);
	} else {
		buf[8] = 0;
		len = 0;
	}
	len += 9;
	status = Netlib_Send(s, (char*)buf, len, MSG_NODUMP);
	if (status == SOCKET_ERROR || status < len) {
//		TlenLog("Send failed (%d), thread ended", WSAGetLastError());
		Netlib_CloseHandle(s);
		return NULL;
	}
	status = Netlib_Recv(s, (char*)buf, 8, MSG_NODUMP);
	if (status == SOCKET_ERROR || status < 8 || buf[1] != 90) {
//		TlenLog("SOCKS4 negotiation failed");
		Netlib_CloseHandle(s);
		return NULL;
	}
	status = Netlib_Recv(s, (char*)buf, sizeof(buf), MSG_NODUMP);
	if ( status == SOCKET_ERROR || status < 7 || buf[0] != 5 || buf[1] != 0) {
//		TlenLog("SOCKS5 request failed");
		Netlib_CloseHandle(s);
		return NULL;
	}
	in.S_un.S_addr = *(PDWORD)(buf+4);
	strcpy(sb->szHost, inet_ntoa(in));
	sb->wPort = htons(*(PWORD)(buf+2));
	ft->s = s;
	forkthread((void (__cdecl *)(void*))TlenFileBindSocks4Thread, 0, ft);
	return s;
}

static HANDLE TlenP2PBindSocks5(SOCKSBIND * sb, TLEN_FILE_TRANSFER *ft)
{	//rfc1928
	BYTE buf[512];
	int len, status;
	struct in_addr in;

	NETLIBOPENCONNECTION nloc = { sizeof(nloc) };
	nloc.szHost = sb->szHost;
	nloc.wPort = sb->wPort;
	HANDLE s = (HANDLE) CallService(MS_NETLIB_OPENCONNECTION, (WPARAM) ft->proto->hFileNetlibUser, (LPARAM) &nloc);
	if (s == NULL) {
		ft->proto->debugLogA("Connection failed (%d), thread ended", WSAGetLastError());
		return NULL;
	}
	buf[0] = 5;  //yep, socks5
	buf[1] = 1;  //one auth method
	buf[2] = sb->useAuth?2:0; // authorization
	status = Netlib_Send(s, (char*)buf, 3, MSG_NODUMP);
	if (status == SOCKET_ERROR || status < 3) {
		ft->proto->debugLogA("Send failed (%d), thread ended", WSAGetLastError());
		Netlib_CloseHandle(s);
		return NULL;
	}
	status = Netlib_Recv(s, (char*)buf, 2, MSG_NODUMP);
	if (status == SOCKET_ERROR || status < 2 || (buf[1] != 0 && buf[1] != 2)) {
		ft->proto->debugLogA("SOCKS5 negotiation failed");
		Netlib_CloseHandle(s);
		return NULL;
	}
	if (buf[1] == 2) {		//rfc1929
		int nUserLen, nPassLen;
		PBYTE pAuthBuf;

		nUserLen = (int)strlen(sb->szUser);
		nPassLen = (int)strlen(sb->szPassword);
		pAuthBuf = (PBYTE)mir_alloc(3+nUserLen+nPassLen);
		pAuthBuf[0] = 1;		//auth version
		pAuthBuf[1] = nUserLen;
		memcpy(pAuthBuf+2, sb->szUser, nUserLen);
		pAuthBuf[2+nUserLen]=nPassLen;
		memcpy(pAuthBuf+3+nUserLen,sb->szPassword,nPassLen);
		status = Netlib_Send(s, (char*)pAuthBuf, 3+nUserLen+nPassLen, MSG_NODUMP);
		mir_free(pAuthBuf);
		if (status == SOCKET_ERROR || status < 3 + nUserLen+nPassLen) {
			ft->proto->debugLogA("Send failed (%d), thread ended", WSAGetLastError());
			Netlib_CloseHandle(s);
			return NULL;
		}
		status = Netlib_Recv(s, (char*)buf, sizeof(buf), MSG_NODUMP);
		if (status == SOCKET_ERROR || status < 2 || buf[1] != 0) {
			ft->proto->debugLogA("SOCKS5 sub-negotiation failed");
			Netlib_CloseHandle(s);
			return NULL;
		}
	}

	{	PBYTE pInit;
		int nHostLen=4;
		DWORD hostIP=INADDR_ANY;
		pInit=(PBYTE)mir_alloc(6+nHostLen);
		pInit[0]=5;   //SOCKS5
		pInit[1]=2;   //bind
		pInit[2]=0;   //reserved
		pInit[3]=1;
		*(PDWORD)(pInit+4)=hostIP;
		*(PWORD)(pInit+4+nHostLen)=htons(0);
		status = Netlib_Send(s, (char*)pInit, 6+nHostLen, MSG_NODUMP);
		mir_free(pInit);
		if (status == SOCKET_ERROR || status < 6 + nHostLen) {
//			TlenLog("Send failed (%d), thread ended", WSAGetLastError());
			Netlib_CloseHandle(s);
			return NULL;
		}
	}
	status = Netlib_Recv(s, (char*)buf, sizeof(buf), MSG_NODUMP);
	if ( status == SOCKET_ERROR || status < 7 || buf[0] != 5 || buf[1] != 0) {
//		TlenLog("SOCKS5 request failed");
		Netlib_CloseHandle(s);
		return NULL;
	}
	if (buf[2] == 1) { // domain
		len = buf[4];
		memcpy(sb->szHost, buf+5, len);
		sb->szHost[len]=0;
		len += 4;
	} else { // ip address
		in.S_un.S_addr = *(PDWORD)(buf+4);
		strcpy(sb->szHost, inet_ntoa(in));
		len = 8;
	}
	sb->wPort = htons(*(PWORD)(buf+len));
	ft->s = s;

	forkthread((void (__cdecl *)(void*))TlenFileBindSocks5Thread, 0, ft);
	return s;
}


HANDLE TlenP2PListen(TLEN_FILE_TRANSFER *ft)
{
	NETLIBBIND nlb = {0};
	HANDLE s = NULL;
	int	  useProxy;
	DBVARIANT dbv;
	SOCKSBIND sb;
	struct in_addr in;
	TlenProtocol *proto = ft->proto;
	useProxy=0;
	if (ft->localName != NULL) mir_free(ft->localName);
	ft->localName = NULL;
	ft->wPort = 0;
	if (db_get_b(NULL, proto->m_szModuleName, "UseFileProxy", FALSE)) {
		if (!db_get(NULL, proto->m_szModuleName, "FileProxyHost", &dbv)) {
			strcpy(sb.szHost, dbv.pszVal);
			db_free(&dbv);
			sb.wPort = db_get_w(NULL, proto->m_szModuleName, "FileProxyPort", 0);
			sb.useAuth = FALSE;
			strcpy(sb.szUser, "");
			strcpy(sb.szPassword, "");
			if (db_get_b(NULL, proto->m_szModuleName, "FileProxyAuth", FALSE)) {
				sb.useAuth = TRUE;
				if (!db_get_s(NULL, proto->m_szModuleName, "FileProxyUsername", &dbv)) {
					strcpy(sb.szUser, dbv.pszVal);
					db_free(&dbv);
				}
				if (!db_get_s(NULL, proto->m_szModuleName, "FileProxyPassword", &dbv)) {
					strcpy(sb.szPassword, dbv.pszVal);
					db_free(&dbv);
				}
			}
			switch (db_get_w(NULL, proto->m_szModuleName, "FileProxyType", 0)) {
				case 0: // forwarding
					useProxy = 1;
					break;
				case 1: // socks4
					s = TlenP2PBindSocks4(&sb, ft);
					useProxy = 2;
					break;
				case 2: // socks5
					s = TlenP2PBindSocks5(&sb, ft);
					useProxy = 2;
					break;
			}
			ft->localName = mir_strdup(sb.szHost);
			ft->wPort = sb.wPort;
			ft->wLocalPort = sb.wPort;
		}
	}
	if (useProxy<2) {
		nlb.cbSize = sizeof(NETLIBBIND);
		nlb.pfnNewConnectionV2 = ft->pfnNewConnectionV2;
		nlb.wPort = 0;	// Use user-specified incoming port ranges, if available
		nlb.pExtra = proto;
		ft->proto->debugLogA("Calling MS_NETLIB_BINDPORT");
		s = (HANDLE) CallService(MS_NETLIB_BINDPORT, (WPARAM) ft->proto->m_hNetlibUser, (LPARAM) &nlb);
		ft->proto->debugLogA("listening on %d",s);
	}
	if (useProxy == 0) {
		in.S_un.S_addr = htonl(nlb.dwExternalIP);
		ft->localName = mir_strdup(inet_ntoa(in));
		ft->wPort = nlb.wPort;
		ft->wLocalPort = nlb.wExPort;
	}
	if (s != NULL) {
//		listenCount++;
	}
	return s;
}

void TlenP2PStopListening(HANDLE s) {
	if (s != NULL) {
//		listenCount--;
//		if (listenCount <= 0) {
//			Netlib_CloseHandle(s);
//		}
	}
}
