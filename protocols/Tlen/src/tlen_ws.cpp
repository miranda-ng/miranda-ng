/*

Jabber Protocol Plugin for Miranda IM
Tlen Protocol Plugin for Miranda NG
Copyright (C) 2002-2004  Santithorn Bunchua
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

BOOL TlenWsInit(TlenProtocol *proto)
{
	NETLIBUSER nlu = {0};
	NETLIBUSERSETTINGS nlus = {0};
	TCHAR name[128];


	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_TCHAR;	// | NUF_HTTPGATEWAY;
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s connection"), proto->m_tszUserName);
	nlu.ptszDescriptiveName = name;
	nlu.szSettingsModule = proto->m_szModuleName;
	proto->m_hNetlibUser = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) &nlu);

	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_NOOPTIONS | NUF_TCHAR;
	mir_sntprintf(name, SIZEOF(name), TranslateT("%s SOCKS connection"), proto->m_tszUserName);
	nlu.ptszDescriptiveName = name;
	proto->hFileNetlibUser = (HANDLE) CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM) &nlu);
	nlus.cbSize = sizeof(nlus);
	nlus.useProxy = 0;
	CallService(MS_NETLIB_SETUSERSETTINGS, (WPARAM) proto->hFileNetlibUser, (LPARAM) &nlus);

	return (proto->m_hNetlibUser != NULL)?TRUE:FALSE;
}

void TlenWsUninit(TlenProtocol *proto)
{
	if (proto->m_hNetlibUser != NULL) Netlib_CloseHandle(proto->m_hNetlibUser);
	if (proto->hFileNetlibUser != NULL) Netlib_CloseHandle(proto->hFileNetlibUser);
	proto->m_hNetlibUser = NULL;
	proto->hFileNetlibUser = NULL;
}

HANDLE TlenWsConnect(TlenProtocol *proto, char *host, WORD port)
{
	NETLIBOPENCONNECTION nloc = {0};

	nloc.cbSize = sizeof(NETLIBOPENCONNECTION); //NETLIBOPENCONNECTION_V1_SIZE;
	nloc.szHost = host;
	nloc.wPort = port;
	nloc.flags = 0;
	nloc.timeout = 6;
	return (HANDLE) CallService(MS_NETLIB_OPENCONNECTION, (WPARAM) proto->m_hNetlibUser, (LPARAM) &nloc);
}


int TlenWsSend(TlenProtocol *proto, HANDLE s, char *data, int datalen)
{
	int len;
	if ((len=Netlib_Send(s, data, datalen, /*MSG_NODUMP|*/MSG_DUMPASTEXT)) == SOCKET_ERROR || len != datalen) {
	proto->debugLogA("Netlib_Send() failed, error=%d", WSAGetLastError());
		return FALSE;
	}
	return TRUE;
}

int TlenWsRecv(TlenProtocol *proto, HANDLE s, char *data, long datalen)
{
	int ret;
	ret = Netlib_Recv(s, data, datalen, /*MSG_NODUMP|*/MSG_DUMPASTEXT);
	if (ret == SOCKET_ERROR) {
		proto->debugLogA("Netlib_Recv() failed, error=%d", WSAGetLastError());
		return 0;
	}
	if (ret == 0) {
		proto->debugLogA("Connection closed gracefully");
		return 0;
	}
	return ret;
}


int TlenWsSendAES(TlenProtocol *proto, char *data, int datalen, aes_context *aes_ctx, unsigned char *aes_iv)
{
	int len, sendlen;
	unsigned char aes_input[16];
	unsigned char aes_output[256];
	if (proto->threadData == NULL) {
		return FALSE;
	}
	while (datalen > 0) {
		len = 0;
		while (datalen > 0 && len < 256) {
			int pad = datalen < 16 ? 16 - datalen : 0;
			memcpy(aes_input, data, datalen < 16 ? datalen : 16);
			memset(aes_input + 16 - pad, ' ', pad);
			aes_crypt_cbc(aes_ctx, AES_ENCRYPT, 16, aes_iv, aes_input, aes_output + len);
			datalen -= 16;
			data += 16;
			len += 16;
		}
		if (len > 0) {
			proto->debugLogA("Sending %d bytes", len);
			if ((sendlen=Netlib_Send(proto->threadData->s, (char *)aes_output, len, MSG_NODUMP)) == SOCKET_ERROR || len != sendlen) {
				proto->debugLogA("Netlib_Send() failed, error=%d", WSAGetLastError());
				return FALSE;
			}
		}
	}
	return TRUE;
}

int TlenWsRecvAES(TlenProtocol *proto, char *data, long datalen, aes_context *aes_ctx, unsigned char *aes_iv)
{
	int ret, len = 0, maxlen = datalen;
	unsigned char aes_input[16];
	unsigned char *aes_output = (unsigned char *)data;
	if (proto->threadData == NULL) {
		return 0;
	}
	for (maxlen = maxlen & ~0xF; maxlen != 0; maxlen = maxlen & 0xF) {
		ret = Netlib_Recv(proto->threadData->s, data, maxlen, MSG_NODUMP);
		if (ret == SOCKET_ERROR) {
			proto->debugLogA("Netlib_Recv() failed, error=%d", WSAGetLastError());
			return 0;
		}
		if (ret == 0) {
			proto->debugLogA("Connection closed gracefully");
			return 0;
		}
		data += ret;
		len += ret;
		maxlen -= ret;
	}

	ret = len;
	while (len > 15) {
		memcpy(aes_input, aes_output, 16);
		aes_crypt_cbc(aes_ctx, AES_DECRYPT, 16, aes_iv, aes_input, aes_output);
		aes_output += 16;
		len -= 16;
	}
	return ret;
}

