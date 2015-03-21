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

BOOL g_bFileActive;
static TCHAR *g_ptszFileStamp, *g_ptszFileName;

/////////////////////////////////////////////////////////////////////////////////////////
// Prepares the log file:
// - calculates the absolute path (and store it in the db)
// - creates the directory

void InitFileOutput(void)
{
	ptrT tszFileName(db_get_tsa(NULL, S_MOD, "FileName"));
	if (tszFileName == NULL)
		tszFileName = mir_tstrdup(DEFAULT_FILENAME);
	replaceStrT(g_ptszFileName, VARST(tszFileName));

	TCHAR *tszPath = NEWTSTR_ALLOCA(g_ptszFileName);
	TCHAR *p = _tcsrchr(tszPath, '\\');
	if (p) *p = 0;
	CreateDirectoryTreeT(tszPath);
	
	ptrT tszFileStamp(db_get_tsa(NULL, S_MOD, "FileStamp"));
	replaceStrT(g_ptszFileStamp, (tszFileStamp == NULL) ? DEFAULT_FILESTAMP : tszFileStamp);
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
	FILE *log = _tfopen(ParseString(g_ptszFileName, hcontact, 1), _T("a"));
	if (log != NULL) {
		_fputts(ParseString(g_ptszFileStamp, hcontact, 1), log);
		fputs("\r\n", log);
		fclose(log);
	}
}
