/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (C) 2012-22 Miranda NG team

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

bool CJabberProto::WsInit(void)
{
	m_lastTicks = ::GetTickCount();

	wchar_t name[128];
	mir_snwprintf(name, TranslateT("%s connection"), m_tszUserName);

	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_INCOMING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szDescriptiveName.w = name;
	nlu.szSettingsModule = m_szModuleName;
	m_hNetlibUser = Netlib_RegisterUser(&nlu);
	return m_hNetlibUser != nullptr;
}

HNETLIBCONN CJabberProto::WsConnect(char* host, uint16_t port)
{
	NETLIBOPENCONNECTION nloc = {};
	nloc.szHost = host;
	nloc.wPort = port;
	nloc.timeout = 6;
	return Netlib_OpenConnection(m_hNetlibUser, &nloc);
}

int CJabberProto::WsSend(HNETLIBCONN hConn, char* data, int datalen, int flags)
{
	m_lastTicks = ::GetTickCount();
	int len;

	if ((len = Netlib_Send(hConn, data, datalen, flags)) == SOCKET_ERROR || len != datalen) {
		debugLogA("Netlib_Send() failed, error=%d", WSAGetLastError());
		return SOCKET_ERROR;
	}
	return len;
}

int CJabberProto::WsRecv(HNETLIBCONN hConn, char* data, long datalen, int flags)
{
	int ret;

	ret = Netlib_Recv(hConn, data, datalen, flags);
	if (ret == SOCKET_ERROR) {
		debugLogA("Netlib_Recv() failed, error=%d", WSAGetLastError());
		return 0;
	}
	if (ret == 0) {
		debugLogA("Connection closed gracefully");
		return 0;
	}
	return ret;
}
