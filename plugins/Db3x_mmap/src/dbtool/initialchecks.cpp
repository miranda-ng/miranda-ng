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

int CDb3Mmap::WorkInitialCheckHeaders()
{
	if (memcmp(m_dbHeader.signature, &dbSignatureU, sizeof(m_dbHeader.signature)) &&
		memcmp(m_dbHeader.signature, &dbSignatureE, sizeof(m_dbHeader.signature)) &&
		memcmp(m_dbHeader.signature, &dbSignatureIM, sizeof(m_dbHeader.signature)) &&
		memcmp(m_dbHeader.signature, &dbSignatureSA, sizeof(m_dbHeader.signature)))
	{
		cb->pfnAddLogMessage(STATUS_FATAL, TranslateT("Database signature is corrupted, automatic repair is impossible"));
		return ERROR_BAD_FORMAT;
	}

	switch (m_dbHeader.version) {
	case DB_OLD_VERSION:
	case DB_094_VERSION:
	case DB_095_VERSION:
	case DB_095_1_VERSION:
		break;

	default:
		cb->pfnAddLogMessage(STATUS_FATAL, TranslateT("Database version doesn't match this driver's one. Convert a database first"));
		return ERROR_BAD_FORMAT;
	}

	return ERROR_SUCCESS;
}

int CDb3Mmap::WorkInitialChecks(int)
{
	sourceFileSize = GetFileSize(m_hDbFile, NULL);
	if (sourceFileSize == 0) {
		cb->pfnAddLogMessage(STATUS_WARNING, TranslateT("Database is newly created and has no data to process"));
		cb->pfnAddLogMessage(STATUS_SUCCESS, TranslateT("Processing completed successfully"));
		return ERROR_INVALID_DATA;
	}

	int res = WorkInitialCheckHeaders();
	if (res)
		return res;

	if (!m_pDbCache) {
		cb->pfnAddLogMessage(STATUS_FATAL, TranslateT("Can't create map view of file (%u)"), GetLastError());
		return ERROR_ACCESS_DENIED;
	}
	if (ReadSegment(0, &m_dbHeader, sizeof(m_dbHeader)) != ERROR_SUCCESS)
		return ERROR_READ_FAULT;

	if (WriteSegment(0, &m_dbHeader, sizeof(m_dbHeader)) == WS_ERROR)
		return ERROR_HANDLE_DISK_FULL;

	cb->spaceUsed = m_dbHeader.ofsFileEnd - m_dbHeader.slackSpace;
	m_dbHeader.ofsFileEnd = sizeof(m_dbHeader);
	return ERROR_NO_MORE_ITEMS;
}
