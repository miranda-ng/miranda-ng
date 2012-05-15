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

Revision       : $Revision: 7543 $
Last change on : $Date: 2008-03-30 22:55:23 +0300 (Вс, 30 мар 2008) $
Last change by : $Author: dchervov $

*/

#include "jabber.h"
#include "jabber_message_manager.h"

BOOL CJabberMessageManager::FillPermanentHandlers()
{
	AddPermanentHandler( &CJabberProto::OnMessageError, JABBER_MESSAGE_TYPE_ERROR, JABBER_MESSAGE_PARSE_FROM | JABBER_MESSAGE_PARSE_HCONTACT, NULL, FALSE, _T("error") );
	AddPermanentHandler( &CJabberProto::OnMessageIbb, 0, 0, _T(JABBER_FEAT_IBB), FALSE, _T("data") );
	AddPermanentHandler( &CJabberProto::OnMessagePubsubEvent, 0, 0, _T(JABBER_FEAT_PUBSUB_EVENT), FALSE, _T("event") );
	AddPermanentHandler( &CJabberProto::OnMessageGroupchat, JABBER_MESSAGE_TYPE_GROUPCHAT, JABBER_MESSAGE_PARSE_FROM, NULL, FALSE, NULL );
	return TRUE;
}

BOOL CJabberMessageManager::HandleMessagePermanent(HXML node, ThreadData *pThreadData)
{
	BOOL bStopHandling = FALSE;
	Lock();
	CJabberMessagePermanentInfo *pInfo = m_pPermanentHandlers;
	while ( pInfo && !bStopHandling ) {
	// have to get all data here, in the loop, because there's always possibility that previous handler modified it
		CJabberMessageInfo messageInfo;

		LPCTSTR szType = xmlGetAttrValue(node, _T("type"));
		if ( szType )
		{
			if ( !_tcsicmp( szType, _T("normal")))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_NORMAL;
			else if ( !_tcsicmp( szType, _T("error")))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_ERROR;
			else if ( !_tcsicmp( szType, _T("chat")))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_CHAT;
			else if ( !_tcsicmp( szType, _T("groupchat")))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_GROUPCHAT;
			else if ( !_tcsicmp( szType, _T("headline")))
				messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_HEADLINE;
			else
				break; // m_nMessageType = JABBER_MESSAGE_TYPE_FAIL;
		}
		else {
			messageInfo.m_nMessageType = JABBER_MESSAGE_TYPE_NORMAL;
		}

		if ( (pInfo->m_nMessageTypes & messageInfo.m_nMessageType )) {
			int i;
			for ( i = xmlGetChildCount( node ) - 1; i >= 0; i-- ) {
			// enumerate all children and see whether this node suits handler criteria
				HXML child = xmlGetChild( node, i );
				
				LPCTSTR szTagName = xmlGetName(child);
				LPCTSTR szXmlns = xmlGetAttrValue( child, _T("xmlns"));

				if ( (!pInfo->m_szXmlns || ( szXmlns && !_tcscmp( pInfo->m_szXmlns, szXmlns ))) &&
				( !pInfo->m_szTag || !_tcscmp( pInfo->m_szTag, szTagName ))) {
				// node suits handler criteria, call the handler
					messageInfo.m_hChildNode = child;
					messageInfo.m_szChildTagName = szTagName;
					messageInfo.m_szChildTagXmlns = szXmlns;
					messageInfo.m_pUserData = pInfo->m_pUserData;
					messageInfo.m_szFrom = xmlGetAttrValue( node, _T("from")); // is necessary for ppro->Log() below, that's why we must parse it even if JABBER_MESSAGE_PARSE_FROM flag is not set

					if (pInfo->m_dwParamsToParse & JABBER_MESSAGE_PARSE_ID_STR)
						messageInfo.m_szId = xmlGetAttrValue( node, _T("id"));

					if (pInfo->m_dwParamsToParse & JABBER_IQ_PARSE_TO)
						messageInfo.m_szTo = xmlGetAttrValue( node, _T("to"));

					if (pInfo->m_dwParamsToParse & JABBER_MESSAGE_PARSE_HCONTACT)
						messageInfo.m_hContact = ppro->HContactFromJID( messageInfo.m_szFrom, 3 );

					if (messageInfo.m_szFrom)
						ppro->Log( "Handling message from " TCHAR_STR_PARAM, messageInfo.m_szFrom );
					if ((ppro->*(pInfo->m_pHandler))(node, pThreadData, &messageInfo)) {
						bStopHandling = TRUE;
						break;
					}
				}
			}
		}
		pInfo = pInfo->m_pNext;
	}
	Unlock();

	return bStopHandling;
}
