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

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/ex_import/svc_ExImVCF.h $
Revision       : $Revision: 187 $
Last change on : $Date: 2010-09-08 16:05:54 +0400 (Ср, 08 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/

#pragma once

class CLineBuffer
{
private:
	PBYTE	_pVal;
	PBYTE	_pTok;
	size_t	_cbVal;
	size_t	_cbUsed;

	BOOLEAN _resizeBuf(const size_t cbReq);

public:
	CLineBuffer();
	~CLineBuffer();
	
	size_t operator = (const CHAR *szVal);

	size_t operator + (const CHAR *szVal);
	size_t operator + (const WCHAR *wszVal);
	size_t operator + (const CHAR cVal);
	size_t operator + (const BYTE bVal);
	size_t operator + (const SHORT sVal);
	size_t operator + (const WORD wVal);
	size_t operator + (const LONG lVal);
	size_t operator + (const DWORD dVal);

	size_t GetLength();
	LPCSTR GetBuffer();

	VOID TruncToLength(size_t cbLength);
	VOID Truncate(size_t count);
	VOID TruncateSMS();
	
	VOID fput(FILE *outfile);
	VOID fputEncoded(FILE *outFile);
	INT	fgetEncoded(FILE *inFile);

	size_t GetTokenFirst(const CHAR delim, CLineBuffer * pBuf);
	size_t GetTokenNext(const CHAR delim, CLineBuffer * pBuf);
	INT	 DBWriteTokenFirst(HANDLE hContact, const CHAR* pszModule, const CHAR* pszSetting, const CHAR delim);
	INT	 DBWriteTokenNext(HANDLE hContact, const CHAR* pszModule, const CHAR* pszSetting, const CHAR delim);
	INT	 DBWriteSettingString(HANDLE hContact, const CHAR* pszModule, const CHAR* pszSetting);
};

class CVCardFileVCF
{
private:
	CLineBuffer		 _clVal;
	FILE			*_pFile;
	HANDLE			 _hContact;
	const CHAR		*_pszBaseProto;
	WORD			 _cbRew;
	BOOLEAN			 _useUtf8;
	WORD			 _hasUtf8;

	size_t	packList(LPIDSTRLIST pList, UINT nList, INT iID, size_t *cbRew = NULL);
	BOOLEAN GetSetting(const CHAR *pszModule, const CHAR *pszSetting, DBVARIANT *dbv);
	size_t	packDB(const CHAR *pszModule, const CHAR *pszSetting, size_t *cbRew = NULL);
	size_t	packDBList(const CHAR *pszModule, const CHAR *pszSetting, MIRANDASERVICE GetList, BOOLEAN bSigned = FALSE, size_t *cbRew = NULL);
	
	VOID	writeLine(const CHAR *szSet, size_t *cbRew = NULL);
	VOID	writeLineEncoded(const CHAR *szSet, size_t *cbRew = NULL);
	INT		readLine(LPSTR szVCFSetting, WORD cchSetting);

public:
	CVCardFileVCF();

	BOOLEAN Open(HANDLE hContact, LPCSTR pszFileName, LPCSTR pszMode);
	VOID	Close(VOID);
	BOOLEAN Export(BOOLEAN bExportUtf);
	BOOLEAN Import();
};
