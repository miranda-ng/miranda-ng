////////////////////////////////////////////////////////////////////////////////
// Gadu-Gadu Plugin for Miranda IM
//
// Copyright (c) 2003-2006 Adam Strzelecki <ono+miranda@java.pl>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
////////////////////////////////////////////////////////////////////////////////

#include "gg.h"

/* NOTE: Eventhough SetTimer seems to support UINT_PTR for idEvent, it seems that TimerProc
 * does not get full pointer but just 2 byte lower bytes.
 */
#define MAX_TIMERS 8
GGPROTO *g_timers[MAX_TIMERS] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

static VOID CALLBACK gg_keepalive(HWND hwnd, UINT message, UINT_PTR idEvent, DWORD dwTime)
{
	int i;
	
	//Search for GGPROTO* context
	for(i = 0; i < MAX_TIMERS; i++)
		if (g_timers[i]->timer == idEvent)
			break;

	if (i < MAX_TIMERS)
	{
		GGPROTO *gg = g_timers[i];
		if (gg->isonline())
		{
	#ifdef DEBUGMODE
			gg->netlog("Sending keep-alive");
	#endif
			EnterCriticalSection(&gg->sess_mutex);
			gg_ping(gg->sess);
			LeaveCriticalSection(&gg->sess_mutex);
		}
	}
}

void GGPROTO::keepalive_init()
{
	if (db_get_b(NULL, m_szModuleName, GG_KEY_KEEPALIVE, GG_KEYDEF_KEEPALIVE))
	{
		int i;
		for(i = 0; i < MAX_TIMERS && g_timers[i] != NULL; i++);
		if (i < MAX_TIMERS)
		{
	#ifdef DEBUGMODE
			netlog("gg_keepalive_init(): Initializing Timer %d", i);
	#endif
			timer = SetTimer(NULL, 0, 1000 * 30, gg_keepalive);
			g_timers[i] = this;
		}
	}
}

void GGPROTO::keepalive_destroy()
{
#ifdef DEBUGMODE
	netlog("gg_destroykeepalive(): Killing Timer");
#endif
	if (timer)
	{
		int i;
		KillTimer(NULL, timer);
		for(i = 0; i < MAX_TIMERS; i++)
			if (g_timers[i] == this) {
				g_timers[i] = NULL;
				break;
			}
		timer = 0;
#ifdef DEBUGMODE
		netlog("gg_destroykeepalive(): Killed Timer %d", i);
#endif
	}
#ifdef DEBUGMODE
	netlog("gg_destroykeepalive(): End");
#endif
}
