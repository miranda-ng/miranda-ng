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

#include "tlen.h"

#ifndef _TLEN_P2P_OLD_H_
#define _TLEN_P2P_OLD_H_

enum {
	TLEN_FILE_PACKET_CONNECTION_REQUEST = 0x01,
	TLEN_FILE_PACKET_CONNECTION_REQUEST_ACK = 0x02,
	TLEN_FILE_PACKET_FILE_LIST = 0x32,
	TLEN_FILE_PACKET_FILE_LIST_ACK = 0x33,
	TLEN_FILE_PACKET_FILE_REQUEST = 0x34,
	TLEN_FILE_PACKET_FILE_DATA = 0x35,
	TLEN_FILE_PACKET_END_OF_FILE = 0x37,
	TLEN_VOICE_PACKET = 0x96
};

typedef struct {
	unsigned int maxDataLen;
	char *packet;
	DWORD type;
	DWORD len;
} TLEN_FILE_PACKET;

typedef struct {
	char	szHost[256];
	int		wPort;
	int		useAuth;
	char	szUser[256];
	char	szPassword[256];
}SOCKSBIND;

extern void TlenP2PFreeFileTransfer(TLEN_FILE_TRANSFER *ft);
extern TLEN_FILE_PACKET *TlenP2PPacketCreate(int datalen);
extern void TlenP2PPacketFree(TLEN_FILE_PACKET *packet);
extern void TlenP2PPacketSetType(TLEN_FILE_PACKET *packet, DWORD type);
extern void TlenP2PPacketSetLen(TLEN_FILE_PACKET *packet, DWORD len);
extern void TlenP2PPacketPackDword(TLEN_FILE_PACKET *packet, DWORD data);
extern void TlenP2PPacketPackBuffer(TLEN_FILE_PACKET *packet, char *buffer, int len);
extern int TlenP2PPacketSend(HANDLE s, TLEN_FILE_PACKET *packet);
extern TLEN_FILE_PACKET* TlenP2PPacketReceive(HANDLE s);
extern void TlenP2PEstablishOutgoingConnection(TLEN_FILE_TRANSFER *ft, BOOL sendAck);
extern TLEN_FILE_TRANSFER* TlenP2PEstablishIncomingConnection(TlenProtocol *proto, HANDLE s, TLEN_LIST list, BOOL sendAck);
extern HANDLE TlenP2PListen(TLEN_FILE_TRANSFER *ft);
extern void TlenP2PStopListening(HANDLE s);

void __cdecl TlenProcessP2P(XmlNode *node, void *userdata);


#endif
