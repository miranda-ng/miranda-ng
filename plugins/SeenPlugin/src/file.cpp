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

#include "stdafx.h"

bool g_bFileActive;
static wchar_t *g_ptszFileStamp, *g_ptszFileName;

/////////////////////////////////////////////////////////////////////////////////////////
// Prepares the log file:
// - calculates the absolute path (and store it in the db)
// - creates the directory

void InitFileOutput(void)
{
	ptrW tszFileName(g_plugin.getWStringA("FileName"));
	if (tszFileName == NULL)
		tszFileName = mir_wstrdup(DEFAULT_FILENAME);
	replaceStrW(g_ptszFileName, VARSW(tszFileName));

	wchar_t *tszPath = NEWWSTR_ALLOCA(g_ptszFileName);
	wchar_t *p = wcsrchr(tszPath, '\\');
	if (p) *p = 0;
	CreateDirectoryTreeW(tszPath);
	
	ptrW tszFileStamp(g_plugin.getWStringA("FileStamp"));
	replaceStrW(g_ptszFileStamp, (tszFileStamp == NULL) ? DEFAULT_FILESTAMP : tszFileStamp);
}

void UninitFileOutput()
{
	mir_free(g_ptszFileName);
	mir_free(g_ptszFileStamp);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Writes a line into the log.

void FileWrite(MCONTACT hcontact)
{
	FILE *log = _wfopen(ParseString(g_ptszFileName, hcontact), L"a");
	if (log != nullptr) {
		fputws(ParseString(g_ptszFileStamp, hcontact), log);
		fputs("\r\n", log);
		fclose(log);
	}
}
