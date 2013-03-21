/*

WinPopup Protocol plugin for Miranda IM.

Copyright (C) 2004-2010 Nikolay Raspopov

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

#define MAX_MESSAGE_SIZE		424 // Размер пакета данных (байт) посылаемых/принимаемых
									// через мейлслот

mailslot pluginMailslot;			// Мейлслот для приема сообщений

/*const struct {					// Список конфликтующих процессов
	LPCTSTR name;
} blacklist [] = {
	_T("winpopup"),
	_T("vikpopup"),
	_T("netter"),
	_T("realpopup"),
	NULL
};*/

////////////////////////////////////////////////////////////////////////
// Class mailslot

mailslot::mailslot () :
	m_hMailslot (INVALID_HANDLE_VALUE),
	m_MonitorTerm (NULL),
	m_Monitor (NULL)
{
}

mailslot::~mailslot ()
{
	Destroy ();
}

bool mailslot::Create (LPCTSTR Name)
{
	CLock oLock( m_cs );

	m_sName = Name;

	bool ret = true;
	if ( ! IsValid() )
	{		
		// Открытие мейлслота
		CString sAddr;
		sAddr.Format( _T("\\\\.\\mailslot\\%s"), (LPCTSTR)m_sName );

		m_hMailslot = CreateMailslot( sAddr, 0, 2000, NULL );
		if ( ! IsValid() )
		{
			ret = false;
			DWORD err = GetLastError ();
			if (err == ERROR_ALREADY_EXISTS)
				WarningBox (NULL, 0, _T("%s\r\n%s"), T_CREATE_ERROR,
					TranslateT ("Please shutdown any other IM applications and/or Messenger service"));
			else
				WarningBox (NULL, err, T_CREATE_ERROR);
		}
	}
	if (ret)
	{
		if (m_MonitorTerm)
			ResetEvent (m_MonitorTerm);
		else
			m_MonitorTerm = CreateEvent (NULL, TRUE, FALSE, NULL);
		m_Monitor = (HANDLE)mir_forkthread( MonitorThread, this );
	}

	return ret;
}

void mailslot::AskForDestroy()
{
	if (m_MonitorTerm)
		SetEvent (m_MonitorTerm);
}

void mailslot::Destroy ()
{
	CLock oLock( m_cs );
	
	// Запрос останова мониторинга мейлслота
	AskForDestroy();
	
	// Закрытие мейлслота
	if ( IsValid() )
	{
		CloseHandle (m_hMailslot);
		m_hMailslot = INVALID_HANDLE_VALUE;
	}

	// Ожидание останова мониторинга
	if (m_Monitor)
	{
		if (WaitForSingleObject (m_Monitor, ALMOST_INFINITE) == WAIT_TIMEOUT)
		{
			LOG("Terminate mailslot monitor!");
			TerminateThread (m_Monitor, 0);
		}
		m_Monitor = NULL;
	}
	if (m_MonitorTerm)
	{
		CloseHandle (m_MonitorTerm);
		m_MonitorTerm = NULL;
	}
}

bool mailslot::IsValid() const
{
	return ( m_hMailslot != INVALID_HANDLE_VALUE );
}

