//This file is part of HTTPServer a Miranda IM plugin
//Copyright (C)2002 Kennet Nielsen
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "Glob.h"

/////////////////////////////////////////////////////////////////////
// Member Function : FileTimeToUnixTime
// Type            : Global
// Parameters      : pft - ?
//                   t   - ?
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030826, 26 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void FileTimeToUnixTime(LPFILETIME pft, time_t* t) {
	LONGLONG ll = ((LONGLONG)pft->dwHighDateTime) << 32;
	ll = ll + pft->dwLowDateTime - 116444736000000000;
	*t = (time_t)(ll / 10000000);
}


/////////////////////////////////////////////////////////////////////
// Function        : nUnescapedURI
// Type            : Local
// Parameters      : pszURI - URI to convert buffer is modifyed !!
// Returns         : length of the Unescaped URI
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030911, 11 september 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

static int nUnescapedURI(TCHAR *pszURI) {
	if (! pszURI)
		return 0;

	TCHAR *pszOrigURI = pszURI;
	int sumb;
	int more = -1;
	TCHAR *pszCurInsert = pszURI;

	for (; *pszURI && pszURI[0] != ' ' ; pszURI++) {
		int nNewChar;
		if (pszURI[0] == '%') {
			// we need to unescape the char
			if (pszURI[1] == NULL || pszURI[2] == NULL || pszURI[1] == ' ' || pszURI[2] == ' ')
				break; // handles error like "%2 " or "%a"

			if (_stscanf(&pszURI[1], _T("%2X"), &nNewChar) != 1) {
				// can't convert to number
				pszURI += 2;
				continue; // we skip it !!!
			}
			pszURI += 2;
		}
		/*else if( pszURI[0] == '+' )
		{
		nNewChar = ' ';
		}*/
		else
			nNewChar = (int)tolower(pszURI[0]); // also make the request lowercase

		if (nNewChar == '\\')
			nNewChar = '/';

		if ((nNewChar & 0x80) == 0x00) {		// 0xxxxxxx (yields 7 bits)
			more = -1; // this is to avoid sequence like %C3k%A6 will end up beeing "kæ" insted it will be "k"
			*pszCurInsert = (char)nNewChar;
			pszCurInsert++;
			continue;
		}

		if ((nNewChar & 0xc0) == 0x80) {			// 10xxxxxx (continuation byte)
			sumb = (sumb << 6) | (nNewChar & 0x3f) ;	// Add 6 bits to sumb
			more--;
			if (more == 0) {
				*pszCurInsert = (char)sumb; // here we just throw away all the fine UTF-8 encoding
				pszCurInsert++;
			}
		} else if ((nNewChar & 0xe0) == 0xc0) {		// 110xxxxx (yields 5 bits)
			sumb = nNewChar & 0x1f;
			more = 1;				// Expect 1 more byte
		} else if ((nNewChar & 0xf0) == 0xe0) {		// 1110xxxx (yields 4 bits)
			sumb = nNewChar & 0x0f;
			more = 2;				// Expect 2 more bytes
		} else if ((nNewChar & 0xf8) == 0xf0) {		// 11110xxx (yields 3 bits)
			sumb = nNewChar & 0x07;
			more = 3;				// Expect 3 more bytes
		} else if ((nNewChar & 0xfc) == 0xf8) {		// 111110xx (yields 2 bits)
			sumb = nNewChar & 0x03;
			more = 4;				// Expect 4 more bytes
		} else /*if ((nNewChar & 0xfe) == 0xfc)*/
		{	// 1111110x (yields 1 bit)
			sumb = nNewChar & 0x01;
			more = 5;				// Expect 5 more bytes
		}
	}
	return (pszCurInsert - pszOrigURI);
}


/////////////////////////////////////////////////////////////////////
// Member Function : CLHttpUser
// Type            : Private / Public / Protected
// Parameters      : hCon  - ?
//                   stAdd - ?
//                   pcl   - ?
// Returns         :
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030918, 18 september 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

