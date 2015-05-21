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
#include "jabber_byte.h"
#include "jabber_ibb.h"
#include "jabber_caps.h"

void CJabberProto::FtCancel(filetransfer *ft)
{
	debugLogA("Invoking JabberFtCancel()");

	// For file sending session that is still in si negotiation phase
	if (m_iqManager.ExpireByUserData(ft))
		return;

	// For file receiving session that is still in si negotiation phase
	LISTFOREACH(i, this, LIST_FTRECV)
	{
		JABBER_LIST_ITEM *item = ListGetItemPtrFromIndex(i);
		if (item->ft == ft) {
			debugLogA("Canceling file receiving session while in si negotiation");
			ListRemoveByIndex(i);
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
			delete ft;
			return;
		}
	}
	// For file transfer through bytestream
	JABBER_BYTE_TRANSFER *jbt = ft->jbt;
	if (jbt != NULL) {
		debugLogA("Canceling bytestream session");
		jbt->state = JBT_ERROR;
		if (jbt->hConn) {
			debugLogA("Force closing bytestream session");
			Netlib_CloseHandle(jbt->hConn);
			jbt->hConn = NULL;
		}
		if (jbt->hSendEvent) SetEvent(jbt->hSendEvent);
		if (jbt->hEvent) SetEvent(jbt->hEvent);
		if (jbt->hProxyEvent) SetEvent(jbt->hProxyEvent);
	}

	// For file transfer through IBB
	JABBER_IBB_TRANSFER *jibb = ft->jibb;
	if (jibb != NULL) {
		debugLogA("Canceling IBB session");
		jibb->state = JIBB_ERROR;
		m_iqManager.ExpireByUserData(jibb);
	}
}

///////////////// File sending using stream initiation /////////////////////////

void CJabberProto::FtInitiate(TCHAR* jid, filetransfer *ft)
{
	TCHAR *rs;
	TCHAR *filename, *p;
	int i;
	TCHAR sid[9];

	if (jid == NULL || ft == NULL || !m_bJabberOnline || (rs=ListGetBestClientResourceNamePtr(jid)) == NULL) {
		if (ft) {
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
			delete ft;
		}
		return;
	}
	ft->type = FT_SI;
	for (i=0; i<8; i++)
		sid[i] = (rand()%10) + '0';
	sid[8] = '\0';
	if (ft->sid != NULL) mir_free(ft->sid);
	ft->sid = mir_tstrdup(sid);
	filename = ft->std.ptszFiles[ft->std.currentFileNumber];
	if ((p = _tcsrchr(filename, '\\')) != NULL)
		filename = p+1;

	TCHAR tszJid[ 512 ];
	mir_sntprintf(tszJid, SIZEOF(tszJid), _T("%s/%s"), jid, rs);

	XmlNodeIq iq( AddIQ(&CJabberProto::OnFtSiResult, JABBER_IQ_TYPE_SET, tszJid, JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO, -1, ft));
	HXML si = iq << XCHILDNS(_T("si"), JABBER_FEAT_SI) << XATTR(_T("id"), sid)
						<< XATTR(_T("mime-type"), _T("binary/octet-stream")) << XATTR(_T("profile"), JABBER_FEAT_SI_FT);
	si << XCHILDNS(_T("file"), JABBER_FEAT_SI_FT) << XATTR(_T("name"), filename)
		<< XATTRI64(_T("size"), ft->fileSize[ft->std.currentFileNumber]) << XCHILD(_T("desc"), ft->szDescription);

	HXML field = si << XCHILDNS(_T("feature"), JABBER_FEAT_FEATURE_NEG)
							<< XCHILDNS(_T("x"), JABBER_FEAT_DATA_FORMS) << XATTR(_T("type"), _T("form"))
							<< XCHILD(_T("field")) << XATTR(_T("var"), _T("stream-method")) << XATTR(_T("type"), _T("list-single"));

	BOOL bDirect = m_options.BsDirect;
	BOOL bProxy = m_options.BsProxyManual;

	// bytestreams support?
	if (bDirect || bProxy)
		field << XCHILD(_T("option")) << XCHILD(_T("value"), JABBER_FEAT_BYTESTREAMS);

	field << XCHILD(_T("option")) << XCHILD(_T("value"), JABBER_FEAT_IBB);
	m_ThreadInfo->send(iq);
}

