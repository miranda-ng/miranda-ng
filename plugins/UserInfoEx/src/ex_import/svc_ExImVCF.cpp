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

/**
 * system & local includes:
 **/

#include "../commonheaders.h"

#define BLOCKSIZE 260

#define DELIM	";"

/**
 * name:	IsUSASCII
 * desc:	determine whether the pBuffer string is ascii or not
 * param:	pBuffer	- string to check
 *
 * return	TRUE or FALSE
 **/
BYTE IsUSASCII(LPCSTR pBuffer, LPDWORD pcbBuffer) 
{
	 BYTE c;
	 PBYTE s = (PBYTE)pBuffer;
	 BYTE bIsUTF = 0;

	 if (s == NULL) return 1;
	 while ((c = *s++) != 0) {
			if (c < 0x80) continue;
			if (!pcbBuffer) return 0;
			bIsUTF = 1;
	 }
	 if (pcbBuffer) *pcbBuffer = s - (PBYTE)pBuffer;
	 return !bIsUTF;
}

/*
=========================================================================================================================
 class CLineBuffer
=========================================================================================================================
*/


/**
 * name:	CLineBuffer::CLineBuffer
 * desc:	initializes all members on construction of the class
 * param:	none
 *
 * return:	nothing
 **/
CLineBuffer::CLineBuffer()
{
	_pVal	= NULL;
	_pTok	= NULL;
	_cbVal	= 0;
	_cbUsed	= 0;
}

/**
 * name:	CLineBuffer::~CLineBuffer
 * desc:	frees up all memory on class destruction
 * param:	none
 *
 * return:	nothing
 **/
CLineBuffer::~CLineBuffer()
{
	if (_pVal) mir_free(_pVal);
}

/**
 * name:	CLineBuffer::_resizeBuf
 * desc:	ensure, the right size for the buffer
 * param:	cbReq	-	number of bytes required for the next operation
 *
 * return:	TRUE if reallocation successful or memoryblock is large enough, FALSE otherwise
 **/
BYTE CLineBuffer::_resizeBuf(const size_t cbReq)
{
	if (cbReq > _cbVal - _cbUsed) {
		if (!(_pVal = (PBYTE)mir_realloc(_pVal, BLOCKSIZE + _cbVal + 1))) {
			_cbVal = 0;
			_cbUsed = 0;
			return FALSE;
		}
		_cbVal += BLOCKSIZE;
	}
	return TRUE;
}

/**
 * name:	CLineBuffer::operator =
 * desc:	applys the specified string to the class's _pVal member
 * param:	szVal	-	string to apply
 *
 * return:	length of the string, added
 **/
size_t CLineBuffer::operator = (const CHAR *szVal)
{
	if (szVal) {
		size_t cbLength = mir_strlen(szVal);

		_cbUsed = 0;
		if (_resizeBuf(cbLength)) {
			memcpy(_pVal, szVal, cbLength);
			_cbUsed += cbLength;
			return	cbLength;
		}
	}
	return 0;
}

/**
 * name:	CLineBuffer::operator +
 * desc:	appends the specified string to the class's _pVal member
 * param:	szVal	-	string to add
 *
 * return:	length of the string, added
 **/
size_t CLineBuffer::operator + (const CHAR *szVal)
{
	if (szVal) {
		size_t cbLength = mir_strlen(szVal);

		if (_resizeBuf(cbLength)) {
			memcpy(_pVal + _cbUsed, szVal, cbLength);
			_cbUsed += cbLength;
			return	cbLength;
		}
	}
	return 0;
}

/**
 * name:	CLineBuffer::operator +
 * desc:	appends the specified unicode string to the class's _pVal member
 * param:	wszVal	-	string to add
 *
 * return:	length of the string, added
 **/
size_t CLineBuffer::operator + (const WCHAR *wszVal)
{
	if (wszVal) {
		size_t cbLength = mir_wstrlen(wszVal);
		CHAR* szVal = mir_u2a(wszVal);

		if (szVal) {
			size_t cbLength = mir_strlen(szVal);

			if (_resizeBuf(cbLength)) {
				memcpy(_pVal + _cbUsed, szVal, cbLength);
				_cbUsed += cbLength;
				return	cbLength;
			}
		}
	}
	return 0;
}

/**
 * name:	CLineBuffer::operator +
 * desc:	appends the specified character's value (-127 ... 128) as a string to the class's _pVal member
 * param:	cVal	-	character whose value to add
 *
 * return:	length of the string, added
 **/
size_t CLineBuffer::operator + (const CHAR cVal)
{
	if (_resizeBuf(1)) {
		*(_pVal + _cbUsed++) = cVal;
		return	1;
	}
	return 0;
}

/**
 * name:	CLineBuffer::operator +
 * desc:	appends the specified bytes's value (0 ... 255) as a string to the class's _pVal member
 * param:	bVal	-	character whose value to add
 *
 * return:	length of the string, added
 **/
size_t CLineBuffer::operator + (const BYTE bVal)
{
	size_t cbLength = 3;
	
	if (_resizeBuf(cbLength)) {
		cbLength = mir_strlen(_itoa(bVal, (LPSTR)(_pVal + _cbUsed), 10));
		_cbUsed += cbLength;
		return	cbLength;
	}
	return 0;
}

