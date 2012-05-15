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

extern DWORD sourceFileSize,spaceUsed;

int WorkInitialChecks(int firstTime)
{
	DWORD bytesRead;

	sourceFileSize=GetFileSize(opts.hFile,NULL);
	if(sourceFileSize==0) {
		AddToStatus(STATUS_WARNING,TranslateT("Database is newly created and has no data to process"));
		AddToStatus(STATUS_SUCCESS,TranslateT("Processing completed successfully"));
		return ERROR_INVALID_DATA;
	}
	ReadFile(opts.hFile,&dbhdr,sizeof(dbhdr),&bytesRead,NULL);
	if(bytesRead<sizeof(dbhdr)) {
		AddToStatus(STATUS_FATAL,TranslateT("Database is corrupted and too small to contain any recoverable data"));
		return ERROR_BAD_FORMAT;
	}
	if(memcmp(dbhdr.signature,&dbSignature,sizeof(dbhdr.signature))) {
		AddToStatus(STATUS_FATAL,TranslateT("Database signature is corrupted, automatic repair is impossible"));
		return ERROR_BAD_FORMAT;
	}
	if(dbhdr.version!=0x00000700) {
		AddToStatus(STATUS_FATAL,TranslateT("Database is marked as belonging to an unknown version of Miranda"));
		return ERROR_BAD_FORMAT;
	}
	_tcscpy(opts.workingFilename,opts.filename);

	if(opts.bCheckOnly) {
		_tcscpy( opts.outputFilename, TranslateT("<check only>"));
		opts.hOutFile=INVALID_HANDLE_VALUE;
	}
	else {
		_tcscpy(opts.outputFilename,opts.filename);
		*_tcsrchr(opts.outputFilename,'.')=0;
		_tcscat(opts.outputFilename,TranslateT(" (Output).dat"));
		opts.hOutFile = CreateFile(opts.outputFilename,GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
		if ( opts.hOutFile == INVALID_HANDLE_VALUE ) {
			AddToStatus(STATUS_FATAL,TranslateT("Can't create output file (%u)"),GetLastError());
			return ERROR_ACCESS_DENIED;
		}
	}

	opts.hMap = CreateFileMapping(opts.hFile, NULL, opts.bAggressive?PAGE_WRITECOPY:PAGE_READONLY, 0, 0, NULL);

	if (opts.hMap)
		opts.pFile = (BYTE*)MapViewOfFile(opts.hMap, opts.bAggressive?FILE_MAP_COPY:FILE_MAP_READ, 0, 0 ,0);
	else {
		AddToStatus(STATUS_FATAL,TranslateT("Can't create file mapping (%u)"),GetLastError());
		return ERROR_ACCESS_DENIED;
	}

	if (!opts.pFile) {
		AddToStatus(STATUS_FATAL,TranslateT("Can't create map view of file (%u)"),GetLastError());
		return ERROR_ACCESS_DENIED;
	}
	if(ReadSegment(0,&dbhdr,sizeof(dbhdr))!=ERROR_SUCCESS) return ERROR_READ_FAULT;
	if(WriteSegment(0,&dbhdr,sizeof(dbhdr))==WS_ERROR) return ERROR_HANDLE_DISK_FULL;
	spaceUsed=dbhdr.ofsFileEnd-dbhdr.slackSpace;
	dbhdr.ofsFileEnd=sizeof(dbhdr);
	return ERROR_NO_MORE_ITEMS;
}
