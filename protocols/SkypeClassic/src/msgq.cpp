/*
 * Implements a simple message queue for send and receive queue.
 * We could use memlist.c, but it's not efficient 
 * enough for this purpose (would always memmove on removing first
 * element), therefore it's implemented as tail queue.
 */

#include "skype.h"
#include "msgq.h"
#include "debug.h"


void MsgQ_Init(TYP_MSGQ *q)
{
	TAILQ_INIT(&q->l);
	InitializeCriticalSection (&q->cs);
}

void MsgQ_Exit(TYP_MSGQ *q)
{
	struct MsgQueue *ptr;

	EnterCriticalSection(&q->cs);
	while ((ptr=q->l.tqh_first) != NULL)
		free(MsgQ_RemoveMsg(q, ptr));
	LeaveCriticalSection(&q->cs);
	DeleteCriticalSection (&q->cs);
}

BOOL MsgQ_Add(TYP_MSGQ *q, char *msg)
{
	struct MsgQueue *ptr;

	if ((ptr=(struct MsgQueue*)malloc(sizeof(struct MsgQueue))) == NULL)
		return FALSE;
	ptr->message = _strdup(msg); // Don't forget to free!
	ptr->tAdded = SkypeTime(NULL);
	SkypeTime(&ptr->tReceived);
	EnterCriticalSection(&q->cs);
	TAILQ_INSERT_TAIL(&q->l, ptr, l);
	//LOG (("MsgQ_Add (%s) @%lu/%ld", msg, ptr->tReceived, ptr->tAdded));
	LeaveCriticalSection(&q->cs);
	return TRUE;
}

char *MsgQ_RemoveMsg(TYP_MSGQ *q, struct MsgQueue *ptr)
{
	char *msg;

	if (!ptr) return NULL;
	EnterCriticalSection(&q->cs);
	TAILQ_REMOVE(&q->l, ptr, l);
	LeaveCriticalSection(&q->cs);
	msg=ptr->message;
	free(ptr);
	return msg;
}

char *MsgQ_Get(TYP_MSGQ *q)
{
	char *msg;

	msg=MsgQ_RemoveMsg(q, q->l.tqh_first);
	return msg;
}

int MsgQ_CollectGarbage(TYP_MSGQ *q, time_t age)
{
	struct MsgQueue *ptr;
	int i=0;

	EnterCriticalSection(&q->cs);
	ptr=q->l.tqh_first;
	while (ptr)
	{
		if (ptr->tAdded && SkypeTime(NULL)-ptr->tAdded>age)
		{
			struct MsgQueue *ptr_;
			LOG(("GarbageCollector throwing out message: %s", ptr->message));
			ptr_=ptr;
			ptr=ptr->l.tqe_next;
			free(MsgQ_RemoveMsg(q, ptr_));
			i++;
			continue;
		}
		ptr=ptr->l.tqe_next;
	}
	LeaveCriticalSection(&q->cs);
	return i;
}
