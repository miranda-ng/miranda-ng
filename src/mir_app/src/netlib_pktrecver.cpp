/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include "netlib.h"

MIR_APP_DLL(HANDLE) Netlib_CreatePacketReceiver(HNETLIBCONN nlc, int iMaxSize)
{
	if (GetNetlibHandleType(nlc) != NLH_CONNECTION || iMaxSize == 0) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return nullptr;
	}
	
	NetlibPacketRecver *nlpr = (struct NetlibPacketRecver*)mir_calloc(sizeof(struct NetlibPacketRecver));
	nlpr->handleType = NLH_PACKETRECVER;
	nlpr->nlc = nlc;
	nlpr->packetRecver.bufferSize = iMaxSize;
	nlpr->packetRecver.buffer = (PBYTE)mir_alloc(nlpr->packetRecver.bufferSize);
	nlpr->packetRecver.bytesUsed = 0;
	nlpr->packetRecver.bytesAvailable = 0;
	return nlpr;
}

MIR_APP_DLL(int) Netlib_GetMorePackets(HANDLE hReceiver, NETLIBPACKETRECVER *nlprParam)
{
	NetlibPacketRecver *nlpr = (NetlibPacketRecver*)hReceiver;
	if (GetNetlibHandleType(nlpr) != NLH_PACKETRECVER || nlprParam == nullptr || nlprParam->bytesUsed > nlpr->packetRecver.bytesAvailable) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return SOCKET_ERROR;
	}
	if (Miranda_IsTerminated()) { /* HACK: Lame, break while loops of protocols that can't kill their while loops, (cough, ICQ, cough) */
		SetLastError(ERROR_TIMEOUT);
		return SOCKET_ERROR;
	}
	nlpr->packetRecver.dwTimeout = nlprParam->dwTimeout;
	if (nlprParam->bytesUsed == 0) {
		if (nlpr->packetRecver.bytesAvailable == nlpr->packetRecver.bufferSize) {
			nlpr->packetRecver.bytesAvailable = 0;
			Netlib_Logf(nlpr->nlc->nlu, "Packet recver: packet overflowed buffer, ditching");
		}
	}
	else {
		memmove(nlpr->packetRecver.buffer, nlpr->packetRecver.buffer + nlprParam->bytesUsed, nlpr->packetRecver.bytesAvailable - nlprParam->bytesUsed);
		nlpr->packetRecver.bytesAvailable -= nlprParam->bytesUsed;
	}
	
	if (nlprParam->dwTimeout != INFINITE) {
		if (!Netlib_SslPending(nlpr->nlc->hSsl) && WaitUntilReadable(nlpr->nlc->s, nlprParam->dwTimeout) <= 0) {
			*nlprParam = nlpr->packetRecver;
			return SOCKET_ERROR;
		}
	}
	
	INT_PTR recvResult = Netlib_Recv(nlpr->nlc, (char*)nlpr->packetRecver.buffer + nlpr->packetRecver.bytesAvailable, nlpr->packetRecver.bufferSize - nlpr->packetRecver.bytesAvailable, 0);
	if (recvResult > 0)
		nlpr->packetRecver.bytesAvailable += recvResult;
	*nlprParam = nlpr->packetRecver;
	return recvResult;
}
