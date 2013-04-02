/*
 * Miranda-IM Vypress Chat/quickChat plugins
 * Copyright (C) Saulius Menkevicius
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: msgloop.c,v 1.13 2005/03/08 17:42:25 bobas Exp $
 */

#include "main.h"
#include "msgloop.h"
#include "pthread.h"

/* constants and struct defs
 */

/* max packet output rate is 2 packets per sec */
#define PACKET_OUTPUT_RATE	500

struct msgloop_message_list_entry {
	struct msgloop_message_list_entry * prev, * next;
	vqp_msg_t msg;
};

/* static data
 */
static PAPCFUNC	s_lpfnMsgHandleApc;
static HANDLE	s_hMainThread, s_hLoopThread;
static BOOL	s_fStopLoop = TRUE,
		s_fLoopStarted;
static HANDLE	s_hStopWaitEvent;

static UINT_PTR s_outputTimer;
static struct msgloop_message_list_entry
		* s_outputListHead, * s_outputListTail;
static BOOL	s_outputSentThisTick = FALSE, s_outputSentThisTickInTimer;

/* static routines
 */

static void CALLBACK msgloop_apc_stop(ULONG_PTR unused)
{
	s_fStopLoop = TRUE;
}

static void CALLBACK msgloop_apc_send_msg(ULONG_PTR vqmsg)
{
	vqp_link_send((vqp_msg_t)vqmsg);
	vqp_msg_free((vqp_msg_t)vqmsg);
}

static void msgloop_loop(void * vqlink)
{
	WSAEVENT hReadEvent;
	SOCKET rx_socket = vqp_link_rx_socket((vqp_link_t) vqlink);

	s_fLoopStarted = TRUE;

	hReadEvent = WSACreateEvent();
	WSAEventSelect(rx_socket, hReadEvent, FD_READ);

	while(!Miranda_Terminated() && !s_fStopLoop) {
		DWORD nEvent = WSAWaitForMultipleEvents(
				1, &hReadEvent, FALSE, WSA_INFINITE, TRUE);

		if(nEvent==WSA_WAIT_EVENT_0) {
			vqp_msg_t msg;

			WSAResetEvent(hReadEvent);
	
			if(!vqp_link_recv((vqp_link_t) vqlink, &msg)) {
				QueueUserAPC(
					s_lpfnMsgHandleApc,
					s_hMainThread, (ULONG_PTR)msg);
			}
		}
	}

	WSACloseEvent(hReadEvent);

	/* wait for pending user APC */
	while(SleepEx(10, TRUE) == WAIT_IO_COMPLETION) /* nothing */;

	/* signal that we've finished */
	SetEvent(s_hStopWaitEvent);
}

static void CALLBACK msgloop_message_output_timer_cb(
	HWND hwnd, UINT nmsg, UINT_PTR idevent, DWORD dwtime)
{
	if(s_outputListTail && (!s_outputSentThisTick
					|| (s_outputSentThisTick && s_outputSentThisTickInTimer)))
	{
		struct msgloop_message_list_entry * entry = s_outputListTail;
		QueueUserAPC(msgloop_apc_send_msg, s_hLoopThread, (ULONG_PTR)entry->msg);

		s_outputListTail = entry->next;
		if(s_outputListTail) {
			s_outputListTail->prev = NULL;
		} else {
			s_outputListHead = NULL;
		}

		free(entry);

		s_outputSentThisTick = TRUE;
		s_outputSentThisTickInTimer = TRUE;
	} else {
		s_outputSentThisTick = FALSE;
	}
}

/* exported routines
 */

/* msgloop_start:
 *	starts msg loop
 */
