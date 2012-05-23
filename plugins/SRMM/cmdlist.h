/*
Copyright 2000-2010 Miranda IM project, 
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
#ifndef SRMM_CMDLIST_H
#define SRMM_CMDLIST_H

typedef struct _TCmdList 
{
	TCHAR *szCmd;
} 
TCmdList;

int tcmdlist_append(SortedList *list, TCHAR *data);
void tcmdlist_free(SortedList *list);

__inline TCHAR* tcmdlist_getitem(SortedList *list, int ind) 
{ return ((TCmdList*)list->items[ind])->szCmd; }


typedef struct _TMsgQueue 
{
	HANDLE id;
	HANDLE hContact;
	TCHAR* szMsg;
	HANDLE hDbEvent;
	unsigned ts;
} 
TMsgQueue;

void msgQueue_add(HANDLE hContact, HANDLE id, const TCHAR* szMsg, HANDLE hDbEvent);
void msgQueue_processack(HANDLE hContact, HANDLE id, BOOL success, const char* szErr);
void msgQueue_init(void);
void msgQueue_destroy(void);

#endif
