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

volatile WPARAM		pluginRequestedStatus	= ID_STATUS_OFFLINE;
volatile WPARAM		pluginCurrentStatus		= ID_STATUS_OFFLINE;
CIntStrMap			pluginStatusMessage;
CString				pluginMachineName;
CString				pluginUserName;
CString				pluginDomainName;
HMODULE				pluginModule			= NULL;
volatile bool		pluginBusy				= false;
volatile bool		pluginInstalled			= false;
volatile bool		pluginInitialized		= false;
HANDLE				pluginNetLibUser		= NULL;
HANDLE				pluginInternalState		= NULL;
bool				pluginChatEnabled		= false;
OSVERSIONINFO		pluginOS				= { sizeof( OSVERSIONINFO ) };

CComAutoCriticalSection	pluginGuard;		// Защита доступа к картам:
CThreadContactMap	pluginAwayThreadMap;	// Карта асинхронных запросов эвей-сообщений
CThreadContactMap	pluginAvatarThreadMap;	// Карта асинхронных запросов аватаров

CString GetNick(HANDLE hContact)
{
	CString sNick;
	DBVARIANT dbv = {};
	if ( ! db_get_ts( hContact, modname, "Nick", &dbv ) )
	{
		sNick = dbv.ptszVal;
		db_free( &dbv );
	}
	return sNick;
}

void SetNick(HANDLE hContact, LPCTSTR szNick)
{
	db_set_ts( hContact, modname, "Nick", szNick );
}

CComAutoCriticalSection _LOG_SECTION;

int LOG(const char *fmt,...)
{
	CComCritSecLock< CComAutoCriticalSection > _Lock( _LOG_SECTION );

	int ret = 0;
	const int size = 512;
	if ( char* szText = (char*)mir_alloc( size ) )
	{
		*szText = 0;
		va_list va;
		va_start( va, fmt );
		mir_vsnprintf( szText, size, fmt, va );
		va_end( va );
		ret = CallService( MS_NETLIB_LOG, (WPARAM)pluginNetLibUser, (LPARAM)szText );
		mir_free( szText );
	}
	return ret;
}

void GetAvatarCache(LPTSTR szPath)
{
	// Получить путь новым способом
	if ( ServiceExists( MS_UTILS_REPLACEVARS ) )
	{
		LPTSTR szAvatarCache = Utils_ReplaceVarsT(
			_T("%miranda_avatarcache%\\") modname_t _T("\\") );
		if ( szAvatarCache && szAvatarCache != (LPTSTR)0x80000000 )
		{
			lstrcpyn( szPath, szAvatarCache, MAX_PATH );

			// Создание пути до будущего файла аватара
			CallService( MS_UTILS_CREATEDIRTREET, 0, (LPARAM)szPath );
			return;
		}
	}

	// Получить путь старым способом
	char szProfilePath[ MAX_PATH ], szProfileName[ MAX_PATH ];
	CallService( MS_DB_GETPROFILEPATH, MAX_PATH, (LPARAM)szProfilePath );
	CallService( MS_DB_GETPROFILENAME, MAX_PATH, (LPARAM)szProfileName );
	char *pos = strrchr( szProfileName, '.' );
	if ( lstrcmpA( pos, ".dat" ) == 0 )
		*pos = 0;
	lstrcpy( szPath, CA2T( szProfilePath ) );
	lstrcat( szPath, _T("\\") );
	lstrcat( szPath, CA2T( szProfileName ) );
	lstrcat( szPath, _T("\\AvatarCache\\") modname_t _T("\\") );

	// Создание пути до будущего файла аватара
	CallService( MS_UTILS_CREATEDIRTREET, 0, (LPARAM)szPath );
	return;
}

static LONG cookie = (LONG)GetTickCount();

HANDLE GenerateCookie()
{
	return (HANDLE)InterlockedIncrement( &cookie );
}

DWORD time()
{
	LARGE_INTEGER lft = {};
	GetSystemTimeAsFileTime ((FILETIME*) &lft);
	return (DWORD) ((lft.QuadPart - 116444736000000000) / 10000000);
}

bool IsMyContact(HANDLE hContact)
{
	if ( ! hContact )
		// Это вообще не контакт
		return false;

	char* proto = (char*)CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0 );
	if ( ! proto )
		// Это ничейный контакт
		return false;

	if ( lstrcmpA( proto, modname ) )
		// Это не наш контакт
		return false;

	// Это наш контакт
	return true;
}