/**
 * name:	CLineBuffer::operator +
 * desc:	appends the specified short integer as a string to the class's _pVal member
 * param:	sVal	-	short integer whose value to add
 *
 * return:	length of the string, added
 **/
size_t CLineBuffer::operator + (const SHORT sVal)
{
	size_t cbLength = 6;
	
	if (_resizeBuf(cbLength)) {
		cbLength = mir_strlen(_itoa(sVal, (LPSTR)(_pVal + _cbUsed), 10));
		_cbUsed += cbLength;
		return	cbLength;
	}
	return 0;
}

/**
 * name:	CLineBuffer::operator +
 * desc:	appends the specified word as a string to the class's _pVal member
 * param:	wVal	-	word whose value to add
 *
 * return:	length of the string, added
 **/
size_t CLineBuffer::operator + (const WORD wVal)
{
	size_t cbLength = 5;
	
	if (_resizeBuf(cbLength)) {
		cbLength = mir_strlen(_itoa(wVal, (LPSTR)(_pVal + _cbUsed), 10));
		_cbUsed += cbLength;
		return cbLength;
	}
	return 0;
}

/**
 * name:	CLineBuffer::operator +
 * desc:	appends the specified long integer as a string to the class's _pVal member
 * param:	lVal	-	long integer whose value to add
 *
 * return:	length of the string, added
 **/
size_t CLineBuffer::operator + (const LONG lVal)
{
	size_t cbLength = 11;
	
	if (_resizeBuf(cbLength)) {
		cbLength = mir_strlen(_ltoa(lVal, (LPSTR)(_pVal + _cbUsed), 10));
		_cbUsed += cbLength;
		return	cbLength;
	}
	return 0;
}

/**
 * name:	CLineBuffer::operator +
 * desc:	appends the specified double word integer as a string to the class's _pVal member
 * param:	dVal	-	double word integer whose value to add
 *
 * return:	length of the string, added
 **/
size_t CLineBuffer::operator + (const DWORD dVal)
{
	size_t cbLength = 10;
	
	if (_resizeBuf(cbLength)) {
		cbLength = mir_strlen(_ltoa(dVal, (LPSTR)(_pVal + _cbUsed), 10));
		_cbUsed += cbLength;
		return	cbLength;
	}
	return 0;
}

/**
 * name:	CLineBuffer::GetLength
 * desc:	returns the length of the _pVal string
 * param:	nothing
 *
 * return:	length of the string
 **/
size_t CLineBuffer::GetLength()
{
	return _cbUsed;
}

/**
 * name:	CLineBuffer::GetBuffer
 * desc:	returns the pointer of the _pVal string
 *			!!Use carefully
 * param:	nothing
 *
 * return:	pointer to _pVal
 **/
LPCSTR CLineBuffer::GetBuffer() 
{
	return (LPCSTR)_pVal;
}

/**
 * name:	CLineBuffer::TruncToLength
 * desc:	resulting string has cbLength characters
 * param:	cbLength	- desired length of the string member
 *
 * return:	nothing
 **/
void CLineBuffer::TruncToLength(size_t cbLength)
{
	if (cbLength < _cbUsed) {
		_cbUsed = cbLength;
		_pVal[cbLength] = 0;
	}
}

/**
 * name:	CLineBuffer::TruncToLength
 * desc:	resulting string is Truncated by the specified number of bytes
 * param:	count	- desired count of bytes to Truncate
 *
 * return:	nothing
 **/
void CLineBuffer::Truncate(size_t count)
{
	if (_cbUsed <= count) {
		_cbUsed = 0;
		*_pVal = 0;
	}
	else {
		_cbUsed -= count;
		_pVal[_cbUsed] = 0;
	}
}

/**
 * name:	CLineBuffer::TruncateSMS
 * desc:	resulting string is Truncated by the " SMS" if found
 * param:	nothing
 *
 * return:	nothing
 **/
void CLineBuffer::TruncateSMS()
{
	if (!strncmp((LPSTR)(_pVal + _cbUsed - 4), " SMS", 4)) {
		_cbUsed -= 4;
		_pVal[_cbUsed] = 0;
	}
}

/**
 * name:	CLineBuffer::fput
 * desc:	string member is written to the specified stream and Truncated to zero afterwards
 * param:	outfile		-	the stream to write to
 *
 * return:	nothing
 **/
void CLineBuffer::fput(FILE *outfile)
{
	if (_pVal) {
		_pVal[_cbUsed] = 0;
		fputs((LPCSTR)_pVal, outfile);
		_cbUsed = 0;
		*_pVal = 0;
	}
}

/**
 * name:	CLineBuffer::fputEncoded
 * desc:	string member is encoded and written to the specified stream and Truncated to zero afterwards
 * param:	outfile		-	the stream to write to
 *
 * return:	nothing
 **/