bool mailslot::SendMailslotMessage(HANDLE hContact, LPCTSTR msg, DWORD& err)
{
	// Получение адресата
	CString sTo = GetNick( hContact );
	if ( sTo.IsEmpty() )
	{
		err = ERROR_BAD_NETPATH;
		return false;
	}

	// Получение своего имени
	CString sFrom = GetNick( NULL );
	if ( sFrom.IsEmpty() )
	{
		err = ERROR_BAD_NETPATH;
		return false;
	}

	// Нет разницы группа это или компьютер
	// bool bGroup = IsGroup( hContact );

	// Сборка пакета сообщения: FROM<00>TO<00>MESSAGE<00>
	COemString sOemMessage = msg;
	COemString sOemTo = (LPCTSTR)sTo;
	COemString sOemFrom = (LPCTSTR)sFrom;

	// Размер заголовка пакета
	int fixed_size = sOemFrom.GetLength() + 1 + sOemTo.GetLength() + 1 + 1;
	if ( fixed_size >= MAX_MESSAGE_SIZE )
	{
		err = ERROR_BAD_LENGTH;
		return false;
	}

	// Создание мейлслота для отправки сообщения
	CString sAddr;
	sAddr.Format( _T("\\\\%s\\mailslot\\%s"), sTo, (LPCTSTR)m_sName );
	HANDLE hFile = CreateFile( sAddr, GENERIC_WRITE, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ); 
	if ( hFile == INVALID_HANDLE_VALUE )
	{
		err = GetLastError();
		return false;
	}

	int max_message_size = MAX_MESSAGE_SIZE - fixed_size;
	char buf[ MAX_MESSAGE_SIZE ] = {};
	lstrcpynA( buf, sOemFrom, sOemFrom.GetLength() + 1 );
	lstrcpynA( buf + sOemFrom.GetLength() + 1, sOemTo, sOemTo.GetLength() + 1 );
	do
	{
		int message_size = ( sOemMessage.GetLength() < max_message_size ) ? 
			sOemMessage.GetLength() : max_message_size;
		lstrcpynA( buf + fixed_size - 1, sOemMessage, message_size + 1 );

		// Отсылка пакета
		DWORD written = 0;
		if ( ! WriteFile( hFile, buf, (DWORD)fixed_size + message_size, &written, NULL ) ||
			( written < (DWORD)fixed_size ) )
		{
			err = GetLastError();
			CloseHandle( hFile );
			return false;
		}
		Sleep( 100 );

		// Укорачивание на отосланный пакет
		sOemMessage.CutFromStart( message_size );
	}
	while ( sOemMessage.GetLength() );

	err = ERROR_SUCCESS;
	CloseHandle( hFile );
	return true;
}

bool mailslot::Receive(unsigned char* buf /* OEM */, DWORD size)
{
	// Разборка сообщения <FROM><00><TO><00><MESSAGE><00> (последний <00> необязателен)
	if (size)
	{
		char* from = (char*) buf;
		char* to = lstrnchr (from, 0, (int)size);
		if (to)
		{
			DWORD from_len = (DWORD)( to - from + 1 );
			if ( from_len < size )
			{
				to++;
				size -= from_len;
				char* msg = lstrnchr (to, 0, (int)size);
				if (msg)
				{
					DWORD to_len = (DWORD)( msg - to + 1 );
					if (to_len < size)
					{
						msg++;
						size -= to_len;
						char* eof = lstrnchr (msg, 0, (int)size);
						DWORD msg_len = eof ? (DWORD)( eof - msg + 1 ) : size;
						if (msg_len == size)
						{
							CAnsiString sFrom (from);
							CAnsiString sTo (to);							
							CAnsiString sMessage (msg);
							ReceiveContactMessage(sFrom, sTo, sMessage, sMessage.GetLength ());
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

void mailslot::MonitorThread(void* param)
{
	if ( mailslot* pMailslot = (mailslot*)param )
	{
		pMailslot->Monitor();	
		pMailslot->m_Monitor = NULL;
	}
}

void mailslot::Monitor ()
{
	// Ожидание прерывания 500 мс при закрытом майлслоте, иначе 50 мс
	while ( WaitForSingleObject( m_MonitorTerm, IsValid() ? 50u : 500u ) == WAIT_TIMEOUT )
	{
		// Проверка входящих сообщений
		LPSTR buf = NULL;
		for ( DWORD buf_size = MAX_MESSAGE_SIZE; IsValid(); buf_size += 1024 )
		{
			if ( WaitForSingleObject( m_MonitorTerm, 0 ) != WAIT_TIMEOUT )
				break;

			if ( buf ) mir_free( buf );
			buf = (LPSTR)mir_alloc( buf_size );

			DWORD readed = 0;
			DWORD err = ReadFile (m_hMailslot, buf, buf_size,
				&readed, NULL) ? ERROR_SUCCESS : GetLastError ();
			if (err == ERROR_ACCESS_DENIED || err == ERROR_SEM_TIMEOUT)
			{
				// Тайм-аут мейлслота
				break;
			}
			else if (err == ERROR_SUCCESS)
			{
				// Данные приняты
				if (readed)
					if (!Receive((LPBYTE)buf, readed))
						LOG("Receive error (bad format?)");
				break;
			}
			else if (err == ERROR_INSUFFICIENT_BUFFER)
			{
				// Нехватка размера буфера
				continue;
			}
			else 
			{
				// Другие ошибки
				// ERROR_HANDLE_EOF - хендл мейлслота закрыт
				LOG("ReadFile form mailslot error: %d", err);
				break;
			}
		}
		if ( buf ) mir_free( buf );
	}
}
