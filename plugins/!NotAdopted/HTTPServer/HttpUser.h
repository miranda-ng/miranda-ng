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

static char * szParmStr[eLastParam] = {
	"Range: ",
	"If-Range: ",
	"Unless-Modified-Since: ",
	"If-Modified-Since: ",
	"User-Agent: ",
	"Host: "
};


class CLHttpUser : public CLShareUser {
public:
	CLHttpUser(HANDLE hCon, in_addr stAdd);
	virtual ~CLHttpUser();

	bool bReadGetParameters(char * pszRequest);

	//bool bSendFile( HANDLE hFile ,
	bool bCloseTransfers() {
		return true;
	}
	bool bProcessGetRequest(char * pszRequest, bool bIsGetCommand);
	const char * pszCustomInfo() {
		return apszParam[eUserAgent];
	}
	void HandleNewConnection();
private:
	HANDLE hFile;
	char *apszParam[eLastParam];

	void SendError(int iErrorCode, const char * pszError, const char * pszDescription = NULL);
	void SendRedir(int iErrorCode, const char * pszError, const char * pszDescription = NULL, const char * pszRedirect = NULL);
};

#endif