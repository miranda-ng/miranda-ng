/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-08  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2008-09  Dmitriy Chervov
Copyright (�) 2012-15 Miranda NG project

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

BOOL CJabberProto::OnMessageError(HXML node, ThreadData*, CJabberMessageInfo* pInfo)
{
	// we check if is message delivery failure
	int id = JabberGetPacketID(node);
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, pInfo->GetFrom());
	if (item == NULL)
		item = ListGetItemPtr(LIST_CHATROOM, pInfo->GetFrom());
	if (item != NULL) { // yes, it is
		TCHAR *szErrText = JabberErrorMsg(pInfo->GetChildNode());
		if (id != -1) {
			char *errText = mir_t2a(szErrText);
			ProtoBroadcastAck(pInfo->GetHContact(), ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)id, (LPARAM)errText);
			mir_free(errText);
		}
		else {
			TCHAR buf[512];
			HXML bodyNode = XmlGetChild(node, "body");
			if (bodyNode)
				mir_sntprintf(buf, _T("%s:\n%s\n%s"), pInfo->GetFrom(), XmlGetText(bodyNode), szErrText);
			else
				mir_sntprintf(buf, _T("%s:\n%s"), pInfo->GetFrom(), szErrText);

			 MsgPopup(NULL, buf, TranslateT("Jabber Error"));
		}
		mir_free(szErrText);
	}
	return TRUE;
}

BOOL CJabberProto::OnMessageIbb(HXML, ThreadData*, CJabberMessageInfo* pInfo)
{
	BOOL bOk = FALSE;
	const TCHAR *sid = XmlGetAttrValue(pInfo->GetChildNode(), _T("sid"));
	const TCHAR *seq = XmlGetAttrValue(pInfo->GetChildNode(), _T("seq"));
	if (sid && seq && XmlGetText(pInfo->GetChildNode()))
		bOk = OnIbbRecvdData(XmlGetText(pInfo->GetChildNode()), sid, seq);

	return TRUE;
}

BOOL CJabberProto::OnMessagePubsubEvent(HXML node, ThreadData*, CJabberMessageInfo*)
{
	OnProcessPubsubEvent(node);
	return TRUE;
}

BOOL CJabberProto::OnMessageGroupchat(HXML node, ThreadData*, CJabberMessageInfo* pInfo)
{
	JABBER_LIST_ITEM *chatItem = ListGetItemPtr(LIST_CHATROOM, pInfo->GetFrom());
	if (chatItem) // process GC message
		GroupchatProcessMessage(node);
	
	// got message from unknown conference... let's leave it :)
	else { 
//			TCHAR *conference = NEWTSTR_ALLOCA(from);
//			if (TCHAR *s = _tcschr(conference, _T('/'))) *s = 0;
//			XmlNode p("presence"); XmlAddAttr(p, "to", conference); XmlAddAttr(p, "type", "unavailable");
//			info->send(p);
	}
	return TRUE;
}
