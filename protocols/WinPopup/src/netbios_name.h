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

#define SMB_ANY_NAME		_T("*               ")
#define FACILITY_NETBIOS	(0x0f0f)
#define SMB_HEADER_SIZE		(32)

#pragma pack (push, 1)

typedef struct SESSION_INFO_BLOCK
{
	SESSION_HEADER sib_header;
	SESSION_BUFFER sib_Names[ NCBNAMSZ ];
} SESSION_INFO_BLOCK, *PSESSION_INFO_BLOCK;

#pragma pack (pop)

class netbios_name
{
public:
	netbios_name(LPCTSTR n = SMB_ANY_NAME /* ANSI */, UCHAR type = 0, bool group = false, UCHAR lana = 0);
	netbios_name(const NAME_BUFFER& n, UCHAR lana);
	netbios_name& operator=(const netbios_name& n);
	netbios_name& operator=(const UCHAR* n);
	// Сравнение (производиться только по имени, номеру имени, типу имени)
	bool operator==(const NAME_BUFFER& n) const;
	// Сравнение (производиться только по имени, номеру имени, типу имени)
	bool operator!=(const NAME_BUFFER& n) const;
	// Сравнение (производиться только по имени, номеру имени, типу имени, адаптеру)
	bool operator==(const netbios_name& n) const;
	// Сравнение (производиться только по имени, номеру имени, типу имени, адаптеру)
	bool operator!=(const netbios_name& n) const;
	// Регистрация имени
	bool Register();
	// Для ускорения
	void AskForDestroy();
	// Дерегистрация имени
	void Destroy();
	// Обратное преобразование NetBIOS-имени из OEM в ANSI в виде: NAME <TYPE>
	CStringA  GetANSIFullName() const;

	CStringA		original;	// Полное имя (в ANSI кодировке)
	NAME_BUFFER		netbiosed;	// NetBIOS имя (в OEM кодировке):
								//	typedef struct _NAME_BUFFER {
								//		UCHAR   name[NCBNAMSZ];
								//		UCHAR   name_num;
								//		UCHAR   name_flags;
								//	}	NAME_BUFFER, *PNAME_BUFFER;

	// Получение атрибутов имени:

	UCHAR GetType() const;
	bool IsGroupName() const;
	bool IsRegistered() const;
	bool IsDuplicated() const;
	bool IsError() const;
	bool IsOwnName() const;
	UCHAR GetLana() const;
	size_t GetLength() const;	// Длина имени NetBIOS

protected:
	bool	m_managed;			// Флаг управляемого имени
	bool	m_registered;		// Флаг успешной регистрации имени
	bool	m_duplicated;		// Флаг существования имени до регистрации
	bool	m_error;			// Флаг ошибки прослушивания имени
	UCHAR	m_lana;				// Номер адаптера
	HANDLE	m_listener;			// Хэндлер рабочего потока-слушателя сессий
	HANDLE	m_dgreceiver;		// Хэндлер рабочего потока-слушателя датаграмм
	HANDLE	m_term;				// Хэндлер события останова рабочего потока-слушателя

	// Обратное преобразование NetBIOS-имени из OEM в ANSI
	CStringA  GetANSIName() const;

	bool GetRealSender(UCHAR lsn, CStringA& sRealFrom) const;
	UCHAR AddName();
	UCHAR DeleteName();

	// Ожидание начала сессии
	void Listener();
	static void ListenerThread(LPVOID param);

	// Ожидание датаграммы
	void DatagramReceiver();
	static void DatagramReceiverThread(LPVOID param);

	// Обработка сессии (из Listener)
	void Receiver(UCHAR lsn);
	static void ReceiverThread(LPVOID param);
};

typedef CAtlArray <netbios_name*> netbios_name_array;
typedef CAtlList  <netbios_name>  netbios_name_list;

/*
Name                Number(h)  Type  Usage
--------------------------------------------------------------------------
<computername>         00       U    Workstation Service
<computername>         01       U    Messenger Service
<\\--__MSBROWSE__>     01       G    Master Browser
<computername>         03       U    Messenger Service
<computername>         06       U    RAS Server Service
<computername>         1F       U    NetDDE Service
<computername>         20       U    File Server Service
<computername>         21       U    RAS Client Service
<computername>         22       U    Microsoft Exchange Interchange(MSMail
Connector)
<computername>         23       U    Microsoft Exchange Store
<computername>         24       U    Microsoft Exchange Directory
<computername>         30       U    Modem Sharing Server Service
<computername>         31       U    Modem Sharing Client Service
<computername>         43       U    SMS Clients Remote Control
<computername>         44       U    SMS Administrators Remote Control
Tool
<computername>         45       U    SMS Clients Remote Chat
<computername>         46       U    SMS Clients Remote Transfer
<computername>         4C       U    DEC Pathworks TCPIP service on
Windows NT
<computername>         42       U    mccaffee anti-virus
<computername>         52       U    DEC Pathworks TCPIP service on
Windows NT
<computername>         87       U    Microsoft Exchange MTA
<computername>         6A       U    Microsoft Exchange IMC
<computername>         BE       U    Network Monitor Agent
<computername>         BF       U    Network Monitor Application
<username>             03       U    Messenger Service
<domain>               00       G    Domain Name
<domain>               1B       U    Domain Master Browser
<domain>               1C       G    Domain Controllers
<domain>               1D       U    Master Browser
<domain>               1E       G    Browser Service Elections
<INet~Services>        1C       G    IIS
<IS~computer name>     00       U    IIS
<computername>         [2B]     U    Lotus Notes Server Service
IRISMULTICAST          [2F]     G    Lotus Notes
IRISNAMESERVER         [33]     G    Lotus Notes
Forte_$ND800ZA         [20]     U    DCA IrmaLan Gateway Server Service 
*/
