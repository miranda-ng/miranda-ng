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
static CMStringW g_ptszFileStamp, g_ptszFileName;

/////////////////////////////////////////////////////////////////////////////////////////
// Prepares the log file:
// - calculates the absolute path (and store it in the db)
// - creates the directory

void InitFileOutput(void)
{
	g_ptszFileName = VARSW(g_plugin.getMStringW("FileName", DEFAULT_FILENAME));
	CreatePathToFileW(g_ptszFileName);
	
	g_ptszFileStamp = g_plugin.getMStringW("FileStamp", DEFAULT_FILESTAMP);
}

void UninitFileOutput()
{
	g_ptszFileName.Empty();
	g_ptszFileStamp.Empty();
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
