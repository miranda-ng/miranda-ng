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

#pragma once

#include "common.h"
#include "job_upload.h"
#include "utils.h"

class DBEntry
{
private:
	static int entryID;
	static bool entryExists(GenericJob *job);

public:
	int fileID;
	int iFtpNum;
	char szFileName[256];
	DWORD deleteTS;

	static mir_cs mutexDB;

	DBEntry();
	DBEntry(DBEntry *entry);

	static DBEntry *getFirts();
	static DBEntry *getNext(DBEntry *entry);
	static void cleanupDB();

	static DBEntry *get(int fileID);
	static void remove(int fileID);

	static void add(GenericJob *job);
	static void setDeleteTS(GenericJob *job);
};
