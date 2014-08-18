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

#ifndef _JABBER_MESSAGE_MANAGER_H_
#define _JABBER_MESSAGE_MANAGER_H_

#include "jabber_xml.h"

struct CJabberProto;
typedef void (CJabberProto::*JABBER_MESSAGE_PFUNC)(HXML messageNode, void *usedata);
typedef void (*MESSAGE_USER_DATA_FREE_FUNC)(void *pUserData);

class CJabberMessageInfo;

typedef BOOL (CJabberProto::*JABBER_PERMANENT_MESSAGE_HANDLER)(HXML messageNode, ThreadData *pThreadData, CJabberMessageInfo* pInfo);

#define JABBER_MESSAGE_PARSE_FROM					(1<<3)
#define JABBER_MESSAGE_PARSE_HCONTACT				((1<<4)|JABBER_MESSAGE_PARSE_FROM)
#define JABBER_MESSAGE_PARSE_TO						(1<<5)
#define JABBER_MESSAGE_PARSE_ID_STR					(1<<6)

class CJabberMessageInfo : public MZeroedObject
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
	MCONTACT m_hContact;
	LPCTSTR m_szTo;
	LPCTSTR m_szId;

public:
	__forceinline int GetMessageType()
	{	return m_nMessageType;
	}
	__forceinline void* GetUserData()
	{	return m_pUserData;
	}
	__forceinline LPCTSTR GetFrom()
	{	return m_szFrom;
	}
	__forceinline LPCTSTR GetTo()
	{	return m_szTo;
	}
	__forceinline LPCTSTR GetIdStr()
	{	return m_szId;
	}
	__forceinline MCONTACT GetHContact()
	{	return m_hContact;
	}
	__forceinline HXML GetChildNode()
	{	return m_hChildNode;
	}
	__forceinline LPCTSTR GetChildNodeName()
	{	return m_szChildTagName;
	}
};

class CJabberMessagePermanentInfo : public MZeroedObject
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
	~CJabberMessagePermanentInfo()
	{
		if (m_pUserDataFree)
			m_pUserDataFree(m_pUserData);
		mir_free(m_szXmlns);
		mir_free(m_szTag);
	}
};

class CJabberMessageManager
{
protected:
	CJabberProto *ppro;
	mir_cs m_cs;
	CJabberMessagePermanentInfo* m_pPermanentHandlers;

public:
	CJabberMessageManager(CJabberProto* proto);
	~CJabberMessageManager();

	CJabberMessagePermanentInfo* AddPermanentHandler(JABBER_PERMANENT_MESSAGE_HANDLER pHandler, int nMessageTypes, DWORD dwParamsToParse, const TCHAR *szXmlns, BOOL bAllowPartialNs, const TCHAR *szTag, void *pUserData = NULL, MESSAGE_USER_DATA_FREE_FUNC pUserDataFree = NULL, int iPriority = JH_PRIORITY_DEFAULT);
	bool DeletePermanentHandler(CJabberMessagePermanentInfo *pInfo);

	bool HandleMessagePermanent(HXML node, ThreadData *pThreadData);
	void FillPermanentHandlers();
};

#endif
