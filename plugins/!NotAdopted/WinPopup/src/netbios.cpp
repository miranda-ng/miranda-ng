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

netbios pluginNetBIOS;	// Прием/отправка сообщений через NetBIOS

// Макрос печати NetBIOS-имени
#define NETBIOS_NAME_TRACE(bb,mm,nn) mir_snprintf((bb),(mm),"%s #%d %02x %c %s", \
	(nn.GetANSIFullName()), (nn.name_num), (nn.name_flags), \
	(((nn.name_flags & GROUP_NAME) == GROUP_NAME) ? 'G' : 'U'), \
	(((nn.name_flags & (NAME_FLAGS_MASK & ~GROUP_NAME)) == REGISTERING) ? "REGISTERING" : \
	(((nn.name_flags & (NAME_FLAGS_MASK & ~GROUP_NAME)) == REGISTERED) ? "REGISTERED" : \
	(((nn.name_flags & (NAME_FLAGS_MASK & ~GROUP_NAME)) == DEREGISTERED) ? "DEREGISTERED" : \
	(((nn.name_flags & (NAME_FLAGS_MASK & ~GROUP_NAME)) == DUPLICATE) ? "DUPLICATE" : \
	(((nn.name_flags & (NAME_FLAGS_MASK & ~GROUP_NAME)) == DUPLICATE_DEREG) ? "DUPLICATE_DEREG" : \
	"UNKNOWN"))))));

// NetBIOS commands
static const struct
{
	UCHAR	code;		// NetBIOS command code
	LPCSTR	name;		// NetBIOS command description
}
NCBCommands[] = 
{
	{ NCBCALL,			"CALL" },
	{ NCBLISTEN,		"LISTEN" },
	{ NCBHANGUP,		"HANG UP" },
	{ NCBSEND,			"SEND" },
	{ NCBRECV,			"RECEIVE" },
	{ NCBRECVANY,		"RECEIVE ANY" },
	{ NCBCHAINSEND,		"CHAIN SEND" },
	{ NCBDGSEND,		"SEND DATAGRAM" },
	{ NCBDGRECV,		"RECEIVE DATAGRAM" },
	{ NCBDGSENDBC,		"SEND BROADCAST DATAGRAM" },
	{ NCBDGRECVBC,		"RECEIVE BROADCAST DATAGRAM" },
	{ NCBADDNAME,		"ADD NAME" },
	{ NCBDELNAME,		"DELETE NAME" },
	{ NCBRESET,			"RESET" },
	{ NCBASTAT,			"ADAPTER STATUS" },
	{ NCBSSTAT,			"SESSION STATUS" },
	{ NCBCANCEL,		"CANCEL" },
	{ NCBADDGRNAME,		"ADD GROUP NAME" },
	{ NCBENUM,			"ENUMERATE LANA NUMBERS" },
	{ NCBUNLINK,		"UNLINK" },
	{ NCBSENDNA,		"SEND NO ACK" },
	{ NCBCHAINSENDNA,	"CHAIN SEND NO ACK" },
	{ NCBLANSTALERT,	"LAN STATUS ALERT" },
	{ NCBACTION,		"ACTION" },
	{ NCBFINDNAME,		"FIND NAME" },
	{ NCBTRACE,			"TRACE" },
	{ 0,				NULL }
};

// NetBIOS errors
static const struct
{
	UCHAR	error;		// NetBIOS error code
	LPCSTR	message;	// NetBIOS error message
}
NRCErrors [] =
{
	{ NRC_GOODRET,		"The operation succeeded." },
	{ NRC_BUFLEN,		"An illegal buffer length was supplied." },
	{ NRC_ILLCMD,		"An illegal command was supplied." },
	{ NRC_CMDTMO,		"The command was timed out." },
	{ NRC_INCOMP,		"The message was incomplete. The application is to issue another command." },
	{ NRC_BADDR,		"The buffer address was illegal." },
	{ NRC_SNUMOUT,		"The session number was out of range." },
	{ NRC_NORES,		"No resource was available." },
	{ NRC_SCLOSED,		"The session was closed." },
	{ NRC_CMDCAN,		"The command was canceled." },
	{ NRC_DUPNAME,		"A duplicate name existed in the local name table." },
	{ NRC_NAMTFUL,		"The name table was full." },
	{ NRC_ACTSES,		"The command finished; the name has active sessions and is no longer registered." },
	{ NRC_LOCTFUL,		"The local session table was full." },
	{ NRC_REMTFUL,		"The remote session table was full. The request to open a session was rejected." },
	{ NRC_ILLNN,		"An illegal name number was specified." },
	{ NRC_NOCALL,		"The system did not find the name that was called." },
	{ NRC_NOWILD,		"Wildcards are not permitted in the ncb_name member." },
	{ NRC_INUSE,		"The name was already in use on the remote adapter." },
	{ NRC_NAMERR,		"The name was deleted." },
	{ NRC_SABORT,		"The session ended abnormally." },
	{ NRC_NAMCONF,		"A name conflict was detected." },
	{ NRC_IFBUSY,		"The interface was busy." },
	{ NRC_TOOMANY,		"Too many commands were outstanding; the application can retry the command later." },
	{ NRC_BRIDGE,		"The ncb_lana_num member did not specify a valid network number." },
	{ NRC_CANOCCR,		"The command finished while a cancel operation was occurring." },
	{ NRC_CANCEL,		"The NCBCANCEL command was not valid; the command was not canceled." },
	{ NRC_DUPENV,		"The name was defined by another local process." },
	{ NRC_ENVNOTDEF,	"The environment was not defined." },
	{ NRC_OSRESNOTAV,	"Operating system resources were exhausted. The application can retry the command later." },
	{ NRC_MAXAPPS,		"The maximum number of applications was exceeded." },
	{ NRC_NOSAPS,		"No service access points (SAPs) were available for NetBIOS." },
	{ NRC_NORESOURCES,	"The requested resources were not available." },
	{ NRC_INVADDRESS,	"The NCB address was not valid." },
	{ NRC_INVDDID,		"The NCB DDID was invalid." },
	{ NRC_LOCKFAIL,		"The attempt to lock the user area failed." },
	{ NRC_OPENERR,		"An error occurred during an open operation being performed by the device driver." },
	{ NRC_SYSTEM,		"A system error occurred." },
	{ NRC_PENDING,		"An asynchronous operation is not yet finished." },
	{ 0,				NULL }
};

