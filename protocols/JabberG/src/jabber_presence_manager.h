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

#ifndef _JABBER_PRESENCE_MANAGER_H_
#define _JABBER_PRESENCE_MANAGER_H_

#include "jabber_xml.h"

struct CJabberProto;
typedef void (CJabberProto::*JABBER_PRESENCE_PFUNC)(HXML node, void *usedata);
typedef void (*PRESENCE_USER_DATA_FREE_FUNC)(void *pUserData);

class CJabberPresenceInfo;

typedef BOOL (CJabberProto::*JABBER_PRESENCE_HANDLER)(HXML node, ThreadData *pThreadData, CJabberPresenceInfo* pInfo);

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
		memset(this, 0, sizeof(*this));
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

	JABBER_PRESENCE_HANDLER m_pHandler;
	void *m_pUserData;
	PRESENCE_USER_DATA_FREE_FUNC m_pUserDataFree;
	int m_iPriority;

public:
	CJabberPresencePermanentInfo()
	{
		memset(this, 0, sizeof(CJabberPresencePermanentInfo));
	}

	~CJabberPresencePermanentInfo()
	{
		if (m_pUserDataFree)
			m_pUserDataFree(m_pUserData);
	}

	__forceinline int getPriority() const { return m_iPriority; }
};

class CJabberPresenceManager
{
protected:
	CJabberProto *ppro;
	mir_cs m_cs;
	OBJLIST<CJabberPresencePermanentInfo> m_arHandlers;

public:
	CJabberPresenceManager(CJabberProto*);
	~CJabberPresenceManager();

	CJabberPresencePermanentInfo* AddPermanentHandler(JABBER_PRESENCE_HANDLER pHandler, void *pUserData = NULL, PRESENCE_USER_DATA_FREE_FUNC pUserDataFree = NULL, int iPriority = JH_PRIORITY_DEFAULT);
	bool DeletePermanentHandler(CJabberPresencePermanentInfo *pInfo);
	
	bool HandlePresencePermanent(HXML node, ThreadData *pThreadData);
};

#endif
