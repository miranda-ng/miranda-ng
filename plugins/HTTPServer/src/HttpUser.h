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

#ifndef HTTP_USER_H
#define HTTP_USER_H

#include <windows.h>
#include "m_HTTPServer.h"
#include "FileShareNode.h"

enum ENParamTypes {
	eFirstParam = 0,
	eRange = 0,
	eIfRange,
	eUnlessModifiedSince,
	eIfModifiedSince,
	eUserAgent,
	eHost,
	eLastParam
};

static TCHAR * szParmStr[eLastParam] = {
	_T("Range: "),
	_T("If-Range: "),
	_T("Unless-Modified-Since: "),
	_T("If-Modified-Since: "),
	_T("User-Agent: "),
	_T("Host: ")
};


class CLHttpUser : public CLShareUser {
public:
	CLHttpUser(HANDLE hCon, in_addr stAdd);
	virtual ~CLHttpUser();

	bool bReadGetParameters(TCHAR *pszRequest);

	//bool bSendFile( HANDLE hFile ,
	bool bCloseTransfers() {
		return true;
	}
	bool bProcessGetRequest(TCHAR *pszRequest, bool bIsGetCommand);
	const TCHAR * pszCustomInfo() {
		return apszParam[eUserAgent];
	}
	void HandleNewConnection();
private:
	HANDLE hFile;
	TCHAR *apszParam[eLastParam];

	void SendError(int iErrorCode, const TCHAR *pszError, const TCHAR *pszDescription = NULL);
	void SendRedir(int iErrorCode, const TCHAR *pszError, const TCHAR *pszDescription = NULL, const TCHAR *pszRedirect = NULL);
};

#endif