LPCSTR GetNetbiosCommand(UCHAR command)
{
	command &= 0x7f; // strip ASYNCH bit
	for ( int i = 0; NCBCommands[ i ].name; ++i )
		if ( NCBCommands[ i ].code == command )
			return NCBCommands[ i ].name;
	return "UNKNOWN";
}

LPCSTR GetNetbiosError(UCHAR err)
{
	for ( int i = 0; NRCErrors [ i ].message; ++i )
		if ( NRCErrors [ i ].error == err )
			return NRCErrors [ i ].message;
	return "Unknown error.";
}

UCHAR NetbiosEx(NCB* pNCB)
{
	UCHAR command = pNCB->ncb_command;
	UCHAR ret = Netbios( pNCB );
	//if ( ret != NRC_GOODRET )
	if ( ret == pNCB->ncb_retcode )
	{
		LOG( "NetBIOS call 0x%02x \"%s\" result: 0x%02x \"%s\"",
			command, GetNetbiosCommand( command ),
			ret, GetNetbiosError( ret ) );
	}
	else
	{
		LOG( "NetBIOS call 0x%02x \"%s\" result: 0x%02x \"%s\", return: 0x%02x \"%s\"",
			command, GetNetbiosCommand( command ),
			ret, GetNetbiosError( ret ),
			pNCB->ncb_retcode, GetNetbiosError( pNCB->ncb_retcode ) );
	}
	return ret;
}

// Определение NCB не на стеке
// KB317437: "NetBIOS Listen May Return a Damaged NCB Structure"
// The _NCB structure that is returned when a NetBIOS Listen call completes may have a changed server name offset. 
class CNCB
{
public:
	CNCB() :
		m_buf( VirtualAlloc( NULL, 4096, MEM_COMMIT, PAGE_READWRITE ) )
	{
	}

	~CNCB()
	{
		if ( m_buf )
		{
			VirtualFree( m_buf, 0, MEM_RELEASE );
		}
	}

	inline operator bool() const
	{
		return ( m_buf != NULL );
	}

	inline operator NCB*() const
	{
		return (NCB*)m_buf;
	}

	inline NCB* operator->() const
	{
		return (NCB*)m_buf;
	}

protected:
	void* m_buf;
};

////////////////////////////////////////////////////////////////////////
// Class netbios

netbios::netbios () :
	m_initialized( false )
{
	ZeroMemory (&m_le, sizeof (m_le));
}

netbios::~netbios ()
{
	Destroy ();
}

bool netbios::Create (BOOL registration)
{
	CLock oLock( m_csData );

	if ( ! m_initialized )
	{
		// Перечисление адаптеров
		if ( EnumLanas( m_le ) == NRC_GOODRET )
		{		
			// Сброс адаптеров
			for ( UCHAR i = 0; i < m_le.length; i++ )
			{
				ResetLana( m_le.lana [i] );
			}

			// Регистрация имён, если нужна
			if ( registration )
			{
				Register ();
			}
		}

		m_initialized = true;
	}

	return m_initialized;
}

void netbios::AskForDestroy()
{
	CLock oLock( m_csData );

	// Запрос на дерегистрацию имён
	for ( size_t i = 0; i < m_names.GetCount (); ++i )
		m_names [i]->AskForDestroy();
}

void netbios::Destroy ()
{
	CLock oLock( m_csData );

	if ( m_initialized )
	{
		Deregister();
		m_initialized = false;
	}
}

unsigned char* netbios::SetSMBHeaderCommand (unsigned char* szHeader, BYTE iCommandCode, size_t iBufferLen)
{
	ZeroMemory (szHeader, iBufferLen);
	*(DWORD*)szHeader = SMB_MAGIC;
	szHeader [4] = iCommandCode;
	return (szHeader + SMB_HEADER_SIZE);
}

netbios::operator bool() const
{
	return m_initialized;
}

