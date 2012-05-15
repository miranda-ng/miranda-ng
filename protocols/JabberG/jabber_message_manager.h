/*

Jabber Protocol Plugin for Miranda IM
Copyright ( C ) 2002-04  Santithorn Bunchua
Copyright ( C ) 2005-08  George Hazan
Copyright ( C ) 2007     Maxim Mluhov
Copyright ( C ) 2008-09  Dmitriy Chervov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Revision       : $Revision: 7173 $
Last change on : $Date: 2008-01-28 15:32:35 +0200 $
Last change by : $Author: dchervov $

*/

#ifndef _JABBER_MESSAGE_MANAGER_H_
#define _JABBER_MESSAGE_MANAGER_H_

#include "jabber_xml.h"

struct CJabberProto;
typedef void ( CJabberProto::*JABBER_MESSAGE_PFUNC )( HXML messageNode, void *usedata );
typedef void ( *MESSAGE_USER_DATA_FREE_FUNC )( void *pUserData );

void  __stdcall replaceStr( char*& dest, const char* src );
void  __stdcall replaceStr( WCHAR*& dest, const WCHAR* src );

class CJabberMessageInfo;

typedef BOOL ( CJabberProto::*JABBER_PERMANENT_MESSAGE_HANDLER )( HXML messageNode, ThreadData *pThreadData, CJabberMessageInfo* pInfo );

#define JABBER_MESSAGE_PARSE_FROM					(1<<3)
#define JABBER_MESSAGE_PARSE_HCONTACT				((1<<4)|JABBER_MESSAGE_PARSE_FROM)
#define JABBER_MESSAGE_PARSE_TO						(1<<5)
#define JABBER_MESSAGE_PARSE_ID_STR					(1<<6)

class CJabberMessageInfo
{
protected:
	friend class CJabberMessageManager;
	JABBER_PERMANENT_MESSAGE_HANDLER m_pHandler;
	CJabberMessageInfo* m_pNext;

public:
	void *m_pUserData;
// parsed data
	int m_nMessageType;
	LPCTSTR m_szFrom;
	LPCTSTR m_szChildTagXmlns;
	LPCTSTR m_szChildTagName;
	HXML m_hChildNode;
	HANDLE m_hContact; 
	LPCTSTR m_szTo;
	LPCTSTR m_szId;

public:
	CJabberMessageInfo()
	{
		ZeroMemory(this, sizeof(*this));
	}
	~CJabberMessageInfo()
	{
	}
	int GetMessageType()
	{
		return m_nMessageType;
	}
	void* GetUserData()
	{
		return m_pUserData;
	}
	LPCTSTR GetFrom()
	{
		return m_szFrom;
	}
	LPCTSTR GetTo()
	{
		return m_szTo;
	}
	LPCTSTR GetIdStr()
	{
		return m_szId;
	}
	HANDLE GetHContact()
	{
		return m_hContact;
	}
	HXML GetChildNode()
	{
		return m_hChildNode;
	}
	LPCTSTR GetChildNodeName()
	{
		return m_szChildTagName;
	}
};

class CJabberMessagePermanentInfo
{
	friend class CJabberMessageManager;

	CJabberMessagePermanentInfo* m_pNext;

	JABBER_PERMANENT_MESSAGE_HANDLER m_pHandler;
	DWORD m_dwParamsToParse;
	int m_nMessageTypes;
	LPTSTR m_szXmlns;
	LPTSTR m_szTag;
	BOOL m_bAllowPartialNs;
	void *m_pUserData;
	MESSAGE_USER_DATA_FREE_FUNC m_pUserDataFree;
	int m_iPriority;
public:
	CJabberMessagePermanentInfo()
	{
		ZeroMemory(this, sizeof(CJabberMessagePermanentInfo));
	}
	~CJabberMessagePermanentInfo()
	{
		if ( m_pUserDataFree )
			m_pUserDataFree(m_pUserData);
		mir_free(m_szXmlns);
		mir_free(m_szTag);
	}
};

