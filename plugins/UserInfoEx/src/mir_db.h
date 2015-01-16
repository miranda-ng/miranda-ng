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
	__forceinline LPTSTR DisplayName(MCONTACT hContact)
	{	return pcli->pfnGetContactDisplayName(hContact, 0);
	}

	LPSTR   Proto(MCONTACT hContact);

	INT_PTR GetCount();

	MCONTACT Add();
	BYTE    Delete(MCONTACT hContact);

	DWORD   WhenAdded(DWORD dwUIN, LPCSTR szBaseProto);

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

	BYTE	Get(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv, const BYTE nType);
	static FORCEINLINE BYTE GetAsIs(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return Get(hContact, pszModule, pszSetting, dbv, 0); }
	static FORCEINLINE BYTE GetAString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return Get(hContact, pszModule, pszSetting, dbv, DBVT_ASCIIZ); }

	LPSTR	GetAString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);
	static FORCEINLINE BYTE	GetWString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return Get(hContact, pszModule, pszSetting, dbv, DBVT_WCHAR); }

	LPWSTR GetWString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);
	static FORCEINLINE BYTE	GetUString(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return Get(hContact, pszModule, pszSetting, dbv, DBVT_UTF8); }

	BYTE GetEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv, const BYTE nType);
	static FORCEINLINE BYTE GetAsIsEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return GetEx(hContact, pszModule, szProto, pszSetting, dbv, 0); }
	static FORCEINLINE LPSTR GetAStringEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR szProto, LPCSTR pszSetting)
	{ DBVARIANT dbv; return (!GetEx(hContact, pszModule, szProto, pszSetting, &dbv, DBVT_ASCIIZ) && dbv.type == DBVT_ASCIIZ) ? dbv.pszVal : NULL; }
	static FORCEINLINE LPWSTR GetWStringEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR szProto, LPCSTR pszSetting)
	{ DBVARIANT dbv; return (!GetEx(hContact, pszModule, szProto, pszSetting, &dbv, DBVT_WCHAR) && dbv.type == DBVT_WCHAR) ? dbv.pwszVal : NULL; }
	static FORCEINLINE LPSTR GetUStringEx(MCONTACT hContact, LPCSTR pszModule, LPCSTR szProto, LPCSTR pszSetting)
	{ DBVARIANT dbv; return (!GetEx(hContact, pszModule, szProto, pszSetting, &dbv, DBVT_UTF8) && dbv.type == DBVT_UTF8) ? dbv.pszVal : NULL; }

	WORD GetCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR szSubModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv, const BYTE nType);
	static FORCEINLINE WORD	GetAsIsCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR szSubModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return GetCtrl(hContact, pszModule, szSubModule, szProto, pszSetting, dbv, 0); }
	static FORCEINLINE WORD	GetAStringCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR szSubModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return GetCtrl(hContact, pszModule, szSubModule, szProto, pszSetting, dbv, DBVT_ASCIIZ); }
	static FORCEINLINE WORD	GetWStringCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR szSubModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return GetCtrl(hContact, pszModule, szSubModule, szProto, pszSetting, dbv, DBVT_WCHAR); }
	static FORCEINLINE WORD	GetUStringCtrl(MCONTACT hContact, LPCSTR pszModule, LPCSTR szSubModule, LPCSTR szProto, LPCSTR pszSetting, DBVARIANT *dbv)
	{ return GetCtrl(hContact, pszModule, szSubModule, szProto, pszSetting, dbv, DBVT_UTF8); }

	#define GetTString     GetWString
	#define GetTStringEx   GetWStringEx
	#define GetTStringCtrl GetWStringCtrl
	#define WriteTString   WriteWString

	/**
	 * misc operations
	 **/
	BYTE	Exists(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszSetting);
	void	DeleteArray(MCONTACT hContact, LPCSTR pszModule, LPCSTR pszFormat, int iStart);

} /* namespace Setting */

namespace Variant {
	BYTE	ConvertString(DBVARIANT* dbv, const BYTE destType);
	BYTE	dbv2String(DBVARIANT* dbv, const BYTE destType);
} /* namespace Variant */

namespace Event   {
	HANDLE FindLast(MCONTACT hContact);
	bool   GetInfo(MEVENT hEvent, DBEVENTINFO *dbei);
	bool   GetInfoWithData(MEVENT hEvent, DBEVENTINFO *dbei);
	DWORD  GetTime(MEVENT hEvent);
	bool   Exists(MCONTACT hContact, MEVENT& hDbExistingEvent, DBEVENTINFO *dbei);
} /* namespace Events */

/**
 * enumerating
 **/

class CEnumList : public ::LIST<CHAR>
{
	static int EnumProc        (LPCSTR pszName, DWORD ofsModuleName, LPARAM lParam);
	static int EnumSettingsProc(LPCSTR pszName, LPARAM lParam);

	static int CompareProc     (LPCSTR p1, LPCSTR p2);
	
public:
	CEnumList();
	~CEnumList();

	LPSTR	Insert(LPCSTR str);

	INT_PTR	EnumModules();
	INT_PTR	EnumSettings(MCONTACT hContact, LPCSTR pszModule);
};

} /* namespace DB */