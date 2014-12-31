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

#include "jabber.h"

static int CompareItems(const CJabberSendPermanentInfo *p1, const CJabberSendPermanentInfo *p2)
{
	return p1->getPriority() - p2->getPriority();
}

CJabberSendManager::CJabberSendManager(CJabberProto* proto) :
	m_arHandlers(1, CompareItems)
{
	ppro = proto;
}

CJabberSendManager::~CJabberSendManager()
{
}

CJabberSendPermanentInfo* CJabberSendManager::AddPermanentHandler(JABBER_SEND_HANDLER pHandler, void *pUserData, SEND_USER_DATA_FREE_FUNC pUserDataFree, int iPriority)
{
	CJabberSendPermanentInfo *pInfo = new CJabberSendPermanentInfo();
	pInfo->m_pHandler = pHandler;
	pInfo->m_pUserData = pUserData;
	pInfo->m_pUserDataFree = pUserDataFree;
	pInfo->m_iPriority = iPriority;

	mir_cslock lck(m_cs);
	m_arHandlers.insert(pInfo);
	return pInfo;
}

bool CJabberSendManager::DeletePermanentHandler(CJabberSendPermanentInfo *pInfo)
{ 
	mir_cslock lck(m_cs);
	return m_arHandlers.remove(pInfo) == 1;
}

bool CJabberSendManager::HandleSendPermanent(HXML node, ThreadData *pThreadData)
{
	for (int i = 0; i < m_arHandlers.getCount(); i++) {
		CJabberSendPermanentInfo &pInfo = m_arHandlers[i];

		CJabberSendInfo sendInfo;
		sendInfo.m_pUserData = pInfo.m_pUserData;
		if ((ppro->*(pInfo.m_pHandler))(node, pThreadData, &sendInfo))
			return true;
	}

	return false;
}
