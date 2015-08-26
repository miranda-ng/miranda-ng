/*
FTP File YM plugin
Copyright (C) 2007-2010 Jan Holub

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

DeleteTimer *DeleteTimer::instance = NULL;
DeleteTimer &deleteTimer = DeleteTimer::getInstance();

extern Options &opt;

void DeleteTimer::init()
{
	timerId = 0;
	if (opt.bAutoDelete)
		start();
}

void DeleteTimer::deinit()
{
	stop();
	delete this;
}

void DeleteTimer::start()
{
	if (!timerId)
		timerId = SetTimer(NULL, 0, 1000 * 60 * 5, (TIMERPROC)AutoDeleteTimerProc);
}

void DeleteTimer::stop()
{
	if (timerId) {
		KillTimer(NULL, timerId);
		timerId = 0;
	}
}

void CALLBACK DeleteTimer::AutoDeleteTimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	mir_cslock lock(DBEntry::mutexDB);

	DBEntry *entry = DBEntry::getFirst();
	while (entry != NULL) {
		if (entry->m_deleteTS > 0 && entry->m_deleteTS < time(NULL)) {
			DeleteJob *job = new DeleteJob(new DBEntry(entry), NULL);
			job->start();
		}

		entry = DBEntry::getNext(entry);
	}
}
