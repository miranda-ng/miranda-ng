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

#ifndef FILE_SHARE_NODE_H
#define FILE_SHARE_NODE_H

#include <windows.h>
#include "m_HTTPServer.h"

class CLShareUser {
public:
	CLShareUser(HANDLE hCon, in_addr stAdd);
	virtual ~CLShareUser();
	CLShareUser * pclNext;
	in_addr stAddr;
	DWORD dwCurrentDL;
	DWORD dwTotalSize;
	char  szCurrentDLSrvPath[MAX_PATH];
	virtual DWORD dwGetDownloadSpeed();
	virtual bool bCloseTransfers() = NULL;
	virtual const char * pszCustomInfo() = NULL;
	void CloseSocket();
protected:
	HANDLE hConnection;
	DWORD dwSpeed;
};

class CLFileShareNode {
public://, 	DWORD dwAllowedIP, DWORD dwAllowedMask, int nMaxDownloads );
	CLFileShareNode(char * pszSrvPath, char * pszRealPath);
	CLFileShareNode(STFileShareInfo * pstInfo);
	~CLFileShareNode();
	bool bSetPaths(char * pszSrvPath, char * pszRealPath);
	bool bSetInfo(STFileShareInfo * pstInfo);
	int nGetSrvPathLen() {
		return st.dwMaxSrvPath -1;
	}
	bool bIsDirectory() {
		return st.pszSrvPath[st.dwMaxSrvPath-2] == '/';
	}

	STFileShareInfo st;
	char *pszOrigRealPath;
	CLFileShareNode * pclNext;
	//CLHttpUser * pclAddHttpUser( HANDLE hConnection, in_addr stAddr );
	//bool bRemoveHttpUser( CLHttpUser * pclUser );
	bool bAddUser(CLShareUser * pclUser);
	bool bRemoveUser(CLShareUser * pclUser);
	bool bAnyUsers() {
		return pclCurrentUsers != 0;
	}
	void CloseAllTransfers();
	CLShareUser *pclGetUsers() {
		return pclCurrentUsers;
	}
private:
	CLShareUser * pclCurrentUsers;

};

extern CLFileShareNode * pclFirstNode;
extern mir_cs csFileShareListAccess;

class CLFileShareListAccess {
	bool bLocked;
public:
	CLFileShareListAccess() {
		bLocked = false;
		Lock();
	}
	~CLFileShareListAccess() {
		Unlock();
	}
	void Lock() {
		if (bLocked)
			return;
		mir_cslock lck(csFileShareListAccess);
		bLocked = true;
	}
	void Unlock() {
		if (!bLocked)
			return;
		bLocked = false;
	}

};

#endif