void CJabberProto::OnFtSiResult(HXML iqNode, CJabberIqInfo *pInfo)
{
	HXML siNode, featureNode, xNode, fieldNode, valueNode;
	filetransfer *ft = (filetransfer *)pInfo->GetUserData();
	if (!ft) return;

	if ((pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) && pInfo->m_szFrom && pInfo->m_szTo) {
		if ((siNode = xmlGetChild(iqNode , "si")) != NULL) {

			// fix for very smart clients, like gajim
			BOOL bDirect = m_options.BsDirect;
			BOOL bProxy = m_options.BsProxyManual;

			if ((featureNode = xmlGetChild(siNode , "feature")) != NULL) {
				if ((xNode = xmlGetChildByTag(featureNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS)) != NULL) {
					if ((fieldNode = xmlGetChildByTag(xNode, "field", "var", _T("stream-method"))) != NULL) {
						if ((valueNode = xmlGetChild(fieldNode , "value")) != NULL && xmlGetText(valueNode) != NULL) {
							if ((bDirect || bProxy) && !_tcscmp(xmlGetText(valueNode), JABBER_FEAT_BYTESTREAMS)) {
								// Start Bytestream session
								JABBER_BYTE_TRANSFER *jbt = new JABBER_BYTE_TRANSFER;
								memset(jbt, 0, sizeof(JABBER_BYTE_TRANSFER));
								jbt->srcJID = mir_tstrdup(pInfo->m_szTo);
								jbt->dstJID = mir_tstrdup(pInfo->m_szFrom);
								jbt->sid = mir_tstrdup(ft->sid);
								jbt->pfnSend = &CJabberProto::FtSend;
								jbt->pfnFinal = &CJabberProto::FtSendFinal;
								jbt->ft = ft;
								ft->type = FT_BYTESTREAM;
								ft->jbt = jbt;
								ForkThread((MyThreadFunc)&CJabberProto::ByteSendThread, jbt);
							}
							else if (!_tcscmp(xmlGetText(valueNode), JABBER_FEAT_IBB)) {
								JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *) mir_alloc(sizeof (JABBER_IBB_TRANSFER));
								memset(jibb, 0, sizeof(JABBER_IBB_TRANSFER));
								jibb->srcJID = mir_tstrdup(pInfo->m_szTo);
								jibb->dstJID = mir_tstrdup(pInfo->m_szFrom);
								jibb->sid = mir_tstrdup(ft->sid);
								jibb->pfnSend = &CJabberProto::FtIbbSend;
								jibb->pfnFinal = &CJabberProto::FtSendFinal;
								jibb->ft = ft;
								ft->type = FT_IBB;
								ft->jibb = jibb;
								ForkThread((MyThreadFunc)&CJabberProto::IbbSendThread, jibb);
	}	}	}	}	}	}	}
	else {
		debugLogA("File transfer stream initiation request denied or failed");
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR ? ACKRESULT_DENIED : ACKRESULT_FAILED, ft, 0);
		delete ft;
	}
}

BOOL CJabberProto::FtSend(HANDLE hConn, filetransfer *ft)
{
	struct _stati64 statbuf;
	int fd;
	char* buffer;
	int numRead;

	debugLog(_T("Sending [%s]"), ft->std.ptszFiles[ft->std.currentFileNumber]);
	_tstati64(ft->std.ptszFiles[ft->std.currentFileNumber], &statbuf);	// file size in statbuf.st_size
	if ((fd = _topen(ft->std.ptszFiles[ft->std.currentFileNumber], _O_BINARY|_O_RDONLY)) < 0) {
		debugLog(_T("File cannot be opened"));
		return FALSE;
	}

	ft->std.flags |= PFTS_SENDING;
	ft->std.currentFileSize = statbuf.st_size;
	ft->std.currentFileProgress = 0;

	if ((buffer=(char*)mir_alloc(2048)) != NULL) {
		while ((numRead=_read(fd, buffer, 2048)) > 0) {
			if (Netlib_Send(hConn, buffer, numRead, 0) != numRead) {
				mir_free(buffer);
				_close(fd);
				return FALSE;
			}
			ft->std.currentFileProgress += numRead;
			ft->std.totalProgress += numRead;
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
		}
		mir_free(buffer);
	}
	_close(fd);
	return TRUE;
}

