//***************************************************************************************
//
//   Google Extension plugin for the Miranda IM's Jabber protocol
//   Copyright (c) 2011 bems@jabber.org, George Hazan (ghazan@jabber.ru)
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License
//   as published by the Free Software Foundation; either version 2
//   of the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the Free Software
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
//***************************************************************************************

#include "StdAfx.h"
#include "options.h"

char *WtoA(LPCTSTR W)
{
	char* result = (char*)malloc(lstrlen(W) + 1);
	__try {
		int i;
		for (i = 0; W[i]; i++) {
			result[i] = W[i];
		}

		result[i] = 0;
	}
	__except(
		free(result),
		EXCEPTION_CONTINUE_SEARCH
	){}
	return result;
}

LPTSTR ReadJidSetting(LPCSTR name, LPCTSTR jid)
{
	char *ansiJid = WtoA(jid);
	__try {
		DBVARIANT dbv = {0};
		__try {
			if (DBGetContactSettingTString(0, name, ansiJid, &dbv))
			{
				LPTSTR result = (LPTSTR)malloc(2 * sizeof(TCHAR));
				result[0] = '0';
				result[1] = NULL;
				return result;
			}

			return _tcsdup(dbv.ptszVal);
		}
		__finally {
			DBFreeVariant(&dbv);
		}
	}
	__finally {
		free(ansiJid);
	}

	assert(false);
	return NULL; // relax compiler
}

void WriteJidSetting(LPCSTR name, LPCTSTR jid, LPCTSTR setting)
{
	char *ansiJid = WtoA(jid);
	__try {
		DBWriteContactSettingTString(0, name, ansiJid, setting);
	}
	__finally {
		free(ansiJid);
	}
}

void RenewPseudocontactHandles()
{
	int count = 0;
	PROTOCOLDESCRIPTOR **protos;
	CallService(MS_PROTO_ENUMPROTOCOLS, (WPARAM)&count, (LPARAM)&protos);
	for (int i = 0; i < count; i++) {
		DBDeleteContactSetting(0, protos[i]->szName, PSEUDOCONTACT_LINK);
		DBDeleteContactSetting(0, protos[i]->szName, "GMailExtNotifyContact");	// remove this
	}

	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) {
		if (DBGetContactSettingByte(hContact, SHORT_PLUGIN_NAME, PSEUDOCONTACT_FLAG, 0)) {
			LPCSTR proto = (LPCSTR)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
			DBWriteContactSettingDword(0, proto, PSEUDOCONTACT_LINK, (DWORD)hContact);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	};
}