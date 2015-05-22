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

int DBEntry::entryID;
Mutex DBEntry::mutexDB;

DBEntry::DBEntry()
{ }

DBEntry::DBEntry(DBEntry *entry)
{
	this->fileID = entry->fileID;
	this->iFtpNum = entry->iFtpNum;
	mir_strcpy(this->szFileName, entry->szFileName);
}

DBEntry *DBEntry::getFirts()
{
	entryID = 0;
	DBEntry *entry = new DBEntry();
	return getNext(entry);
}

DBEntry *DBEntry::getNext(DBEntry *entry)
{
	char szValue[256];
	int count = db_get_dw(0, MODULE_FILES, "NextFileID", 0);

	for (; entryID < count; entryID++) 
	{
		int ftpNum = DB::getByteF(0, MODULE_FILES, "Ftp%d", entryID, -1);
		if (ftpNum != -1)
		{	
			if (!DB::getAStringF(0, MODULE_FILES, "Filename%d", entryID, szValue))
			{
				entry->fileID = entryID;
				entry->iFtpNum = ftpNum;
				mir_strcpy(entry->szFileName, szValue);
				entry->deleteTS = DB::getDwordF(0, MODULE_FILES, "DeleteTS%d", entryID, 0);
				entryID++;
				return entry;
			}
		}
	}

	delete entry;
	return NULL;
}

void DBEntry::cleanupDB()
{
	int count = 0;
	
	DBEntry *entry = getFirts();
	while (entry != NULL)
	{
		DB::setByteF(0, MODULE_FILES, "Ftp%d", count, entry->iFtpNum);
		DB::setAStringF(0, MODULE_FILES, "Filename%d", count, entry->szFileName);
		if (entry->deleteTS != 0)
			DB::setDwordF(0, MODULE_FILES, "DeleteTS%d", count, entry->deleteTS);

		count++; 
		entry = getNext(entry);
	}

	db_set_dw(0, MODULE_FILES, "NextFileID", count);
}

DBEntry *DBEntry::get(int fileID)
{
	char szValue[256];
	DBEntry *entry = new DBEntry();

	int ftpNum = DB::getByteF(0, MODULE_FILES, "Ftp%d", fileID, -1);
	if (ftpNum != -1)
	{	
		if (!DB::getAStringF(0, MODULE_FILES, "Filename%d", fileID, szValue))
		{
			entry->fileID = fileID;
			entry->iFtpNum = ftpNum;
			mir_strcpy(entry->szFileName, szValue);
			entry->deleteTS = DB::getDwordF(0, MODULE_FILES, "DeleteTS%d", fileID, 0);
			return entry;
		}
	}

	return NULL;
}

void DBEntry::remove(int fileID)
{
	DB::deleteSettingF(0, MODULE_FILES, "Ftp%d", fileID);
	DB::deleteSettingF(0, MODULE_FILES, "Filename%d", fileID);
	DB::deleteSettingF(0, MODULE_FILES, "DeleteTS%d", fileID);
}

bool DBEntry::entryExists(GenericJob *job)
{
	Lock *lock = new Lock(mutexDB);

	DBEntry *entry = getFirts();
	while (entry != NULL)
	{
		if (entry->iFtpNum == job->iFtpNum && !strcmp(entry->szFileName, job->szSafeFileName))
		{
			delete lock;
			return true;
		}
		entry = getNext(entry);
	}

	delete lock;
	return false;
}

void DBEntry::add(GenericJob *job)
{
	if (entryExists(job)) 
		return;

	Lock *lock = new Lock(mutexDB);
	int id = db_get_dw(0, MODULE_FILES, "NextFileID", 0);
	DB::setByteF(0, MODULE_FILES, "Ftp%d", id, job->iFtpNum);
	DB::setAStringF(0, MODULE_FILES, "Filename%d", id, job->szSafeFileName);

	if (job->tab->iOptAutoDelete != -1) 
	{
		time_t deleteTS = time(NULL);
		deleteTS += (job->tab->iOptAutoDelete * 60);
		DB::setDwordF(0, MODULE_FILES, "DeleteTS%d", id, deleteTS);
	}
	
	db_set_dw(0, MODULE_FILES, "NextFileID", id + 1);
	job->fileID = id;

	delete lock;	
}

void DBEntry::setDeleteTS(GenericJob *job)
{
	if (job->tab->iOptAutoDelete != -1) 
	{
		time_t deleteTS = time(NULL);
		deleteTS += (job->tab->iOptAutoDelete * 60);
		DB::setDwordF(0, MODULE_FILES, "DeleteTS%d", job->fileID, deleteTS);
	}
	else
	{
		DB::deleteSettingF(0, MODULE_FILES, "DeleteTS%d", job->fileID);
	}
}