bool netbios::SendNetBIOSMessage(HANDLE hContact, LPCTSTR msg, DWORD& err)
{
	// Created by Ilja Razinkov (also known as IPv6), 2002, IPv6Intendo@yandex.ru
	// Keep this comment if you redistribute this file

	//Схема отправки большого NB-сообщения:
	//1) Шлется D5. Текст сообщения:
	//- 00, длина остатка A (1 байт), 00 (3 байта)
	//- остаток A. 04, строка КТО, 00, 04, строка КОМУ, 00
	//в ответ приходит КОД СООБЩЕНИЯ (2 байта) и 00,00,00 (всего 5 байт), с D5 в заголовке
	//
	//2) Шлется D7. Текст сообщения:
	//- КОД СООБЩЕНИЯ (2 байта), 00, длина остатка (A+B) (1 байт), 00 (всего 5 байт)
	//- остаток A. 01, длина остатка B, 00 (3 байта)
	//- остаток B. очередной кусок посланного СООБЩЕНИЯ
	//в ответ приходит пустое сообщение (с 3мя 0-лями), с D7 в заголовке
	//
	//3) Пункт 2 повторяется пока не будет отослано все СООБЩЕНИЕ
	//
	//4) Шлется D6.Текст сообщения:
	//- КОД СООБЩЕНИЯ (2 байта), 00, 00, 00 (всего 5 байт)
	//в ответ приходит пустое сообщение (с 3мя 0-лями), с D6 в заголовке

	// Получение адресата
	CString sTo = GetNick( hContact );
	if ( sTo.IsEmpty() )
	{
		err = (DWORD)MAKE_HRESULT( 0, FACILITY_NETBIOS, NRC_NOCALL );
		return false;
	}

	// Получение своего имени
	CString sFrom = GetNick( NULL );
	if ( sFrom.IsEmpty() )
	{
		err = (DWORD)MAKE_HRESULT( 0, FACILITY_NETBIOS, NRC_NOCALL );
		return false;
	}

	bool bGroup = IsGroup( hContact );

	// Подготовка данных
	CStringA sMessage( msg );
	sMessage.AnsiToOem();
	sMessage.Replace( "\r\n", "\x14" );	// <CR><LF> -> <14>
	sMessage.Replace( "\r", "\x14" );	// <CR> -> <14>
	sMessage.Replace( "\n", "\x14" );	// <LF> -> <14>
	netbios_name nname_To( sTo, 3, bGroup );
	netbios_name nname_From( sFrom, 3 );

	// Поиск адаптера
	UCHAR lana = 0;
	if ( ! FindNameLana( nname_To, lana ) )
	{
		LOG ("SendNetBIOSMessage : Unknown name");
		err = (DWORD)MAKE_HRESULT( 0, FACILITY_NETBIOS, NRC_NOCALL );
		return false;
	}

	// Коннект к клиенту
	UCHAR lsn = 0;
	err = Call (lana, nname_From, nname_To, lsn);
	if (err != NRC_GOODRET)
	{
		LOG ("SendNetBIOSMessage : Cannot connect" );
		err = (DWORD)MAKE_HRESULT( 0, FACILITY_NETBIOS, err );
		return false;
	}

	UCHAR SMBBlock [168] = { 0 };
	UCHAR* szSMBData = NULL;
	UCHAR iFromLen = (UCHAR)nname_From.GetLength ();
	UCHAR iToLen = (UCHAR)nname_To.GetLength ();
	UCHAR iHiMsgCode = 0, iLoMsgCode = 0;

	// 1. Шлем заголовок
	LOG ( "SendNetBIOSMessage : Send start of multi-block message" );
	szSMBData = SetSMBHeaderCommand (SMBBlock, SMBsendstrt, sizeof (SMBBlock));
	UCHAR dwD5ALength = (UCHAR)( 1 + iFromLen + 2 + iToLen + 1 );
	szSMBData[1] = dwD5ALength;
	szSMBData[3] = 4;
	szSMBData[4+iFromLen+1] = 4;
	CopyMemory ( szSMBData + 4, nname_From.netbiosed.name, iFromLen );
	CopyMemory ( szSMBData + 4 + iFromLen + 2, nname_To.netbiosed.name, iToLen );
	UCHAR dwD5Length = (UCHAR)( 3 + dwD5ALength );
	err = Send (lana, lsn, SMBBlock, (WORD) (SMB_HEADER_SIZE + dwD5Length));
	if (err != NRC_GOODRET)
	{
		LOG ( "SendNetBIOSMessage : Can`t start session" );
		Hangup (lana, lsn);
		err = (DWORD)MAKE_HRESULT( 0, FACILITY_NETBIOS, err );
		return false;
	}

	// Ждем подтверждения
	WORD length = sizeof (SMBBlock);
	err = Recv (lana, lsn, SMBBlock, length);
	if (err != NRC_GOODRET)
	{
		LOG ( "SendNetBIOSMessage : No reply (start session)" );
		Hangup (lana, lsn);
		err = (DWORD)MAKE_HRESULT( 0, FACILITY_NETBIOS, err );
		return false;
	}
	iHiMsgCode=SMBBlock[SMB_HEADER_SIZE];
	iLoMsgCode=SMBBlock[SMB_HEADER_SIZE+1];

	// 2. Шлем сообщение (кусочками)
	UCHAR dwD7BLength = 0;
	for (int iSendedBytes = 0; iSendedBytes < sMessage.GetLength (); iSendedBytes += dwD7BLength)
	{
		dwD7BLength = sizeof (SMBBlock) - (5 + 3 + SMB_HEADER_SIZE);
		szSMBData = SetSMBHeaderCommand (SMBBlock, SMBsendtxt, sizeof (SMBBlock));
		if (iSendedBytes + dwD7BLength > sMessage.GetLength ())
			dwD7BLength = (UCHAR)( sMessage.GetLength () - iSendedBytes );
		szSMBData[0]=iHiMsgCode;
		szSMBData[1]=iLoMsgCode;
		szSMBData[3]=(UCHAR)( dwD7BLength + 3 );
		szSMBData[5]=1;
		szSMBData[6]=dwD7BLength;
		CopyMemory (szSMBData + 5 + 3, (LPCSTR) sMessage + iSendedBytes, dwD7BLength);
		LOG( "SendNetBIOSMessage : Send text (%u-%u bytes) of multi-block message" , iSendedBytes, iSendedBytes + dwD7BLength - 1);
		err = Send (lana, lsn, SMBBlock, (WORD) (SMB_HEADER_SIZE + 5 + 3 + dwD7BLength));
		if (err != NRC_GOODRET)
		{
			LOG ( "SendNetBIOSMessage : Can`t use session" );
			Hangup (lana, lsn);
			err = (DWORD)MAKE_HRESULT( 0, FACILITY_NETBIOS, err );
			return false;
		}
		// Ждем подтверждения
		length = sizeof (SMBBlock);
		err = Recv (lana, lsn, SMBBlock, length);
		if (err != NRC_GOODRET)
		{
			LOG ( "SendNetBIOSMessage : No reply (use session)" );
			Hangup (lana, lsn);
			err = (DWORD)MAKE_HRESULT( 0, FACILITY_NETBIOS, err );
			return false;
		}
	}

	// 3. Шлем извещение что все отослано
	LOG ( "SendNetBIOSMessage : Send and of multi-block message" );
	szSMBData = SetSMBHeaderCommand (SMBBlock, SMBsendend, sizeof (SMBBlock));
	DWORD dwD6Length=5;
	szSMBData[0]=iHiMsgCode;
	szSMBData[1]=iLoMsgCode;
	err = Send (lana, lsn, SMBBlock, (WORD) (SMB_HEADER_SIZE + dwD6Length));
	if (err != NRC_GOODRET)
	{
		LOG ( "SendNetBIOSMessage : Can`t close session" );
		Hangup (lana, lsn);
		err = (DWORD)MAKE_HRESULT( 0, FACILITY_NETBIOS, err );
		return false;
	}

	// Ждем подтверждения
	length = sizeof (SMBBlock);
	err = Recv (lana, lsn, SMBBlock, length);
	if (err != NRC_GOODRET)
	{
		LOG ( "SendNetBIOSMessage : No reply (close session)" );
		Hangup (lana, lsn);
		err = (DWORD)MAKE_HRESULT( 0, FACILITY_NETBIOS, err );
		return false;
	}

	// Закрываем сессию
	Hangup (lana, lsn);
	err = (DWORD)MAKE_HRESULT( 1, FACILITY_NETBIOS, NRC_GOODRET );
	return true;
}

