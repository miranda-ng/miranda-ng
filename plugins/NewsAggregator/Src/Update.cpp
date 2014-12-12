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
bool ThreadRunning;
UPDATELIST *UpdateListHead = NULL;
UPDATELIST *UpdateListTail = NULL;

// main auto-update timer
void CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD)
{
	// only run if it is not current updating and the auto update option is enabled
	if (!ThreadRunning && !Miranda_Terminated()) {
		bool HaveUpdates = FALSE;
		for (MCONTACT hContact = db_find_first(MODULE); hContact; hContact = db_find_next(hContact, MODULE)) {
			if (db_get_dw(hContact, MODULE, "UpdateTime", DEFAULT_UPDATE_TIME)) {
				double diff = difftime(time(NULL), (time_t)db_get_dw(hContact, MODULE, "LastCheck", 0));
				if (db_get_b(NULL, MODULE, "AutoUpdate", 1) != 0 && diff >= db_get_dw(hContact, MODULE, "UpdateTime", DEFAULT_UPDATE_TIME) * 60) {
					UpdateListAdd(hContact);
					HaveUpdates = TRUE;
				}
			}
		}
		if (!ThreadRunning && HaveUpdates)
			mir_forkthread(UpdateThreadProc, 0);
	}
}

// temporary timer for first run
// when this is run, it kill the old startup timer and create the permenant one above
void CALLBACK timerProc2(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(NULL, timerId);
	ThreadRunning = FALSE;

	if (db_get_b(NULL, MODULE, "AutoUpdate", 1) && !Miranda_Terminated()) {
		if (db_get_b(NULL, MODULE, "StartupRetrieve", 1))
			CheckAllFeeds(0, 1);
		timerId = SetTimer(NULL, 0, 30000, (TIMERPROC)timerProc);
	}
}

void UpdateListAdd(MCONTACT hContact)
{
	UPDATELIST *newItem = (UPDATELIST*)mir_alloc(sizeof(UPDATELIST));
	newItem->hContact = hContact;
	newItem->next = NULL;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	if (UpdateListTail == NULL)
		UpdateListHead = newItem;
	else UpdateListTail->next = newItem;
	UpdateListTail = newItem;

	ReleaseMutex(hUpdateMutex);
}

MCONTACT UpdateGetFirst()
{
	MCONTACT hContact = NULL;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	if (UpdateListHead != NULL) {
		UPDATELIST* Item = UpdateListHead;
		hContact = Item->hContact;
		UpdateListHead = Item->next;
		mir_free(Item);

		if (UpdateListHead == NULL)
			UpdateListTail = NULL;
	}

	ReleaseMutex(hUpdateMutex);

	return hContact;
}

void DestroyUpdateList(void)
{
	WaitForSingleObject(hUpdateMutex, INFINITE);

	// free the list one by one
	UPDATELIST *temp = UpdateListHead;
	while (temp != NULL) {
		UpdateListHead = temp->next;
		mir_free(temp);
		temp = UpdateListHead;
	}
	// make sure the entire list is clear
	UpdateListTail = NULL;

	ReleaseMutex(hUpdateMutex);
}

void UpdateThreadProc(void *AvatarCheck)
{
	WaitForSingleObject(hUpdateMutex, INFINITE);
	if (ThreadRunning) {
		ReleaseMutex(hUpdateMutex);
		return;
	}
	ThreadRunning = TRUE;	// prevent 2 instance of this thread running
	ReleaseMutex(hUpdateMutex);

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// update news by getting the first station from the queue until the queue is empty
	while (UpdateListHead != NULL && !Miranda_Terminated()) {
		if (AvatarCheck != NULL)
			CheckCurrentFeedAvatar(UpdateGetFirst());
		else
			CheckCurrentFeed(UpdateGetFirst());
	}

	// exit the update thread
	ThreadRunning = FALSE;

	CoUninitialize();
}
