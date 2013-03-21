/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2009 Nikolay Raspopov

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

search pluginSearch;

// Функция простых регулярных выражений
bool MatchPattern (LPCTSTR String, LPCTSTR Pattern);
// Получение NetBIOS-имен хоста и проверка их при помощи простых регулярных выражений
bool MatchPatternNetBIOS (LPCTSTR Host, LPCTSTR Pattern);

search::search()
	: m_count( 0 )
	, m_event( NULL )
{
}

INT_PTR search::StartSearch(LPCTSTR szId)
{
	LOG("Search \"%s\"", szId);

	if (m_event)
		// Запрос на останов предыдущего поиска
		SetEvent (m_event);
	else
		m_event = CreateEvent (NULL, TRUE, FALSE, NULL);

	// Начало нового поиска
	if ( BasicSearchData* data = new BasicSearchData )
	{
		data->me = this;
		HANDLE cookie = data->cookie = GenerateCookie();
		data->id = szId;
		data->id.MakeUpper();
		data->root = NULL;
		mir_forkthread( BasicSearchThread, data );
		return (INT_PTR)cookie;
	}

	return 0;
}

void search::AskForDestroy()
{
	if ( m_event )
		SetEvent( m_event );
}

void search::Destroy()
{
	if ( m_event )
	{
		while( m_count )
		{
			SetEvent( m_event );
			Sleep( 250 );
		}
		CloseHandle( m_event );
		m_event = NULL;
	}
}

bool MatchPattern (LPCTSTR String, LPCTSTR Pattern)
{
	TCHAR c, p, l;
	for (;;)
	{
		switch ( p = *Pattern++ )
		{
		case 0:
			// end of pattern
			return *String ? false : true;	// if end of string TRUE

		case _T('*'):
			// match zero or more char
			while (*String)
				if (MatchPattern (String++, Pattern))
					return true;
			return MatchPattern (String, Pattern);

		case _T('?'):
			// match any one char
			if (*String++ == 0)
				return false;	// not end of string
			break;

		case _T('['):
			// match char set
			if ((c = *String++) == 0)
				return false; // syntax
			l = 0;
			if (*Pattern == _T('!'))
			{ // match a char if NOT in set []
				++Pattern;
				while ((p = *Pattern++)!= _T('\0'))
				{
					if (p == _T(']')) // if end of char set, then
						break; // no match found
					if (p == _T('-'))
					{ // check a range of chars?
						p = *Pattern;
						// get high limit of range
						if (p == 0 || p == _T(']'))
							return false; // syntax
						if (c >= l && c <= p)
							return false; // if in range
					}
					l = p;
					// if char matches this element
					if (c == p) 
						return false;
				}
			}
			else
			{ // match if char is in set []
				while ((p = *Pattern++) != _T('\0'))
				{
					if (p == _T(']')) // if end of char set, then
						return false; // no match found
					if (p == _T('-')) { // check a range of chars?
						p = *Pattern;
						// get high limit of range
						if (p == 0 || p == _T(']'))
							return false; // syntax
						if (c >= l && c <= p)
							break; // if in range, move on
					}
					l = p;
					// if char matches this element
					if (c == p)
						break; // move on
				}
				while (p && p != _T(']')) // got a match in char set
					p = *Pattern++; // skip to end of set
			}
			break;

		case _T('#'):
			c = *String++;
			if (c < _T('0') || c > _T('9'))
				return false; // not a digit
			break;

		default:
			// check for exact char
			c = *String++;
			if (c != p)
				return false; // not a match
			break;
		}
	}
}

bool MatchPatternNetBIOS (LPCTSTR Host, LPCTSTR Pattern)
{
	netbios_name_list names;
	if ( pluginNetBIOS.GetNames( names, Host, false ) )
	{
		POSITION pos = names.GetHeadPosition ();
		CString n;
		while ( pos )
		{
			netbios_name& name = names.GetNext (pos);
			if ( name.GetType() == 3 )
			{
				CA2T sName( name.original );
				if ( MatchPattern( (LPCTSTR)sName, Pattern ) )
					return true;
			}
		}
	}
	return false;
}