ip netbios::FindNameIP (LPCTSTR szName, UCHAR type)
{
	// Поиск имени
	netbios_name nname( szName, type );
	ip addr = INADDR_NONE;
	FIND_NAME_BLOCK fn = { 0 };
	for (UCHAR i = 0; i < m_le.length; i++) {
		UINT uReturn = FindName (nname, m_le.lana [i], fn);
		if (uReturn == NRC_GOODRET) {
			LOG( "Found %s at %u boxes. LAN #%u IP: %u.%u.%u.%u", nname.GetANSIFullName(),
				fn.fnb_header.node_count, m_le.lana [i],
				fn.fnb_Names [0].source_addr[2],
				fn.fnb_Names [0].source_addr[3],
				fn.fnb_Names [0].source_addr[4],
				fn.fnb_Names [0].source_addr[5]);
			addr = ((ip)fn.fnb_Names [0].source_addr[5]) |
				((ip)fn.fnb_Names [0].source_addr[4] << 8) |
				((ip)fn.fnb_Names [0].source_addr[3] << 16) |
				((ip)fn.fnb_Names [0].source_addr[2] << 24);
			break;
		}
	}
	return addr;
}

void netbios::GetRegisteredNames (netbios_name_list& names)
{
	CLock oLock( m_csData );

	for (size_t i = 0; i < m_names.GetCount (); ++i)
		names.AddTail (*m_names [i]);
}

