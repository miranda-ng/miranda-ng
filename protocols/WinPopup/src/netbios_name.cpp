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

////////////////////////////////////////////////////////////////////////
// Class netbios_name

netbios_name::netbios_name(LPCTSTR n, UCHAR type, bool group, UCHAR lana) :
	m_managed (false),
	m_registered (false),
	m_duplicated (false),
	m_error (false),
	m_lana (lana),
	m_listener (NULL),
	m_dgreceiver (NULL),
	m_term (NULL)
{
	// Преобразование имени в NetBIOS-имя
	int len = NCBNAMSZ - 1;
	CT2A nA( n );
	LPCSTR src = (LPCSTR)nA;
	LPSTR dst = (LPSTR)netbiosed.name;
	for ( ; len && *src; --len, ++dst, ++src )
		*dst = *src;
	for ( ; len; --len )
		*dst++ = ' ';
	*dst = (CHAR)type;
	CharUpperBuffA( (LPSTR)netbiosed.name, NCBNAMSZ - 1 );
	CharToOemBuffA( (LPSTR)netbiosed.name, (LPSTR)netbiosed.name, NCBNAMSZ - 1 );
	netbiosed.name_num = 0;
	netbiosed.name_flags = (UCHAR)( group ? GROUP_NAME : UNIQUE_NAME );

	original = GetANSIName();
}

netbios_name::netbios_name(const NAME_BUFFER& n, UCHAR lana) :
	m_managed (false),
	m_registered (false),
	m_duplicated (false),
	m_error (false),
	m_lana (lana),
	m_listener (NULL),
	m_dgreceiver (NULL),
	m_term (NULL)
{
	CopyMemory (&netbiosed, &n, sizeof (NAME_BUFFER));
	original = GetANSIName();
}

netbios_name& netbios_name::operator=(const netbios_name& n)
{
	_ASSERTE (m_managed == false);
	m_managed = n.m_managed;
	_ASSERTE (m_registered == false);
	m_registered = n.m_registered;
	_ASSERTE (m_duplicated == false);
	m_duplicated = n.m_duplicated;
	_ASSERTE (m_error == false);
	m_error = n.m_error;
	m_lana = n.m_lana;
	_ASSERTE( m_listener == NULL );
	m_listener = NULL;
	_ASSERTE( m_dgreceiver == NULL );
	m_dgreceiver = NULL;
	_ASSERTE( m_term == NULL );
	m_term = NULL;
	CopyMemory (&netbiosed, &n.netbiosed, sizeof( NAME_BUFFER ));
	original = n.original;
	return *this;
}

netbios_name& netbios_name::operator= (const UCHAR* n)
{
	_ASSERTE (m_managed == false);
	m_managed = false;
	_ASSERTE (m_registered == false);
	m_registered = false;
	_ASSERTE (m_duplicated == false);
	m_duplicated = false;
	_ASSERTE (m_error == false);
	m_error = false;
	m_lana = 0;
	_ASSERTE( m_listener == NULL );
	m_listener = NULL;
	_ASSERTE( m_dgreceiver == NULL );
	m_dgreceiver = NULL;
	_ASSERTE( m_term == NULL );
	m_term = NULL;
	CopyMemory (netbiosed.name, n, NCBNAMSZ);
	netbiosed.name_num = 0;
	netbiosed.name_flags = UNIQUE_NAME;
	original = GetANSIName();

	return *this;
}

bool netbios_name::operator== (const NAME_BUFFER& n) const
{
	return ( netbiosed.name [NCBNAMSZ - 1] == n.name [NCBNAMSZ - 1] ) &&
		( ( netbiosed.name_flags & GROUP_NAME ) == ( n.name_flags & GROUP_NAME ) ) &&
		( memcmp( netbiosed.name, n.name, NCBNAMSZ - 1 ) == 0 );
}

bool netbios_name::operator!= (const NAME_BUFFER& n) const
{
	return ! operator==( n );
}

bool netbios_name::operator== (const netbios_name& n) const
{
	return ( m_lana == n.m_lana ) && operator==( n.netbiosed );
}

bool netbios_name::operator!= (const netbios_name& n) const
{
	return ( m_lana != n.m_lana ) || operator!=( n.netbiosed );
}