CLHttpUser::CLHttpUser(HANDLE hCon, in_addr stAdd) : CLShareUser(hCon, stAdd) {
	memset(apszParam, 0, sizeof(apszParam));
	hFile = INVALID_HANDLE_VALUE;
}


/////////////////////////////////////////////////////////////////////
// Member Function : CLHttpUser
// Type            : Private / Public / Protected
// Parameters      : None
// Returns         :
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 031124, 24 november 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

CLHttpUser::~CLHttpUser() {
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
}

/////////////////////////////////////////////////////////////////////
// Member Function : bReadGetParameters
// Type            : Private / Public / Protected
// Parameters      : pszRequest - ?
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030928, 28 september 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool CLHttpUser::bReadGetParameters(TCHAR *pszRequest) {
	bool bRet = true;
	for (; *pszRequest ; pszRequest++) {
		if (pszRequest[0] != '\n') {
			if (pszRequest[0] == '\r')
				pszRequest[0] = 0;
			continue;
		}
		pszRequest[0] = 0;
		pszRequest++;
		for (int nCur = 0; nCur < eLastParam ; nCur++) {
			int nLen = (int)_tcslen(szParmStr[nCur]);
			if (_tcsncmp(pszRequest, szParmStr[nCur], nLen) == 0) {
				if (apszParam[nCur]) {
					bRet = false;
					// already set !!
				} else {
					pszRequest += nLen;
					apszParam[nCur] = pszRequest;
					pszRequest += _tcscspn(pszRequest, _T("\r\n")) - 1;
					TCHAR *psz = pszRequest;
					while (*psz == ' ') {
						psz[0] = 0;// owerwrite ' ' or '\r' or '\n'
						psz--;
					}
				}
				break;
			}
		}
	}
	return bRet;
}

/////////////////////////////////////////////////////////////////////
// Member Function : SendError
// Type            : Private / Public / Protected
// Parameters      : iErrorCode     - ?
//                   pszError       - ?
//                   pszDescription - ?
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 031124, 24 november 2003
// Developer       : KN, Houdini
// Changed         : 21 January 2006 by Vampik
/////////////////////////////////////////////////////////////////////

void CLHttpUser::SendError(int iErrorCode, const TCHAR *pszError, const TCHAR *pszDescription)
{
	TCHAR szCurTime[100];
	time_t ltime;
	time(&ltime);
	_tcsftime(szCurTime, SIZEOF(szCurTime), _T("%a, %d %b %Y %H:%M:%S GMT"), gmtime(&ltime));

	if (!pszDescription)
		pszDescription = pszError;

	TCHAR szBuf[1000];
	DWORD dwBytesToWrite = mir_sntprintf(szBuf, SIZEOF(szBuf),
	    _T("HTTP/1.1 %i %s\r\n")
	    _T("Date: %s\r\n")
		_T("Server: MirandaWeb/%s\r\n")
		_T("Transfer-Encoding: chunked\r\n")
		_T("Content-Type: text/html; charset=iso-8859-1\r\n")
		_T("\r\n")
		_T("10f\r\n")
		_T("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n")
		_T("<HTML><HEAD>\n")
		_T("<TITLE>%i %s</TITLE>\n")
		_T("</HEAD><BODY>\n")
		_T("<H1>%s</H1>\n")
		_T("%s<P>\n")
		_T("<HR>\n")
		_T("<ADDRESS>MirandaWeb/%s</ADDRESS>\n")
		_T("</BODY></HTML>\n")
		_T("\r\n")
		_T("\r\n"),
	    iErrorCode, pszError, szCurTime, PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM), iErrorCode, pszError, pszError, pszDescription, PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM));

	Netlib_Send(hConnection, _T2A(szBuf), dwBytesToWrite, 0);
}

/////////////////////////////////////////////////////////////////////
// Member Function : SendRedir
// Type            : Private / Public / Protected
// Parameters      : iErrorCode     - ?
//                   pszError       - ?
//                   pszDescription - ?
//                   pszRedirect    - ?
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 21 january 2006
// Developer       : KN, Houdini, Vampik
/////////////////////////////////////////////////////////////////////

