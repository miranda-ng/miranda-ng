/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2011 Nikolay Raspopov

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

contact_scanner	pluginScanner;

contact_scanner::contact_scanner () :
	m_ScannerTerm (NULL),
	m_Scanner (NULL)
{
}

contact_scanner::~contact_scanner ()
{
	Destroy ();
}

bool contact_scanner::Create ()
{
	// Запуск сканера контактов
	if (m_ScannerTerm)
		ResetEvent (m_ScannerTerm);
	else
		m_ScannerTerm = CreateEvent (NULL, TRUE, FALSE, NULL);

	if ( ! m_Scanner )
		m_Scanner = (HANDLE)mir_forkthread( ScannerThread, this );

	return ( m_Scanner != NULL );
}

void contact_scanner::AskForDestroy()
{
	if (m_ScannerTerm)
		SetEvent (m_ScannerTerm);
}

void contact_scanner::Destroy ()
{
	AskForDestroy();

	if (m_Scanner)
	{
		if (WaitForSingleObject (m_Scanner, ALMOST_INFINITE) == WAIT_TIMEOUT)
		{
			LOG("Terminate scanner!");
			TerminateThread (m_Scanner, 0);
		}
		m_Scanner = NULL;
	}

	if (m_ScannerTerm) {
		CloseHandle (m_ScannerTerm);
		m_ScannerTerm = NULL;
	}
}

contact_scanner::operator bool () const
{
	return ( m_Scanner != NULL );
}

// First, Next, ... Next, NULL, First, Next...
HANDLE contact_scanner::GetNextScannableContact()
{
	static HANDLE hContact = NULL;
	if (!hContact)
		hContact = (HANDLE) CallService (MS_DB_CONTACT_FINDFIRST, 0, 0);

	// Циклический перебор контактов
	while( hContact )
	{
		// Проверка на совпадение протокола контакта
		if ( IsMyContact( hContact ) &&
			!db_get_b (hContact, "CList", "NotOnList", 0) &&
			!db_get_b (hContact, "CList", "Hidden", 0) )
		{
			// Вычисление сколько секунд прошло со времени последнего
			// обновления статуса контакта. Нужна проверка?
			DWORD elapsed = GetElapsed (hContact, "LastSeen");
			if ( elapsed >= MIN_PING_INTERVAL )
				break;
		}
		hContact = (HANDLE) CallService (MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	return hContact;
}

int contact_scanner::ScanContact(HANDLE  hContact)
{
	if ( ! pluginInstalled )
		return ID_STATUS_OFFLINE;

#ifdef CHAT_ENABLED
	if ( IsChatRoom( hContact ) )
	{
		CString sSession = GetChatSession( hContact );
		if ( pluginChatEnabled && ! sSession.IsEmpty() )
		{
			CAtlList< CString > lst;
			EnumWorkgroups( lst );
			for ( POSITION pos = lst.GetHeadPosition(); pos; )
			{
				if ( lst.GetNext( pos ) == sSession )
					return ID_STATUS_ONLINE;
			}
		}
		return ID_STATUS_OFFLINE;	
	}
	else
#endif // CHAT_ENABLED
	{
		int status = ID_STATUS_OFFLINE;

		// Получение статуса "Always Online"
		if ( db_get_b( hContact, modname, "AlwaysOnline", FALSE ) )
			status = ID_STATUS_ONLINE;

		// Получение имени контакта
		CString sNick = GetNick( hContact );
		if ( ! sNick.IsEmpty() )
		{
			if ( IsGroup( hContact ) )
			{
				// Имя "ВСЕ" вегда онлайн
				if ( sNick == _T("*") )
					return ID_STATUS_ONLINE;

				// Перечисление групп и сличение с нашей
				CAtlList< CString > lst;
				EnumWorkgroups( lst );
				for ( POSITION pos = lst.GetHeadPosition(); pos; )
				{
					if ( lst.GetNext( pos ).CompareNoCase( sNick ) == 0 )
						return ID_STATUS_ONLINE;
				}
			}
			else if ( IsLegacyOnline( NULL ) || IsLegacyOnline( hContact ) )
			{
				// Синхронный опрос хоста на наличие NetBIOS-имени "Nick <00> U"
				netbios_name nname( sNick, 0x00, false );
				UCHAR foo;
				if ( pluginNetBIOS.FindNameLana( nname, foo ) )
				{
					status = ID_STATUS_ONLINE;

					// Асинхронный опрос контакта "Nick <00> U"
					pluginNetBIOS.AskStatus( nname );
				}
			}
			else
			{
				// Синхронный опрос хоста на наличие NetBIOS-имени "Nick <03> U"
				netbios_name nname( sNick, 0x03, false );
				UCHAR foo;
				if ( pluginNetBIOS.FindNameLana( nname, foo ) )
				{
					status = ID_STATUS_ONLINE;

					// Асинхронный опрос контакта "Nick <03> U"
					pluginNetBIOS.AskStatus( nname );
				}
			}
		}
		return status;
	}
}

void contact_scanner::Scanner ()
{
	while (WaitForSingleObject (m_ScannerTerm, 1000) == WAIT_TIMEOUT)
	{
		// Выборка следующего кандидата на проверку
		if ( HANDLE hContact = GetNextScannableContact() )
		{
			// Проверка контакта
			SetContactStatus( hContact, ScanContact( hContact ), true );
		}
	}
}

void contact_scanner::ScannerThread (LPVOID lpParameter)
{
	if ( contact_scanner* pScanner = (contact_scanner*)lpParameter )
	{
		pScanner->Scanner();
		pScanner->m_Scanner = NULL;
	}
}