void SetPluginStatus(WPARAM status)
{
	WPARAM old = pluginCurrentStatus;
	pluginCurrentStatus = status;
	if (pluginInstalled)
	{
		ProtoBroadcastAck (modname, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS,
			(HANDLE) old, (LPARAM) pluginCurrentStatus);

		if ( old != pluginCurrentStatus &&
			pluginCurrentStatus >= ID_STATUS_OFFLINE && 
			pluginCurrentStatus <= ID_STATUS_OUTTOLUNCH )
		{
			pluginNetBIOS.BroadcastStatus();
		}
	}
}

bool InternalStartup()
{
	_ASSERT( pluginInstalled == true );

	LOG ("Startup begin");

	ResetEvent( pluginInternalState );

	bool err = false;

	BYTE method = (BYTE) db_get_b (NULL, modname, "SendMethod", 0);
	if ( method == 2 )
	{
		// Инициализация "Службы Сообщений" с запуском
		if (pluginMessenger.Create (TRUE))
			err = true;
		LOG ("Startup : Messenger");

		// Инициализация NetBIOS
		if (!pluginNetBIOS.Create (FALSE))
			err = true;
		LOG ("Startup : NetBIOS");
	}
	else
	{
		// Инициализация "Службы Сообщений" с остановом
		if (pluginMessenger.Create (FALSE))
			err = true;

		// Запуск мониторинга входящих сообщений через мейлслот
		if ( ! pluginMailslot.Create( MESSENGER_MAIL ) )
			err = true;
		LOG ("Startup : Mailslot");

		// Инициализация и запуск мониторинга входящих сообщений через NetBIOS
		if (!pluginNetBIOS.Create (TRUE))
			err = true;
		LOG ("Startup : NetBIOS");
	}

	// Запуск мониторинга контактов
	if (!pluginScanner.Create ())
		err = true;
	LOG ("Startup : Scanner");

	LOG ("Startup end");

	return !err;
}

void InternalShutdown ()
{
	LOG ("Shutdown begin");

	// Предварительные запросы на закрытие
	pluginSearch.AskForDestroy();
	pluginMailslot.AskForDestroy();
	pluginScanner.AskForDestroy();
	pluginNetBIOS.AskForDestroy();
	pluginMessenger.AskForDestroy();

	// Закрытие мейлслота (быстро)
	pluginMailslot.Destroy ();
	LOG ("Shutdown : Mailslot");

	// Завершение поиска (медленно, если идёт поиск)
	pluginSearch.Destroy();
	LOG ("Shutdown : Search");

	// Дерегистрация NetBIOS имён (медленно)
	pluginNetBIOS.Destroy ();
	LOG ("Shutdown : NetBIOS");

	// Останов мониторинга контактов (медленно, если идёт опрос)
	pluginScanner.Destroy ();
	LOG ("Shutdown : Scanner");

	// Освобождение "Службы Сообщений" (медленно, если нужно запускать сервис)
	pluginMessenger.Destroy ();
	LOG ("Shutdown : Messenger");

	LOG ("Shutdown end");
	SetEvent( pluginInternalState );
}

void GotoOnline ()
{
	if ( pluginCurrentStatus != ID_STATUS_OFFLINE )
	{
		// Уже в онлайне
		if ( pluginCurrentStatus != ID_STATUS_CONNECTING )
		{
			// Просто установка нового статуса
			SetPluginStatus (pluginRequestedStatus);
			return;
		}
	}

	SetPluginStatus (ID_STATUS_CONNECTING);

	if (!pluginInstalled || !pluginInitialized || pluginBusy)
		// Отложенное подключение (или запрет)
		return;
	pluginBusy = true;

	// Начало соединения
	mir_forkthread( GotoOnlineTread, NULL );
}

void GotoOffline()
{
	// Подтверждение текущего статуса
	SetPluginStatus (ID_STATUS_OFFLINE);

	if (pluginBusy)
		// Двойной вызов
		return;
	pluginBusy = true;

	// Перевод всех контактов в оффлайн
	FOR_EACH_CONTACT( hContact ) {
		SetContactStatus (hContact, ID_STATUS_OFFLINE, true);
	}

	// Начало отсоединения
	mir_forkthread( GotoOfflineTread, NULL );
}

void GotoOnlineTread(LPVOID /* status */)
{
	// Запуск
	InternalStartup();
	pluginBusy = false;
	Sleep( 1000 );

	// Перепроверка статуса
	if ( ! pluginInstalled || pluginRequestedStatus == ID_STATUS_OFFLINE )
		// Черт, нужно обратно в оффлайн
		GotoOffline ();
	else
		// Все в порядке, установка статуса
		SetPluginStatus (pluginRequestedStatus);
}

void GotoOfflineTread(LPVOID /* status */)
{
	// Выключение всего
	InternalShutdown ();
	pluginBusy = false;
	Sleep( 1000 );

	// Перепроверка статуса
	if ( pluginInstalled && pluginRequestedStatus != ID_STATUS_OFFLINE )
		// Черт, нужно обратно в онлайн
		GotoOnline ();
	else
		// Подтверждение текущего статуса
		SetPluginStatus (ID_STATUS_OFFLINE);
}

