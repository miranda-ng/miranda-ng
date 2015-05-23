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

#include "common.h"

DeleteTimer *DeleteTimer::instance = NULL;
DeleteTimer &deleteTimer = DeleteTimer::getInstance();

extern Options &opt;

void DeleteTimer::init()
{ 
	timerId = 0;
	if (opt.bAutoDelete) 
		this->start(); 
}

void DeleteTimer::deinit()
{ 
	this->stop();
	delete this;
}

void DeleteTimer::start()
{
	if (!timerId)
		timerId = SetTimer(NULL, 0, 1000 * 60 * 5, (TIMERPROC)AutoDeleteTimerProc);
}

void DeleteTimer::stop()
{
	if (timerId)
	{
		KillTimer(NULL, timerId);
		timerId = 0;
	}
}

void CALLBACK DeleteTimer::AutoDeleteTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
	mir_cslock lock(DBEntry::mutexDB);

	DBEntry *entry = DBEntry::getFirts();
	while (entry != NULL)
	{
		if (entry->deleteTS > 0 && entry->deleteTS < time(NULL))
		{
			DeleteJob *job = new DeleteJob(new DBEntry(entry), NULL);
			job->start();
		}	

		entry = DBEntry::getNext(entry);
	}
}