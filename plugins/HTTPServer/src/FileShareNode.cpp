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

CLFileShareNode * pclFirstNode = NULL;
mir_cs csFileShareListAccess;



/////////////////////////////////////////////////////////////////////
// Member Function : CLShareUser
// Type            : Private / Public / Protected
// Parameters      : None
// Returns         :
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030918, 18 september 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

CLShareUser::CLShareUser(HANDLE hCon, in_addr stAdd) {
	hConnection = hCon;
	stAddr = stAdd;
	pclNext = NULL;
	dwTotalSize = 0;
	dwCurrentDL = 0;
	dwSpeed = 0;
}

/////////////////////////////////////////////////////////////////////
// Member Function : ~CLShareUser
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

CLShareUser::~CLShareUser() {
	if (hConnection) {
		CloseSocket();
		Netlib_CloseHandle(hConnection);
	}
}


/////////////////////////////////////////////////////////////////////
// Member Function : CloseSend
// Type            : Private / Public / Protected
// Parameters      : None
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 031124, 24 november 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void CLShareUser::CloseSocket() {
	SOCKET s = CallService(MS_NETLIB_GETSOCKET, (WPARAM) hConnection, 0);
	if (s != INVALID_SOCKET) {
		shutdown(s, SD_SEND);
		int nBytesRead;
		do {
			char szBuf[100];
			nBytesRead = Netlib_Recv(hConnection, szBuf, sizeof(szBuf), 0);
		} while (nBytesRead && nBytesRead != SOCKET_ERROR);
		//shutdown( s, SD_RECEIVE );
	}
}

/////////////////////////////////////////////////////////////////////
// Member Function : dwGetDownloadSpeed
// Type            : Private / Public / Protected
// Parameters      : None
// Returns         : DWORD
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030918, 18 september 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

DWORD CLShareUser::dwGetDownloadSpeed() {
	return dwSpeed;
}



/////////////////////////////////////////////////////////////////////
// Member Function : CLFileShareNode
// Type            : Private / Public / Protected
// Parameters      : NULL - ?
// Returns         :
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030826, 26 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

CLFileShareNode::CLFileShareNode(char * pszSrvPath, char * pszRealPath) {
	memset(&st, 0, sizeof(STFileShareInfo));
	st.lStructSize = sizeof(STFileShareInfo);
	pclNext = NULL;
	pclCurrentUsers = NULL;

	bSetPaths(pszSrvPath, pszRealPath);
}

/////////////////////////////////////////////////////////////////////
// Member Function : CLFileShareNode
// Type            : Global
// Parameters      : pstInfo - ?
// Returns         :
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030829, 29 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

CLFileShareNode::CLFileShareNode(STFileShareInfo * pstInfo) {
	memset(&st, 0, sizeof(STFileShareInfo));
	st.lStructSize = sizeof(STFileShareInfo);
	pclNext = NULL;
	pclCurrentUsers = NULL;

	bSetInfo(pstInfo);
}

/////////////////////////////////////////////////////////////////////
// Member Function : ~CLFileShareNode
// Type            : Private / Public / Protected
// Parameters      : None
// Returns         :
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030826, 26 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

CLFileShareNode::~CLFileShareNode() {
	delete [] st.pszSrvPath;
	delete [] st.pszRealPath;

	CLShareUser * pclCur = pclCurrentUsers;
	while (pclCur) {
		CLShareUser * pclNext = pclCur->pclNext;
		delete pclCur;
		pclCur = pclNext;
	}
}