BOOL CJabberProto::FtIbbSend(int blocksize, filetransfer *ft)
{
	debugLog(_T("Sending [%s]"), ft->std.ptszFiles[ft->std.currentFileNumber]);

	struct _stati64 statbuf;
	_tstati64(ft->std.ptszFiles[ft->std.currentFileNumber], &statbuf);	// file size in statbuf.st_size
	
	int fd = _topen(ft->std.ptszFiles[ft->std.currentFileNumber], _O_BINARY|_O_RDONLY);
	if (fd < 0) {
		debugLogA("File cannot be opened");
		return FALSE;
	}

	ft->std.flags |= PFTS_SENDING;
	ft->std.currentFileSize = statbuf.st_size;
	ft->std.currentFileProgress = 0;

	ptrA buffer((char*)mir_alloc(blocksize));
	if (buffer != NULL) {
		int numRead;
		while ((numRead = _read(fd, buffer, blocksize)) > 0) {
			int iqId = SerialNext();
			XmlNode msg(_T("message"));
			xmlAddAttr(msg, _T("to"), ft->jibb->dstJID);
			msg << XATTRID(iqId);

			// let others send data too
			Sleep(2);

			char *encoded = mir_base64_encode((PBYTE)(char*)buffer, numRead);

			msg << XCHILD(_T("data"), _A2T(encoded)) << XATTR(_T("xmlns"), JABBER_FEAT_IBB)
				<< XATTR(_T("sid"), ft->jibb->sid) << XATTRI(_T("seq"), ft->jibb->wPacketId);

			HXML ampNode = msg << XCHILDNS(_T("amp"), JABBER_FEAT_AMP);
			ampNode << XCHILD(_T("rule")) << XATTR(_T("condition"), _T("deliver-at"))
				<< XATTR(_T("value"), _T("stored")) << XATTR(_T("action"), _T("error"));
			ampNode << XCHILD(_T("rule")) << XATTR(_T("condition"), _T("match-resource"))
				<< XATTR(_T("value"), _T("exact")) << XATTR(_T("action"), _T("error"));
			ft->jibb->wPacketId++;

			mir_free(encoded);

			if (ft->jibb->state == JIBB_ERROR || ft->jibb->bStreamClosed || m_ThreadInfo->send(msg) == SOCKET_ERROR) {
				debugLogA("JabberFtIbbSend unsuccessful exit");
				_close(fd);
				return FALSE;
			}

			ft->jibb->dwTransferredSize += (DWORD)numRead;

			ft->std.currentFileProgress += numRead;
			ft->std.totalProgress += numRead;
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
		}
	}
	_close(fd);
	return TRUE;
}

void CJabberProto::FtSendFinal(BOOL success, filetransfer *ft)
{
	if (!success) {
		debugLogA("File transfer complete with error");
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ft->state == FT_DENIED ? ACKRESULT_DENIED : ACKRESULT_FAILED, ft, 0);
	}
	else {
		if (ft->std.currentFileNumber < ft->std.totalFiles-1) {
			ft->std.currentFileNumber++;
			replaceStrT(ft->std.tszCurrentFile, ft->std.ptszFiles[ft->std.currentFileNumber]);
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
			FtInitiate(ft->jid, ft);
			return;
		}

		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
	}

	delete ft;
}

///////////////// File receiving through stream initiation /////////////////////////

