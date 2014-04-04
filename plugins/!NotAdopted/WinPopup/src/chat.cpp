/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2008-2010 Nikolay Raspopov

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

#include "stdafx.h"

#ifdef CHAT_ENABLED

HANDLE	plugin_CHAT_EVENT			= NULL;

static int __cdecl CHAT_EVENT(WPARAM /* wParam */, LPARAM lParam)
{
	GCHOOK* pgch = (GCHOOK*)lParam;
	switch ( pgch->pDest->iType )
	{
	case GC_USER_MESSAGE:
		{
			// Дублирование в чат
			ChatMessage( pgch->pDest->ptszID, pgch->ptszText );

			// Отправка сообщения
			DWORD dwLastError = 0;
			SendMessage( pgch->pDest->ptszID, pgch->ptszText, dwLastError );
		}
		break;
	}

	return 0;
}

bool ChatRegister()
{
	GCREGISTER gcr =
	{
		sizeof( GCREGISTER ),
		0,
		modname,
		modname,
		0,
		0,
		NULL
	};
	int result = CallServiceSync( MS_GC_REGISTER, 0, (LPARAM)&gcr );
	if ( result != 0 )
		return false;

	_ASSERT (plugin_CHAT_EVENT == NULL);
	plugin_CHAT_EVENT = HookEvent (ME_GC_EVENT, CHAT_EVENT);
	_ASSERT (plugin_CHAT_EVENT != NULL);

	return true;
}

void ChatUnregister()
{
	if (plugin_CHAT_EVENT)
	{
		UnhookEvent (plugin_CHAT_EVENT);
		plugin_CHAT_EVENT = NULL;
	}
}

bool ChatNewSession(LPCTSTR szSession)
{
	GCSESSION gcr =
	{
		sizeof( GCSESSION ),
		GCW_CHATROOM,
		modname,
		(LPCSTR)szSession,
		(LPCSTR)szSession,
		NULL,
		0,
		0
	};
	return ( CallServiceSync( MS_GC_NEWSESSION, 0, (LPARAM)&gcr ) == 0 );
}

bool ChatAddGroup(LPCTSTR szSession, LPCTSTR szGroup)
{
	GCDEST gcdest =
	{
		modname,
		(LPSTR)szSession,
		GC_EVENT_ADDGROUP
	};
	GCEVENT gce =
	{
		sizeof( GCEVENT ),
		&gcdest,
		NULL,
		NULL,
		NULL,
		(LPCSTR)szGroup,
		NULL,
		FALSE,
		0,
		0,
		NULL
	};
	return ( CallServiceSync( MS_GC_EVENT, 0, (LPARAM)&gce ) == 0 );
}

bool ChatJoinMe(LPCTSTR szSession, LPCTSTR szGroup)
{
	CString sMe;

	CString sMyNick = GetNick( NULL );
	if ( ! sMyNick.IsEmpty() )
		sMe = sMyNick;
	else
		sMe = pluginMachineName;

	GCDEST gcdest =
	{
		modname,
		(LPSTR)szSession,
		GC_EVENT_JOIN
	};
	GCEVENT gce =
	{
		sizeof( GCEVENT ),
		&gcdest,
		NULL,
		(LPCSTR)(LPCTSTR)sMe,
		(LPCSTR)(LPCTSTR)sMe,
		(LPCSTR)szGroup,
		NULL,
		TRUE,
		0,
		0,
		NULL
	};
	return ( CallServiceSync( MS_GC_EVENT, 0, (LPARAM)&gce ) == 0 );
}

bool ChatJoinUser(LPCTSTR szSession, LPCTSTR szUser, LPCTSTR szGroup)
{
	GCDEST gcdest =
	{
		modname,
		(LPSTR)szSession,
		GC_EVENT_JOIN
	};
	GCEVENT gce =
	{
		sizeof( GCEVENT ),
		&gcdest,
		NULL,
		(LPCSTR)szUser,
		(LPCSTR)szUser,
		(LPCSTR)szGroup,
		NULL,
		FALSE,
		0,
		0,
		NULL
	};
	return ( CallServiceSync( MS_GC_EVENT, 0, (LPARAM)&gce ) == 0 );
}

bool ChatInitDone(LPCTSTR szSession)
{
	GCDEST gcdest =
	{
		modname,
		(LPSTR)szSession,
		GC_EVENT_CONTROL
	};
	GCEVENT gce =
	{
		sizeof( GCEVENT ),
		&gcdest,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		FALSE,
		0,
		0,
		NULL
	};
	return ( CallServiceSync( MS_GC_EVENT, SESSION_INITDONE, (LPARAM)&gce ) == 0 );
}

bool ChatOnline(LPCTSTR szSession)
{
	GCDEST gcdest =
	{
		modname,
		(LPSTR)szSession,
		GC_EVENT_CONTROL
	};
	GCEVENT gce =
	{
		sizeof( GCEVENT ),
		&gcdest,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		FALSE,
		0,
		0,
		NULL
	};
	return ( CallServiceSync( MS_GC_EVENT, SESSION_ONLINE, (LPARAM)&gce ) == 0 );
}

bool ChatOffline(LPCTSTR szSession)
{
	GCDEST gcdest =
	{
		modname,
		(LPSTR)szSession,
		GC_EVENT_CONTROL
	};
	GCEVENT gce =
	{
		sizeof( GCEVENT ),
		&gcdest,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		FALSE,
		0,
		0,
		NULL
	};
	return ( CallServiceSync( MS_GC_EVENT, SESSION_OFFLINE, (LPARAM)&gce ) == 0 );
}

bool ChatMessage(LPCTSTR szSession, LPCTSTR szFrom, LPCTSTR szMessage)
{
	GCDEST gcdest =
	{
		modname,
		(LPSTR)szSession,
		GC_EVENT_MESSAGE
	};
	GCEVENT gce =
	{
		sizeof( GCEVENT ),
		&gcdest,
		(LPCSTR)szMessage,
		(LPCSTR)szFrom,
		(LPCSTR)szFrom,
		NULL,
		NULL,
		FALSE,
		0,
		0,
		time()
	};
	return ( CallServiceSync( MS_GC_EVENT, 0, (LPARAM)&gce ) == 0 );
}

bool ChatMessage(LPCTSTR szSession, LPCTSTR szMessage)
{
	CString sMe;

	CString sMyNick = GetNick( NULL );
	if ( ! sMyNick.IsEmpty() )
		sMe = sMyNick;
	else
		sMe = pluginMachineName;

	GCDEST gcdest =
	{
		modname,
		(LPSTR)szSession,
		GC_EVENT_MESSAGE
	};
	GCEVENT gce =
	{
		sizeof( GCEVENT ),
		&gcdest,
		(LPCSTR)szMessage,
		(LPCSTR)(LPCTSTR)sMe,
		(LPCSTR)(LPCTSTR)sMe,
		NULL,
		NULL,
		TRUE,
		0,
		0,
		time()
	};
	return ( CallServiceSync( MS_GC_EVENT, 0, (LPARAM)&gce ) == 0 );
}

CString GetChatSession(HANDLE hContact)
{
	CString sContact;
	DBVARIANT dbv = {};
	if ( ! db_get_ts( hContact, modname, "ChatRoomID", &dbv ) )
	{
		sContact = dbv.pszVal;
		db_free( &dbv );
	}
	return sContact;
}

bool IsChatRoom(HANDLE hContact)
{
	return ( db_get_b( hContact, modname, "ChatRoom", 0 ) != 0 );
}

#endif // CHAT_ENABLED
