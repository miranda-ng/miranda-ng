/*
Plugin for Miranda NG for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-14 Miranda NG Team

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

#include "msn_global.h"
#include "msn_proto.h"

void CMsnProto::AvatarQueue_Init()
{
	hevAvatarQueue = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	ForkThread(&CMsnProto::MSN_AvatarsThread, 0);
}

void CMsnProto::AvatarQueue_Uninit()
{
	::CloseHandle(hevAvatarQueue);
}

void CMsnProto::pushAvatarRequest(MCONTACT hContact, LPCSTR pszUrl)
{
	ProtoBroadcastAck(hContact, ACKTYPE_AVATAR, ACKRESULT_STATUS, NULL, 0);

	if (pszUrl != NULL && *pszUrl != 0) {
		mir_cslock lck(csAvatarQueue);

		for (int i=0; i < lsAvatarQueue.getCount(); i++)
			if (lsAvatarQueue[i]->hContact == hContact)
				return;

		lsAvatarQueue.insert(new AvatarQueueEntry(hContact, pszUrl));
		SetEvent(hevAvatarQueue);
	}
}

bool CMsnProto::loadHttpAvatar(AvatarQueueEntry *p)
{
	NETLIBHTTPHEADER nlbhHeaders[1];
	nlbhHeaders[0].szName = "User-Agent";
	nlbhHeaders[0].szValue = (char*)MSN_USER_AGENT;

	NETLIBHTTPREQUEST nlhr = { sizeof(nlhr) };
	nlhr.requestType = REQUEST_GET;
	nlhr.flags = NLHRF_HTTP11 | NLHRF_REDIRECT;
	nlhr.szUrl = p->pszUrl;
	nlhr.headers = (NETLIBHTTPHEADER*)&nlbhHeaders;
	nlhr.headersCount = 1;

	NETLIBHTTPREQUEST *nlhrReply = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)m_hNetlibUser, (LPARAM)&nlhr);
	if (nlhrReply == NULL)
		return false;

	if (nlhrReply->resultCode != 200 || nlhrReply->dataLength == 0) {
LBL_Error:
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
		return false;
	}

	const TCHAR *szExt;
	int fmt = ProtoGetBufferFormat(nlhrReply->pData, &szExt);
	if (fmt == PA_FORMAT_UNKNOWN)
		goto LBL_Error;

	PROTO_AVATAR_INFORMATIONT AI = { sizeof(AI) };
	AI.format = fmt;
	AI.hContact = p->hContact;
	MSN_GetAvatarFileName(AI.hContact, AI.filename, SIZEOF(AI.filename), szExt);
	_tremove(AI.filename);

	int fileId = _topen(AI.filename, _O_CREAT | _O_TRUNC | _O_WRONLY | O_BINARY, _S_IREAD | _S_IWRITE);
	if (fileId == -1)
		goto LBL_Error;

	_write(fileId, nlhrReply->pData, (unsigned)nlhrReply->dataLength);
	_close(fileId);

	ProtoBroadcastAck(p->hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &AI, 0);
	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)nlhrReply);
	return true;
}

void __cdecl CMsnProto::MSN_AvatarsThread(void*)
{
	while(true) {
		if (WaitForSingleObject(hevAvatarQueue, INFINITE) != WAIT_OBJECT_0)
			break;

		if ( Miranda_Terminated())
			break;

		AvatarQueueEntry *p = NULL;
		{
			mir_cslock lck(csAvatarQueue);
			if (lsAvatarQueue.getCount() > 0) {
				p = lsAvatarQueue[0];
				lsAvatarQueue.remove(0);
			}
		}

		if (p == NULL)
			continue;

		if ( !loadHttpAvatar(p))
			ProtoBroadcastAck(p->hContact, ACKTYPE_AVATAR, ACKRESULT_FAILED, 0, 0);
		delete p;
	}
}
