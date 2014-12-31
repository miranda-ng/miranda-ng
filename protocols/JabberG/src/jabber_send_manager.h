/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-08  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2008-09  Dmitriy Chervov
Copyright (ñ) 2012-15 Miranda NG project

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
		memset(this, 0, sizeof(*this));
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
		memset(this, 0, sizeof(CJabberSendPermanentInfo));
	}
	
	~CJabberSendPermanentInfo()
	{
		if (m_pUserDataFree)
			m_pUserDataFree(m_pUserData);
	}

	__forceinline int getPriority() const { return m_iPriority; }
};

class CJabberSendManager
{
protected:
	CJabberProto *ppro;
	mir_cs m_cs;
	OBJLIST<CJabberSendPermanentInfo> m_arHandlers;

public:
	CJabberSendManager(CJabberProto* proto);
	~CJabberSendManager();

	CJabberSendPermanentInfo* AddPermanentHandler(JABBER_SEND_HANDLER pHandler, void *pUserData = NULL, SEND_USER_DATA_FREE_FUNC pUserDataFree = NULL, int iPriority = JH_PRIORITY_DEFAULT);
	bool DeletePermanentHandler(CJabberSendPermanentInfo *pInfo);

	bool HandleSendPermanent(HXML node, ThreadData *pThreadData);
};

#endif