void GetAwayMsgThread(LPVOID param)
{
	// Задержка чтобы миранда успела "переварить" ответ
	Sleep( 250 );

	ContactData* data = (ContactData*)param;

	bool ret = false;

	bool bGroup = IsGroup( data->hContact );
	CString sNick = GetNick( data->hContact );
	if ( ! bGroup && ! sNick.IsEmpty() )
	{
		ThreadEvent te = { CreateEvent( NULL, TRUE, FALSE, NULL ), data->cookie };

		// Отмечаемся в карте ожидающих нитей
		{
			CLock oLock( pluginGuard );
			pluginAwayThreadMap.SetAt( data->hContact, te );
		}

		// Асинхронный запрос эвей-сообщения
		ret = pluginNetBIOS.AskAway( netbios_name( sNick, 0x03, false ) );

		// Ожидание завершения (3 секунды)
		ret = ret && ( WaitForSingleObject( te.evt, 3000 ) == WAIT_OBJECT_0 );

		// Очистка карты ожидающих нитей
		{
			CLock oLock( pluginGuard );
			pluginAwayThreadMap.Lookup( data->hContact, te );
			pluginAwayThreadMap.RemoveKey( data->hContact );
		}

		CloseHandle( te.evt );
	}

	if ( ! ret )
	{
		// Уведомление о неудаче
		ProtoBroadcastAck (modname, data->hContact, ACKTYPE_AWAYMSG,
			ACKRESULT_SUCCESS /* ACKRESULT_FAILED */, data->cookie, (LPARAM)"" );
		LOG( "Get away message failed" );
	}

	delete data;
}

void SetContactAway(HANDLE hContact, LPCSTR away)
{
	if ( ! pluginInstalled )
		return;

	// Определение ожидающей нити по контакту
	bool ret = false;
	ThreadEvent te = {};
	{
		CLock oLock( pluginGuard );
		if ( pluginAwayThreadMap.Lookup( hContact, te ) )
		{
			SetEvent( te.evt );
			ret = true;
		}
	}

	if ( ret )
	{
		// Уведомление об эвей-сообщении
		ProtoBroadcastAck( modname, hContact, ACKTYPE_AWAYMSG,
			ACKRESULT_SUCCESS, te.cookie, (LPARAM)away );
	}
	else
	{
		// Принудительная установка эфей-сообщения для контакта
	}
}

void GetInfoThread(LPVOID param)
{
	// Задержка чтобы миранда успела "переварить" ответ
	Sleep( 500 );

	HANDLE hContact = (HANDLE)param;

	ProtoBroadcastAck( modname, hContact, ACKTYPE_GETINFO,
		ACKRESULT_SUCCESS, (HANDLE)0, 0 );
}

void Autoanswer(HANDLE hContact)
{
	switch ( pluginCurrentStatus )
	{
	case ID_STATUS_AWAY:
	case ID_STATUS_DND:
	case ID_STATUS_NA:
	case ID_STATUS_OCCUPIED:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		{
			CString msg;
			if ( pluginStatusMessage.Lookup( pluginCurrentStatus, msg ) )
			{
				// Отсылка сообщения отправителю
				CString answer (TranslateT ("Auto-reply"));
				answer += _T(":\r\n");
				answer += msg;
				DWORD foo;
				SendContactMessage( hContact, answer, foo );

				// Добавление сообщения
				DBEVENTINFO ei = {};
				ei.cbSize = sizeof (DBEVENTINFO);
				ei.szModule = modname;
				ei.timestamp = time();
				ei.flags = DBEF_SENT;
				ei.eventType = EVENTTYPE_MESSAGE;
				ei.cbBlob = (DWORD)answer.GetLength () + 1;
				ei.pBlob = (PBYTE) (LPCTSTR) answer;
				CallServiceSync( MS_DB_EVENT_ADD, (WPARAM)hContact, (LPARAM)&ei );
			}
		}
		break;
	}
}

