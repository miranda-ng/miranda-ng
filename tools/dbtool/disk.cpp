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

extern DWORD spaceProcessed,sourceFileSize;

int SignatureValid(DWORD ofs,DWORD signature)
{
	DWORD sig;

	if(ofs+sizeof(sig)>=sourceFileSize)	{
		AddToStatus(STATUS_ERROR,TranslateT("Invalid offset found (database truncated?)"));
		return 0;
	}

	sig = *(DWORD*)(opts.pFile+ofs);

	return sig==signature;
}

int PeekSegment(DWORD ofs,PVOID buf,int cbBytes)
{
	DWORD bytesRead;

	if(ofs>=sourceFileSize) {
		AddToStatus(STATUS_ERROR,TranslateT("Invalid offset found"));
		return ERROR_SEEK;
	}

	if (ofs+cbBytes>sourceFileSize)
		bytesRead = sourceFileSize - ofs;
	else
		bytesRead = cbBytes;

	if(bytesRead==0) {
		AddToStatus(STATUS_ERROR,TranslateT("Error reading, database truncated? (%u)"),GetLastError());
		return ERROR_READ_FAULT;
	}

	CopyMemory(buf, opts.pFile+ofs, bytesRead);

	if((int)bytesRead<cbBytes) return ERROR_HANDLE_EOF;
	return ERROR_SUCCESS;
}

int ReadSegment(DWORD ofs,PVOID buf,int cbBytes)
{
	int ret;

	ret=PeekSegment(ofs,buf,cbBytes);
	if(ret!=ERROR_SUCCESS && ret!=ERROR_HANDLE_EOF) return ret;

	if(opts.bAggressive) {
		if (ofs+cbBytes>sourceFileSize) {
			AddToStatus(STATUS_WARNING,TranslateT("Can't write to working file, aggressive mode may be too aggressive now"));
			ZeroMemory(opts.pFile+ofs,sourceFileSize-ofs);
		}
		else
			ZeroMemory(opts.pFile+ofs,cbBytes);
	}
	spaceProcessed+=cbBytes;
	return ERROR_SUCCESS;
}

DWORD WriteSegment(DWORD ofs,PVOID buf,int cbBytes)
{
	DWORD bytesWritten;
	if(opts.bCheckOnly) return 0xbfbfbfbf;
	if(ofs==WSOFS_END) {
		ofs=dbhdr.ofsFileEnd;
		dbhdr.ofsFileEnd+=cbBytes;
	}
	SetFilePointer(opts.hOutFile,ofs,NULL,FILE_BEGIN);
	WriteFile(opts.hOutFile,buf,cbBytes,&bytesWritten,NULL);
	if((int)bytesWritten<cbBytes) {
		AddToStatus(STATUS_FATAL,TranslateT("Can't write to output file - disk full? (%u)"),GetLastError());
		return WS_ERROR;
	}
	return ofs;
}


int ReadWrittenSegment(DWORD ofs,PVOID buf,int cbBytes)
{
	DWORD bytesRead;
	if(opts.bCheckOnly) return 0xbfbfbfbf;
	if(ofs + cbBytes > dbhdr.ofsFileEnd )
		return ERROR_SEEK;

	SetFilePointer(opts.hOutFile,ofs,NULL,FILE_BEGIN);
	ReadFile(opts.hOutFile,buf,cbBytes,&bytesRead,NULL);
	if((int)bytesRead<cbBytes)
		return ERROR_READ_FAULT;

	return ERROR_SUCCESS;
}