bool netbios_name::Register()
{
	m_managed = true;

	UCHAR ret = AddName ();
	LOG("Register NetBIOS name \"%s\" on lana %d num=%d : 0x%02x \"%s\"", GetANSIFullName(), m_lana, netbiosed.name_num, ret, GetNetbiosError( ret ) );
	m_registered = (ret == NRC_GOODRET);
	m_duplicated = (ret == NRC_DUPNAME);
	if ( ret != NRC_GOODRET && ret != NRC_DUPNAME )
	{
		WarningBox (NULL, (DWORD)MAKE_HRESULT (0, FACILITY_NETBIOS, ret),
			_T("%s: %s"), TranslateT ("Cannot register NetBIOS name"), (LPCTSTR)CA2T( GetANSIFullName() ) );
	}

	if (!m_term)
		m_term = CreateEvent (NULL, TRUE, FALSE, NULL);
	else
		ResetEvent (m_term);

	if ( m_term && !m_listener )
		m_listener = (HANDLE)mir_forkthread( ListenerThread, this );

	if ( m_term && !m_dgreceiver &&
		// NOTE: Под Win9x нельзя запускать ожидание датаграмм для имён-дубликатов
		// т.к. потом невозможно выбить управление из функции Netbios() даже если
		// разрегистрировать имя
		!m_duplicated )
	{
		m_dgreceiver = (HANDLE)mir_forkthread( DatagramReceiverThread, this );
	}

	return m_registered;
}

void netbios_name::AskForDestroy()
{
	if (m_term)
		SetEvent (m_term);
}

void netbios_name::Destroy()
{
	// Запрос останова рабочего потока-слушателя
	if ( m_term ) SetEvent( m_term );

	// Удаление имени (если не удалить, то слушатель не завершиться)
	UCHAR ret = DeleteName ();
	LOG("Unregister NetBIOS name \"%s\" on lana %d num=%d : 0x%02x \"%s\"", GetANSIFullName(), m_lana, netbiosed.name_num, ret, GetNetbiosError( ret ) );
	m_registered = !(ret == NRC_GOODRET);
	if ( m_duplicated )
	{
		// Восстановление имени
		m_duplicated = false;

		// NOTE: Восстанавливать не надо - проблем больше
		//	uReturn = AddName ();
		//	LOG("Restore NetBIOS name \"%s\" on lana %d : 0x%02x", GetANSIFullName(), m_lana, uReturn);
	}

	// Ожидание, а затем принудительный останов
	if ( m_listener )
	{
		if ( m_term ) SetEvent( m_term );
		if (WaitForSingleObject (m_listener, ALMOST_INFINITE) == WAIT_TIMEOUT)
		{
			LOG("Terminate NetBIOS listener!");
			TerminateThread (m_listener, 0);
		}
		m_listener = NULL;
	}

	if ( m_dgreceiver )
	{
		if ( m_term ) SetEvent( m_term );
		if (WaitForSingleObject (m_dgreceiver, ALMOST_INFINITE) == WAIT_TIMEOUT)
		{
			LOG("Terminate NetBIOS datagram receiver!");
			TerminateThread (m_dgreceiver, 0);
		}
		m_dgreceiver = NULL;
	}

	if ( m_term )
	{
		CloseHandle (m_term);
		m_term = NULL;
	}
}

CStringA netbios_name::GetANSIName() const
{
	CStringA sName;
	LPSTR szName = sName.GetBuffer( NCBNAMSZ );
	CopyMemory( szName, (LPCSTR)netbiosed.name, NCBNAMSZ - 1 );
	szName[ NCBNAMSZ - 1 ] = 0;
	sName.ReleaseBuffer();
	sName.Trim();
	sName.OemToAnsi();
	return sName;
}

CStringA netbios_name::GetANSIFullName() const
{
	CStringA sType;
	sType.Format( " <%02X>", GetType() );
	return original + sType;
}

UCHAR netbios_name::GetType () const
{
	return netbiosed.name [NCBNAMSZ - 1];
}

bool netbios_name::IsGroupName () const
{
	return ((netbiosed.name_flags & GROUP_NAME) == GROUP_NAME);
}

bool netbios_name::IsRegistered () const
{
	return m_registered;
}

