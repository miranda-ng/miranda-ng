/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-08  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (c) 2008-09  Dmitriy Chervov
Copyright (C) 2012-19 Miranda NG team

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

BOOL CJabberProto::OnMessageError(const TiXmlElement *node, ThreadData*, CJabberMessageInfo* pInfo)
{
	// we check if is message delivery failure
	int id = JabberGetPacketID(node);
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_ROSTER, pInfo->GetFrom());
	if (item == nullptr)
		item = ListGetItemPtr(LIST_CHATROOM, pInfo->GetFrom());
	if (item != nullptr) { // yes, it is
		CMStringW szErrText(JabberErrorMsg(pInfo->GetChildNode()));
		if (id != -1)
			ProtoBroadcastAck(pInfo->GetHContact(), ACKTYPE_MESSAGE, ACKRESULT_FAILED, (HANDLE)id, (LPARAM)szErrText.c_str());
		else {
			wchar_t buf[512];
			auto *body = XmlGetChildText(node, "body");
			if (body)
				mir_snwprintf(buf, L"%s:\n%s\n%s", pInfo->GetFrom(), body, szErrText.c_str());
			else
				mir_snwprintf(buf, L"%s:\n%s", pInfo->GetFrom(), szErrText.c_str());

			MsgPopup(0, buf, TranslateT("Jabber Error"));
		}
	}
	return TRUE;
}

BOOL CJabberProto::OnMessageIbb(const TiXmlElement*, ThreadData*, CJabberMessageInfo* pInfo)
{
	BOOL bOk = FALSE;
	const char *sid = XmlGetAttr(pInfo->GetChildNode(), "sid");
	const char *seq = XmlGetAttr(pInfo->GetChildNode(), "seq");
	if (sid && seq && pInfo->GetChildNode()->GetText())
		bOk = OnIbbRecvdData(pInfo->GetChildNode()->GetText(), sid, seq);

	return TRUE;
}

BOOL CJabberProto::OnMessagePubsubEvent(const TiXmlElement *node, ThreadData*, CJabberMessageInfo*)
{
	OnProcessPubsubEvent(node);
	return TRUE;
}

BOOL CJabberProto::OnMessageGroupchat(const TiXmlElement *node, ThreadData*, CJabberMessageInfo* pInfo)
{
	JABBER_LIST_ITEM *chatItem = ListGetItemPtr(LIST_CHATROOM, pInfo->GetFrom());
	if (chatItem) // process GC message
		GroupchatProcessMessage(node);
	
	// got message from unknown conference... let's leave it :)
	else { 
//			wchar_t *conference = NEWWSTR_ALLOCA(from);
//			if (wchar_t *s = wcschr(conference, '/')) *s = 0;
//			XmlNode p("presence"); XmlAddAttr(p, "to", conference); XmlAddAttr(p, "type", "unavailable");
//			info->send(p);
	}
	return TRUE;
}
