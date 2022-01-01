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

#include "stdafx.h"

static int CompareItems(const CJabberPresencePermanentInfo *p1, const CJabberPresencePermanentInfo *p2)
{
	return p1->getPriority() - p2->getPriority();
}

CJabberPresenceManager::CJabberPresenceManager(CJabberProto *proto) :
	m_arHandlers(1, &CompareItems)
{
	ppro = proto;
}

CJabberPresenceManager::~CJabberPresenceManager()
{
}

CJabberPresencePermanentInfo* CJabberPresenceManager::AddPermanentHandler(
	JABBER_PRESENCE_HANDLER pHandler,
	void *pUserData,
	PRESENCE_USER_DATA_FREE_FUNC pUserDataFree,
	int iPriority)
{
	CJabberPresencePermanentInfo* pInfo = new CJabberPresencePermanentInfo();
	pInfo->m_pHandler = pHandler;
	pInfo->m_pUserData = pUserData;
	pInfo->m_pUserDataFree = pUserDataFree;
	pInfo->m_iPriority = iPriority;

	mir_cslock lck(m_cs);
	m_arHandlers.insert(pInfo);
	return pInfo;
}

bool CJabberPresenceManager::DeletePermanentHandler(CJabberPresencePermanentInfo *pInfo)
{
	mir_cslock lck(m_cs);
	return m_arHandlers.remove(pInfo) == 1;
}

bool CJabberPresenceManager::HandlePresencePermanent(const TiXmlElement *node, ThreadData *pThreadData)
{
	for (auto &it : m_arHandlers) {
		CJabberPresenceInfo presenceInfo;
		presenceInfo.m_pUserData = it->m_pUserData;
		if ((ppro->*(it->m_pHandler))(node, pThreadData, &presenceInfo))
			return true;
	}

	return false;
}