bool netbios_name::IsDuplicated () const
{
	return m_duplicated;
}

bool netbios_name::IsError () const
{
	return m_error;
}

bool netbios_name::IsOwnName () const
{
	return m_managed;
}

UCHAR netbios_name::GetLana () const
{
	return m_lana;
}

bool netbios_name::GetRealSender (UCHAR lsn, CStringA& sRealFrom) const
{
	sRealFrom.Empty ();

	SESSION_INFO_BLOCK sibSession = {};
	UCHAR dwInfoRes = pluginNetBIOS.Stat( *this, &sibSession );
	if ( dwInfoRes == NRC_GOODRET )
	{
		for ( int i = 0; i < sibSession.sib_header.num_sess; i++ )
		{
			if ( sibSession.sib_Names [i].lsn == lsn )
			{
				// Наша сессия
				const char* n = (const char*)sibSession.sib_Names [i].remote_name;
				BYTE j = NCBNAMSZ - 2;
				for ( ; j && ( n [ j ] == ' ' ); --j );
				sRealFrom.Append( n, j + 1 );
				sRealFrom.OemToAnsi();
				return true;
			}
		}
	}
	return false;
}

size_t netbios_name::GetLength() const
{
	return (size_t)original.GetLength();
}

UCHAR netbios_name::AddName()
{
	return pluginNetBIOS.AddName( *this );
}

UCHAR netbios_name::DeleteName()
{
	return pluginNetBIOS.DeleteName( *this );
}

typedef struct _ReceiverData
{
	netbios_name* self;
	UCHAR lsn;
} ReceiverData;

void netbios_name::Listener()
{
	m_error = false;
	while ( WaitForSingleObject( m_term, 50 ) == WAIT_TIMEOUT )
	{
		UCHAR lsn = 0;
		UCHAR ret = pluginNetBIOS.Listen( *this, lsn );
		if ( ret != NRC_GOODRET )
		{
			LOG( "Listener : Closing \"%s\"", GetANSIFullName() );
			m_error = true;
			break;
		}

		LOG( "Listener : Got packet for \"%s\"", GetANSIFullName() );
		if ( ReceiverData* data = (ReceiverData*)mir_alloc( sizeof( ReceiverData ) ) )
		{
			data->self = this;
			data->lsn = lsn;
			mir_forkthread( ReceiverThread, data );
		}
	}
}

void netbios_name::ListenerThread(LPVOID param)
{
	if ( netbios_name* pName = (netbios_name*)param )
	{
		pName->Listener();
		pName->m_listener = NULL;
	}
}

