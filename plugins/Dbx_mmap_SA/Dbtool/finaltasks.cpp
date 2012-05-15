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
#include "dbtool.h"

extern int errorCount;

int WorkFinalTasks(int firstTime)
{
	FreeModuleChain();
	AddToStatus(STATUS_MESSAGE,TranslateT("Processing final tasks"));
	dbhdr.slackSpace=0;
	if(WriteSegment(0,&dbhdr,sizeof(dbhdr))==WS_ERROR)
		return ERROR_WRITE_FAULT;
	if (opts.hFile) {
		CloseHandle(opts.hFile);
		opts.hFile = NULL;
	}
	if ( opts.hOutFile ) {
		CloseHandle(opts.hOutFile);
		opts.hOutFile = NULL;
	}
	if (opts.pFile) {
		UnmapViewOfFile(opts.pFile);
		opts.pFile = NULL;
	}
	if (opts.hMap) {
		CloseHandle(opts.hMap);
		opts.hMap = NULL;
	}
	if(errorCount && !opts.bBackup && !opts.bCheckOnly) {
		extern time_t ts;
		time_t dlg_ts = time(NULL);
		if ( IDYES == MessageBox( NULL,
							TranslateT("Errors were encountered, however you selected not to backup the original database. It is strongly recommended that you do so in case important data was omitted. Do you wish to keep a backup of the original database?"),
							TranslateT("Miranda Database Tool"), MB_YESNO ))
			opts.bBackup = 1;
		ts += time(NULL) - dlg_ts;
	}
	if(opts.bBackup) {
		int i;
		TCHAR dbPath[MAX_PATH],dbFile[MAX_PATH];
		_tcscpy( dbPath, opts.filename );
		TCHAR* str2 = _tcsrchr( dbPath, '\\' );
		if ( str2 != NULL ) {
			_tcscpy( dbFile, str2+1 );
			*str2 = 0;
		}
		else {
			_tcscpy( dbFile, dbPath );
			dbPath[0] = 0;
		}
		for(i=1;;i++) {
			if(i==1) wsprintf(opts.backupFilename,TranslateT("%s\\Backup of %s"),dbPath,dbFile);
			else wsprintf(opts.backupFilename,TranslateT("%s\\Backup (%d) of %s"),dbPath,i,dbFile);
			if(_taccess(opts.backupFilename,0)==-1) break;
		}
		MoveFile(opts.filename,opts.backupFilename) || AddToStatus(STATUS_WARNING,TranslateT("Unable to rename original file"));
	}
	else if(!opts.bCheckOnly) DeleteFile(opts.filename) || AddToStatus(STATUS_WARNING,TranslateT("Unable to delete original file"));
	if(!opts.bCheckOnly) MoveFile(opts.outputFilename,opts.filename) || AddToStatus(STATUS_WARNING,TranslateT("Unable to rename output file"));
	return ERROR_NO_MORE_ITEMS;
}
