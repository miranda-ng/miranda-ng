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

// Данные используемые плагином в базе данных Miranda IM
//
// Собственные данные плагина:
// Nick						String	- свое название машины (авт.обновляется)
// User						String	- свое название пользователя (авт.обновляется)
// Workgroup				String	- свое название домена или рабочей группы машины (авт.обновляется)
// Auto-answer				BYTE	- флаг авто-ответа (по-умолчанию: 0)
// Filter-dups				BYTE	- флаг фильтрации дубликатов (по-умолчанию: 1)
// SendMethod   			BYTE    - способ отсылки сообщений: 0 - mailslot, 1 - NetBIOS, 2 - Messenger  (по-умолчанию: 0)
// RegisterNick 			BYTE    - флаг регистрации NetBIOS имени Nick<01> и Nick<03> (по-умолчанию: 1)
// RegisterUser 			BYTE    - флаг регистрации NetBIOS имени User<03> (по-умолчанию: 1)
// RegisterStatus 			BYTE	- флаг регистрации NetBIOS имени MNS_STATUS<ab> (по-умолчанию: 1)
// Check00ForOnline			BYTE	- флаг дополнительной проверки NetBIOS имён Nick<00> на онлайн статус, для всех контактов (по умолчанию: 0) (переименовано из AlwaysCheck00ForOnline)
// AvatarFile				String	- путь к файлу аватара
//
// Данные плагина для контактов:
// Nick						String	- название машины
// IP						DWORD	- адрес машины
// RealIP					DWORD	- адрес машины (совпадает с IP)
// IPTimeL					DWORD	- время последнего ресолвинга имени в адрес
// IPTimeH					DWORD
// LastSeenL				DWORD	- время последней проверка контакта
// LastSeenH				DWORD
// PingCounter				WORD	- счётчик посланных пингов без ответа (больше не используется)
// Status					WORD	- статус контакта
// About					String	- комментарий
// AlwaysOnline				BYTE	- флаг отображения контакта в online-состоянии всегда
// Check00ForOnline 		BYTE	- флаг дополнительной проверки NetBIOS имени Nick<00> на онлайн статус (по умолчанию: 0)
// AvatarFile				String	- путь к файлу аватара
// Group					BYTE	- 1/0 - это групповой контакт
//
// Неиспользуемые настройки:
// ChatRoom					BYTE	- 1/0 - это чат
// ChatRoomID				String	- идентификатор чата, обычно имя рабочей группы
//
// Данные CList (главного списка контаков) для контактов:
// MyHandle					String	- название контакта (ред. пользователем)
// NotOnList				BYTE	- флаг временного контакта
// Hidden					BYTE	- флаг скрытого контакта
//
// Данные Icons (иконки статуса) для плагина:
// WinPopup Protocol40071	String	- путь к иконке статуса
// WinPopup Protocol40072	String	- путь к иконке статуса
// WinPopup Protocol40073	String	- путь к иконке статуса
// WinPopup Protocol40074	String	- путь к иконке статуса
// WinPopup Protocol40075	String	- путь к иконке статуса
// WinPopup Protocol40076	String	- путь к иконке статуса
// WinPopup Protocol40077	String	- путь к иконке статуса
// WinPopup Protocol40078	String	- путь к иконке статуса
// WinPopup Protocol40079	String	- путь к иконке статуса
// WinPopup Protocol40080	String	- путь к иконке статуса

// Название плагина
#define modname				"WinPopup Protocol"
#define modname_t			_T( modname )
// Краткое название плагина
#define modtitle			"WinPopup"
#define modtitle_t			_T( modtitle )
// "Не могу остановить Службу Сообщений"
#define T_STOP_ERROR		TranslateT("Cannot stop Messenger service")
// "Не могу запустить Службу Сообщений"
#define T_START_ERROR		TranslateT("Cannot start Messenger service")
// "Не могу включить Службу Сообщений"
#define T_ENABLE_ERROR		TranslateT("Cannot enable Messenger service")
// "Не могу создать приёмный мейлслот"
#define T_CREATE_ERROR		TranslateT("Cannot create receiving mailslot")
// "Ошибка"
#define T_ERROR				TranslateT("Error")
// Название сервиса Messenger
#define MESSENGER			_T("Messenger")
// Название мейлслота Messenger
#define MESSENGER_MAIL		_T("messngr")
// Время доверия IP адресу (3 часа)
#define MAX_TRUSTED_IP_TIME	3*60*60
// Минимальнок время обновления контакта (раз в 30 с)
#define MIN_PING_INTERVAL	30