void CLHttpUser::SendRedir(int iErrorCode, const TCHAR *pszError, const TCHAR *pszDescription, const TCHAR *pszRedirect)
{
	TCHAR szCurrTime[100];
	time_t ltime;
	time(&ltime);
	_tcsftime(szCurrTime, SIZEOF(szCurrTime), _T("%a, %d %b %Y %H:%M:%S GMT"), gmtime(&ltime));

	if (!pszDescription)
		pszDescription = pszError;

	TCHAR szBuff[1000];
	DWORD dwBytesToWrite = mir_sntprintf(szBuff, SIZEOF(szBuff),
		_T("HTTP/1.1 %i %s\r\n")
		_T("Date: %s\r\n")
		_T("Server: MirandaWeb/%s\r\n")
		_T("Location: %s/\r\n")
		_T("Transfer-Encoding: chunked\r\n")
		_T("Content-Type: text/html; charset=iso-8859-1\r\n")
		_T("\r\n")
		_T("10f\r\n")
		_T("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n")
		_T("<HTML><HEAD>\n")
		_T("<TITLE>%i %s</TITLE>\n")
		_T("</HEAD><BODY>\n")
		_T("<H1>%s</H1>\n")
		_T("%s<P>\n")
		_T("<HR>\n")
		_T("<ADDRESS>MirandaWeb/%s</ADDRESS>\n")
		_T("</BODY></HTML>\n")
		_T("\r\n")
		_T("\r\n"),
	    iErrorCode, pszError, szCurrTime, PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM), pszRedirect, iErrorCode, pszError, pszError, pszDescription, PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM));

	Netlib_Send(hConnection, _T2A(szBuff), dwBytesToWrite, 0);
}

/////////////////////////////////////////////////////////////////////
// Function        : strmcat
// Type            : Local
// Parameters      : pszDest   - ?
//                   pszSrc    - ?
//                   MaxLength - ?
// Returns         :
// Description     : Fills pszDest until it is MaxLength long or
//                   pszSrc is finished - always appends a \0
// References      : -
// Remarks         : -
// Created         :
// Developer       : Houdini
/////////////////////////////////////////////////////////////////////

static void _tcsmcat(TCHAR *pszDest, const TCHAR *pszSrc, int iMaxLength)
{
	int iLength = 0;
	while (*pszDest != '\0') {
		pszDest++;
		iLength++;
	}

	while (iLength < iMaxLength - 1 && *pszSrc != '\0') {
		*pszDest = *pszSrc;
		pszDest++;
		pszSrc++;
		iLength++;
	}

	*pszDest = '\0';
}

/////////////////////////////////////////////////////////////////////
// Member Function : bProcessGetRequest
// Type            : Global
// Parameters      : hNewConnection - ?
//                   dwRemoteIP     - ?
//                   pszRequest     - ?
// Returns         : Returns true if a file was send
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030813, 13 august 2003
// Developer       : KN, Houdini
// Changed         : 27 January 2005 by Vampik
// Changed         : 21 January 2006 by Vampik
/////////////////////////////////////////////////////////////////////

