/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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
#include "jabber_iq.h"
#include "jabber_ibb.h"
#include "jabber_caps.h"

#define JABBER_IBB_BLOCK_SIZE 2048

void JabberIbbFreeJibb(JABBER_IBB_TRANSFER *jibb)
{
	if (jibb) {
		filetransfer* pft = jibb->ft;
		if (pft)
			pft->jibb = nullptr;

		mir_free(jibb->srcJID);
		mir_free(jibb->dstJID);
		mir_free(jibb->sid);

		mir_free(jibb);
	}
}

bool CJabberProto::OnFtHandleIbbIq(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	if (!mir_strcmp(pInfo->GetChildNodeName(), "open"))
		FtHandleIbbRequest(iqNode, true);
	else if (!mir_strcmp(pInfo->GetChildNodeName(), "close"))
		FtHandleIbbRequest(iqNode, false);
	else if (!mir_strcmp(pInfo->GetChildNodeName(), "data")) {
		bool bOk = false;
		const char *sid = XmlGetAttr(pInfo->GetChildNode(), "sid");
		const char *seq = XmlGetAttr(pInfo->GetChildNode(), "seq");
		if (sid && seq && pInfo->GetChildNode()->GetText())
			bOk = OnIbbRecvdData(pInfo->GetChildNode()->GetText(), sid, seq);

		if (bOk)
			m_ThreadInfo->send(XmlNodeIq("result", pInfo));
		else
			m_ThreadInfo->send(
				XmlNodeIq("error", pInfo)
				<< XCHILD("error") << XATTRI("code", 404) << XATTR("type", "cancel")
				<< XCHILDNS("item-not-found", "urn:ietf:params:xml:ns:xmpp-stanzas"));
	}
	return true;
}

void CJabberProto::OnIbbInitiateResult(const TiXmlElement*, CJabberIqInfo *pInfo)
{
	JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *)pInfo->GetUserData();
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT)
		jibb->bStreamInitialized = true;
	if (jibb->hEvent)
		SetEvent(jibb->hEvent);
}

void CJabberProto::OnIbbCloseResult(const TiXmlElement*, CJabberIqInfo *pInfo)
{
	JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *)pInfo->GetUserData();
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT)
		jibb->bStreamClosed = true;
	if (jibb->hEvent)
		SetEvent(jibb->hEvent);
}

void CJabberProto::IbbSendThread(JABBER_IBB_TRANSFER *jibb)
{
	debugLogA("Thread started: type=ibb_send");
	Thread_SetName("Jabber: IbbSendThread");

	jibb->hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	jibb->bStreamInitialized = false;
	jibb->bStreamClosed = false;
	jibb->state = JIBB_SENDING;

	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIbbInitiateResult, JABBER_IQ_TYPE_SET, jibb->dstJID, jibb))
		<< XCHILDNS("open", JABBER_FEAT_IBB) << XATTR("sid", jibb->sid) << XATTRI("block-size", JABBER_IBB_BLOCK_SIZE)
		<< XATTR("stanza", "message"));

	WaitForSingleObject(jibb->hEvent, INFINITE);
	CloseHandle(jibb->hEvent);
	jibb->hEvent = nullptr;

	if (jibb->bStreamInitialized) {
		jibb->wPacketId = 0;

		BOOL bSent = (this->*jibb->pfnSend)(JABBER_IBB_BLOCK_SIZE, jibb->ft);

		if (!jibb->bStreamClosed) {
			jibb->hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::OnIbbCloseResult, JABBER_IQ_TYPE_SET, jibb->dstJID, jibb))
				<< XCHILDNS("close", JABBER_FEAT_IBB) << XATTR("sid", jibb->sid));

			WaitForSingleObject(jibb->hEvent, INFINITE);
			CloseHandle(jibb->hEvent);
			jibb->hEvent = nullptr;

			if (jibb->bStreamClosed && bSent)
				jibb->state = JIBB_DONE;
		}
		else jibb->state = JIBB_ERROR;
	}

	(this->*jibb->pfnFinal)(jibb->state == JIBB_DONE, jibb->ft);
	jibb->ft = nullptr;
	JabberIbbFreeJibb(jibb);
}

void __cdecl CJabberProto::IbbReceiveThread(JABBER_IBB_TRANSFER *jibb)
{
	debugLogA("Thread started: type=ibb_recv");
	Thread_SetName("Jabber: IbbReceiveThread");

	filetransfer *ft = jibb->ft;

	jibb->hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	jibb->bStreamClosed = false;
	jibb->wPacketId = 0;
	jibb->dwTransferredSize = 0;
	jibb->state = JIBB_RECVING;

	WaitForSingleObject(jibb->hEvent, INFINITE);

	CloseHandle(jibb->hEvent);
	jibb->hEvent = nullptr;

	if (jibb->state == JIBB_ERROR)
		m_ThreadInfo->send(XmlNodeIq("set", SerialNext(), jibb->dstJID) << XCHILDNS("close", JABBER_FEAT_IBB) << XATTR("sid", jibb->sid));

	if (jibb->bStreamClosed && jibb->dwTransferredSize == ft->dwExpectedRecvFileSize)
		jibb->state = JIBB_DONE;

	(this->*jibb->pfnFinal)(jibb->state == JIBB_DONE, jibb->ft);
	jibb->ft = nullptr;

	ListRemove(LIST_FTRECV, jibb->sid);

	JabberIbbFreeJibb(jibb);
}

bool CJabberProto::OnIbbRecvdData(const char *data, const char *sid, const char *seq)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTRECV, sid);
	if (item == nullptr)
		return false;

	uint16_t wSeq = (uint16_t)atoi(seq);
	if (wSeq != item->jibb->wPacketId) {
		if (item->jibb->hEvent)
			SetEvent(item->jibb->hEvent);
		return false;
	}

	item->jibb->wPacketId++;

	size_t length;
	ptrA decodedData((char*)mir_base64_decode(data, &length));
	if (decodedData == nullptr)
		return false;

	(this->*item->jibb->pfnRecv)(nullptr, item->ft, decodedData, (int)length);
	item->jibb->dwTransferredSize += (uint32_t)length;
	return true;
}