void CLineBuffer::fputEncoded(FILE *outFile)
{
	PBYTE pVal = _pVal;

	if (pVal && _cbUsed > 0) {
		_pVal[_cbUsed] = 0;
		while (_cbUsed > 0 && *pVal) {
			switch (*pVal) {
				// translate special characters
				case ':':
				case ';':
				case '\r':
				case '\n':
				case '\t':
					fprintf(outFile, "=%02X", *pVal);
					break;
				// Some database texts may contain encoded escapes, that have to be translated too.
				case '\\':
					if (*(pVal+1) == 'r') {
						fprintf(outFile, "=%02X", '\r');
						pVal++;
						break;
					}
					if (*(pVal+1) == 't') {
						fprintf(outFile, "=%02X", '\t');
						pVal++;
						break;
					}
					if (*(pVal+1) == 'n') {
						fprintf(outFile, "=%02X", '\n');
						pVal++;
						break;
					}
				// translate all characters which are not contained in the USASCII code
				default:
					if (*pVal > 127)	fprintf(outFile, "=%02X", *pVal);
					else fputc(*pVal, outFile);
					break;
			}
			pVal++;
			(_cbUsed)--;
		}
		*_pVal = 0;
	}
}

/**
 * name:	CLineBuffer::fgetEncoded
 * desc:	string member is read from the specified stream decoded
 * param:	outfile		-	the stream to write to
 *
 * return:	nothing
 **/
int CLineBuffer::fgetEncoded(FILE *inFile)
{
	CHAR c;
	CHAR hex[3];
	WORD wAdd = 0;

	hex[2] = 0;

	_cbUsed = 0;

	while (EOF != (c = fgetc(inFile))) {
		switch (c) {
			case '\n':
				if (_cbUsed > 0 && _pVal[_cbUsed - 1] == '\r') {
					_pVal[--_cbUsed] = 0;
					wAdd--;
				}
				else
					_pVal[_cbUsed] = 0;
				return wAdd;
			case '=':
				if (_resizeBuf(1)) {
					fread(hex, 2, 1, inFile);
					*(_pVal + _cbUsed++) = (BYTE)strtol(hex, NULL, 16);
					wAdd++;
				}
				break;
			default:
				if (_resizeBuf(1)) {
					*(_pVal + _cbUsed++) = c;
					wAdd++;
				}
				break;
		}
	}
	_pVal[_cbUsed] = 0;
	return _cbUsed > 0 ? wAdd : EOF;
}

/**
 * name:	CLineBuffer::GetTokenFirst
 * desc:	scans for the first <delim> in the _pVal member and returns all characters
 *			that come before the first <delim> in a new CLineBuffer class
 * param:	delim		-	the deliminer which delimins the string
 *			pBuf		-	pointer to a new CLineBuffer class holding the result
 *
 * return:	length of the found string value
 **/
size_t CLineBuffer::GetTokenFirst(const CHAR delim, CLineBuffer * pBuf)
{
	PBYTE here;
	size_t wLength;

	_pTok = _pVal;

	if (!_pTok || !*_pTok)
		return 0;

	for (here = _pTok;; here++) {
		if (*here == 0 || *here == '\n' || *here == delim) {
			wLength = here - _pTok;
			if (pBuf) {
				CHAR c = *here;
				*here = 0;
				*pBuf = (LPCSTR)_pTok;
				*here = c;
			}
			_pTok = (*here == 0	|| *here == '\n') ? NULL : ++here;
			break;
		}
	}
	return wLength;
}

/**
 * name:	CLineBuffer::GetTokenNext
 * desc:	scans for the next <delim> in the _pVal member and returns all characters
 *			that come before the first <delim> in a new CLineBuffer class
 * param:	delim		-	the deliminer which delimins the string
 *			pBuf		-	pointer to a new CLineBuffer class holding the result
 *
 * return:	length of the found string value
 **/
size_t CLineBuffer::GetTokenNext(const CHAR delim, CLineBuffer * pBuf)
{
	PBYTE here;
	size_t wLength;

	if (!_pTok || !*_pTok)
		return 0;

	for (here = _pTok;; here++) {
		if (*here == 0 || *here == '\n' || *here == delim) {
			wLength = here - _pTok;

			if (pBuf) {
				CHAR c = *here;
				
				*here = 0;
				*pBuf = (LPCSTR)_pTok;
				*here = c;
			}
			_pTok = (*here == 0	|| *here == '\n') ? NULL : ++here;
			break;
		}
	}
	return wLength;
}

/**
 * name:	CLineBuffer::DBWriteTokenFirst
 * desc:	scans for the first <delim> in the _pVal member and writes all characters
 *			that come before the first <delim> to database
 * param:	hContact	-	handle to the contact to write the setting to
 *			pszModule	-	destination module
 *			pszSetting	-	destination setting for the value
 *			delim		-	the deliminer which delimins the string
 *
 * return:	0 if successful, 1 otherwise
 **/
int CLineBuffer::DBWriteTokenFirst(MCONTACT hContact, const CHAR* pszModule, const CHAR* pszSetting, const CHAR delim)
{
	PBYTE here;
	int iRet = 1;
	_pTok = _pVal;

	if (_pTok && *_pTok) {
		for (here = _pTok;; here++) {
			if (*here == 0 || *here == '\n' || *here == delim) {
				
				if (here - _pTok > 0) {
					CHAR c = *here;
				
					*here = 0;
					iRet = db_set_s(hContact, pszModule, pszSetting, (LPSTR)_pTok);
					*here = c;
				}
				_pTok = (*here == 0	|| *here == '\n') ? NULL : ++here;
				break;
			}
		}
	}
	if (iRet) iRet = db_unset(hContact, pszModule, pszSetting);
	return iRet;
}