void CJabberProto::FtHandleSiRequest(HXML iqNode)
{
	const TCHAR *from, *sid, *str, *szId, *filename;
	HXML siNode, fileNode, featureNode, xNode, fieldNode, n;
	int i;
    unsigned __int64 filesize;

	if (!iqNode ||
		  (from = xmlGetAttrValue(iqNode, _T("from"))) == NULL ||
		  (str = xmlGetAttrValue(iqNode,  _T("type"))) == NULL || _tcscmp(str, _T("set")) ||
		  (siNode = xmlGetChildByTag(iqNode, "si", "xmlns", JABBER_FEAT_SI)) == NULL)
		return;

	szId = xmlGetAttrValue(iqNode,  _T("id"));
	if ((sid = xmlGetAttrValue(siNode,  _T("id"))) != NULL &&
		(fileNode = xmlGetChildByTag(siNode, "file", "xmlns", JABBER_FEAT_SI_FT)) != NULL &&
		(filename = xmlGetAttrValue(fileNode,  _T("name"))) != NULL &&
		(str = xmlGetAttrValue(fileNode,  _T("size"))) != NULL) {

		filesize = _ttoi64(str);
		if ((featureNode = xmlGetChildByTag(siNode, "feature", "xmlns", JABBER_FEAT_FEATURE_NEG)) != NULL &&
			(xNode = xmlGetChildByTag(featureNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS)) != NULL &&
			(fieldNode = xmlGetChildByTag(xNode, "field", "var", _T("stream-method"))) != NULL) {

			BOOL bIbbOnly = m_options.BsOnlyIBB;
			HXML optionNode = NULL;
			JABBER_FT_TYPE ftType = FT_OOB;

			if (!bIbbOnly) {
				for (i=0; ; i++) {
					optionNode = xmlGetChild(fieldNode ,i);
					if (!optionNode)
						break;

					if (!mir_tstrcmp(xmlGetName(optionNode), _T("option"))) {
						if ((n = xmlGetChild(optionNode , "value")) != NULL && xmlGetText(n)) {
							if (!_tcscmp(xmlGetText(n), JABBER_FEAT_BYTESTREAMS)) {
								ftType = FT_BYTESTREAM;
								break;
			}	}	}	}	}

			// try IBB only if bytestreams support not found or BsOnlyIBB flag exists
			if (bIbbOnly || !optionNode) {
				for (i=0; ; i++) {
					optionNode = xmlGetChild(fieldNode ,i);
					if (!optionNode)
						break;

					if (!mir_tstrcmp(xmlGetName(optionNode), _T("option"))) {
						if ((n = xmlGetChild(optionNode , "value")) != NULL && xmlGetText(n)) {
							if (!_tcscmp(xmlGetText(n), JABBER_FEAT_IBB)) {
								ftType = FT_IBB;
								break;
			}	}	}	}	}

			if (optionNode != NULL) {
				// Found known stream mechanism
				filetransfer *ft = new filetransfer(this);
				ft->dwExpectedRecvFileSize = filesize;
				ft->jid = mir_tstrdup(from);
				ft->std.hContact = HContactFromJID(from);
				ft->sid = mir_tstrdup(sid);
				ft->szId = mir_tstrdup(szId);
				ft->type = ftType;
				ft->std.totalFiles = 1;
				ft->std.tszCurrentFile = mir_tstrdup(filename);
				ft->std.totalBytes = ft->std.currentFileSize = filesize;

				PROTORECVFILET pre = { 0 };
				pre.dwFlags = PRFF_TCHAR;
				pre.fileCount = 1;
				pre.timestamp = time(NULL);
				pre.ptszFiles = (TCHAR**)&filename;
				pre.lParam = (LPARAM)ft;
				if ((n = xmlGetChild(fileNode , "desc")) != NULL)
					pre.tszDescription = (TCHAR*)xmlGetText(n);

				ProtoChainRecvFile(ft->std.hContact, &pre);
				return;
			}
			else {
				// Unknown stream mechanism
				XmlNodeIq iq(_T("error"), szId, from);
				HXML e = iq << XCHILD(_T("error")) << XATTRI(_T("code"), 400) << XATTR(_T("type"), _T("cancel"));
				e << XCHILDNS(_T("bad-request"), _T("urn:ietf:params:xml:ns:xmpp-stanzas"));
				e << XCHILDNS(_T("no-valid-streams"), JABBER_FEAT_SI);
				m_ThreadInfo->send(iq);
				return;
	}	}	}

	// Bad stream initiation, reply with bad-profile
	XmlNodeIq iq(_T("error"), szId, from);
	HXML e = iq << XCHILD(_T("error")) << XATTRI(_T("code"), 400) << XATTR(_T("type"), _T("cancel"));
	e << XCHILDNS(_T("bad-request"), _T("urn:ietf:params:xml:ns:xmpp-stanzas"));
	e << XCHILDNS(_T("bad-profile"), JABBER_FEAT_SI);
	m_ThreadInfo->send(iq);
}

