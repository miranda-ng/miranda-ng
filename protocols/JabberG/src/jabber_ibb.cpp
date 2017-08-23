/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
Copyright (ñ) 2012-17 Miranda NG project

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
	if (jibb)  {
		filetransfer* pft = jibb->ft;
		if (pft)
			pft->jibb = nullptr;

		mir_free(jibb->srcJID);
		mir_free(jibb->dstJID);
		mir_free(jibb->sid);

		mir_free(jibb);
}	}

BOOL CJabberProto::OnFtHandleIbbIq(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!mir_wstrcmp(pInfo->GetChildNodeName(), L"open"))
		FtHandleIbbRequest(iqNode, TRUE);
	else if (!mir_wstrcmp(pInfo->GetChildNodeName(), L"close"))
		FtHandleIbbRequest(iqNode, FALSE);
	else if (!mir_wstrcmp(pInfo->GetChildNodeName(), L"data")) {
		BOOL bOk = FALSE;
		const wchar_t *sid = XmlGetAttrValue(pInfo->GetChildNode(), L"sid");
		const wchar_t *seq = XmlGetAttrValue(pInfo->GetChildNode(), L"seq");
		if (sid && seq && XmlGetText(pInfo->GetChildNode()))
			bOk = OnIbbRecvdData(XmlGetText(pInfo->GetChildNode()), sid, seq);

		if (bOk)
			m_ThreadInfo->send( XmlNodeIq(L"result", pInfo));
		else
			m_ThreadInfo->send(
				XmlNodeIq(L"error", pInfo)
					<< XCHILD(L"error") << XATTRI(L"code", 404) << XATTR(L"type", L"cancel")
						<< XCHILDNS(L"item-not-found", L"urn:ietf:params:xml:ns:xmpp-stanzas"));
	}
	return TRUE;
}

void CJabberProto::OnIbbInitiateResult(HXML, CJabberIqInfo *pInfo)
{
	JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *)pInfo->GetUserData();
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT)
		jibb->bStreamInitialized = TRUE;
	if (jibb->hEvent)
		SetEvent(jibb->hEvent);
}

void CJabberProto::OnIbbCloseResult(HXML, CJabberIqInfo *pInfo)
{
	JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *)pInfo->GetUserData();
	if (pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT)
		jibb->bStreamClosed = TRUE;
	if (jibb->hEvent)
		SetEvent(jibb->hEvent);
}

void CJabberProto::IbbSendThread(JABBER_IBB_TRANSFER *jibb)
{
	debugLogA("Thread started: type=ibb_send");
	Thread_SetName("Jabber: IbbSendThread");

	jibb->hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	jibb->bStreamInitialized = FALSE;
	jibb->bStreamClosed = FALSE;
	jibb->state = JIBB_SENDING;

	m_ThreadInfo->send(
		XmlNodeIq(AddIQ(&CJabberProto::OnIbbInitiateResult, JABBER_IQ_TYPE_SET, jibb->dstJID, 0, -1, jibb))
			<< XCHILDNS(L"open", JABBER_FEAT_IBB) << XATTR(L"sid", jibb->sid) << XATTRI(L"block-size", JABBER_IBB_BLOCK_SIZE)
			<< XATTR(L"stanza", L"message"));

	WaitForSingleObject(jibb->hEvent, INFINITE);
	CloseHandle(jibb->hEvent);
	jibb->hEvent = nullptr;

	if (jibb->bStreamInitialized) {
		jibb->wPacketId = 0;

		BOOL bSent = (this->*jibb->pfnSend)(JABBER_IBB_BLOCK_SIZE, jibb->ft);

		if (!jibb->bStreamClosed) {
			jibb->hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

			m_ThreadInfo->send(
				XmlNodeIq(AddIQ(&CJabberProto::OnIbbCloseResult, JABBER_IQ_TYPE_SET, jibb->dstJID, 0, -1, jibb))
					<< XCHILDNS(L"close", JABBER_FEAT_IBB) << XATTR(L"sid", jibb->sid));

			WaitForSingleObject(jibb->hEvent, INFINITE);
			CloseHandle(jibb->hEvent);
			jibb->hEvent = nullptr;

			if (jibb->bStreamClosed && bSent)
				jibb->state = JIBB_DONE;
		}
		else jibb->state = JIBB_ERROR;
	}

	(this->*jibb->pfnFinal)((jibb->state==JIBB_DONE)?TRUE:FALSE, jibb->ft);
	jibb->ft = nullptr;
	JabberIbbFreeJibb(jibb);
}

void __cdecl CJabberProto::IbbReceiveThread(JABBER_IBB_TRANSFER *jibb)
{
	debugLogA("Thread started: type=ibb_recv");
	Thread_SetName("Jabber: IbbReceiveThread");

	filetransfer *ft = jibb->ft;

	jibb->hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	jibb->bStreamClosed = FALSE;
	jibb->wPacketId = 0;
	jibb->dwTransferredSize = 0;
	jibb->state = JIBB_RECVING;

	WaitForSingleObject(jibb->hEvent, INFINITE);

	CloseHandle(jibb->hEvent);
	jibb->hEvent = nullptr;

	if (jibb->state == JIBB_ERROR)
		m_ThreadInfo->send( XmlNodeIq(L"set", SerialNext(), jibb->dstJID) << XCHILDNS(L"close", JABBER_FEAT_IBB) << XATTR(L"sid", jibb->sid));

	if (jibb->bStreamClosed && jibb->dwTransferredSize == ft->dwExpectedRecvFileSize)
		jibb->state = JIBB_DONE;

	(this->*jibb->pfnFinal)((jibb->state==JIBB_DONE)?TRUE:FALSE, jibb->ft);
	jibb->ft = nullptr;

	ListRemove(LIST_FTRECV, jibb->sid);

	JabberIbbFreeJibb(jibb);
}

BOOL CJabberProto::OnIbbRecvdData(const wchar_t *data, const wchar_t *sid, const wchar_t *seq)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTRECV, sid);
	if (item == nullptr) return FALSE;

	WORD wSeq = (WORD)_wtoi(seq);
	if (wSeq != item->jibb->wPacketId) {
		if (item->jibb->hEvent)
			SetEvent(item->jibb->hEvent);
		return FALSE;
	}

	item->jibb->wPacketId++;

	unsigned length;
	ptrA decodedData((char*)mir_base64_decode( _T2A(data), &length));
	if (decodedData == nullptr)
		return FALSE;

	(this->*item->jibb->pfnRecv)(nullptr, item->ft, decodedData, length);
	item->jibb->dwTransferredSize += (DWORD)length;
	return TRUE;
}
