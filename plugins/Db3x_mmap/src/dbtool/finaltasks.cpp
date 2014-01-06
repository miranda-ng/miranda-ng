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

int CDb3Base::WorkFinalTasks(int firstTime)
{
	FreeModuleChain();
	cb->pfnAddLogMessage(STATUS_MESSAGE, TranslateT("Processing final tasks"));
	m_dbHeader.slackSpace = 0;
	if (WriteSegment(0, &m_dbHeader, sizeof(m_dbHeader)) == WS_ERROR)
		return ERROR_WRITE_FAULT;

	if (m_hDbFile) {
		CloseHandle(m_hDbFile);
		m_hDbFile = NULL;
	}
	if (m_pDbCache) {
		UnmapViewOfFile(m_pDbCache);
		m_pDbCache = NULL;
	}
	if (m_hMap) {
		CloseHandle(m_hMap);
		m_hMap = NULL;
	}
	return ERROR_NO_MORE_ITEMS;
}
