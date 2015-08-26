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

int DBEntry::entryID;
mir_cs DBEntry::mutexDB;

DBEntry::DBEntry()
{
}

DBEntry::DBEntry(DBEntry *entry)
{
	m_fileID = entry->m_fileID;
	m_iFtpNum = entry->m_iFtpNum;
	mir_strcpy(m_szFileName, entry->m_szFileName);
}

DBEntry* DBEntry::getFirst()
{
	entryID = 0;
	DBEntry *entry = new DBEntry();
	return getNext(entry);
}

DBEntry *DBEntry::getNext(DBEntry *entry)
{
	char szValue[256];
	int count = db_get_dw(0, MODULE_FILES, "NextFileID", 0);

	for (; entryID < count; entryID++) { 
		int ftpNum = DB::getByteF(0, MODULE_FILES, "Ftp%d", entryID, -1);
		if (ftpNum != -1) {
			if (!DB::getAStringF(0, MODULE_FILES, "Filename%d", entryID, szValue)) {
				entry->m_fileID = entryID;
				entry->m_iFtpNum = ftpNum;
				mir_strcpy(entry->m_szFileName, szValue);
				entry->m_deleteTS = DB::getDwordF(0, MODULE_FILES, "DeleteTS%d", entryID, 0);
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

	DBEntry *entry = getFirst();
	while (entry != NULL) {
		DB::setByteF(0, MODULE_FILES, "Ftp%d", count, entry->m_iFtpNum);
		DB::setAStringF(0, MODULE_FILES, "Filename%d", count, entry->m_szFileName);
		if (entry->m_deleteTS != 0)
			DB::setDwordF(0, MODULE_FILES, "DeleteTS%d", count, entry->m_deleteTS);

		count++;
		entry = getNext(entry);
	}

	db_set_dw(0, MODULE_FILES, "NextFileID", count);
}

DBEntry* DBEntry::get(int fileID)
{
	char szValue[256];
	DBEntry *entry = new DBEntry();

	int ftpNum = DB::getByteF(0, MODULE_FILES, "Ftp%d", fileID, -1);
	if (ftpNum != -1) {
		if (!DB::getAStringF(0, MODULE_FILES, "Filename%d", fileID, szValue)) {
			entry->m_fileID = fileID;
			entry->m_iFtpNum = ftpNum;
			mir_strcpy(entry->m_szFileName, szValue);
			entry->m_deleteTS = DB::getDwordF(0, MODULE_FILES, "DeleteTS%d", fileID, 0);
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
	mir_cslock lock(mutexDB);

	DBEntry *entry = getFirst();
	while (entry != NULL) {
		if (entry->m_iFtpNum == job->m_iFtpNum && !strcmp(entry->m_szFileName, job->m_szSafeFileName))
			return true;

		entry = getNext(entry);
	}

	return false;
}

void DBEntry::add(GenericJob *job)
{
	if (entryExists(job))
		return;

	mir_cslock lock(mutexDB);
	int id = db_get_dw(0, MODULE_FILES, "NextFileID", 0);
	DB::setByteF(0, MODULE_FILES, "Ftp%d", id, job->m_iFtpNum);
	DB::setAStringF(0, MODULE_FILES, "Filename%d", id, job->m_szSafeFileName);

	if (job->m_tab->m_iOptAutoDelete != -1) {
		time_t deleteTS = time(NULL);
		deleteTS += (job->m_tab->m_iOptAutoDelete * 60);
		DB::setDwordF(0, MODULE_FILES, "DeleteTS%d", id, deleteTS);
	}

	db_set_dw(0, MODULE_FILES, "NextFileID", id + 1);
	job->m_fileID = id;
}

void DBEntry::setDeleteTS(GenericJob *job)
{
	if (job->m_tab->m_iOptAutoDelete != -1) {
		time_t deleteTS = time(NULL);
		deleteTS += (job->m_tab->m_iOptAutoDelete * 60);
		DB::setDwordF(0, MODULE_FILES, "DeleteTS%d", job->m_fileID, deleteTS);
	}
	else DB::deleteSettingF(0, MODULE_FILES, "DeleteTS%d", job->m_fileID);
}