typedef struct _ContactData
{
	HANDLE			cookie;		// Идентификатор запроса ядра миранды для ответа
	HANDLE			hContact;	// Хэндлер контакта
} ContactData;

typedef struct _SendMsgData
{
	HANDLE			cookie;		// Идентификатор запроса ядра миранды для ответа
	HANDLE			hContact;	// Хэндлер контакта
	CString			text;
} SendMsgData;

typedef struct _ThreadEvent
{
	HANDLE			evt;		// Хэндлер события
	HANDLE			cookie;		// Идентификатор запроса ядра миранды для ответа
} ThreadEvent;

typedef CAtlMap < WPARAM, CString > CIntStrMap;
typedef CAtlMap < HANDLE, ThreadEvent > CThreadContactMap;

// Класс для конвертации ANSI строки в OEM и последующего хранения
class COemString
{
public:
	COemString(LPCTSTR ansi) :
		len( (size_t)lstrlen( ansi ) + 1 ),
		pos( 0 )
	{
		oem = (LPSTR)mir_alloc( len );
		CharToOemBuff( ansi, oem, (DWORD)len );
	}

	~COemString()
	{
		mir_free( oem );
	}

	inline operator LPCSTR() const
	{
		return oem + pos;
	}

	// Длина строки (без null)
	inline int GetLength() const
	{
		return (int)( len - pos - 1 );
	}

	// "Отрезание" от начала строки
	inline void CutFromStart(int n)
	{
		if ( GetLength() > n )
			pos += n;
		else
			pos = len - 1;
	}

protected:
	size_t	len;
	size_t	pos;
	LPSTR	oem;
};

// Класс для конвертации OEM строки в ANSI и последующего хранения
class CAnsiString
{
public:
	CAnsiString(LPCSTR oem) :
		len( lstrlenA( oem ) + 1 ),
		pos( 0 )
	{
		ansi = (LPTSTR)mir_alloc( len * sizeof( TCHAR ) );
		OemToCharBuff( oem, ansi, (DWORD)len );
	}
	~CAnsiString()
	{
		mir_free( ansi );
	}
	inline operator LPCTSTR() const
	{
		return ansi + pos;
	}
	// Длина строки (без null)
	inline int GetLength() const
	{
		return len - pos - 1;
	}
	// "Отрезание" от начала строки
	inline void CutFromStart(int n)
	{
		if ( len - pos - 1 > n )
			pos += n;
		else
			pos = len - 1;
	}
protected:
	int		len;
	int		pos;
	LPTSTR	ansi;
};

extern volatile WPARAM		pluginRequestedStatus;	// Требуемый статус
extern volatile WPARAM		pluginCurrentStatus;	// Текущий статус
extern CIntStrMap			pluginStatusMessage;	// Карта статусных сообщений
extern CString				pluginMachineName;		// Имя компьютера
extern CString				pluginUserName;			// Имя пользователя
extern CString				pluginDomainName;		// Имя домена или группы
extern HMODULE				pluginModule;			// Хэндлер на модуль
extern volatile bool		pluginInstalled;		// Флаг разрешения работы плагина,
													// сбрасывается в false при деинсталляции
													// или при выключении
extern volatile bool		pluginInitialized;		// Флаг завершения инициализации плагина
extern HANDLE				pluginNetLibUser;		// Хэндлер NetLib
extern HANDLE				pluginInternalState;	// Хэндлер события (сигнальное состояние - плагин остановлен)
extern bool					pluginChatEnabled;		// Используем Chat-плагин?
extern OSVERSIONINFO		pluginOS;				// Версия системы

