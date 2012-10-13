/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
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
	InitializeCriticalSection(&csMsgQueue);
}

void CMsnProto::MsgQueue_Uninit(void)
{
	MsgQueue_Clear();
	DeleteCriticalSection(&csMsgQueue);
}

int  CMsnProto::MsgQueue_Add(const char* wlid, int msgType, const char* msg, int msgSize, filetransfer* ft, int flags, STRLIST *cnt)
{
	EnterCriticalSection(&csMsgQueue);

	MsgQueueEntry* E = new MsgQueueEntry;
	msgQueueList.insert(E);

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

	LeaveCriticalSection(&csMsgQueue);
	return seq;
}

// shall we create another session?
const char* CMsnProto::MsgQueue_CheckContact(const char* wlid, time_t tsc)
{
	EnterCriticalSection(&csMsgQueue);

	time_t ts = time(NULL);
	const char* ret = NULL;
	for (int i=0; i < msgQueueList.getCount(); i++)
	{
		if (_stricmp(msgQueueList[i].wlid, wlid) == 0 && (tsc == 0 || (ts - msgQueueList[i].ts) < tsc))
		{	
			ret = wlid;
			break;
		}	
	}

	LeaveCriticalSection(&csMsgQueue);
	return ret;
}

//for threads to determine who they should connect to
const char* CMsnProto::MsgQueue_GetNextRecipient(void)
{
	EnterCriticalSection(&csMsgQueue);

	const char* ret = NULL;
	for (int i=0; i < msgQueueList.getCount(); i++)
	{
		MsgQueueEntry& E = msgQueueList[i];
		if (!E.allocatedToThread)
		{
			E.allocatedToThread = 1;
			ret = E.wlid;

			while(++i < msgQueueList.getCount())
				if (_stricmp(msgQueueList[i].wlid, ret) == 0)
					msgQueueList[i].allocatedToThread = 1;

			break;
		}	
	}

	LeaveCriticalSection(&csMsgQueue);
	return ret;
}

//deletes from list. Must mir_free() return value
bool  CMsnProto::MsgQueue_GetNext(const char* wlid, MsgQueueEntry& retVal)
{
	int i;

	EnterCriticalSection(&csMsgQueue);
	for(i=0; i < msgQueueList.getCount(); i++)
		if (_stricmp(msgQueueList[i].wlid, wlid) == 0)
			break;
	
	bool res = i != msgQueueList.getCount();
	if (res)
	{	
		retVal = msgQueueList[i];
		msgQueueList.remove(i);
	}
	LeaveCriticalSection(&csMsgQueue);
	return res;
}

int  CMsnProto::MsgQueue_NumMsg(const char* wlid)
{
	int res = 0;
	EnterCriticalSection(&csMsgQueue);

	for(int i=0; i < msgQueueList.getCount(); i++)
		res += (_stricmp(msgQueueList[i].wlid, wlid) == 0);
	
	LeaveCriticalSection(&csMsgQueue);
	return res;
}

void  CMsnProto::MsgQueue_Clear(const char* wlid, bool msg)
{
	int i;

	EnterCriticalSection(&csMsgQueue);
	if (wlid == NULL)
	{

		for(i=0; i < msgQueueList.getCount(); i++)
		{
			const MsgQueueEntry& E = msgQueueList[i];
			if (E.msgSize == 0)
			{
				HANDLE hContact = MSN_HContactFromEmail(E.wlid);
				SendBroadcast(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, 
					(HANDLE)E.seq, (LPARAM)MSN_Translate("Message delivery failed"));
			}
			mir_free(E.message);
			mir_free(E.wlid);
			if (E.cont) delete E.cont;
		}
		msgQueueList.destroy();

		msgQueueSeq = 1;
	}
	else
	{
		for(i=0; i < msgQueueList.getCount(); i++)
		{
			time_t ts = time(NULL);
			const MsgQueueEntry& E = msgQueueList[i];
			if (_stricmp(msgQueueList[i].wlid, wlid) == 0 && (!msg || E.msgSize == 0))
			{
				bool msgfnd = E.msgSize == 0 && E.ts < ts;
				int seq = E.seq;
				
				mir_free(E.message);
				mir_free(E.wlid);
				if (E.cont) delete E.cont;
				msgQueueList.remove(i);

				if (msgfnd) 
				{
					LeaveCriticalSection(&csMsgQueue);
					HANDLE hContact = MSN_HContactFromEmail(wlid);
					SendBroadcast(hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)seq, 
						(LPARAM)MSN_Translate("Message delivery failed"));
					i = 0;
					EnterCriticalSection(&csMsgQueue);
				}
			}
		}
	}
	LeaveCriticalSection(&csMsgQueue);
}

void __cdecl CMsnProto::MsgQueue_AllClearThread(void* arg)
{
	MsgQueue_Clear((char*)arg);
	mir_free(arg);
}