void ReceiveContactMessage(LPCTSTR msg_from, LPCTSTR msg_to, LPCTSTR msg_text, int msg_len)
{
	if ( ! pluginInstalled )
		return;

	CString from( msg_from );
	CString to( msg_to );
	CString text( msg_text, msg_len );

	from.MakeUpper();
	to.MakeUpper();

	// Свое собственное сообщение?
	if ( IsItMe( from ) )
	{
		LOG ( "Ignoring my message." );
		return;
	}

	// Нормализация разбиения сообщения на строки
	Normalize( text );

	// Дубликат?
	if (db_get_b (NULL, modname, "Filter-dups", TRUE))
	{		
		// Вычисление прошедшего времени с последнего сообщения
		static FILETIME last_time = { 0, 0 };
		FILETIME current_time;
		GetSystemTimeAsFileTime (&current_time);
		ULONGLONG elapsed =
			((ULONGLONG) current_time.dwLowDateTime |
				(ULONGLONG) current_time.dwHighDateTime << 32) -
			((ULONGLONG) last_time.dwLowDateTime |
				(ULONGLONG) last_time.dwHighDateTime << 32);

		// Вычисление MD5-хэшей отправителя
		MD5Context ctx;
		md5init (&ctx);
		md5update (&ctx, (const unsigned char*)(LPCTSTR)from,
			from.GetLength() * sizeof (TCHAR));
		unsigned char digest_from_current [16] = {0};
		static unsigned char digest_from_last [16] = {0};
		md5final (digest_from_current, &ctx);

		// Вычисление MD5-хэшей сообщения
		md5init (&ctx);
		md5update (&ctx, (const unsigned char*)(LPCTSTR)text,
			text.GetLength() * sizeof (TCHAR));
		unsigned char digest_text_current [16] = {0};
		static unsigned char digest_text_last [16] = {0};
		md5final (digest_text_current, &ctx);

		// Если прошло менее 2 секунд между сообщениями
		if (elapsed < 20000000)
		{
			// И отправители совпадают
			if (memcmp (digest_from_current, digest_from_last, 16) == 0)
			{
				// И сообщение совпадает
				if (memcmp (digest_text_current, digest_text_last, 16) == 0)
				{
					// то пропускаем такое сообщение
					LOG ("Duplicate message detected");
					return;
				}
			}
		}
		last_time = current_time;
		CopyMemory (digest_from_last, digest_from_current, 16);
		CopyMemory (digest_text_last, digest_text_current, 16);
	}

#ifdef CHAT_ENABLED
	if ( ! IsItMe( to ) && pluginChatEnabled ) // Групповой адрес?
	{
		// Формирование группового сообщения
		if ( ChatNewSession( to ) )
		{
			// Добавить группу
			ATLVERIFY( ChatAddGroup( to, _T("Normal") ) );

			// Добавить себя
			ATLVERIFY( ChatJoinMe( to, _T("Normal") ) );

			// Добавить "От Кого"
			ATLVERIFY( ChatJoinUser( to, from, _T("Normal") ) );

			// Завершение создания чата
			ATLVERIFY( ChatInitDone( to ) );

			// Перевод чат-контакта в онлайн
			ATLVERIFY( ChatOnline( to ) );

			// Сообщение
			ATLVERIFY( ChatMessage( to, from, text ) );
		}
	}
	else
#endif // CHAT_ENABLED
	{
		// Формирование приватного сообщения
		HANDLE hContact = AddToListByName( from, 0, NULL, false, false );
		if ( hContact )
		{
			PROTORECVEVENT pre = { 0 };
			pre.timestamp = time ();
			CT2A textA( text );
			pre.szMessage = (LPSTR)(LPCSTR)textA;
			CCSDATA ccs = { 0 };
			ccs.szProtoService = PSR_MESSAGE;
			ccs.hContact = hContact;
			db_unset (ccs.hContact, "CList", "Hidden");
			ccs.lParam = (LPARAM) &pre;
			CallServiceSync (MS_PROTO_CHAINRECV, 0, (LPARAM) &ccs);

			// Переводим контакт в онлайн
			SetContactStatus( hContact, contact_scanner::ScanContact( hContact ), true );

			// Авто-ответчик
			if ( db_get_b( NULL, modname, "Auto-answer", FALSE ) )
				Autoanswer( hContact );
		}
	}
}

void Normalize(CString& msg)
{
	enum { CR, LF, NOP };
	int line_break = NOP;
	int i = 0;
	for (; i < msg.GetLength (); ++i)
	{
		switch (msg [i])
		{
		case _T('\r'):
		case _T('\xb6'):
			switch (line_break)
			{
			case CR:	// CRCR
			case LF:	// LFCR
				msg.Delete (i);
				msg.Delete (i - 1);
				msg.Insert (i - 1, _T('\r'));
				msg.Insert (i, _T('\n'));
				line_break = NOP;
				break;
			default:	// xxCR
				line_break = CR;
			}
			break;
		case _T('\n'):
			switch (line_break)
			{
			case CR:	// CRLF
				line_break = NOP;
				break;
			case LF:	// LFLF
				msg.Delete (i);
				msg.Delete (i - 1);
				msg.Insert (i - 1, _T('\r'));
				msg.Insert (i, _T('\n'));
				line_break = NOP;
				break;
			default:	// xxLF
				line_break = LF;
			}
			break;
		default:
			switch (line_break)
			{
			case CR:	// CR без LF
			case LF:	// LF без CR
				msg.Delete (i - 1);
				msg.Insert (i - 1, _T('\r'));
				msg.Insert (i, _T('\n'));
				++i;
				break;
			}
			line_break = NOP;
		}
	}
	switch (line_break)
	{
	case CR:	// CR без LF
	case LF:	// LF без CR
		msg.Delete (i - 1);
		msg.Insert (i - 1, _T('\r'));
		msg.Insert (i, _T('\n'));
		break;
	}
}