bool netbios::GetNames(netbios_name_list& names, LPCTSTR name, bool bGroup)
{
	// Получение имен
	ADAPTER_STATUS_BLOCK astat = { 0 };
	netbios_name nname( name, 0, bGroup );
	UINT uReturn = NRC_GOODRET;
	for (UCHAR i = 0; i < m_le.length; i++)
	{
		uReturn = GetAdapterStatus (nname, m_le.lana [i], astat);
		if (uReturn == NRC_GOODRET)
		{
			for (int j = 0; j < astat.asb_header.name_count; ++j)
			{
				names.AddTail (netbios_name (astat.asb_Names [j], m_le.lana [i]));
			}
		}
	}
	return (uReturn == NRC_GOODRET);
}

netbios_name* netbios::GetName (const netbios_name& nname)
{
	CLock oLock( m_csData );

	netbios_name* ret = NULL;
	for (size_t i = 0; i < m_names.GetCount (); ++i)
	{
		if ( nname == *(m_names [i]) )
		{
			// Похожее имя обнаружено
			ret = m_names [i];
			break;
		}
	}

	return ret;
}

bool netbios::FindNameLana(const netbios_name& nname, UCHAR& lana)
{
	// Получение имен
	ADAPTER_STATUS_BLOCK astat = {};
	for (UCHAR i = 0; i < m_le.length; i++)
	{
		UINT uReturn = GetAdapterStatus (nname, m_le.lana [i], astat);
		if ( uReturn == NRC_GOODRET )
		{
			for ( int j = 0; j < astat.asb_header.name_count; ++j )
			{
				if (nname == astat.asb_Names [j])
				{
					// Имя обнаружено
					LOG ( "FindNameLana : Name \"%s\" found at #%d", nname.GetANSIFullName(), m_le.lana [i]);
					lana = m_le.lana [i];
					return true;
				}
			}
		}
		LOG( "FindNameLana : Name \"%s\" not found", nname.GetANSIFullName());
	}

	return false;
}

bool netbios::GetMAC (UCHAR lana, CString& mac)
{
	ADAPTER_STATUS_BLOCK astat = { 0 };
	netbios_name nname;
	UINT uReturn = GetAdapterStatus (nname, lana, astat);
	if (uReturn == NRC_GOODRET) {
		mac.Format (_T("%02x:%02x:%02x:%02x:%02x:%02x"),
			astat.asb_header.adapter_address[0],
			astat.asb_header.adapter_address[1],
			astat.asb_header.adapter_address[2],
			astat.asb_header.adapter_address[3],
			astat.asb_header.adapter_address[4],
			astat.asb_header.adapter_address[5]);
	} else
		mac.Empty ();
	return true;
}

UCHAR netbios::FindName (const netbios_name& nname, UCHAR lana, FIND_NAME_BLOCK& fn)
{
	ZeroMemory (&fn, sizeof (FIND_NAME_BLOCK));
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	ncb->ncb_command = NCBFINDNAME;
	ncb->ncb_lana_num = lana;
	CopyMemory( ncb->ncb_callname, nname.netbiosed.name, NCBNAMSZ );
	ncb->ncb_buffer = reinterpret_cast <BYTE*> (&fn);
	ncb->ncb_length = sizeof (FIND_NAME_BLOCK);
	NetbiosEx (ncb);
	return ncb->ncb_retcode;
}

UCHAR netbios::GetAdapterStatus (const netbios_name& nname, UCHAR lana, ADAPTER_STATUS_BLOCK& astat)
{
	ZeroMemory (&astat, sizeof (ADAPTER_STATUS_BLOCK));
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	ncb->ncb_command = NCBASTAT;
	ncb->ncb_lana_num = lana;
	CopyMemory( ncb->ncb_callname, nname.netbiosed.name, NCBNAMSZ );
	ncb->ncb_buffer = reinterpret_cast <BYTE*> (&astat);
	ncb->ncb_length = sizeof (ADAPTER_STATUS_BLOCK);
	NetbiosEx (ncb);
	return ncb->ncb_retcode;
}

UCHAR netbios::EnumLanas (LANA_ENUM& le)
{
	// Перечисление адаптеров
	ZeroMemory (&le, sizeof (LANA_ENUM));
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	ncb->ncb_command = NCBENUM;
	ncb->ncb_buffer = (PUCHAR) &le;
	ncb->ncb_length = sizeof (LANA_ENUM);
	NetbiosEx (ncb);
	return ncb->ncb_retcode;
}

UCHAR netbios::ResetLana (UCHAR lana)
{
	// Сброс адаптера
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	ncb->ncb_command = NCBRESET;
	ncb->ncb_lana_num = lana;
	ncb->ncb_callname [0] = 20;	// maximum sessions 
	ncb->ncb_callname [2] = 30;	// maximum names 
	NetbiosEx (ncb);
	return ncb->ncb_retcode;
}

UCHAR netbios::Hangup (UCHAR lana, UCHAR lsn)
{
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	ncb->ncb_command = NCBHANGUP;
	ncb->ncb_lana_num = lana;
	ncb->ncb_lsn = lsn;
	NetbiosEx (ncb);
	return ncb->ncb_retcode;
}

