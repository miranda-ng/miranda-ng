/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-08  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2008-09  Dmitriy Chervov
Copyright (C) 2012-22 Miranda NG team

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
typedef void (CJabberProto::*JABBER_MESSAGE_PFUNC)(const TiXmlElement *messageNode, void *usedata);
typedef void (*MESSAGE_USER_DATA_FREE_FUNC)(void *pUserData);

class CJabberMessageInfo;

typedef bool (CJabberProto::*JABBER_PERMANENT_MESSAGE_HANDLER)(const TiXmlElement *messageNode, ThreadData *pThreadData, CJabberMessageInfo* pInfo);

#define JABBER_MESSAGE_PARSE_FROM					(1<<3)
#define JABBER_MESSAGE_PARSE_HCONTACT				((1<<4)|JABBER_MESSAGE_PARSE_FROM)
#define JABBER_MESSAGE_PARSE_TO						(1<<5)
#define JABBER_MESSAGE_PARSE_ID_STR					(1<<6)

class CJabberMessageInfo : public MZeroedObject
{
protected:
	friend class CJabberMessageManager;
	JABBER_PERMANENT_MESSAGE_HANDLER m_pHandler;
	CJabberMessageInfo *m_pNext;

public:
	void *m_pUserData;
// parsed data
	int m_nMessageType;
	const char *m_szFrom;
	const char *m_szChildTagXmlns;
	const char *m_szChildTagName;
	const TiXmlElement *m_hChildNode;
	MCONTACT m_hContact;
	const char *m_szTo;
	const char *m_szId;

public:
	__forceinline int GetMessageType()
	{	return m_nMessageType;
	}
	__forceinline void* GetUserData()
	{	return m_pUserData;
	}
	__forceinline const char* GetFrom()
	{	return m_szFrom;
	}
	__forceinline const char* GetTo()
	{	return m_szTo;
	}
	__forceinline const char* GetIdStr()
	{	return m_szId;
	}
	__forceinline MCONTACT GetHContact()
	{	return m_hContact;
	}
	__forceinline const TiXmlElement* GetChildNode()
	{	return m_hChildNode;
	}
	__forceinline const char *GetChildNodeName()
	{	return m_szChildTagName;
	}
};

class CJabberMessagePermanentInfo : public MZeroedObject
{
	friend class CJabberMessageManager;

	JABBER_PERMANENT_MESSAGE_HANDLER m_pHandler;
	uint32_t m_dwParamsToParse;
	int m_nMessageTypes;
	char *m_szXmlns;
	char *m_szTag;
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

	__forceinline int getPriority() const { return m_iPriority; }
};

class CJabberMessageManager
{
protected:
	CJabberProto *ppro;
	mir_cs m_cs;
	OBJLIST<CJabberMessagePermanentInfo> m_arHandlers;

public:
	CJabberMessageManager(CJabberProto *proto);
	~CJabberMessageManager();

	CJabberMessagePermanentInfo* AddPermanentHandler(JABBER_PERMANENT_MESSAGE_HANDLER pHandler, int nMessageTypes, uint32_t dwParamsToParse, const char *szXmlns, BOOL bAllowPartialNs, const char *szTag, void *pUserData = nullptr, MESSAGE_USER_DATA_FREE_FUNC pUserDataFree = nullptr, int iPriority = JH_PRIORITY_DEFAULT);
	bool DeletePermanentHandler(CJabberMessagePermanentInfo *pInfo);

	bool HandleMessagePermanent(const TiXmlElement *node, ThreadData *pThreadData);
	void FillPermanentHandlers();
};

#endif
