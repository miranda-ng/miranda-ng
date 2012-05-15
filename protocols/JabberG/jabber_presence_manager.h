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

#ifndef _JABBER_PRESENCE_MANAGER_H_
#define _JABBER_PRESENCE_MANAGER_H_

#include "jabber_xml.h"

struct CJabberProto;
typedef void ( CJabberProto::*JABBER_PRESENCE_PFUNC )( HXML node, void *usedata );
typedef void ( *PRESENCE_USER_DATA_FREE_FUNC )( void *pUserData );

class CJabberPresenceInfo;

typedef BOOL ( CJabberProto::*JABBER_PRESENCE_HANDLER )( HXML node, ThreadData *pThreadData, CJabberPresenceInfo* pInfo );

class CJabberPresenceInfo
{
protected:
	friend class CJabberPresenceManager;
	JABBER_PRESENCE_HANDLER m_pHandler;
	CJabberPresenceInfo* m_pNext;

public:
	void *m_pUserData;

	CJabberPresenceInfo()
	{
		ZeroMemory(this, sizeof(*this));
	}
	~CJabberPresenceInfo()
	{
	}
	void* GetUserData()
	{
		return m_pUserData;
	}
};

class CJabberPresencePermanentInfo
{
	friend class CJabberPresenceManager;

	CJabberPresencePermanentInfo* m_pNext;

	JABBER_PRESENCE_HANDLER m_pHandler;
	void *m_pUserData;
	PRESENCE_USER_DATA_FREE_FUNC m_pUserDataFree;
	int m_iPriority;
public:
	CJabberPresencePermanentInfo()
	{
		ZeroMemory(this, sizeof(CJabberPresencePermanentInfo));
	}
	~CJabberPresencePermanentInfo()
	{
		if ( m_pUserDataFree )
			m_pUserDataFree(m_pUserData);
	}
};

class CJabberPresenceManager
{
protected:
	CJabberProto* ppro;
	CRITICAL_SECTION m_cs;
	CJabberPresencePermanentInfo* m_pPermanentHandlers;

public:
	CJabberPresenceManager( CJabberProto* proto )
	{
		InitializeCriticalSection(&m_cs);
		m_pPermanentHandlers = NULL;
		ppro = proto;
	}
	~CJabberPresenceManager()
	{
		Lock();
		CJabberPresencePermanentInfo *pInfo = m_pPermanentHandlers;
		while ( pInfo )
		{
			CJabberPresencePermanentInfo *pTmp = pInfo->m_pNext;
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
	CJabberPresencePermanentInfo* AddPermanentHandler(JABBER_PRESENCE_HANDLER pHandler, void *pUserData = NULL, PRESENCE_USER_DATA_FREE_FUNC pUserDataFree = NULL, int iPriority = JH_PRIORITY_DEFAULT)
	{
		CJabberPresencePermanentInfo* pInfo = new CJabberPresencePermanentInfo();
		if (!pInfo)
			return NULL;

		pInfo->m_pHandler = pHandler;
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
				CJabberPresencePermanentInfo* pTmp = m_pPermanentHandlers;
				while (pTmp->m_pNext && pTmp->m_pNext->m_iPriority <= pInfo->m_iPriority)
					pTmp = pTmp->m_pNext;
				pInfo->m_pNext = pTmp->m_pNext;
				pTmp->m_pNext = pInfo;
			}
		}
		Unlock();

		return pInfo;
	}
	BOOL DeletePermanentHandler(CJabberPresencePermanentInfo *pInfo)
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
			CJabberPresencePermanentInfo* pTmp = m_pPermanentHandlers;
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
	BOOL HandlePresencePermanent(HXML node, ThreadData *pThreadData);
	BOOL FillPermanentHandlers();
};

#endif