// Получение ника контакта
CString GetNick(HANDLE hContact);
// Установка ника контакта
void SetNick(HANDLE hContact, LPCTSTR szNick);
// Вывод в стандартный для миранды лог NetLib
int LOG(const char *fmt,...);
// Получение пути где хранятся аватары
void GetAvatarCache(LPTSTR szPath);
// Получение "уникального" числа
HANDLE GenerateCookie();
// Win32 API версия функции time() (для освобождения от зависимости CRT)
DWORD time();
// Определение принадлежности контакта плагину
bool IsMyContact(HANDLE hContact);
// Установка статуса плагина
void SetPluginStatus(WPARAM status);
// Включение всех подсистем плагина
bool InternalStartup();
// Выключение всех подсистем плагина
void InternalShutdown();
// Перевод плагина в Online
void GotoOnline();
// Перевод плагина в Offline
void GotoOffline();
// Рабочая нить асинхронного перехода плагина в Online
void GotoOnlineTread(LPVOID status);
// Рабочая нить асинхронного перехода плагина в Offline
void GotoOfflineTread(LPVOID status);
// Рабочая нить асинхронного получения эвей-сообщения контакта
void GetAwayMsgThread(LPVOID param);
// Рабочая нить асинхронного возвращения информации о контакте
void GetInfoThread(LPVOID param);
// Автоответчик статусного сообщения
void Autoanswer(HANDLE hContact);
// Приём сообщения, проверка на дубликаты, очистка и отправка в базу Miranda IM
void ReceiveContactMessage(LPCTSTR msg_from, LPCTSTR msg_to, LPCTSTR msg_text, int msg_len);
// Добавление контакта по имени (с нахождением его IP-адреса)
HANDLE AddToListByName (const CString& sName, WPARAM flags, LPCTSTR notes, bool bInteractive, bool bGroup);
// Получение "nameL" | ("nameH" << 32) контакта
DWORD GetElapsed(HANDLE hContact, const char* name);
// Установка "LastSeen" контакта равным текущему времени
void SetElapsed(HANDLE hContact, const char* name);
// Поиск контакта по "IP"
HANDLE GetContact(ip addr);
// Поиск контакта по "Nick"
HANDLE GetContact(LPCTSTR name);
// Установка статуса контакта (simple == true - установка только online/offline)
void SetContactStatus(HANDLE hContact, int status, bool simple);
// Установка эвей-текста контакта
void SetContactAway(HANDLE hContact, LPCSTR away);
// Установка аватара контакта
void SetContactAvatar(HANDLE hContact, LPCVOID pBuffer, DWORD nLength);
// Получение IP-адреса контакта (с проверкой и ресолвингом, если требуется)
ip GetContactIP(HANDLE hContact);
// Установка "IP" контакта
void SetContactIP(HANDLE hContact, ip addr);
// Это групповой контакт?
bool IsGroup(HANDLE hContact);
// Установка группового контакта
void SetGroup(HANDLE hContact, bool bGroup);
// Используется старый метод онлайн-проверки?
bool IsLegacyOnline(HANDLE hContact);
// Установка старого метода онлайн-проверки
void SetLegacyOnline(HANDLE hContact, bool bOnline);
// Посылка сообщения
bool SendContactMessage(HANDLE hContact, LPCTSTR msg, DWORD& err);
// Рабочая нить асинхронной посылки сообщения
void SendMsgThread(LPVOID param);
// Рабочая нить асинхронного запроса аватара
void GetAvatarInfoThread(LPVOID param);
// Моё имя?
bool IsItMe(LPCTSTR name);
// Нормализация разбиения сообщения на строки
void Normalize(CString& msg);
// Получение списка рабочих групп / доменов
void EnumWorkgroups(CAtlList< CString >& lst, LPNETRESOURCE hRoot = NULL);

// Макрос организации цикла перебора своих контактов в списке контактов
#define FOR_EACH_CONTACT(h) \
	HANDLE h = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 ); \
	for ( ; h != NULL; \
	h = (HANDLE)CallService( MS_DB_CONTACT_FINDNEXT, (WPARAM)h, 0 ) ) \
		if ( IsMyContact( h ) )

// Макрос для печати названия статуса (отладочный)
#define STATUS2TEXT(s) \
	((((s) == ID_STATUS_OFFLINE) ? "Offline" : \
	(((s) == ID_STATUS_ONLINE) ? "Online" : \
	(((s) == ID_STATUS_AWAY) ? "Away" : \
	(((s) == ID_STATUS_DND) ? "DND" : \
	(((s) == ID_STATUS_NA) ? "NA" : \
	(((s) == ID_STATUS_OCCUPIED) ? "Occupied" : \
	(((s) == ID_STATUS_FREECHAT) ? "Free to chat" : \
	(((s) == ID_STATUS_INVISIBLE) ? "Invisible" : \
	(((s) == ID_STATUS_ONTHEPHONE) ? "On the phone" : \
	(((s) == ID_STATUS_OUTTOLUNCH) ? "Out to lunch" : \
	(((s) == ID_STATUS_IDLE) ? "Idle" : \
	(((s) == (ID_STATUS_CONNECTING + 0)) ? "Connecting 1" : \
	(((s) == (ID_STATUS_CONNECTING + 1)) ? "Connecting 2" : \
	(((s) <  (ID_STATUS_CONNECTING + MAX_CONNECT_RETRIES)) ? "Connecting > 2" : \
	"Unknown")))))))))))))))
