/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "seen.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Prepares the log file:
// - calculates the absolute path (and store it in the db)
// - creates the directory

int InitFileOutput(void)
{
	TCHAR szfpath[MAX_PATH], szmpath[MAX_PATH];
	GetModuleFileName(NULL, szmpath, MAX_PATH);

	DBVARIANT dbv;
	if (!db_get_ts(NULL, S_MOD, "FileName", &dbv)) {
		_tcsncpy(szfpath, dbv.ptszVal, MAX_PATH);
		db_free(&dbv);
	}
	else _tcsncpy(szfpath, DEFAULT_FILENAME, MAX_PATH);

	if (szfpath[0] == '\\')
		_tcsncpy(szfpath, szfpath + 1, MAX_PATH);

	TCHAR *str = _tcsrchr(szmpath, '\\');
	if (str != NULL)
		*++str = 0;

	_tcsncat(szmpath, szfpath, MAX_PATH);
	_tcsncpy(szfpath, szmpath, MAX_PATH);

	str = _tcsrchr(szmpath, '\\');
	if (str != NULL)
		*++str = 0;

	db_set_ts(NULL, S_MOD, "PathToFile", szfpath);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Writes a line into the log.

void FileWrite(MCONTACT hcontact)
{
	TCHAR szout[1024];

	DBVARIANT dbv;
	if (!db_get_ts(NULL, S_MOD, "PathToFile", &dbv)) {
		_tcsncpy(szout, ParseString(dbv.ptszVal, hcontact, 1), SIZEOF(szout));
		db_free(&dbv);
	}
	else _tcsncpy(szout, DEFAULT_FILENAME, SIZEOF(szout));

	HANDLE fhout = CreateFile(szout, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
	if (fhout == INVALID_HANDLE_VALUE) {
		TCHAR fullpath[1024];
		_tcsncpy(fullpath, szout, SIZEOF(fullpath));
		TCHAR *dirpath = _tcsrchr(fullpath, '\\');
		if (dirpath != NULL)
			*dirpath = '\0';
		CreateDirectoryTreeT(fullpath);
		fhout = CreateFile(szout, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
		if (fhout == INVALID_HANDLE_VALUE)
			return;
	}
	SetFilePointer(fhout, 0, 0, FILE_END);

	if (!db_get_ts(NULL, S_MOD, "FileStamp", &dbv)) {
		_tcsncpy(szout, ParseString(dbv.ptszVal, hcontact, 1), SIZEOF(szout));
		db_free(&dbv);
	}
	else _tcsncpy(szout, ParseString(DEFAULT_FILESTAMP, hcontact, 1), SIZEOF(szout));

	DWORD byteswritten;
	WriteFile(fhout, _T2A(szout), (DWORD)_tcslen(szout), &byteswritten, NULL);
	WriteFile(fhout, "\r\n", 2, &byteswritten, NULL);

	CloseHandle(fhout);
}