/////////////////////////////////////////////////////////////////////
// Member Function : bSetPaths
// Type            : Private / Public / Protected
// Parameters      : pszSrvPath  - ?
//                   pszRealPath - ?
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030829, 29 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool CLFileShareNode::bSetPaths(char * pszSrvPath, char * pszRealPath) {
	/* This might be a problem !!
	if( nDownloadsInProgress > 0 )
		return false;
	*/

	if (!pszSrvPath || !pszRealPath)
		return false;

	delete [] st.pszSrvPath;
	delete [] st.pszRealPath;

	st.dwMaxSrvPath = (int)mir_strlen(pszSrvPath) + 1;
	st.pszSrvPath = new char[ st.dwMaxSrvPath ];
	mir_strcpy(st.pszSrvPath, pszSrvPath);

	int nRealLen = (int)mir_strlen(pszRealPath);
	if (nRealLen <= 2 || !(pszRealPath[1] == ':' ||
	    (pszRealPath[0] == '\\' && pszRealPath[1] == '\\'))) {
		// Relative path
		// we will prepend plugin path to avoid problems
		st.dwMaxRealPath = nPluginPathLen  + nRealLen + 1;
		st.pszRealPath = new char[ st.dwMaxRealPath ];
		mir_strcpy(st.pszRealPath, szPluginPath);
		pszOrigRealPath = &st.pszRealPath[nPluginPathLen];
	} else {
		st.dwMaxRealPath = nRealLen + 1;
		st.pszRealPath = new char[ st.dwMaxRealPath ];
		pszOrigRealPath = st.pszRealPath;
	}
	mir_strcpy(pszOrigRealPath, pszRealPath);
	return true;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bSetInfo
// Type            : Private / Public / Protected
// Parameters      : pstInfo - ?
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030829, 29 august 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool CLFileShareNode::bSetInfo(STFileShareInfo * pstInfo) {
	if (! bSetPaths(pstInfo->pszSrvPath, pstInfo->pszRealPath))
		return false;
	if (pstInfo->nMaxDownloads < -1)
		return false;

	st.dwAllowedIP = pstInfo->dwAllowedIP;
	st.dwAllowedMask = pstInfo->dwAllowedMask;
	st.nMaxDownloads = pstInfo->nMaxDownloads;
	st.dwOptions = pstInfo->dwOptions;
	return true;
}

/*
/////////////////////////////////////////////////////////////////////
// Member Function : bAddHttpUser
// Type            : Private / Public / Protected
// Parameters      : hConnection - ?
//                   stAddr      - ?
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030902, 02 september 2003
// Developer       : KN, Houdini
/////////////////////////////////////////////////////////////////////

CLHttpUser * CLFileShareNode::pclAddHttpUser( HANDLE hConnection, in_addr stAddr )
{
	CLHttpUser * pclCur = pclCurrentUsers;
	while( pclCur )
	{
		if( pclCur->hConnection == hConnection )
		{
			return NULL;
		}
		pclCur = pclCur->pclNext;
	}
	pclCurrentUsers = new CLHttpUser( hConnection, stAddr, pclCurrentUsers );
	UpdateStatisticsView();
	return pclCurrentUsers;
}
*/

bool CLFileShareNode::bAddUser(CLShareUser * pclUser) {
	// deny access
	if (bIsOnline || !bLimitOnlyWhenOnline) {
		int nConnectionCount = 0;
		int nUserConnectionCount = 0;

		// iterate through all shares
		CLFileShareNode * pclShare = pclFirstNode;
		while (pclShare) {
			// iterate through its users
			CLShareUser * pclCur = pclShare->pclCurrentUsers;
			while (pclCur) {
				//mir_strcmp(pclCur->szCurrentDLSrvPath, pclUser->szCurrentDLSrvPath) == 0) // same file
				if (memcmp(&pclCur->stAddr, &pclUser->stAddr, sizeof(in_addr)) == 0)  // same IP
					nUserConnectionCount++;

				nConnectionCount++;

				pclCur = pclCur->pclNext;
			}
			pclShare = pclShare->pclNext;
		}

		if (nMaxConnectionsTotal >= 0)
			if (nConnectionCount + 1 > nMaxConnectionsTotal)
				return false;

		if (nMaxConnectionsPerUser >= 0)
			if (nUserConnectionCount + 1 > nMaxConnectionsPerUser)
				return false;
	}

	pclUser->pclNext = pclCurrentUsers;
	pclCurrentUsers = pclUser;
	UpdateStatisticsView();
	return true;
}

/////////////////////////////////////////////////////////////////////
// Member Function : bRemoveUser
// Type            : Private / Public / Protected
// Parameters      : pclUser - ?
// Returns         : Returns true if
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030918, 18 september 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

bool CLFileShareNode::bRemoveUser(CLShareUser * pclUser) {
	CLShareUser **pclPrev = &pclCurrentUsers;
	CLShareUser * pclCur = pclCurrentUsers;
	while (pclCur) {
		if (pclCur == pclUser) {
			*pclPrev = pclCur->pclNext;
			UpdateStatisticsView();
			return true;
		}
		pclPrev = &pclCur->pclNext;
		pclCur = pclCur->pclNext;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////
// Member Function : CloseAllTransfers
// Type            : Private / Public / Protected
// Parameters      : None
// Returns         : void
// Description     :
//
// References      : -
// Remarks         : -
// Created         : 030902, 02 september 2003
// Developer       : KN
/////////////////////////////////////////////////////////////////////

void CLFileShareNode::CloseAllTransfers() {
	CLShareUser * pclCur = pclCurrentUsers;
	while (pclCur) {
		pclCur->CloseSocket();
		pclCur = pclCur->pclNext;
	}
}