class CJabberMessageManager
{
protected:
	CJabberProto* ppro;
	CRITICAL_SECTION m_cs;
	CJabberMessagePermanentInfo* m_pPermanentHandlers;

public:
	CJabberMessageManager( CJabberProto* proto )
	{
		InitializeCriticalSection(&m_cs);
		m_pPermanentHandlers = NULL;
		ppro = proto;
	}
	~CJabberMessageManager()
	{
		Lock();
		CJabberMessagePermanentInfo *pInfo = m_pPermanentHandlers;
		while ( pInfo )
		{
			CJabberMessagePermanentInfo *pTmp = pInfo->m_pNext;
			delete pInfo;
			pInfo = pTmp;
		}
		m_pPermanentHandlers = NULL;
		Unlock();
		DeleteCriticalSection(&m_cs);
	}
	BOOL Start()
	{
		return TRUE;
	}
	BOOL Shutdown()
	{
		return TRUE;
	}
	void Lock()
	{
		EnterCriticalSection(&m_cs);
	}
	void Unlock()
	{
		LeaveCriticalSection(&m_cs);
	}
	CJabberMessagePermanentInfo* AddPermanentHandler(JABBER_PERMANENT_MESSAGE_HANDLER pHandler, int nMessageTypes, DWORD dwParamsToParse, const TCHAR* szXmlns, BOOL bAllowPartialNs, const TCHAR* szTag, void *pUserData = NULL, MESSAGE_USER_DATA_FREE_FUNC pUserDataFree = NULL, int iPriority = JH_PRIORITY_DEFAULT)
	{
		CJabberMessagePermanentInfo* pInfo = new CJabberMessagePermanentInfo();
		if (!pInfo)
			return NULL;

		pInfo->m_pHandler = pHandler;
		pInfo->m_nMessageTypes = nMessageTypes ? nMessageTypes : JABBER_MESSAGE_TYPE_ANY;
		replaceStr( pInfo->m_szXmlns, szXmlns );
		pInfo->m_bAllowPartialNs = bAllowPartialNs;
		replaceStr( pInfo->m_szTag, szTag );
		pInfo->m_dwParamsToParse = dwParamsToParse;
		pInfo->m_pUserData = pUserData;
		pInfo->m_pUserDataFree = pUserDataFree;
		pInfo->m_iPriority = iPriority;

		Lock();
		if (!m_pPermanentHandlers)
			m_pPermanentHandlers = pInfo;
		else
		{
			if (m_pPermanentHandlers->m_iPriority > pInfo->m_iPriority) {
				pInfo->m_pNext = m_pPermanentHandlers;
				m_pPermanentHandlers = pInfo;
			} else
			{
				CJabberMessagePermanentInfo* pTmp = m_pPermanentHandlers;
				while (pTmp->m_pNext && pTmp->m_pNext->m_iPriority <= pInfo->m_iPriority)
					pTmp = pTmp->m_pNext;
				pInfo->m_pNext = pTmp->m_pNext;
				pTmp->m_pNext = pInfo;
			}
		}
		Unlock();

		return pInfo;
	}
	BOOL DeletePermanentHandler(CJabberMessagePermanentInfo *pInfo)
	{ // returns TRUE when pInfo found, or FALSE otherwise
		Lock();
		if (!m_pPermanentHandlers)
		{
			Unlock();
			return FALSE;
		}
		if (m_pPermanentHandlers == pInfo) // check first item
		{
			m_pPermanentHandlers = m_pPermanentHandlers->m_pNext;
			delete pInfo;
			Unlock();
			return TRUE;
		} else
		{
			CJabberMessagePermanentInfo* pTmp = m_pPermanentHandlers;
			while (pTmp->m_pNext)
			{
				if (pTmp->m_pNext == pInfo)
				{
					pTmp->m_pNext = pTmp->m_pNext->m_pNext;
					delete pInfo;
					Unlock();
					return TRUE;
				}
				pTmp = pTmp->m_pNext;
			}
		}
		Unlock();
		return FALSE;
	}
	BOOL HandleMessagePermanent(HXML node, ThreadData *pThreadData);
	BOOL FillPermanentHandlers();
};

#endif