UCHAR netbios::Send (UCHAR lana, UCHAR lsn, unsigned char* data, WORD length)
{
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	ncb->ncb_lana_num = lana;
	ncb->ncb_command = NCBSEND;
	ncb->ncb_lsn = lsn;
	ncb->ncb_length = length;
	ncb->ncb_buffer = data;
	ncb->ncb_rto = ncb->ncb_sto = (UCHAR) 10;	// 10 * 500 ms = 5 s
	NetbiosEx (ncb);
	return ncb->ncb_retcode;
}

UCHAR netbios::Recv (UCHAR lana, UCHAR lsn, unsigned char* data, WORD& length)
{
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	ncb->ncb_command = NCBRECV;
	ncb->ncb_lana_num = lana;
	ncb->ncb_lsn = lsn;
	ncb->ncb_length = length;
	ncb->ncb_buffer = data;
	ncb->ncb_rto = ncb->ncb_sto = (UCHAR) 10;	// 10 * 500 ms = 5 s
	NetbiosEx (ncb);
	length = ncb->ncb_length;
	return ncb->ncb_retcode;
}

UCHAR netbios::Stat (const netbios_name& nname, SESSION_INFO_BLOCK* psibSession)
{
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	ncb->ncb_command = NCBSSTAT;
	ncb->ncb_lana_num = nname.GetLana();
	ncb->ncb_buffer = (unsigned char*)psibSession;
	ncb->ncb_length = sizeof (SESSION_INFO_BLOCK);
	CopyMemory (ncb->ncb_name, nname.netbiosed.name, NCBNAMSZ);
	NetbiosEx (ncb);
	return ncb->ncb_retcode;
}

UCHAR netbios::Listen (const netbios_name& nname, UCHAR& lsn)
{
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	CopyMemory (ncb->ncb_name, nname.netbiosed.name, NCBNAMSZ);
	CopyMemory (ncb->ncb_callname, SMB_ANY_NAME, NCBNAMSZ);
	ncb->ncb_command = NCBLISTEN;
	ncb->ncb_num = nname.netbiosed.name_num;
	ncb->ncb_lana_num = nname.GetLana();
	ncb->ncb_rto = ncb->ncb_sto = (UCHAR) 2;	// 2 * 500 ms = 1 s
	NetbiosEx (ncb);
	lsn = ncb->ncb_lsn;
	return ncb->ncb_retcode;
}

UCHAR netbios::AddName (netbios_name& nname)
{
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	ncb->ncb_command = (UCHAR)( nname.IsGroupName() ? NCBADDGRNAME : NCBADDNAME );
	ncb->ncb_lana_num = nname.GetLana();
	CopyMemory (ncb->ncb_name, nname.netbiosed.name, NCBNAMSZ);
	NetbiosEx (ncb);
	nname.netbiosed.name_num = ncb->ncb_num;
	return ncb->ncb_retcode;
}

UCHAR netbios::DeleteName (const netbios_name& nname)
{
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	ncb->ncb_command = NCBDELNAME;
	ncb->ncb_lana_num = nname.GetLana();
	ncb->ncb_num = nname.netbiosed.name_num;
	CopyMemory (ncb->ncb_name, nname.netbiosed.name, NCBNAMSZ);
	NetbiosEx (ncb);
	return ncb->ncb_retcode;
}

UCHAR netbios::SendDatagram (const netbios_name& nname_from, const netbios_name& nname_to, unsigned char* data, WORD length)
{
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	CopyMemory( ncb->ncb_name, nname_from.netbiosed.name, NCBNAMSZ );
	CopyMemory( ncb->ncb_callname, nname_to.netbiosed.name, NCBNAMSZ );
	ncb->ncb_command = NCBDGSEND;
	ncb->ncb_num = nname_from.netbiosed.name_num;
	ncb->ncb_lana_num = nname_from.GetLana();
	ncb->ncb_buffer = data;
	ncb->ncb_length = length;

	CLock oLock( m_csNetBIOS );
	Sleep( 100 );

	NetbiosEx (ncb);

	return ncb->ncb_retcode;
}

UCHAR netbios::RecvDatagram (netbios_name& nname_from, const netbios_name& nname_to, unsigned char* data, WORD& length)
{
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	CopyMemory (ncb->ncb_name, nname_to.netbiosed.name, NCBNAMSZ);
	ncb->ncb_command = NCBDGRECV;
	ncb->ncb_num = nname_to.netbiosed.name_num;
	ncb->ncb_lana_num = nname_to.GetLana();
	ncb->ncb_buffer = data;
	ncb->ncb_length = length;
	NetbiosEx (ncb);
	nname_from = ncb->ncb_callname;
	length = ncb->ncb_length;
	return ncb->ncb_retcode;
}

UCHAR netbios::Call (UCHAR lana, const netbios_name& nname_from, const netbios_name& nname_to, UCHAR& lsn)
{
	CNCB ncb;
	if ( ! ncb ) return NRC_SYSTEM;
	CopyMemory( ncb->ncb_name, nname_from.netbiosed.name, NCBNAMSZ );
	CopyMemory( ncb->ncb_callname, nname_to.netbiosed.name, NCBNAMSZ );
	ncb->ncb_lana_num = lana;
	ncb->ncb_rto = ncb->ncb_sto = 10; // 5 секунд
	ncb->ncb_command = NCBCALL;
	NetbiosEx (ncb);
	lsn = ncb->ncb_lsn;
	return ncb->ncb_retcode;
}

