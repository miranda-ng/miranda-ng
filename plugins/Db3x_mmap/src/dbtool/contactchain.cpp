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

static DWORD ofsThisContact, ofsDestPrevContact;
static DWORD contactCount;
static DWORD ofsDestThis, ofsNextContact;
static int phase;
static DBContact dbc;

int CDb3Mmap::WorkContactChain(int firstTime)
{
	int first = 0;
	int ret;

	if (firstTime) {
		cb->pfnAddLogMessage(STATUS_MESSAGE, TranslateT("Processing contact chain"));
		ofsDestPrevContact = 0;
		ofsThisContact = m_dbHeader.ofsFirstContact;
		contactCount = 0;
		m_dbHeader.ofsFirstContact = 0;
		phase = 0;
	}

	switch (phase) {
	case 0:
		if (ofsThisContact == 0) {
		LBL_FinishUp:
			if (contactCount != m_dbHeader.contactCount)
				cb->pfnAddLogMessage(STATUS_WARNING, TranslateT("Contact count marked wrongly: correcting"));
			m_dbHeader.contactCount = contactCount;
			return ERROR_NO_MORE_ITEMS;
		}
		if (!SignatureValid(ofsThisContact, DBCONTACT_SIGNATURE)) {
			cb->pfnAddLogMessage(STATUS_ERROR, TranslateT("Contact chain corrupted, further entries ignored"));
			goto LBL_FinishUp;
		}

		if (m_dbHeader.version < DB_095_VERSION) {
			if (ReadSegment(ofsThisContact, &dbc, offsetof(DBContact, dwContactID)) != ERROR_SUCCESS)
				goto LBL_FinishUp;
			dbc.dwContactID = m_dwMaxContactId++;
		}
		else {
			if (ReadSegment(ofsThisContact, &dbc, sizeof(dbc)) != ERROR_SUCCESS)
				goto LBL_FinishUp;
		}

		ofsNextContact = dbc.ofsNext;
		dbc.ofsNext = 0;
		if (!cb->bCheckOnly) {
			if ((ofsDestThis = WriteSegment(WSOFS_END, &dbc, sizeof(dbc))) == WS_ERROR)
				return ERROR_HANDLE_DISK_FULL;
			if (ofsDestPrevContact)
				WriteSegment(ofsDestPrevContact + offsetof(DBContact, ofsNext), &ofsDestThis, sizeof(DWORD));
			else
				m_dbHeader.ofsFirstContact = ofsDestThis;
		}
		else ofsDestThis = ofsThisContact; // needed in event chain worker

		contactCount++;
		phase++; first = 1;

		// fall thru
	case 1:
		ret = WorkSettingsChain(&dbc, first);
		if (ret == ERROR_NO_MORE_ITEMS) {
			phase++; first = 1;
		}
		else if (ret) return ret;
		else break;

		// fall thru
	case 2:
		ret = WorkEventChain(ofsDestThis, &dbc, first);
		if (ret == ERROR_NO_MORE_ITEMS) {
			phase++; first = 1;
		}
		else if (ret) return ret;
		else break;

		// fall thru
	case 3:
		if (WriteSegment(ofsDestThis, &dbc, sizeof(DBContact)) == WS_ERROR)
			return ERROR_HANDLE_DISK_FULL;
		ofsDestPrevContact = ofsDestThis;
		ofsThisContact = ofsNextContact;
		phase = 0;
		break;
	}
	return ERROR_SUCCESS;
}