bool CLHttpUser::bProcessGetRequest(TCHAR *pszRequest, bool bIsGetCommand) {
	//LogEvent("Request", pszRequest);

	int nUriLength = nUnescapedURI(pszRequest);
	if (nUriLength <= 0)
		return false;

	CLFileShareListAccess clCritSection;

	if (bShutdownInProgress)
		return false;

	static TCHAR szTempfile[MAX_PATH+1];
	szTempfile[0] = '\0';

	if (!bReadGetParameters(pszRequest)) {
		SendError(400, _T("Bad Request"));
		return false;
	}

	DWORD dwRemoteIP = ntohl(stAddr.S_un.S_addr);
	for (CLFileShareNode * pclCur = pclFirstNode; pclCur ; pclCur = pclCur->pclNext) {
		if ((pclCur->st.dwAllowedIP ^ dwRemoteIP) & pclCur->st.dwAllowedMask)
			continue; // Not an allowed IP address

		if (!pclCur->bIsDirectory() && pclCur->nGetSrvPathLen() != nUriLength)
			continue; // not the right length, quickly move on to the next.

		if (pclCur->bIsDirectory() ?
		    (_tcsncmp(pclCur->st.pszSrvPath, pszRequest, pclCur->nGetSrvPathLen() - 1) == 0) :
		    (_tcsncmp(pclCur->st.pszSrvPath, pszRequest, pclCur->nGetSrvPathLen()) == 0)) {
			/*OutputDebugString( "Request for file OK : ");
			OutputDebugString( pclCur->st.pszSrvPath );
			OutputDebugString( "\n" );*/

			static TCHAR szSrvPath[MAX_PATH+1];
			static TCHAR szRealPath[MAX_PATH+1];
			TCHAR *pszSrvPath  = pclCur->st.pszSrvPath;
			TCHAR *pszRealPath = pclCur->st.pszRealPath;

			if (pclCur->bIsDirectory()) {
				_tcscpy(szRealPath, pclCur->st.pszRealPath);
				_tcscpy(szSrvPath, pclCur->st.pszSrvPath);
				pszRealPath = szRealPath;
				pszSrvPath = szSrvPath;

				if (nUriLength > MAX_PATH)
					nUriLength = MAX_PATH;

				pszRequest[nUriLength] = '\0';

				if (pclCur->nGetSrvPathLen() - nUriLength == 1) {
					SendRedir(302, _T("Found"), _T("The document has moved"), pszRequest);
					return false;
				} else {
					_tcsmcat(pszRealPath, &pszRequest[pclCur->nGetSrvPathLen()], MAX_PATH);
					_tcsmcat(pszSrvPath,  &pszRequest[pclCur->nGetSrvPathLen()], MAX_PATH);
				}
				pszRequest[nUriLength] = ' ';

				// hacker protection - should be removed by the browser
				if (_tcsstr(pszRealPath, _T(".."))) {
					SendError(404, _T("Not Found"), _T("The requested URL was not found on this server."));
					return false;
				}

				TCHAR *pszTmp = pszRealPath;
				while (pszTmp = _tcschr(pszTmp, '/'))
					* pszTmp = '\\';

				hFile = CreateFile(pszRealPath, GENERIC_READ ,
				    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile == INVALID_HANDLE_VALUE) {
					if (pszSrvPath[_tcslen(pszSrvPath)-1] != '/') {
						_tcsmcat(pszRealPath, _T("\\"), MAX_PATH);
						_tcsmcat(pszSrvPath,  _T("/"), MAX_PATH);
					}

					// a directory with index.htm
					_tcsmcat(szRealPath, _T("index.htm"), MAX_PATH);

					hFile = CreateFile(pszRealPath, GENERIC_READ ,
					    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN, NULL);

					if (hFile == INVALID_HANDLE_VALUE) {
						// a directory with index.html
						_tcsmcat(szRealPath, _T("l"), MAX_PATH);

						hFile = CreateFile(pszRealPath, GENERIC_READ ,
						    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_HIDDEN, NULL);

						if (hFile == INVALID_HANDLE_VALUE) {
							// generate directory index in temporary file
							if (*szTempfile == '\0') {
								GetTempPath(MAX_PATH, szTempfile);
								_tcsmcat(szTempfile, _T("\\HttpServerTemp"), MAX_PATH);
								_tcsmcat(szTempfile, pszSrvPath, MAX_PATH);
								TCHAR *pszTmp = szTempfile;
								while (pszTmp = _tcschr(pszTmp, '/'))
									* pszTmp = '~';
							}
							pszRealPath[_tcslen(pszRealPath) - 10] = '\0';

							// detecting browser function removed
							// every browser should support it by now
							bool BrowserSupportsXML = true; 
							  //(apszParam[eUserAgent] != NULL) &&
							  //  (strstr(apszParam[eUserAgent], "Firefox") ||
							  //   (strstr(apszParam[eUserAgent], "MSIE") && !strstr(apszParam[eUserAgent], "Opera")));

							if ((indexCreationMode == INDEX_CREATION_XML ||
							     (indexCreationMode == INDEX_CREATION_DETECT && BrowserSupportsXML)) &&
							    bCreateIndexXML(pszRealPath, szTempfile, pszSrvPath, dwRemoteIP)) {
								hFile = CreateFile(szTempfile, GENERIC_READ ,
								    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

								_tcscpy(szRealPath, _T("a.xml")); // restore .xml for mime type
							} else if ((indexCreationMode == INDEX_CREATION_HTML ||
							    indexCreationMode == INDEX_CREATION_DETECT) &&
							    bCreateIndexHTML(pszRealPath, szTempfile, pszSrvPath, dwRemoteIP)) {
								hFile = CreateFile(szTempfile, GENERIC_READ,
								    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

								_tcscpy(szRealPath, _T("a.html")); // restore .html for mime type
							} else {
								continue;
							}
						} else {
							_tcsmcat(pszSrvPath, _T("index.html"), MAX_PATH);
							szTempfile[0] = '\0';
						}
					} else {
						_tcsmcat(pszSrvPath, _T("index.htm"), MAX_PATH);
						szTempfile[0] = '\0';
					}
				} else {
					szTempfile[0] = '\0';
				}
			} else {
				hFile = CreateFile(pszRealPath, GENERIC_READ ,
				    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile == INVALID_HANDLE_VALUE) {
					SendError(404, _T("Not Found"), _T("HTTP server failed to open local file"));
					return false;
				}
			}

			_tcscpy(this->szCurrentDLSrvPath, pszSrvPath);

			DWORD nDataSize = GetFileSize(hFile, NULL);
			dwTotalSize = nDataSize;

			FILETIME stFileTime;
			GetFileTime(hFile, NULL, NULL, &stFileTime);

			TCHAR szCurTime[100];
			time_t ltime;
			time(&ltime);
			_tcsftime(szCurTime, SIZEOF(szCurTime), _T("%a, %d %b %Y %H:%M:%S GMT"), gmtime(&ltime));

			TCHAR szFileTime[100];
			FileTimeToUnixTime(&stFileTime, &ltime);
			_tcsftime(szFileTime, SIZEOF(szFileTime), _T("%a, %d %b %Y %H:%M:%S GMT"), gmtime(&ltime));
			
			if (apszParam[eIfModifiedSince] && _tcscmp(apszParam[eIfModifiedSince], szFileTime) == 0) {
				SendError(304, _T("Not Modified"));
				return true;
			}
	
			// we found match send file !!
			if (bIsGetCommand) {
				if (! pclCur->bAddUser(this)) {
					SendError(403, _T("Forbidden"), _T("Access has been denied because there are too many connections"));
					return false;
				}
			}

			if (*(ULONG*)(&stAddr) != 0x0100007F && // do not show popup of 127.0.0.1
			    _tcsstr(pszRealPath, _T("\\@")) == NULL) { // and of shares which start with an @
				ShowPopupWindow(_A2T(inet_ntoa(stAddr)), pszSrvPath);
			}

			clCritSection.Unlock();

			DWORD dwFileStart = 0;
			DWORD dwDataToSend = nDataSize;

			TCHAR szETag[50];
			{
				int nETagLen = mir_sntprintf(szETag, SIZEOF(szETag), _T("\"%x-%x-%x\""),
				    nDataSize, stFileTime.dwHighDateTime, stFileTime.dwLowDateTime);

				if (!apszParam[eIfRange] || (_tcsncmp(szETag, apszParam[eIfRange], nETagLen) == 0)) {
					TCHAR *pszRange = apszParam[eRange];
					if (pszRange) {
						if (_tcsncmp(pszRange, _T("bytes="), _tcslen(_T("bytes="))) == 0) {
							pszRange += 6;
							// Do resume !!!
							TCHAR *pszEnd;
							if (pszRange[0] == '-') {
								// its a suffix-byte-range-spec
								DWORD dwLen = _tcstol(pszRange + 1, &pszEnd, 10);
								if (dwLen < nDataSize)
									dwFileStart = nDataSize - dwLen;
							} else {
								DWORD dwLen = _tcstol(pszRange, &pszEnd, 10);
								if (*pszEnd == '-' && dwLen < nDataSize) {
									dwFileStart = dwLen;
									pszRange = pszEnd + 1;
									if (*pszRange != 0) {
										dwLen = _tcstol(pszEnd + 1, &pszEnd, 10);
										if (dwLen > dwFileStart)
											dwDataToSend = (dwLen + 1) - dwFileStart;
										else
											dwFileStart = 0;
									}
								} else {
									SendError(400, _T("Bad Request"));
									return false;
								}
							}
						}
					}
				}
			}

			if (dwFileStart >= nDataSize)
				dwFileStart = 0;

			if (dwFileStart + dwDataToSend >= nDataSize)
				dwDataToSend = nDataSize - dwFileStart;


			DWORD dwBytesToWrite = 0;
			// To optimize send speed it it ideal to always send larges size packages
			// But this size depended on network media but on Ethernet it is 1518 bytes.
			// Ethernet, IP and TCP headers use some of this space and leaves 1460 bytes
			// for data transfer.
			// We will use a multiply of this to always send optimal sized packages.
			TCHAR szBuf[1460 * 4];

			if (dwFileStart > 0 || dwDataToSend != nDataSize) {
				if (SetFilePointer(hFile, dwFileStart, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
					SendError(416, _T("Requested Range Not Satisfiable"));
					return true;
				}

				const TCHAR szHttpPartial[] = _T("HTTP/1.1 206 Partial Content\r\n")
					_T("Connection: Keep-Alive\r\n")
					_T("Date: %s\r\n")
					_T("Server: MirandaWeb/%s\r\n")
					_T("Accept-Ranges: bytes\r\n")
					_T("ETag: %s\r\n")
					_T("Content-Length: %d\r\n")
					_T("Content-Type: %s\r\n")
					_T("Content-Range: bytes %d-%d/%d\r\n")
					_T("Last-Modified: %s\r\n")
					_T("\r\n");

				dwBytesToWrite = mir_sntprintf(szBuf, SIZEOF(szBuf), szHttpPartial,
				    szCurTime,
				    PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
				    szETag,
				    dwDataToSend,
				    pszGetMimeType(pszRealPath),
				    dwFileStart,
				    (dwFileStart + dwDataToSend - 1),
				    nDataSize,
				    szFileTime);
			} else {
				const TCHAR szHttpOk[] = _T("HTTP/1.1 200 OK\r\n")
					_T("Connection: Keep-Alive\r\n")
					_T("Date: %s\r\n")
					_T("Server: MirandaWeb/%s\r\n")
					_T("Accept-Ranges: bytes\r\n")
					_T("ETag: %s\r\n")
					_T("Content-Length: %d\r\n")
					_T("Content-Type: %s\r\n")
					_T("Last-Modified: %s\r\n")
					_T("\r\n");

				dwBytesToWrite = mir_sntprintf(szBuf, SIZEOF(szBuf), szHttpOk,
				    szCurTime,
				    PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
				    szETag,
				    nDataSize,
				    pszGetMimeType(pszRealPath),
				    szFileTime);
			}

			Netlib_Send(hConnection, _T2A(szBuf), dwBytesToWrite, 0);

			if (bIsGetCommand) {
				static int nThreadCount = 0;
				nThreadCount++;

				DWORD dwLastUpdate = GetTickCount();
				DWORD dwLastCurrentDL = 0;
				/*

				dwLastDLSTickCount = dwCurTick;
				dwCurrentDL;*/

				//DWORD dwMaxSpeed = 8192;// Byte pr Sek
				//DWORD dwMaxSpeed = 20000;// Byte pr Sek
				//DWORD dwMaxSpeed = 163840;// Byte pr Sek
				//DWORD dwMaxSpeed = 4096;// Byte pr Sek

				DWORD dwLastResetTime = GetTickCount();
				int nMaxBytesToSend = nMaxUploadSpeed / nThreadCount;

				for (;;) {
					{
						DWORD dwCurTick = GetTickCount();
						if (dwCurTick - dwLastUpdate >= 1000) {
							dwSpeed = ((dwCurrentDL - dwLastCurrentDL) * 1000) / (dwCurTick - dwLastUpdate);
							dwLastUpdate = dwCurTick;
							dwLastCurrentDL = dwCurrentDL;
						}
					}

					if (nMaxUploadSpeed == 0) {
						Sleep(1000);
						continue;
					}

					bool bSpeedLimit = (nMaxUploadSpeed >= 0) && (bIsOnline || !bLimitOnlyWhenOnline);

					DWORD dwCurOpr = SIZEOF(szBuf);
					if (bSpeedLimit)
						dwCurOpr = min(nMaxBytesToSend, SIZEOF(szBuf));

					if (!ReadFile(hFile, szBuf, dwCurOpr, &dwBytesToWrite, NULL))
						break;

					if (dwBytesToWrite <= 0)
						break;

					if (dwCurrentDL + dwBytesToWrite > dwDataToSend)
						dwBytesToWrite = dwDataToSend - dwCurrentDL;

					if (bSpeedLimit)
						nMaxBytesToSend -= dwBytesToWrite;

					DWORD dwSend = Netlib_Send(hConnection, _T2A(szBuf), dwBytesToWrite, MSG_NODUMP);
					if (dwSend == SOCKET_ERROR)
						break;
					dwCurrentDL += dwSend;
					if (dwSend != dwBytesToWrite)
						break;

					if (dwCurrentDL >= dwDataToSend)
						break;

					if (bSpeedLimit && nMaxBytesToSend <= 0) {
						// we have reached the limmit
						DWORD dwTimeUsed = GetTickCount() - dwLastResetTime;
						if (dwTimeUsed < 1000)
							Sleep(1000 - dwTimeUsed);
						dwLastResetTime = GetTickCount();
						nMaxBytesToSend = nMaxUploadSpeed / nThreadCount;
					}
				}

				// file is always closed in destructor 
				if (szTempfile[0] != '\0') {
					// and here - since it is a temporary index which as to be deleted
					CloseHandle(hFile);
					hFile = INVALID_HANDLE_VALUE;

					DeleteFile(szTempfile);
				}
				clCritSection.Lock();
				nThreadCount--;

				bool bNeedToWriteConfig = false;

				if (dwCurrentDL == nDataSize) {
					if (pclCur->st.nMaxDownloads > 0) {
						pclCur->st.nMaxDownloads--;
						bNeedToWriteConfig = true;
					}
				}

				pclCur->bRemoveUser(this);

				// nMaxDownloads can have been decreesed by another thread.
				// Therefore we test it even if we did'en decreese it
				if (pclCur->st.nMaxDownloads == 0 && !pclCur->bAnyUsers()) {
					CLFileShareNode **pclPrev = &pclFirstNode;
					for (CLFileShareNode * pcl = pclFirstNode ; pcl ; pcl = pcl->pclNext) {
						if (pcl == pclCur) {
							*pclPrev = pclCur->pclNext;
							ShowPopupWindow(TranslateT("Share removed"), pclCur->st.pszSrvPath, RGB(255, 189, 189));
							delete pclCur;
							bNeedToWriteConfig = true;
							break;
						}
						pclPrev = &pcl->pclNext;
					}
				}

				if (bNeedToWriteConfig) {
					bWriteConfigurationFile();
				}
			}

			return true;
		}
	}


#ifdef _DEBUG
	OutputDebugString(_T("###########   Request Failed   ###########\n"));
	OutputDebugString(pszRequest);
#endif

	pszRequest[nUriLength] = 0;

	if ((nUriLength != 12 || _tcsncmp(pszRequest, _T("/favicon.ico"), nUriLength)) &&  // do not show popup of favicon.ico
	    *(ULONG*)(&stAddr) != 0x0100007F) { // do not show popup of 127.0.0.1
		ShowPopupWindow(_A2T(inet_ntoa(stAddr)), pszRequest, RGB(255, 189, 189));
	}

	SendError(404, _T("Not Found"), _T("The requested URL was not found on this server."));

	return false;
}


void CLHttpUser::HandleNewConnection()
{
	
/*
 {
 SOCKET s = CallService(MS_NETLIB_GETSOCKET, (WPARAM) hConnection, 0);
 sockaddr_in MyAddr;
 int nSize = sizeof( MyAddr );
 getsockname( s, (sockaddr*)&MyAddr, &nSize );
 ShowPopupWindow( "My IP address", inet_ntoa( MyAddr.sin_addr ) );
 //OutputDebugString(  );
 }*/
	/*
	{
	LINGER li;
	int nLenght = sizeof( li );
	int ret = getsockopt( s, IPPROTO_TCP, SO_LINGER, (char *)&li, &nLenght );
	if( ret )
	{
	DWORD error = WSAGetLastError();
	if( error
	WSANOTINITIALISED
	WSAENETDOWN The network subsystem has failed.
	WSAEFAULT One of the optval or the optlen parameters is not a valid part of the user address space, or the optlen parameter is too small.
	WSAEINPROGRESS A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
	WSAEINVAL The level parameter is unknown or invalid.
	WSAENOPROTOOPT The option is unknown or unsupported by the indicated protocol family.
	WSAENOTSOCK
	}
	nLenght = sizeof( li );
	li.l_onoff = 1;
	li.l_linger = 0;// time is default
	ret = setsockopt( s, IPPROTO_TCP, SO_LINGER, (const char *)&li, nLenght );
	if( ret )
	{
	// error
	}
	int nLenght = sizeof( li );
	int ret = getsockopt( s, IPPROTO_TCP, SO_LINGER, (char *)&li, &nLenght );
	}
	*/

	TCHAR szBuf[1000];
	int nCurPos = 0;
	while (SIZEOF(szBuf) - nCurPos > 10 && !bShutdownInProgress) {
		int nBytesRead = Netlib_Recv(hConnection, _T2A(&szBuf[nCurPos]), SIZEOF(szBuf) - nCurPos, 0);
		if (! nBytesRead) {
			// socket closed gracefully
			break;
		}
		if (nBytesRead == SOCKET_ERROR) {
			// socket closed with error
			// WSAGetLastError();
			break;
		}
		int nOldCurPos = nCurPos;
		nCurPos += nBytesRead;
		if (nCurPos <= 5)
			continue;

		bool bIsGetCommand = _tcsncmp(szBuf, _T("GET "), _tcslen(_T("GET "))) == 0;
		if (!bIsGetCommand && _tcsncmp(szBuf, _T("HEAD "), _tcslen(_T("HEAD "))) != 0) {
			SendError(501, _T("Not Implemented"));
			break; // We only support GET and HEAD commands !!
		}

		if (nOldCurPos < 4)
			nOldCurPos = 4; // scan forward from end of "GET " to locate the next space ' '

		bool bBreakWhile = false;
		for (; nOldCurPos < nCurPos; nOldCurPos++) {
			if (szBuf[nOldCurPos-2] == '\n' && szBuf[nOldCurPos-1] == '\r' && szBuf[nOldCurPos] == '\n') {
				// we have a walid request !!! scan to see if we have this file
				szBuf[nOldCurPos] = NULL;
				bProcessGetRequest(&szBuf[bIsGetCommand?4:5], bIsGetCommand);

				bBreakWhile = true;
				break;
			}
		}
		if (bBreakWhile)
			break;
	}
}