void msgloop_start(vqp_link_t vqlink, PAPCFUNC lpfMsgHandlerApc)
{
	ASSERT_RETURNIFFAIL(s_fStopLoop);
			
	s_lpfnMsgHandleApc = lpfMsgHandlerApc;
	DuplicateHandle(
		GetCurrentProcess(), GetCurrentThread(),
		GetCurrentProcess(), &s_hMainThread,
		THREAD_SET_CONTEXT, FALSE, 0);
	
	s_fStopLoop = FALSE;
	s_fLoopStarted = FALSE;	/* this will be set in msgloop_loop() */
	s_hLoopThread = (HANDLE)pthread_create(msgloop_loop, (void*)vqlink);

	/* start scheduler timer:
	 *	this will make packets to be send no faster than specific rate
	 */
	s_outputTimer = SetTimer(NULL, 0, PACKET_OUTPUT_RATE, msgloop_message_output_timer_cb);
	s_outputSentThisTick = FALSE;
}

/* msgloop_stop:
 *	msgloop_stop should be called from miranda's gui thread
 *	to stop the qcs_msg loop.
 */
void msgloop_stop()
{
	struct msgloop_message_list_entry * entry;

	ASSERT_RETURNIFFAIL(!s_fStopLoop);

	/* stop packet scheduler timer
	 */
	KillTimer(NULL, s_outputTimer);
	s_outputTimer = 0;

	/* signal the thread to stop */
	s_hStopWaitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	QueueUserAPC(msgloop_apc_stop, s_hLoopThread, (ULONG_PTR)NULL);
	
	/* wait for the thread to stop
	 * and handle all the queued APCs
	 */
	while(WaitForSingleObjectEx(s_hStopWaitEvent, 10, TRUE) == WAIT_IO_COMPLETION)
		/* do nothing */ ;

	CloseHandle(s_hStopWaitEvent);
	CloseHandle(s_hMainThread);

	/* do cleanup */
	s_fLoopStarted = FALSE;
	
	/* send all the queued packets that didn't get sent
	 * by packet scheduler in msgloop_packet_output_timer_c()
	 */
	entry = s_outputListTail;
	while(entry) {
		struct msgloop_message_list_entry * next = entry->next;
		vqp_link_send(entry->msg);
		vqp_msg_free(entry->msg);
		free(entry);

		entry = next;
	}
	s_outputListTail = NULL;
	s_outputListHead = NULL;
}

/* msgloop_send:
 *	msgloop_send should be used to send a message from miranda's
 *	gui thread to send a message with qcs_msg loop.
 */
void msgloop_send(vqp_msg_t msg, int never_wait)
{
	ASSERT_RETURNIFFAIL(VALIDPTR(msg));

	/* send this message */
	if(s_fLoopStarted && s_outputTimer) {
		/* check if we've sent a packet this tick,
		 * and if we did, we'll have to wait for the next tick
		 */
		if(s_outputSentThisTick && !never_wait) {
			/* add msg to message queue for scheduler timer
			 */
			struct msgloop_message_list_entry * entry;
			entry = malloc(sizeof(struct msgloop_message_list_entry));
			entry->msg = msg;
			entry->next = NULL;
			entry->prev = s_outputListHead;
			if(entry->prev) {
				entry->prev->next = entry;
			} else {
				s_outputListTail = entry;
			}
			s_outputListHead = entry;
		} else {
			QueueUserAPC(msgloop_apc_send_msg, s_hLoopThread, (ULONG_PTR)msg);
			s_outputSentThisTick = TRUE;
		}
	} else {
		if(s_outputSentThisTick && !never_wait) {
			Sleep(PACKET_OUTPUT_RATE);
		}
		vqp_link_send(msg);
		vqp_msg_free(msg);
		s_outputSentThisTick = TRUE;
		s_outputSentThisTickInTimer = FALSE;
	}
}

/* msgloop_send_to:
 *	sends message to the specified address only
 */
void msgloop_send_to(vqp_msg_t msg, int never_wait, vqp_addr_t addr) {
	vqp_msg_set_dst_addr(msg, addr);
	msgloop_send(msg, never_wait);
}