HANDLE AddToListByName(const CString& sName, WPARAM flags, LPCTSTR about, bool bInteractive, bool bGroup)
{
	ip addr = INADDR_NONE;
	CString sShortName( sName );

	if ( ! bGroup )
	{
		// Попытка получить IP из имени
		if ( addr == INADDR_NONE )
			addr = ResolveToIP( sShortName );

		// Поиск NetBIOS-имени
		if ( addr == INADDR_NONE )
			addr = pluginNetBIOS.FindNameIP( sName );

		// Неизвестный контакт
		if ( addr == INADDR_NONE && bInteractive )
		{
			if ( MessageBox( NULL,
				TranslateT("Cannot resolve contacts IP-address. Add it anyway?"),
				modname_t, MB_YESNO | MB_ICONQUESTION ) != IDYES )
			{
				return NULL;
			}
		}
	}

	// Поиск существующего контакта
	HANDLE hContact = GetContact( sShortName );
	if ( ! hContact )
	{
		// Добавление контакта
		hContact = (HANDLE)CallService( MS_DB_CONTACT_ADD, 0, 0 );
		if ( hContact )
		{
			CallService( MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)modname );
			SetNick( hContact, sShortName );
			SetGroup( hContact, bGroup );
			db_set_ts( hContact, "CList", "MyHandle", sShortName );
			db_set_b( hContact, "CList", "NotOnList", 1 );
			db_set_b( hContact, "CList", "Hidden", 1 );
			SetContactIP( hContact, addr );
			SetElapsed( hContact, "IPTime" );
			if ( about )
				db_set_ts( hContact, modname, "About", about );

			contact_scanner::ScanContact( hContact );
		}
	}
	if ( hContact && ! ( flags & PALF_TEMPORARY ) &&
		db_get_b( hContact, "CList", "NotOnList", 1 ) )
	{
		// Оставляем контакт
		db_unset( hContact, "CList", "NotOnList" );
		db_unset( hContact, "CList", "Hidden" );
	}
	return hContact;
}

DWORD GetElapsed (HANDLE hContact, const char* name)
{
	FILETIME current = {};
	GetSystemTimeAsFileTime (&current);
	LARGE_INTEGER currentL = {};
	currentL.LowPart = current.dwLowDateTime;
	currentL.HighPart = (LONG)current.dwHighDateTime;					
	LARGE_INTEGER lastseenL = {};
	lastseenL.LowPart = db_get_dw (hContact, modname,
		CStringA (name) + "L", 0);
	lastseenL.HighPart = (LONG)db_get_dw (hContact, modname,
		CStringA (name) + "H", 0);
	return (DWORD) (((currentL.QuadPart - lastseenL.QuadPart) / 10000000L) & 0xffffffff);
}

void SetElapsed (HANDLE hContact, const char* name)
{
	FILETIME current = {};
	GetSystemTimeAsFileTime (&current);
	db_set_dw (hContact, modname,
		CStringA (name) + "L", current.dwLowDateTime);
	db_set_dw (hContact, modname,
		CStringA (name) + "H", current.dwHighDateTime);
}

HANDLE GetContact (ip addr)
{
	// Перебор списка контактов
	FOR_EACH_CONTACT( hContact )
	{
		// Получение имени контакта
		ip contact_addr = db_get_dw (hContact, modname, "IP", 0);
		if (contact_addr && (contact_addr == addr))
			// Найдено совпадение
			break;
	}
	return hContact;
}

HANDLE GetContact (LPCTSTR name)
{
	// Перебор списка контактов
	FOR_EACH_CONTACT( hContact )
	{
		// Получение имени контакта
		CString sNick = GetNick( hContact );
		if ( ! sNick.IsEmpty() )
		{
			// Сличение имен
			if ( sNick.CompareNoCase( name ) == 0 )
				// Найдено совпадение
				break;
		}
	}
	return hContact;
}