void netbios_name::DatagramReceiver()
{
	UCHAR* SMBBlock = (UCHAR*)mir_alloc( 65536 );
	if ( ! SMBBlock )
	{
		m_error = true;
		return;
	}

	m_error = false;
	while ( WaitForSingleObject ( m_term, 50 ) == WAIT_TIMEOUT )
	{
		ZeroMemory( SMBBlock, 65536 );

		WORD iReadedBytes = 65535;
		netbios_name nname_from;
		UCHAR ret = pluginNetBIOS.RecvDatagram( nname_from, *this, SMBBlock, iReadedBytes );
		if ( ret != NRC_GOODRET )
		{
			// Ошибка - выход
			m_error = true;
			break;
		}

		nname_from.m_lana = m_lana;

		LOG( "Got datagram from \"%s\" to \"%s\"", nname_from.GetANSIFullName(), GetANSIFullName() );

		// Свое собственное сообщение?
		if ( IsItMe ( CA2T( nname_from.original ) ) )
		{
			LOG( "DatagramReceiver : Ignoring my datagram" );
			continue;
		}

		// Обработка датаграмм
		if ( iReadedBytes > 2 && *(WORD*)SMBBlock == SM_MAGIC )
		{
			UCHAR iMsgType = SMBBlock[ 2 ];
			switch ( iMsgType )
			{
			case SM_GETSTATUS:
				// Отвечаем своим статусом
				LOG( "DatagramReceiver : It's status request" );
				pluginNetBIOS.SendStatus( *this, nname_from );
				break;

			case SM_SENDSTATUS:
				// Находим контакт и ставим его статус
				if ( iReadedBytes == 2 + 1 + 4 )
				{
					HANDLE hContact = GetContact(  CA2T( nname_from.original ) );
					if ( hContact )
					{
						LOG( "DatagramReceiver : It's status answer" );
						SetContactStatus( hContact, *(__int32*)(SMBBlock + 2 + 1), false );
					}
					else
						LOG( "DatagramReceiver : Unknown contact" );
				}
				else
					LOG( "DatagramReceiver : Invalid format" );
				break;

			case SM_GETAWAYMESSAGE:
				// Отвечаем своим эвей-сообщением
				LOG( "DatagramReceiver : It's away request" );
				pluginNetBIOS.SendAway( *this, nname_from );
				break;

			case SM_SENDAWAYMESSAGE:
				// Находим контакт и ставим его эвей-сообщение
				if ( iReadedBytes >= 2 + 1 + 4 )
				{
					if ( HANDLE hContact = GetContact( CA2T( nname_from.original ) ) )
					{
						LPCSTR szAway = (LPCSTR)( SMBBlock + 2 + 1 + 4 );
						SMBBlock[ iReadedBytes ] = 0;	// ASCII -> ASCIIZ

						LOG( "DatagramReceiver : It's away answer \"%s\"", szAway );
						SetContactAway( hContact, szAway );
					}
					else
						LOG( "DatagramReceiver : Unknown contact" );
				}
				else
					LOG( "DatagramReceiver : Invalid format" );
				break;

			case SM_GETAVATAR:
				// Отвечаем своим аватаром
				LOG( "DatagramReceiver : It's avatar request." );
				pluginNetBIOS.SendAvatar( *this, nname_from );
				break;

			case SM_SENDAVATAR:
				// Находим контакт и ставим его аватар
				if ( iReadedBytes >= 2 + 1 && iReadedBytes < MAX_AVATAR_SIZE + 3 )
				{
					if ( HANDLE hContact = GetContact( CA2T( nname_from.original ) ) )
					{
						LOG( "DatagramReceiver : It's avatar answer" );
						SetContactAvatar( hContact, SMBBlock + 2 + 1, (DWORD)iReadedBytes - 3 );
					}
					else
						LOG( "DatagramReceiver : Unknown contact" );
				}
				else
					LOG( "DatagramReceiver : Invalid format or too big avatar" );
				break;

			default:
				LOG( "DatagramReceiver : Unsupported message type 0x%02x", iMsgType );
			}
		}
		else
			LOG( "DatagramReceiver : Unsupported data 0x%04x", *(WORD*)SMBBlock );
	}

	mir_free( SMBBlock );
}

void netbios_name::DatagramReceiverThread(LPVOID param)
{
	if ( netbios_name* pName = (netbios_name*)param )
	{
		pName->DatagramReceiver();
		pName->m_dgreceiver = NULL;
	}
}

