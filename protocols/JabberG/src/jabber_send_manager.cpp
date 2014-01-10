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

#include "jabber.h"
#include "jabber_send_manager.h"

BOOL CJabberSendManager::FillPermanentHandlers()
{
	return TRUE;
}

BOOL CJabberSendManager::HandleSendPermanent(HXML node, ThreadData *pThreadData)
{
	mir_cslock lck(m_cs);
	for (CJabberSendPermanentInfo *pInfo = m_pPermanentHandlers; pInfo; pInfo = pInfo->m_pNext) {
		CJabberSendInfo sendInfo;
		sendInfo.m_pUserData = pInfo->m_pUserData;
		if ((ppro->*(pInfo->m_pHandler))(node, pThreadData, &sendInfo))
			return true;
	}

	return false;
}