void SetContactStatus (HANDLE hContact, int status, bool simple)
{
	if ( ! pluginInstalled )
		return;

	SetElapsed (hContact, "LastSeen");

#ifdef CHAT_ENABLED
	if ( IsChatRoom( hContact ) )
	{
		CString sSession = GetChatSession( hContact );
		if ( pluginChatEnabled && ! sSession.IsEmpty() )
		{
			if ( status != ID_STATUS_OFFLINE )
				ChatOnline( sSession );
			else
				ChatOffline( sSession );
		}
	}
	else
#endif // CHAT_ENABLED
	{
		int ns = db_get_w (hContact, modname, "Status", -1);
		if ( ns != status )
		{
			// Изменение статуса
			if ( ! simple )
				// Точная установка статуса
				db_set_w (hContact, modname, "Status", (WORD) status);
			else if ( ns == -1 || ns == ID_STATUS_OFFLINE || status != ID_STATUS_ONLINE )
				// Примерная установка статуса
				db_set_w (hContact, modname, "Status", (WORD) status);
		}
	}
}

void GetAvatarInfoThread(LPVOID param)
{
	// Задержка чтобы миранда успела "переварить" ответ
	Sleep( 500 );

	ContactData* data = (ContactData*)param;

	bool ret = false;
	ThreadEvent te = { CreateEvent( NULL, TRUE, FALSE, NULL ), data->cookie };
	PROTO_AVATAR_INFORMATION pai = { sizeof( PROTO_AVATAR_INFORMATION ), data->hContact };

	TCHAR szPath[ MAX_PATH ];
	GetAvatarCache( szPath );

	// Получение кешированного аватара
	DBVARIANT dbv = {};
	if ( ! db_get_ts( data->hContact, modname, "AvatarFile", &dbv ) )
	{
		lstrcat( szPath, dbv.ptszVal );

		if ( GetFileAttributes( szPath ) != INVALID_FILE_ATTRIBUTES )
		{
			ret = true;

			lstrcpyA( pai.filename, CT2A( szPath ) );

			// Определение формата
			LPCTSTR szExt = _tcsrchr( dbv.ptszVal, _T('.') );
			if ( ! szExt )
			{
				pai.format = PA_FORMAT_UNKNOWN;
			}
			else if ( lstrcmpi( szExt, _T(".png") ) == 0 || lstrcmpi( szExt, _T(".dat") ) == 0 )
			{
				pai.format = PA_FORMAT_PNG;
			}
			else if ( lstrcmpi( szExt, _T(".jpg") ) == 0 )
			{
				pai.format = PA_FORMAT_JPEG;
			}
			else if ( lstrcmpi( szExt, _T(".ico") ) == 0 )
			{
				pai.format = PA_FORMAT_ICON;
			}
			else if ( lstrcmpi( szExt, _T(".bmp") ) == 0 )
			{
				pai.format = PA_FORMAT_BMP;
			}
			else if ( lstrcmpi( szExt, _T(".gif") ) == 0 )
			{
				pai.format = PA_FORMAT_GIF;
			}
			else if ( lstrcmpi( szExt, _T(".swf") ) == 0 )
			{
				pai.format = PA_FORMAT_SWF;
			}
			else if ( lstrcmpi( szExt, _T(".xml") ) == 0 )
			{
				pai.format = PA_FORMAT_XML;
			}
			else
			{
				pai.format = PA_FORMAT_UNKNOWN;
			}
		}
		db_free( &dbv );
	}
	if ( ret )
	{
		ProtoBroadcastAck( modname, data->hContact, ACKTYPE_AVATAR,
			ACKRESULT_SUCCESS, &pai, 0 );
		LOG( "Returned cached avatar." );
	}
	else
	{
		bool bGroup = IsGroup( data->hContact );
		CString sNick = GetNick( data->hContact );
		if ( ! bGroup && ! sNick.IsEmpty() )
		{
			// Асинхронный сетевой запрос аватара
			ProtoBroadcastAck( modname, data->hContact, ACKTYPE_AVATAR,
				ACKRESULT_SENTREQUEST, &pai, 0 );

			// Отмечаемся в карте ожидающих нитей
			{
				CLock oLock( pluginGuard );
				pluginAvatarThreadMap.SetAt( data->hContact, te );
			}

			ret = pluginNetBIOS.AskAvatar( netbios_name( sNick, 0x03 ) );

			// Ожидание завершения (3 секунды)
			ret = ret && ( WaitForSingleObject( te.evt, 3000 ) == WAIT_OBJECT_0 );

			// Очистка карты ожидающих нитей
			{
				CLock oLock( pluginGuard );
				pluginAvatarThreadMap.Lookup( data->hContact, te );
				pluginAvatarThreadMap.RemoveKey( data->hContact );
			}
		}
		if ( ! ret )
		{
			ProtoBroadcastAck( modname, data->hContact, ACKTYPE_AVATAR,
				ACKRESULT_FAILED, &pai, 0 );
			LOG( "Get avatar failed" );
		}
	}

	if ( te.evt )
		CloseHandle( te.evt );

	delete data;
}

