/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#pragma once

namespace DB {

/**
 * This namespace contains all functions used to access
 * or modify contacts in the database.
 **/
namespace Contact {

	uint32_t   WhenAdded(uint32_t dwUIN, LPCSTR szBaseProto);
} /* namespace Contact */

namespace Module {
	void	Delete(MCONTACT hContact, LPCSTR pszModule);
	bool  IsEmpty(MCONTACT hContact, LPCSTR pszModule);
	bool  IsMeta(LPCSTR pszModule);
	bool  IsMetaAndScan(LPCSTR pszModule);

} /* namespace Module */

/**
 * This namespace defines all functions used to read and write settings from the database.
 **/
namespace Setting {

	uint8_t	Get(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv, const uint8_t nType);
	static FORCEINLINE uint8_t GetAsIs(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return Get(hContact, pszModule, pszSetting, dbv, 0); }
	static FORCEINLINE uint8_t GetAString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return Get(hContact, pszModule, pszSetting, dbv, DBVT_ASCIIZ); }

	LPSTR	GetAString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);
	static FORCEINLINE uint8_t	GetWString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return Get(hContact, pszModule, pszSetting, dbv, DBVT_WCHAR); }

	LPWSTR GetWString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);
	static FORCEINLINE uint8_t	GetUString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return Get(hContact, pszModule, pszSetting, dbv, DBVT_UTF8); }

	uint8_t GetEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv, const uint8_t nType);
	static FORCEINLINE uint8_t GetAsIsEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return GetEx(hContact, pszModule, szProto, pszSetting, dbv, 0); }
	static FORCEINLINE LPSTR GetAStringEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR szProto, LPCSTR pszSetting)
	{ DBVARIANT dbv; return (!GetEx(hContact, pszModule, szProto, pszSetting, &dbv, DBVT_ASCIIZ) && dbv.type == DBVT_ASCIIZ) ? dbv.pszVal : nullptr; }
	static FORCEINLINE LPWSTR GetWStringEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR szProto, LPCSTR pszSetting)
	{ DBVARIANT dbv; return (!GetEx(hContact, pszModule, szProto, pszSetting, &dbv, DBVT_WCHAR) && dbv.type == DBVT_WCHAR) ? dbv.pwszVal : nullptr; }
	static FORCEINLINE LPSTR GetUStringEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR szProto, LPCSTR pszSetting)
	{ DBVARIANT dbv; return (!GetEx(hContact, pszModule, szProto, pszSetting, &dbv, DBVT_UTF8) && dbv.type == DBVT_UTF8) ? dbv.pszVal : nullptr; }

	uint16_t GetCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR szSubModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv, const uint8_t nType);
	static FORCEINLINE uint16_t	GetAsIsCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR szSubModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return GetCtrl(hContact, pszModule, szSubModule, szProto, pszSetting, dbv, 0); }
	static FORCEINLINE uint16_t	GetAStringCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR szSubModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return GetCtrl(hContact, pszModule, szSubModule, szProto, pszSetting, dbv, DBVT_ASCIIZ); }
	static FORCEINLINE uint16_t	GetWStringCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR szSubModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return GetCtrl(hContact, pszModule, szSubModule, szProto, pszSetting, dbv, DBVT_WCHAR); }
	static FORCEINLINE uint16_t	GetUStringCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR szSubModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return GetCtrl(hContact, pszModule, szSubModule, szProto, pszSetting, dbv, DBVT_UTF8); }

	#define GetTString     GetWString
	#define GetTStringEx   GetWStringEx
	#define GetTStringCtrl GetWStringCtrl
	#define WriteTString   WriteWString

	/**
	 * misc operations
	 **/
	uint8_t	Exists(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);
	void	DeleteArray(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszFormat, int iStart);

} /* namespace Setting */

namespace Variant {
	uint8_t	ConvertString(DBVARIANT* dbv, const uint8_t destType);
	uint8_t	dbv2String(DBVARIANT* dbv, const uint8_t destType);
} /* namespace Variant */

namespace Event   {
	HANDLE FindLast(MCONTACT hContact);
	bool   GetInfo(MEVENT hEvent, DBEVENTINFO *dbei);
	bool   GetInfoWithData(MEVENT hEvent, DBEVENTINFO *dbei);
	uint32_t  GetTime(MEVENT hEvent);
	bool   Exists(MCONTACT hContact, MEVENT& hDbExistingEvent, DBEVENTINFO *dbei);
} /* namespace Events */

/**
 * enumerating
 **/

class CEnumList : public ::LIST<CHAR>
{
	static int EnumProc        (LPCSTR pszName, void *param);
	static int EnumSettingsProc(LPCSTR pszName, void *param);

	static int CompareProc     (LPCSTR p1, LPCSTR p2);
	
public:
	CEnumList();
	~CEnumList();

	LPSTR	Insert(LPCSTR str);

	INT_PTR	EnumModules();
	INT_PTR	EnumSettings(MCONTACT hContact, LPCSTR pszModule);
};

} /* namespace DB */