void search::BasicSearchJob(const BasicSearchData* data)
{
	if (WaitForSingleObject (m_event, 0) != WAIT_TIMEOUT)
		return;

	HANDLE hEnum = NULL;
	DWORD res = WNetOpenEnum (RESOURCE_GLOBALNET, RESOURCETYPE_ANY,
		RESOURCEUSAGE_CONTAINER, data->root, &hEnum);
	if (res == NO_ERROR)
	{
		for (;;)
		{
			if (WaitForSingleObject (m_event, 0) != WAIT_TIMEOUT)
				return;

			DWORD cCount = 1;
			DWORD BufferSize = 4096;
			char* Buffer = (char*)mir_alloc( BufferSize );
			if ( ! Buffer )
				break;
			res = WNetEnumResource( hEnum, &cCount, Buffer, &BufferSize );
			if ( res == NO_ERROR )
			{
				if (WaitForSingleObject (m_event, 0) != WAIT_TIMEOUT)
					return;

				LPNETRESOURCE lpnr = (LPNETRESOURCE)Buffer;
				if ( lpnr->dwDisplayType == RESOURCEDISPLAYTYPE_SERVER )
				{
					// Сверка названия контакта
					// имя компьютера, комментарий, имя рабочей группы
					CharUpper (lpnr->lpRemoteName);
					if (MatchPattern (lpnr->lpRemoteName + 2, data->id) ||
						(lpnr->lpComment && MatchPattern (lpnr->lpComment, data->id)) ||
						(data->root && MatchPattern (data->root->lpRemoteName, data->id)) ||
						MatchPatternNetBIOS (lpnr->lpRemoteName + 2, data->id))
					{
						// Добавление контакта
						PROTOSEARCHRESULT psr = {};
						psr.cbSize = sizeof( PROTOSEARCHRESULT );
						psr.nick = lpnr->lpRemoteName + 2;
						psr.firstName = lpnr->lpComment;
						psr.lastName = data->root ? data->root->lpRemoteName : _T("");
						psr.email = _T("");
						ProtoBroadcastAck (modname, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA,
							data->cookie, (LPARAM) &psr);
					}
				}
				else
				{
					if ( ( lpnr->dwUsage & 0xffff ) == RESOURCEUSAGE_CONTAINER )
					{
						if ( BasicSearchData* data1 = new BasicSearchData )
						{
							data1->me = data->me;
							data1->cookie = data->cookie;
							data1->id = data->id;
							data1->root = lpnr;
							Buffer = NULL;
							res = (DWORD)InterlockedIncrement (&m_count);
							mir_forkthread( BasicSearchThread, data1 );
						}
					}
				}
				mir_free( Buffer );
			}
			else
			{
				mir_free( Buffer );
				break;
			}
		}
		WNetCloseEnum (hEnum);
	}
}

void search::BasicSearchThread(LPVOID param)
{
	if ( BasicSearchData* data = (BasicSearchData*)param )
	{
		data->me->BasicSearch( data );
		if ( data->root ) mir_free( data->root );
		delete data;
	}
}

void search::BasicSearch(const BasicSearchData* data)
{
	// Повторный запуск?
	if ( data->root == NULL )
	{
		while( m_count )
		{
			if ( ! pluginInstalled )
				return;

			// Уже есть поиск в процессе, ожидание останова
			Sleep (100);
		}
		InterlockedIncrement( &m_count );
		ResetEvent( m_event );
	}

	// С плагином всё в порядке?
	if ( pluginInstalled )
	{
		BasicSearchJob( data );

		LONG res = InterlockedDecrement( &m_count );
		_ASSERTE( res >= 0 );
		if ( res == 0 )
		{
			// Поиск завершён штатно?
			if ( WaitForSingleObject( m_event, 0 ) != WAIT_OBJECT_0 )
			{
				ProtoBroadcastAck (modname, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, data->cookie, 0);
			}
			else
			{
				LOG("Search aborted by another search");
			}
		}
	}
}
