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

bool CJabberProto::OnMessageError(const TiXmlElement *node, ThreadData*, CJabberMessageInfo* pInfo)
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
			CMStringW wszErrorText(Utf2T(pInfo->GetFrom()));
			wszErrorText.Append(L":\n");

			if (auto *body = XmlGetChildText(node, "body")) {
				wszErrorText.Append(Utf2T(body));
				wszErrorText.AppendChar('\n');
			}

			wszErrorText += szErrText;
			MsgPopup(0, wszErrorText, TranslateT("Error"));
		}
	}
	return true;
}

bool CJabberProto::OnMessageIbb(const TiXmlElement*, ThreadData*, CJabberMessageInfo* pInfo)
{
	bool bOk = false;
	const char *sid = XmlGetAttr(pInfo->GetChildNode(), "sid");
	const char *seq = XmlGetAttr(pInfo->GetChildNode(), "seq");
	if (sid && seq && pInfo->GetChildNode()->GetText())
		bOk = OnIbbRecvdData(pInfo->GetChildNode()->GetText(), sid, seq);

	return true;
}

bool CJabberProto::OnMessagePubsubEvent(const TiXmlElement *node, ThreadData*, CJabberMessageInfo*)
{
	OnProcessPubsubEvent(node);
	return true;
}

bool CJabberProto::OnMessageGroupchat(const TiXmlElement *node, ThreadData*, CJabberMessageInfo* pInfo)
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
	return true;
}