void SetContactAvatar(HANDLE hContact, LPCVOID pBuffer, DWORD nLength)
{
	if ( ! pluginInstalled )
		return;

	PROTO_AVATAR_INFORMATION pai = { sizeof( PROTO_AVATAR_INFORMATION ), hContact };

	CString sFilename, sNick = GetNick( hContact );
	if ( sNick.IsEmpty() || sNick.FindOneOf( _T("/\\*?:|\"<>%") ) != -1 )
	{
		// Получение старого имени
		DBVARIANT dbv = {};
		if ( ! db_get_ts( hContact, modname, "AvatarFile", &dbv ) )
		{
			sFilename = dbv.ptszVal;
			db_free( &dbv );
		}
		else
			// Генерация уникального имени
			sFilename.Format( _T("%08x_avt"), (DWORD)hContact );
	}
	else
		// Генерация уникального имени
		sFilename.Format( _T("%s_%08x_avt"), sNick, (DWORD)hContact );

	// Определение типа изображения
	if ( ! memcmp( pBuffer, "%PNG", 4 ) )
	{
		pai.format = PA_FORMAT_PNG;
		sFilename += _T(".png");
	}
	else if ( *(DWORD*)pBuffer == 0xE0FFD8FFul || *(DWORD*)pBuffer == 0xE1FFD8FFul )
	{
		pai.format = PA_FORMAT_JPEG;
		sFilename += _T(".jpg");
	}
	else if ( *(DWORD*)pBuffer == 0x00010000 )
	{
		pai.format = PA_FORMAT_ICON;
		sFilename += _T(".ico");
	}
	else if ( ! memcmp( pBuffer, "BM", 2 ) )
	{
		pai.format = PA_FORMAT_BMP;
		sFilename += _T(".bmp");
	}
	else if ( ! memcmp( pBuffer, "GIF", 3 ) )
	{
		pai.format = PA_FORMAT_GIF;
		sFilename += _T(".gif");
	}
	else if ( ! memcmp( pBuffer, "CWS", 3 ) )
	{
		pai.format = PA_FORMAT_SWF;
		sFilename += _T(".swf");
	}
	else if ( ! memcmp( pBuffer, "<?xml", 5 ) )
	{
		pai.format = PA_FORMAT_XML;
		sFilename += _T(".xml");
	}
	else
	{
		pai.format = PA_FORMAT_UNKNOWN;
		sFilename += _T(".dat");
	}

	// Сборка полного пути к файлу
	TCHAR szPath[ MAX_PATH ];
	GetAvatarCache( szPath );

	lstrcat( szPath, sFilename );

	CAtlFile oAvatarFile;
	if ( FAILED( oAvatarFile.Create( szPath, GENERIC_WRITE,
		FILE_SHARE_READ, CREATE_ALWAYS ) ) )
		// Файл не найден
		return;

	if ( FAILED( oAvatarFile.Write( pBuffer, nLength ) ) )
		// Ошибка записи в файл
		return;

	oAvatarFile.Close();

	db_set_ts( hContact, modname, "AvatarFile", sFilename );

	// Определение ожидающей нити по контакту
	bool ret = false;
	ThreadEvent te = {};
	{
		CLock oLock( pluginGuard );
		if ( pluginAvatarThreadMap.Lookup( hContact, te ) )
		{
			SetEvent( te.evt );
			ret = true;
		}
	}

	lstrcpyA( pai.filename, CT2A( szPath ) );

	if ( ret )
	{
		// Уведомление о новом аватаре
		ProtoBroadcastAck( modname, hContact, ACKTYPE_AVATAR, ACKRESULT_SUCCESS, &pai, 0 );
	}
	else
	{
		// Либо пропустили событие, либо его не было - ставим аватар принудительно
		if ( ServiceExists( MS_AV_SETAVATAR ) )
		{
			CallService( MS_AV_SETAVATAR, (WPARAM)hContact, (LPARAM)pai.filename );
		}
	}
}

