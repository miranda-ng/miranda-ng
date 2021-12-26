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

class CExImContactBase
{
	uint8_t compareUID(DBVARIANT *dbv);

protected:
	LPSTR _pszNick;		// utf8 encoded
	LPSTR _pszDisp;		// utf8 encoded
	LPSTR _pszGroup;		// utf8 encoded
	LPSTR _pszAMPro;
	LPSTR _pszProto;
	LPSTR _pszProtoOld;
	LPSTR _pszUIDKey;
	uint32_t _dbvUIDHash;
	DBVARIANT _dbvUID;
	MCONTACT _hContact;
	uint8_t _isNewContact;	// is this contact a new one?

	MCONTACT findHandle();

public:
	CExImContactBase();
	~CExImContactBase();

	__inline DBVARIANT &uid() { return _dbvUID; }
	__inline MCONTACT handle() const { return _hContact; }

	__inline void	disp(LPCSTR val) { _pszDisp = val ? mir_strdup(val) : nullptr; }
	__inline void	group(LPCSTR val) { _pszGroup = val ? mir_strdup(val) : nullptr; }
	__inline void	nick(LPCSTR val) { _pszNick = val ? mir_strdup(val) : nullptr; }
	__inline void	proto(LPCSTR val) { _pszProto = val ? mir_strdup(val) : nullptr; }
	__inline void	ampro(LPCSTR val) { _pszAMPro = val ? mir_strdup(val) : nullptr; }
	__inline void	uidk(LPCSTR val) { _pszUIDKey = val ? mir_strdup(val) : nullptr; }
	__inline void	uid(uint8_t val) { _dbvUID.type = DBVT_BYTE;  _dbvUID.bVal = val; }
	__inline void	uid(uint16_t val) { _dbvUID.type = DBVT_WORD;  _dbvUID.wVal = val; }
	__inline void	uid(uint32_t val) { _dbvUID.type = DBVT_DWORD; _dbvUID.dVal = val; }
	__inline void	uidn(uint8_t *val, uint32_t len) { _dbvUID.type = DBVT_BLOB;  _dbvUID.pbVal = val; _dbvUID.cpbVal = (uint16_t)len; }
	__inline void	uida(LPCSTR val)
	{
		_dbvUID.type = (_dbvUID.pszVal = mir_utf8decodeA(val)) ? DBVT_ASCIIZ : DBVT_DELETED;
		_dbvUIDHash = mir_hashstr(_dbvUID.pszVal);
	}
	__inline void	uidu(LPCSTR val)
	{
		_dbvUID.type = (_dbvUID.pszVal = mir_strdup(val)) ? DBVT_UTF8 : DBVT_DELETED;
		LPWSTR temp = mir_utf8decodeW(val);
		_dbvUIDHash = mir_hashstrW(temp);
		mir_free(temp);
	}

	uint8_t isHandle(MCONTACT hContact);
	uint8_t isMeta() const;

	LPSTR uid2String(uint8_t bPrependType);

	uint8_t fromDB(MCONTACT hContact);
	uint8_t fromIni(LPSTR &row);

	MCONTACT toDB();
	void toIni(FILE *file, int modCount);

	uint8_t operator=(MCONTACT hContact) { return fromDB(hContact); }
};
