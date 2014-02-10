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

HANDLE CListFindGroup(LPCSTR pszGroup);

class CExImContactBase
{
	BYTE compareUID(DBVARIANT *dbv);

protected:
	LPSTR _pszNick;		// utf8 encoded
	LPSTR _pszDisp;		// utf8 encoded
	LPSTR _pszGroup;		// utf8 encoded
	LPSTR _pszAMPro;
	LPSTR _pszProto;
	LPSTR _pszProtoOld;
	LPSTR _pszUIDKey;
	DWORD _dbvUIDHash;
	DBVARIANT _dbvUID;
	MCONTACT _hContact;
	BYTE _isNewContact;	// is this contact a new one?

	MCONTACT findHandle();

public:
	CExImContactBase();
	~CExImContactBase();

	__inline DBVARIANT& uid()		{ return _dbvUID;	}
	__inline MCONTACT handle() const { return _hContact; }
	
	__inline void	disp(LPCSTR val)			{ _pszDisp		=	val ? mir_strdup(val): NULL; }
	__inline void	group(LPCSTR val)			{ _pszGroup		=	val ? mir_strdup(val): NULL; }
	__inline void	nick(LPCSTR val)			{ _pszNick		=	val ? mir_strdup(val): NULL; }
	__inline void	proto(LPCSTR val)			{ _pszProto		=	val ? mir_strdup(val): NULL; }
	__inline void	ampro(LPCSTR val)			{ _pszAMPro		=	val ? mir_strdup(val): NULL; }
	__inline void	uidk(LPCSTR val)			{ _pszUIDKey	=	val ? mir_strdup(val): NULL; }
	__inline void	uid(BYTE val)				{ _dbvUID.type	= DBVT_BYTE;  _dbvUID.bVal = val; }
	__inline void	uid(WORD val)				{ _dbvUID.type	= DBVT_WORD;  _dbvUID.wVal = val; }
	__inline void	uid(DWORD val)				{ _dbvUID.type	= DBVT_DWORD; _dbvUID.dVal = val; }
	__inline void	uidn(PBYTE val, DWORD len)	{ _dbvUID.type	= DBVT_BLOB;  _dbvUID.pbVal= val; _dbvUID.cpbVal = (WORD)len; }
	__inline void	uida(LPCSTR val)
	{
		_dbvUID.type = (_dbvUID.pszVal = mir_utf8decodeA(val))? DBVT_ASCIIZ : DBVT_DELETED;
		_dbvUIDHash  = hashSetting_M2(_dbvUID.pszVal);
	}
	__inline void	uidu(LPCSTR val)
	{
		_dbvUID.type = (_dbvUID.pszVal = mir_strdup(val))? DBVT_UTF8 : DBVT_DELETED;
		LPWSTR temp  = mir_utf8decodeW(val);
		_dbvUIDHash  = hashSettingW_M2((const char *)temp);
		mir_free(temp);
	}

	BYTE isHandle(MCONTACT hContact);
	BYTE isMeta() const;

	LPSTR uid2String(BYTE bPrependType);

	BYTE fromDB(MCONTACT hContact);
	BYTE fromIni(LPSTR& row);

	MCONTACT toDB();
	void toIni(FILE* file, int modCount);

	BYTE operator = (MCONTACT hContact)	{ return fromDB(hContact);	}
};