ip GetContactIP (HANDLE hContact)
{
	ip addr = INADDR_NONE;
	bool got_nick = false, nick_invalid = false;
	DBVARIANT dbv = {};
	if ( hContact )
	{
		CString sNick = GetNick( hContact );
		if ( ! sNick.IsEmpty() )
		{
			got_nick = true;
		}
		else if ( ! db_get_ts( hContact, "CList", "MyHandle", &dbv ) )
		{
			nick_invalid = true;
			got_nick = true;
		}

		if ( got_nick )
		{
			// Восстановление ника
			if ( nick_invalid )
				SetNick( hContact, dbv.ptszVal );

			// Если не истекло время кеширования адреса, то из контакта 
			DWORD elapsed = GetElapsed (hContact, "IPTime");
			if (elapsed <= MAX_TRUSTED_IP_TIME)
				addr = db_get_dw (hContact, modname, "IP", 0);
			if (addr == INADDR_NONE) {
				// Разбор DNS-адреса
				CString name (dbv.pszVal);
				addr = ResolveToIP (name);
				if (addr != INADDR_NONE)
					SetElapsed (hContact, "IPTime");
			}
			if (addr != INADDR_NONE)
				SetContactIP (hContact, addr);
			db_free (&dbv);
		}
	}
	return addr;
}

void SetContactIP(HANDLE hContact, ip addr)
{
	if ( ! pluginInstalled )
		return;

	if ( hContact )
	{
		db_set_dw	(hContact, modname, "IP", addr);
		db_set_dw	(hContact, modname, "RealIP", addr);
	}
}

bool IsGroup(HANDLE hContact)
{
	return ( db_get_b( hContact, modname, "Group", 0u ) != 0u );
}

void SetGroup(HANDLE hContact, bool bGroup)
{
	db_set_b( hContact, modname, "Group", ( bGroup ? 1u : 0u ) );
}

bool IsLegacyOnline(HANDLE hContact)
{
	return ( db_get_b( hContact, modname, "Check00ForOnline", 0u ) != 0u );
}

void SetLegacyOnline(HANDLE hContact, bool bOnline)
{
	db_set_b( hContact, modname, "Check00ForOnline", ( bOnline ? 1u : 0u ) );
}

bool SendContactMessage(HANDLE hContact, LPCTSTR msg, DWORD& err)
{
	// Получение метода посылки
	BYTE method = (BYTE)db_get_b( NULL, modname, "SendMethod", 0 );
	switch ( method )
	{
		case 0:
			return pluginMailslot.SendMailslotMessage( hContact, msg, err );
		case 1:
			return pluginNetBIOS.SendNetBIOSMessage( hContact, msg, err );
		case 2:
			return pluginMessenger.SendMessengerMessage( hContact, msg, err );
		default:
			return false;
	}
}

void SendMsgThread(LPVOID param)
{
	SendMsgData* data = (SendMsgData*)param;

	DWORD dwLastError = 0;
	if ( SendContactMessage( data->hContact, data->text, dwLastError ) )
	{
		ProtoBroadcastAck ( modname, data->hContact, ACKTYPE_MESSAGE,
			ACKRESULT_SUCCESS, data->cookie, 0 );
	}
	else
	{
		// Уведомление об ошибке
		CString msg, buf;
		GetErrorMessage (dwLastError, buf);
		msg.Format( _T("%s\r\n%s"), TranslateT ("Cannot send message"), (LPCTSTR)buf);
		ProtoBroadcastAck (modname, data->hContact, ACKTYPE_MESSAGE,
			ACKRESULT_FAILED, data->cookie, (LPARAM)(LPCTSTR)msg );

		// И заодно попапом
		WarningBox( data->hContact, dwLastError, TranslateT("Cannot send message") );
	}

	delete data;
}

bool IsItMe(LPCTSTR name)
{
	return ! pluginMachineName.CompareNoCase( name ) ||
		! pluginUserName.CompareNoCase( name );
}

void EnumWorkgroups(CAtlList< CString >& lst, LPNETRESOURCE hRoot)
{
	HANDLE hEnum = NULL;
	DWORD res = WNetOpenEnum( RESOURCE_GLOBALNET, RESOURCETYPE_ANY,
		RESOURCEUSAGE_CONTAINER, hRoot, &hEnum );
	if ( res == NO_ERROR )
	{
		for (;;)
		{
			DWORD cCount = 1;
			DWORD BufferSize = 4096;
			char* Buffer = (char*)mir_alloc( BufferSize );
			if ( ! Buffer )
				break;
			res = WNetEnumResource( hEnum, &cCount, Buffer, &BufferSize );
			if ( res == NO_ERROR )
			{
				LPNETRESOURCE lpnr = (LPNETRESOURCE)Buffer;
				if ( lpnr->dwDisplayType == RESOURCEDISPLAYTYPE_DOMAIN )
				{
					CharUpper ( lpnr->lpRemoteName );
					lst.AddTail( lpnr->lpRemoteName );
				}
				else if ( ( lpnr->dwUsage & 0xffff ) == RESOURCEUSAGE_CONTAINER )
				{
					EnumWorkgroups( lst, lpnr );
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

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID /*lpReserved*/)
{
	pluginModule = hModule;
	return TRUE;
}
