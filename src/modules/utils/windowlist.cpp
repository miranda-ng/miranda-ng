/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
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
#include "commonheaders.h"

static WINDOWLISTENTRY *windowList=NULL;
static int windowListCount=0;
static int nextWindowListId=1;

static INT_PTR AllocWindowList(WPARAM, LPARAM)
{
	return nextWindowListId++;
}

static INT_PTR AddToWindowList(WPARAM, LPARAM lParam)
{
	windowList=(WINDOWLISTENTRY*)mir_realloc(windowList,sizeof(WINDOWLISTENTRY)*(windowListCount+1));
	windowList[windowListCount++]=*(WINDOWLISTENTRY*)lParam;
	return 0;
}

static INT_PTR RemoveFromWindowList(WPARAM wParam,LPARAM lParam)
{
	int i;
	for(i=0;i<windowListCount;i++)
		if(windowList[i].hwnd==(HWND)lParam && windowList[i].hList==(HANDLE)wParam) {
			MoveMemory(&windowList[i],&windowList[i+1],sizeof(WINDOWLISTENTRY)*(windowListCount-i-1));
			windowListCount--;
			return 0;
		}
	return 1;
}

static INT_PTR FindInWindowList(WPARAM wParam,LPARAM lParam)
{
	int i;
	for(i=0;i<windowListCount;i++)
		if(windowList[i].hContact==(HANDLE)lParam && windowList[i].hList==(HANDLE)wParam)
			return (INT_PTR)windowList[i].hwnd;
	return (INT_PTR)(HWND)NULL;
}

static INT_PTR BroadcastToWindowList(WPARAM wParam,LPARAM lParam)
{
	int i;
	MSG *msg=(MSG*)lParam;
	for(i=0;i<windowListCount;i++)
		if(windowList[i].hList==(HANDLE)wParam)
			SendMessage(windowList[i].hwnd,msg->message,msg->wParam,msg->lParam);
	return 0;
}

static INT_PTR BroadcastToWindowListAsync(WPARAM wParam,LPARAM lParam)
{
	int i;
	MSG *msg=(MSG*)lParam;
	for(i=0;i<windowListCount;i++)
		if(windowList[i].hList==(HANDLE)wParam)
			PostMessage(windowList[i].hwnd,msg->message,msg->wParam,msg->lParam);
	return 0;
}

int InitWindowList(void)
{
	CreateServiceFunction(MS_UTILS_ALLOCWINDOWLIST,AllocWindowList);
	CreateServiceFunction(MS_UTILS_ADDTOWINDOWLIST,AddToWindowList);
	CreateServiceFunction(MS_UTILS_REMOVEFROMWINDOWLIST,RemoveFromWindowList);
	CreateServiceFunction(MS_UTILS_BROADCASTTOWINDOWLIST,BroadcastToWindowList);
	CreateServiceFunction(MS_UTILS_BROADCASTTOWINDOWLIST_ASYNC,BroadcastToWindowListAsync);
	CreateServiceFunction(MS_UTILS_FINDWINDOWINLIST,FindInWindowList);
	return 0;
}

void FreeWindowList(void)
{
	if ( windowList ) {
		mir_free(windowList);
		windowList = NULL;
	}
	windowListCount=0;
	nextWindowListId=1;
}