/**
 * name:	CLineBuffer::GetTokenNext
 * desc:	scans for the next <delim> in the _pVal member and writes all characters
 *			that come before the first <delim> to database
 * param:	hContact	-	handle to the contact to write the setting to
 *			pszModule	-	destination module
 *			pszSetting	-	destination setting for the value
 *			delim		-	the deliminer which delimins the string
 *
 * return:	0 if successful, 1 otherwise
 **/
int CLineBuffer::DBWriteTokenNext(MCONTACT hContact, const CHAR* pszModule, const CHAR* pszSetting, const CHAR delim)
{
	PBYTE here;
	int iRet = 1;

	if (_pTok && *_pTok) {
		for (here = _pTok;; here++) {
			if (*here == 0 || *here == '\n' || *here == delim) {
				
				if (here - _pTok > 0) {
					CHAR c = *here;
				
					*here = 0;
					iRet = db_set_s(hContact, pszModule, pszSetting, (LPSTR)_pTok);
					*here = c;
				}
				_pTok = (*here == 0	|| *here == '\n') ? NULL : ++here;
				break;
			}
		}
	}
	if (iRet) iRet = db_unset(hContact, pszModule, pszSetting);
	return iRet;
}

/**
 * name:	CLineBuffer::GetTokenNext
 * desc:	writes _pVal member to database
 * param:	hContact	-	handle to the contact to write the setting to
 *			pszModule	-	destination module
 *			pszSetting	-	destination setting for the value
 *
 * return:	0 if successful, 1 otherwise
 **/
int CLineBuffer::DBWriteSettingString(MCONTACT hContact, const CHAR* pszModule, const CHAR* pszSetting)
{
	if (_pVal && _cbUsed > 0)
		return db_set_s(hContact, pszModule, pszSetting, (LPSTR)_pVal);
	return 1;
}

/*
=========================================================================================================================
 class CVCardFileVCF
=========================================================================================================================
*/

/**
 * name:	CVCardFileVCF
 * desc:	default constructor
 * param:	none
 * return	none
 **/
CVCardFileVCF::CVCardFileVCF()
{
	_pFile = NULL;
	_hContact = INVALID_CONTACT_ID;
	_pszBaseProto = NULL;
	_hasUtf8 = 0;
	_useUtf8 = FALSE;
}

/**
 * name:	CVCardFileVCF::CVCardFileVCF
 * desc:	searches a static stringlist for the occureance of iID and adds 
 *			the translated string value to the line buffer
 * param:	pList	-	pointer to the list to search in
 *			nList	-	number of items in the list
 *			iID		-	the id to search for
 *			cbRew	-	number of characters to truncate the _clVal by before writing to file
 *
 * return	number of the added bytes
 **/
size_t CVCardFileVCF::packList(LPIDSTRLIST pList, UINT nList, int iID, size_t *cbRew)
{
	UINT i;
	WORD wAdd = 0;

	for (i = 0; i < nList; i++) {
		if (pList[i].nID == iID) {
			return (_clVal + pList[i].ptszTranslated);
		}
	}
	if (cbRew) (*cbRew)++;
	return 0;
}

/**
 * name:	CVCardFileVCF::GetSetting
 * desc:	trys to read a value from the pszModule and than from the basic protocol module of the contact
 *			where strings are converted to ansi
 * param:	pszModule	-	module to read the value from
 *			pszSetting	-	setting to read the value from
 *			dbv			-	pointer to the structure holding the result
 *
 * return	value type
 **/
BYTE CVCardFileVCF::GetSetting(const CHAR *pszModule, const CHAR *pszSetting, DBVARIANT *dbv)
{
	int type = _useUtf8 ? DBVT_UTF8 : DBVT_ASCIIZ;
	dbv->pszVal = NULL;
	if (!pszModule || db_get_s(_hContact, pszModule, pszSetting, dbv, type) || (dbv->type == DBVT_ASCIIZ && !dbv->pszVal && !*dbv->pszVal))
		if (!_pszBaseProto || db_get_s(_hContact, _pszBaseProto, pszSetting, dbv) || (dbv->type == DBVT_ASCIIZ && !dbv->pszVal && !*dbv->pszVal))
			return DBVT_DELETED;

	_hasUtf8 += _useUtf8 && !IsUSASCII(dbv->pszVal, NULL);
	return dbv->type;
}

/**
 * name:	CVCardFileVCF::packDB
 * desc:	read a value from the database and add it to the line buffer
 * param:	pszModule	-	module to read the value from
 *			pszSetting	-	setting to read the value from
 *			cbRew		-	number of characters to truncate the _clVal by before writing to file
 *
 * return	number of bytes, added to the linebuffer
 **/
