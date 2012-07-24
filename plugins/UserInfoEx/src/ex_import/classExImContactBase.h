/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

===============================================================================

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/ex_import/classExImContactBase.h $
Revision       : $Revision: 196 $
Last change on : $Date: 2010-09-21 03:24:30 +0400 (Вт, 21 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/
#pragma once

#include "tinyxml.h"
#include "..\svc_gender.h"

HANDLE CListFindGroup(LPCSTR pszGroup);

class CExImContactBase {
	BOOLEAN		compareUID(DBVARIANT *dbv);

protected:
	LPSTR		_pszNick;		// utf8 encoded
	LPSTR		_pszDisp;		// utf8 encoded
	LPSTR		_pszGroup;		// utf8 encoded
	LPSTR		_pszAMPro;
	LPSTR		_pszProto;
	LPSTR		_pszProtoOld;
	LPSTR		_pszUIDKey;
	DWORD		_dbvUIDHash;
	DBVARIANT	_dbvUID;
	HANDLE		_hContact;
	BOOLEAN		_isNewContact;	// is this contact a new one?

	HANDLE		findHandle();

public:
	CExImContactBase();
	~CExImContactBase();

//	__inline LPCSTR disp() const	{ return mir_strcmp(_pszDisp,"")? _pszDisp : NULL;		}
//	__inline LPCSTR group() const	{ return mir_strcmp(_pszGroup,"")? _pszGroup : NULL;	}
//	__inline LPCSTR nick() const	{ return mir_strcmp(_pszNick,"")? _pszNick : NULL;		}
//	__inline LPCSTR proto() const	{ return mir_strcmp(_pszProto,"")? _pszProto : NULL;	}
//	__inline LPCSTR ampro() const	{ return mir_strcmp(_pszAMPro,"")? _pszAMPro : NULL;	}
//	__inline LPCSTR uidk() const	{ return mir_strcmp(_pszUIDKey,"")? _pszUIDKey : NULL;	}
	__inline DBVARIANT& uid()		{ return _dbvUID;	}
	__inline HANDLE handle() const	{ return _hContact;	}
	
	__inline VOID	disp(LPCSTR val)			{ _pszDisp		=	val ? mir_strdup(val): NULL; }
	__inline VOID	group(LPCSTR val)			{ _pszGroup		=	val ? mir_strdup(val): NULL; }
	__inline VOID	nick(LPCSTR val)			{ _pszNick		=	val ? mir_strdup(val): NULL; }
	__inline VOID	proto(LPCSTR val)			{ _pszProto		=	val ? mir_strdup(val): NULL; }
	__inline VOID	ampro(LPCSTR val)			{ _pszAMPro		=	val ? mir_strdup(val): NULL; }
	__inline VOID	uidk(LPCSTR val)			{ _pszUIDKey	=	val ? mir_strdup(val): NULL; }
	__inline VOID	uid(BYTE val)				{ _dbvUID.type	= DBVT_BYTE;  _dbvUID.bVal = val; }
	__inline VOID	uid(WORD val)				{ _dbvUID.type	= DBVT_WORD;  _dbvUID.wVal = val; }
	__inline VOID	uid(DWORD val)				{ _dbvUID.type	= DBVT_DWORD; _dbvUID.dVal = val; }
	__inline VOID	uidn(PBYTE val, DWORD len)	{ _dbvUID.type	= DBVT_BLOB;  _dbvUID.pbVal= val; _dbvUID.cpbVal = (WORD)len; }
	__inline VOID	uida(LPCSTR val)
	{
		_dbvUID.type = (_dbvUID.pszVal = mir_utf8decodeA(val))? DBVT_ASCIIZ : DBVT_DELETED;
		_dbvUIDHash  = hashSetting_M2(_dbvUID.pszVal);
	}
	__inline VOID	uidu(LPCSTR val)
	{
		_dbvUID.type = (_dbvUID.pszVal = mir_strdup(val))? DBVT_UTF8 : DBVT_DELETED;
		LPWSTR temp  = mir_utf8decodeW(val);
		_dbvUIDHash  = hashSettingW_M2((const char *)temp);
		mir_free(temp);
	}

	BOOLEAN			isHandle(HANDLE hContact);
	BOOLEAN			isMeta() const;

	LPSTR			uid2String(BOOLEAN bPrependType);

	BOOLEAN			fromDB(HANDLE hContact);
	BOOLEAN			fromIni(LPSTR& row);

	HANDLE			toDB();
	VOID			toIni(FILE* file, int modCount);

	BOOLEAN operator = (HANDLE hContact)	{ return fromDB(hContact);	}
};
