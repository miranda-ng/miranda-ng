/*
Copyright (C) 2005 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#include "commonheaders.h"


BOOL g_shutDown = FALSE;
static HANDLE hShutdownEvent = NULL;
static HANDLE hOkToExit = NULL;


static int OkToExitProc(WPARAM wParam, LPARAM lParam)
{
	g_shutDown = TRUE;
	SetEvent(hShutdownEvent);
	CloseHandle(hShutdownEvent);
	UnhookEvent(hOkToExit);
	return 0;
}


void init_mir_thread() 
{
	hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hOkToExit = HookEvent(ME_SYSTEM_OKTOEXIT, OkToExitProc);
}


void mir_sleep(int time) 
{
	if (!g_shutDown)
		WaitForSingleObject(hShutdownEvent, time);
}
