/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2014 Miranda NG Team
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

#include "msn_global.h"
#include "msn_proto.h"

//a few little functions to manage queuing send message requests until the
//connection is established

void CMsnProto::MsgQueue_Init(void)
{
	msgQueueSeq = 1;
}

void CMsnProto::MsgQueue_Uninit(void)
{
	MsgQueue_Clear();
}

int CMsnProto::MsgQueue_Add(const char* wlid, int msgType, const char* msg, int msgSize, filetransfer* ft, int flags, STRLIST *cnt)
{
	mir_cslock lck(csMsgQueue);

	MsgQueueEntry* E = new MsgQueueEntry;
	lsMessageQueue.insert(E);

	int seq = msgQueueSeq++;

	E->wlid = mir_strdup(wlid);
	E->msgSize = msgSize;
	E->msgType = msgType;
	if (msgSize <= 0)
		E->message = mir_strdup(msg);
	else
		memcpy(E->message = (char*)mir_alloc(msgSize), msg, msgSize);
	E->ft = ft;
	E->cont = cnt;
	E->seq = seq;
	E->flags = flags;
	E->allocatedToThread = 0;
	E->ts = time(NULL);
	return seq;
}

// shall we create another session?
const char* CMsnProto::MsgQueue_CheckContact(const char* wlid, time_t tsc)
{
	time_t ts = time(NULL);

	mir_cslock lck(csMsgQueue);

	for (int i = 0; i < lsMessageQueue.getCount(); i++)
		if (_stricmp(lsMessageQueue[i].wlid, wlid) == 0 && (tsc == 0 || (ts - lsMessageQueue[i].ts) < tsc))
			return wlid;

	return NULL;
}

//for threads to determine who they should connect to
const char* CMsnProto::MsgQueue_GetNextRecipient(void)
{
	mir_cslock lck(csMsgQueue);

	for (int i = 0; i < lsMessageQueue.getCount(); i++) {
		MsgQueueEntry& E = lsMessageQueue[i];
		if (!E.allocatedToThread) {
			E.allocatedToThread = 1;

			const char *ret = E.wlid;
			while (++i < lsMessageQueue.getCount())
				if (_stricmp(lsMessageQueue[i].wlid, ret) == 0)
					lsMessageQueue[i].allocatedToThread = 1;

			return ret;
		}
	}

	return NULL;
}

//deletes from list. Must mir_free() return value
bool CMsnProto::MsgQueue_GetNext(const char* wlid, MsgQueueEntry& retVal)
{
	int i;

	mir_cslock lck(csMsgQueue);
	for (i = 0; i < lsMessageQueue.getCount(); i++)
		if (_stricmp(lsMessageQueue[i].wlid, wlid) == 0)
			break;

	bool res = i != lsMessageQueue.getCount();
	if (res) {
		retVal = lsMessageQueue[i];
		lsMessageQueue.remove(i);
	}

	return res;
}

int CMsnProto::MsgQueue_NumMsg(const char* wlid)
{
	int res = 0;
	mir_cslock lck(csMsgQueue);

	for (int i = 0; i < lsMessageQueue.getCount(); i++)
		res += (_stricmp(lsMessageQueue[i].wlid, wlid) == 0);

	return res;
}

void CMsnProto::MsgQueue_Clear(const char* wlid, bool msg)
{
	int i;

	mir_cslockfull lck(csMsgQueue);
	if (wlid == NULL) {
		for (i = 0; i < lsMessageQueue.getCount(); i++) {
			const MsgQueueEntry& E = lsMessageQueue[i];
			if (E.msgSize == 0) {
				MCONTACT hContact = MSN_HContactFromEmail(E.wlid);
				ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED,
					(HANDLE)E.seq, (LPARAM)Translate("Message delivery failed"));
			}
			mir_free(E.message);
			mir_free(E.wlid);
			if (E.cont) delete E.cont;
		}
		lsMessageQueue.destroy();

		msgQueueSeq = 1;
	}
	else {
		for (i = 0; i < lsMessageQueue.getCount(); i++) {
			time_t ts = time(NULL);
			const MsgQueueEntry& E = lsMessageQueue[i];
			if (_stricmp(lsMessageQueue[i].wlid, wlid) == 0 && (!msg || E.msgSize == 0)) {
				bool msgfnd = E.msgSize == 0 && E.ts < ts;
				int seq = E.seq;

				mir_free(E.message);
				mir_free(E.wlid);
				if (E.cont) delete E.cont;
				lsMessageQueue.remove(i);

				if (msgfnd) {
					lck.unlock();
					MCONTACT hContact = MSN_HContactFromEmail(wlid);
					ProtoBroadcastAck(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)seq,
						(LPARAM)Translate("Message delivery failed"));
					i = 0;
					lck.lock();
				}
			}
		}
	}
}

void __cdecl CMsnProto::MsgQueue_AllClearThread(void* arg)
{
	MsgQueue_Clear((char*)arg);
	mir_free(arg);
}
