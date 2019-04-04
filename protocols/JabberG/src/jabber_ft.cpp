/*

Jabber Protocol Plugin for Miranda NG

Copyright (c) 2002-04  Santithorn Bunchua
Copyright (c) 2005-12  George Hazan
Copyright (c) 2007     Maxim Mluhov
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
	if (jbt != nullptr) {
		debugLogA("Canceling bytestream session");
		jbt->state = JBT_ERROR;
		if (jbt->hConn) {
			debugLogA("Force closing bytestream session");
			Netlib_CloseHandle(jbt->hConn);
			jbt->hConn = nullptr;
		}
		if (jbt->hSendEvent) SetEvent(jbt->hSendEvent);
		if (jbt->hEvent) SetEvent(jbt->hEvent);
		if (jbt->hProxyEvent) SetEvent(jbt->hProxyEvent);
	}

	// For file transfer through IBB
	JABBER_IBB_TRANSFER *jibb = ft->jibb;
	if (jibb != nullptr) {
		debugLogA("Canceling IBB session");
		jibb->state = JIBB_ERROR;
		m_iqManager.ExpireByUserData(jibb);
	}
}

///////////////// File sending using stream initiation /////////////////////////

void CJabberProto::FtInitiate(char* jid, filetransfer *ft)
{
	char *rs;
	int i;
	char sid[9];

	if (jid == nullptr || ft == nullptr || !m_bJabberOnline || (rs = ListGetBestClientResourceNamePtr(jid)) == nullptr) {
		if (ft) {
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_FAILED, ft, 0);
			delete ft;
		}
		return;
	}
	ft->type = FT_SI;
	for (i = 0; i < 8; i++)
		sid[i] = (rand() % 10) + '0';
	sid[8] = '\0';
	replaceStr(ft->sid, sid);
	wchar_t *filename = ft->std.pszFiles.w[ft->std.currentFileNumber];
	if (wchar_t *p = wcsrchr(filename, '\\'))
		filename = p + 1;

	XmlNodeIq iq(AddIQ(&CJabberProto::OnFtSiResult, JABBER_IQ_TYPE_SET, MakeJid(jid, rs), JABBER_IQ_PARSE_FROM | JABBER_IQ_PARSE_TO, -1, ft));
	TiXmlElement *si = iq << XCHILDNS("si", JABBER_FEAT_SI) << XATTR("id", sid)
		<< XATTR("mime-type", "binary/octet-stream") << XATTR("profile", JABBER_FEAT_SI_FT);
	si << XCHILDNS("file", JABBER_FEAT_SI_FT) << XATTR("name", T2Utf(filename))
		<< XATTRI64("size", ft->fileSize[ft->std.currentFileNumber]) << XCHILD("desc", T2Utf(ft->szDescription));

	TiXmlElement *field = si << XCHILDNS("feature", JABBER_FEAT_FEATURE_NEG)
		<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "form")
		<< XCHILD("field") << XATTR("var", "stream-method") << XATTR("type", "list-single");

	BOOL bDirect = m_bBsDirect;
	BOOL bProxy = m_bBsProxyManual;

	// bytestreams support?
	if (bDirect || bProxy)
		field << XCHILD("option") << XCHILD("value", JABBER_FEAT_BYTESTREAMS);

	field << XCHILD("option") << XCHILD("value", JABBER_FEAT_IBB);
	m_ThreadInfo->send(iq);
}

void CJabberProto::OnFtSiResult(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	filetransfer *ft = (filetransfer *)pInfo->GetUserData();
	if (!ft)
		return;

	if ((pInfo->GetIqType() == JABBER_IQ_TYPE_RESULT) && pInfo->m_szFrom && pInfo->m_szTo) {
		if (auto *siNode = XmlFirstChild(iqNode, "si")) {
			// fix for very smart clients, like gajim
			BOOL bDirect = m_bBsDirect;
			BOOL bProxy = m_bBsProxyManual;

			if (auto *featureNode = XmlFirstChild(siNode, "feature")) {
				if (auto *xNode = XmlGetChildByTag(featureNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS)) {
					if (auto *fieldNode = XmlGetChildByTag(xNode, "field", "var", "stream-method")) {
						if (auto *valueNode = XmlFirstChild(fieldNode, "value")) {
							if ((bDirect || bProxy) && !mir_strcmp(valueNode->GetText(), JABBER_FEAT_BYTESTREAMS)) {
								// Start Bytestream session
								JABBER_BYTE_TRANSFER *jbt = new JABBER_BYTE_TRANSFER;
								memset(jbt, 0, sizeof(JABBER_BYTE_TRANSFER));
								jbt->srcJID = mir_strdup(pInfo->m_szTo);
								jbt->dstJID = mir_strdup(pInfo->m_szFrom);
								jbt->sid = mir_strdup(ft->sid);
								jbt->pfnSend = &CJabberProto::FtSend;
								jbt->pfnFinal = &CJabberProto::FtSendFinal;
								jbt->ft = ft;
								ft->type = FT_BYTESTREAM;
								ft->jbt = jbt;
								ForkThread((MyThreadFunc)&CJabberProto::ByteSendThread, jbt);
							}
							else if (!mir_strcmp(valueNode->GetText(), JABBER_FEAT_IBB)) {
								JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *)mir_alloc(sizeof(JABBER_IBB_TRANSFER));
								memset(jibb, 0, sizeof(JABBER_IBB_TRANSFER));
								jibb->srcJID = mir_strdup(pInfo->m_szTo);
								jibb->dstJID = mir_strdup(pInfo->m_szFrom);
								jibb->sid = mir_strdup(ft->sid);
								jibb->pfnSend = &CJabberProto::FtIbbSend;
								jibb->pfnFinal = &CJabberProto::FtSendFinal;
								jibb->ft = ft;
								ft->type = FT_IBB;
								ft->jibb = jibb;
								ForkThread((MyThreadFunc)&CJabberProto::IbbSendThread, jibb);
							}
						}
					}
				}
			}
		}
	}
	else {
		debugLogA("File transfer stream initiation request denied or failed");
		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, pInfo->GetIqType() == JABBER_IQ_TYPE_ERROR ? ACKRESULT_DENIED : ACKRESULT_FAILED, ft, 0);
		delete ft;
	}
}

BOOL CJabberProto::FtSend(HNETLIBCONN hConn, filetransfer *ft)
{
	struct _stati64 statbuf;
	int fd;
	char* buffer;
	int numRead;

	debugLogW(L"Sending [%s]", ft->std.pszFiles.w[ft->std.currentFileNumber]);
	_wstat64(ft->std.pszFiles.w[ft->std.currentFileNumber], &statbuf);	// file size in statbuf.st_size
	if ((fd = _wopen(ft->std.pszFiles.w[ft->std.currentFileNumber], _O_BINARY | _O_RDONLY)) < 0) {
		debugLogA("File cannot be opened");
		return FALSE;
	}

	ft->std.flags |= PFTS_SENDING;
	ft->std.currentFileSize = statbuf.st_size;
	ft->std.currentFileProgress = 0;

	if ((buffer = (char*)mir_alloc(2048)) != nullptr) {
		while ((numRead = _read(fd, buffer, 2048)) > 0) {
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
	debugLogW(L"Sending [%s]", ft->std.pszFiles.w[ft->std.currentFileNumber]);

	struct _stati64 statbuf;
	_wstat64(ft->std.pszFiles.w[ft->std.currentFileNumber], &statbuf);	// file size in statbuf.st_size

	int fd = _wopen(ft->std.pszFiles.w[ft->std.currentFileNumber], _O_BINARY | _O_RDONLY);
	if (fd < 0) {
		debugLogA("File cannot be opened");
		return FALSE;
	}

	ft->std.flags |= PFTS_SENDING;
	ft->std.currentFileSize = statbuf.st_size;
	ft->std.currentFileProgress = 0;

	ptrA buffer((char*)mir_alloc(blocksize));
	if (buffer != nullptr) {
		int numRead;
		while ((numRead = _read(fd, buffer, blocksize)) > 0) {
			int iqId = SerialNext();
			XmlNode msg("message");
			XmlAddAttr(msg, "to", ft->jibb->dstJID);
			msg << XATTRID(iqId);

			// let others send data too
			Sleep(2);

			msg << XCHILD("data", ptrA(mir_base64_encode(buffer, numRead))) << XATTR("xmlns", JABBER_FEAT_IBB)
				<< XATTR("sid", ft->jibb->sid) << XATTRI("seq", ft->jibb->wPacketId);

			TiXmlElement *ampNode = msg << XCHILDNS("amp", JABBER_FEAT_AMP);
			ampNode << XCHILD("rule") << XATTR("condition", "deliver-at")
				<< XATTR("value", "stored") << XATTR("action", "error");
			ampNode << XCHILD("rule") << XATTR("condition", "match-resource")
				<< XATTR("value", "exact") << XATTR("action", "error");
			ft->jibb->wPacketId++;

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
		if (ft->std.currentFileNumber < ft->std.totalFiles - 1) {
			ft->std.currentFileNumber++;
			replaceStrW(ft->std.szCurrentFile.w, ft->std.pszFiles.w[ft->std.currentFileNumber]);
			ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_NEXTFILE, ft, 0);
			FtInitiate(ft->jid, ft);
			return;
		}

		ProtoBroadcastAck(ft->std.hContact, ACKTYPE_FILE, ACKRESULT_SUCCESS, ft, 0);
	}

	delete ft;
}

///////////////// File receiving through stream initiation /////////////////////////

void CJabberProto::FtHandleSiRequest(const TiXmlElement *iqNode)
{
	const char *from, *sid, *str, *filename;
	const TiXmlElement *siNode, *fileNode, *featureNode, *xNode, *fieldNode;

	if (!iqNode ||
		(from = XmlGetAttr(iqNode, "from")) == nullptr ||
		(str = XmlGetAttr(iqNode, "type")) == nullptr || mir_strcmp(str, "set") ||
		(siNode = XmlGetChildByTag(iqNode, "si", "xmlns", JABBER_FEAT_SI)) == nullptr)
		return;

	const char *szId = XmlGetAttr(iqNode, "id");
	if ((sid = XmlGetAttr(siNode, "id")) != nullptr &&
		(fileNode = XmlGetChildByTag(siNode, "file", "xmlns", JABBER_FEAT_SI_FT)) != nullptr &&
		(filename = XmlGetAttr(fileNode, "name")) != nullptr &&
		(str = XmlGetAttr(fileNode, "size")) != nullptr) {

		unsigned __int64 filesize = _atoi64(str);
		if ((featureNode = XmlGetChildByTag(siNode, "feature", "xmlns", JABBER_FEAT_FEATURE_NEG)) != nullptr &&
			(xNode = XmlGetChildByTag(featureNode, "x", "xmlns", JABBER_FEAT_DATA_FORMS)) != nullptr &&
			(fieldNode = XmlGetChildByTag(xNode, "field", "var", "stream-method")) != nullptr) {

			BOOL bIbbOnly = m_bBsOnlyIBB;
			const TiXmlElement *optionNode = nullptr;
			JABBER_FT_TYPE ftType = FT_OOB;

			if (!bIbbOnly) {
				for (auto *it : TiXmlFilter(fieldNode, "option")) {
					if (!mir_strcmp(XmlGetChildText(it, "value"), JABBER_FEAT_BYTESTREAMS)) {
						optionNode = it;
						ftType = FT_BYTESTREAM;
						break;
					}
				}
			}

			// try IBB only if bytestreams support not found or BsOnlyIBB flag exists
			if (bIbbOnly || !optionNode) {
				for (auto *it : TiXmlFilter(fieldNode, "option")) {
					if (!mir_strcmp(XmlGetChildText(it, "value"), JABBER_FEAT_IBB)) {
						optionNode = it;
						ftType = FT_IBB;
						break;
					}
				}
			}

			if (optionNode != nullptr) {
				// Found known stream mechanism
				filetransfer *ft = new filetransfer(this);
				ft->dwExpectedRecvFileSize = filesize;
				ft->jid = mir_strdup(from);
				ft->std.hContact = HContactFromJID(from);
				ft->sid = mir_strdup(sid);
				ft->szId = mir_strdup(szId);
				ft->type = ftType;
				ft->std.totalFiles = 1;
				ft->std.szCurrentFile.w = mir_utf8decodeW(filename);
				ft->std.totalBytes = ft->std.currentFileSize = filesize;

				PROTORECVFILE pre = { 0 };
				pre.fileCount = 1;
				pre.timestamp = time(0);
				pre.files.a = &filename;
				pre.lParam = (LPARAM)ft;
				pre.descr.a = XmlGetChildText(fileNode, "desc");
				ProtoChainRecvFile(ft->std.hContact, &pre);
				return;
			}
		}
	}

	// Bad stream initiation, reply with bad-profile
	XmlNodeIq iq("error", szId, from);
	TiXmlElement *e = iq << XCHILD("error") << XATTRI("code", 400) << XATTR("type", "cancel");
	e << XCHILDNS("bad-request", "urn:ietf:params:xml:ns:xmpp-stanzas");
	e << XCHILDNS("bad-profile", JABBER_FEAT_SI);
	m_ThreadInfo->send(iq);
}

void CJabberProto::FtAcceptSiRequest(filetransfer *ft)
{
	if (!m_bJabberOnline || ft == nullptr || ft->jid == nullptr || ft->sid == nullptr) return;

	JABBER_LIST_ITEM *item;
	if ((item = ListAdd(LIST_FTRECV, ft->sid)) != nullptr) {
		item->ft = ft;

		m_ThreadInfo->send(
			XmlNodeIq("result", ft->szId, ft->jid)
			<< XCHILDNS("si", JABBER_FEAT_SI)
			<< XCHILDNS("feature", JABBER_FEAT_FEATURE_NEG)
			<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "submit")
			<< XCHILD("field") << XATTR("var", "stream-method")
			<< XCHILD("value", JABBER_FEAT_BYTESTREAMS));
	}
}

void CJabberProto::FtAcceptIbbRequest(filetransfer *ft)
{
	if (!m_bJabberOnline || ft == nullptr || ft->jid == nullptr || ft->sid == nullptr) return;

	JABBER_LIST_ITEM *item;
	if ((item = ListAdd(LIST_FTRECV, ft->sid)) != nullptr) {
		item->ft = ft;

		m_ThreadInfo->send(
			XmlNodeIq("result", ft->szId, ft->jid)
			<< XCHILDNS("si", JABBER_FEAT_SI)
			<< XCHILDNS("feature", JABBER_FEAT_FEATURE_NEG)
			<< XCHILDNS("x", JABBER_FEAT_DATA_FORMS) << XATTR("type", "submit")
			<< XCHILD("field") << XATTR("var", "stream-method")
			<< XCHILD("value", JABBER_FEAT_IBB));
	}
}

BOOL CJabberProto::FtHandleBytestreamRequest(const TiXmlElement *iqNode, CJabberIqInfo *pInfo)
{
	auto *queryNode = pInfo->GetChildNode();

	const char *sid = XmlGetAttr(queryNode, "sid");
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTRECV, sid);

	if ((sid ) != nullptr && (item ) != nullptr) {
		// Start Bytestream session
		JABBER_BYTE_TRANSFER *jbt = new JABBER_BYTE_TRANSFER;
		jbt->iqNode = iqNode->DeepClone(&jbt->doc)->ToElement();
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

BOOL CJabberProto::FtHandleIbbRequest(const TiXmlElement *iqNode, BOOL bOpen)
{
	if (iqNode == nullptr) return FALSE;

	const char *id = XmlGetAttr(iqNode, "id");
	const char *from = XmlGetAttr(iqNode, "from");
	const char *to = XmlGetAttr(iqNode, "to");
	if (!id || !from || !to) return FALSE;

	auto *ibbNode = XmlGetChildByTag(iqNode, bOpen ? "open" : "close", "xmlns", JABBER_FEAT_IBB);
	if (!ibbNode) return FALSE;

	const char *sid = XmlGetAttr(ibbNode, "sid");
	if (!sid) return FALSE;

	// already closed?
	JABBER_LIST_ITEM *item = ListGetItemPtr(LIST_FTRECV, sid);
	if (item == nullptr) {
		m_ThreadInfo->send(
			XmlNodeIq("error", id, from)
			<< XCHILD("error") << XATTRI("code", 404) << XATTR("type", "cancel")
			<< XCHILDNS("item-not-found", "urn:ietf:params:xml:ns:xmpp-stanzas"));
		return FALSE;
	}

	// open event
	if (bOpen) {
		if (!item->jibb) {
			JABBER_IBB_TRANSFER *jibb = (JABBER_IBB_TRANSFER *)mir_alloc(sizeof(JABBER_IBB_TRANSFER));
			memset(jibb, 0, sizeof(JABBER_IBB_TRANSFER));
			jibb->srcJID = mir_strdup(from);
			jibb->dstJID = mir_strdup(to);
			jibb->sid = mir_strdup(sid);
			jibb->pfnRecv = &CJabberProto::FtReceive;
			jibb->pfnFinal = &CJabberProto::FtReceiveFinal;
			jibb->ft = item->ft;
			item->ft->jibb = jibb;
			item->jibb = jibb;
			ForkThread((MyThreadFunc)&CJabberProto::IbbReceiveThread, jibb);

			m_ThreadInfo->send(XmlNodeIq("result", id, from));
			return TRUE;
		}
		// stream already open
		m_ThreadInfo->send(
			XmlNodeIq("error", id, from)
			<< XCHILD("error") << XATTRI("code", 404) << XATTR("type", "cancel")
			<< XCHILDNS("item-not-found", "urn:ietf:params:xml:ns:xmpp-stanzas"));
		return FALSE;
	}

	// close event && stream already open
	if (item->jibb && item->jibb->hEvent) {
		item->jibb->bStreamClosed = TRUE;
		SetEvent(item->jibb->hEvent);

		m_ThreadInfo->send(XmlNodeIq("result", id, from));
		return TRUE;
	}

	ListRemove(LIST_FTRECV, sid);

	return FALSE;
}

int CJabberProto::FtReceive(HNETLIBCONN, filetransfer *ft, char* buffer, int datalen)
{
	if (ft->create() == -1)
		return -1;

	__int64 remainingBytes = ft->std.currentFileSize - ft->std.currentFileProgress;
	if (remainingBytes > 0) {
		int writeSize = (remainingBytes < datalen) ? remainingBytes : datalen;
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