void CJabberProto::FtAcceptSiRequest(filetransfer *ft)
{
	if (!m_bJabberOnline || ft == NULL || ft->jid == NULL || ft->sid == NULL) return;

	JABBER_LIST_ITEM *item;
	if ((item=ListAdd(LIST_FTRECV, ft->sid)) != NULL) {
		item->ft = ft;

		m_ThreadInfo->send(
			XmlNodeIq(_T("result"), ft->szId, ft->jid)
				<< XCHILDNS(_T("si"), JABBER_FEAT_SI)
				<< XCHILDNS(_T("feature"), JABBER_FEAT_FEATURE_NEG)
				<< XCHILDNS(_T("x"), JABBER_FEAT_DATA_FORMS) << XATTR(_T("type"), _T("submit"))
				<< XCHILD(_T("field")) << XATTR(_T("var"), _T("stream-method"))
				<< XCHILD(_T("value"), JABBER_FEAT_BYTESTREAMS));
}	}

void CJabberProto::FtAcceptIbbRequest(filetransfer *ft)
{
	if (!m_bJabberOnline || ft == NULL || ft->jid == NULL || ft->sid == NULL) return;

	JABBER_LIST_ITEM *item;
	if ((item=ListAdd(LIST_FTRECV, ft->sid)) != NULL) {
		item->ft = ft;

		m_ThreadInfo->send(
			XmlNodeIq(_T("result"), ft->szId, ft->jid)
				<< XCHILDNS(_T("si"), JABBER_FEAT_SI)
				<< XCHILDNS(_T("feature"), JABBER_FEAT_FEATURE_NEG)
				<< XCHILDNS(_T("x"), JABBER_FEAT_DATA_FORMS) << XATTR(_T("type"), _T("submit"))
				<< XCHILD(_T("field")) << XATTR(_T("var"), _T("stream-method"))
				<< XCHILD(_T("value"), JABBER_FEAT_IBB));
}	}

BOOL CJabberProto::FtHandleBytestreamRequest(HXML iqNode, CJabberIqInfo *pInfo)
{
	HXML queryNode = pInfo->GetChildNode();

	const TCHAR *sid;
	JABBER_LIST_ITEM *item;

	if ((sid = xmlGetAttrValue(queryNode, _T("sid"))) != NULL && (item = ListGetItemPtr(LIST_FTRECV, sid)) != NULL) {
		// Start Bytestream session
		JABBER_BYTE_TRANSFER *jbt = new JABBER_BYTE_TRANSFER;
		memset(jbt, 0, sizeof(JABBER_BYTE_TRANSFER));
		jbt->iqNode = xi.copyNode(iqNode);
		jbt->pfnRecv = &CJabberProto::FtReceive;
		jbt->pfnFinal = &CJabberProto::FtReceiveFinal;
		jbt->ft = item->ft;
		item->ft->jbt = jbt;
		ForkThread((MyThreadFunc)&CJabberProto::ByteReceiveThread, jbt);
		ListRemove(LIST_FTRECV, sid);
		return TRUE;
	}

	debugLogA("File transfer invalid bytestream initiation request received");
	return TRUE;
}

