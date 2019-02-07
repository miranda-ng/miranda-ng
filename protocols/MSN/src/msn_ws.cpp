/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2019 Miranda NG team
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "msn_proto.h"

//=======================================================================================

int ThreadData::send(const char data[], size_t datalen)
{
	resetTimeout();

	int rlen = Netlib_Send(s, data, (int)datalen);
	if (rlen == SOCKET_ERROR) {
		// should really also check if sendlen is the same as datalen
		proto->debugLogA("Send failed: %d", WSAGetLastError());
		return FALSE;
	}

	return TRUE;
}

void ThreadData::resetTimeout(bool term)
{
	int timeout = term ? 10 : mIsMainThread ? 65 : 120;
	mWaitPeriod = clock() + timeout * CLOCKS_PER_SEC;
}

bool ThreadData::isTimeout(void)
{
	if (mWaitPeriod >= clock())
		return false;

	bool res = false;
	if (mIsMainThread)
		res = true;
	else if (mJoinedContactsWLID.getCount() <= 1 || mChatID[0] == 0)
		res = true;

	if (res) {
		proto->debugLogA("Dropping the idle p2p due to inactivity");
		if (termPending)
			return true;
	}
	else resetTimeout();

	return false;
}

int ThreadData::recv(char* data, size_t datalen)
{
	resetTimeout();

	NETLIBSELECT nls = {};
	nls.dwTimeout = 1000;
	nls.hReadConns[0] = s;

	for (;;) {
		int ret = Netlib_Select(&nls);
		if (ret < 0) {
			proto->debugLogA("Connection abortively closed, error %d", WSAGetLastError());
			return ret;
		}
		else if (ret == 0) {
			if (isTimeout()) return 0;
		}
		else
			break;
	}

	int ret = Netlib_Recv(s, data, (int)datalen);
	if (ret == 0) {
		proto->debugLogA("Connection closed gracefully");
		return 0;
	}

	if (ret < 0) {
		proto->debugLogA("Connection abortively closed, error %d", WSAGetLastError());
		return ret;
	}

	return ret;
}
