/* 
Copyright (C) 2012 Mataes

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

#include "common.h"

// check if Feed is currently updating
BOOL ThreadRunning;
UPDATELIST *UpdateListHead = NULL;
UPDATELIST *UpdateListTail = NULL;

// main auto-update timer
VOID CALLBACK timerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	// only run if it is not current updating and the auto update option is enabled
	if (!ThreadRunning && !Miranda_Terminated())
	{
		BOOL HaveUpdates = FALSE;
		HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
		while (hContact != NULL) 
		{
			if(IsMyContact(hContact)) 
			{
				if (DBGetContactSettingDword(hContact, MODULE, "UpdateTime", 60))
				{
					double diff = difftime(time(NULL), DBGetContactSettingDword(hContact, MODULE, "LastCheck", 0));
					if (diff >= DBGetContactSettingDword(hContact, MODULE, "UpdateTime", 60) * 60)
					{
						UpdateListAdd(hContact);
						HaveUpdates = TRUE;
					}
				}
			}
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
		}
		if (!ThreadRunning && HaveUpdates)
			mir_forkthread(UpdateThreadProc, NULL);
	}
}

// temporary timer for first run
// when this is run, it kill the old startup timer and create the permenant one above
VOID CALLBACK timerProc2(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	KillTimer(NULL, timerId);
	ThreadRunning = FALSE;

	if (!Miranda_Terminated())
	{
		CheckAllFeeds(0,0);
		timerId = SetTimer(NULL, 0, 30000, (TIMERPROC)timerProc);
	}
}

void UpdateListAdd(HANDLE hContact)
{
	UPDATELIST *newItem;

	newItem = (UPDATELIST*)mir_alloc(sizeof(UPDATELIST));
	newItem->hContact = hContact;
	newItem->next = NULL;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	if (UpdateListTail == NULL) UpdateListHead = newItem;
	else UpdateListTail->next = newItem;
	UpdateListTail = newItem;

	ReleaseMutex(hUpdateMutex);
}

HANDLE UpdateGetFirst() 
{
	HANDLE hContact = NULL;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	if (UpdateListHead != NULL) 
	{
		UPDATELIST* Item = UpdateListHead; 

		hContact = Item->hContact;
		UpdateListHead = Item->next;
		mir_free(Item);

		if (UpdateListHead == NULL) UpdateListTail = NULL; 
	}

	ReleaseMutex(hUpdateMutex);

	return hContact;
}

void DestroyUpdateList(void) 
{
	UPDATELIST *temp;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	temp = UpdateListHead;

	// free the list one by one
	while (temp != NULL) 
	{
		UpdateListHead = temp->next;
		mir_free(temp);
		temp = UpdateListHead;
	}
	// make sure the entire list is clear
	UpdateListTail = NULL;

	ReleaseMutex(hUpdateMutex);
}

void UpdateThreadProc(LPVOID hWnd)
{
	WaitForSingleObject(hUpdateMutex, INFINITE);
	if (ThreadRunning)
	{
		ReleaseMutex(hUpdateMutex);
		return;
	}
	ThreadRunning = TRUE;	// prevent 2 instance of this thread running
	ReleaseMutex(hUpdateMutex);

	// update weather by getting the first station from the queue until the queue is empty
	while (UpdateListHead != NULL && !Miranda_Terminated())	
		CheckCurrentFeed(UpdateGetFirst());

	// exit the update thread
	ThreadRunning = FALSE;
}