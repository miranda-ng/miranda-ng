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
static mir_cs g_csUpdate;
static std::vector<MCONTACT> g_arUpdates;

// main auto-update timer
void CALLBACK timerProc(HWND, UINT, UINT_PTR, DWORD)
{
	// only run if it is not current updating and the auto update option is enabled
	if (!ThreadRunning && !Miranda_IsTerminated()) {
		bool HaveUpdates = FALSE;
		for (auto &hContact : Contacts(MODULENAME)) {
			if (g_plugin.getDword(hContact, "UpdateTime", DEFAULT_UPDATE_TIME)) {
				double diff = difftime(time(0), (time_t)g_plugin.getDword(hContact, "LastCheck", 0));
				if (g_plugin.getByte("AutoUpdate", 1) != 0 && diff >= g_plugin.getDword(hContact, "UpdateTime", DEFAULT_UPDATE_TIME) * 60) {
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
	mir_cslock lck(g_csUpdate);
	g_arUpdates.push_back(hContact);
}

void UpdateThreadProc(void *AvatarCheck)
{
	{
		mir_cslock lck(g_csUpdate);
		if (ThreadRunning)
			return;

		ThreadRunning = TRUE;	// prevent 2 instance of this thread running
	}

	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

	// update news by getting the first station from the queue until the queue is empty
	while (!Miranda_IsTerminated()) {
		MCONTACT hContact;
		{
			mir_cslock lck(g_csUpdate);
			if (g_arUpdates.empty())
				break;

			hContact = g_arUpdates[0];
			g_arUpdates.erase(g_arUpdates.begin());
		}

		if (AvatarCheck != nullptr)
			CheckCurrentFeedAvatar(hContact);
		else
			CheckCurrentFeed(hContact);
	}

	// exit the update thread
	ThreadRunning = FALSE;

	CoUninitialize();
}