BOOL CJabberProto::FtHandleIbbRequest(HXML iqNode, BOOL bOpen)
{
	if (iqNode == NULL) return FALSE;

	const TCHAR *id = xmlGetAttrValue(iqNode, _T("id"));
	const TCHAR *from = xmlGetAttrValue(iqNode, _T("from"));
	const TCHAR *to = xmlGetAttrValue(iqNode, _T("to"));
	if (!id || !from || !to) return FALSE;

	HXML ibbNode = xmlGetChildByTag(iqNode, bOpen ? "open" : "close", "xmlns", JABBER_FEAT_IBB);
	if (!ibbNode) return FALSE;

	const TCHAR *sid = xmlGetAttrValue(ibbNode, _T("sid"));
	if (!sid) return FALSE;

	// already closed?
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTRECV, sid);
	if (item == NULL) {
		m_ThreadInfo->send(
			XmlNodeIq(_T("error"), id, from)
				<< XCHILD(_T("error")) << XATTRI(_T("code"), 404) << XATTR(_T("type"), _T("cancel"))
					<< XCHILDNS(_T("item-not-found"), _T("urn:ietf:params:xml:ns:xmpp-stanzas")));
		return FALSE;
	}

	// open event
	if (bOpen) {
		if (!item->jibb) {
			JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *) mir_alloc(sizeof(JABBER_IBB_TRANSFER));
			memset(jibb, 0, sizeof(JABBER_IBB_TRANSFER));
			jibb->srcJID = mir_tstrdup(from);
			jibb->dstJID = mir_tstrdup(to);
			jibb->sid = mir_tstrdup(sid);
			jibb->pfnRecv = &CJabberProto::FtReceive;
			jibb->pfnFinal = &CJabberProto::FtReceiveFinal;
			jibb->ft = item->ft;
			item->ft->jibb = jibb;
			item->jibb = jibb;
			ForkThread((MyThreadFunc)&CJabberProto::IbbReceiveThread, jibb);

			m_ThreadInfo->send( XmlNodeIq(_T("result"), id, from));
			return TRUE;
		}
		// stream already open
		m_ThreadInfo->send(
			XmlNodeIq(_T("error"), id, from)
				<< XCHILD(_T("error")) << XATTRI(_T("code"), 404) << XATTR(_T("type"), _T("cancel"))
					<< XCHILDNS(_T("item-not-found"), _T("urn:ietf:params:xml:ns:xmpp-stanzas")));
		return FALSE;
	}

	// close event && stream already open
	if (item->jibb && item->jibb->hEvent) {
		item->jibb->bStreamClosed = TRUE;
		SetEvent(item->jibb->hEvent);

		m_ThreadInfo->send( XmlNodeIq(_T("result"), id, from));
		return TRUE;
	}

	ListRemove(LIST_FTRECV, sid);

	return FALSE;
}

int CJabberProto::FtReceive(HANDLE, filetransfer *ft, char* buffer, int datalen)
{
	if (ft->create() == -1)
		return -1;

	__int64 remainingBytes = ft->std.currentFileSize - ft->std.currentFileProgress;
	if (remainingBytes > 0) {
		int writeSize = (remainingBytes<datalen) ? remainingBytes : datalen;
		if (_write(ft->fileId, buffer, writeSize) != writeSize) {
			debugLogA("_write() error");
			return -1;
		}

		ft->std.currentFileProgress += writeSize;
		ft->std.totalProgress += writeSize;
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_DATA, ft, (LPARAM)&ft->std);
		return (ft->std.currentFileSize == ft->std.currentFileProgress) ? 0 : writeSize;
	}

	return 0;
}

void CJabberProto::FtReceiveFinal(BOOL success, filetransfer *ft)
{
	if (success) {
		debugLogA("File transfer complete successfully");
		ft->complete();
	}
	else debugLogA("File transfer complete with error");

	delete ft;
}
