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

const DWORD		SMB_MAGIC			= 0x424d53ff;		// 0xff 'S' 'M' 'B'
const WORD		SM_MAGIC			= 0x4d53;			// 'S' 'M'

// NetBIOS datagrams:
// (obsolete)
const UCHAR		SM_CHANGESTATUS		= 0x01;
// Status request:					'S' 'M' 0x02
const UCHAR		SM_GETSTATUS		= 0x02;
// Status answer:					'S' 'M' 0x03 status(i32)
const UCHAR		SM_SENDSTATUS		= 0x03;
// Away message request:			'S' 'M' 0x04
const UCHAR		SM_GETAWAYMESSAGE	= 0x04;
// Away message answer:				'S' 'M' 0x05 0x00(i32) message(ASCII)
const UCHAR		SM_SENDAWAYMESSAGE	= 0x05;
// Avatar request:					'S' 'M' 0x06
const UCHAR		SM_GETAVATAR		= 0x06;
// Avatar answer:					'S' 'M' 0x07 avatar
const UCHAR		SM_SENDAVATAR		= 0x07;

const LPCTSTR	MNS_STATUS			= _T("MNS_STATUS");

#define MAX_AVATAR_SIZE	65000	// Максимальный размер файла аватара (байт)

#pragma pack (push, 1)

typedef struct _FIND_NAME_BLOCK
{
	FIND_NAME_HEADER fnb_header;
	FIND_NAME_BUFFER fnb_Names [256];
} FIND_NAME_BLOCK, *PFIND_NAME_BLOCK;

typedef struct _ADAPTER_STATUS_BLOCK
{
	ADAPTER_STATUS asb_header;
	NAME_BUFFER asb_Names [NCBNAMSZ];
} ADAPTER_STATUS_BLOCK, *PADAPTER_STATUS_BLOCK;

#pragma pack (pop)

class netbios
{
public:
	netbios();
	~netbios();

	bool Create (BOOL registration);
	void AskForDestroy();					// Для ускорения
	void Destroy ();
	operator bool() const;

	bool SendNetBIOSMessage (HANDLE hContact, LPCTSTR msg /* ANSI */, DWORD& err);
	ip FindNameIP (LPCTSTR szName /* ANSI */, UCHAR type = 3);
	void GetRegisteredNames (netbios_name_list& names);
	bool GetNames (netbios_name_list& names, LPCTSTR name /* ANSI */, bool bGroup);

	// Получение имени из списка по указанному имени
	netbios_name* GetName (const netbios_name& nname);

	// Поиск номера адаптера через который можно достучаться до указанного имени
	bool FindNameLana (const netbios_name& nname, UCHAR& lana);

	bool GetMAC (UCHAR lana, CString& mac);
	UCHAR FindName (const netbios_name& nname, UCHAR lana, FIND_NAME_BLOCK& fn);
	UCHAR GetAdapterStatus (const netbios_name& nname, UCHAR lana, ADAPTER_STATUS_BLOCK& astat);
	UCHAR EnumLanas (LANA_ENUM& le);
	UCHAR ResetLana (UCHAR lana);
	UCHAR Hangup (UCHAR lana, UCHAR lsn);
	UCHAR Send (UCHAR lana, UCHAR lsn, unsigned char* data, WORD length);
	UCHAR Recv (UCHAR lana, UCHAR lsn, unsigned char* data, WORD& length);
	UCHAR Stat (const netbios_name& nname, SESSION_INFO_BLOCK* psibSession);
	UCHAR Listen (const netbios_name& nname, UCHAR& lsn);
	UCHAR AddName (netbios_name& nname);
	UCHAR DeleteName (const netbios_name& nname);
	UCHAR SendDatagram (const netbios_name& nname_from, const netbios_name& nname_to, unsigned char* data, WORD length);
	UCHAR RecvDatagram (netbios_name& nname_from, const netbios_name& nname_to, unsigned char* data, WORD& length);
	UCHAR Call (UCHAR lana, const netbios_name& nname_from, const netbios_name& nname_to, UCHAR& lsn);
	unsigned char* SetSMBHeaderCommand (unsigned char* szHeader, BYTE iCommandCode, size_t iBufferLen);

	// Запрос эвей-сообщения
	bool AskAway(const netbios_name& nname_to);
	// Отправка эвей-сообщения
	bool SendAway(netbios_name& nname_from, const netbios_name& nname_to);
	// Запрос статуса
	bool AskStatus(const netbios_name& nname_to);
	// Отправка статуса
	bool SendStatus(netbios_name& nname_from, const netbios_name& nname_to);
	// Отправка статуса на всех адаптерах (от COMPUTER<03> U на MNS_STATUS<ab> G)
	bool BroadcastStatus();
	// Запрос аватара
	bool AskAvatar(const netbios_name& nname_to);
	// Отправка аватара
	bool SendAvatar(netbios_name& nname_from, const netbios_name& nname_to);

protected:
	bool					m_initialized;	// Флаг инициализации подсистемы NetBIOS
	netbios_name_array		m_names;		// Все регистрируемые NetBIOS-имена для слушателя
	LANA_ENUM				m_le;			// Список адаптеров
	CComAutoCriticalSection	m_csData;		// Защита данных
	CComAutoCriticalSection	m_csNetBIOS;	// Синхронизация NetBIOS

	bool Register ();						// Регистрация всех NetBIOS-имен
	void Deregister ();						// Дерегистрация всех NetBIOS-имен
};

extern netbios pluginNetBIOS;				// Прием/отправка сообщений через NetBIOS

LPCSTR GetNetbiosCommand(UCHAR command);
LPCSTR GetNetbiosError(UCHAR err);
UCHAR NetbiosEx(NCB* pNCB);
