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

#include "..\commonheaders.h"

int CDb3Mmap::SignatureValid(DWORD ofs, DWORD signature)
{
	if (ofs + sizeof(DWORD) >= sourceFileSize) {
		cb->pfnAddLogMessage(STATUS_ERROR, TranslateT("Invalid offset found (database truncated?)"));
		return 0;
	}

	DWORD *sig = (DWORD*)(m_pDbCache + ofs);
	return *sig == signature;
}

int CDb3Mmap::PeekSegment(DWORD ofs, PVOID buf, int cbBytes)
{
	if (ofs >= sourceFileSize) {
		cb->pfnAddLogMessage(STATUS_ERROR, TranslateT("Invalid offset found"));
		return ERROR_SEEK;
	}

	DWORD bytesRead;
	if (ofs + cbBytes > sourceFileSize)
		bytesRead = sourceFileSize - ofs;
	else
		bytesRead = cbBytes;

	if (bytesRead == 0) {
		cb->pfnAddLogMessage(STATUS_ERROR, TranslateT("Error reading, database truncated? (%u)"), GetLastError());
		return ERROR_READ_FAULT;
	}

	memcpy(buf, m_pDbCache + ofs, bytesRead);

	if ((int)bytesRead<cbBytes) return ERROR_HANDLE_EOF;
	return ERROR_SUCCESS;
}

int CDb3Mmap::ReadSegment(DWORD ofs, PVOID buf, int cbBytes)
{
	int ret = PeekSegment(ofs, buf, cbBytes);
	if (ret != ERROR_SUCCESS && ret != ERROR_HANDLE_EOF) return ret;

	if (cb->bAggressive) {
		if (ofs + cbBytes > sourceFileSize) {
			cb->pfnAddLogMessage(STATUS_WARNING, TranslateT("Can't write to working file, aggressive mode may be too aggressive now"));
			memset((m_pDbCache + ofs), 0, (sourceFileSize - ofs));
		}
		else memset((m_pDbCache + ofs), 0, cbBytes);
	}
	cb->spaceProcessed += cbBytes;
	return ERROR_SUCCESS;
}

DWORD CDb3Mmap::WriteSegment(DWORD ofs, PVOID buf, int cbBytes)
{
	DWORD bytesWritten;
	if (cb->bCheckOnly) return 0xbfbfbfbf;
	if (ofs == WSOFS_END) {
		ofs = m_dbHeader.ofsFileEnd;
		m_dbHeader.ofsFileEnd += cbBytes;
	}
	SetFilePointer(cb->hOutFile, ofs, NULL, FILE_BEGIN);
	WriteFile(cb->hOutFile, buf, cbBytes, &bytesWritten, NULL);
	if ((int)bytesWritten < cbBytes) {
		cb->pfnAddLogMessage(STATUS_FATAL, TranslateT("Can't write to output file - disk full? (%u)"), GetLastError());
		return WS_ERROR;
	}
	return ofs;
}

int CDb3Mmap::ReadWrittenSegment(DWORD ofs, PVOID buf, int cbBytes)
{
	DWORD bytesRead;
	if (cb->bCheckOnly) return 0xbfbfbfbf;
	if (ofs + cbBytes > m_dbHeader.ofsFileEnd)
		return ERROR_SEEK;

	SetFilePointer(cb->hOutFile, ofs, NULL, FILE_BEGIN);
	ReadFile(cb->hOutFile, buf, cbBytes, &bytesRead, NULL);
	if ((int)bytesRead < cbBytes)
		return ERROR_READ_FAULT;

	return ERROR_SUCCESS;
}