size_t CVCardFileVCF::packDB(const CHAR *pszModule, const CHAR *pszSetting, size_t *cbRew)
{
	DBVARIANT dbv;

	switch (GetSetting(pszModule, pszSetting, &dbv)) {
		case DBVT_DELETED:
			if (cbRew) (*cbRew)++;
			break;
		case DBVT_BYTE:
			return _clVal + dbv.bVal;
		case DBVT_WORD:
			return _clVal + dbv.wVal;
		case DBVT_DWORD:
			return _clVal + dbv.dVal;
		case DBVT_UTF8:
		case DBVT_ASCIIZ:
		{
			size_t wAdd = _clVal + dbv.pszVal;
			db_free(&dbv);
			return wAdd;
		}
		default:
			if (cbRew) (*cbRew)++;
			db_free(&dbv);
			break;
	}
	return 0;
}

/**
 * name:	CVCardFileVCF::packDBList
 * desc:	read a value from the database and add a found list item to the line buffer
 * param:	pszModule	-	module to read the value from
 *			pszSetting	-	setting to read the value from
 *			GetList		-	pointer to a function retrieving the list pointer
 *			bSigned		-	is the read database value signed?
 *			cbRew		-	number of characters to truncate the _clVal by before writing to file
 *
 * return	number of bytes, added to the linebuffer
 **/
size_t CVCardFileVCF::packDBList(const CHAR *pszModule, const CHAR *pszSetting, MIRANDASERVICE GetList, BYTE bSigned, size_t *cbRew)
{
	DBVARIANT dbv;
	UINT nList;
	LPIDSTRLIST pList;
	size_t wAdd = 0;

	GetList((WPARAM)&nList, (LPARAM)&pList);
	switch (GetSetting(pszModule, pszSetting, &dbv)) {
		case DBVT_BYTE:
			wAdd = packList(pList, nList, (int)(bSigned ? dbv.cVal : dbv.bVal), cbRew);
			break;
		case DBVT_WORD:
			wAdd = packList(pList, nList, (int)(bSigned ? dbv.sVal : dbv.wVal), cbRew);
			break;
		case DBVT_DWORD:
			wAdd = packList(pList, nList, (int)(bSigned ? dbv.lVal : dbv.dVal), cbRew);
			break;
		case DBVT_UTF8:
		case DBVT_ASCIIZ:
			wAdd = _clVal + Translate(dbv.pszVal);
			db_free(&dbv);
			break;
		case DBVT_DELETED:
			wAdd = 0;
			break;
		default:
			wAdd = 0;
			db_free(&dbv);
			break;
	}
	if (cbRew) *cbRew = wAdd ? 0 : *cbRew + 1;
	return wAdd;
}

/**
 * name:	CVCardFileVCF::writeLine
 * desc:	write a line as clear text to the vcard file
 * param:	szSet	-	the string, which identifies the line
 *			cbRew	-	number of characters to truncate the _clVal by before writing to file
 *
 * return	number of bytes, added to the linebuffer
 **/
void CVCardFileVCF::writeLine(const CHAR *szSet, size_t *cbRew)
{
	if (cbRew) {
		_clVal.Truncate(*cbRew);
		*cbRew = 0;
	}
	if (_clVal.GetLength() > 0) {
		fputs(szSet, _pFile);
		if (_hasUtf8) {
			fputs(";CHARSET=UTF-8;ENCODING=QUOTED-PRINTABLE:", _pFile);
			_clVal.fputEncoded(_pFile);
			_hasUtf8 = FALSE;
		}
		else {
			fputc(':', _pFile);
			_clVal.fput(_pFile);
		}
		fputc('\n', _pFile);
	}
}

/**
 * name:	CVCardFileVCF::writeLineEncoded
 * desc:	write a line as encoded text to the vcard file
 * param:	szSet	-	the string, which identifies the line
 *			cbRew	-	number of characters to truncate the _clVal by before writing to file
 *
 * return	number of bytes, added to the linebuffer
 **/
void CVCardFileVCF::writeLineEncoded(const CHAR *szSet, size_t *cbRew)
{
	if (cbRew) {
		_clVal.Truncate(*cbRew);
		*cbRew = 0;
	}
	if (_clVal.GetLength() > 0) {
		fputs(szSet, _pFile);
		if (_hasUtf8) {
			fputs(";CHARSET=UTF-8", _pFile);
			_hasUtf8 = FALSE;
		}
		fputs(";ENCODING=QUOTED-PRINTABLE:", _pFile);
		_clVal.fputEncoded(_pFile);
		fputc('\n', _pFile);
	}
}

/**
 * name:	Open
 * desc:	open a specified filename and link to the contact
 * param:	hContact	- handle to the contact to link with the vCard file
 *			pszFileName	- path to the file to open
 *			pszMode		- the mode the file should be opened with
 * return	TRUE or FALSE
 **/
BYTE CVCardFileVCF::Open(MCONTACT hContact,	LPCSTR pszFileName, LPCSTR pszMode)
{
	if (!(_pFile = fopen(pszFileName, pszMode)))
		return FALSE;
	if ((_hContact = hContact) == INVALID_CONTACT_ID)
		return FALSE;
	if (!(_pszBaseProto = DB::Contact::Proto(_hContact)))
		return FALSE;
	return TRUE;
}

