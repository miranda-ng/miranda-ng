/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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

#include "stdafx.h"

void ProcessingDone(void);

void __cdecl WorkerThread(DbToolOptions *opts)
{
	time_t ts = time(nullptr);

	AddToStatus(STATUS_MESSAGE, TranslateT("Database worker thread activated"));

	DWORD sp = 0;

	if (opts->bMarkRead) {
		int nCount = 0;

		for (auto &cc : Contacts()) {
			DB::ECPTR pCursor(DB::Events(cc));
			while (MEVENT hEvent = pCursor.FetchNext()) {
				DBEVENTINFO dbei = {};
				if (db_event_get(hEvent, &dbei))
					continue;

				if (!dbei.markedRead()) {
					db_event_markRead(cc, hEvent);
					nCount++;
				}
			}
		}

		if (nCount)
			AddToStatus(STATUS_MESSAGE, TranslateT("%d events marked as read"), nCount);
	}

	DBCHeckCallback callback;
	callback.pfnAddLogMessage = AddToStatus;
	opts->dbChecker->Start(&callback);

	for (int task = 0;; task++) {
		if (callback.spaceProcessed / (callback.spaceUsed / 1000 + 1) > sp) {
			sp = callback.spaceProcessed / (callback.spaceUsed / 1000 + 1);
			SetProgressBar(sp);
		}
		WaitForSingleObject(opts->hEventRun, INFINITE);
		if (WaitForSingleObject(opts->hEventAbort, 0) == WAIT_OBJECT_0) {
			AddToStatus(STATUS_FATAL, TranslateT("Processing aborted by user"));
			break;
		}

		int ret = opts->dbChecker->CheckDb(task);
		if (ret == ERROR_OUT_OF_PAPER) {
			opts->dbChecker->Destroy();
			opts->dbChecker = nullptr;

			AddToStatus(STATUS_MESSAGE, TranslateT("Elapsed time: %d sec"), time(nullptr) - ts);
			if (errorCount)
				AddToStatus(STATUS_SUCCESS, TranslateT("All tasks completed but with errors (%d)"), errorCount);
			else
				AddToStatus(STATUS_SUCCESS, TranslateT("All tasks completed successfully"));
			break;
		}
		else if (ret != ERROR_SUCCESS)
			break;
	}

	opts->bFinished = true;
	ProcessingDone();
}