bool netbios::AskAway(const netbios_name& nname_to)
{
	bool ret = false;
	if ( m_initialized )
	{
		// Сборка пакета
		const WORD packet_size = sizeof( WORD ) + 1;
		if ( UCHAR* packet = (UCHAR*)mir_alloc( packet_size ) )
		{
			*(WORD*)packet = SM_MAGIC;
			packet[ 2 ] = SM_GETAWAYMESSAGE;

			for (UCHAR i = 0; i < m_le.length; i++)
			{
				if ( netbios_name* nname = GetName( netbios_name(
					pluginMachineName, 0x03, false, m_le.lana [i] ) ) )
				{
					LOG( "Send \"Ask Away\" request to \"%s\"", nname_to.GetANSIFullName() );

					if ( SendDatagram( *nname, nname_to, packet, packet_size ) == NRC_GOODRET )
					{
						ret = true;
					}
				}
			}

			mir_free( packet );
		}
	}
	return ret;
}

bool netbios::SendAway(netbios_name& nname_from, const netbios_name& nname_to)
{
	bool ret = false;
	if ( m_initialized )
	{
		CString sAwayT;
		pluginStatusMessage.Lookup( pluginCurrentStatus, sAwayT );
		CT2A sAwayA( sAwayT );
		WORD len = (WORD)min( lstrlenA( sAwayA ), 250 );

		// Сборка пакета
		WORD packet_size = (WORD)( 2 + 1 + 4 + len );
		if ( UCHAR* packet = (UCHAR*)mir_alloc( packet_size ) )
		{
			*(WORD*)packet = SM_MAGIC;
			packet [ 2 ] = SM_SENDAWAYMESSAGE;
			*(__int32*)( packet + 2 + 1 ) = 0;
			CopyMemory( packet + 2 + 1 + 4, sAwayA, len );

			LOG( "Send \"Away\" answer from \"%s\" to \"%s\" : \"%s\"", nname_from.GetANSIFullName(), nname_to.GetANSIFullName(), (LPCSTR)sAwayA );

			ret = ( SendDatagram( nname_from, nname_to, packet, packet_size ) == NRC_GOODRET );

			mir_free( packet );
		}
	}
	return ret;
}

bool netbios::AskStatus(const netbios_name& nname_to)
{
	bool ret = false;
	if ( m_initialized )
	{
		// Сборка пакета
		const WORD packet_size = 2 + 1;
		if ( UCHAR* packet = (UCHAR*)mir_alloc( packet_size ) )
		{
			*(WORD*)packet = SM_MAGIC;
			packet[ 2 ] = SM_GETSTATUS;

			for (UCHAR i = 0; i < m_le.length; i++)
			{
				if ( netbios_name* nname = GetName( netbios_name(
					pluginMachineName, 0x03, false, m_le.lana [i] ) ) )
				{
					LOG( "Send \"Ask Status\" request to \"%s\"", nname_to.GetANSIFullName() );

					if ( SendDatagram( *nname, nname_to, packet, packet_size ) == NRC_GOODRET )
					{
						ret = true;
					}
				}
			}

			mir_free( packet );
		}
	}
	return ret;
}

bool netbios::SendStatus(netbios_name& nname_from, const netbios_name& nname_to)
{
	bool ret = false;
	if ( m_initialized )
	{
		// Сборка пакета
		const WORD packet_size = 2 + 1 + 4;
		if ( UCHAR* packet = (UCHAR*)mir_alloc( packet_size ) )
		{
			*(WORD*)packet = SM_MAGIC;
			packet [ 2 ] = SM_SENDSTATUS;
			*(__int32*)( packet + 2 + 1 ) = (__int32)pluginCurrentStatus;

			LOG( "Send \"Status\" answer from \"%s\" to \"%s\" : \"%s\"", nname_from.GetANSIFullName(), nname_to.GetANSIFullName(), STATUS2TEXT(pluginCurrentStatus) );

			ret = ( SendDatagram( nname_from, nname_to, packet, packet_size ) == NRC_GOODRET );

			mir_free( packet );
		}
	}
	return ret;
}

bool netbios::BroadcastStatus()
{
	bool ret = false;
	if ( m_initialized )
	{
		for (UCHAR i = 0; i < m_le.length; i++)
		{
			netbios_name nname_to( MNS_STATUS, 0xab, true, m_le.lana [i] );
			netbios_name* nname = GetName(
				netbios_name ( pluginMachineName, 0x03, false, m_le.lana [i] ) );
			if ( nname )
				ret = SendStatus( *nname, nname_to ) || ret;
		}
	}
	return ret;
}

