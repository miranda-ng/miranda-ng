/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-08  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2008-09  Dmitriy Chervov
Copyright (c) 2012-14  Miranda NG project

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _JABBER_SEND_MANAGER_H_
#define _JABBER_SEND_MANAGER_H_

#include "jabber_xml.h"

struct CJabberProto;
typedef void (CJabberProto::*JABBER_SEND_PFUNC)(HXML node, void *usedata);
typedef void (*SEND_USER_DATA_FREE_FUNC)(void *pUserData);

class CJabberSendInfo;

typedef BOOL (CJabberProto::*JABBER_SEND_HANDLER)(HXML node, ThreadData *pThreadData, CJabberSendInfo* pInfo);

class CJabberSendInfo
{
protected:
	friend class CJabberSendManager;
	JABBER_SEND_HANDLER m_pHandler;
	CJabberSendInfo* m_pNext;

public:
	void *m_pUserData;

	CJabberSendInfo()
	{
		ZeroMemory(this, sizeof(*this));
	}
	~CJabberSendInfo()
	{
	}
	void* GetUserData()
	{
		return m_pUserData;
	}
};

class CJabberSendPermanentInfo
{
	friend class CJabberSendManager;

	CJabberSendPermanentInfo* m_pNext;

	JABBER_SEND_HANDLER m_pHandler;
	void *m_pUserData;
	SEND_USER_DATA_FREE_FUNC m_pUserDataFree;
	int m_iPriority;
public:
	CJabberSendPermanentInfo()
	{
		ZeroMemory(this, sizeof(CJabberSendPermanentInfo));
	}
	~CJabberSendPermanentInfo()
	{
		if (m_pUserDataFree)
			m_pUserDataFree(m_pUserData);
	}
};

class CJabberSendManager
{
protected:
	CJabberProto *ppro;
	mir_cs m_cs;
	CJabberSendPermanentInfo* m_pPermanentHandlers;

public:
	CJabberSendManager(CJabberProto* proto)
	{
		m_pPermanentHandlers = NULL;
		ppro = proto;
	}
	~CJabberSendManager()
	{
		CJabberSendPermanentInfo *pInfo = m_pPermanentHandlers;
		while (pInfo) {
			CJabberSendPermanentInfo *pTmp = pInfo->m_pNext;
			delete pInfo;
			pInfo = pTmp;
		}
		m_pPermanentHandlers = NULL;
	}
	BOOL Start()
	{
		return TRUE;
	}
	BOOL Shutdown()
	{
		return TRUE;
	}
	CJabberSendPermanentInfo* AddPermanentHandler(JABBER_SEND_HANDLER pHandler, void *pUserData = NULL, SEND_USER_DATA_FREE_FUNC pUserDataFree = NULL, int iPriority = JH_PRIORITY_DEFAULT)
	{
		CJabberSendPermanentInfo* pInfo = new CJabberSendPermanentInfo();
		if (!pInfo)
			return NULL;

		pInfo->m_pHandler = pHandler;
		pInfo->m_pUserData = pUserData;
		pInfo->m_pUserDataFree = pUserDataFree;
		pInfo->m_iPriority = iPriority;

		mir_cslock lck(m_cs);
		if (!m_pPermanentHandlers)
			m_pPermanentHandlers = pInfo;
		else {
			if (m_pPermanentHandlers->m_iPriority > pInfo->m_iPriority) {
				pInfo->m_pNext = m_pPermanentHandlers;
				m_pPermanentHandlers = pInfo;
			}
			else {
				CJabberSendPermanentInfo* pTmp = m_pPermanentHandlers;
				while (pTmp->m_pNext && pTmp->m_pNext->m_iPriority <= pInfo->m_iPriority)
					pTmp = pTmp->m_pNext;
				pInfo->m_pNext = pTmp->m_pNext;
				pTmp->m_pNext = pInfo;
			}
		}
		return pInfo;
	}
	BOOL DeletePermanentHandler(CJabberSendPermanentInfo *pInfo)
	{ // returns TRUE when pInfo found, or FALSE otherwise
		mir_cslock lck(m_cs);
		if (!m_pPermanentHandlers)
			return FALSE;

		if (m_pPermanentHandlers == pInfo) { // check first item
			m_pPermanentHandlers = m_pPermanentHandlers->m_pNext;
			delete pInfo;
			return TRUE;
		}

		CJabberSendPermanentInfo* pTmp = m_pPermanentHandlers;
		while (pTmp->m_pNext) {
			if (pTmp->m_pNext == pInfo) {
				pTmp->m_pNext = pTmp->m_pNext->m_pNext;
				delete pInfo;
				return TRUE;
			}
			pTmp = pTmp->m_pNext;
		}
		return FALSE;
	}
	BOOL HandleSendPermanent(HXML node, ThreadData *pThreadData);
	BOOL FillPermanentHandlers();
};

#endif
