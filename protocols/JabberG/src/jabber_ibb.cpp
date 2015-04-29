/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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
			pft->jibb = NULL;

		mir_free(jibb->srcJID);
		mir_free(jibb->dstJID);
		mir_free(jibb->sid);

		mir_free(jibb);
}	}

BOOL CJabberProto::OnFtHandleIbbIq(HXML iqNode, CJabberIqInfo *pInfo)
{
	if (!_tcscmp(pInfo->GetChildNodeName(), _T("open")))
		FtHandleIbbRequest(iqNode, TRUE);
	else if (!_tcscmp(pInfo->GetChildNodeName(), _T("close")))
		FtHandleIbbRequest(iqNode, FALSE);
	else if (!_tcscmp(pInfo->GetChildNodeName(), _T("data"))) {
		BOOL bOk = FALSE;
		const TCHAR *sid = xmlGetAttrValue(pInfo->GetChildNode(), _T("sid"));
		const TCHAR *seq = xmlGetAttrValue(pInfo->GetChildNode(), _T("seq"));
		if (sid && seq && xmlGetText(pInfo->GetChildNode()))
			bOk = OnIbbRecvdData(xmlGetText(pInfo->GetChildNode()), sid, seq);

		if (bOk)
			m_ThreadInfo->send( XmlNodeIq(_T("result"), pInfo));
		else
			m_ThreadInfo->send(
				XmlNodeIq(_T("error"), pInfo)
					<< XCHILD(_T("error")) << XATTRI(_T("code"), 404) << XATTR(_T("type"), _T("cancel"))
						<< XCHILDNS(_T("item-not-found"), _T("urn:ietf:params:xml:ns:xmpp-stanzas")));
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

	jibb->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	jibb->bStreamInitialized = FALSE;
	jibb->bStreamClosed = FALSE;
	jibb->state = JIBB_SENDING;

	m_ThreadInfo->send(
		XmlNodeIq( AddIQ(&CJabberProto::OnIbbInitiateResult, JABBER_IQ_TYPE_SET, jibb->dstJID, 0, -1, jibb))
			<< XCHILDNS(_T("open"), JABBER_FEAT_IBB) << XATTR(_T("sid"), jibb->sid) << XATTRI(_T("block-size"), JABBER_IBB_BLOCK_SIZE)
			<< XATTR(_T("stanza"), _T("message")));

	WaitForSingleObject(jibb->hEvent, INFINITE);
	CloseHandle(jibb->hEvent);
	jibb->hEvent = NULL;

	if (jibb->bStreamInitialized) {
		jibb->wPacketId = 0;

		BOOL bSent = (this->*jibb->pfnSend)(JABBER_IBB_BLOCK_SIZE, jibb->ft);

		if (!jibb->bStreamClosed) {
			jibb->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

			m_ThreadInfo->send(
				XmlNodeIq( AddIQ(&CJabberProto::OnIbbCloseResult, JABBER_IQ_TYPE_SET, jibb->dstJID, 0, -1, jibb))
					<< XCHILDNS(_T("close"), JABBER_FEAT_IBB) << XATTR(_T("sid"), jibb->sid));

			WaitForSingleObject(jibb->hEvent, INFINITE);
			CloseHandle(jibb->hEvent);
			jibb->hEvent = NULL;

			if (jibb->bStreamClosed && bSent)
				jibb->state = JIBB_DONE;
		}
		else jibb->state = JIBB_ERROR;
	}

	(this->*jibb->pfnFinal)((jibb->state==JIBB_DONE)?TRUE:FALSE, jibb->ft);
	jibb->ft = NULL;
	JabberIbbFreeJibb(jibb);
}

void __cdecl CJabberProto::IbbReceiveThread(JABBER_IBB_TRANSFER *jibb)
{
	debugLogA("Thread started: type=ibb_recv");

	filetransfer *ft = jibb->ft;

	jibb->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	jibb->bStreamClosed = FALSE;
	jibb->wPacketId = 0;
	jibb->dwTransferredSize = 0;
	jibb->state = JIBB_RECVING;

	WaitForSingleObject(jibb->hEvent, INFINITE);

	CloseHandle(jibb->hEvent);
	jibb->hEvent = NULL;

	if (jibb->state == JIBB_ERROR)
		m_ThreadInfo->send( XmlNodeIq(_T("set"), SerialNext(), jibb->dstJID) << XCHILDNS(_T("close"), JABBER_FEAT_IBB) << XATTR(_T("sid"), jibb->sid));

	if (jibb->bStreamClosed && jibb->dwTransferredSize == ft->dwExpectedRecvFileSize)
		jibb->state = JIBB_DONE;

	(this->*jibb->pfnFinal)((jibb->state==JIBB_DONE)?TRUE:FALSE, jibb->ft);
	jibb->ft = NULL;

	ListRemove(LIST_FTRECV, jibb->sid);

	JabberIbbFreeJibb(jibb);
}

BOOL CJabberProto::OnIbbRecvdData(const TCHAR *data, const TCHAR *sid, const TCHAR *seq)
{
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTRECV, sid);
	if (item == NULL) return FALSE;

	WORD wSeq = (WORD)_ttoi(seq);
	if (wSeq != item->jibb->wPacketId) {
		if (item->jibb->hEvent)
			SetEvent(item->jibb->hEvent);
		return FALSE;
	}

	item->jibb->wPacketId++;

	unsigned length;
	ptrA decodedData((char*)mir_base64_decode( _T2A(data), &length));
	if (decodedData == NULL)
		return FALSE;

	(this->*item->jibb->pfnRecv)(NULL, item->ft, decodedData, length);
	item->jibb->dwTransferredSize += (DWORD)length;
	return TRUE;
}
