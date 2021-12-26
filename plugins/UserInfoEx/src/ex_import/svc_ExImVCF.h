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

class CLineBuffer
{
private:
	uint8_t *_pVal;
	uint8_t *_pTok;
	size_t	_cbVal;
	size_t	_cbUsed;

	uint8_t _resizeBuf(const size_t cbReq);

public:
	CLineBuffer();
	~CLineBuffer();

	size_t operator=(const CHAR *szVal);

	size_t operator + (const CHAR *szVal);
	size_t operator + (const wchar_t *wszVal);
	size_t operator + (const CHAR cVal);
	size_t operator + (const uint8_t bVal);
	size_t operator + (const SHORT sVal);
	size_t operator + (const uint16_t wVal);
	size_t operator + (const LONG lVal);
	size_t operator + (const uint32_t dVal);

	size_t GetLength();
	LPCSTR GetBuffer();

	void   TruncToLength(size_t cbLength);
	void   Truncate(size_t count);
	void   TruncateSMS();

	void   fput(FILE *outfile);
	void   fputEncoded(FILE *outFile);
	int    fgetEncoded(FILE *inFile);

	size_t GetTokenFirst(const CHAR delim, CLineBuffer *pBuf);
	size_t GetTokenNext(const CHAR delim, CLineBuffer *pBuf);
	int	 DBWriteTokenFirst(MCONTACT hContact, const CHAR *pszModule, const CHAR *pszSetting, const CHAR delim);
	int	 DBWriteTokenNext(MCONTACT hContact, const CHAR *pszModule, const CHAR *pszSetting, const CHAR delim);
	int	 DBWriteSettingString(MCONTACT hContact, const CHAR *pszModule, const CHAR *pszSetting);
};

class CVCardFileVCF
{
private:
	CLineBuffer _clVal;
	FILE*       _pFile;
	MCONTACT    _hContact;
	const CHAR* _pszBaseProto;
	uint16_t        _cbRew;
	uint8_t        _useUtf8;
	uint16_t        _hasUtf8;

	size_t packList(LPIDSTRLIST pList, UINT nList, int iID, size_t *cbRew = nullptr);
	uint8_t   GetSetting(const CHAR *pszModule, const CHAR *pszSetting, DBVARIANT *dbv);
	size_t packDB(const CHAR *pszModule, const CHAR *pszSetting, size_t *cbRew = nullptr);
	size_t packDBList(const CHAR *pszModule, const CHAR *pszSetting, MIRANDASERVICE GetList, uint8_t bSigned = FALSE, size_t *cbRew = nullptr);

	void writeLine(const CHAR *szSet, size_t *cbRew = nullptr);
	void writeLineEncoded(const CHAR *szSet, size_t *cbRew = nullptr);
	int  readLine(LPSTR szVCFSetting, uint16_t cchSetting);

public:
	CVCardFileVCF();

	uint8_t Open(MCONTACT hContact, const wchar_t *pszFileName, const wchar_t *pszMode);
	void Close(void);
	uint8_t Export(uint8_t bExportUtf);
	uint8_t Import();
};