void netbios_name::Receiver(UCHAR lsn)
{
	// Created by Ilja Razinkov (also known as IPv6), 2002, IPv6Intendo@yandex.ru
	// Keep this comment if you redistribute this file

	UCHAR* SMBBlock = (UCHAR*)mir_alloc( 65536 );
	if ( ! SMBBlock )
		return;

	CStringA sTo, sFrom, sMessage;
	UCHAR nRes;
	for (;;)
	{
		ZeroMemory( SMBBlock, 65536 );

		// Получение очередного блока данных
		WORD iReadedBytes = 65535;
		nRes = pluginNetBIOS.Recv (m_lana, lsn, SMBBlock, iReadedBytes);
		if (nRes != NRC_GOODRET)
		{
			LOG( "Receiver : Error while receiving data block" );
			break;
		}

		// Смотрим что к нам пришло - сообщение или что-то неизвестное
		if ( iReadedBytes < 4 || *(DWORD*)SMBBlock != SMB_MAGIC )
		{
			LOG( "Receiver : Unsupported data 0x%08x", *(DWORD*)SMBBlock );
			break;
		}

		UCHAR iMsgType = SMBBlock [4];
		if (iMsgType != SMBsends &&
			iMsgType != SMBsendstrt &&
			iMsgType != SMBsendend &&
			iMsgType != SMBsendtxt)
		{
			LOG( "Receiver : Unsupported message type 0x%02x", iMsgType );
			break;
		}

		// Шлём подтверждение
		UCHAR szReply [SMB_HEADER_SIZE + 5];
		UCHAR* szReplyData =
			pluginNetBIOS.SetSMBHeaderCommand (szReply, iMsgType, sizeof (szReply));
		if (iMsgType == SMBsendstrt)
		{
			// Код сообщения
			static UCHAR rnd = 1;
			szReplyData [0] = 1;		// Номер сообщения
			szReplyData [1] = rnd++;	//
			if ( rnd > 5 )
				rnd = 1;
		}
		nRes = pluginNetBIOS.Send (m_lana, lsn, szReply,
			(WORD)( (iMsgType == SMBsendstrt) ? (SMB_HEADER_SIZE + 5) : (SMB_HEADER_SIZE + 3) ) );
		if ( nRes != NRC_GOODRET )
		{
			// Нефатальная ошибка
			LOG( "Receiver : Error while sending ack" );
		}

		// Действия в зависимости от типа сообщения
		if (iMsgType == SMBsends)
		{
			LOG( "Receiver : Got single-block message" );
			// Короткое сообщение, вытаскиваем данные и выходим...
			sFrom = (const char*) SMBBlock + SMB_HEADER_SIZE + 4;
			int iFromOffset = sFrom.GetLength ();
			sTo = (const char*) SMBBlock + SMB_HEADER_SIZE + 4 + iFromOffset + 2;
			int iToOffset = sTo.GetLength ();
			sMessage = (const char*) SMBBlock + SMB_HEADER_SIZE + 4 + iFromOffset + 2 +
				iToOffset + 4;
			break;
		}
		else if (iMsgType == SMBsendstrt)
		{
			LOG( "Receiver : Got start of multi-block message" );
			// Кусочковое сообщение, начало, смотрим от кого и кому
			sFrom = (const char*) SMBBlock + SMB_HEADER_SIZE + 4;
			int iFromOffset = sFrom.GetLength ();
			sTo = (const char*) SMBBlock + SMB_HEADER_SIZE + iFromOffset + 4 + 2;
		}
		else if (iMsgType == SMBsendtxt)
		{
			// Кусочковое сообщение, блок с данными, конкатенируем...
			int iConcatSize = iReadedBytes - SMB_HEADER_SIZE - 8;
			LOG( "Receiver : Got text (%d-%d bytes) of multi-block message", sMessage.GetLength(), sMessage.GetLength() + iConcatSize - 1 );
			sMessage.Append ((const char*) (SMBBlock + SMB_HEADER_SIZE + 8), iConcatSize);
		}
		else if (iMsgType == SMBsendend)
		{
			LOG( "Receiver : Got end of multi-block message" );
			// Кусочковое сообщение, конец, все получено, выходим
			break;
		}
	}

	sMessage.Replace( "\x14", "\r\n" ); // <14> -> <CR><LF>

	sTo.OemToAnsi();
	sFrom.OemToAnsi();
	sMessage.OemToAnsi();

	// Смотрим информацию о сессии
	CStringA sRealFrom;
	if (GetRealSender (lsn, sRealFrom))
	{
		LOG( "Receiver : Message from \"%s\" (real \"%s\") to \"%s\"", (LPCSTR)sFrom, (LPCSTR)sRealFrom, (LPCSTR)sTo);
		sFrom = sRealFrom;
	}
	else
	{
		LOG( "Receiver : Message from \"%s\" (real sender unknown) to \"%s\"", (LPCSTR)sFrom, (LPCSTR)sTo);
	}

	// Скажем до свиданья...
	pluginNetBIOS.Hangup (m_lana, lsn);

	// Открываем сообщение (пустое игнорируем)
	if ( ! sMessage.IsEmpty() )
	{
		ReceiveContactMessage( CA2T( sFrom ), CA2T( sTo ), CA2T( sMessage ), sMessage.GetLength ());
	}
	else
	{
		LOG( "Receiver : Ignoring empty message" );
	}

	mir_free( SMBBlock );
}

void netbios_name::ReceiverThread(LPVOID param)
{
	if ( ReceiverData* data = (ReceiverData*)param )
	{
		data->self->Receiver( data->lsn );
		mir_free( data );
	}
}
