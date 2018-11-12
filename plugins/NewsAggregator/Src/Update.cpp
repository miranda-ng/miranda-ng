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

#include "stdafx.h"

// check if Feed is currently updating
bool ThreadRunning;
UPDATELIST *UpdateListHead = nullptr;
UPDATELIST *UpdateListTail = nullptr;

// main auto-update timer
void CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD)
{
	// only run if it is not current updating and the auto update option is enabled
	if (!ThreadRunning && !Miranda_IsTerminated()) {
		bool HaveUpdates = FALSE;
		for (auto &hContact : Contacts(MODULENAME)) {
			if (db_get_dw(hContact, MODULENAME, "UpdateTime", DEFAULT_UPDATE_TIME)) {
				double diff = difftime(time(0), (time_t)db_get_dw(hContact, MODULENAME, "LastCheck", 0));
				if (g_plugin.getByte("AutoUpdate", 1) != 0 && diff >= db_get_dw(hContact, MODULENAME, "UpdateTime", DEFAULT_UPDATE_TIME) * 60) {
					UpdateListAdd(hContact);
					HaveUpdates = TRUE;
				}
			}
		}
		if (!ThreadRunning && HaveUpdates)
			mir_forkthread(UpdateThreadProc);
	}
}

// temporary timer for first run
// when this is run, it kill the old startup timer and create the permenant one above
void CALLBACK timerProc2(HWND, UINT, UINT_PTR, DWORD)
{
	KillTimer(nullptr, timerId);
	ThreadRunning = FALSE;

	if (g_plugin.getByte("AutoUpdate", 1) && !Miranda_IsTerminated()) {
		if (g_plugin.getByte("StartupRetrieve", 1))
			CheckAllFeeds(0, 1);
		timerId = SetTimer(nullptr, 0, 30000, (TIMERPROC)timerProc);
	}
}

void UpdateListAdd(MCONTACT hContact)
{
	UPDATELIST *newItem = (UPDATELIST*)mir_alloc(sizeof(UPDATELIST));
	newItem->hContact = hContact;
	newItem->next = nullptr;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	if (UpdateListTail == nullptr)
		UpdateListHead = newItem;
	else UpdateListTail->next = newItem;
	UpdateListTail = newItem;

	ReleaseMutex(hUpdateMutex);
}

MCONTACT UpdateGetFirst()
{
	MCONTACT hContact = NULL;

	WaitForSingleObject(hUpdateMutex, INFINITE);

	if (UpdateListHead != nullptr) {
		UPDATELIST* Item = UpdateListHead;
		hContact = Item->hContact;
		UpdateListHead = Item->next;
		mir_free(Item);

		if (UpdateListHead == nullptr)
			UpdateListTail = nullptr;
	}

	ReleaseMutex(hUpdateMutex);

	return hContact;
}

void DestroyUpdateList(void)
{
	WaitForSingleObject(hUpdateMutex, INFINITE);

	// free the list one by one
	UPDATELIST *temp = UpdateListHead;
	while (temp != nullptr) {
		UpdateListHead = temp->next;
		mir_free(temp);
		temp = UpdateListHead;
	}
	// make sure the entire list is clear
	UpdateListTail = nullptr;

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

	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	// update news by getting the first station from the queue until the queue is empty
	while (UpdateListHead != nullptr && !Miranda_IsTerminated()) {
		if (AvatarCheck != nullptr)
			CheckCurrentFeedAvatar(UpdateGetFirst());
		else
			CheckCurrentFeed(UpdateGetFirst());
	}

	// exit the update thread
	ThreadRunning = FALSE;

	CoUninitialize();
}
