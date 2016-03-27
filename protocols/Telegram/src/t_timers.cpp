
#include "stdafx.h"

extern HANDLE hQueue;

struct tgl_timer
{
	tgl_state *TLS;
	void(*cb)(struct tgl_state *TLS, void *arg);
	void *arg;
	HANDLE hTimer;
};

VOID CALLBACK WaitOrTimerCallback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired)
{
	tgl_timer *p = (tgl_timer*)lpParameter;
	p->cb(p->TLS, p->arg);
	p->hTimer = 0;
}


struct tgl_timer *mtgl_timer_alloc (struct tgl_state *TLS, void (*cb)(struct tgl_state *TLS, void *arg), void *arg) 
{
    tgl_timer *p = (tgl_timer *)mir_calloc(sizeof (tgl_timer));
    p->TLS = TLS;
    p->cb = cb;
    p->arg = arg;
    return p;
}

void mtgl_timer_insert (struct tgl_timer *t, double p) 
{
	HANDLE hNewTimer = 0;
	CreateTimerQueueTimer(&hNewTimer, hQueue, WaitOrTimerCallback, t, (DWORD)(p * 1000), 0, 0);
	t->hTimer = hNewTimer;
}

void mtgl_timer_delete (struct tgl_timer *t)
{
	DeleteTimerQueueTimer(hQueue, t->hTimer, 0);
	t->hTimer = 0;
}

void mtgl_timer_free (struct tgl_timer *t)
{
	if (t->hTimer) mtgl_timer_delete(t);
    mir_free(t);
}


struct tgl_timer_methods mtgl_libevent_timers = {
  mtgl_timer_alloc, 
  mtgl_timer_insert,
  mtgl_timer_delete,
  mtgl_timer_free
};