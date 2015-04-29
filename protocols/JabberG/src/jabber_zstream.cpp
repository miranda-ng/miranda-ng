/*

Jabber Protocol Plugin for Miranda NG

XEP-0138 (Stream Compression) implementation

Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Kostya Chukavin, Taras Zackrepa
Copyright (ñ) 2012-15 Miranda NG project

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

#include "stdafx.h"

BOOL ThreadData::zlibInit(void)
{
	proto->debugLogA("Zlib init...");
	zStreamIn.zalloc = Z_NULL;
	zStreamIn.zfree = Z_NULL;
	zStreamIn.opaque = Z_NULL;
	zStreamIn.next_in = Z_NULL;
	zStreamIn.avail_in = 0;

	zStreamOut.zalloc = Z_NULL;
	zStreamOut.zfree = Z_NULL;
	zStreamOut.opaque = Z_NULL;

	if (deflateInit(&zStreamOut, Z_BEST_COMPRESSION) != Z_OK) return FALSE;
	if (inflateInit(&zStreamIn) != Z_OK) return FALSE;

	zRecvReady = true;
	return TRUE;
}

void ThreadData::zlibUninit(void)
{
	deflateEnd(&zStreamOut);
	inflateEnd(&zStreamIn);
}

int ThreadData::zlibSend(char* data, int datalen)
{
	char send_data[ ZLIB_CHUNK_SIZE ];
	int bytesOut = 0;

	zStreamOut.avail_in = datalen;
	zStreamOut.next_in = (unsigned char*)data;

	do {
		zStreamOut.avail_out = ZLIB_CHUNK_SIZE;
		zStreamOut.next_out = (unsigned char*)send_data;

		switch (deflate(&zStreamOut, Z_SYNC_FLUSH)) {
			case Z_OK:         proto->debugLogA("Deflate: Z_OK");         break;
			case Z_BUF_ERROR:  proto->debugLogA("Deflate: Z_BUF_ERROR");  break;
			case Z_DATA_ERROR: proto->debugLogA("Deflate: Z_DATA_ERROR"); break;
			case Z_MEM_ERROR:  proto->debugLogA("Deflate: Z_MEM_ERROR");  break;
		}

		int len, send_datalen = ZLIB_CHUNK_SIZE - zStreamOut.avail_out;

		if ((len = sendws(send_data, send_datalen, MSG_NODUMP)) == SOCKET_ERROR || len != send_datalen) {
			proto->debugLogA("Netlib_Send() failed, error=%d", WSAGetLastError());
			return FALSE;
		}

		bytesOut += len;
	}
		while (zStreamOut.avail_out == 0);

	if (db_get_b(NULL, "Netlib", "DumpSent", TRUE) == TRUE)
		proto->debugLogA("(ZLIB) Data sent\n%s\n===OUT: %d(%d) bytes", data, datalen, bytesOut);

	return TRUE;
}

int ThreadData::zlibRecv(char* data, long datalen)
{
	if (zRecvReady) {
retry:
		zRecvDatalen = recvws(zRecvData, ZLIB_CHUNK_SIZE, MSG_NODUMP);
		if (zRecvDatalen == SOCKET_ERROR) {
			proto->debugLogA("Netlib_Recv() failed, error=%d", WSAGetLastError());
			return SOCKET_ERROR;
		}
		if (zRecvDatalen == 0)
			return 0;

		zStreamIn.avail_in = zRecvDatalen;
		zStreamIn.next_in = (Bytef*)zRecvData;
	}

	zStreamIn.avail_out = datalen;
	zStreamIn.next_out = (BYTE*)data;

	switch (inflate(&zStreamIn, Z_NO_FLUSH)) {
		case Z_OK:         proto->debugLogA("Inflate: Z_OK");         break;
		case Z_BUF_ERROR:  proto->debugLogA("Inflate: Z_BUF_ERROR");  break;
		case Z_DATA_ERROR: proto->debugLogA("Inflate: Z_DATA_ERROR"); break;
		case Z_MEM_ERROR:  proto->debugLogA("Inflate: Z_MEM_ERROR");  break;
	}

	int len = datalen - zStreamIn.avail_out;
	if (db_get_b(NULL, "Netlib", "DumpRecv", TRUE) == TRUE) {
		char* szLogBuffer = (char*)alloca(len+32);
		memcpy(szLogBuffer, data, len);
		szLogBuffer[ len ]='\0';
		proto->debugLogA("(ZLIB) Data received\n%s\n===IN: %d(%d) bytes", szLogBuffer, len, zRecvDatalen);
	}

	if (len == 0)
		goto retry;

	zRecvReady = (zStreamIn.avail_out != 0);
	return len;
}