bool netbios::AskAvatar(const netbios_name& nname_to)
{
	bool ret = false;
	if ( m_initialized )
	{
		// Сборка пакета
		const WORD packet_size = 2 + 1;
		if ( UCHAR* packet = (UCHAR*)mir_alloc( packet_size ) )
		{
			*(WORD*)packet = SM_MAGIC;
			packet[ 2 ] = SM_GETAVATAR;

			for (UCHAR i = 0; i < m_le.length; i++)
			{
				if ( netbios_name* nname = GetName( netbios_name(
					pluginMachineName, 0x03, false, m_le.lana [i] ) ) )
				{
					LOG( "Send \"Ask Avatar\" request to \"%s\"", nname_to.GetANSIFullName() );

					if ( SendDatagram( *nname, nname_to, packet, packet_size ) == NRC_GOODRET )
					{
						ret = true;
					}
				}
			}

			mir_free( packet );
		}
	}
	return ret;
}

bool netbios::SendAvatar(netbios_name& nname_from, const netbios_name& nname_to)
{
	if ( ! m_initialized )
		return false;

	if ( ! ServiceExists( MS_AV_GETMYAVATAR ) )
		// Нет аватарского плагина
		return false;

	// Запрос аватара протокола
	AVATARCACHEENTRY* pAvatar = (AVATARCACHEENTRY*)CallService(
		MS_AV_GETMYAVATAR, 0, (LPARAM)modname );
	if ( ! pAvatar )
		// Запрос общего аватара
		pAvatar = (AVATARCACHEENTRY*)CallService( MS_AV_GETMYAVATAR, 0, (LPARAM)"" );
	if ( ! pAvatar || pAvatar->cbSize < sizeof( AVATARCACHEENTRY ) )
		// Нет аватара
		return false;

	CString sFilename = pAvatar->szFilename;

	CAtlFile oAvatarFile;
	if ( FAILED( oAvatarFile.Create( sFilename, GENERIC_READ,
		FILE_SHARE_READ, OPEN_EXISTING ) ) )
		// Файл не найден
		return false;

	ULONGLONG avatar_size = 0;
	if ( FAILED( oAvatarFile.GetSize( avatar_size ) ) ||
		avatar_size < 16 || avatar_size > MAX_AVATAR_SIZE )
		// Слишком маленький или слишком большой файл
		return false;

	bool ret = false;

	// Сборка статусного пакета
	WORD packet_size = (WORD)( 2 + 1 + avatar_size );
	if ( UCHAR* packet = (UCHAR*)mir_alloc( packet_size ) )
	{
		*(WORD*)packet = SM_MAGIC;
		packet[ 2 ] = SM_SENDAVATAR;

		// Чтение аватара из файла
		if ( SUCCEEDED( oAvatarFile.Read( packet + 2 + 1, avatar_size ) ) )
		{
			LOG( "Send \"Avatar\" answer from \"%s\" to \"%s\"", nname_from.GetANSIFullName(), nname_to.GetANSIFullName() );

			ret = ( SendDatagram( nname_from, nname_to, packet, packet_size ) == NRC_GOODRET );
		}

		mir_free( packet );
	}

	return ret;
}

bool netbios::Register ()
{
	CLock oLock( m_csData );

	bool ret = false;

	// Удаление имён, если они есть
	for (size_t i = 0; i < m_names.GetCount (); ++i)
		delete m_names [i];
	m_names.RemoveAll ();

	// Добавление имен на каждом адаптере
	for (UCHAR i = 0; i < m_le.length; i++)
	{
		// COMPUTER <01> U
		netbios_name *pnn1 =
			db_get_b (NULL, modname, "RegisterNick", TRUE) ? 
			new netbios_name ( pluginMachineName, 0x01, false, m_le.lana [i]) : NULL;
		if (pnn1)
			m_names.Add (pnn1);

		// COMPUTER <03> U
		netbios_name *pnn2 =
			db_get_b (NULL, modname, "RegisterNick", TRUE) ? 
			new netbios_name ( pluginMachineName, 0x03, false, m_le.lana [i]) : NULL;
		if (pnn2)
			m_names.Add (pnn2);

		// USER <03> U
		netbios_name *pnn3 =
			db_get_b (NULL, modname, "RegisterUser", TRUE) ? 
			new netbios_name ( pluginUserName, 0x03, false, m_le.lana [i]) : NULL;
		if (pnn3) {
			// Проверка на совпадение имени пользователя и имени компьютера
			if (pnn2 && *pnn3 == *pnn2)
				// Имена совпадают
				delete pnn3;
			else
				m_names.Add (pnn3);
		}

		// MNS_STATUS <AB> G
		netbios_name *pnn4 =
			db_get_b (NULL, modname, "RegisterStatus", TRUE) ? 
			new netbios_name (MNS_STATUS, 0xab, true, m_le.lana [i]) : NULL;
		if ( pnn4 )
			m_names.Add( pnn4 );
	}

	// Регистрация имен
	for ( size_t i = 0; i < m_names.GetCount (); ++i )
	{
		if ( m_names [i]->Register() )
		{
			ret = true;
		}
	}

	return ret;
}

// Дерегистрация NetBIOS-имен
void netbios::Deregister ()
{
	CLock oLock( m_csData );

	// Дерегистрация имён
	for (size_t i = 0; i < m_names.GetCount (); ++i)
		m_names [i]->Destroy();

	// Удаление имён
	for (size_t i = 0; i < m_names.GetCount (); ++i)
		delete m_names [i];
	m_names.RemoveAll ();
}