/**
 * name:	Close
 * desc:	close up the file
 * param:	hContact	- handle to the contact to link with the vCard file
 *			pszFileName	- path to the file to open
 *			pszMode		- the mode the file should be opened with
 * return	TRUE or FALSE
 **/
void CVCardFileVCF::Close(void)
{
	if (_pFile) 
		fclose(_pFile);
	_pFile = NULL;
	_hContact = INVALID_CONTACT_ID;
	_pszBaseProto = NULL;
}

/**
 * name:	Export
 * desc:	export the contacts information
 * param:	none
 * return	TRUE or FALSE
 **/
BYTE CVCardFileVCF::Export(BYTE bExportUtf)
{
	size_t cbRew = 0;

	_useUtf8 = bExportUtf;

	fputs("BEGIN:VCARD\nVERSION:2.1\n", _pFile);

	//
	// naming
	//
	packDB(USERINFO, SET_CONTACT_LASTNAME, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_FIRSTNAME, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_SECONDNAME, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_TITLE, &cbRew);
	_clVal + DELIM;
	packDBList(USERINFO, SET_CONTACT_PREFIX, (MIRANDASERVICE)GetNamePrefixList, FALSE, &cbRew);
	writeLine("N", &cbRew);

	if (packDB(USERINFO, SET_CONTACT_TITLE))
		_clVal + " ";

	if (packDB(USERINFO, SET_CONTACT_FIRSTNAME))
		_clVal + " ";
	else
		cbRew = 1;

	if (packDB(USERINFO, SET_CONTACT_SECONDNAME))
		_clVal + " ";
	else
		cbRew = 1;

	if (packDB(USERINFO, SET_CONTACT_LASTNAME))
		_clVal + " ";
	else
		cbRew = 1;

	packDBList(USERINFO, SET_CONTACT_PREFIX, (MIRANDASERVICE)GetNamePrefixList, FALSE, &cbRew);
	writeLine("FN");

	packDB(USERINFO, SET_CONTACT_NICK);
	writeLine("NICKNAME");

	//
	// organisation
	//
	packDB(USERINFO, SET_CONTACT_COMPANY, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_COMPANY_DEPARTMENT, &cbRew);
	writeLine("ORG", &cbRew);
	packDB(USERINFO, SET_CONTACT_COMPANY_POSITION);
	writeLine("TITLE");
	packDBList(USERINFO, SET_CONTACT_COMPANY_OCCUPATION, (MIRANDASERVICE)GetOccupationList, FALSE);
	writeLine("ROLE");

	//
	// phone numbers
	//
	if (packDB(USERINFO, SET_CONTACT_PHONE)) {
		_clVal.TruncateSMS();
		writeLine("TEL;HOME;VOICE");
	}
	if (packDB(USERINFO, SET_CONTACT_FAX)) {
		_clVal.TruncateSMS();
		writeLine("TEL;HOME;FAX");
	}
	if (packDB(USERINFO, SET_CONTACT_CELLULAR)) {
		_clVal.TruncateSMS();
		writeLine("TEL;CELL;VOICE");
	}
	if (packDB(USERINFO, SET_CONTACT_COMPANY_PHONE)) {
		_clVal.TruncateSMS();
		writeLine("TEL;WORK;VOICE");
	}
	if (packDB(USERINFO, SET_CONTACT_COMPANY_FAX)) {
		_clVal.TruncateSMS();
		writeLine("TEL;WORK;FAX");
	}
	if (packDB(USERINFO, SET_CONTACT_COMPANY_CELLULAR)) {
		_clVal.TruncateSMS();
		writeLine("TEL;PAGER;VOICE");
	}

	//
	// private address
	//
	_clVal + ";;"; 
	cbRew = 1;
	packDB(USERINFO, SET_CONTACT_STREET, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_CITY, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_STATE, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_ZIP, &cbRew);
	_clVal + DELIM;
	packDBList(USERINFO, SET_CONTACT_COUNTRY, (MIRANDASERVICE)GetCountryList, FALSE, &cbRew);
	writeLine("ADR;HOME", &cbRew);

	//
	// company address
	//
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_COMPANY_OFFICE, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_COMPANY_STREET, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_COMPANY_CITY, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_COMPANY_STATE, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_COMPANY_ZIP, &cbRew);
	_clVal + DELIM;
	packDBList(USERINFO, SET_CONTACT_COMPANY_COUNTRY, (MIRANDASERVICE)GetCountryList, FALSE, &cbRew);
	writeLine("ADR;WORK", &cbRew);

	//
	// origin address
	//
	_clVal + ";;";
	cbRew = 1;
	packDB(USERINFO, SET_CONTACT_ORIGIN_STREET, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_ORIGIN_CITY, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_ORIGIN_STATE, &cbRew);
	_clVal + DELIM;
	packDB(USERINFO, SET_CONTACT_ORIGIN_ZIP, &cbRew);
	_clVal + DELIM;
	packDBList(USERINFO, SET_CONTACT_ORIGIN_COUNTRY, (MIRANDASERVICE)GetCountryList, FALSE, &cbRew);
	writeLine("ADR;POSTAL", &cbRew);

	//
	// homepages
	//
	if (packDB(USERINFO, SET_CONTACT_HOMEPAGE))
		writeLine("URL;HOME");
	if (packDB(USERINFO, SET_CONTACT_COMPANY_HOMEPAGE))
		writeLine("URL;WORK");

	//
	// e-mails
	//
	if (packDB(USERINFO, SET_CONTACT_EMAIL))
		writeLine("EMAIL;PREF;intERNET");
	if (packDB(USERINFO, SET_CONTACT_EMAIL0))
		writeLine("EMAIL;intERNET");
	if (packDB(USERINFO, SET_CONTACT_EMAIL1))
		writeLine("EMAIL;intERNET");

	//
	// gender
	//
	{
		BYTE gender = db_get_b(_hContact, USERINFO, SET_CONTACT_GENDER, 0);
		if (!gender) gender = db_get_b(_hContact, _pszBaseProto, SET_CONTACT_GENDER, 0);
		switch (gender) {
			case 'F':
				fputs("X-WAB-GENDER:1\n", _pFile);
				break;
			case 'M':
				fputs("X-WAB-GENDER:2\n", _pFile);
				break;
		}
	}

	//
	// birthday
	//
	{
		MAnnivDate mdb;

		if (!mdb.DBGetBirthDate(_hContact, NULL))
			fprintf(_pFile, "BDAY:%d%02d%02d\n\0", mdb.Year(), mdb.Month(), mdb.Day());
	}
	
	//
	// notes
	//
	if (packDB(USERINFO, SET_CONTACT_MYNOTES))
		writeLineEncoded("NOTE");

	//
	// about
	//
	if (packDB(USERINFO, SET_CONTACT_ABOUT))
		writeLineEncoded("ABOUT");

	//
	// contacts protocol, uin setting, uin value
	//
	{
		CHAR szUID[MAXUID];
		LPCSTR uid;

		uid = (LPCSTR)CallProtoService(_pszBaseProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0);
		if ((INT_PTR)uid != CALLSERVICE_NOTFOUND && uid) {
			if (!db_get_static(_hContact, _pszBaseProto, uid, szUID, sizeof(szUID)))
				fprintf(_pFile, "IM;%s;%s:%s\n", _pszBaseProto, uid, szUID);
		}
	}
	
	//
	// time of creation
	//
	{
		SYSTEMTIME	st;
	
		GetLocalTime(&st);
		fprintf(_pFile, "REV:%04d%02d%02dD%02d%02d%02dT\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	}

	fputs("END:VCARD", _pFile);
	return 0;
}

/**
 * name:	CVCardFileVCF::readLine
 * desc:	read one line from the VCF file and return the setting string
 *			to the szVCFSetting and the value to _clVal
 * param:	szVCFSetting	- string holding the setting information
 *			cchSetting		- number of bytes the szVCFSetting can hold
 *
 * return:	number of characters read from the file or EOF
 **/
int CVCardFileVCF::readLine(LPSTR szVCFSetting, WORD cchSetting)
{
	LPSTR here;
	
	// read setting (size is never larger than MAX_SETTING, error otherwise!)
	for (here = szVCFSetting; here - szVCFSetting < cchSetting && EOF != (*here = fgetc(_pFile)); here++) {
		// end of the setting string
		if (*here == ':') {
			*here = 0;
			break;
		}
		// end of line before value?
		if (*here == '\n')
			return 0;
	}
	// ignore line if setting was not read correctly
	if (here - szVCFSetting == cchSetting)
		return 0;
	
	// read the value to the linebuffer, because its length may be very large
	return _clVal.fgetEncoded(_pFile);
}

/**
 * name:	CVCardFileVCF::Import
 * desc:	imports all lines from the file and writes them to database
 * param:	nothing
 *
 * return:	number of characters read from the file or EOF
 **/
BYTE CVCardFileVCF::Import()
{
	CHAR szEnt[MAX_PATH];
	LPSTR pszParam;
	int cbLine;
	BYTE numEmails = 0;

	while (EOF != (cbLine = readLine(szEnt, MAX_PATH))) {

		// ignore empty lines
		if (!cbLine) continue;

		// isolate the param string
		if (pszParam = mir_strchr(szEnt, ';')) {
			*(pszParam++) = 0;
		}
		switch (*szEnt) {
			case 'A':
				if (!strcmp(szEnt, "ABOUT")) {
					_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_ABOUT);
					continue;
				}
				if (!strcmp(szEnt, "ADR")) {
					if (!pszParam) continue;
					if (!strcmp(pszParam, "HOME")) {
						_clVal.GetTokenFirst(';', NULL);
						_clVal.GetTokenNext(';', NULL);
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_STREET, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_CITY, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_STATE, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_ZIP, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_COUNTRY, ';');
						continue;
					}
					if (!strcmp(pszParam, "WORK")) {
						_clVal.GetTokenFirst(';', NULL);
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_COMPANY_OFFICE, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_COMPANY_STREET, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_COMPANY_CITY, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_COMPANY_STATE, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_COMPANY_ZIP, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_COMPANY_COUNTRY, ';');
						continue;
					}
					if (!strcmp(pszParam, "POSTAL")) {
						_clVal.GetTokenFirst(';', NULL);
						_clVal.GetTokenNext(';', NULL);
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_ORIGIN_STREET, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_ORIGIN_CITY, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_ORIGIN_STATE, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_ORIGIN_ZIP, ';');
						_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_ORIGIN_COUNTRY, ';');
					}
				}
				continue;

			case 'B':
				if (!strcmp(szEnt, "BDAY")) {
					if (_clVal.GetLength() == 8) {
						CHAR buf[5];

						memcpy(buf, _clVal.GetBuffer(), 4);
						buf[4] = 0;
						db_set_w(_hContact, MOD_MBIRTHDAY, SET_CONTACT_BIRTHYEAR, (WORD)strtol(buf, NULL, 10));
						memcpy(buf, _clVal.GetBuffer() + 4, 2);
						buf[2] = 0;
						db_set_b(_hContact, MOD_MBIRTHDAY, SET_CONTACT_BIRTHMONTH, (BYTE)strtol(buf, NULL, 10));
						memcpy(buf, _clVal.GetBuffer() + 6, 2);
						buf[2] = 0;
						db_set_b(_hContact, MOD_MBIRTHDAY, SET_CONTACT_BIRTHDAY, (BYTE)strtol(buf, NULL, 10));
					}
				}
				continue;

			case 'E':
				if (!strcmp(szEnt, "EMAIL")) {
					if (!pszParam || !strstr(pszParam, "intERNET"))
						continue;
					if (strstr(pszParam, "PREF")) {
						_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_EMAIL);
						continue;
					}
					switch (numEmails++) {
						case 0:
							_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_EMAIL0);
							break;
						case 1:
							_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_EMAIL1);
							break;
					}
				}
				continue;
			/*
			case 'I':
				if (!strcmp(szEnt, "IM")) {
					LPSTR	pszModule, pszSetting;

					if (pszParam && (pszModule = strtok(pszParam, DELIM)) && (pszSetting = strtok(NULL, DELIM)))
						_clVal.DBWriteSettingString(_hContact, pszModule, pszSetting);
				}
				continue;
			*/
			case 'N':
				if (!strcmp(szEnt, "N")) {
					_clVal.DBWriteTokenFirst(_hContact, USERINFO, SET_CONTACT_LASTNAME, ';');
					_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_FIRSTNAME, ';');
					_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_SECONDNAME, ';');
					_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_TITLE, ';');
					_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_PREFIX, ';');
					continue;
				}
				if (!strcmp(szEnt, "NICKNAME")) {
					_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_NICK);
					continue;
				}
				if (!strcmp(szEnt, "NOTE")) {
					_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_MYNOTES);
				}
				continue;

			case 'O':
				if (!strcmp(szEnt, "ORG")) {
					_clVal.DBWriteTokenFirst(_hContact, USERINFO, SET_CONTACT_COMPANY, ';');
					_clVal.DBWriteTokenNext(_hContact, USERINFO, SET_CONTACT_COMPANY_DEPARTMENT, ';');
				}
				continue;

			case 'R':
				if (!strcmp(szEnt, "ROLE")) {
					_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_COMPANY_OCCUPATION);
				}
				continue;

			case 'T':
				if (!strcmp(szEnt, "TITLE")) {
					_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_COMPANY_POSITION);
					continue;
				}
				if (!strcmp(szEnt, "TEL")) {

					if (!pszParam) continue;
					
					if (!strcmp(pszParam, "HOME;VOICE")) {
						_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_PHONE);
						continue;
					}
					if (!strcmp(pszParam, "HOME;FAX")) {
						_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_FAX);
						continue;
					}
					if (!strcmp(pszParam, "CELL;VOICE")) {
						_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_CELLULAR);
						continue;
					}
					if (!strcmp(pszParam, "WORK;VOICE")) {
						_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_COMPANY_PHONE);
						continue;
					}
					if (!strcmp(pszParam, "WORK;FAX")) {
						_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_COMPANY_FAX);
						continue;
					}
					if (!strcmp(pszParam, "PAGER;VOICE")) {
						_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_COMPANY_CELLULAR);
						continue;
					}
				}
				continue;
		
			case 'U':
				if (!strcmp(szEnt, "URL")) {

					if (!pszParam) continue;

					if (!strcmp(pszParam, "HOME")) {
						_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_HOMEPAGE);
						continue;
					}
					if (!strcmp(pszParam, "WORK")) {
						_clVal.DBWriteSettingString(_hContact, USERINFO, SET_CONTACT_COMPANY_HOMEPAGE);
					}
				}
				continue;

			case 'X':
				if (!strcmp(szEnt, "X-WAB-GENDER")) {
					if (!strcmp(_clVal.GetBuffer(), "1"))
						db_set_b(_hContact, USERINFO, SET_CONTACT_GENDER, 'F');
					else if (!strcmp(_clVal.GetBuffer(), "2"))
						db_set_b(_hContact, USERINFO, SET_CONTACT_GENDER, 'M');
				}
				continue;
		}
	}
	return TRUE;
}
